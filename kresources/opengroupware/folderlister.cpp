/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "folderlister.h"

#include "webdavhandler.h"

#include <kio/davjob.h>
#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>

#include <qdom.h>

using namespace KCal;

FolderLister::FolderLister( Type type )
  : mType( type )
{
}

void FolderLister::setFolders( const FolderLister::Entry::List &folders )
{
  mFolders = folders;
}

void FolderLister::setWriteDestinationId( const QString &id )
{
  mWriteDestinationId = id;
}

QStringList FolderLister::activeFolderIds() const
{
  QStringList ids;

  FolderLister::Entry::List::ConstIterator it;
  for( it = mFolders.begin(); it != mFolders.end(); ++it ) {
    if ( (*it).active ) {
      ids.append( (*it).id );
    }
  }
  
  return ids;
}

bool FolderLister::isActive( const QString &id ) const
{
  FolderLister::Entry::List::ConstIterator it;
  for( it = mFolders.begin(); it != mFolders.end(); ++it ) {
    if ( (*it).id == id ) return (*it).active;
  }
  return false;  
}

void FolderLister::readConfig( const KConfig *config )
{
  kdDebug() << "FolderLister::readConfig()" << endl;

  QStringList ids = config->readListEntry( "FolderIds" );
  QStringList names = config->readListEntry( "FolderNames" );
  QStringList active = config->readListEntry( "ActiveFolders" );
  
  QStringList::ConstIterator it;
  QStringList::ConstIterator it2 = names.begin();
  for( it = ids.begin(); it != ids.end(); ++it ) {
    Entry e;
    e.id = *it;
    if ( it2 != names.end() ) e.name = *it2;
    else e.name = *it;
    if ( active.find( e.id ) != active.end() ) e.active = true;
    
    mFolders.append( e );
    
    ++it2;
  }

  mWriteDestinationId = config->readEntry( "WriteDestinationId" );
}

void FolderLister::writeConfig( KConfig *config )
{
  QStringList ids;
  QStringList names;
  QStringList active;

  Entry::List::ConstIterator it;
  for( it = mFolders.begin(); it != mFolders.end(); ++it ) {
    ids.append( (*it).id );
    names.append( (*it).name );
    if ( (*it).active ) active.append( (*it).id );
  }
  
  config->writeEntry( "FolderIds", ids );
  config->writeEntry( "FolderNames", names );
  config->writeEntry( "ActiveFolders", active );

  config->writeEntry( "WriteDestinationId", mWriteDestinationId );
}

KURL FolderLister::adjustUrl( const KURL &u )
{
  KURL url = u;
  if ( mType == Calendar ) {
    url.addPath( "Groups" );
  } else {
    url.addPath( "public" );
  }
  return url;
}

KIO::DavJob *FolderLister::createJob( const KURL &url )
{
  QDomDocument props = WebdavHandler::createAllPropsRequest();
  kdDebug(7000) << "props: " << props.toString() << endl;
  return KIO::davPropFind( url, props, "1", false );
}

void FolderLister::retrieveFolders( const KURL &u )
{
  mUrl = WebdavHandler::toDAV( u );

  kdDebug(7000) << "FolderLister::retrieveFolders: " << getUrl().prettyURL() << endl;

  KURL url( adjustUrl( getUrl() ) );

  mListEventsJob = createJob( url );

  kdDebug(7000) << "FolderLister::retrieveFolders: adjustedURL=" << url.prettyURL() << endl;
  connect( mListEventsJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotListJobResult( KIO::Job * ) ) );
}

FolderLister::Entry::List FolderLister::defaultFolders()
{
  Entry::List newFolders;

  // Personal calendar/addressbook
  Entry personal;
  KURL url = getUrl();
  url.setUser( QString::null );
  url.setPass( QString::null );
  if ( mType == Calendar ) {
    personal.name = i18n("Personal Calendar");
    personal.id = url.url();
  } else {
    personal.name = i18n("Personal Addressbook");
    url.addPath( "Contacts" );
    personal.id = url.url();
  }
  newFolders.append( personal );
  return newFolders;
}

FolderLister::FolderType FolderLister::getFolderType( const QDomNode &folderNode )
{
  QDomNode n4;
  for( n4 = folderNode.firstChild(); !n4.isNull(); n4 = n4.nextSibling() ) {
    QDomElement e = n4.toElement();
        
    if ( e.tagName() == "resourcetype" ) {
      if ( !e.namedItem( "vevent-collection" ).isNull() )
        return CalendarFolder;
      if ( !e.namedItem( "vtodo-collection" ).isNull() )
        return TasksFolder;
      if ( !e.namedItem( "vcard-collection" ).isNull() )
        return ContactsFolder;
      if ( !e.namedItem( "collection" ).isNull() ) 
        return Folder;
    }
  }
  return Unknown;

}

void FolderLister::slotListJobResult( KIO::Job *job )
{
  kdDebug() << "OpenGroupware::slotListJobResult(): " << endl;

  if ( job->error() ) {
    kdError() << "Unable to retrieve folders." << endl;
  } else {
    QDomDocument doc = mListEventsJob->response();

    kdDebug() << " Doc: " << doc.toString() << endl;

    bool firstRetrieve = mFolders.isEmpty();

    Entry::List newFolders( defaultFolders() );

    for ( Entry::List::Iterator it = newFolders.begin(); it != newFolders.end(); ++it ) {
      if ( firstRetrieve ) {
        (*it).active = true;
      } else {
        (*it).active = isActive( (*it).id );
      }
    }

    QDomElement docElement = doc.documentElement();
    QDomNode n;
    for( n = docElement.firstChild(); !n.isNull();
      n = n.nextSibling() ) {

      QDomElement ee1 = n.toElement();

      QDomNode n2 = n.namedItem( "propstat" );
      QDomNode n3 = n2.namedItem( "prop" );

      QString displayName;
      
      FolderType type = getFolderType( n3 );

      QDomNode n4;
      for( n4 = n3.firstChild(); !n4.isNull(); n4 = n4.nextSibling() ) {
        QDomElement e = n4.toElement();
        if ( e.tagName() == "displayname" ) displayName = e.text();
      }

      if ( ( mType == Calendar && 
              ( type == CalendarFolder || type == TasksFolder || 
                type == JournalsFolder ) ) ||
           ( type == ContactsFolder && mType == AddressBook ) ) {
        QDomNode n6 = n.namedItem( "href" );
        QDomElement e2 = n6.toElement();
        QString href = e2.text();
        
        Entry entry;
        entry.id = href;
        entry.name = displayName;
        entry.active = isActive( entry.id );
        
        newFolders.append( entry );
      
        kdDebug() << "FOLDER: " << displayName << endl;
      }
    }

    mFolders = newFolders;

    emit foldersRead();
  }

  mListEventsJob = 0;
}

#include "folderlister.moc"
