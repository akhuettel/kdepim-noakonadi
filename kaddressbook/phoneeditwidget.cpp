/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>                   
                                                                        
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qlayout.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qbuttongroup.h>

#include <kbuttonbox.h>
#include <klistview.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <kabc/phonenumber.h>

#include "typecombo.h"

#include "phoneeditwidget.h"

PhoneEditWidget::PhoneEditWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *layout = new QGridLayout( this, 5, 2 );
  layout->setSpacing( KDialog::spacingHint() );
  
  mPrefCombo = new TypeCombo( mPhoneList, this );
  mPrefEdit = new KLineEdit( this );
  mPrefCombo->setLineEdit( mPrefEdit );
  layout->addWidget( mPrefCombo, 0, 0 );
  layout->addWidget( mPrefEdit, 0, 1 );

  mSecondCombo = new TypeCombo( mPhoneList, this );
  mSecondEdit = new KLineEdit( this );
  mSecondCombo->setLineEdit( mSecondEdit );
  layout->addWidget( mSecondCombo, 1, 0 );
  layout->addWidget( mSecondEdit, 1, 1 );

  mThirdCombo = new TypeCombo( mPhoneList, this );
  mThirdEdit = new KLineEdit( this );
  mThirdCombo->setLineEdit( mThirdEdit );
  layout->addWidget( mThirdCombo, 2, 0 );
  layout->addWidget( mThirdEdit, 2, 1 );

  mFourthCombo = new TypeCombo( mPhoneList, this );
  mFourthEdit = new KLineEdit( this );
  mFourthCombo->setLineEdit( mFourthEdit );
  layout->addWidget( mFourthCombo, 3, 0 );
  layout->addWidget( mFourthEdit, 3, 1 );

  // Four numbers don't fit in the current dialog
  mFourthCombo->hide();
  mFourthEdit->hide();

  QPushButton *editButton = new QPushButton( i18n( "Edit Phone Numbers..." ),
                                             this );
  layout->addMultiCellWidget( editButton, 4, 4, 0, 1 );

  connect( mPrefEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotPrefEditChanged() ) );
  connect( mSecondEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotSecondEditChanged() ) );
  connect( mThirdEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotThirdEditChanged() ) );
  connect( mFourthEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotFourthEditChanged() ) );

  connect( editButton, SIGNAL( clicked() ), SLOT( edit() ) );

  connect( mPrefCombo, SIGNAL( activated( int ) ),
           SLOT( updatePrefEdit() ) );
  connect( mSecondCombo, SIGNAL( activated( int ) ),
           SLOT( updateSecondEdit() ) );
  connect( mThirdCombo, SIGNAL( activated( int ) ),
           SLOT( updateThirdEdit() ) );
  connect( mFourthCombo, SIGNAL( activated( int ) ),
           SLOT( updateFourthEdit() ) );
}

PhoneEditWidget::~PhoneEditWidget()
{
}

void PhoneEditWidget::insertType( const KABC::PhoneNumber::List &list,
                                  int type )
{
  uint i;
  for ( i = 0; i < list.count(); ++i ) {
    if ( list[ i ].type() == type ) {
      mPhoneList.append( list[ i ] );
      break;
    }
  }
  if ( i == list.count() ) {
    mPhoneList.append( PhoneNumber( "", type ) );
  }
}

