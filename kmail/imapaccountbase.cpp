/** -*- c++ -*-
 * imapaccountbase.cpp
 *
 * Copyright (c) 2000-2002 Michael Haeckel <haeckel@kde.org>
 * Copyright (c) 2002 Marc Mutz <mutz@kde.org>
 *
 * This file is based on work on pop3 and imap account implementations
 * by Don Sanders <sanders@kde.org> and Michael Haeckel <haeckel@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "imapaccountbase.h"
using KMail::SieveConfig;

#include "kmacctmgr.h"
#include "kmfolder.h"
#include "broadcaststatus.h"
using KPIM::BroadcastStatus;
#include "kmmainwin.h"
#include "kmfolderimap.h"
#include "kmmainwidget.h"
#include "kmmainwin.h"
#include "kmmsgpart.h"
#include "acljobs.h"
#include "kmfoldercachedimap.h"
#include "bodyvisitor.h"
using KMail::BodyVisitor;
#include "imapjob.h"
using KMail::ImapJob;
#include "protocols.h"
#include "progressmanager.h"
using KPIM::ProgressManager;
#include "kmfoldermgr.h"
#include "listjob.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
using KIO::MetaData;
#include <kio/passdlg.h>
using KIO::PasswordDialog;
#include <kio/scheduler.h>
#include <kio/slave.h>
#include <mimelib/bodypart.h>
#include <mimelib/body.h>
#include <mimelib/headers.h>
#include <mimelib/message.h>
//using KIO::Scheduler; // use FQN below

#include <qregexp.h>
#include <qstylesheet.h>

namespace KMail {

  static const unsigned short int imapDefaultPort = 143;

  //
  //
  // Ctor and Dtor
  //
  //

  ImapAccountBase::ImapAccountBase( KMAcctMgr * parent, const QString & name, uint id )
    : NetworkAccount( parent, name, id ),
      mTotal( 0 ),
      mCountUnread( 0 ),
      mCountLastUnread( 0 ),
      mAutoExpunge( true ),
      mHiddenFolders( false ),
      mOnlySubscribedFolders( false ),
      mLoadOnDemand( true ),
      mListOnlyOpenFolders( false ),
      mProgressEnabled( false ),
      mErrorDialogIsActive( false ),
      mPasswordDialogIsActive( false ),
      mACLSupport( true ),
      mAnnotationSupport( true ),
      mSlaveConnected( false ),
      mSlaveConnectionError( false ),
      mCheckingSingleFolder( false ),
      mListDirProgressItem( 0 )
  {
    mPort = imapDefaultPort;
    mBodyPartList.setAutoDelete(true);
    KIO::Scheduler::connect(SIGNAL(slaveError(KIO::Slave *, int, const QString &)),
                            this, SLOT(slotSchedulerSlaveError(KIO::Slave *, int, const QString &)));
    KIO::Scheduler::connect(SIGNAL(slaveConnected(KIO::Slave *)),
                            this, SLOT(slotSchedulerSlaveConnected(KIO::Slave *)));
    connect(&mNoopTimer, SIGNAL(timeout()), SLOT(slotNoopTimeout()));
    connect(&mIdleTimer, SIGNAL(timeout()), SLOT(slotIdleTimeout()));
  }

  ImapAccountBase::~ImapAccountBase() {
    kdWarning( mSlave, 5006 )
      << "slave should have been destroyed by subclass!" << endl;
  }

  void ImapAccountBase::init() {
    mAutoExpunge = true;
    mHiddenFolders = false;
    mOnlySubscribedFolders = false;
    mLoadOnDemand = true;
    mListOnlyOpenFolders = false;
    mProgressEnabled = false;
  }

  void ImapAccountBase::pseudoAssign( const KMAccount * a ) {
    NetworkAccount::pseudoAssign( a );

    const ImapAccountBase * i = dynamic_cast<const ImapAccountBase*>( a );
    if ( !i ) return;

    setAutoExpunge( i->autoExpunge() );
    setHiddenFolders( i->hiddenFolders() );
    setOnlySubscribedFolders( i->onlySubscribedFolders() );
    setLoadOnDemand( i->loadOnDemand() );
    setListOnlyOpenFolders( i->listOnlyOpenFolders() );
    setNamespaces( i->namespaces() );
    setNamespaceToDelimiter( i->namespaceToDelimiter() );
  }

  unsigned short int ImapAccountBase::defaultPort() const {
    return imapDefaultPort;
  }

  QString ImapAccountBase::protocol() const {
    return useSSL() ? IMAP_SSL_PROTOCOL : IMAP_PROTOCOL;
  }

  //
  //
  // Getters and Setters
  //
  //

  void ImapAccountBase::setAutoExpunge( bool expunge ) {
    mAutoExpunge = expunge;
  }

  void ImapAccountBase::setHiddenFolders( bool show ) {
    mHiddenFolders = show;
  }

  void ImapAccountBase::setOnlySubscribedFolders( bool show ) {
    mOnlySubscribedFolders = show;
  }

  void ImapAccountBase::setLoadOnDemand( bool load ) {
    mLoadOnDemand = load;
  }

  void ImapAccountBase::setListOnlyOpenFolders( bool only ) {
    mListOnlyOpenFolders = only;
  }

  //
  //
  // read/write config
  //
  //

  void ImapAccountBase::readConfig( /*const*/ KConfig/*Base*/ & config ) {
    NetworkAccount::readConfig( config );

    setAutoExpunge( config.readBoolEntry( "auto-expunge", false ) );
    setHiddenFolders( config.readBoolEntry( "hidden-folders", false ) );
    setOnlySubscribedFolders( config.readBoolEntry( "subscribed-folders", false ) );
    setLoadOnDemand( config.readBoolEntry( "loadondemand", false ) );
    setListOnlyOpenFolders( config.readBoolEntry( "listOnlyOpenFolders", false ) );
    // read namespaces
    nsMap map;
    QStringList list = config.readListEntry( QString::number( PersonalNS ) );
    if ( !list.isEmpty() )
      map[PersonalNS] = list.gres( "\"", "" );
    list = config.readListEntry( QString::number( OtherUsersNS ) );
    if ( !list.isEmpty() )
      map[OtherUsersNS] = list.gres( "\"", "" );
    list = config.readListEntry( QString::number( SharedNS ) );
    if ( !list.isEmpty() )
      map[SharedNS] = list.gres( "\"", "" );
    setNamespaces( map );
    // read namespace - delimiter
    namespaceDelim entries = config.entryMap( config.group() );
    namespaceDelim namespaceToDelimiter;
    for ( namespaceDelim::ConstIterator it = entries.begin(); 
          it != entries.end(); ++it )
    {
      if ( it.key().startsWith( "Namespace:" ) )
      {
        QString key = it.key().right( it.key().length() - 10 );
        namespaceToDelimiter[key] = it.data();
      }
    }
    setNamespaceToDelimiter( namespaceToDelimiter );
    mOldPrefix = config.readEntry( "prefix" );
    if ( !mOldPrefix.isEmpty() ) {
      makeConnection();
    }
  }

  void ImapAccountBase::writeConfig( KConfig/*Base*/ & config ) /*const*/ {
    NetworkAccount::writeConfig( config );

    config.writeEntry( "auto-expunge", autoExpunge() );
    config.writeEntry( "hidden-folders", hiddenFolders() );
    config.writeEntry( "subscribed-folders", onlySubscribedFolders() );
    config.writeEntry( "loadondemand", loadOnDemand() );
    config.writeEntry( "listOnlyOpenFolders", listOnlyOpenFolders() );
    QString data;
    for ( nsMap::Iterator it = mNamespaces.begin(); it != mNamespaces.end(); ++it )
    {
      if ( !it.data().isEmpty() )
      {
        data = "\"" + it.data().join("\",\"") + "\"";
        config.writeEntry( QString::number( it.key() ), data );
      }
    }
    QString key;
    for ( namespaceDelim::ConstIterator it = mNamespaceToDelimiter.begin(); 
          it != mNamespaceToDelimiter.end(); ++it )
    {
      key = "Namespace:" + it.key();
      config.writeEntry( key, it.data() );
    }
  }

  //
  //
  // Network processing
  //
  //

  MetaData ImapAccountBase::slaveConfig() const {
    MetaData m = NetworkAccount::slaveConfig();

    m.insert( "auth", auth() );
    if ( autoExpunge() )
      m.insert( "expunge", "auto" );

    return m;
  }

  ImapAccountBase::ConnectionState ImapAccountBase::makeConnection() 
  {
    if ( mSlave && mSlaveConnected ) {
      return Connected;
    }
    if ( mPasswordDialogIsActive ) return Connecting;

    if( mAskAgain || ( ( passwd().isEmpty() || login().isEmpty() ) &&
                         auth() != "GSSAPI" ) ) {

      Q_ASSERT( !mSlave ); // disconnected on 'wrong login' error already, or first try
      QString log = login();
      QString pass = passwd();
      // We init "store" to true to indicate that we want to have the
      // "keep password" checkbox. Then, we set [Passwords]Keep to
      // storePasswd(), so that the checkbox in the dialog will be
      // init'ed correctly:
      bool store = true;
      KConfigGroup passwords( KGlobal::config(), "Passwords" );
      passwords.writeEntry( "Keep", storePasswd() );
      QString msg = i18n("You need to supply a username and a password to "
			 "access this mailbox.");
      mPasswordDialogIsActive = true;
      if ( PasswordDialog::getNameAndPassword( log, pass, &store, msg, false,
					       QString::null, name(),
					       i18n("Account:") )
          != QDialog::Accepted ) {
        mPasswordDialogIsActive = false;
        mAskAgain = false;
        emit connectionResult( KIO::ERR_USER_CANCELED, QString::null );
        return Error;
      }
      mPasswordDialogIsActive = false;
      // The user has been given the chance to change login and
      // password, so copy both from the dialog:
      setPasswd( pass, store );
      setLogin( log );
      mAskAgain = false;
    }
    // already waiting for a connection?
    if ( mSlave && !mSlaveConnected ) return Connecting;

    mSlaveConnected = false;
    mSlave = KIO::Scheduler::getConnectedSlave( getUrl(), slaveConfig() );
    if ( !mSlave ) {
      KMessageBox::error(0, i18n("Could not start process for %1.")
			 .arg( getUrl().protocol() ) );
      return Error;
    }
    if ( mSlave->isConnected() ) {
      slotSchedulerSlaveConnected( mSlave );
      return Connected;
    }

    return Connecting;
  }

  bool ImapAccountBase::handleJobError( KIO::Job *job, const QString& context, bool abortSync )
  {
    JobIterator it = findJob( job );
    if ( it != jobsEnd() && (*it).progressItem )
    {
      (*it).progressItem->setComplete();
      (*it).progressItem = 0;
    }
    return handleError( job->error(), job->errorText(), job, context, abortSync );
  }

  // Called when we're really all done.
  void ImapAccountBase::postProcessNewMail( bool showStatusMsg ) {
    setCheckingMail(false);
    int newMails = 0;
    if ( mCountUnread > 0 && mCountUnread > mCountLastUnread ) {
      newMails = mCountUnread  - mCountLastUnread;
      mCountLastUnread = mCountUnread;
      mCountUnread = 0;
      checkDone( true, CheckOK );
    } else {
      mCountUnread = 0;
      checkDone( false, CheckOK );
    }
    if ( showStatusMsg )
      BroadcastStatus::instance()->setStatusMsgTransmissionCompleted(
          name(), newMails);
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::changeSubscription( bool subscribe, const QString& imapPath )
  {
    // change the subscription of the folder
    KURL url = getUrl();
    url.setPath(imapPath);

    QByteArray packedArgs;
    QDataStream stream( packedArgs, IO_WriteOnly);

    if (subscribe)
      stream << (int) 'u' << url;
    else
      stream << (int) 'U' << url;

    // create the KIO-job
    if ( makeConnection() != Connected ) 
      return;// ## doesn't handle Connecting
    KIO::SimpleJob *job = KIO::special(url, packedArgs, FALSE);
    KIO::Scheduler::assignJobToSlave(mSlave, job);
    jobData jd( url.url(), NULL );
    // a bit of a hack to save one slot
    if (subscribe) jd.onlySubscribed = true;
    else jd.onlySubscribed = false;
    insertJob(job, jd);

    connect(job, SIGNAL(result(KIO::Job *)),
        SLOT(slotSubscriptionResult(KIO::Job *)));
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::slotSubscriptionResult( KIO::Job * job )
  {
    // result of a subscription-job
    JobIterator it = findJob( job );
    if ( it == jobsEnd() ) return;
    bool onlySubscribed = (*it).onlySubscribed;
    QString path = static_cast<KIO::SimpleJob*>(job)->url().path();
    if (job->error())
    {
      handleJobError( job, i18n( "Error while trying to subscribe to %1:" ).arg( path ) + '\n' );
      // ## emit subscriptionChanged here in case anyone needs it to support continue/cancel
    }
    else
    {
      emit subscriptionChanged( path, onlySubscribed );
      if (mSlave) removeJob(job);
    }
  }

  //-----------------------------------------------------------------------------
  // TODO imapPath can be removed once parent can be a KMFolderImapBase or whatever
  void ImapAccountBase::getUserRights( KMFolder* parent, const QString& imapPath )
  {
    // There isn't much point in asking the server about a user's rights on his own inbox,
    // it might not be the effective permissions (at least with Cyrus, one can admin his own inbox,
    // even after a SETACL that removes the admin permissions. Other imap servers apparently
    // don't even allow removing one's own admin permission, so this code won't hurt either).
    if ( imapPath == "/INBOX/" ) {
      if ( parent->folderType() == KMFolderTypeImap )
        static_cast<KMFolderImap*>( parent->storage() )->setUserRights( ACLJobs::All );
      else if ( parent->folderType() == KMFolderTypeCachedImap )
        static_cast<KMFolderCachedImap*>( parent->storage() )->setUserRights( ACLJobs::All );
      emit receivedUserRights( parent ); // warning, you need to connect first to get that one
      return;
    }

    KURL url = getUrl();
    url.setPath(imapPath);

    ACLJobs::GetUserRightsJob* job = ACLJobs::getUserRights( mSlave, url );

    jobData jd( url.url(), parent );
    jd.cancellable = true;
    insertJob(job, jd);

    connect(job, SIGNAL(result(KIO::Job *)),
            SLOT(slotGetUserRightsResult(KIO::Job *)));
  }

  void ImapAccountBase::slotGetUserRightsResult( KIO::Job* _job )
  {
    ACLJobs::GetUserRightsJob* job = static_cast<ACLJobs::GetUserRightsJob *>( _job );
    JobIterator it = findJob( job );
    if ( it == jobsEnd() ) return;

    KMFolder* folder = (*it).parent;
    if ( job->error() ) {
      if ( job->error() == KIO::ERR_UNSUPPORTED_ACTION ) // that's when the imap server doesn't support ACLs
          mACLSupport = false;
      else
        kdWarning(5006) << "slotGetUserRightsResult: " << job->errorString() << endl;
    } else {
#ifndef NDEBUG
      //kdDebug(5006) << "User Rights: " << ACLJobs::permissionsToString( job->permissions() ) << endl;
#endif
      // Store the permissions
      if ( folder->folderType() == KMFolderTypeImap )
        static_cast<KMFolderImap*>( folder->storage() )->setUserRights( job->permissions() );
      else if ( folder->folderType() == KMFolderTypeCachedImap )
        static_cast<KMFolderCachedImap*>( folder->storage() )->setUserRights( job->permissions() );
    }
    if (mSlave) removeJob(job);
    emit receivedUserRights( folder );
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::getACL( KMFolder* parent, const QString& imapPath )
  {
    KURL url = getUrl();
    url.setPath(imapPath);

    ACLJobs::GetACLJob* job = ACLJobs::getACL( mSlave, url );
    jobData jd( url.url(), parent );
    jd.cancellable = true;
    insertJob(job, jd);

    connect(job, SIGNAL(result(KIO::Job *)),
            SLOT(slotGetACLResult(KIO::Job *)));
  }

  void ImapAccountBase::slotGetACLResult( KIO::Job* _job )
  {
    ACLJobs::GetACLJob* job = static_cast<ACLJobs::GetACLJob *>( _job );
    JobIterator it = findJob( job );
    if ( it == jobsEnd() ) return;

    KMFolder* folder = (*it).parent;
    emit receivedACL( folder, job, job->entries() );
    if (mSlave) removeJob(job);
  }


  void ImapAccountBase::slotNoopTimeout()
  {
    if ( mSlave ) {
      QByteArray packedArgs;
      QDataStream stream( packedArgs, IO_WriteOnly );

      stream << ( int ) 'N';

      KIO::SimpleJob *job = KIO::special( getUrl(), packedArgs, false );
      KIO::Scheduler::assignJobToSlave(mSlave, job);
      connect( job, SIGNAL(result( KIO::Job * ) ),
          this, SLOT( slotSimpleResult( KIO::Job * ) ) );
    } else {
      /* Stop the timer, we have disconnected. We have to make sure it is
         started again when a new slave appears. */
      mNoopTimer.stop();
    }
  }

  void ImapAccountBase::slotIdleTimeout()
  {
    if ( mSlave ) {
      KIO::Scheduler::disconnectSlave(mSlave);
      mSlave = 0;
      mSlaveConnected = false;
      /* As for the noop timer, we need to make sure this one is started
         again when a new slave goes up. */
      mIdleTimer.stop();
    }
  }

  void ImapAccountBase::slotAbortRequested( KPIM::ProgressItem* item )
  {
    if ( item )
      item->setComplete();
    killAllJobs();
  }


  //-----------------------------------------------------------------------------
  void ImapAccountBase::slotSchedulerSlaveError(KIO::Slave *aSlave, int errorCode,
      const QString &errorMsg)
  {
    if (aSlave != mSlave) return;
    handleError( errorCode, errorMsg, 0, QString::null, true );
    if ( mAskAgain )
      makeConnection();
    else {
      if ( !mSlaveConnected ) {
        mSlaveConnectionError = true;
        mOwner->resetConnectionList( this );
        if ( mSlave )
        {
          KIO::Scheduler::disconnectSlave( slave() );
          mSlave = 0;
        }
      }
      emit connectionResult( errorCode, errorMsg );
    }
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::slotSchedulerSlaveConnected(KIO::Slave *aSlave)
  {
    if (aSlave != mSlave) return;
    mSlaveConnected = true;
    mNoopTimer.start( 60000 ); // make sure we start sending noops
    emit connectionResult( 0, QString::null ); // success

    if ( mNamespaces.isEmpty() || mNamespaceToDelimiter.isEmpty() ) {
      connect( this, SIGNAL( namespacesFetched( const ImapAccountBase::nsDelimMap& ) ),
          this, SLOT( slotSaveNamespaces( const ImapAccountBase::nsDelimMap& ) ) );
      getNamespaces();
    }

    // get capabilities
    QByteArray packedArgs;
    QDataStream stream( packedArgs, IO_WriteOnly);
    stream << (int) 'c';
    KIO::SimpleJob *job = KIO::special( getUrl(), packedArgs, false );
    KIO::Scheduler::assignJobToSlave( mSlave, job );
    connect( job, SIGNAL(infoMessage(KIO::Job*, const QString&)),
	   SLOT(slotCapabilitiesResult(KIO::Job*, const QString&)) );
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::slotCapabilitiesResult( KIO::Job*, const QString& result )
  {
    mCapabilities = QStringList::split(' ', result.lower() );
    kdDebug(5006) << "capabilities:" << mCapabilities << endl;
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::getNamespaces()
  {
    disconnect( this, SIGNAL( connectionResult(int, const QString&) ),
          this, SLOT( getNamespaces() ) );
    if ( makeConnection() != Connected || !mSlave ) {
      kdDebug(5006) << "getNamespaces - wait for connection" << endl;
      if ( mNamespaces.isEmpty() || mNamespaceToDelimiter.isEmpty() ) {
        // when the connection is established slotSchedulerSlaveConnected notifies us
      } else {
        // getNamespaces was called by someone else
        connect( this, SIGNAL( connectionResult(int, const QString&) ),
            this, SLOT( getNamespaces() ) );
      }
      return;
    }
    
    QByteArray packedArgs;
    QDataStream stream( packedArgs, IO_WriteOnly);
    stream << (int) 'n';
    jobData jd;
    jd.total = 1; jd.done = 0; jd.cancellable = true;
    jd.progressItem = ProgressManager::createProgressItem( 
        ProgressManager::getUniqueID(),
        i18n("Retrieving Namespaces"),
        QString::null, true, useSSL() || useTLS() );
    jd.progressItem->setTotalItems( 1 );
    connect ( jd.progressItem,
        SIGNAL( progressItemCanceled( KPIM::ProgressItem* ) ),
        this,
        SLOT( slotAbortRequested( KPIM::ProgressItem* ) ) );
    KIO::SimpleJob *job = KIO::special( getUrl(), packedArgs, false );
    KIO::Scheduler::assignJobToSlave( mSlave, job );
    insertJob( job, jd );
    connect( job, SIGNAL( infoMessage(KIO::Job*, const QString&) ),
        SLOT( slotNamespaceResult(KIO::Job*, const QString&) ) );
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::slotNamespaceResult( KIO::Job* job, const QString& str )
  {
    JobIterator it = findJob( job );
    if ( it == jobsEnd() ) return;

    nsDelimMap map;
    namespaceDelim nsDelim;
    QStringList ns = QStringList::split( ",", str );
    for ( QStringList::Iterator it = ns.begin(); it != ns.end(); ++it ) {
      // split, allow empty parts as we can get empty namespaces
      QStringList parts = QStringList::split( "=", *it, true );
      imapNamespace section = imapNamespace( parts[0].toInt() );
      if ( map.contains( section ) ) {
        nsDelim = map[section];
      } else {
        nsDelim.clear();
      }
      // map namespace to delimiter
      nsDelim[parts[1]] = parts[2];
      map[section] = nsDelim;
    }
    removeJob(it);

    kdDebug(5006) << "namespaces fetched" << endl;
    emit namespacesFetched( map );
  }
    
  //-----------------------------------------------------------------------------
  void ImapAccountBase::slotSaveNamespaces( const ImapAccountBase::nsDelimMap& map )
  {
    kdDebug(5006) << "slotSaveNamespaces " << name() << endl;
    // extract the needed information
    mNamespaces.clear();
    mNamespaceToDelimiter.clear();
    for ( uint i = 0; i < 3; ++i ) {
      imapNamespace section = imapNamespace( i );
      namespaceDelim ns = map[ section ];
      namespaceDelim::ConstIterator it;
      QStringList list;
      for ( it = ns.begin(); it != ns.end(); ++it ) {
        list += it.key();
        mNamespaceToDelimiter[ it.key() ] = it.data();
      }
      if ( !list.isEmpty() ) {
        mNamespaces[section] = list;
      }
    }
    // see if we need to migrate an old prefix
    if ( !mOldPrefix.isEmpty() ) {
      migratePrefix();
    }
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::migratePrefix()
  {
    if ( !mOldPrefix.isEmpty() && mOldPrefix != "/" ) {
      // strip /
      if ( mOldPrefix.startsWith("/") ) {
        mOldPrefix = mOldPrefix.right( mOldPrefix.length()-1 );
      }
      if ( mOldPrefix.endsWith("/") ) {
        mOldPrefix = mOldPrefix.left( mOldPrefix.length()-1 );
      }
      QStringList list = mNamespaces[PersonalNS];
      bool done = false;
      for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( (*it).startsWith( mOldPrefix ) ) {
          // should be ok
          done = true;
          kdDebug(5006) << "migratePrefix - no migration needed" << endl;
          break;
        }
      }
      if ( !done ) {
        QString msg = i18n("KMail has detected a prefix entry in the "
            "configuration of the account \"%1\" which is obsolete with the "
            "support of IMAP namespaces.").arg( name() );
        if ( list.contains( QString::null ) ) {
          // replace empty entry with the old prefix
          list.remove( QString::null );
          list += mOldPrefix;
          mNamespaces[PersonalNS] = list;
          if ( mNamespaceToDelimiter.contains( QString::null ) ) {
            QString delim = mNamespaceToDelimiter[QString::null];
            mNamespaceToDelimiter.remove( QString::null );
            mNamespaceToDelimiter[mOldPrefix] = delim;
          }
          kdDebug(5006) << "migratePrefix - replaced empty with " << mOldPrefix << endl;
          msg += i18n("The configuration was automatically migrated but you should check "
              "your account configuration.");
        } else if ( list.count() == 1 ) {
          // only one entry in the personal namespace so replace it
          QString old = list.first();
          list.clear();
          list += mOldPrefix;
          mNamespaces[PersonalNS] = list;
          if ( mNamespaceToDelimiter.contains( old ) ) {
            QString delim = mNamespaceToDelimiter[old];
            mNamespaceToDelimiter.remove( old );
            mNamespaceToDelimiter[mOldPrefix] = delim;
          }
          kdDebug(5006) << "migratePrefix - replaced single with " << mOldPrefix << endl;
          msg += i18n("The configuration was automatically migrated but you should check "
              "your account configuration.");
        } else {
          kdDebug(5006) << "migratePrefix - migration failed" << endl;
          msg += i18n("It was not possible to migrate your configuration automatically "
              "so please check your account configuration.");
        }
        KMessageBox::information( kmkernel->getKMMainWidget(), msg );
      }
    } else 
    {
      kdDebug(5006) << "migratePrefix - no migration needed" << endl;
    }
    mOldPrefix = QString::null;
  }

  //-----------------------------------------------------------------------------
  QString ImapAccountBase::namespaceForFolder( FolderStorage* storage )
  {
    QString path;
    if ( storage->folderType() == KMFolderTypeImap ) {
      path = static_cast<KMFolderImap*>( storage )->imapPath();
    } else if ( storage->folderType() == KMFolderTypeCachedImap ) {
      path = static_cast<KMFolderCachedImap*>( storage )->imapPath();
    }

    nsMap::Iterator it;
    for ( it = mNamespaces.begin(); it != mNamespaces.end(); ++it )
    {
      QStringList::Iterator strit;
      for ( strit = it.data().begin(); strit != it.data().end(); ++strit )
      {
        // first ignore an empty prefix as it would match always
        if ( !(*strit).isEmpty() && path.find( *strit ) != -1 ) {
          return (*strit);
        }
      }
    }
    return QString::null;
  }  

  //-----------------------------------------------------------------------------
  QString ImapAccountBase::delimiterForNamespace( const QString& prefix )
  {
    // try to match exactly
    if ( mNamespaceToDelimiter.contains(prefix) ) {
      return mNamespaceToDelimiter[prefix];
    }

    // then try if the prefix is part of a namespace
    // exclude empty namespace
    for ( namespaceDelim::ConstIterator it = mNamespaceToDelimiter.begin(); 
          it != mNamespaceToDelimiter.end(); ++it )
    {
      if ( !it.key().isEmpty() && prefix.contains( it.key() ) ) {
        return it.data();
      }
    }
    // see if we have an empty namespace
    if ( mNamespaceToDelimiter.contains( QString::null ) ) {
      return mNamespaceToDelimiter[QString::null];
    }
    // well, we tried
    return QString::null;
  }

  //-----------------------------------------------------------------------------
  QString ImapAccountBase::delimiterForFolder( FolderStorage* storage )
  {
    QString prefix = namespaceForFolder( storage );
    QString delim;
    if ( mNamespaceToDelimiter.contains(prefix) )
      delim = mNamespaceToDelimiter[prefix];
    return delim;
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::slotSimpleResult(KIO::Job * job)
  {
    JobIterator it = findJob( job );
    bool quiet = false;
    if (it != mapJobData.end()) {
      quiet = (*it).quiet;
      if ( !(job->error() && !quiet) ) // the error handler removes in that case
        removeJob(it);
    }
    if (job->error()) {
      if (!quiet)
        handleJobError(job, QString::null );
      else {
        if ( job->error() == KIO::ERR_CONNECTION_BROKEN && slave() ) {
          // make sure ERR_CONNECTION_BROKEN is properly handled and the slave
          // disconnected even when quiet()
          KIO::Scheduler::disconnectSlave( slave() );
          mSlave = 0;
        }
        if (job->error() == KIO::ERR_SLAVE_DIED)
          slaveDied();
      }
    }
  }

  //-----------------------------------------------------------------------------
  bool ImapAccountBase::handlePutError( KIO::Job* job, jobData& jd, KMFolder* folder )
  {
    Q_ASSERT( !jd.msgList.isEmpty() );
    KMMessage* msg = jd.msgList.first();
    // Use double-quotes around the subject to keep the sentence readable,
    // but don't use double quotes around the sender since from() might return a double-quoted name already
    const QString subject = msg->subject().isEmpty() ? i18n( "<unknown>" ) : QString("\"%1\"").arg( msg->subject() );
    const QString from = msg->from().isEmpty() ? i18n( "<unknown>" ) : msg->from();
    QString myError = "<p><b>" + i18n("Error while uploading message")
      + "</b></p><p>"
      + i18n("Could not upload the message dated %1 from %2 with subject %3 on the server.").arg( msg->dateStr(), QStyleSheet::escape( from ), QStyleSheet::escape( subject ) )
      + "</p><p>"
      + i18n("The destination folder was %1, which has the URL %2.").arg( QStyleSheet::escape( folder->label() ), QStyleSheet::escape( jd.htmlURL() ) )
      + "</p><p>"
      + i18n("The error message from the server communication is here:") + "</p>";
    return handleJobError( job, myError );
  }

  //-----------------------------------------------------------------------------
  bool ImapAccountBase::handleError( int errorCode, const QString &errorMsg, KIO::Job* job, const QString& context, bool abortSync )
  {
    // Copy job's data before a possible killAllJobs
    QStringList errors;
    if ( job && job->error() != KIO::ERR_SLAVE_DEFINED /*workaround for kdelibs-3.2*/)
      errors = job->detailedErrorStrings();

    bool jobsKilled = true;
    switch( errorCode ) {
    case KIO::ERR_SLAVE_DIED: slaveDied(); killAllJobs( true ); break;
    case KIO::ERR_COULD_NOT_LOGIN: // bad password
      mAskAgain = true;
      // fallthrough intended
    case KIO::ERR_CONNECTION_BROKEN:
    case KIO::ERR_COULD_NOT_CONNECT:
    case KIO::ERR_SERVER_TIMEOUT:
      // These mean that we'll have to reconnect on the next attempt, so disconnect and set mSlave to 0.
      killAllJobs( true );
      break;
    case KIO::ERR_USER_CANCELED:
      killAllJobs( false );
      break;
    default:
      if ( abortSync )
        killAllJobs( false );
      else
        jobsKilled = false;
      break;
    }

    // check if we still display an error
    if ( !mErrorDialogIsActive && errorCode != KIO::ERR_USER_CANCELED ) {
      mErrorDialogIsActive = true;
      QString msg = context + '\n' + KIO::buildErrorString( errorCode, errorMsg );
      QString caption = i18n("Error");
      
      if ( jobsKilled || errorCode == KIO::ERR_COULD_NOT_LOGIN ) {
        if ( errorCode == KIO::ERR_SERVER_TIMEOUT || errorCode == KIO::ERR_CONNECTION_BROKEN ) {
          msg = i18n("The connection to the server %1 was unexpectedly closed or timed out. It will be re-established automatically if possible.").
            arg( name() );
          KMessageBox::information( kapp->activeWindow(), msg, caption, "kmailConnectionBrokenErrorDialog" );
          // Show it in the status bar, in case the user has ticked "don't show again"
          if ( errorCode == KIO::ERR_CONNECTION_BROKEN )
            KPIM::BroadcastStatus::instance()->setStatusMsg(
                i18n(  "The connection to account %1 was broken." ).arg( name() ) );
          else if ( errorCode == KIO::ERR_SERVER_TIMEOUT )
            KPIM::BroadcastStatus::instance()->setStatusMsg(
                i18n(  "The connection to account %1 timed out." ).arg( name() ) );
        } else {
          if ( !errors.isEmpty() )
              KMessageBox::detailedError( kapp->activeWindow(), msg, errors.join("\n").prepend("<qt>"), caption );
          else
              KMessageBox::error( kapp->activeWindow(), msg, caption );
          }
      }
      else { // i.e. we have a chance to continue, ask the user about it
        if ( errors.count() >= 3 ) { // there is no detailedWarningContinueCancel... (#86517)
          msg = QString( "<qt>") + context + errors[1] + '\n' + errors[2];
          caption = errors[0];
        }
        int ret = KMessageBox::warningContinueCancel( kapp->activeWindow(), msg, caption );
        if ( ret == KMessageBox::Cancel ) {
          jobsKilled = true;
          killAllJobs( false );
        }
      }
      mErrorDialogIsActive = false;
    } else {
      if ( mErrorDialogIsActive )
        kdDebug(5006) << "suppressing error:" << errorMsg << endl;
    }
    if ( job && !jobsKilled )
      removeJob( job );
    return !jobsKilled; // jobsKilled==false -> continue==true
    }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::cancelMailCheck()
  {
    QMap<KIO::Job*, jobData>::Iterator it = mapJobData.begin();
    while ( it != mapJobData.end() ) {
      kdDebug(5006) << "cancelMailCheck: job is cancellable: " << (*it).cancellable << endl;
      if ( (*it).cancellable ) {
        it.key()->kill();
        QMap<KIO::Job*, jobData>::Iterator rmit = it;
        ++it;
        mapJobData.remove( rmit );
        // We killed a job -> this kills the slave
        mSlave = 0;
      } else
        ++it;
    }

    for( QPtrListIterator<FolderJob> it( mJobList ); it.current(); ++it ) {
      if ( it.current()->isCancellable() ) {
        FolderJob* job = it.current();
        job->setPassiveDestructor( true );
        mJobList.remove( job );
        delete job;
      } else
        ++it;
    }
  }


  //-----------------------------------------------------------------------------
  QString ImapAccountBase::jobData::htmlURL() const
  {
    KURL u(  url );
    return u.htmlURL();
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::processNewMailSingleFolder(KMFolder* folder)
  {
    mFoldersQueuedForChecking.append(folder);
    mCheckingSingleFolder = true;
    if ( checkingMail() )
    {
      disconnect( this, SIGNAL( finishedCheck( bool, CheckStatus ) ),
                  this, SLOT( slotCheckQueuedFolders() ) );
      connect( this, SIGNAL( finishedCheck( bool, CheckStatus ) ),
               this, SLOT( slotCheckQueuedFolders() ) );
    } else {
      slotCheckQueuedFolders();
    }
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::slotCheckQueuedFolders()
  {
    disconnect( this, SIGNAL( finishedCheck( bool, CheckStatus ) ),
                this, SLOT( slotCheckQueuedFolders() ) );

    QValueList<QGuardedPtr<KMFolder> > mSaveList = mMailCheckFolders;
    mMailCheckFolders = mFoldersQueuedForChecking;
    kmkernel->acctMgr()->singleCheckMail(this, true);
    mMailCheckFolders = mSaveList;
    mFoldersQueuedForChecking.clear();
  }

  //-----------------------------------------------------------------------------
  bool ImapAccountBase::checkingMail( KMFolder *folder )
  {
    if (checkingMail() && mFoldersQueuedForChecking.contains(folder))
      return true;
    return false;
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::handleBodyStructure( QDataStream & stream, KMMessage * msg,
                                             const AttachmentStrategy *as )
  {
    mBodyPartList.clear();
    mCurrentMsg = msg;
    // first delete old parts as we construct our own
    msg->deleteBodyParts();
    // make the parts and fill the mBodyPartList
    constructParts( stream, 1, 0, 0, msg->asDwMessage() );
    if ( mBodyPartList.count() == 1 ) // we directly set the body later
      msg->deleteBodyParts();

    if ( !as )
    {
      kdWarning(5006) << k_funcinfo << " - found no attachment strategy!" << endl;
      return;
    }

    // see what parts have to loaded according to attachmentstrategy
    BodyVisitor *visitor = BodyVisitorFactory::getVisitor( as );
    visitor->visit( mBodyPartList );
    QPtrList<KMMessagePart> parts = visitor->partsToLoad();
    delete visitor;
    QPtrListIterator<KMMessagePart> it( parts );
    KMMessagePart *part;
    int partsToLoad = 0;
    // check how many parts we have to load
    while ( (part = it.current()) != 0 )
    {
      ++it;
      if ( part->loadPart() )
      {
        ++partsToLoad;
      }
    }
    if ( (mBodyPartList.count() * 0.5) < partsToLoad )
    {
      // more than 50% of the parts have to be loaded anyway so it is faster
      // to load the message completely
      kdDebug(5006) << "Falling back to normal mode" << endl;
      FolderJob *job = msg->parent()->createJob(
          msg, FolderJob::tGetMessage, 0, "TEXT" );
      job->start();
      return;
    }
    it.toFirst();
    while ( (part = it.current()) != 0 )
    {
      ++it;
      kdDebug(5006) << "ImapAccountBase::handleBodyStructure - load " << part->partSpecifier()
        << " (" << part->originalContentTypeStr() << ")" << endl;
      if ( part->loadHeaders() )
      {
        kdDebug(5006) << "load HEADER" << endl;
        FolderJob *job = msg->parent()->createJob(
            msg, FolderJob::tGetMessage, 0, part->partSpecifier()+".MIME" );
        job->start();
      }
      if ( part->loadPart() )
      {
        kdDebug(5006) << "load Part" << endl;
        FolderJob *job = msg->parent()->createJob(
            msg, FolderJob::tGetMessage, 0, part->partSpecifier() );
        job->start();
      }
    }
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::constructParts( QDataStream & stream, int count, KMMessagePart* parentKMPart,
                                        DwBodyPart * parent, const DwMessage * dwmsg )
  {
    int children;
    for (int i = 0; i < count; i++)
    {
      stream >> children;
      KMMessagePart* part = new KMMessagePart( stream );
      part->setParent( parentKMPart );
      mBodyPartList.append( part );
      kdDebug(5006) << "ImapAccountBase::constructParts - created id " << part->partSpecifier()
        << " of type " << part->originalContentTypeStr() << endl;
      DwBodyPart *dwpart = mCurrentMsg->createDWBodyPart( part );

      if ( parent )
      {
        // add to parent body
        parent->Body().AddBodyPart( dwpart );
        dwpart->Parse();
//        kdDebug(5006) << "constructed dwpart " << dwpart << ",dwmsg " << dwmsg << ",parent " << parent
//          << ",dwparts msg " << dwpart->Body().Message() <<",id "<<dwpart->ObjectId() << endl;
      } else if ( part->partSpecifier() != "0" &&
                  !part->partSpecifier().endsWith(".HEADER") )
      {
        // add to message
        dwmsg->Body().AddBodyPart( dwpart );
        dwpart->Parse();
//        kdDebug(5006) << "constructed dwpart " << dwpart << ",dwmsg " << dwmsg << ",parent " << parent
//          << ",dwparts msg " << dwpart->Body().Message() <<",id "<<dwpart->ObjectId() << endl;
      } else
        dwpart = 0;

      if ( !parentKMPart )
        parentKMPart = part;

      if (children > 0)
      {
        DwBodyPart* newparent = dwpart;
        const DwMessage* newmsg = dwmsg;
        if ( part->originalContentTypeStr() == "MESSAGE/RFC822" &&
             dwpart->Body().Message() )
        {
          // set the encapsulated message as new parent message
          newparent = 0;
          newmsg = dwpart->Body().Message();
        }
        KMMessagePart* newParentKMPart = part;
        if ( part->partSpecifier().endsWith(".HEADER") ) // we don't want headers as parent
          newParentKMPart = parentKMPart;

        constructParts( stream, children, newParentKMPart, newparent, newmsg );
      }
    }
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::setImapStatus( KMFolder* folder, const QString& path, const QCString& flags )
  {
     // set the status on the server, the uids are integrated in the path
     kdDebug(5006) << "setImapStatus path=" << path << " to: " << flags << endl;
     KURL url = getUrl();
     url.setPath(path);

     QByteArray packedArgs;
     QDataStream stream( packedArgs, IO_WriteOnly);

     stream << (int) 'S' << url << flags;

     if ( makeConnection() != Connected ) 
       return; // can't happen with dimap

     KIO::SimpleJob *job = KIO::special(url, packedArgs, FALSE);
     KIO::Scheduler::assignJobToSlave(slave(), job);
     ImapAccountBase::jobData jd( url.url(), folder );
     jd.path = path;
     insertJob(job, jd);
     connect(job, SIGNAL(result(KIO::Job *)),
           SLOT(slotSetStatusResult(KIO::Job *)));
  }
  //-----------------------------------------------------------------------------
  void ImapAccountBase::slotSetStatusResult(KIO::Job * job)
  {
     ImapAccountBase::JobIterator it = findJob(job);
     if ( it == jobsEnd() ) return;
     int errorCode = job->error();
     if (errorCode && errorCode != KIO::ERR_CANNOT_OPEN_FOR_WRITING)
     {
       bool cont = handleJobError( job, i18n( "Error while uploading status of messages to server: " ) + '\n' );
       emit imapStatusChanged( (*it).parent, (*it).path, cont );
     }
     else
     {
       emit imapStatusChanged( (*it).parent, (*it).path, true );
       removeJob(it);
     }
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::setFolder(KMFolder* folder, bool addAccount)
  {
    if (folder)
    {
      folder->setSystemLabel(name());
      folder->setId(id());
    }
    NetworkAccount::setFolder(folder, addAccount);
  }

  //-----------------------------------------------------------------------------
  void ImapAccountBase::removeJob( JobIterator& it )
  {
    if( (*it).progressItem ) {
      (*it).progressItem->setComplete();
      (*it).progressItem = 0;
    }
    mapJobData.remove( it );
  }

  //-----------------------------------------------------------------------------
  void KMail::ImapAccountBase::removeJob( KIO::Job* job )
  {
    mapJobData.remove( job );
  }

  //-----------------------------------------------------------------------------
  KPIM::ProgressItem* ImapAccountBase::listDirProgressItem()
  {
    if ( !mListDirProgressItem )
    {
      mListDirProgressItem = ProgressManager::createProgressItem(
          "ListDir" + name(),
          name(),
          i18n("retrieving folders"),
          true,
          useSSL() || useTLS() );
      connect ( mListDirProgressItem,
                SIGNAL( progressItemCanceled( KPIM::ProgressItem* ) ),
                this,
                SLOT( slotAbortRequested( KPIM::ProgressItem* ) ) );
      // Start with a guessed value of the old folder count plus 5%. As long
      // as the list of folders doesn't constantly change, that should be good
      // enough.
      unsigned int count = folderCount();
      mListDirProgressItem->setTotalItems( count + (unsigned int)(count*0.05) );
    }
    return mListDirProgressItem;
  }

  //-----------------------------------------------------------------------------
  unsigned int ImapAccountBase::folderCount() const
  {
    if ( !rootFolder() || !rootFolder()->folder() || !rootFolder()->folder()->child() )
      return 0;
    return kmkernel->imapFolderMgr()->folderCount( rootFolder()->folder()->child() );
  }

  //------------------------------------------------------------------------------
  QString ImapAccountBase::addPathToNamespace( const QString& prefix )
  {
    QString myPrefix = prefix;
    if ( !myPrefix.startsWith( "/" ) ) {
      myPrefix = "/" + myPrefix;
    }
    if ( !myPrefix.endsWith( "/" ) ) {
      myPrefix += "/";
    }

    return myPrefix;
  }

  //------------------------------------------------------------------------------
  bool ImapAccountBase::isNamespaceFolder( QString& name )
  {
    QStringList ns = mNamespaces[OtherUsersNS];
    ns += mNamespaces[SharedNS];
    ns += mNamespaces[PersonalNS];
    QString nameWithDelimiter;
    for ( QStringList::Iterator it = ns.begin(); it != ns.end(); ++it )
    {
      nameWithDelimiter = name + delimiterForNamespace( *it );
      if ( *it == name || *it == nameWithDelimiter )
        return true;
    }
    return false;
  }

  //------------------------------------------------------------------------------
  ImapAccountBase::nsDelimMap ImapAccountBase::namespacesWithDelimiter()
  {
    nsDelimMap map;
    nsMap::ConstIterator it;
    for ( uint i = 0; i < 3; ++i )
    {
      imapNamespace section = imapNamespace( i );
      QStringList namespaces = mNamespaces[section];
      namespaceDelim nsDelim;
      QStringList::Iterator lit;
      for ( lit = namespaces.begin(); lit != namespaces.end(); ++lit )
      {
        nsDelim[*lit] = delimiterForNamespace( *lit );
      }
      map[section] = nsDelim;
    }
    return map;
  }

} // namespace KMail

#include "imapaccountbase.moc"
