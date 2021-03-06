/*
    This file is part of KMail.

    Copyright (c) 2003 Steffen Hansen <steffen@klaralvdalens-datakonsult.se>
    Copyright (c) 2003 - 2004 Bo Thorsen <bo@sonofthor.dk>
    Copyright (c) 2004 Till Adam <adam@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "kmailicalifaceimpl.h"
#include "kmfolder.h"
#include "kmfolderdir.h"
#include "kmgroupware.h"
#include "kmfoldermgr.h"
#include "kmcommands.h"
#include "kmfolderindex.h"
#include "kmmsgdict.h"
#include "kmmsgpart.h"
#include <QByteArray>
#include <QList>
using KMail::AccountManager;
#include "kmfolderimap.h"
#include "globalsettings.h"
#include "accountmanager.h"
#include "kmfoldercachedimap.h"
#include "kmacctcachedimap.h"
#include "acljobs.h"

#include <mimelib/enum.h>
#include <mimelib/utility.h>
#include <mimelib/body.h>
#include <mimelib/mimepp.h>

#include <QFile>
#include <QMap>
#include <QTextCodec>
#include <QDBusMessage>
#include <QDBusConnection>

#include <kdebug.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kurl.h>
#include <ktemporaryfile.h>
#include <kconfiggroup.h>
#include "groupwareadaptor.h"

using namespace KMail;

QMap<QString, QString> *KMailICalIfaceImpl::mSubResourceUINamesMap = new QMap<QString, QString>;

// Local helper methods
static void vPartMicroParser( const QString& str, QString& s );
static void reloadFolderTree();

// The index in this array is the KMail::FolderContentsType enum
typedef struct _FolderContentsDescriptor {
  const char* contentsTypeStr; // the string used in the D-Bus interface
  const char* mimetype;
  KPIM::FolderTreeWidgetItem::FolderType treeItemType;
  const char* annotation;
  const char* translatedName;
} FolderContentsDescriptor;

static const FolderContentsDescriptor s_folderContentsType[] =
{
  {
    "Mail", "application/x-vnd.kolab.mail",
    KPIM::FolderTreeWidgetItem::Other,
    "mail", I18N_NOOP2( "type of folder content", "Mail" )
  },
  {
    "Calendar", "application/x-vnd.kolab.event",
    KPIM::FolderTreeWidgetItem::Calendar,
    "event", I18N_NOOP2( "type of folder content", "Calendar" )
  },
  {
    "Contact", "application/x-vnd.kolab.contact",
    KPIM::FolderTreeWidgetItem::Contacts,
    "contact", I18N_NOOP2( "type of folder content", "Contacts" )
  },
  {
    "Note", "application/x-vnd.kolab.note",
    KPIM::FolderTreeWidgetItem::Notes,
    "note", I18N_NOOP2( "type of folder content", "Notes" )
  },
  {
    "Task", "application/x-vnd.kolab.task",
    KPIM::FolderTreeWidgetItem::Tasks,
    "task", I18N_NOOP2( "type of folder content", "Tasks" )
  },
  {
    "Journal", "application/x-vnd.kolab.journal",
    KPIM::FolderTreeWidgetItem::Journals,
    "journal", I18N_NOOP2( "type of folder content", "Journal" )
  }
};

static QString folderContentsType( KMail::FolderContentsType type )
{
  return s_folderContentsType[type].contentsTypeStr;
}

static QString folderKolabMimeType( KMail::FolderContentsType type )
{
  return s_folderContentsType[type].mimetype;
}

StorageFormat KMailICalIfaceImpl::globalStorageFormat() const {
  return GlobalSettings::self()->theIMAPResourceStorageFormat()
    == GlobalSettings::EnumTheIMAPResourceStorageFormat::XML ? StorageXML : StorageIcalVcard;
}

static KMail::FolderContentsType folderContentsType( const QString& type )
{
  for ( uint i = 0 ; i < sizeof s_folderContentsType / sizeof *s_folderContentsType; ++i )
    if ( type == s_folderContentsType[i].contentsTypeStr )
      return static_cast<KMail::FolderContentsType>( i );
  return KMail::ContentsTypeMail;
}

static QString localizedDefaultFolderName( KMail::FolderContentsType type )
{
  return i18nc( "type of folder content", s_folderContentsType[type].translatedName );
}

const char* KMailICalIfaceImpl::annotationForContentsType( KMail::FolderContentsType type )
{
  return s_folderContentsType[type].annotation;
}

ExtraFolder::ExtraFolder( KMFolder* f )
    : folder( f )
{
    folder->open("kmailicaliface::extrafolder");
}

ExtraFolder::~ExtraFolder()
{
    if ( folder )
        folder->close("kmailicaliface::extrafolder");
}


/*
  This interface has three parts to it - libkcal interface;
  kmail interface; and helper functions.

  The libkcal interface and the kmail interface have the same three
  methods: add, delete and refresh. The only difference is that the
  libkcal interface is used from the IMAP resource in libkcal and
  the kmail interface is used from the groupware object in kmail.
*/

KMailICalIfaceImpl::KMailICalIfaceImpl()
  : mContacts( 0 ), mCalendar( 0 ), mNotes( 0 ), mTasks( 0 ), mJournals( 0 ),
    mFolderLanguage( 0 ), mFolderParentDir( 0 ), mFolderType( KMFolderTypeUnknown ),
    mUseResourceIMAP( false ), mResourceQuiet( false ), mHideFolders( true )
{
  setObjectName( "KMailICalIFaceImpl" );

  // Listen to config changes
  connect( kmkernel, SIGNAL( configChanged() ), this, SLOT( readConfig() ) );
  connect( kmkernel, SIGNAL( folderRemoved( KMFolder* ) ),
           this, SLOT( slotFolderRemoved( KMFolder* ) ) );
}

KMailICalIfaceImpl::~KMailICalIfaceImpl()
{
  QDBusConnection::sessionBus().unregisterService( "org.kde.kmail.groupware" );
  qDeleteAll( mExtraFolders );
  qDeleteAll( mAccumulators );
  mAccumulators.clear();
  mExtraFolders.clear();
}

void KMailICalIfaceImpl::registerWithDBus()
{
  KMail::registerGroupwareTypes();
  QDBusConnection::sessionBus().registerObject( "/Groupware", this, QDBusConnection::ExportAdaptors );
  new GroupwareAdaptor( this );

  // Register a dummy service, so we can use isServiceRegistered() in other applications
  // to find out if the KMail groupware services are available.
  // And no, D-Bus does not provide an easy way to just check for the /Groupware object :(
  QDBusConnection::sessionBus().registerService( "org.kde.kmail.groupware" );
}

/* libkcal part of the interface, called from the resources using this
 * when incidences are added or deleted */

// Helper function to find an attachment of a given mimetype
// Can't use KMMessage::findDwBodyPart since it only works with known mimetypes.
static DwBodyPart* findBodyPartByMimeType( const KMMessage& msg, const char* sType, const char* sSubtype, bool startsWith = false )
{
  // quickly searching for our message part: since Kolab parts are
  // top-level parts we do *not* have to travel into embedded multiparts
  DwBodyPart* part = msg.getFirstDwBodyPart();
  while( part ){
  //    kDebug() << part->Headers().ContentType().TypeStr().c_str()
  //             << part->Headers().ContentType().SubtypeStr().c_str();
    if ( part->hasHeaders() ) {
      DwMediaType& contentType = part->Headers().ContentType();
      if ( startsWith ) {
        if ( contentType.TypeStr() == sType
             && QString( contentType.SubtypeStr().c_str() ).startsWith( sSubtype ) )
          return part;
      }
      else
        if ( contentType.TypeStr() == sType
             && contentType.SubtypeStr() == sSubtype )
          return part;
    }
    part = part->Next();
  }
  return 0;
}

// Helper function to find an attachment with a given filename
static DwBodyPart* findBodyPart( const KMMessage& msg, const QString& attachmentName )
{
  // quickly searching for our message part: since Kolab parts are
  // top-level parts we do *not* have to travel into embedded multiparts
  for ( DwBodyPart* part = msg.getFirstDwBodyPart(); part; part = part->Next() ) {
    //kDebug() << "findBodyPart:  -" << part->Headers().ContentDisposition().Filename().c_str();
    if ( part->hasHeaders()
         && attachmentName == part->Headers().ContentDisposition().Filename().c_str() )
      return part;
    if ( part->hasHeaders() && attachmentName == part->Headers().ContentType().Name().c_str() )
      return part;
  }
  return 0;
}

#if 0
static void debugBodyParts( const char* foo, const KMMessage& msg )
{
  kDebug() << "--debugBodyParts" << foo <<"--";
  for ( DwBodyPart* part = msg.getFirstDwBodyPart(); part; part = part->Next() ) {
    if ( part->hasHeaders() ) {
      kDebug() << " bodypart:" << part;
      kDebug() << part->Headers().AsString().c_str();
    }
    else
      kDebug() << " part" << part << "has no headers";
  }
}
#else
inline static void debugBodyParts( const char*, const KMMessage& ) {}
#endif


// Add (or overwrite, resp.) an attachment in an existing mail,
// attachments must be local files, they are identified by their names.
// If lookupByName if false the attachment to replace is looked up by mimetype.
// return value: wrong if attachment could not be added/updated
bool KMailICalIfaceImpl::updateAttachment( KMMessage& msg,
                                           const QString& attachmentURL,
                                           const QString& attachmentName,
                                           const QString& attachmentMimetype,
                                           bool lookupByName )
{
  kDebug() << attachmentURL;

  bool bOK = false;

  KUrl url( attachmentURL );
  if ( url.isValid() && url.isLocalFile() ) {
    const QString fileName( url.toLocalFile() );
    QFile file( fileName );
    if( file.open( QIODevice::ReadOnly ) ) {
      QByteArray rawData = file.readAll();
      file.close();

      // create the new message part with data read from temp file
      KMMessagePart msgPart;
      msgPart.setName( attachmentName );

      const int iSlash = attachmentMimetype.indexOf('/');
      const QByteArray sType    = attachmentMimetype.left( iSlash   ).toLatin1();
      const QByteArray sSubtype = attachmentMimetype.mid(  iSlash+1 ).toLatin1();
      msgPart.setTypeStr( sType );
      msgPart.setSubtypeStr( sSubtype );
      QByteArray ctd("attachment;\n  filename=\"");
      ctd.append( attachmentName.toLatin1() );
      ctd.append("\"");
      msgPart.setContentDisposition( ctd );
      QList<int> dummy;
      msgPart.setBodyAndGuessCte( rawData, dummy );
      msgPart.setPartSpecifier( fileName );

      DwBodyPart* newPart = msg.createDWBodyPart( &msgPart );
      // This whole method is a bit special. We mix code for writing and code for reading.
      // E.g. we need to parse the content-disposition again for ContentDisposition().Filename()
      // to work later on.
      newPart->Headers().ContentDisposition().Parse();

      DwBodyPart* part = lookupByName ? findBodyPart( msg, attachmentName )
                         : findBodyPartByMimeType( msg, sType, sSubtype );
      if ( part ) {
        // Make sure the replacing body part is pointing
        // to the same next part as the original body part.
        newPart->SetNext( part->Next() );
        // call DwBodyPart::operator =
        // which calls DwEntity::operator =
        *part = *newPart;
        delete newPart;
        msg.setNeedsAssembly();
        kDebug() << "Attachment" << attachmentName << "updated.";
      } else {
        msg.addDwBodyPart( newPart );
        kDebug() << "Attachment" << attachmentName << "added.";
      }
      bOK = true;
    }else{
      kDebug() << "Attachment" << attachmentURL << "can not be read.";
    }
  }else{
    kDebug() << "Attachment" << attachmentURL << "not a local file.";
  }

  return bOK;
}

