/*
    kmgroupware.h

    This file is part of KMail.

    Copyright (c) 2003 Bo Thorsen <bo@klaralvdalens-datakonsult.se>
    Copyright (c) 2002 Karl-Heinz Zimmer <khz@klaralvdalens-datakonsult.se>
    Copyright (c) 2003 Steffen Hansen <steffen@klaralvdalens-datakonsult.se>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KMGROUPWARE_H
#define KMGROUPWARE_H

#include "kmfoldertype.h"
#include <kfoldertree.h>
#include <qguardedptr.h>

class QSplitter;
class QDateTime;

class DCOPClient;

class KMKernel;
class KMFolder;
class KMFolderDir;
class KMFolderTreeItem;
class KMAccount;
class KMMainWin;
class KMMessage;
class KMHeaders;
class KMReaderWin;
class KMMimePartTree;
class KMMsgBase;
class KURL;

namespace KParts {
  class ReadOnlyPart;
};


class KMGroupware : public QObject
{
  Q_OBJECT

public:
  KMGroupware( QObject* parent = 0, const char* name = 0 );
  virtual ~KMGroupware();

  ////////////////////////////////////////////////////////////////
  // Resource IMAP interface - See also kmailicalifaceimap.(h|cpp)
  bool addIncidence( const QString& type,
		     const QString& uid,
		     const QString& ical );
  bool deleteIncidence( const QString& type, const QString& uid );
  QStringList incidences( const QString& type );

signals:
  void incidenceAdded( const QString& type, const QString& ical );
  void incidenceDeleted( const QString& type, const QString& uid );

  /** Make the IMAP resource re-read all of the given type */
  void signalRefresh( const QString& type);

private slots:
  // internal slots for new interface
  void slotIncidenceAdded( KMFolder*, Q_UINT32 );
  void slotIncidenceDeleted( KMFolder*, Q_UINT32 );
  void slotRefreshCalendar();
  void slotRefreshTasks();

  ////////////////////////////////////////////////////////////////

public:

  /**
   * Returns true if groupware mode is enabled and folder is one of the
   * groupware folders.
   */
  bool isGroupwareFolder( KMFolder* folder ) const;

  /**
   * Returns the groupware folder type. Other is returned if groupware
   * isn't enabled or it isn't a groupware folder.
   */
  KFolderTreeItem::Type folderType( KMFolder* folder ) const;

  QString folderName( KFolderTreeItem::Type type, int language = -1 ) const;
  bool folderSelected( KMFolder* folder );
  bool isContactsFolder( KMFolder* folder ) const;
  bool checkFolders() const;

  /** Initialize all folders. */
  void initFolders();

  /** Disconnect all slots and close the dirs. */
  void cleanup();

  bool setFolderPixmap(const KMFolder& folder, KMFolderTreeItem& fti) const;
  void setupKMReaderWin(KMReaderWin* reader);
  void setMimePartTree(KMMimePartTree* mimePartTree);
  void createKOrgPart(QWidget* parent);
  void reparent(QSplitter* panner);
  void moveToLast();
  void setupActions(); // Not const since it emits a signal
  void enableActions(bool on) const;
  void processVCalRequest( const QCString& receiver, const QString& vCalIn,
                           QString& choice );
  void processVCalReply( const QCString& sender, const QString& vCalIn,
			 const QString& choice );

  /* (Re-)Read configuration file */
  void readConfig();

  bool isEnabled() const { return mUseGroupware; }

  bool hidingMimePartTree(){ return mGroupwareIsHidingMimePartTree; }

  // find message matching a given UID and possibly take if from the folder
  static KMMessage* findMessageByUID( const QString& uid, KMFolder* folder );

  // Convenience function to delete a message
  static void deleteMsg( KMMessage* msg );

  // retrieve matching body part (either text/vCal (or vCard) or application/ms-tnef)
  // and decode it
  // returns a readable vPart in *s or in *sc or in both
  // (please make sure to set s or sc to o if you don't want ot use it)
  // note: Additionally the number of the update counter (if any was found) is
  //       returned in aUpdateCounter, this applies only to TNEF data - in the
  //       iCal standard (RfC2445,2446) there is no update counter.
  static bool vPartFoundAndDecoded( KMMessage* msg,
                                    int& aUpdateCounter,
                                    QString& s );

  enum DefaultUpdateCounterValue { NoUpdateCounter=-1 };
  // functions to be called by KMReaderWin for 'print formatting'
  static bool vPartToHTML( int aUpdateCounter, const QString& vCal, QString fname,
                           bool useGroupware, QString& prefix, QString& postfix );
  static bool msTNEFToVPart( const QByteArray& tnef,
                             int& aUpdateCounter, // there is no such counter in RfC2466 (khz)
                             QString& aVPart );
  static bool msTNEFToHTML( KMReaderWin* reader, QString& vPart, QString fname, bool useGroupware,
                            QString& prefix, QString& postfix );

  // function to be called by KMReaderWin for analyzing of clicked URL
  static bool foundGroupwareLink( const QString aUrl,
                                  QString& gwType,
                                  QString& gwAction,
                                  QString& gwAction2,
                                  QString& gwData );

  /** KMReaderWin calls this with an URL. Return true if a groupware url was
      handled. */
  virtual bool handleLink( const KURL &aUrl, KMMessage* msg );

  /** These methods are called by KMKernel's DCOP functions. */
  virtual void requestAddresses( QString );
  virtual bool storeAddresses(QString, QStringList);
  virtual bool lockContactsFolder();
  virtual bool unlockContactsFolder();

  // automatic resource handling
  bool incomingResourceMessage( KMAccount*, KMMessage* );

  // tell KOrganizer about messages to be deleted
  void msgRemoved( KMFolder*, KMMessage* );

  void setMainWin(KMMainWin *mainWin) { mMainWin = mainWin; }
  void setHeaders(KMHeaders* headers );

