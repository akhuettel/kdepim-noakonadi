// kmfoldertree.cpp
#include <qfiledefs.h>
#include <qdragobject.h>
#include <qmessagebox.h>
#include <qheader.h>
#include <qbitmap.h>
#include <unistd.h>
#include <assert.h>
#include <kapp.h>
#include <kconfig.h>
#include <qpixmap.h>
#include <kiconloader.h>
#include <qtimer.h>
#include <qpopupmenu.h>
#include <klocale.h>
#include <kglobal.h>
#include "kfontutils.h"

#include "kmglobal.h"
#include "kmdragdata.h"
#include "kmfoldermgr.h"
#include "kmfolderdir.h"
#include "kmfolder.h"
#include "kmfoldertree.h"
#include "kmfolderdia.h"

QPixmap* KMFolderTree::pixDir = 0;
QPixmap* KMFolderTree::pixNode = 0;
QPixmap* KMFolderTree::pixPlain = 0;
QPixmap* KMFolderTree::pixFld = 0;
QPixmap* KMFolderTree::pixFull = 0;
QPixmap* KMFolderTree::pixIn = 0;
QPixmap* KMFolderTree::pixOut = 0;
QPixmap* KMFolderTree::pixTr = 0;
QPixmap* KMFolderTree::pixSent = 0;

//-----------------------------------------------------------------------------
// KMFolderTreeItem methods
// Begin this code may be relicensed by Troll Tech
static QBitmap * verticalLine = 0;
static QBitmap * horizontalLine = 0;

static void cleanupBitmapLines()
{
    delete verticalLine;
    delete horizontalLine;
    verticalLine = 0;
    horizontalLine = 0;
}
// End this code may be relicensed by Troll Tech

class KMFolderTreeItem : public QListViewItem
{

public:
  KMFolder* folder;
  QString unread;
  KMPaintInfo *mPaintInfo;

  /* Construct the root item */
  KMFolderTreeItem( QListView *parent, 
		    KMPaintInfo *aPaintInfo )
    : QListViewItem( parent, i18n("Mail") ), 
      folder( 0 ),
      unread( 0 ),
      mPaintInfo( aPaintInfo )
    {}

  /* Construct a child item */
  KMFolderTreeItem( QListViewItem* parent, 
		    KMFolder* folder, 
		    KMPaintInfo *aPaintInfo )
    : QListViewItem( parent, folder->label() ), 
      folder( folder ),
      unread( 0 ),
      mPaintInfo( aPaintInfo )
    {}

// Begin this code may be relicensed by Troll Tech
  void paintBranches( QPainter * p, const QColorGroup & cg,
		      int w, int y, int h, GUIStyle s )
{
  /******************************************************
   This code has been replaced due to licensing problems
   -sanders
  *******************************************************/
  QListViewItem::paintBranches( p, cg, w, y, h, s);
}


void paintCell( QPainter * p, const QColorGroup & cg,
		  int column, int width, int align )
{
  if ( !p )
    return;

  QListView *lv = listView();
  int r = lv ? lv->itemMargin() : 1;
  const QPixmap *icon = pixmap( column );
  int marg = lv ? lv->itemMargin() : 1;

  if (!mPaintInfo->pixmapOn)
    p->fillRect( 0, 0, width, height(), cg.base() );

  if ( isSelected() &&
       (column==0 || listView()->allColumnsShowFocus()) ) {
    p->fillRect( r - marg, 0, width - r + marg, height(),
		 cg.brush( QColorGroup::Highlight ) );
    p->setPen( cg.highlightedText() );
  } else {
    p->setPen( mPaintInfo->colFore );
  }
  
  if ( icon ) {
    p->drawPixmap( r, (height()-icon->height())/2, *icon );
    r += icon->width() + listView()->itemMargin();
  }
  
  QString t = text( column );
  if ( !t.isEmpty() ) {
    QRect br;
    p->drawText( r, 0, width-marg-r, height(),
		 align | AlignVCenter, t, -1, &br );
    if (!isSelected())
      p->setPen( mPaintInfo->colUnread );
    if (column == 0)
      p->drawText( br.right(), 0, width-marg-br.right(), height(),
		   align | AlignVCenter, unread);
    
  }
}
// End this code may be relicensed by Troll Tech

// May sure system folders come first when sorting
// (or last when sorting in descending order)
virtual QString key( int /*column*/, bool /*ascending*/ ) const {
    if (folder->label() == i18n("inbox"))
      return "\t0";
    else if (folder->label() == i18n("outbox"))
      return "\t1";
    else if (folder->label() == i18n("sent-mail"))
      return "\t2";
    else if (folder->label() == i18n("trash"))
      return "\t3";
    return text(0).lower();
  }
};