// Look for the attachment with the right mimetype
bool KMailICalIfaceImpl::kolabXMLFoundAndDecoded( const KMMessage& msg, const QString& mimetype, QString& s )
{
  const int iSlash = mimetype.indexOf('/');
  const QByteArray sType    = mimetype.left( iSlash   ).toLatin1();
  const QByteArray sSubtype = mimetype.mid(  iSlash+1 ).toLatin1();
  DwBodyPart* part = findBodyPartByMimeType( msg, sType, sSubtype, true /* starts with sSubtype, to accept application/x-vnd.kolab.contact.distlist */ );
  if ( part ) {
    KMMessagePart msgPart;
    KMMessage::bodyPart(part, &msgPart);
    s = msgPart.bodyToUnicode( QTextCodec::codecForName( "utf8" ) );
    return true;
  }
  return false;
}

// Delete an attachment in an existing mail.
// return value: wrong if attachment could not be deleted
//
// This code could be optimized: for now we just replace
// the attachment by an empty dummy attachment since Mimelib
// does not provide an option for deleting attachments yet.
bool KMailICalIfaceImpl::deleteAttachment( KMMessage& msg,
                                           const QString& attachmentName )
{
  kDebug() << attachmentName;

  bool bOK = false;

  // quickly searching for our message part: since Kolab parts are
  // top-level parts we do *not* have to travel into embedded multiparts
  DwBodyPart* part = findBodyPart( msg, attachmentName );
  if ( part ) {
    msg.getTopLevelPart()->Body().RemoveBodyPart( part );
    delete part;
    msg.setNeedsAssembly();
    kDebug() << "Attachment deleted.";
    bOK = true;
  }

  if( !bOK ){
    kDebug() << "Attachment" << attachmentName << "not found.";
  }

  return bOK;
}

static void setIcalVcardContentTypeHeader( KMMessage *msg, KMail::FolderContentsType t, KMFolder *folder )
{
  KMAcctCachedImap::GroupwareType groupwareType = KMAcctCachedImap::GroupwareKolab;

  KMFolderCachedImap *imapFolder = dynamic_cast<KMFolderCachedImap*>( folder->storage() );
  if ( imapFolder )
    groupwareType = imapFolder->account()->groupwareType();

  msg->setType( DwMime::kTypeText );
  if ( t == KMail::ContentsTypeCalendar || t == KMail::ContentsTypeTask
      || t == KMail::ContentsTypeJournal ) {
    msg->setSubtype( DwMime::kSubtypeVCal );

    if ( groupwareType == KMAcctCachedImap::GroupwareKolab )
      msg->setHeaderField("Content-Type",
          "text/calendar; method=REQUEST; charset=\"utf-8\"");
  } else if ( t == KMail::ContentsTypeContact ) {
    msg->setSubtype( DwMime::kSubtypeXVCard );
    if ( groupwareType == KMAcctCachedImap::GroupwareKolab )
      msg->setHeaderField( "Content-Type", "Text/X-VCard; charset=\"utf-8\"" );
  } else {
    kWarning() <<"Attempt to write non-groupware contents to folder";
  }
}

static void setXMLContentTypeHeader( KMMessage *msg, const QString &plainTextBody )
{
   // add a first body part to be displayed by all mailer
    // than can NOT display Kolab data: no matter if these
    // mailers are MIME compliant or not
    KMMessagePart firstPart;
    firstPart.setType( DwMime::kTypeText );
    firstPart.setSubtype( DwMime::kSubtypePlain );
    msg->removeHeaderField( "Content-Type" );
    msg->setType( DwMime::kTypeMultipart );
    msg->setSubtype( DwMime::kSubtypeMixed );
    msg->headers().ContentType().CreateBoundary( 0 );
    msg->headers().ContentType().Assemble();
    firstPart.setBodyFromUnicode( plainTextBody );
    msg->addBodyPart( &firstPart );
}

// Store a new entry that was received from the resource
quint32 KMailICalIfaceImpl::addIncidenceKolab( KMFolder& folder,
                                                const QString& subject,
                                                const QString& plainTextBody,
                                                const KMail::CustomHeader::List& customHeaders,
                                                const QStringList& attachmentURLs,
                                                const QStringList& attachmentNames,
                                                const QStringList& attachmentMimetypes )
{
  kDebug() << attachmentNames;

  quint32 sernum = 0;
  bool bAttachOK = true;

  // Make a new message for the incidence
  KMMessage* msg = new KMMessage();
  msg->initHeader();
  msg->setSubject( subject );
  msg->setAutomaticFields( true );

  foreach ( const CustomHeader& header, customHeaders )
    msg->setHeaderField( header.name, header.value );
  // In case of the ical format, simply add the plain text content with the
  // right content type
  if ( storageFormat( &folder ) == StorageXML ) {
    setXMLContentTypeHeader( msg, plainTextBody );
  } else if ( storageFormat( &folder ) == StorageIcalVcard ) {
    const KMail::FolderContentsType t = folder.storage()->contentsType();
    setIcalVcardContentTypeHeader( msg, t, &folder );
    msg->setBodyEncoded( plainTextBody.toUtf8() );
  } else {
    kWarning() << "Attempt to write to folder with unknown storage type";
  }

  Q_ASSERT( attachmentMimetypes.count() == attachmentURLs.count() );
  Q_ASSERT( attachmentNames.count() == attachmentURLs.count() );
  // Add all attachments by reading them from their temp. files
  QStringList::ConstIterator itmime = attachmentMimetypes.constBegin();
  QStringList::ConstIterator iturl = attachmentURLs.constBegin();
  for( QStringList::ConstIterator itname = attachmentNames.constBegin();
       itname != attachmentNames.constEnd()
       && itmime != attachmentMimetypes.constEnd()
       && iturl != attachmentURLs.constEnd();
       ++itname, ++iturl, ++itmime ){
    bool byname = !(*itmime).startsWith( QLatin1String("application/x-vnd.kolab.") );
    if( !updateAttachment( *msg, *iturl, *itname, *itmime, byname ) ){
      kWarning() << "Attachment error, can not add Incidence.";
      bAttachOK = false;
      break;
    }
  }

  if( bAttachOK ){
    // Mark the message as read and store it in the folder
    msg->cleanupHeader();
    //debugBodyParts( "after cleanup", *msg );
    msg->touch();
    kDebug( KMKernel::storageDebug() ) << "***<msg>***" << msg->asString() << "***</msg>***";
    if ( folder.addMsg( msg ) == 0 )
      // Message stored
      sernum = msg->getMsgSerNum();
    kDebug() << "Message done and saved. Sernum:" << sernum;

    //debugBodyParts( "after addMsg", *msg );
    addFolderChange( &folder,ContentsChanged );
    syncFolder( &folder );
  } else
    kError() << "Message *NOT* saved!";

  return sernum;
}

bool KMailICalIfaceImpl::deleteIncidenceKolab( const QString& resource,
                                               quint32 sernum )
{
  // Find the message from the serial number and delete it.
  if( !mUseResourceIMAP )
    return false;

  kDebug() << resource << ", " << sernum;

  // Find the folder
  KMFolder* f = findResourceFolder( resource );
  if( !f ) {
    kError() << resource << ": Not an IMAP resource folder";
    return false;
  }

  bool rc = false;

  KMMessage* msg = findMessageBySerNum( sernum, f );
  if( msg ) {
    // Message found - delete it and return happy
    deleteMsg( msg );
    syncFolder( f );
    rc = true;
  } else {
    kDebug() << "Message not found, cannot remove serNum" << sernum;
  }
  return rc;
}


int KMailICalIfaceImpl::incidencesKolabCount( const QString& mimetype,
                                              const QString& resource )
{
  Q_UNUSED( mimetype ); // honoring that would be too slow...

  if( !mUseResourceIMAP )
    return 0;

  KMFolder* f = findResourceFolder( resource );
  if( !f ) {
    kError() <<"incidencesKolab(" << resource <<") : Not an IMAP resource folder";
    return 0;
  }

  f->open( "kolabcount" );
  int n = f->count();
  f->close( "kolabcount" );
  kDebug() << resource << " returned" << n;
  return n;
}

KMail::SernumDataPair::List KMailICalIfaceImpl::incidencesKolab( const QString& mimetype,
                                                                 const QString& resource,
                                                                 int startIndex,
                                                                 int nbMessages )
{
  /// Get the mimetype attachments from this folder. Returns a
  /// QMap with serialNumber/attachment pairs.
  /// (serial numbers of the mail are provided for easier later update)

  KMail::SernumDataPair::List aMap;
  if( !mUseResourceIMAP )
    return aMap;

  KMFolder* f = findResourceFolder( resource );
  if( !f ) {
    kError() <<"incidencesKolab(" << resource <<") : Not an IMAP resource folder";
    return aMap;
  }

  f->open( "incidences" );

  int stopIndex = nbMessages == -1 ? f->count() :
                  qMin( f->count(), startIndex + nbMessages );
  kDebug() <<  mimetype << "," << resource << " from" << startIndex << "to" << stopIndex;

  for(int i = startIndex; i < stopIndex; ++i) {
#if 0
    bool unget = !f->isMessage(i);
    KMMessage* msg = f->getMsg( i );
#else // faster
    KMMessage* msg = f->storage()->readTemporaryMsg(i);
#endif
    if ( msg ) {
      const int iSlash = mimetype.indexOf('/');
      const QByteArray sType    = mimetype.left( iSlash   ).toLatin1();
      const QByteArray sSubtype = mimetype.mid(  iSlash+1 ).toLatin1();
      if ( sType.isEmpty() || sSubtype.isEmpty() ) {
        kError() << mimetype << "not an type/subtype combination";
      } else {
        DwBodyPart* dwPart = findBodyPartByMimeType( *msg, sType, sSubtype );
        if ( dwPart ) {
          KMMessagePart msgPart;
          KMMessage::bodyPart(dwPart, &msgPart);
          aMap << SernumDataPair(msg->getMsgSerNum(), msgPart.bodyToUnicode( QTextCodec::codecForName( "utf8" ) ));
        } else {
          // Check if the whole message has the right types. This is what
          // happens in the case of ical storage, where the whole mail is
          // the data
          const QByteArray type( msg->typeStr() );
          const QByteArray subtype( msg->subtypeStr() );
          if (type.toLower() == sType && subtype.toLower() == sSubtype ) {
            aMap << SernumDataPair( msg->getMsgSerNum(), msg->bodyToUnicode() );
          }
          // This is *not* an error: it may be that not all of the messages
          // have a message part that is matching the wanted MIME type
        }
      }
#if 0
      if( unget ) f->unGetMsg(i);
#else
      if ( msg->transferInProgress() )
        msg->deleteWhenUnused();
      else
        delete msg;
#endif
    }
  }
  f->close( "incidences" );
  return aMap;
}


/* Called when a message that was downloaded from an online imap folder
 * arrives. Needed when listing incidences on online account folders. */