void PhoneEditWidget::setPhoneNumbers( const KABC::PhoneNumber::List &list )
{
  mPhoneList.clear();

  kdDebug() << "PhoneEditWidget::setPhoneNumbers(): count: "
            << list.count() << endl;

  QValueList<int> defaultTypes;
  insertType( list, KABC::PhoneNumber::Home );
  insertType( list, KABC::PhoneNumber::Work );
  insertType( list, KABC::PhoneNumber::Cell );
  insertType( list, KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
  insertType( list, KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );

  uint i;
  for ( i = 0; i < list.count(); ++i ) {
    uint j;
    for( j = 0; j < mPhoneList.count(); ++j ) {
      if ( list[ i ].id() == mPhoneList[ j ].id() ) break;
    }
    if ( j == mPhoneList.count() ) {
      mPhoneList.append( list[ i ] );
    }
  }

  updateCombos();

  mPrefCombo->selectType( PhoneNumber::Home );
  mSecondCombo->selectType( PhoneNumber::Work );
  mThirdCombo->selectType( PhoneNumber::Cell );
  mFourthCombo->selectType( PhoneNumber::Work | PhoneNumber::Fax );

  updateLineEdits();
}

void PhoneEditWidget::updateLineEdits()
{
  updatePrefEdit();
  updateSecondEdit();
  updateThirdEdit();
  updateFourthEdit();
}

void PhoneEditWidget::updateCombos()
{
  mPrefCombo->updateTypes();
  mSecondCombo->updateTypes();
  mThirdCombo->updateTypes();
  mFourthCombo->updateTypes();
}

const KABC::PhoneNumber::List &PhoneEditWidget::phoneNumbers()
{
  return mPhoneList;
}

void PhoneEditWidget::edit()
{
  PhoneEditDialog dlg( mPhoneList, this );
  
  if ( dlg.exec() ) {
    mPhoneList = dlg.phoneNumbers();
    updateCombos();
    emit modified();
  }
}

void PhoneEditWidget::updatePrefEdit()
{
  updateEdit( mPrefCombo );
}

void PhoneEditWidget::updateSecondEdit()
{
  updateEdit( mSecondCombo );
}

void PhoneEditWidget::updateThirdEdit()
{
  updateEdit( mThirdCombo );
}

void PhoneEditWidget::updateFourthEdit()
{
  updateEdit( mFourthCombo );
}

void PhoneEditWidget::updateEdit( TypeCombo *combo )
{
//  kdDebug() << "updateEdit()" << endl;

  QLineEdit *edit = combo->lineEdit();
  if ( !edit ) {
    return;
  }

#if 0
  if ( edit == mPrefEdit ) kdDebug() << " prefEdit" << endl;
  if ( edit == mSecondEdit ) kdDebug() << " secondEdit" << endl;
  if ( edit == mThirdEdit ) kdDebug() << " thirdEdit" << endl;
  if ( edit == mFourthEdit ) kdDebug() << " fourthEdit" << endl;
#endif

  PhoneNumber::List::Iterator it = combo->selectedElement();
  if ( it != mPhoneList.end() ) {
    edit->setText( (*it).number() );
  } else {
    kdDebug() << "PhoneEditWidget::updateEdit(): no selected element" << endl;
  }
}

void PhoneEditWidget::slotPrefEditChanged()
{
  updatePhoneNumber( mPrefCombo );
}

void PhoneEditWidget::slotSecondEditChanged()
{
  updatePhoneNumber( mSecondCombo );
}

void PhoneEditWidget::slotThirdEditChanged()
{
  updatePhoneNumber( mThirdCombo );
}

void PhoneEditWidget::slotFourthEditChanged()
{
  updatePhoneNumber( mFourthCombo );
}

void PhoneEditWidget::updatePhoneNumber( TypeCombo *combo )
{
//  kdDebug() << "PhoneEditWidget::updatePhoneNumber()" << endl;

  QLineEdit *edit = combo->lineEdit();
  if ( !edit ) return;

  PhoneNumber::List::Iterator it = combo->selectedElement();
  if ( it != mPhoneList.end() ) {
    (*it).setNumber( edit->text() );
  } else {
    kdDebug() << "PhoneEditWidget::updatePhoneNumber(): no selected element"
              << endl;
  }

  updateOtherEdit( combo, mPrefCombo );
  updateOtherEdit( combo, mSecondCombo );
  updateOtherEdit( combo, mThirdCombo );
  updateOtherEdit( combo, mFourthCombo );

  emit modified();
}

void PhoneEditWidget::updateOtherEdit( TypeCombo *combo, TypeCombo *otherCombo )
{
//  kdDebug() << "PhoneEditWidget::updateOtherEdit()" << endl;

  if ( combo == otherCombo ) return;

  if ( combo->currentItem() == otherCombo->currentItem() ) {
    updateEdit( otherCombo );
  }
}

///////////////////////////////////////////
// PhoneEditDialog

class PhoneViewItem : public QListViewItem
{
public:
  PhoneViewItem( QListView *parent, const KABC::PhoneNumber &number );

  void setPhoneNumber( const KABC::PhoneNumber &number )
  {
    mPhoneNumber = number;
    makeText();
  }

  QString key() { return mPhoneNumber.id(); }
  QString country() { return ""; }
  QString region() { return ""; }
  QString number() { return ""; }

  KABC::PhoneNumber phoneNumber() { return mPhoneNumber; }

private:
  void makeText();

  KABC::PhoneNumber mPhoneNumber;
};

PhoneViewItem::PhoneViewItem( QListView *parent, const KABC::PhoneNumber &number )
  : QListViewItem( parent ), mPhoneNumber( number )
{
  makeText();
}

void PhoneViewItem::makeText()
{
  /**
   * Will be used in future versions of kaddressbook/libkabc

    setText( 0, mPhoneNumber.country() );
    setText( 1, mPhoneNumber.region() );
    setText( 2, mPhoneNumber.number() );
    setText( 3, mPhoneNumber.typeLabel() );
   */

  setText( 0, mPhoneNumber.number() );
  setText( 1, mPhoneNumber.typeLabel() );
}

PhoneEditDialog::PhoneEditDialog( const KABC::PhoneNumber::List &list, QWidget *parent, const char *name )
  : KDialogBase( KDialogBase::Plain, i18n( "Edit Phone Numbers" ),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, true)
{
  mPhoneNumberList = list;

  QWidget *page = plainPage();

  QGridLayout *layout = new QGridLayout( page, 1, 2 );
  layout->setSpacing( spacingHint() );

  mListView = new KListView( page );
  mListView->setAllColumnsShowFocus( true );
  mListView->addColumn( i18n( "Number" ) );
  mListView->addColumn( i18n( "Type" ) );
  
  KButtonBox *buttonBox = new KButtonBox( page, Vertical );

  buttonBox->addButton( i18n( "&Add..." ), this, SLOT( slotAddPhoneNumber() ) );
  mEditButton = buttonBox->addButton( i18n( "&Edit..." ), this, SLOT( slotEditPhoneNumber() ) );
  mEditButton->setEnabled( false );
  mRemoveButton = buttonBox->addButton( i18n( "&Remove" ), this, SLOT( slotRemovePhoneNumber() ) );
  mRemoveButton->setEnabled( false );
  buttonBox->layout();

  layout->addWidget( mListView, 0, 0 );
  layout->addWidget( buttonBox, 0, 1 );

  connect( mListView, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()) );

  KABC::PhoneNumber::List::Iterator it;
  for ( it = mPhoneNumberList.begin(); it != mPhoneNumberList.end(); ++it )
    new PhoneViewItem( mListView, *it );
}

