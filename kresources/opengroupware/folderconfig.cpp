/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "folderconfig.h"

#include "folderlister.h"

#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>

#include <qlayout.h>
#include <qgroupbox.h>
#include <qpushbutton.h>

using namespace KCal;

class FolderItem : public QCheckListItem
{
  public:
    FolderItem( KListView *listView, const FolderLister::Entry &folder )
      : QCheckListItem( listView, folder.name,
          QCheckListItem::CheckBoxController ), mFolder( folder )
    {
      setOn( mFolder.active );
    }

    FolderLister::Entry folder() const
    {
      return mFolder;
    }
  
  private:
    FolderLister::Entry mFolder;
};

FolderConfig::FolderConfig( QWidget *parent )
  : QWidget( parent ), mFolderLister( 0 )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->addSpacing( KDialog::spacingHint() );

  QGroupBox *topBox = new QGroupBox( 1, Horizontal, i18n("Folder Selection"),
    this );
  topLayout->addWidget( topBox );
  
  QPushButton *button = new QPushButton( i18n("Update Folder List"), topBox );
  connect( button, SIGNAL( clicked() ), SLOT( retrieveFolderList() ) );
  
  mFolderList = new KListView( topBox );
  mFolderList->addColumn( i18n("Folder") );
}

FolderConfig::~FolderConfig()
{
}

void FolderConfig::setFolderLister( FolderLister *f )
{
  mFolderLister = f;
  connect( mFolderLister, SIGNAL( foldersRead() ), SLOT( updateFolderList() ) );
}

void FolderConfig::retrieveFolderList()
{
  kdDebug() << "FolderConfig::updateFolderList()" << endl;

  mFolderLister->retrieveFolders();
}

void FolderConfig::updateFolderList()
{
  mFolderList->clear();

  FolderLister::Entry::List folders = mFolderLister->folders();
  FolderLister::Entry::List::ConstIterator it;
  for( it = folders.begin(); it != folders.end(); ++it ) {
    new FolderItem( mFolderList, (*it) );
  }
}

void FolderConfig::saveSettings()
{
  QPtrList<QListViewItem> lst;

  FolderLister::Entry::List folders;

  QListViewItemIterator it( mFolderList );
  while ( it.current() ) {
    FolderItem *item = static_cast<FolderItem *>( it.current() );
    FolderLister::Entry folder = item->folder();
    folder.active = item->isOn();
    folders.append( folder );
    ++it;
  }

  mFolderLister->setFolders( folders );
}

#include "folderconfig.moc"