// TODO: Till, port me
void KMailICalIfaceImpl::slotMessageRetrieved( KMMessage* msg )
{
  if( !msg ) return;

  KMFolder *parent = msg->parent();
  Q_ASSERT( parent );
  quint32 sernum = msg->getMsgSerNum();

  // do we have an accumulator for this folder?
  // TODO (till): Remove this accumulator stuff, it is dead code.
  Accumulator *ac = mAccumulators.value( parent->location(), 0 );
  if( ac ) {
    QString s;
    if ( !vPartFoundAndDecoded( msg, s ) ) return;
    QString uid( "UID" );
    vPartMicroParser( s, uid );
    const quint32 sernum = msg->getMsgSerNum();
    mUIDToSerNum.insert( uid, sernum );
    ac->add( s );
    if( ac->isFull() ) {
      /* if this was the last one we were waiting for, tell the resource
       * about the new incidences and clean up. */
      //asyncLoadResult( ac->incidences, ac->type, ac->folder );
      delete mAccumulators.take( ac->folder );
    }
  } else {
    /* We are not accumulating for this folder, so this one was added
     * by KMail. Do your thang. */
     slotIncidenceAdded( msg->parent(), msg->getMsgSerNum() );
  }

  if ( mTheUnGetMes.contains( sernum ) ) {
    mTheUnGetMes.remove( sernum );
    int i = 0;
    KMFolder* folder = 0;
    KMMsgDict::instance()->getLocation( sernum, &folder, &i );
    folder->unGetMsg( i );
  }
}

static int dimapAccountCount()
{
  KMail::AccountManager *mgr = kmkernel->acctMgr();
  QList<KMAccount*>::iterator accountIt = mgr->begin();
  int count = 0;
  while ( accountIt != mgr->end() ) {
    KMAccount *account = *accountIt;
    ++accountIt;
    if ( dynamic_cast<KMAcctCachedImap*>( account ) )
      ++count;
  }
  return count;
}

static QString subresourceLabelForPresentation( const KMFolder * folder )
{
  if( KMailICalIfaceImpl::getResourceMap()->contains( folder->location() ) ) {
    return folder->label();
  }

  QString label = folder->prettyUrl();
  QStringList parts = label.split( QChar('/') );
  // In the common special case of some other user's folder shared with us
  // the url looks like "Server Name/user/$USERNAME/Folder/Name". Make
  // those a bit nicer.
  if ( parts[1] == QLatin1String("user") ) {
    QStringList remainder(parts);
    remainder.pop_front();
    remainder.pop_front();
    remainder.pop_front();
    label = i18nc( "calendar|tasks|contacts=%2 belonging to %1",
                   "%1's %2", parts[2], remainder.join( QLatin1String("/") ) );
  }
  // Another special case is our own folders, under the imap INBOX, make
  // those prettier too
  const KMFolder *parent = folder;
  while ( parent->parent() && parent->parent()->owner() ) {
    parent = parent->parent()->owner();
    if ( parent->isSystemFolder() ) {
      QStringList remainder(parts);
      remainder.pop_front();
      remainder.pop_front();
      if ( dimapAccountCount() > 1 && folder && folder->storage() &&
           static_cast<const KMFolderCachedImap*>( folder->storage() )->account() ) {
        label = i18nc( "folder name (folder storage name)", "%1 (%2)",
                       remainder.join( QString::fromLatin1("/") ),
                       static_cast<const KMFolderCachedImap*>( folder->storage() )->account()->name() );
      } else {
        label = remainder.join( QString::fromLatin1("/") );
      }

      if ( folder && folder->storage() ) {
        // Although the strings here are all the same, they may be different in other languages
        switch ( folder->storage()->contentsType() ) {
          case ContentsTypeCalendar:
            label = i18nc( "My Calendar", "My %1", label );
            break;
          case ContentsTypeContact:
            label = i18nc( "My Contacts", "My %1", label );
            break;
          case ContentsTypeJournal:
            label = i18nc( "My Journal", "My %1", label );
            break;
          case ContentsTypeNote:
            label = i18nc( "My Notes", "My %1", label );
            break;
          case ContentsTypeTask:
            label = i18nc( "My Tasks", "My %1", label );
            break;
          default:
            kWarning() << "Unexpected subresource type for folder" << folder->location();
            break;
        }
      }
    }
  }
  return label;
}

/* list all available subresources */
QList<SubResource> KMailICalIfaceImpl::subresourcesKolab( const QString& contentsType )
{
  QList<SubResource> subResources;

  // Add the default one
  KMFolder* f = folderFromType( contentsType, QString() );
  if ( f ) {
    subResources.append( SubResource( f->location(), subresourceLabelForPresentation( f ),
                                      f->isWritable(), folderIsAlarmRelevant( f ) ) );
    kDebug() << "Adding(1) folder" << f->location()
             <<  ( !f->isWritable() ? "readonly" : "" );
  }

  // get the extra ones
  const KMail::FolderContentsType t = folderContentsType( contentsType );
  QHash<QString, KMail::ExtraFolder*>::iterator it = mExtraFolders.begin();
  for ( ; it != mExtraFolders.end(); ++it) {
    f = it.value()->folder;
    if ( f && f->storage()->contentsType() == t ) {
      subResources.append( SubResource( f->location(), subresourceLabelForPresentation( f ),
                                        f->isWritable(), folderIsAlarmRelevant( f ) ) );
      kDebug() << "Adding(2) folder" << f->location()
               << ( !f->isWritable() ? "readonly" : "" );
    }
  }

  if ( subResources.isEmpty() ) {
    kDebug() << "No folder found for" << contentsType;
  }
  return subResources;
}

bool KMailICalIfaceImpl::triggerSync( const QString& contentsType )
{
  kDebug() ;
  QList<SubResource> folderList = subresourcesKolab( contentsType );
  for ( QList<SubResource>::const_iterator it( folderList.constBegin() ),
                                                                    end( folderList.constEnd() );
        it != end ; ++it ) {
    KMFolder * const f = findResourceFolder( (*it).location );
    if ( !f ) continue;
    if ( f->folderType() == KMFolderTypeImap || f->folderType() == KMFolderTypeCachedImap ) {
      if ( !kmkernel->askToGoOnline() ) {
        return false;
      }
    }

    if ( f->folderType() == KMFolderTypeImap ) {
      KMFolderImap *imap = static_cast<KMFolderImap*>( f->storage() );
      imap->getAndCheckFolder();
    } else if ( f->folderType() == KMFolderTypeCachedImap ) {
      KMFolderCachedImap* cached = static_cast<KMFolderCachedImap*>( f->storage() );
      cached->account()->processNewMailInFolder( f );
    }
  }
  return true;
}

/* Used by the resource to query whether folders are writable. */
bool KMailICalIfaceImpl::isWritableFolder( const QString& type,
                                           const QString& resource )
{
  KMFolder* f = folderFromType( type, resource );
  if ( !f )
    // Definitely not writable
    return false;

  return f->isWritable();
}

/* Used by the resource to query the storage format of the folder. */
StorageFormat KMailICalIfaceImpl::storageFormat( const QString& resource )
{
  StorageFormat format;
  KMFolder* f = findResourceFolder( resource );
  if ( f )
    format = storageFormat( f );
  else
    format = globalStorageFormat();
  return format;
}

/**
  // This finds the message with serial number "sernum", sets the
  // xml attachments to hold the contents of "xml", and updates all
  // attachments.
  // The mail can have additional attachments, and these are not
  // touched!  They belong to other clients - like Outlook
  // So we delete all the attachments listed in the
  // "deletedAttachments" arg, and then update/add all the attachments
  // given by the urllist attachments.

  // If the mail does not already exist, id will not be a valid serial
  // number, and the mail is just added instead. In this case
  // the deletedAttachments can be forgotten.
*/
quint32 KMailICalIfaceImpl::update( const QString& resource,
                                     quint32 sernum,
                                     const QString& subject,
                                     const QString& plainTextBody,
                                     const KMail::CustomHeader::List& customHeaders,
                                     const QStringList& attachmentURLs,
                                     const QStringList& attachmentMimetypes,
                                     const QStringList& attachmentNames,
                                     const QStringList& deletedAttachments )
{
  quint32 rc = 0;

   if( !mUseResourceIMAP )
    return rc;

  Q_ASSERT( !resource.isEmpty() );

  kDebug() << resource << "," << sernum;
  kDebug() << attachmentURLs;
  kDebug() << attachmentMimetypes;
  kDebug() << attachmentNames;
  kDebug() << "deleted attachments:" << deletedAttachments;

  // Find the folder
  KMFolder* f = findResourceFolder( resource );
  if( !f ) {
    kError() << "update(" << resource <<") : Not an IMAP resource folder";
    return rc;
  }

  f->open( "ifaceupdate" );

  KMMessage* msg = 0;
  if ( sernum != 0 ) {
    msg = findMessageBySerNum( sernum, f );
    if ( !msg ) return 0;
    // Message found - make a copy and update it:
    KMMessage* newMsg = new KMMessage( *msg );
    newMsg->setSubject( subject );
    foreach ( const CustomHeader& header, customHeaders )
      newMsg->setHeaderField( header.name, header.value );
    newMsg->setParent( 0 ); // workaround strange line in KMMsgBase::assign. newMsg is not in any folder yet.
    // Note that plainTextBody isn't used in this branch. We assume it's still valid from when the mail was created.

    // Delete some attachments according to list
    for( QStringList::ConstIterator it = deletedAttachments.constBegin();
         it != deletedAttachments.constEnd();
         ++it ){
      if( !deleteAttachment( *newMsg, *it ) ){
        // Note: It is _not_ an error if an attachment was already deleted.
      }
    }

    const KMail::FolderContentsType t = f->storage()->contentsType();
    const QByteArray type = msg->typeStr();
    const QByteArray subtype = msg->subtypeStr();
    const bool messageWasIcalVcardFormat = ( type.toLower() == "text" &&
        ( subtype.toLower() == "calendar" || subtype.toLower() == "x-vcard" ) );

    if ( storageFormat( f ) == StorageIcalVcard ) {
      //kDebug() << " StorageFormatIcalVcard";
      if ( !messageWasIcalVcardFormat ) {
        setIcalVcardContentTypeHeader( newMsg, t, f );
      }
      newMsg->setBodyEncoded( plainTextBody.toUtf8() );
    } else if ( storageFormat( f ) == StorageXML ) {
      if ( messageWasIcalVcardFormat ) {
        // this was originally an ical event, but the folder changed to xml,
        // convert
       setXMLContentTypeHeader( newMsg, plainTextBody );
      }
      //kDebug() << " StorageFormatXML";
      // Add all attachments by reading them from their temp. files
      QStringList::ConstIterator iturl = attachmentURLs.constBegin();
      QStringList::ConstIterator itmime = attachmentMimetypes.constBegin();
      QStringList::ConstIterator itname = attachmentNames.constBegin();
      for( ;
          iturl != attachmentURLs.constEnd()
          && itmime != attachmentMimetypes.constEnd()
          && itname != attachmentNames.constEnd();
          ++iturl, ++itname, ++itmime ){
        bool byname = !(*itmime).startsWith( QLatin1String("application/x-vnd.kolab.") );
        if( !updateAttachment( *newMsg, *iturl, *itname, *itmime, byname ) ){
          kDebug() << "Attachment error, can not update attachment" << *iturl;
          break;
        }
      }
    }

    //debugBodyParts( "in update, before cleanup", *newMsg );

    // This is necessary for the headers to be readable later on
    newMsg->cleanupHeader();

    //debugBodyParts( "in update, after cleanup", *newMsg );

    deleteMsg( msg );
    if ( f->addMsg( newMsg ) == 0 ) {
      // Message stored
      rc = newMsg->getMsgSerNum();
      kDebug() << "forget about" << sernum <<", it's" << rc << "now";
    }
    addFolderChange( f, ContentsChanged );
    syncFolder( f );
  } else {
    // Message not found - store it newly
    rc = addIncidenceKolab( *f, subject, plainTextBody, customHeaders,
                            attachmentURLs,
                            attachmentNames,
                            attachmentMimetypes );
  }

  f->close( "ifaceupdate" );
  return rc;
}

