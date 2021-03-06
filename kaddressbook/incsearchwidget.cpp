/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "incsearchwidget.h"

#include <QtCore/QTimer>
#include <QtGui/QHBoxLayout>
#include <QtGui/QKeyEvent>
#include <QtGui/QLabel>

#include <kcombobox.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>

IncSearchWidget::IncSearchWidget( QWidget *parent, const char *name )
    : QWidget( parent )
{
  setObjectName( name );
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( 2 );

  mSearchText = new KLineEdit( this );
  mSearchText->setClearButtonShown(true);
  mSearchText->setClickMessage( i18nc( "Search for contacts in the address book", "Search" ) );
  mSearchText->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  mSearchText->setWhatsThis( i18n( "The incremental search<p>Enter some text here will start the search for the contact, which matches the search pattern best. The part of the contact, which will be used for matching, depends on the field selection.</p>" ) );
  layout->addWidget( mSearchText );

  QLabel *label = new QLabel( i18nc( "as in 'Search in:'", "&in:" ), this );
  label->setObjectName( "kde toolbar widget" );
  label->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
  layout->addWidget( label );

  mFieldCombo = new KComboBox( this );
  mFieldCombo->setEditable( false );
  layout->addWidget( mFieldCombo );
  label->setBuddy(mFieldCombo);

  mFieldCombo->setToolTip( i18n( "Select incremental search field" ) );
  mFieldCombo->setWhatsThis( i18n( "Here you can choose the field, which shall be used for incremental search." ) );

  mInputTimer = new QTimer( this );
  mInputTimer->setSingleShot( true );

  connect( mInputTimer, SIGNAL( timeout() ),
           SLOT( timeout() ) );
  connect( mSearchText, SIGNAL( textChanged( const QString& ) ),
           SLOT( announceDoSearch() ) );
  connect( mSearchText, SIGNAL( returnPressed() ),
           SLOT( announceDoSearch() ) );
  connect( mFieldCombo, SIGNAL( activated( const QString& ) ),
           SLOT( announceDoSearch() ) );
  connect( mSearchText, SIGNAL( clearButtonClicked() ),
           SLOT( announceDoSearch() ) );

  initFields();

  mSearchText->installEventFilter( this );

  setFocusProxy( mSearchText );
}

IncSearchWidget::~IncSearchWidget()
{
}

void IncSearchWidget::announceDoSearch()
{
  if ( mInputTimer->isActive() )
    mInputTimer->stop();

  mInputTimer->start( 0 );
}

void IncSearchWidget::timeout()
{
  emit doSearch( mSearchText->text() );
}

void IncSearchWidget::initFields()
{
  mFieldList = KABC::Field::allFields();

  mFieldCombo->clear();
  mFieldCombo->addItem( i18n( "Visible Fields" ) );
  mFieldCombo->addItem( i18n( "All Fields" ) );

  KABC::Field::List::ConstIterator it;
  for ( it = mFieldList.constBegin(); it != mFieldList.constEnd(); ++it )
    mFieldCombo->addItem( (*it)->label() );

  announceDoSearch();
}

KABC::Field::List IncSearchWidget::currentFields() const
{
  KABC::Field::List fieldList;

  if ( mFieldCombo->currentIndex() == 0 )
    fieldList = mViewFields;
  else if ( mFieldCombo->currentIndex() > 1 )
    fieldList.append( mFieldList[ mFieldCombo->currentIndex() - 2 ] );

  return fieldList;
}

void IncSearchWidget::setCurrentItem( int pos )
{
  mFieldCombo->setCurrentIndex( pos );
}

int IncSearchWidget::currentItem() const
{
  return mFieldCombo->currentIndex();
}

void IncSearchWidget::setViewFields( const KABC::Field::List &fields )
{
  mViewFields = fields;
}

void IncSearchWidget::clear()
{
  mSearchText->clear();
}

void IncSearchWidget::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Up ) {
    event->accept();
    emit scrollUp();
  } else if ( event->key() == Qt::Key_Down ) {
    event->accept();
    emit scrollDown();
  }
}

#include "incsearchwidget.moc"