void KMFolderTree::drawContentsOffset( QPainter * p, int ox, int oy,
				       int cx, int cy, int cw, int ch ) {
    int c = 0;
    if (mPaintInfo.pixmapOn)
      paintEmptyArea( p, QRect( c - ox, cy - oy, cx + cw - c, ch ) );

    QListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );
  }


//-----------------------------------------------------------------------------
KMFolderTree::KMFolderTree(QWidget *parent,const char *name)
  : QListView( parent, name ), mList()
{
  KIconLoader* loader = KGlobal::iconLoader();
  static bool pixmapsLoaded = FALSE;

  initMetaObject();

  mUpdateTimer = NULL;

  connect(this, SIGNAL(currentChanged(QListViewItem*)),
	  this, SLOT(doFolderSelected(QListViewItem*)));
  connect(folderMgr, SIGNAL(changed()),
	  this, SLOT(doFolderListChanged()));

  readConfig();

  addColumn( i18n("Folders"), 400 );

  if (!pixmapsLoaded)
  {
    pixmapsLoaded = TRUE;

    pixDir   = new QPixmap( loader->loadIcon("closed"));
    pixNode  = new QPixmap( loader->loadIcon("green-bullet"));
    pixPlain = new QPixmap( loader->loadIcon("kmfolder"));
    pixFld   = new QPixmap( loader->loadIcon("kmfolder"));
    pixFull  = new QPixmap( loader->loadIcon("kmfolderfull"));
    pixIn    = new QPixmap( loader->loadIcon("kmfldin"));
    pixOut   = new QPixmap( loader->loadIcon("kmfldout"));
    pixSent  = new QPixmap( loader->loadIcon("kmfldsent"));
    pixTr    = new QPixmap( loader->loadIcon("kmtrash"));
  }
  setUpdatesEnabled(TRUE);
  reload();

  setAcceptDrops( TRUE );
  viewport()->setAcceptDrops( TRUE );

  // Drag and drop hover opening and autoscrolling support
  connect( &autoopen_timer, SIGNAL( timeout() ),
	   this, SLOT( openFolder() ) );

  connect( &autoscroll_timer, SIGNAL( timeout() ),
	   this, SLOT( autoScroll() ) );
}

//-----------------------------------------------------------------------------
void KMFolderTree::readConfig (void)
{
  KConfig* conf = app->config();
  QString fntStr;

  // Backing pixmap support
  conf->setGroup("Pixmaps");
  QString pixmapFile = conf->readEntry("FolderTree","");
  mPaintInfo.pixmapOn = FALSE;
  if (pixmapFile != "") {
    mPaintInfo.pixmapOn = TRUE;
    mPaintInfo.pixmap = QPixmap( pixmapFile );
  }

  // Custom/System color support
  conf->setGroup("Reader");
  QColor c1=QColor(app->palette().normal().text());
  QColor c2=QColor("blue");
  QColor c4=QColor(app->palette().normal().base());

  if (!conf->readBoolEntry("defaultColors",TRUE)) {
    mPaintInfo.colFore = conf->readColorEntry("ForegroundColor",&c1);
    mPaintInfo.colBack = conf->readColorEntry("BackgroundColor",&c4);
    QPalette newPal = palette();
    newPal.setColor( QColorGroup::Base, mPaintInfo.colBack );
    setPalette( newPal );
    mPaintInfo.colUnread = conf->readColorEntry("FollowedColor",&c2);
  }
  else {
    mPaintInfo.colFore = c1;
    mPaintInfo.colUnread = c2;
    QPalette newPal = palette();
    newPal.setColor( QColorGroup::Base, c4 );
    setPalette( newPal );
  }

  // Custom/Ssystem font support
  conf->setGroup("Fonts");
  if (!conf->readBoolEntry("defaultFonts",TRUE)) {
    fntStr = conf->readEntry("folder-font", "helvetica-medium-r-12");
    setFont(kstrToFont(fntStr));
  }
  else
    setFont(KGlobal::generalFont());
}