QString KMailICalIfaceImpl::getAttachment( const QString& resource,
                                        quint32 sernum,
                                        const QString& filename )
{
  // This finds the attachment with the filename, saves it to a
  // temp file and returns a URL to it. It's up to the resource
  // to delete the tmp file later.
  if( !mUseResourceIMAP )
    return QString();

  kDebug() << resource << ", " << sernum << ", " << filename;

  // Find the folder
  KMFolder* f = findResourceFolder( resource );
  if( !f ) {
    kError() << resource << ": Not an IMAP resource folder";
    return QString();
  }
  if ( storageFormat( f ) != StorageXML ) {
    kError() << resource << ": Folder has wrong storage format" << storageFormat( f );
    return QString();
  }

  KUrl url;

  bool bOK = false;
  bool quiet = mResourceQuiet;
  mResourceQuiet = true;

  KMMessage* msg = findMessageBySerNum( sernum, f );
  if( msg ) {
    // Message found - look for the attachment:

    DwBodyPart* part = findBodyPart( *msg, filename );
    if ( part ) {
      // Save the contents of the attachment.
      KMMessagePart aPart;
      msg->bodyPart( part, &aPart );
      QByteArray rawData( aPart.bodyDecodedBinary() );

      KTemporaryFile file;
      file.setAutoRemove(false);
      file.open( );
      file.write( rawData.data(), rawData.size() );

      url.setPath( file.fileName() );

      bOK = true;
    }

    if( !bOK ){
      kDebug() << "Attachment" << filename << " not found.";
    }
  }else{
    kDebug() << "Message not found.";
  }

  mResourceQuiet = quiet;
  return url.url();
}

QString KMailICalIfaceImpl::attachmentMimetype( const QString & resource,
                                                quint32 sernum,
                                                const QString & filename )
{
  if( !mUseResourceIMAP )
    return QString();
  KMFolder* f = findResourceFolder( resource );
  if( !f || storageFormat( f ) != StorageXML ) {
    kError() << "attachmentMimetype(" << resource << ") : Wrong folder";
    return QString();
  }

  KMMessage* msg = findMessageBySerNum( sernum, f );
  if( msg ) {
    // Message found - look for the attachment:
    DwBodyPart* part = findBodyPart( *msg, filename );
    if ( part ) {
      KMMessagePart kmPart;
      msg->bodyPart( part, &kmPart );
      return QString( kmPart.typeStr() ) + '/' + QString( kmPart.subtypeStr() );
    } else {
      kDebug() << "Attachment " << filename << " not found.";
    }
  } else {
    kDebug() << "Message not found.";
  }

  return QString();
}

QStringList KMailICalIfaceImpl::listAttachments(const QString & resource, quint32 sernum)
{
  QStringList rv;
  if( !mUseResourceIMAP )
    return rv;

  // Find the folder
  KMFolder* f = findResourceFolder( resource );
  if( !f ) {
    kError() <<"listAttachments(" << resource <<") : Not an IMAP resource folder";
    return rv;
  }
  if ( storageFormat( f ) != StorageXML ) {
    kError() <<"listAttachment(" << resource <<") : Folder has wrong storage format" << storageFormat( f );
    return rv;
  }

  KMMessage* msg = findMessageBySerNum( sernum, f );
  if( msg ) {
    for ( DwBodyPart* part = msg->getFirstDwBodyPart(); part; part = part->Next() ) {
      if ( part->hasHeaders() ) {
        QString name;
        DwMediaType& contentType = part->Headers().ContentType();
        if ( QString( contentType.SubtypeStr().c_str() ).startsWith( "x-vnd.kolab." )
           || QString( contentType.SubtypeStr().c_str() ).contains( "tnef" ) )
          continue;
        if ( !part->Headers().ContentDisposition().Filename().empty() )
          name = part->Headers().ContentDisposition().Filename().c_str();
        else if ( !contentType.Name().empty() )
          name = contentType.Name().c_str();
        if ( !name.isEmpty() )
          rv.append( name );
      }
    }
  } else {
    kDebug() << "Message not found.";
  }

  return rv;
}


// ============================================================================

/* KMail part of the interface. These slots are connected to the resource
 * folders and inform us of folders or incidences in them changing, being
 * added or going away. */

void KMailICalIfaceImpl::slotFolderRemoved( KMFolder* folder )
{
  // pretend the folder just changed back to the mail type, which
  // does the right thing, namely remove resource
  folderContentsTypeChanged( folder, KMail::ContentsTypeMail );
  KConfigGroup configGroup( kmkernel->config(), "GroupwareFolderInfo" );
  configGroup.deleteEntry( folder->idString() + "-storageFormat" );
  configGroup.deleteEntry( folder->idString() + "-changes" );
}

// KMail added a file to one of the groupware folders
void KMailICalIfaceImpl::slotIncidenceAdded( KMFolder* folder,
                                             quint32 sernum )
{
  if( mResourceQuiet || !mUseResourceIMAP )
    return;

//  kDebug();
  QString type = folderContentsType( folder->storage()->contentsType() );
  if( type.isEmpty() ) {
    kError() <<"Not an IMAP resource folder";
    return;
  }
  // Get the index of the mail
  int i = 0;
  KMFolder* aFolder = 0;
  KMMsgDict::instance()->getLocation( sernum, &aFolder, &i );
  assert( folder == aFolder );

  bool unget = !folder->isMessage( i );
  QString s;
  QString uid( "UID" );
  KMMessage *msg = folder->getMsg( i );
  if( !msg ) return;
  if( msg->isComplete() ) {

    bool ok = false;
    StorageFormat format = storageFormat( folder );
    switch( format ) {
      case StorageIcalVcard:
        // Read the iCal or vCard
        ok = vPartFoundAndDecoded( msg, s );
        if ( ok )
          vPartMicroParser( s, uid );
        break;
      case StorageXML:
        // Read the XML from the attachment with the given mimetype
        if ( kolabXMLFoundAndDecoded( *msg,
              folderKolabMimeType( folder->storage()->contentsType() ), s ) ) {
          uid = msg->subject();
          ok = true;
        }
        break;
    }
    if ( !ok ) {
      if ( unget )
        folder->unGetMsg( i );
      return;
    }
    const quint32 sernum = msg->getMsgSerNum();
    mUIDToSerNum.insert( uid, sernum );

    // tell the resource if we didn't trigger this ourselves
    if ( mInTransit.contains( uid ) ) {
      mInTransit.remove( uid );
    }
    emit incidenceAdded( type, folder->location(), sernum, format, s );

  } else {
    // go get the rest of it, then try again
    // TODO: Till, port me
    if ( unget ) mTheUnGetMes.insert( msg->getMsgSerNum(), true );
    FolderJob *job = msg->parent()->createJob( msg );
    connect( job, SIGNAL( messageRetrieved( KMMessage* ) ),
        this, SLOT( slotMessageRetrieved( KMMessage* ) ) );
    job->start();
    return;
  }
  if( unget ) folder->unGetMsg(i);
}

// KMail deleted a file
void KMailICalIfaceImpl::slotIncidenceDeleted( KMFolder* folder,
                                               quint32 sernum )
{
  if( mResourceQuiet || !mUseResourceIMAP )
    return;

  QString type = folderContentsType( folder->storage()->contentsType() );
  //kDebug() << folder << type << sernum;
  if( !type.isEmpty() ) {
    // Get the index of the mail
    int i = 0;
    KMFolder* aFolder = 0;
    KMMsgDict::instance()->getLocation( sernum, &aFolder, &i );
    assert( folder == aFolder );

    // Read the iCal or vCard
    bool unget = !folder->isMessage( i );
    QString s;
    bool ok = false;
    KMMessage* msg = folder->getMsg( i );
    QString uid( "UID" );
    switch( storageFormat( folder ) ) {
    case StorageIcalVcard:
        if( vPartFoundAndDecoded( msg, s ) ) {
            vPartMicroParser( s, uid );
            ok = true;
        }
        break;
    case StorageXML:
        if ( kolabXMLFoundAndDecoded( *msg, folderKolabMimeType( folder->storage()->contentsType() ), s ) ) {
          uid = msg->subject();
          ok = true;
        }
        break;
    }
    if ( ok ) {
        kDebug() << "Emitting D-Bus signal incidenceDeleted("
                 << type << "," << folder->location() << "," << uid << ")";

        emit incidenceDeleted( type, folder->location(), uid );
    }
    if( unget ) folder->unGetMsg(i);
  } else
    kError() <<"Not a groupware folder";
}

// KMail orders a refresh
void KMailICalIfaceImpl::slotRefresh( const QString& type )
{
  if( mUseResourceIMAP ) {
    kDebug() << "Emitting D-Bus signal signalRefresh(" << type << " )";
    emit signalRefresh( type, QString() /* PENDING(bo) folder->location() */ );
  }
}

// This is among other things called when an expunge of a folder happens
void KMailICalIfaceImpl::slotRefreshFolder( KMFolder* folder)
{
  // TODO: The resources would of course be better off, if only this
  // folder would need refreshing. Currently it just orders a reload of
  // the type of the folder
  if( mUseResourceIMAP && folder ) {
    if( folder == mCalendar || folder == mContacts
        || folder == mNotes || folder == mTasks
        || folder == mJournals || mExtraFolders.contains( folder->location() ) ) {
      // Refresh the folder of this type
      KMail::FolderContentsType ct = folder->storage()->contentsType();
      slotRefresh( s_folderContentsType[ct].contentsTypeStr );
    }
  }
}

/****************************
 * The folder and message stuff code
 */

KMFolder* KMailICalIfaceImpl::folderFromType( const QString& type,
                                              const QString& folder )
{
  if( mUseResourceIMAP ) {
    KMFolder* f = 0;
    if ( !folder.isEmpty() ) {
      f = extraFolder( type, folder );
      if ( f )
        return f;
    }

    if( type == "Calendar" ) f = mCalendar;
    else if( type == "Contact" ) f = mContacts;
    else if( type == "Note" ) f = mNotes;
    else if( type == "Task" || type == "Todo" ) f = mTasks;
    else if( type == "Journal" ) f = mJournals;

    if ( f && ( folder.isEmpty() || folder == f->location() ) )
      return f;

    kError() <<"No folder (" << type <<"," << folder << ")";
  }

  return 0;
}


