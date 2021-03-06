/*
  This file is part of KOrganizer.

  Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "resourceview.h"
#include "koprefs.h"

#include <KCal/CalendarResources>
#include <KCal/ResourceCalendar>
#include <KResources/ConfigDialog>

#include <KColorCollection>
#include <KColorDialog>
#include <KDebug>
#include <KDialog>
#include <KInputDialog>
#include <KLocale>
#include <KMessageBox>
#include <KRandom>

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPixmap>
#include <QToolButton>
#include <QVBoxLayout>

#include <QDBusInterface>
#include <QDBusReply>

ResourceViewFactory::ResourceViewFactory( KCal::CalendarResources *calendar,
                                          CalendarView *view )
  : mCalendar( calendar ), mView( view ), mResourceView( 0 )
{
}

CalendarViewExtension *ResourceViewFactory::create( QWidget *parent )
{
  mResourceView = new ResourceView( mCalendar, parent );

  QObject::connect( mResourceView, SIGNAL(resourcesChanged()),
                    mView, SLOT(resourcesChanged()) );
  QObject::connect( mResourceView, SIGNAL(resourcesChanged()),
                    mView, SLOT(updateCategories()) );

  QObject::connect( mCalendar,
                    SIGNAL(signalResourceAdded(ResourceCalendar *)),
                    mResourceView,
                    SLOT(addResourceItem(ResourceCalendar *)) );
  QObject::connect( mCalendar,
                    SIGNAL(signalResourceModified(ResourceCalendar *)),
                    mResourceView,
                    SLOT(updateResourceItem(ResourceCalendar *)) );
  QObject::connect( mCalendar, SIGNAL(signalResourceAdded(ResourceCalendar *)),
                    mView, SLOT(updateCategories()) );
  QObject::connect( mCalendar, SIGNAL(signalResourceModified(ResourceCalendar *)),
                    mView, SLOT(updateCategories()) );

  return mResourceView;
}

ResourceView *ResourceViewFactory::resourceView() const
{
  return mResourceView;
}

ResourceItem::ResourceItem( ResourceCalendar *resource, ResourceView *view,
                            QTreeWidget *parent )
  : QTreeWidgetItem( parent ),
    mResource( resource ), mView( view ), mBlockStateChange( false ),
    mIsSubresource( false ), mResourceIdentifier( QString() ),
    mSubItemsCreated( false ), mIsStandardResource( false ),
    mIsReloading( false )
{
  setFlags( flags() | Qt::ItemIsUserCheckable );

  setText( 0, resource->resourceName() );
  mResourceColor = KOPrefs::instance()->resourceColor( resource->identifier() );
  setGuiState();

  if ( mResource->isActive() ) {
    createSubresourceItems();
  }
}

void ResourceItem::createSubresourceItems()
{
  const QStringList subresources = mResource->subresources();
  if ( !subresources.isEmpty() ) {
    setExpanded( true );
    // This resource has subresources
    QStringList::ConstIterator it;
    for ( it = subresources.constBegin(); it != subresources.constEnd(); ++it ) {
      if ( !mView->findItemByIdentifier( *it ) ) {
        new ResourceItem( mResource, *it, mResource->labelForSubresource( *it ),
                          mView, this );
      }
    }
  }
  mSubItemsCreated = true;
}

bool ResourceItem::useColors() const
{
  // assign a color, but only if this is a resource that actually
  // hold items at top level
  return ( KOPrefs::instance()->agendaViewColors() != KOPrefs::CategoryOnly ||
           KOPrefs::instance()->monthViewColors()  != KOPrefs::MonthItemCategoryOnly )  &&
         ( mIsSubresource || ( !mIsReloading && mResource->subresources().isEmpty() ) ||
           !mResource->canHaveSubresources() );
}

QVariant ResourceItem::data( int column, int role ) const
{
  if ( column == 0 &&
       role == Qt::DecorationRole &&
       mResourceColor.isValid() &&
       useColors() ) {
    return QVariant( mResourceColor );
  } else {
    return QTreeWidgetItem::data( column, role );
  }
}

ResourceItem::ResourceItem( KCal::ResourceCalendar *resource,
                            const QString &sub, const QString &label,
                            ResourceView *view, ResourceItem *parent )

  : QTreeWidgetItem( parent ), mResource( resource ),
    mView( view ), mBlockStateChange( false ), mIsSubresource( true ),
    mSubItemsCreated( false ), mIsStandardResource( false ), mActive( false ),
    mIsReloading( false )
{
  setFlags( flags() | Qt::ItemIsUserCheckable );
  setText( 0, label );
  mResourceColor = KOPrefs::instance()->resourceColor( sub );
  mResourceIdentifier = sub;
  setGuiState();

  treeWidget()->setRootIsDecorated( true );
}

void ResourceItem::setGuiState()
{
  mBlockStateChange = true;
  if ( mIsSubresource ) {
    setOn( mResource->subresourceActive( mResourceIdentifier ) );
  } else {
    setOn( mResource->isActive() );
  }
  mBlockStateChange = false;
}

void ResourceItem::setOn( bool checked )
{
  if ( checked ) {
    setCheckState( 0, Qt::Checked );
  } else {
    setCheckState( 0, Qt::Unchecked );
  }
  mActive = checked;
}

void ResourceItem::stateChange( bool active )
{
  if ( mActive == active ) {
    return;
  }

  if ( mBlockStateChange ) {
    return;
  }

  if ( mIsSubresource ) {
    mResource->setSubresourceActive( mResourceIdentifier, active );
  } else {
    if ( active ) {
      if ( mResource->load() ) {
        mResource->setActive( true );
        if ( !mSubItemsCreated ) {
          createSubresourceItems();
        }
      }
    } else {
      // mView->requestClose must be called before mResource->save() because
      // save causes closeResource do be called.
      mView->requestClose( mResource );
      if ( mResource->save() ) {
        mResource->setActive( false );
      }
    }

    setExpanded( mResource->isActive() && childCount() > 0 );
  }

  setGuiState();
  mView->emitResourcesChanged();
}

void ResourceItem::update()
{
  setGuiState();
}

void ResourceItem::setResourceColor( QColor &color )
{
  mResourceColor = color ;
}

void ResourceItem::setStandardResource( bool std )
{
  QFont font = qvariant_cast<QFont>( data( 0, Qt::FontRole ) );
  font.setBold( std );
  setData( 0, Qt::FontRole, font );
}

ResourceView::ResourceView( KCal::CalendarResources *calendar, QWidget *parent )
  : CalendarViewExtension( parent ), mCalendar( calendar )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  QHBoxLayout *buttonBox = new QHBoxLayout();
  buttonBox->setSpacing( KDialog::spacingHint() );
  topLayout->addLayout( buttonBox );

  mListView = new QTreeWidget( this );
  mListView->setWhatsThis(
    i18nc( "@info:whatsthis",
           "This list shows all the calendars currently known to KOrganizer. "
           "Use the associated checkboxes to make a calendar active or inactive. "
           "Use the context menu to add, remove or edit calendars in the list."
           "<p>Events, journal entries and to-dos are retrieved and stored "
           "from their respective calendars. Calendars can be accessed from "
           "groupware servers, local files, etc...</p>"
           "<p>If you have more than one active calendar, you will be "
           "prompted for which calendar to store new items into, unless "
           "configured to always store to the default calendar.</p>" ) );
  mListView->setRootIsDecorated( false );
  mListView->setHeaderLabel( i18n( "Calendars" ) );
  mListView->header()->hide();
  topLayout->addWidget( mListView );

  mSelectedParent = 0;

  connect( mListView, SIGNAL(itemDoubleClicked(QTreeWidgetItem *,int)),
           SLOT(editResource()) );
  connect( mListView, SIGNAL(itemClicked(QTreeWidgetItem *,int)),
           SLOT(slotItemClicked(QTreeWidgetItem *,int)) );
  connect( mListView, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
           SLOT(currentChanged()) );

  mListView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mListView, SIGNAL(customContextMenuRequested(const QPoint &)),
           SLOT(showContextMenu(const QPoint &)) );

  QLabel *calLabel = new QLabel( i18n( "Calendars" ), this );
  buttonBox->addWidget( calLabel );
  buttonBox->addStretch( 1 );

  mAddButton = new QToolButton( this );
  mAddButton->setIcon( KIcon( "list-add" ) );
  buttonBox->addWidget( mAddButton );
  mAddButton->setToolTip( i18nc( "@info:tooltip", "Add calendar" ) );
  mAddButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Press this button to add a new calendar to KOrganizer. "
           "<p>Events, journal entries and to-dos are retrieved and stored "
           "from their respective calendars. Calendars can be accessed from "
           "groupware servers, local files, etc...</p>"
           "<p>If you have more than one active calendar, you will be "
           "prompted for which calendar to store new items into, unless "
           "configured to always store to the default calendar.</p>" ) );
  mEditButton = new QToolButton( this );
  mEditButton->setIcon( KIcon( "document-properties" ) );
  buttonBox->addWidget( mEditButton );
  mEditButton->setToolTip( i18nc( "@info:tooltip", "Edit calendar settings" ) );
  mEditButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Press this button to edit the calendar currently "
           "selected in the list above." ) );
  mDeleteButton = new QToolButton( this );
  mDeleteButton->setIcon( KIcon( "edit-delete" ) );
  buttonBox->addWidget( mDeleteButton );
  mDeleteButton->setToolTip( i18nc( "@info:tooltip", "Remove calendar" ) );
  mDeleteButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Press this button to delete the calendar currently "
           "selected in the list above." ) );
  mDeleteButton->setDisabled( true );
  mEditButton->setDisabled( true );

  connect( mAddButton, SIGNAL( clicked() ), SLOT( slotAddButtonClicked() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), SLOT( removeResource() ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( editResource() ) );

  setMinimumHeight( 50 );
  mListView->setSortingEnabled( true );

  updateView();
}

ResourceView::~ResourceView()
{
}

void ResourceView::updateView()
{
  mListView->clear();

  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();

  KCal::CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    addResourceItem( *it, false );
  }

  mListView->sortItems( 0, Qt::AscendingOrder );

  emit emitResourcesChanged();
}

void ResourceView::emitResourcesChanged()
{
  mCalendar->resourceManager()->writeConfig();
  emit resourcesChanged();
}

void ResourceView::slotAddButtonClicked()
{
  if ( !mListView->selectedItems().isEmpty() ) {
    mSelectedParent = static_cast<ResourceItem*>( mListView->selectedItems().first() );
  } else {
    mSelectedParent = 0;
  }

  addResource();
}

void ResourceView::addResource()
{
  bool ok = false;
  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  ResourceItem *i = mSelectedParent;

  if ( i && ( i->isSubresource() || i->resource()->canHaveSubresources() ) ) {
    const QString folderName =
      KInputDialog::getText( i18n( "Add Calendar Folder" ),
                             i18n( "Please enter a name for the new calendar folder" ),
                             QString(), &ok, this );
    if ( !ok ) {
      return;
    }

    const QString parentId = i->isSubresource() ? i->resourceIdentifier() : QString();
    if ( !i->resource()->addSubresource( folderName, parentId ) ) {
      KMessageBox::error(
        this,
        i18n( "<qt>Unable to create the calendar folder <b>%1</b>.</qt>",
              folderName ) );
    }
    return;
  }

  QStringList types = manager->resourceTypeNames();
  QStringList descs = manager->resourceTypeDescriptions();
  QString desc =
    KInputDialog::getItem( i18n( "Calendar Configuration" ),
                           i18n( "Please select the type of the new calendar:" ),
                           descs, 0, false, &ok, this );
  if ( !ok ) {
    return;
  }

  QString type = types.at( descs.indexOf( desc ) );

  // Create new resource
  ResourceCalendar *resource = manager->createResource( type );
  if( !resource ) {
    KMessageBox::error( this,
                        i18n( "<qt>Unable to create a calendar of type <b>%1</b>.</qt>", type ) );
    return;
  }

  resource->setResourceName( i18n( "%1 calendar", type ) );

  // TODO: Add a fallback (KColorCollection::setName() broken?)
  KColorCollection collection( "Oxygen.colors" );
  // TODO: Be smarter than this
  int rand = ( KRandom::random() % collection.count() ) + 1;
  QColor color = collection.color( rand );

  KOPrefs::instance()->setResourceColor( resource->identifier(), color );

  bool success = true;
  QPointer<KRES::ConfigDialog> dlg =
    new KRES::ConfigDialog( this, QString( "calendar" ), resource );

  if ( dlg->exec() != QDialog::Accepted ) {
    success = false;
  }
  delete dlg;

  if ( success ) {
    resource->setTimeSpec( KOPrefs::instance()->timeSpec() );
    if ( resource->isActive() && ( !resource->open() || !resource->load() ) ) {
      // ### There is a resourceLoadError() signal declared in ResourceCalendar
      //     but no subclass seems to make use of it. We could do better.
      KMessageBox::error( this, i18n( "Unable to create the calendar." ) );
      success = false;
    }
  }

  if ( success ) {
    manager->add( resource );
    // we have to call resourceAdded manually, because for in-process changes
    // the dcop signals are not connected, so the resource's signals would not
    // be connected otherwise
    mCalendar->resourceAdded( resource );
  }

  if ( !success ) {
    delete resource;
    resource = 0;
  }

  //### maybe only do this if ( success )
  emitResourcesChanged();
}

void ResourceView::addResourceItem( ResourceCalendar *resource, bool emitSignal )
{
  new ResourceItem( resource, this, mListView );

  connect( resource,
           SIGNAL(signalSubresourceAdded(ResourceCalendar *,const QString &,const QString &,const QString &)), //krazy:exclude=style
           SLOT(slotSubresourceAdded(ResourceCalendar *,const QString &,const QString &,const QString &)) ); // krazy:exclude=style

  connect( resource,
           SIGNAL(signalSubresourceRemoved(ResourceCalendar *,const QString &,const QString &)),
           SLOT(slotSubresourceRemoved(ResourceCalendar *,const QString &,const QString &)) );

  connect( resource, SIGNAL(resourceSaved(ResourceCalendar *)),
           SLOT(closeResource(ResourceCalendar *)) );

  updateResourceList();
  if ( emitSignal ) {
    emit resourcesChanged();
  }
}

// Add a new entry
void ResourceView::slotSubresourceAdded( ResourceCalendar *calendar,
                                         const QString &type,
                                         const QString &resource,
                                         const QString &label )
{
  Q_UNUSED( type );
  QList<QTreeWidgetItem *> items =
    mListView->findItems( calendar->resourceName(), Qt::MatchExactly, 0 );

  if ( !items.isEmpty() && !findItemByIdentifier( resource ) ) {
    ResourceItem *item = static_cast<ResourceItem *>( items.first() );
    ( void )new ResourceItem( calendar, resource, label, this, item );
    emitResourcesChanged();
  }
}

// Remove an entry
void ResourceView::slotSubresourceRemoved( ResourceCalendar *calendar,
                                           const QString &type,
                                           const QString &resource )
{
  Q_UNUSED( calendar );
  Q_UNUSED( type );
  delete findItemByIdentifier( resource );
  emit resourcesChanged();
}

void ResourceView::closeResource( ResourceCalendar *r )
{
  if ( mResourcesToClose.contains( r ) ) {
    r->close();
    mResourcesToClose.removeAll( r );
  }
}

void ResourceView::updateResourceItem( ResourceCalendar *resource )
{
  ResourceItem *item = findItem( resource );
  if ( item ) {
    item->update();
  }
}

ResourceItem *ResourceView::currentItem()
{
  QTreeWidgetItem *item = mListView->currentItem();
  ResourceItem *rItem = static_cast<ResourceItem *>( item );
  return rItem;
}

void ResourceView::removeResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  int km =
    KMessageBox::warningContinueCancel(
      this,
      i18n( "<qt>Do you really want to remove the calendar <b>%1</b>?</qt>",
            item->text( 0 ) ), "", KStandardGuiItem::remove() );
  if ( km == KMessageBox::Cancel ) {
    return;
  }

// Don't be so restricitve
#if 1
  if ( item->resource() == mCalendar->resourceManager()->standardResource() ) {
    KMessageBox::sorry( this, i18n( "You cannot remove your standard calendar." ) );
    return;
  }
#endif

  if ( item->isSubresource() ) {
    if ( !item->resource()->removeSubresource( item->resourceIdentifier() ) ) {
      KMessageBox::sorry(
        this,
        i18n ( "<qt>Failed to remove the calendar folder <b>%1</b>. "
               "Perhaps it is a built-in folder which cannot be removed, or "
               "maybe the removal of the underlying storage folder failed.</qt>",
               item->text( 0 ) ) );
    }
    return;
  } else {
    mCalendar->resourceManager()->remove( item->resource() );
    delete item;
  }
  updateResourceList();
  emit resourcesChanged();
}

void ResourceView::editResource()
{
  bool ok = false;
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }
  ResourceCalendar *resource = item->resource();

  if ( item->isSubresource() ) {
    if ( resource->type() == "imap" ) {
      QString identifier = item->resourceIdentifier();
      const QString newResourceName =
        KInputDialog::getText( i18n( "Rename Calendar Folder" ),
                               i18n( "Please enter a new name for the calendar folder" ),
                               item->text(0),
                               &ok, this );
      if ( !ok ) {
        return;
      }

      QDBusConnection bus = QDBusConnection::sessionBus();
      QDBusInterface *interface =
        new QDBusInterface( "org.kde.kmail",
                            "/Groupware",
                            "org.kde.kmail.groupware",
                            bus,
                            this );

      QDBusReply<int> reply =
        interface->call( "changeResourceUIName", identifier, newResourceName );
      if ( !reply.isValid() ) {
        kDebug() << "DBUS Call changeResourceUIName() failed ";
      }
    } else {
      const QString subResourceName = resource->labelForSubresource( item->resourceIdentifier() );
      KMessageBox::sorry( this,
                          i18n ( "<qt>Cannot edit the calendar folder <b>%1</b>.</qt>",
                                 subResourceName ) );
    }
  } else {
    QPointer<KRES::ConfigDialog> dlg =
      new KRES::ConfigDialog( this, QString( "calendar" ), resource );
    if ( dlg->exec() ) {
      item->setText( 0, resource->resourceName() );
      mCalendar->resourceManager()->change( resource );
    }
    delete dlg;
  }
  emitResourcesChanged();
}

ResourceItem *ResourceView::findItem( ResourceCalendar *r )
{
  QList<QTreeWidgetItem *> items = mListView->findItems( "*", Qt::MatchWildcard );
  foreach ( QTreeWidgetItem *i, items ) {
    ResourceItem *item = static_cast<ResourceItem *>( i );
    if ( item->resource() == r ) {
      return item;
    }
  }
  return 0;
}

ResourceItem *ResourceView::findItemByIdentifier( const QString &id )
{
  QList<QTreeWidgetItem *>items =
    mListView->findItems( "*", Qt::MatchWildcard | Qt::MatchRecursive );

  foreach ( QTreeWidgetItem *i, items ) {
    ResourceItem *item = static_cast<ResourceItem *>( i );
    if ( item->resourceIdentifier() == id ) {
      return item;
    }
  }
  return 0;
}

void ResourceView::showContextMenu( const QPoint &pos )
{
  QTreeWidgetItem *i = mListView->itemAt( pos );

  if ( !i ) { // No item clicked.
    // Creation of menu entries not specific to one item
    QMenu *menu = new QMenu( this );
    menu->addAction( i18n( "&Add Calendar..." ), this, SLOT(addResource()) );
    menu->popup( mapToGlobal( pos ) );
    mSelectedParent = 0;

    return;
  }

  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  ResourceItem *item = static_cast<ResourceItem *>( i );
  mSelectedParent = item;

  QMenu *menu = new QMenu( this );
  connect( menu, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()) );
  if ( item ) {
    QAction *reloadAction = menu->addAction(
      i18nc( "reload the resource", "Re&load" ), this, SLOT(reloadResource()) );
    reloadAction->setEnabled( item->resource()->isActive() );

    QAction *saveAction = menu->addAction(
      i18nc( "save the resource", "&Save" ), this, SLOT(saveResource()) );
    saveAction->setEnabled( item->resource()->isActive() && !item->resource()->readOnly() );

    menu->addSeparator();

    menu->addAction( i18n( "Show &Info" ), this, SLOT(showInfo()) );
    //FIXME: This is better on the resource dialog
    if ( KOPrefs::instance()->agendaViewColors() != KOPrefs::CategoryOnly ) {
      QMenu *assignMenu = menu->addMenu( i18n( "Calendar Colors" ) );
      assignMenu->addAction( i18n( "&Assign Color..." ), this, SLOT(assignColor()) );
      if ( item->resourceColor().isValid() ) {
        assignMenu->addAction( i18n( "&Disable Color" ), this, SLOT(disableColor()) );
      }
    }

    menu->addAction( i18n( "&Edit..." ), this, SLOT(editResource()) );
    menu->addAction( i18n( "&Remove" ), this, SLOT(removeResource()) );
    if ( item->resource() != manager->standardResource() ) {
      menu->addSeparator();
      menu->addAction( i18n( "Use as &Default Calendar" ), this, SLOT(setStandard()) );
    }
    menu->addSeparator();
  }

  QString label;
  if ( item->isSubresource() || item->resource()->canHaveSubresources() ) {
    label = i18n( "&Add Calendar Folder..." );
  } else {
    label = i18n( "&Add Calendar..." );
  }

  menu->addAction( label, this, SLOT(addResource()) );
  menu->popup( mListView->mapToGlobal( pos ) );
}

void ResourceView::assignColor()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  // A color without initialized is a color invalid
  QColor myColor;
  KCal::ResourceCalendar *cal = item->resource();

  QString identifier = cal->identifier();
  if ( item->isSubresource() ) {
    identifier = item->resourceIdentifier();
  }

  QColor defaultColor = KOPrefs::instance()->resourceColor( identifier );

  int result = KColorDialog::getColor( myColor, defaultColor );
  if ( result == KColorDialog::Accepted ) {
    KOPrefs::instance()->setResourceColor( identifier, myColor );
    item->setResourceColor( myColor );
    item->update();
    emitResourcesChanged();
  }
}

void ResourceView::disableColor()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  QColor colorInvalid;
  KCal::ResourceCalendar *cal = item->resource();
  QString identifier = cal->identifier();
  if ( item->isSubresource() ) {
    identifier = item->resourceIdentifier();
  }
  KOPrefs::instance()->setResourceColor( identifier, colorInvalid );
  item->setResourceColor( colorInvalid );
  item->update();
  emitResourcesChanged();
}
void ResourceView::showInfo()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  QString txt = "<qt>" + item->resource()->infoText() + "</qt>";
  KMessageBox::information( this, txt );
}

void ResourceView::reloadResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }
  item->setIsReloading( true );
  item->resource()->load();
  item->setIsReloading( false );
}

void ResourceView::saveResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  ResourceCalendar *r = item->resource();
  r->save();
}

void ResourceView::setStandard()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  ResourceCalendar *r = item->resource();
  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  manager->setStandardResource( r );
  updateResourceList();
}

void ResourceView::updateResourceList()
{
  ResourceCalendar *stdRes = mCalendar->resourceManager()->standardResource();

  QList<QTreeWidgetItem *> items = mListView->findItems( "*", Qt::MatchWildcard );
  foreach ( QTreeWidgetItem *i, items ) {
    ResourceItem *item = static_cast<ResourceItem *>( i );
    item->setStandardResource( item->resource() == stdRes );
  }
}

void ResourceView::requestClose( ResourceCalendar *r )
{
  mResourcesToClose.append( r );
}

void ResourceView::slotItemClicked( QTreeWidgetItem *i, int )
{
  ResourceItem *item = static_cast<ResourceItem *>( i );
  if ( item ) {
    item->stateChange( item->checkState( 0 ) == Qt::Checked );
  }
}

void ResourceView::currentChanged()
{
  ResourceItem *i = currentItem();

  mDeleteButton->setEnabled( i != 0 );
  mEditButton->setEnabled( i != 0 );
}

#include "resourceview.moc"