PhoneEditDialog::~PhoneEditDialog()
{
}

void PhoneEditDialog::slotAddPhoneNumber()
{
  KABC::PhoneNumber tmp( "", 0 );
  PhoneTypeDialog dlg( tmp, this );
  
  if ( dlg.exec() ) {
    KABC::PhoneNumber phoneNumber = dlg.phoneNumber();
    mPhoneNumberList.append( phoneNumber );
    new PhoneViewItem( mListView, phoneNumber );
  }
}

void PhoneEditDialog::slotRemovePhoneNumber()
{
  PhoneViewItem *item = static_cast<PhoneViewItem*>( mListView->currentItem() );
  if ( !item )
    return;

  mPhoneNumberList.remove( item->phoneNumber() );
  QListViewItem *currItem = mListView->currentItem();
  mListView->takeItem( currItem );
  delete currItem;
}

void PhoneEditDialog::slotEditPhoneNumber()
{
  PhoneViewItem *item = static_cast<PhoneViewItem*>( mListView->currentItem() );
  if ( !item )
    return;

  PhoneTypeDialog dlg( item->phoneNumber(), this );
  
  if ( dlg.exec() ) {
    slotRemovePhoneNumber();
    KABC::PhoneNumber phoneNumber = dlg.phoneNumber();
    mPhoneNumberList.append( phoneNumber );
    new PhoneViewItem( mListView, phoneNumber );
  }
}

void PhoneEditDialog::slotSelectionChanged()
{
  bool state = ( mListView->currentItem() != 0 );

  mRemoveButton->setEnabled( state );
  mEditButton->setEnabled( state );
}

const KABC::PhoneNumber::List &PhoneEditDialog::phoneNumbers()
{
  return mPhoneNumberList;
}
 
///////////////////////////////////////////
// PhoneTypeDialog
PhoneTypeDialog::PhoneTypeDialog( const KABC::PhoneNumber &phoneNumber,
                               QWidget *parent, const char *name)
  : KDialogBase( KDialogBase::Plain, i18n( "Edit Phone Number" ),
                KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                parent, name, true), mPhoneNumber( phoneNumber )
{
  QWidget *page = plainPage();
  QLabel *label = 0;
  QGridLayout *layout = new QGridLayout( page, 3, 2, marginHint(), spacingHint() );

  label = new QLabel( i18n( "Number:" ), page );
  layout->addWidget( label, 0, 0 );
  mNumber = new KLineEdit( page );
  layout->addWidget( mNumber, 0, 1 );

  mPreferredBox = new QCheckBox( i18n( "This is the preferred phone number" ), page );
  layout->addMultiCellWidget( mPreferredBox, 1, 1, 0, 1 );

  mGroup = new QButtonGroup( 2, Horizontal, i18n( "Types" ), page );
  layout->addMultiCellWidget( mGroup, 2, 2, 0, 1 );

  // fill widgets
  mNumber->setText( mPhoneNumber.number() );

  mTypeList = KABC::PhoneNumber::typeList();
  mTypeList.remove( KABC::PhoneNumber::Pref );

  KABC::PhoneNumber::TypeList::Iterator it;
  for ( it = mTypeList.begin(); it != mTypeList.end(); ++it )
    new QCheckBox( KABC::PhoneNumber::typeLabel( *it ), mGroup );

  for ( int i = 0; i < mGroup->count(); ++i ) {
    int type = mPhoneNumber.type();
    QCheckBox *box = (QCheckBox*)mGroup->find( i );
    box->setChecked( type & mTypeList[ i ] );
  }

  mPreferredBox->setChecked( mPhoneNumber.type() & KABC::PhoneNumber::Pref );
}

KABC::PhoneNumber PhoneTypeDialog::phoneNumber()
{
  mPhoneNumber.setNumber( mNumber->text() );

  int type = 0;
  for ( int i = 0; i < mGroup->count(); ++i ) {
    QCheckBox *box = (QCheckBox*)mGroup->find( i );
    if ( box->isChecked() )
      type += mTypeList[ i ];
  }

  if ( mPreferredBox->isChecked() )
    mPhoneNumber.setType( type | KABC::PhoneNumber::Pref );
  else
    mPhoneNumber.setType( type & ~KABC::PhoneNumber::Pref );

  return mPhoneNumber;
}


#include "phoneeditwidget.moc"