// Returns true if folder is a resource folder. If the resource isn't enabled
// this always returns false
bool KMailICalIfaceImpl::isResourceFolder( KMFolder* folder ) const
{
  return mUseResourceIMAP && folder &&
    ( isStandardResourceFolder( folder ) || mExtraFolders.contains( folder->location() ) );
}

bool KMailICalIfaceImpl::isStandardResourceFolder( KMFolder* folder ) const
{
  return ( folder == mCalendar || folder == mTasks || folder == mJournals ||
           folder == mNotes || folder == mContacts );
}

bool KMailICalIfaceImpl::hideResourceFolder( KMFolder* folder ) const
{
  return mHideFolders && isResourceFolder( folder );
}

bool KMailICalIfaceImpl::hideResourceAccountRoot( KMFolder* folder ) const
{
  KMFolderCachedImap *dimapFolder = dynamic_cast<KMFolderCachedImap*>( folder->storage() );
  bool hide = dimapFolder && mHideFolders
       && (int)dimapFolder->account()->id() == GlobalSettings::self()->theIMAPResourceAccount()
       && GlobalSettings::self()->showOnlyGroupwareFoldersForGroupwareAccount();
  return hide;

}

KPIM::FolderTreeWidgetItem::FolderType KMailICalIfaceImpl::folderType( KMFolder* folder ) const
{
  if( mUseResourceIMAP && folder ) {
    if( folder == mCalendar || folder == mContacts
        || folder == mNotes || folder == mTasks
        || folder == mJournals || mExtraFolders.contains( folder->location() ) ) {
      KMail::FolderContentsType ct = folder->storage()->contentsType();
      return s_folderContentsType[ct].treeItemType;
    }
  }

  return KPIM::FolderTreeWidgetItem::Other;
}

// Global tables of foldernames is different languages
// For now: 0->English, 1->German, 2->French, 3->Dutch
static QMap<KPIM::FolderTreeWidgetItem::FolderType,QString> folderNames[4];
QString KMailICalIfaceImpl::folderName( KPIM::FolderTreeWidgetItem::FolderType type, int language ) const
{
  // With the XML storage, folders are always (internally) named in English
  if ( GlobalSettings::self()->theIMAPResourceStorageFormat() == GlobalSettings::EnumTheIMAPResourceStorageFormat::XML )
    language = 0;

  static bool folderNamesSet = false;
  if( !folderNamesSet ) {
    folderNamesSet = true;
    /* NOTE: If you add something here, you also need to update
       GroupwarePage in configuredialog.cpp */

    // English
    folderNames[0][KPIM::FolderTreeWidgetItem::Calendar] = QString::fromLatin1("Calendar");
    folderNames[0][KPIM::FolderTreeWidgetItem::Tasks] = QString::fromLatin1("Tasks");
    folderNames[0][KPIM::FolderTreeWidgetItem::Journals] = QString::fromLatin1("Journal");
    folderNames[0][KPIM::FolderTreeWidgetItem::Contacts] = QString::fromLatin1("Contacts");
    folderNames[0][KPIM::FolderTreeWidgetItem::Notes] = QString::fromLatin1("Notes");

    // German
    folderNames[1][KPIM::FolderTreeWidgetItem::Calendar] = QString::fromLatin1("Kalender");
    folderNames[1][KPIM::FolderTreeWidgetItem::Tasks] = QString::fromLatin1("Aufgaben");
    folderNames[1][KPIM::FolderTreeWidgetItem::Journals] = QString::fromLatin1("Journal");
    folderNames[1][KPIM::FolderTreeWidgetItem::Contacts] = QString::fromLatin1("Kontakte");
    folderNames[1][KPIM::FolderTreeWidgetItem::Notes] = QString::fromLatin1("Notizen");

    // French
    folderNames[2][KPIM::FolderTreeWidgetItem::Calendar] = QString::fromLatin1("Calendrier");
    // Tasks = Tâches (â == 0xE2 in latin1)
    folderNames[2][KPIM::FolderTreeWidgetItem::Tasks] = QString::fromLatin1("T\342ches");
    folderNames[2][KPIM::FolderTreeWidgetItem::Journals] = QString::fromLatin1("Journal");
    folderNames[2][KPIM::FolderTreeWidgetItem::Contacts] = QString::fromLatin1("Contacts");
    folderNames[2][KPIM::FolderTreeWidgetItem::Notes] = QString::fromLatin1("Notes");

    // Dutch
    folderNames[3][KPIM::FolderTreeWidgetItem::Calendar] = QString::fromLatin1("Agenda");
    folderNames[3][KPIM::FolderTreeWidgetItem::Tasks] = QString::fromLatin1("Taken");
    folderNames[3][KPIM::FolderTreeWidgetItem::Journals] = QString::fromLatin1("Logboek");
    folderNames[3][KPIM::FolderTreeWidgetItem::Contacts] = QString::fromLatin1("Contactpersonen");
    folderNames[3][KPIM::FolderTreeWidgetItem::Notes] = QString::fromLatin1("Notities");
  }

  if( language < 0 || language > 3 ) {
    return folderNames[mFolderLanguage][type];
  }
  else {
    return folderNames[language][type];
  }
}


// Find message matching a given UID
KMMessage *KMailICalIfaceImpl::findMessageByUID( const QString& uid, KMFolder* folder )
{
  if( !folder || !mUIDToSerNum.contains( uid ) ) return 0;
  int i;
  KMFolder *aFolder;
  KMMsgDict::instance()->getLocation( mUIDToSerNum[uid], &aFolder, &i );
  Q_ASSERT( aFolder == folder );
  return folder->getMsg( i );
}

// Find message matching a given serial number
KMMessage *KMailICalIfaceImpl::findMessageBySerNum( quint32 serNum, KMFolder* folder )
{
  if( !folder || serNum == 0 ) return 0;

  KMMessage *message = 0;
  KMFolder* aFolder = 0;
  int index;
  KMMsgDict::instance()->getLocation( serNum, &aFolder, &index );
  if( aFolder && aFolder != folder ) {
    kWarning() <<"findMessageBySerNum(" << serNum << ") found it in folder" << aFolder->location() <<", expected" << folder->location();
  } else {
    if( aFolder )
      message = aFolder->getMsg( index );
    if ( !message ) {
      kWarning() << "Invalid serial number" << serNum;
    }
  }
  return message;
}

void KMailICalIfaceImpl::deleteMsg( KMMessage *msg )
{
  if( !msg ) return;
  // Commands are now delayed; can't use that anymore, we need immediate deletion
  //( new KMTrashMsgCommand( msg->parent(), msg ) )->start();
  KMFolder *srcFolder = msg->parent();
  int idx = srcFolder->find(msg);
  assert(idx != -1);
  if ( !msg->transferInProgress() ) {
    // kill existing jobs since we are about to delete the message
    srcFolder->ignoreJobsForMessage( msg );
    srcFolder->removeMsg(idx);
    delete msg;
  } else {
    kDebug() << "Message cannot be deleted now because it is currently in use " << msg;
    msg->deleteWhenUnused();
  }
  addFolderChange( srcFolder, ContentsChanged );
}

void KMailICalIfaceImpl::folderContentsTypeChanged( KMFolder* folder,
                                                    KMail::FolderContentsType contentsType )
{
  if ( !mUseResourceIMAP )
    return;
//  kDebug() << "folderContentsTypeChanged(" << folder->name()
//           << ", " << contentsType << ")\n";

  // The builtins can't change type
  if ( isStandardResourceFolder( folder ) )
    return;

  // Check if already know that 'extra folder'
  const QString location = folder->location();
  ExtraFolder* ef = mExtraFolders.value( location, 0 );
  if ( ef && ef->folder ) {
    // Notify that the old folder resource is no longer available
    emit subresourceDeleted( folderContentsType( folder->storage()->contentsType() ), location );

    if ( contentsType == KMail::ContentsTypeMail ) {
      // Delete the old entry, stop listening and stop here
      delete mExtraFolders.take( location );
      folder->disconnect( this );
      return;
    }
    // So the type changed to another groupware type, ok.
  } else {
    if ( ef && !ef->folder ) // deleted folder, clean up
      delete mExtraFolders.take( location );
    if ( contentsType == KMail::ContentsTypeMail )
        return;

    //kDebug() << "registering" << location << "as extra folder";
    // Make a new entry for the list
    ef = new ExtraFolder( folder );
    mExtraFolders.insert( location, ef );

    FolderInfo info = readFolderInfo( folder );
    mFolderInfoMap.insert( folder, info );

    // Adjust the folder names of all foo.default folders.
    // German users will get Kalender as the name of all default Calendar folders,
    // including their own, so that the default calendar folder of their Japanese
    // coworker appears as /user/hirohito/Kalender, although Hirohito sees his folder
    // in Japanese. On the server the folders are always in English.
    if ( folder->folderType() == KMFolderTypeCachedImap ) {
      QString annotation = static_cast<KMFolderCachedImap*>( folder->storage() )->annotationFolderType();
      kDebug() << "folderContentsTypeChanged:" << folder->name() << "has annotation" << annotation;
      if ( annotation == QString( s_folderContentsType[contentsType].annotation ) + ".default" )
        folder->setLabel( localizedDefaultFolderName( contentsType ) );
    }

    connectFolder( folder );
  }
  // Tell about the new resource
  emit subresourceAdded( folderContentsType( contentsType ), location,
                         subresourceLabelForPresentation( folder ),
                         folder->isWritable(),
                         folderIsAlarmRelevant( folder ) );
}

KMFolder* KMailICalIfaceImpl::extraFolder( const QString& type,
                                           const QString& folder )
{
  // If an extra folder exists that matches the type and folder location,
  // use that
  int t = folderContentsType( type );
  if ( t < 1 || t > 5 )
    return 0;

  ExtraFolder* ef = mExtraFolders.value( folder, 0 );
  if ( ef && ef->folder && ef->folder->storage()->contentsType() == t )
    return ef->folder;

  return 0;
}

StorageFormat KMailICalIfaceImpl::storageFormat( KMFolder* folder ) const
{
  FolderInfoMap::ConstIterator it = mFolderInfoMap.find( folder );
  if ( it != mFolderInfoMap.constEnd() )
    return (*it).mStorageFormat;
  return globalStorageFormat();
}

void KMailICalIfaceImpl::setStorageFormat( KMFolder* folder, StorageFormat format )
{
  FolderInfoMap::Iterator it = mFolderInfoMap.find( folder );
  if ( it != mFolderInfoMap.end() ) {
    (*it).mStorageFormat = format;
  } else {
    FolderInfo info( format, NoChange );
    mFolderInfoMap.insert( folder, info );
  }
  KConfigGroup configGroup( kmkernel->config(), "GroupwareFolderInfo" );
  configGroup.writeEntry( folder->idString() + "-storageFormat",
                          format == StorageXML ? "xml" : "icalvcard" );
}

void KMailICalIfaceImpl::addFolderChange( KMFolder* folder, FolderChanges changes )
{
  FolderInfoMap::Iterator it = mFolderInfoMap.find( folder );
  if ( it != mFolderInfoMap.end() ) {
    (*it).mChanges = static_cast<FolderChanges>( (*it).mChanges | changes );
  } else { // Otherwise, well, it's a folder we don't care about.
    kDebug() << "addFolderChange: nothing known about folder" << folder->location();
  }
  KConfigGroup configGroup( kmkernel->config(), "GroupwareFolderInfo" );
  configGroup.writeEntry( folder->idString() + "-changes", (int)(*it).mChanges );
}