//-----------------------------------------------------------------------------
KMFolderTree::~KMFolderTree()
{
  disconnect(folderMgr, SIGNAL(changed()), this, SLOT(doFolderListChanged()));
  delete mUpdateTimer;
}

//-----------------------------------------------------------------------------
// Updates the count of unread messages (count of unread messages
// is now cached in KMails config file)
void KMFolderTree::updateUnreadAll()
{
  bool upd = isUpdatesEnabled();
  setUpdatesEnabled(FALSE);
  
  KMFolderDir* fdir;
  KMFolderNode* folderNode;
  KMFolder* folder;

  fdir = &folderMgr->dir();
  for (folderNode = fdir->first();
    folderNode != NULL;
    folderNode =fdir->next())
  {
    if (!folderNode->isDir()) {
      folder = static_cast<KMFolder*>(folderNode);
      
      folder->open();
      folder->countUnread();
      folder->close();
    }
  }
  
  setUpdatesEnabled(upd);
}

//-----------------------------------------------------------------------------
// Draw empty area of list view with support for a backing pixmap
void KMFolderTree::paintEmptyArea( QPainter * p, const QRect & rect )
{
  if (mPaintInfo.pixmapOn)
    p->drawTiledPixmap( rect.left(), rect.top(), rect.width(), rect.height(), 
			mPaintInfo.pixmap, 
			rect.left() + contentsX(), 
			rect.top() + contentsY() );
  else 
    p->fillRect( rect, colorGroup().base() );
}


//-----------------------------------------------------------------------------
// Save the configuration file
void KMFolderTree::writeConfig()
{
  QListViewItemIterator it( this );
  while (it.current()) {
    KMFolderTreeItem* fti = static_cast<KMFolderTreeItem*>(it.current());
    writeIsListViewItemOpen(fti);
    ++it;
  }
}

//-----------------------------------------------------------------------------
// Reload the tree of items in the list view
void KMFolderTree::reload(void)
{
  KMFolderDir* fdir;
  QString str;
  bool upd = isUpdatesEnabled();

  setUpdatesEnabled(FALSE);
  writeConfig();

  QListViewItemIterator it( this );
  while (it.current()) {
    KMFolderTreeItem* fti = static_cast<KMFolderTreeItem*>(it.current());
    if (fti->folder)
      disconnect(fti->folder,SIGNAL(numUnreadMsgsChanged(KMFolder*)),
		 this,SLOT(refresh(KMFolder*)));
    ++it;
  }
  clear();

  // We need our own root so that we can make it a KMFolderTreeItem
  // and use our custom paintBranches
  root = new KMFolderTreeItem( this, &mPaintInfo );
  root->setOpen( TRUE );
  fdir = &folderMgr->dir();
  addDirectory(fdir, root);
  
  QListViewItemIterator jt( this );
  while (jt.current()) {
    KMFolderTreeItem* fti = static_cast<KMFolderTreeItem*>(jt.current());
    if (fti->folder)
      connect(fti->folder,SIGNAL(numUnreadMsgsChanged(KMFolder*)),
	      this,SLOT(refresh(KMFolder*)));
    ++jt;
  }

  setUpdatesEnabled(upd);
  if (upd) repaint();
  refresh(0);
}

//-----------------------------------------------------------------------------
// Recursively add a directory of folders to the tree of folders
void KMFolderTree::addDirectory( KMFolderDir *fdir, QListViewItem* parent )
{ 
  KMFolderNode *folderNode;
  KMFolder* folder;
  KMFolderTreeItem* fti;

  for (folderNode = fdir->first();
       folderNode != NULL;
       folderNode = fdir->next())
    if (!folderNode->isDir()) {
      folder = static_cast<KMFolder*>(folderNode);
      ASSERT(parent);
      fti = new KMFolderTreeItem( parent, folder, &mPaintInfo );

      if (folder->label() == i18n("inbox"))
	fti->setPixmap( 0, *pixIn );
      else if (folder->label() == i18n("outbox"))
	fti->setPixmap( 0, *pixOut );
      else if (folder->label() == i18n("sent-mail"))
	fti->setPixmap( 0, *pixSent );
      else if (folder->label() == i18n("trash"))
	fti->setPixmap( 0, *pixTr );
      else
	fti->setPixmap( 0, *pixPlain );

      if (fti->folder && fti->folder->child())
	addDirectory( fti->folder->child(), fti );
      fti->setOpen( readIsListViewItemOpen(fti));
    }
}

