/*
  This file is part of KOrganizer.

  Copyright (c) 1997, 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
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

#include "kojournaleditor.h"
#include "koeditordetails.h"
#include "koeditorgeneraljournal.h"
#include "kohelper.h"
#include "koprefs.h"
#include "korganizer/baseview.h"

#include <KCal/CalendarLocal>
#include <KCal/IncidenceFormatter>

#include <KLocale>
#include <KMessageBox>

#include <QVBoxLayout>

KOJournalEditor::KOJournalEditor( Calendar *calendar, QWidget *parent )
  : KOIncidenceEditor( QString(), calendar, parent ),
    mJournal( 0 ), mCalendar( 0 )
{
}

KOJournalEditor::~KOJournalEditor()
{
  emit dialogClose( mJournal );
}

void KOJournalEditor::init()
{
  setupGeneral();
  setupAttendeesTab();

  connect( mGeneral, SIGNAL(openCategoryDialog()),
           SIGNAL(editCategories()) );
  connect( this, SIGNAL(updateCategoryConfig()),
           mGeneral, SIGNAL(updateCategoryConfig()) );

}

void KOJournalEditor::reload()
{
  if ( mJournal ) {
    readJournal( mJournal, QDate(), true );
  }
}

void KOJournalEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralJournal( mCalendar, this );

  QFrame *topFrame = new QFrame();
  addPage( topFrame, i18nc( "@title general journal settings", "General" ) );

  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    topLayout->setMargin( marginHint() );
  }
  topLayout->setSpacing( spacingHint() );

  mGeneral->initTitle( topFrame, topLayout );
  mGeneral->initDate( topFrame, topLayout );
  mGeneral->initDescription( topFrame, topLayout );
  mGeneral->initCategories( topFrame, topLayout );

  mGeneral->finishSetup();
}

void KOJournalEditor::editIncidence( Incidence *incidence, const QDate &date, Calendar *calendar )
{
  Journal *journal = dynamic_cast<Journal*>( incidence );
  if ( journal ) {
    init();

    mJournal = journal;
    mCalendar = calendar;
    readJournal( mJournal, date, false );
  }

  setCaption( i18nc( "@title:window",
                     "Edit Journal: %1",
                     IncidenceFormatter::resourceString( calendar, incidence ) ) );

}

void KOJournalEditor::newJournal()
{
  init();
  mJournal = 0;
  mCalendar = 0;
  loadDefaults();
  setCaption( i18nc( "@title:window", "New Journal" ) );
}

void KOJournalEditor::setTexts( const QString &summary, const QString &description,
                                bool richDescription )
{
  if ( description.isEmpty() && summary.contains( "\n" ) ) {
    mGeneral->setDescription( summary, false );
    int pos = summary.indexOf( "\n" );
    mGeneral->setSummary( summary.left( pos ) );
  } else {
    mGeneral->setSummary( summary );
    mGeneral->setDescription( description, richDescription );
  }
}

void KOJournalEditor::loadDefaults()
{
  setDate( QDate::currentDate() );
  setTime( QTime::currentTime() );
}

bool KOJournalEditor::processInput()
{
  if ( !validateInput() || !mChanger ) {
    return false;
  }

  if ( mJournal ) {
    bool rc = true;
    Journal *oldJournal = mJournal->clone();
    Journal *journal = mJournal->clone();

    fillJournal( journal );

    if ( *mJournal == *journal ) {
      // Don't do anything
    } else {
      fillJournal( mJournal );
      rc = mChanger->changeIncidence( oldJournal, mJournal, KOGlobals::NOTHING_MODIFIED, this );
    }
    delete journal;
    delete oldJournal;
    return rc;
  } else {
    mJournal = new Journal;
    mJournal->setOrganizer( Person( KOPrefs::instance()->fullName(),
                                    KOPrefs::instance()->email() ) );

    fillJournal( mJournal );

    if ( !mChanger->addIncidence( mJournal, this ) ) {
      delete mJournal;
      mJournal = 0;
      return false;
    }
  }

  return true;
}

void KOJournalEditor::deleteJournal()
{
  if ( mJournal ) {
    emit deleteIncidenceSignal( mJournal );
  }

  emit dialogClose( mJournal );
  reject();
}

void KOJournalEditor::setDate( const QDate &date )
{
  mGeneral->setDate( date );
}

void KOJournalEditor::setTime( const QTime &time )
{
  mGeneral->setTime( time );
}

bool KOJournalEditor::incidenceModified()
{
  Journal *newJournal = 0;
  Journal *oldJournal = 0;
  bool modified;

  if ( mJournal ) { // modification
    oldJournal = mJournal;
  } else { // new one
    oldJournal = &mInitialJournal;
  }

  newJournal = oldJournal->clone();
  fillJournal( newJournal );
  modified = !( *newJournal == *oldJournal );

  delete newJournal;

  return modified;
}

void KOJournalEditor::readJournal( Journal *journal, const QDate & date, bool tmpl )
{
  if ( !journal ) {
    return;
  }

  mGeneral->readJournal( journal, date, tmpl );
  mDetails->readIncidence( journal );
}

void KOJournalEditor::fillJournal( Journal *journal )
{
  mGeneral->fillJournal( journal );
  mDetails->fillIncidence( journal );
}

bool KOJournalEditor::validateInput()
{
  if ( !mGeneral->validateInput() ) {
    return false;
  }
  if ( !mDetails->validateInput() ) {
    return false;
  }
  return true;
}

void KOJournalEditor::modified()
{
  // Play dumb, just reload the Journal. This dialog has become so complicated
  // that there is no point in trying to be smart here...
  reload();
}

void KOJournalEditor::loadTemplate( CalendarLocal &cal )
{
  Journal::List journals = cal.journals();
  if ( journals.count() == 0 ) {
    KMessageBox::error( this, i18nc( "@info", "Template does not contain a valid journal." ) );
  } else {
    readJournal( journals.first(), QDate(), true );
  }
}

void KOJournalEditor::slotSaveTemplate( const QString &templateName )
{
  Journal *journal = new Journal;
  fillJournal( journal );
  saveAsTemplate( journal, templateName );
}

QStringList &KOJournalEditor::templates() const
{
  return KOPrefs::instance()->mJournalTemplates;
}

void KOJournalEditor::show()
{
  fillJournal( &mInitialJournal );
  KOIncidenceEditor::show();
}

#include "kojournaleditor.moc"