KMailICalIfaceImpl::FolderInfo KMailICalIfaceImpl::readFolderInfo( const KMFolder * const folder ) const
{
  KConfigGroup configGroup( kmkernel->config(), "GroupwareFolderInfo" );
  QString str = configGroup.readEntry( folder->idString() + "-storageFormat", QString( "unset" ) );
  FolderInfo info;
  if ( str == "unset" ) {
    info.mStorageFormat = globalStorageFormat();
    configGroup.writeEntry( folder->idString() + "-storageFormat",
                            info.mStorageFormat == StorageXML ? "xml" : "icalvcard" );
  } else {
    info.mStorageFormat = ( str == "xml" ) ? StorageXML : StorageIcalVcard;
  }
  info.mChanges = (FolderChanges) configGroup.readEntry( folder->idString() + "-changes", 0 );
  return info;
}


void KMailICalIfaceImpl::folderSynced( KMFolder* folder, const KUrl& folderURL )
{
  FolderInfoMap::Iterator it = mFolderInfoMap.find( folder );
  if ( it != mFolderInfoMap.end() && (*it).mChanges ) {
    handleFolderSynced( folder, folderURL, (*it).mChanges );
    (*it).mChanges = NoChange;
  }
}

void KMailICalIfaceImpl::handleFolderSynced( KMFolder* folder,
                                             const KUrl& folderURL,
                                             int _changes )
{
  // This is done here instead of in the resource, because
  // there could be 0, 1, or N kolab resources at this point.
  // We can hack the N case, but not the 0 case.
  // So the idea of a D-Bus signal for this wouldn't work.
  if ( ( _changes & KMail::ContentsChanged ) ||
       ( _changes & KMail::ACLChanged ) ) {
    if ( storageFormat( folder ) == StorageXML && folder->storage()->contentsType() == KMail::ContentsTypeCalendar )
      triggerKolabFreeBusy( folderURL );
  }
}

void KMailICalIfaceImpl::folderDeletedOnServer( const KUrl& folderURL )
{
  triggerKolabFreeBusy( folderURL );
}

void KMailICalIfaceImpl::triggerKolabFreeBusy( const KUrl& folderURL )
{
  /* Steffen said: you must issue an authenticated HTTP GET request to
     https://kolabserver/freebusy/trigger/user@domain/Folder/NestedFolder.pfb
     (replace .pfb with .xpfb for extended fb lists). */
  KUrl httpURL( folderURL );
  // Keep username ("user@domain"), pass, and host from the imap url
  httpURL.setProtocol( "https" );
  httpURL.setPort( -1 ); // remove imap port

  // IMAP path is either /INBOX/<path> or /user/someone/<path>
  QString path = folderURL.path( KUrl::RemoveTrailingSlash );
  Q_ASSERT( path.startsWith( '/' ) );
  int secondSlash = path.indexOf( '/', 1 );
  if ( secondSlash == -1 ) {
    kWarning() << "path is too short:" << path;
    return;
  }
  if ( path.startsWith( QLatin1String("/INBOX/"), Qt::CaseInsensitive ) ) {
    // If INBOX, replace it with the username (which is user@domain)
    path = path.mid( secondSlash );
    path.prepend( folderURL.user() );
  } else {
    // If user, just remove it. So we keep the IMAP-returned username.
    // This assumes it's a known user on the same domain.
    path = path.mid( secondSlash );
  }

  httpURL.setPath( "/freebusy/trigger/" + path + ".pfb" );
  httpURL.setQuery( QString() );
  // Ensure that we encode everything with UTF8
  httpURL = KUrl( httpURL.url( KUrl::LeaveTrailingSlash ) );
  kDebug() << "Triggering PFB update for" << folderURL << ": getting" << httpURL;
  // "Fire and forget". No need for error handling, nor for explicit deletion.
  // Maybe we should try to prevent launching it if it's already running (for this URL) though.
  /*KIO::Job* job =*/ KIO::get( httpURL, KIO::NoReload, KIO::HideProgressInfo );
}

void KMailICalIfaceImpl::slotFolderPropertiesChanged( KMFolder* folder )
{
  if ( isResourceFolder( folder ) ) {
    const QString location = folder->location();
    const QString contentsTypeStr = folderContentsType( folder->storage()->contentsType() );

    emit subresourceDeleted( contentsTypeStr, location );
    emit subresourceAdded( contentsTypeStr, location,
                           subresourceLabelForPresentation( folder ),
                           folder->isWritable(),
                           folderIsAlarmRelevant( folder ) );
  }
}

// Must only be connected to a signal from KMFolder!
void KMailICalIfaceImpl::slotFolderRenamed()
{
  const KMFolder* folder = static_cast<const KMFolder *>( sender() );
  slotFolderPropertiesChanged( const_cast<KMFolder*>( folder ) );
}

void KMailICalIfaceImpl::slotFolderLocationChanged( const QString &oldLocation,
                                                    const QString &newLocation )
{
  KMFolder *folder = findResourceFolder( oldLocation );
  ExtraFolder* ef = mExtraFolders.value( oldLocation, 0 );
  if ( ef ) {
    // reuse the ExtraFolder entry, but adjust the key
    ExtraFolder *changedFolder = mExtraFolders.take( oldLocation );
    mExtraFolders.insert( newLocation, changedFolder );
  }

  if ( folder )
    emit subresourceDeleted( folderContentsType( folder->storage()->contentsType() ), oldLocation );
}

KMFolder* KMailICalIfaceImpl::findResourceFolder( const QString& resource )
{
  // Try the standard folders
  if( mCalendar && mCalendar->location() == resource )
    return mCalendar;
  if ( mContacts && mContacts->location() == resource )
    return mContacts;
  if ( mNotes && mNotes->location() == resource )
    return mNotes;
  if ( mTasks && mTasks->location() == resource )
    return mTasks;
  if ( mJournals && mJournals->location() == resource )
    return mJournals;

  // No luck. Try the extrafolders
  ExtraFolder* ef = mExtraFolders.value( resource, 0 );
  if ( ef )
    return ef->folder;

  // No luck at all
  return 0;
}

void KMailICalIfaceImpl::changeResourceUIName( const QString &folderPath, const QString &newName )
{
  kDebug() << "Folder path" << folderPath;
  KMFolder *f = findResourceFolder( folderPath );
  if ( f ) {
    KMailICalIfaceImpl::getResourceMap()->insert( folderPath, newName );
    kmkernel->folderMgr()->renameFolder( f, newName );
    KConfigGroup configGroup( kmkernel->config(), "Resource UINames" );
    configGroup.writeEntry( folderPath, newName );
  }
}

// Builds a folder list from the dimap and the local folder list.
static void createFolderList( QStringList &folderNames, QList<QPointer<KMFolder> > &folderList )
{
  QStringList dimapFolderNames;
  QStringList localFolderNames;
  QList<QPointer<KMFolder> > dimapFolderList;
  QList<QPointer<KMFolder> > localFolderList;
  kmkernel->dimapFolderMgr()->createFolderList( &dimapFolderNames, &dimapFolderList );
  kmkernel->folderMgr()->createFolderList( &localFolderNames, &localFolderList );
  folderNames += dimapFolderNames;
  folderNames += localFolderNames;
  folderList += dimapFolderList;
  folderList += localFolderList;
}

/****************************
 * The config stuff
 */