//-----------------------------------------------------------------------------
// Initiate a delayed refresh of the count of unread messages
// Not really need anymore as count is cached in config file. But causes
// a nice blink in the foldertree, that indicates kmail did something
// when the user manually checks for mail and none was found.
void KMFolderTree::refresh(KMFolder* )
{
  if (!mUpdateTimer)
  {
    mUpdateTimer = new QTimer(this);
    connect(mUpdateTimer, SIGNAL(timeout()), this, SLOT(delayedUpdate()));
  }
  mUpdateTimer->changeInterval(200);
}
                                                               
//-----------------------------------------------------------------------------
// Updates the pixmap and extendedLabel information for items
void KMFolderTree::delayedUpdate()
{
  QString str;
  bool upd = isUpdatesEnabled();
  setUpdatesEnabled(FALSE);

  QListViewItemIterator it( this );
  while (it.current()) {
    bool repaintRequired = false;
    KMFolderTreeItem* fti = static_cast<KMFolderTreeItem*>(it.current());
    if (!fti->folder) {
      ++it;
      continue;
    }

    QString extendedName;
    if (fti->folder->countUnread() > 0) {
      QString num;
	num.setNum(fti->folder->countUnread());
      extendedName = " (" + num + ")";
      if (!fti->folder->isSystemFolder() &&
	  !(fti->folder->label() == "inbox"))
	fti->setPixmap( 0, *pixFull );
    }
    else {
      extendedName = "";
      if (!fti->folder->isSystemFolder() &&
	  !(fti->folder->label() == "inbox"))
	fti->setPixmap( 0, *pixPlain );
    }

    if (extendedName != fti->unread) {
      repaintRequired = true;
      fti->unread = extendedName;
    }

    if (upd && repaintRequired) 
      fti->repaint();
    ++it;
  }
  setUpdatesEnabled(upd);
  mUpdateTimer->stop();
}

//-----------------------------------------------------------------------------
// Folders have been added/deleted update the tree of folders
void KMFolderTree::doFolderListChanged()
{
  KMFolderTreeItem* fti = static_cast< KMFolderTreeItem* >(currentItem());
  KMFolder* folder = fti->folder;
  reload();
  QListViewItem *qlvi = indexOfFolder(folder);
  if (qlvi) {
    setCurrentItem( qlvi );
    setSelected( qlvi, TRUE );
  }
}

//-----------------------------------------------------------------------------
// When not dragging and dropping a change in the selected item
// indicates the user has changed the active folder emit a signal
// so that the header list and reader window can be udpated.
void KMFolderTree::doFolderSelected( QListViewItem* qlvi )
{
  KMFolderTreeItem* fti = static_cast< KMFolderTreeItem* >(qlvi);
  KMFolder* folder = fti->folder;

  setCurrentItem( qlvi );
  setSelected( qlvi, TRUE );
  if (!folder || folder->isDir()) {
    emit folderSelected(NULL); // Root has been selected
  }
  else emit folderSelected(folder);
}

//-----------------------------------------------------------------------------
void KMFolderTree::resizeEvent(QResizeEvent* e)
{
  KConfig* conf = app->config();

  conf->setGroup("Geometry");
  conf->writeEntry(name(), size().width());

  setColumnWidth( 0, size().width() );
  KMFolderTreeInherited::resizeEvent(e);
}

//-----------------------------------------------------------------------------
QListViewItem* KMFolderTree::indexOfFolder(const KMFolder* folder)
{
  QListViewItemIterator it( this );
  while (it.current()) {
    KMFolderTreeItem* fti = static_cast<KMFolderTreeItem*>(it.current());
    if (fti->folder == folder) {
      return it.current();
    }
    ++it;
  }
  return 0;
}

