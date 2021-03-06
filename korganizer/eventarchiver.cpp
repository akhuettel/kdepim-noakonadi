/*
  This file is part of KOrganizer.
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2004 David Faure <faure@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "eventarchiver.h"
#include "koprefs.h"

#include <kio/netaccess.h>
#include <kcal/icalformat.h>
#include <kcal/filestorage.h>
#include <kcal/calendarlocal.h>
#include <kcal/calendar.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <kmessagebox.h>

EventArchiver::EventArchiver( QObject *parent )
 : QObject( parent )
{
}

EventArchiver::~EventArchiver()
{
}

void EventArchiver::runOnce( Calendar *calendar, const QDate &limitDate, QWidget *widget )
{
  run( calendar, limitDate, widget, true, true );
}

void EventArchiver::runAuto( Calendar *calendar, QWidget *widget, bool withGUI )
{
  QDate limitDate( QDate::currentDate() );
  int expiryTime = KOPrefs::instance()->mExpiryTime;
  switch ( KOPrefs::instance()->mExpiryUnit ) {
  case KOPrefs::UnitDays: // Days
    limitDate = limitDate.addDays( -expiryTime );
    break;
  case KOPrefs::UnitWeeks: // Weeks
    limitDate = limitDate.addDays( -expiryTime * 7 );
    break;
  case KOPrefs::UnitMonths: // Months
    limitDate = limitDate.addMonths( -expiryTime );
    break;
  default:
    return;
  }
  run( calendar, limitDate, widget, withGUI, false );
}

void EventArchiver::run( Calendar *calendar, const QDate &limitDate, QWidget *widget,
                         bool withGUI, bool errorIfNone )
{
  // We need to use rawEvents, otherwise events hidden by filters will not be archived.
  Incidence::List incidences;
  Event::List events;
  Todo::List todos;
  Journal::List journals;

  if ( KOPrefs::instance()->mArchiveEvents ) {
    events = calendar->rawEvents(
      QDate( 1769, 12, 1 ),
      // #29555, also advertised by the "limitDate not included" in the class docu
      limitDate.addDays( -1 ),
      KOPrefs::instance()->timeSpec(),
      true );
  }
  if ( KOPrefs::instance()->mArchiveTodos ) {
    Todo::List t = calendar->rawTodos();
    Todo::List::ConstIterator it;
    for ( it = t.constBegin(); it != t.constEnd(); ++it ) {
      if ( isSubTreeComplete( *it, limitDate ) ) {
        todos.append( *it );
      }
    }
  }

  incidences = Calendar::mergeIncidenceList( events, todos, journals );

  kDebug() << "archiving incidences before" << limitDate
           << " ->" << incidences.count() <<" incidences found.";
  if ( incidences.isEmpty() ) {
    if ( withGUI && errorIfNone ) {
      KMessageBox::information( widget,
                                i18n( "There are no items before %1",
                                      KGlobal::locale()->formatDate( limitDate ) ),
                                "ArchiverNoIncidences" );
    }
    return;
  }

  switch ( KOPrefs::instance()->mArchiveAction ) {
  case KOPrefs::actionDelete:
    deleteIncidences( calendar, limitDate, widget, incidences, withGUI );
    break;
  case KOPrefs::actionArchive:
    archiveIncidences( calendar, limitDate, widget, incidences, withGUI );
    break;
  }
}

void EventArchiver::deleteIncidences( Calendar *calendar, const QDate &limitDate, QWidget *widget,
                                      const Incidence::List &incidences, bool withGUI )
{
  QStringList incidenceStrs;
  Incidence::List::ConstIterator it;
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    incidenceStrs.append( (*it)->summary() );
  }

  if ( withGUI ) {
    int result = KMessageBox::warningContinueCancelList(
      widget,
      i18n( "Delete all items before %1 without saving?\n"
            "The following items will be deleted:",
            KGlobal::locale()->formatDate( limitDate ) ),
      incidenceStrs,
      i18n( "Delete Old Items" ), KStandardGuiItem::del() );
    if ( result != KMessageBox::Continue ) {
      return;
    }
  }
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    calendar->deleteIncidence( *it );
  }
  emit eventsDeleted();
}

void EventArchiver::archiveIncidences( Calendar *calendar, const QDate &limitDate, QWidget *widget,
                                       const Incidence::List &incidences, bool withGUI )
{
  Q_UNUSED( limitDate );
  Q_UNUSED( withGUI );
  FileStorage storage( calendar );

  // Save current calendar to disk
  KTemporaryFile tmpFile;
  tmpFile.open();
  storage.setFileName( tmpFile.fileName() );
  if ( !storage.save() ) {
    kDebug() << "Can't save calendar to temp file";
    return;
  }

  // Duplicate current calendar by loading in new calendar object
  CalendarLocal archiveCalendar( KOPrefs::instance()->timeSpec() );

  FileStorage archiveStore( &archiveCalendar );
  archiveStore.setFileName( tmpFile.fileName() );
  ICalFormat *format = new ICalFormat();
  archiveStore.setSaveFormat( format );
  if ( !archiveStore.load() ) {
    kDebug() << "Can't load calendar from temp file";
    return;
  }

  // Strip active events from calendar so that only events to be archived
  // remain. This is not really efficient, but there is no other easy way.
  QStringList uids;
  Incidence::List allIncidences = archiveCalendar.rawIncidences();
  Incidence::List::ConstIterator it;
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    uids << (*it)->uid();
  }
  for ( it = allIncidences.constBegin(); it != allIncidences.constEnd(); ++it ) {
    if ( !uids.contains( (*it)->uid() ) ) {
      archiveCalendar.deleteIncidence( *it );
    }
  }

  // Get or create the archive file
  KUrl archiveURL( KOPrefs::instance()->mArchiveFile );
  QString archiveFile;

  if ( KIO::NetAccess::exists( archiveURL, KIO::NetAccess::SourceSide, widget ) ) {
    if( !KIO::NetAccess::download( archiveURL, archiveFile, widget ) ) {
      kDebug() << "Can't download archive file";
      return;
    }
    // Merge with events to be archived.
    archiveStore.setFileName( archiveFile );
    if ( !archiveStore.load() ) {
      kDebug() << "Can't merge with archive file";
      return;
    }
  } else {
    archiveFile = tmpFile.fileName();
  }

  // Save archive calendar
  if ( !archiveStore.save() ) {
    QString errmess;
    if ( format->exception() ) {
      errmess = format->exception()->message();
    } else {
      errmess = i18nc( "save failure cause unknown", "Reason unknown" );
    }
    KMessageBox::error( widget, i18n( "Cannot write archive file %1. %2",
                                      archiveStore.fileName(), errmess ) );
    return;
  }

  // Upload if necessary
  KUrl srcUrl;
  srcUrl.setPath( archiveFile );
  if ( srcUrl != archiveURL ) {
    if ( !KIO::NetAccess::upload( archiveFile, archiveURL, widget ) ) {
      KMessageBox::error( widget, i18n( "Cannot write archive. %1",
                                        KIO::NetAccess::lastErrorString() ) );
      return;
    }
  }

  KIO::NetAccess::removeTempFile( archiveFile );

  // Delete archived events from calendar
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    calendar->deleteIncidence( *it );
  }
  emit eventsDeleted();
}

bool EventArchiver::isSubTreeComplete( const Todo *todo, const QDate &limitDate, QStringList checkedUids ) const
{
  if ( !todo || !todo->isCompleted() || todo->completed().date() >= limitDate ) {
    return false;
  }

  // This QList is only to prevent infinit recursion
  if ( checkedUids.contains( todo->uid() ) ) {
    // Probably will never happen, calendar.cpp checks for this
    kWarning() << "To-do hierarchy loop detected!";
    return false;
  }

  checkedUids.append( todo->uid() );

  foreach( const Incidence *i, todo->relations() ) {

    if ( i->type() == "Todo" ) {
      const Todo *t = static_cast<const Todo*>( i );
      if ( !isSubTreeComplete( t, limitDate, checkedUids ) ) {
        return false;
      }      
    }
  }

  return true;
}

#include "eventarchiver.moc"