void KMailICalIfaceImpl::readConfig()
{
  bool enabled = GlobalSettings::self()->theIMAPResourceEnabled() &&
                 ( GlobalSettings::self()->theIMAPResourceAccount() != 0 );

  bool justEnabled = false;
  if( !enabled ) {
    if( mUseResourceIMAP == true ) {
      // Shutting down
      mUseResourceIMAP = false;
      cleanup();
      reloadFolderTree();
    }
    return;
  } else {
    justEnabled = enabled != mUseResourceIMAP;
  }
  mUseResourceIMAP = enabled;

  // Read remaining options
  const bool hideFolders = GlobalSettings::self()->hideGroupwareFolders();
  QString parentName = GlobalSettings::self()->theIMAPResourceFolderParent();

  // Find the folder parent
  KMFolderDir* folderParentDir;
  KMFolderType folderType;
  KMFolder* folderParent = kmkernel->findFolderById( parentName );
  if( folderParent == 0 ) {
    // Parent folder not found. It was probably deleted. The user will have to
    // configure things again.
    kDebug() << "Groupware folder" << parentName << "not found. Groupware functionality disabled";
    // Or maybe the inbox simply wasn't created on the first startup
    KMAccount* account = kmkernel->acctMgr()->find( GlobalSettings::self()->theIMAPResourceAccount() );
    if ( account ) {
      // just in case we were connected already
      disconnect( account, SIGNAL( finishedCheck( bool, CheckStatus ) ),
               this, SLOT( slotCheckDone() ) );
      connect( account, SIGNAL( finishedCheck( bool, CheckStatus ) ),
               this, SLOT( slotCheckDone() ) );
    } else {
      kDebug() << "null IMAP account!";
    }
    mUseResourceIMAP = false;
    // We can't really call cleanup(), if those folders were completely deleted.
    mCalendar = 0;
    mTasks    = 0;
    mJournals = 0;
    mContacts = 0;
    mNotes    = 0;
    return;
  } else {
    folderParentDir = folderParent->createChildFolder();
    folderType = folderParent->folderType();
  }

  KMAcctCachedImap::GroupwareType groupwareType = dynamic_cast<KMFolderCachedImap *>( folderParent->storage() )->account()->groupwareType();

  if ( groupwareType == KMAcctCachedImap::GroupwareKolab ) {
    // Make sure the folder parent has the subdirs
    // Globally there are 3 cases: nothing found, some stuff found by type/name heuristics, or everything found OK
    bool noneFound = true;
    bool mustFix = false; // true when at least one was found by heuristics
    QVector<StandardFolderSearchResult> results( KMail::ContentsTypeLast + 1 );
    for ( int i = 0; i < KMail::ContentsTypeLast+1; ++i ) {
      if ( i != KMail::ContentsTypeMail ) {
        results[i] = findStandardResourceFolder( folderParentDir, static_cast<KMail::FolderContentsType>(i) );
        if ( results[i].found == StandardFolderSearchResult::FoundAndStandard )
          noneFound = false;
        else if ( results[i].found == StandardFolderSearchResult::FoundByType ||
                  results[i].found == StandardFolderSearchResult::FoundByName ) {
          mustFix = true;
          noneFound = false;
        } else // NotFound
          mustFix = true;
      }
    }

    // Check if something changed
    if( mUseResourceIMAP && !noneFound && !mustFix && mFolderParentDir == folderParentDir
        && mFolderType == folderType ) {
      // Nothing changed
      if ( hideFolders != mHideFolders ) {
        // Well, the folder hiding has changed
        mHideFolders = hideFolders;
        reloadFolderTree();
      }
      return;
    }

    if( noneFound || mustFix ) {
      QString msg;
      QString parentFolderName = folderParent->name();
      if ( noneFound ) {
        // No subfolder was found, so ask if we can make them
        msg = i18n("KMail will now create the required groupware folders"
                   " as subfolders of %1; if you do not want this, cancel"
                   " and the IMAP resource will be disabled", parentFolderName);
      } else {
        // Some subfolders were found, be more precise
        QString operations = "<ul>";
        for ( int i = 0; i < KMail::ContentsTypeLast+1; ++i ) {
          if ( i != KMail::ContentsTypeMail ) {
            QString typeName = localizedDefaultFolderName( static_cast<KMail::FolderContentsType>( i ) );
            if ( results[i].found == StandardFolderSearchResult::NotFound )
              operations += "<li>" + i18n( "%1: no folder found, will create it", typeName ) + "</li>";
            else if ( results[i].found == StandardFolderSearchResult::FoundByType || results[i].found == StandardFolderSearchResult::FoundByName )
              operations += "<li>" + i18n( "%1: found folder %2, will set it as main groupware folder",
                             typeName, results[i].folder->label() ) + "</li>";
          }
        }
        operations += "</ul>";

        msg = i18n( "<qt>KMail found the following groupware folders in %1 and "
                    "needs to perform the following operations: %2<nl/>"
                    "If you do not want this, cancel"
                    " and the IMAP resource will be disabled</qt>"
                    , parentFolderName, operations );

      }

      if( KMessageBox::questionYesNo( 0, msg,
                                      i18n("Standard Groupware Folders"), KStandardGuiItem::cont(), KStandardGuiItem::cancel() ) == KMessageBox::No ) {

        GlobalSettings::self()->setTheIMAPResourceEnabled( false );
        mUseResourceIMAP = false;
        mFolderParentDir = 0;
        mFolderParent = 0;
        reloadFolderTree();
        return;
      }
    }

    // Make the new settings work
    mUseResourceIMAP = true;
    mFolderLanguage = GlobalSettings::self()->theIMAPResourceFolderLanguage();
    if( mFolderLanguage > 3 ) mFolderLanguage = 0;
    mFolderParentDir = folderParentDir;
    mFolderParent = folderParent;
    mFolderType = folderType;
    mHideFolders = hideFolders;

    // Close the previous folders
    cleanup();

    // Set the new folders
    mCalendar = initFolder( KMail::ContentsTypeCalendar );
    mTasks    = initFolder( KMail::ContentsTypeTask );
    mJournals = initFolder( KMail::ContentsTypeJournal );
    mContacts = initFolder( KMail::ContentsTypeContact );
    mNotes    = initFolder( KMail::ContentsTypeNote );

    if ( !mCalendar || !mTasks || !mJournals || !mContacts || !mNotes )
      return;

    // Store final annotation (with .default) so that we won't ask again on next startup
    if ( mCalendar->folderType() == KMFolderTypeCachedImap )
      static_cast<KMFolderCachedImap *>( mCalendar->storage() )->updateAnnotationFolderType();
    if ( mTasks->folderType() == KMFolderTypeCachedImap )
      static_cast<KMFolderCachedImap *>( mTasks->storage() )->updateAnnotationFolderType();
    if ( mJournals->folderType() == KMFolderTypeCachedImap )
      static_cast<KMFolderCachedImap *>( mJournals->storage() )->updateAnnotationFolderType();
    if ( mContacts->folderType() == KMFolderTypeCachedImap )
      static_cast<KMFolderCachedImap *>( mContacts->storage() )->updateAnnotationFolderType();
    if ( mNotes->folderType() == KMFolderTypeCachedImap )
      static_cast<KMFolderCachedImap *>( mNotes->storage() )->updateAnnotationFolderType();

    kDebug() << "mCalendar=" << mCalendar << mCalendar->location();
    kDebug() << "mContacts=" << mContacts << mContacts->location();
    kDebug() << "mNotes=" << mNotes << mNotes->location();

    // Find all extra folders
    QStringList folderNames;
    QList<QPointer<KMFolder> > folderList;
    createFolderList( folderNames, folderList );
    for(QList<QPointer<KMFolder> >::iterator it = folderList.begin();
        it != folderList.end(); ++it)
    {
      FolderStorage *storage = (*it)->storage();
      KMFolderCachedImap* dimapStorage = dynamic_cast<KMFolderCachedImap*>( storage );
      if ( storage && storage->contentsType() != 0 ) {
        if ( dimapStorage )
          dimapStorage->updateAnnotationFolderType();
        folderContentsTypeChanged( *it, storage->contentsType() );
      }
    }

    // If we just created them, they might have been registered as extra folders temporarily.
    // -> undo that.
    if ( mExtraFolders.contains( mCalendar->location() ) )
      delete mExtraFolders.take( mCalendar->location() );
    if ( mExtraFolders.contains( mTasks->location() ) )
      delete mExtraFolders.take( mTasks->location() );
    if ( mExtraFolders.contains( mJournals->location() ) )
      delete mExtraFolders.take( mJournals->location() );
    if ( mExtraFolders.contains( mContacts->location() ) )
      delete mExtraFolders.take( mContacts->location() );
    if ( mExtraFolders.contains( mNotes->location() ) )
      delete mExtraFolders.take( mNotes->location() );

    emit subresourceAdded( folderContentsType( KMail::ContentsTypeCalendar ),
                           mCalendar->location(), mCalendar->label(), true, true );
    emit subresourceAdded( folderContentsType( KMail::ContentsTypeTask ),
                           mTasks->location(), mTasks->label(), true, true );
    emit subresourceAdded( folderContentsType( KMail::ContentsTypeJournal ),
                           mJournals->location(), mJournals->label(), true, false );
    emit subresourceAdded( folderContentsType( KMail::ContentsTypeContact ),
                           mContacts->location(), mContacts->label(), true, false );
    emit subresourceAdded( folderContentsType( KMail::ContentsTypeNote ),
                           mNotes->location(), mNotes->label(), true, false );
  }

  KSharedConfig::Ptr config = kmkernel->config();
  KConfigGroup cf = config->group( "Resource UINames" );
  *KMailICalIfaceImpl::mSubResourceUINamesMap = cf.entryMap();

  reloadFolderTree();

  if ( justEnabled ) {
    for ( int i = KMail::ContentsTypeCalendar; i <= KMail::ContentsTypeLast; ++i ) {
      foreach ( const KMail::SubResource &res,
                subresourcesKolab( folderContentsType( (FolderContentsType)i ) ) ) {
        emit subresourceAdded( folderContentsType( (FolderContentsType)i ),
                               res.location, res.label,
                               res.writable, res.alarmRelevant );
      }
    }
  }
}

void KMailICalIfaceImpl::slotCheckDone()
{
  QString parentName = GlobalSettings::self()->theIMAPResourceFolderParent();
  KMFolder* folderParent = kmkernel->findFolderById( parentName );
  //kDebug() << "folderParent=" << folderParent;
  if ( folderParent )  // cool it exists now
  {
    KMAccount* account = kmkernel->acctMgr()->find( GlobalSettings::self()->theIMAPResourceAccount() );
    if ( account )
      disconnect( account, SIGNAL( finishedCheck( bool, CheckStatus ) ),
                  this, SLOT( slotCheckDone() ) );
    readConfig();
  }
}

KMFolder* KMailICalIfaceImpl::initFolder( KMail::FolderContentsType contentsType )
{
  // Figure out what type of folder this is supposed to be
  KMFolderType type = mFolderType;
  if( type == KMFolderTypeUnknown ) type = KMFolderTypeMaildir;

  KPIM::FolderTreeWidgetItem::FolderType itemType = s_folderContentsType[contentsType].treeItemType;
  //kDebug() << folderName( itemType );

  // Find the folder
  StandardFolderSearchResult result = findStandardResourceFolder( mFolderParentDir, contentsType );

  // deal with multiple default groupware folders
  if ( result.folders.count() > 1 && result.found == StandardFolderSearchResult::FoundAndStandard ) {
    QStringList labels;
    for ( QList<KMFolder*>::ConstIterator it = result.folders.constBegin(); it != result.folders.constEnd(); ++it )
      labels << (*it)->prettyUrl();
    const QString selected = KInputDialog::getItem( i18n("Default folder"),
        i18nc( "%1 is one of the messages with context 'type of folder content'",
               "There are multiple %1 default folders, please choose one:",
               localizedDefaultFolderName( contentsType ) ), labels );
    if ( !selected.isEmpty() )
      result.folder = result.folders[ labels.indexOf( selected ) ];
  }

  KMFolder* folder = result.folder;

  if ( !folder ) {
    // The folder isn't there yet - create it
    folder =
      mFolderParentDir->createFolder( localizedDefaultFolderName( contentsType ), false, type );
    if( mFolderType == KMFolderTypeImap ) {
      KMFolderImap* parentFolder = static_cast<KMFolderImap*>( mFolderParent->storage() );
      parentFolder->createFolder( localizedDefaultFolderName( contentsType ) );
      static_cast<KMFolderImap*>( folder->storage() )->setAccount( parentFolder->account() );
    }
    // Groupware folder created, use the global setting for storage format
    setStorageFormat( folder, globalStorageFormat() );
  } else {
    FolderInfo info = readFolderInfo( folder );
    mFolderInfoMap.insert( folder, info );
    //kDebug() << "Found existing folder type" << itemType << ":" << folder->location();
  }

  if( !folder->canAccess() ) {
    KMessageBox::sorry(0, i18n("You do not have read/write permission to your %1 folder.",
                         folderName( itemType ) ) );
    return 0;
  }
  folder->storage()->setContentsType( contentsType );
  folder->setSystemFolder( true );
  folder->storage()->writeConfig();
  if ( 0 != folder->open( "ifacefolder" ) ) {
    kWarning() << "folder->open( \"ifacefolder\" ) ) FAILED";
    // TODO: remove data structures?
    return 0;
  }
  connectFolder( folder );
  return folder;
}

void KMailICalIfaceImpl::connectFolder( KMFolder* folder )
{
  // avoid multiple connections
  disconnect( folder, SIGNAL( msgAdded( KMFolder*, quint32 ) ),
              this, SLOT( slotIncidenceAdded( KMFolder*, quint32 ) ) );
  disconnect( folder, SIGNAL( msgRemoved( KMFolder*, quint32 ) ),
              this, SLOT( slotIncidenceDeleted( KMFolder*, quint32 ) ) );
  disconnect( folder, SIGNAL( expunged( KMFolder* ) ),
              this, SLOT( slotRefreshFolder( KMFolder* ) ) );
  disconnect( folder->storage(), SIGNAL( readOnlyChanged( KMFolder* ) ),
              this, SLOT( slotFolderPropertiesChanged( KMFolder* ) ) );
  disconnect( folder, SIGNAL( nameChanged() ),
              this, SLOT( slotFolderRenamed() ) );
  disconnect( folder->storage(), SIGNAL( locationChanged( const QString&, const QString&) ),
              this, SLOT( slotFolderLocationChanged( const QString&, const QString&) ) );

  // Setup the signals to listen for changes
  connect( folder, SIGNAL( msgAdded( KMFolder*, quint32 ) ),
           this, SLOT( slotIncidenceAdded( KMFolder*, quint32 ) ) );
  connect( folder, SIGNAL( msgRemoved( KMFolder*, quint32 ) ),
           this, SLOT( slotIncidenceDeleted( KMFolder*, quint32 ) ) );
  connect( folder, SIGNAL( expunged( KMFolder* ) ),
           this, SLOT( slotRefreshFolder( KMFolder* ) ) );
  connect( folder->storage(), SIGNAL( readOnlyChanged( KMFolder* ) ),
           this, SLOT( slotFolderPropertiesChanged( KMFolder* ) ) );
  connect( folder, SIGNAL( nameChanged() ),
           this, SLOT( slotFolderRenamed() ) );
  connect( folder->storage(), SIGNAL( locationChanged( const QString&, const QString&) ),
           this, SLOT( slotFolderLocationChanged( const QString&, const QString&) ) );

}