//-----------------------------------------------------------------------------
// Handle RMB press, show pop up menu
void KMFolderTree::contentsMouseReleaseEvent(QMouseEvent* event)
{
  if (!(event->button() & RightButton))
    return;
  QListViewItem *lvi = itemAt( contentsToViewport( event->pos()));
  if (!lvi)
    return;
  setCurrentItem( lvi );
  setSelected( lvi, TRUE );

  if (!topLevelWidget()) return; // safe bet

  QPopupMenu *folderMenu = new QPopupMenu;
  KMFolderTreeItem* fti = static_cast<KMFolderTreeItem*>(lvi);
  if (!fti->folder)
    return;

  if (fti->folder && !fti->folder->isSystemFolder()) {
    folderMenu->insertItem(i18n("&Create Child Folder..."), this,
			   SLOT(addChildFolder()));
    folderMenu->insertItem(i18n("&Modify..."), topLevelWidget(),
			   SLOT(slotModifyFolder()));
  }
  folderMenu->insertItem(i18n("C&ompact"), topLevelWidget(),
			 SLOT(slotCompactFolder()));
  folderMenu->insertSeparator();
  folderMenu->insertItem(i18n("&Empty"), topLevelWidget(),
			 SLOT(slotEmptyFolder()));
  if (fti->folder && !fti->folder->isSystemFolder()) {
  folderMenu->insertItem(i18n("&Remove"), topLevelWidget(),
			 SLOT(slotRemoveFolder()));
  }
  folderMenu->exec (QCursor::pos(), 0);
  delete folderMenu;
}

//-----------------------------------------------------------------------------
// Create a child folder.
// Requires creating the appropriate subdirectory and show a dialog
void KMFolderTree::addChildFolder()
{
  KMFolderTreeItem *fti = static_cast<KMFolderTreeItem*>(currentItem());
  KMFolder *aFolder = fti->folder;
  if (fti->folder)
    if (!fti->folder->createChildFolder())
      return;

  KMFolderDir *dir = &(folderMgr->dir());
  if (fti->folder)
    dir = fti->folder->child();

  KMFolderDialog *d;
  d = new KMFolderDialog(0, dir, 
			 app->mainWidget(), i18n( "New Child Folder" ));

  if (d->exec()) {
    QListViewItem *qlvi = indexOfFolder( aFolder );
    if (qlvi) {
      qlvi->setOpen(TRUE);
      setCurrentItem( qlvi );
    }
  }
}

//-----------------------------------------------------------------------------
// Returns whether a folder directory should be open as specified in the
// config file. The root is always open
bool KMFolderTree::readIsListViewItemOpen(KMFolderTreeItem *fti)
{
  KConfig* config = app->config();
  KMFolder *folder = fti->folder;
  if (folder)
    return TRUE;
  int pathLen = folder->path().length() - folderMgr->basePath().length();
  QString path = folder->path().right( pathLen );

  if (!path.isEmpty())
    path = path.right( path.length() - 1 ) + "/";
  config->setGroup("Folder-" + path + folder->name());
  return config->readBoolEntry("isOpen", false);
} 

//-----------------------------------------------------------------------------
// Saves open/closed state of a folder directory into the config file
void KMFolderTree::writeIsListViewItemOpen(KMFolderTreeItem *fti)
{
  KConfig* config = app->config();
  KMFolder *folder = fti->folder;
  if (!folder)
    return;
  int pathLen = folder->path().length() - folderMgr->basePath().length();
  QString path = folder->path().right( pathLen );

  if (!path.isEmpty())
    path = path.right( path.length() - 1 ) + "/";
  config->setGroup("Folder-" + path + folder->name());
  config->writeEntry("isOpen", fti->isOpen());
} 


//-----------------------------------------------------------------------------
// Drag and Drop handling -- based on the Troll Tech dirview example

void KMFolderTree::openFolder()
{
    autoopen_timer.stop();
    if ( dropItem && !dropItem->isOpen() ) {
        dropItem->setOpen( TRUE );
        dropItem->repaint();
    }
}

static const int autoopenTime = 750;