public slots:
  /** View->Groupware menu */
  void slotGroupwareHide();
  /** additional groupware slots */
  void slotGroupwareShow(bool);

  /** KO informs about a new or updated note */
  void slotNewOrUpdatedNote( const QString& id, const QString& geometry, const QColor& color,
			     const QString& text );
  void slotDeleteNote( const QString& id );

  void slotNotesFolderChanged();

  /** Delete and sync the local IMAP cache  */
  void slotInvalidateIMAPFolders();

protected:
  void saveActionEnable( const QString& name, bool on ) const;

  // Figure out if a vCal is a todo, event or neither
  enum VCalType { vCalEvent, vCalTodo, vCalUnknown };
  static VCalType getVCalType( const QString &vCard );

  /** This class functions as an event filter while showing groupware widgets */
  bool eventFilter( QObject *o, QEvent *e ) const;

  // We use QGuardedPtr for everything, since
  // we are not the owner of any of those objects
  QGuardedPtr<KMMainWin>      mMainWin;
  QGuardedPtr<KMHeaders>      mHeaders;
  QGuardedPtr<KMReaderWin>    mReader;
  QGuardedPtr<KMMimePartTree> mMimePartTree;
  // groupware folder icons:
  static QPixmap *pixContacts, *pixCalendar, *pixNotes, *pixTasks;

signals:
  void signalSetKroupwareCommunicationEnabled( QObject* );

  /** Initialize Groupware with a list of Notes entries */
  void signalRefreshNotes( const QStringList& );

  /** Make sure a given time span is visible in the Calendar */
  void signalCalendarUpdateView( const QDateTime&, const QDateTime& );

  void signalShowCalendarView();
  void signalShowContactsView();
  void signalShowNotesView();
  void signalShowTodoView();

  /** Open Groupware to consider accepting/declining an invitation */
  void signalEventRequest( const QCString& receiver, const QString&, bool&,
			   QString&, QString&, bool& );

  /** Use Groupware to create an answer to a resource request. */
  void signalResourceRequest( const QValueList<QPair<QDateTime, QDateTime> >& busy,
                              const QCString& resource,
                              const QString& vCalIn, bool& vCalInOK,
                              QString& vCalOut, bool& vCalOutOK,
                              bool& isFree, QDateTime& start, QDateTime& end );

  /** Accept an invitation without checking: Groupware will *not* show up */
  void signalAcceptedEvent( bool, const QCString&, const QString&, bool&,
			    QString&, bool& );

  /** Reject an invitation: Groupware will *not* show up */
  void signalRejectedEvent( const QCString&, const QString&, bool&, QString&,
			    bool& );

  /** Answer an invitation */
  void signalIncidenceAnswer( const QCString&, const QString&, QString& );

  /** An event was deleted */
  void signalEventDeleted( const QString& );

  /** A task was deleted */
  void signalTaskDeleted( const QString& );

  /** A note was deleted */
  void signalNoteDeleted( const QString& );

  /** The menus were changed */
  void signalMenusChanged();

private:
  /** Helper function for initFolders. Initializes a single folder. */
  KMFolder* initFolder( KFolderTreeItem::Type itemType, const char* typeString );

  void internalCreateKOrgPart();
  void loadPixmaps() const;
  void setEnabled( bool b );

  bool mUseGroupware;
  bool mGroupwareIsHidingMimePartTree;
  int  mFolderLanguage;
  QSplitter* mPanner;
  QGuardedPtr<KParts::ReadOnlyPart> mKOrgPart;
  QGuardedPtr<QWidget> mKOrgPartParent;

  KMFolderDir* mFolderParent;
  KMFolderType mFolderType;

  bool      mContactsLocked;
  KMFolder* mInbox;
  KMFolder* mContacts;
  KMFolder* mCalendar;
  KMFolder* mNotes;
  KMFolder* mTasks;
};

#endif /* KMGROUPWARE_H */