static void cleanupFolder( KMFolder* folder, KMailICalIfaceImpl* _this )
{
  if( folder ) {
    folder->setSystemFolder( false );
    folder->disconnect( _this );
    folder->close( "ifacefolder" );
  }
}

void KMailICalIfaceImpl::cleanup()
{
  cleanupFolder( mContacts, this );
  cleanupFolder( mCalendar, this );
  cleanupFolder( mNotes, this );
  cleanupFolder( mTasks, this );
  cleanupFolder( mJournals, this );

  mContacts = mCalendar = mNotes = mTasks = mJournals = 0;
}

QString KMailICalIfaceImpl::folderPixmap( KPIM::FolderTreeWidgetItem::FolderType type ) const
{
  if( !mUseResourceIMAP )
    return QString();

  if( type == KPIM::FolderTreeWidgetItem::Contacts )
    return QString::fromLatin1( "view-pim-contacts" );
  else if( type == KPIM::FolderTreeWidgetItem::Calendar )
    return QString::fromLatin1( "view-calendar" );
  else if( type == KPIM::FolderTreeWidgetItem::Notes )
    return QString::fromLatin1( "view-pim-notes" );
  else if( type == KPIM::FolderTreeWidgetItem::Tasks )
    return QString::fromLatin1( "view-pim-tasks" );
  else if( type == KPIM::FolderTreeWidgetItem::Journals )
    return QString::fromLatin1( "view-pim-journal" );

  return QString();
}

static void reloadFolderTree()
{
  // Make the folder tree show the icons or not
  kmkernel->folderMgr()->contentsChanged();
}

// This is a very light-weight and fast 'parser' to retrieve
// a data entry from a vCal taking continuation lines
// into account
static void vPartMicroParser( const QString& str, QString& s )
{
  QString line;
  uint len = str.length();

  for( uint i=0; i<len; ++i){
    if( str[i] == '\r' || str[i] == '\n' ){
      if( str[i] == '\r' )
        ++i;
      if( i+1 < len && str[i+1] == ' ' ){
        // found a continuation line, skip it's leading blanc
        ++i;
      }else{
        // found a logical line end, process the line
        if( line.startsWith( s ) ) {
          s = line.mid( s.length() + 1 );
          return;
        }
        line = "";
      }
    } else {
      line += str[i];
    }
  }

  // Not found. Clear it
  s.truncate(0);
}

// Returns the first child folder having the given annotation
static QList<KMFolder*> findFolderByAnnotation( KMFolderDir* folderParentDir, const QString& annotation )
{
  QList<KMFolder*> rv;
  QList<KMFolderNode*>::const_iterator it;
  for ( it = folderParentDir->constBegin(); it != folderParentDir->constEnd(); ++it ) {
    if ( !(*it)->isDir() ) {
      KMFolder* folder = static_cast<KMFolder *>( *it );
      if ( folder->folderType() == KMFolderTypeCachedImap ) {
        QString folderAnnotation = static_cast<KMFolderCachedImap*>( folder->storage() )->annotationFolderType();
        //kDebug() << "findStandardResourceFolder:" << folder->name() << "has annotation" << folderAnnotation;
        if ( folderAnnotation == annotation )
          rv.append( folder );
      }
    }
  }
  return rv;
}

KMailICalIfaceImpl::StandardFolderSearchResult KMailICalIfaceImpl::findStandardResourceFolder( KMFolderDir* folderParentDir, KMail::FolderContentsType contentsType )
{
  if ( GlobalSettings::self()->theIMAPResourceStorageFormat() == GlobalSettings::EnumTheIMAPResourceStorageFormat::XML )
  {
    // Look for a folder with an annotation like "event.default"
    QList<KMFolder*> folders = findFolderByAnnotation( folderParentDir, QString( s_folderContentsType[contentsType].annotation ) + ".default" );
    if ( !folders.isEmpty() )
      return StandardFolderSearchResult( folders, StandardFolderSearchResult::FoundAndStandard );

    // Fallback: look for a folder with an annotation like "event"
    folders = findFolderByAnnotation( folderParentDir, QString( s_folderContentsType[contentsType].annotation ) );
    if ( !folders.isEmpty() )
      return StandardFolderSearchResult( folders, StandardFolderSearchResult::FoundByType );

    // Fallback: look for the folder by name (we'll need to change its type)
    KMFolderNode* node = folderParentDir->hasNamedFolder( localizedDefaultFolderName( contentsType ) );
    if ( node && !node->isDir() )
      return StandardFolderSearchResult( static_cast<KMFolder *>( node ), StandardFolderSearchResult::FoundByName );

    kDebug() << "findStandardResourceFolder: found no resource folder for" << s_folderContentsType[contentsType].annotation;
    return StandardFolderSearchResult( 0, StandardFolderSearchResult::NotFound );
  }
  else // icalvcard: look up standard resource folders by name
  {
    KPIM::FolderTreeWidgetItem::FolderType itemType = s_folderContentsType[contentsType].treeItemType;
    unsigned int folderLanguage = GlobalSettings::self()->theIMAPResourceFolderLanguage();
    if( folderLanguage > 3 ) folderLanguage = 0;
    KMFolderNode* node = folderParentDir->hasNamedFolder( folderName( itemType, folderLanguage ) );
    if ( !node || node->isDir() )
      return StandardFolderSearchResult( 0, StandardFolderSearchResult::NotFound );
    return StandardFolderSearchResult( static_cast<KMFolder*>( node ), StandardFolderSearchResult::FoundAndStandard );
  }
}

/* We treat all folders as relevant wrt alarms for which we have Administer
 * rights or for which the "Incidences relevant for everyone" annotation has
 * been set. It can be reasonably assumed that those are "ours". All local folders
 * must be ours anyhow. */
bool KMailICalIfaceImpl::folderIsAlarmRelevant( const KMFolder *folder )
{
  bool administerRights = true;
  bool relevantForOwner = true;
  bool relevantForEveryone = false;
  if ( folder->folderType() == KMFolderTypeImap ) {
    const KMFolderImap *imapFolder = static_cast<const KMFolderImap*>( folder->storage() );
    administerRights =
      imapFolder->userRights() <= 0 || imapFolder->userRights() & KMail::ACLJobs::Administer;
  }
  if ( folder->folderType() == KMFolderTypeCachedImap ) {
    const KMFolderCachedImap *dimapFolder = static_cast<const KMFolderCachedImap*>( folder->storage() );
    administerRights =
      dimapFolder->userRights() <= 0 || dimapFolder->userRights() & KMail::ACLJobs::Administer;
    relevantForOwner = dimapFolder->incidencesFor () == KMFolderCachedImap::IncForAdmins;
    relevantForEveryone = ( dimapFolder->incidencesFor() == KMFolderCachedImap::IncForReaders );
  }
#if 0
  kDebug() ;
  kDebug() << "Folder:" << folder->label() << "has administer rights:" << administerRights;
  kDebug() << "and is relevant for owner:" << relevantForOwner;
  kDebug() << "and relevant for everyone:"  << relevantForEveryone;
#endif
  return ( administerRights && relevantForOwner ) || relevantForEveryone;
}

void KMailICalIfaceImpl::setResourceQuiet(bool q)
{
  mResourceQuiet = q;
}

bool KMailICalIfaceImpl::isResourceQuiet() const
{
  return mResourceQuiet;
}


bool KMailICalIfaceImpl::addSubresource( const QString& resource,
                                         const QString& parent,
                                         const QString& contentsType )
{
  kDebug() << "Adding subresource to parent:" << parent << "with name:" << resource;
  kDebug() << "contents type:" << contentsType;
  KMFolder *folder = findResourceFolder( parent );
  KMFolderDir *parentFolderDir = !parent.isEmpty() && folder ? folder->createChildFolder(): mFolderParentDir;
  if ( !parentFolderDir || parentFolderDir->hasNamedFolder( resource ) ) return false;

  QString msg;
  if ( parentFolderDir->owner() && !KMFolder::isValidName( parentFolderDir->owner(), resource, msg ) ) {
    KMessageBox::error( 0, msg );
    return false;
  }

  KMFolderType type = mFolderType;
  if( type == KMFolderTypeUnknown ) type = KMFolderTypeMaildir;

  KMFolder* newFolder = parentFolderDir->createFolder( resource, false, type );
  if ( !newFolder ) return false;
  if( mFolderType == KMFolderTypeImap )
    static_cast<KMFolderImap*>( folder->storage() )->createFolder( resource );

  StorageFormat defaultFormat = GlobalSettings::self()->theIMAPResourceStorageFormat() == GlobalSettings::EnumTheIMAPResourceStorageFormat::XML ? StorageXML : StorageIcalVcard;
  setStorageFormat( newFolder, folder ? storageFormat( folder ) : defaultFormat );
  newFolder->storage()->setContentsType( folderContentsType( contentsType ) );
  newFolder->storage()->writeConfig();
  newFolder->open( "KMailICalIfaceImpl::addSubresource" );
  connectFolder( newFolder );
  reloadFolderTree();

  return true;
}

bool KMailICalIfaceImpl::removeSubresource( const QString& location )
{
  kDebug();

  KMFolder *folder = findResourceFolder( location );

  // We don't allow the default folders to be deleted, so check for
  // those first. It would be nicer to produce a more meaningful error,
  // or prevent deletion of the builtin folders from the gui already.
  if ( !folder || isStandardResourceFolder( folder ) )
      return false;

  // the folder will be removed, which implies closed, so make sure
  // nothing is using it anymore first
  emit subresourceDeleted( folderContentsType( folder->storage()->contentsType() ), location );

  if ( mExtraFolders.contains( location ) )
    delete mExtraFolders.take( location );
  folder->disconnect( this );

  if ( folder->folderType() == KMFolderTypeImap )
    kmkernel->imapFolderMgr()->remove( folder );
  else if ( folder->folderType() == KMFolderTypeCachedImap ) {
    // Deleted by user -> tell the account (see KMFolderCachedImap::listDirectory2)
    KMFolderCachedImap* storage = static_cast<KMFolderCachedImap*>( folder->storage() );
    KMAcctCachedImap* acct = storage->account();
    if ( acct )
      acct->addDeletedFolder( folder );
    kmkernel->dimapFolderMgr()->remove( folder );
  }
  return true;
}

void KMailICalIfaceImpl::syncFolder(KMFolder * folder) const
{
  if ( kmkernel->isOffline() || !GlobalSettings::immediatlySyncDIMAPOnGroupwareChanges() )
    return;
  KMFolderCachedImap *dimapFolder = dynamic_cast<KMFolderCachedImap*>( folder->storage() );
  if ( !dimapFolder )
    return;
  // check if the folder exists already, otherwise sync its parent as well to create it
  if ( dimapFolder->imapPath().isEmpty() ) {
    if ( folder->parent() && folder->parent()->owner() )
      syncFolder( folder->parent()->owner() );
    else
      return;
  }
  dimapFolder->account()->processNewMailInFolder( folder );
}

#include "kmailicalifaceimpl.moc"