void KMFolderTree::contentsDragEnterEvent( QDragEnterEvent *e )
{
  if ( !KMHeaderToFolderDrag::canDecode(e) ) {
        e->ignore();
        return;
  }
  
  oldCurrent = currentItem();
  
  QListViewItem *i = itemAt( contentsToViewport(e->pos()) );
  if ( i ) {
    dropItem = i;
    autoopen_timer.start( autoopenTime );
  }
  disconnect(this, SIGNAL(currentChanged(QListViewItem*)),
	     this, SLOT(doFolderSelected(QListViewItem*)));
}

static const int autoscroll_margin = 16;
static const int initialScrollTime = 30;
static const int initialScrollAccel = 5;

void KMFolderTree::startAutoScroll()
{
    if ( !autoscroll_timer.isActive() ) {
        autoscroll_time = initialScrollTime;
        autoscroll_accel = initialScrollAccel;
        autoscroll_timer.start( autoscroll_time );
    }
}

void KMFolderTree::stopAutoScroll()
{
    autoscroll_timer.stop();
}

void KMFolderTree::autoScroll()
{
    QPoint p = viewport()->mapFromGlobal( QCursor::pos() );

    if ( autoscroll_accel-- <= 0 && autoscroll_time ) {
        autoscroll_accel = initialScrollAccel;
        autoscroll_time--;
        autoscroll_timer.start( autoscroll_time );
    }
    int l = QMAX(1,(initialScrollTime-autoscroll_time));

    int dx=0,dy=0;
    if ( p.y() < autoscroll_margin ) {
        dy = -l;
    } else if ( p.y() > visibleHeight()-autoscroll_margin ) {
        dy = +l;
    }
    if ( p.x() < autoscroll_margin ) {
        dx = -l;
    } else if ( p.x() > visibleWidth()-autoscroll_margin ) {
        dx = +l;
    }
    if ( dx || dy ) {
        scrollBy(dx,dy);
    } else {
        stopAutoScroll();
    }
}

void KMFolderTree::contentsDragMoveEvent( QDragMoveEvent *e )
{
    if ( !KMHeaderToFolderDrag::canDecode(e) ) {
        e->ignore();
        return;
    }

    QPoint vp = contentsToViewport(e->pos());
    QRect inside_margin(autoscroll_margin, autoscroll_margin,
                        visibleWidth()-autoscroll_margin*2,
                        visibleHeight()-autoscroll_margin*2);
    QListViewItem *i = itemAt( vp );
    if ( i ) {
        setSelected( i, TRUE );
        if ( !inside_margin.contains(vp) ) {
            startAutoScroll();
            e->accept(QRect(0,0,0,0)); // Keep sending move events
            autoopen_timer.stop();
        } else {
            e->accept();
            if ( i != dropItem ) {
                autoopen_timer.stop();
                dropItem = i;
                autoopen_timer.start( autoopenTime );
            }
        }
        switch ( e->action() ) {
	    case QDropEvent::Copy:
            break;
	    case QDropEvent::Move:
            e->acceptAction();
            break;
	    case QDropEvent::Link:
            e->acceptAction();
            break;
	    default:
            ;
        }
    } else {
        e->ignore();
        autoopen_timer.stop();
        dropItem = 0L;
    }
}

void KMFolderTree::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    autoopen_timer.stop();
    stopAutoScroll();
    dropItem = 0L;

    setCurrentItem( oldCurrent );
    setSelected( oldCurrent, TRUE );
    connect(this, SIGNAL(currentChanged(QListViewItem*)),
	    this, SLOT(doFolderSelected(QListViewItem*)));
}

void KMFolderTree::contentsDropEvent( QDropEvent *e )
{
    autoopen_timer.stop();
    stopAutoScroll();

    QListViewItem *item = itemAt( contentsToViewport(e->pos()) );
    if ( item ) {
        QString str;
	KMFolderTreeItem *fti = static_cast<KMFolderTreeItem*>(item);
	if ((fti != oldCurrent) && (fti->folder))
	  emit folderDrop(fti->folder);
	e->accept();
    } else
      e->ignore();

    // Begin this wasn't necessary in QT 2.0.2
    dropItem = 0L;

    setCurrentItem( oldCurrent );
    setSelected( oldCurrent, TRUE );
    connect(this, SIGNAL(currentChanged(QListViewItem*)),
	    this, SLOT(doFolderSelected(QListViewItem*)));
    // End this wasn't necessary in QT 2.0.2
}

#include "kmfoldertree.moc"

