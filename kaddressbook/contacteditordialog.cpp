/*
    This file is part of KContactManager.

    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

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
*/

#include "contacteditordialog.h"

#include "collectioncombobox.h"
#include "collectionfiltermodel.h"
#include "contacteditor.h"
#include "contactitemeditor.h"

#include <akonadi/descendantsproxymodel.h>
#include <akonadi/item.h>

#include <kabc/addressee.h>

#include <klocale.h>

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

ContactEditorDialog::ContactEditorDialog( Mode mode, QAbstractItemModel *collectionModel, QWidget *parent )
  : KDialog( parent )
{
  setCaption( mode == CreateMode ? i18n( "New Contact" ) : i18n( "Edit Contact" ) );
  setButtons( Ok | Cancel );

  QWidget *mainWidget = new QWidget( this );
  setMainWidget( mainWidget );

  QGridLayout *layout = new QGridLayout( mainWidget );

  mEditor = new ContactItemEditor( mode == CreateMode ? ContactItemEditor::CreateMode : ContactItemEditor::EditMode,
                                   new ContactEditor(), this );

  if ( mode == CreateMode ) {
    QLabel *label = new QLabel( i18n( "Add to:" ), mainWidget );

    // flatten the collection tree structure to a collection list
    Akonadi::DescendantsProxyModel *descendantModel = new Akonadi::DescendantsProxyModel( this );
    descendantModel->setSourceModel( collectionModel );

    // filter for collections that support contacts
    CollectionFilterModel *filterModel = new CollectionFilterModel( this );
    filterModel->addContentMimeTypeFilter( KABC::Addressee::mimeType() );
    filterModel->setRightsFilter( Akonadi::Collection::CanCreateItem );
    filterModel->setSourceModel( descendantModel );

    KABC::CollectionComboBox *box = new KABC::CollectionComboBox( mainWidget );
    box->setModel( filterModel );

    layout->addWidget( label, 0, 0 );
    layout->addWidget( box, 0, 1 );

    connect( box, SIGNAL( selectionChanged( const Akonadi::Collection& ) ),
             mEditor, SLOT( setDefaultCollection( const Akonadi::Collection& ) ) );

    mEditor->setDefaultCollection( box->selectedCollection() );
  }

  layout->addWidget( mEditor, 1, 0, 1, 2 );
  layout->setColumnStretch( 1, 1 );

  connect( mEditor, SIGNAL( contactStored( const Akonadi::Item& ) ),
           this, SIGNAL( contactStored( const Akonadi::Item& ) ) );

  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOkClicked() ) );
  connect( this, SIGNAL( cancelClicked() ), this, SLOT( slotCancelClicked() ) );

  setInitialSize( QSize( 550, 650 ) );
}

ContactEditorDialog::~ContactEditorDialog()
{
}

void ContactEditorDialog::setContact( const Akonadi::Item &contact )
{
  mEditor->loadContact( contact );
}

void ContactEditorDialog::slotOkClicked()
{
  mEditor->saveContact();

  accept();
}

void ContactEditorDialog::slotCancelClicked()
{
  reject();
}

#include "contacteditordialog.moc"
