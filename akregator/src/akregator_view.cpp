/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny, Sashmit Bhaduri            *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *   smt@vfemail.net (Sashmit Bhaduri)                                     *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "akregator_part.h"
#include "akregator_view.h"
#include "addfeeddialog.h"
#include "propertiesdialog.h"
#include "feedstree.h"
#include "articlelist.h"
#include "articleviewer.h"
#include "archive.h"
#include "feed.h"
#include "akregatorconfig.h"

#include <kfiledialog.h>
#include <kfileitem.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <kaction.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <kpassdlg.h>
#include <klistview.h>
#include <khtml_part.h>
#include <kdebug.h>
#include <krun.h>
#include <kurl.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qpopupmenu.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qvaluevector.h>
#include <qtabwidget.h>
#include <qgrid.h>


using namespace Akregator;


aKregatorView::aKregatorView( aKregatorPart *part, QWidget *parent, const char *wName)
    : QWidget(parent, wName), m_feeds()
{
    m_part=part;

    QVBoxLayout *lt = new QVBoxLayout( this );

    m_panner1Sep << 1 << 1;
    m_panner2Sep << 1 << 1;

    m_panner1 = new QSplitter(QSplitter::Horizontal, this, "panner1");
    m_panner1->setOpaqueResize( true );
    lt->addWidget(m_panner1);

    m_tree = new FeedsTree( m_panner1, "FeedsTree" );

    connect(m_tree, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
              this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)));
    connect(m_tree, SIGNAL(selectionChanged(QListViewItem*)),
              this, SLOT(slotItemChanged(QListViewItem*)));
    connect(m_tree, SIGNAL(itemRenamed(QListViewItem *)),
              this, SLOT(slotItemRenamed(QListViewItem *)));

    m_panner1->setResizeMode( m_tree, QSplitter::KeepSize );

    m_tabs = new QTabWidget(m_panner1);
    QWhatsThis::add(m_tabs, i18n("You can view multiple articles in several open tabs."));

    QGrid *w1 = new QGrid(1, this);
    QWhatsThis::add(w1, i18n("Articles list."));

    m_panner2 = new QSplitter(QSplitter::Vertical, w1, "panner2");

    m_articles = new ArticleList( m_panner2, "articles" );
    connect( m_articles, SIGNAL(clicked(QListViewItem *)),
                   this, SLOT( slotArticleSelected(QListViewItem *)) );
    connect( m_articles, SIGNAL(doubleClicked(QListViewItem *, const QPoint &, int)),
                   this, SLOT( slotArticleDoubleClicked(QListViewItem *, const QPoint &, int)) );

    m_panner1->setSizes( m_panner1Sep );
    m_panner2->setSizes( m_panner2Sep );

    m_articleViewer = new ArticleViewer(m_panner2, "article_viewer");

    connect( m_articleViewer, SIGNAL(urlClicked(const KURL&)),
                        this, SLOT(slotOpenTab(const KURL&)) );

    connect( m_articleViewer->browserExtension(), SIGNAL(mouseOverInfo(const KFileItem *)),
                                            this, SLOT(slotMouseOverInfo(const KFileItem *)) );

    QWhatsThis::add(m_articleViewer->widget(), i18n("Browsing area."));

    m_tabs->addTab(w1, i18n( "Articles" ));

    // -- DEFAULT INIT
    // Root item (will be reset when loading from file)
    KListViewItem *elt = new KListViewItem( m_tree, QString::null );
    m_feeds.addFeedGroup(elt)->setTitle( i18n("All Feeds") );
    elt->setOpen(true);

    m_articleViewer->openDefault();
    // -- /DEFAULT INIT

    // Change default view mode
    int viewMode = Settings::viewMode();

    if (viewMode==CombinedView)        slotCombinedView();
    else if (viewMode==WidescreenView) slotWidescreenView();
    else                               slotNormalView();
}

void aKregatorView::slotOpenTab(const KURL& url)
{
   QWidget *test = new QWidget();
   m_tabs->addTab(test, "test_tab");
}

// clears everything out, even removes DEFAULT INIT
void aKregatorView::reset()
{
    m_feeds.clearFeeds();
    m_tree->clear();
    m_part->setModified(false);
}


bool aKregatorView::loadFeeds(const QDomDocument& doc)
{
    // this should be OPML document
    QDomElement root = doc.documentElement();
    kdDebug() << "loading OPML feed "<<root.tagName().lower()<<endl;
    if (root.tagName().lower() != "opml")
        return false;

    // we ignore <head> and only parse <body> part
    QDomNodeList list = root.elementsByTagName("body");
    if (list.count() != 1)
        return false;

    QDomElement body = list.item(0).toElement();
    if (body.isNull())
        return false;

    reset();

    QDomNode n = body.firstChild();
    while( !n.isNull() )
    {
        parseChildNodes(n);
        n = n.nextSibling();
    }

    return true;
}

void aKregatorView::parseChildNodes(QDomNode &node, KListViewItem *parent)
{
    QDomElement e = node.toElement(); // try to convert the node to an element.
    if( !e.isNull() )
    {
        KListViewItem *elt;
        if (parent)
        {
            QListViewItem *lastChild = parent->firstChild();
            while (lastChild && lastChild->nextSibling()) lastChild = lastChild->nextSibling();
            elt = new KListViewItem( parent, lastChild, e.attribute("text") );
        }
        else
            elt = new KListViewItem( m_tree, m_tree->lastItem(), e.attribute("text") );

        if (e.hasAttribute("xmlUrl") || e.hasAttribute("xmlurl"))
        {
            QString xmlurl=e.hasAttribute("xmlUrl") ? e.attribute("xmlUrl") : e.attribute("xmlurl");
            QString title=e.hasAttribute("text") ? e.attribute("text") : e.attribute("title");

            addFeed_Internal( elt,
                              title,
                              xmlurl,
                              e.attribute("htmlUrl"),
                              e.attribute("description"),
                              e.attribute("isLiveJournal") == "true" ? true : false,
                              e.attribute("ljUserName"),
                              Feed::authModeFromString( e.attribute("ljAuthMode", "none") ),
                              e.attribute("ljLogin"),
                              e.attribute("ljPassword"),
                              e.attribute("updateTitle") == "true" ? true : false
                            );
        }
        else
        {
            m_feeds.addFeedGroup(elt);
            FeedGroup *g = m_feeds.find(elt);
            if (g)
                g->setTitle( e.attribute("text") );

            elt->setOpen( e.attribute("isOpen", "true") == "true" ? true : false );
        }

        if (e.hasChildNodes())
        {
            QDomNode child = e.firstChild();
            while(!child.isNull())
            {
                parseChildNodes(child, elt);
                child = child.nextSibling();
            }
        }
    }
}

// oh ugly as hell (pass Feed parameters in a FeedData?)
Feed *aKregatorView::addFeed_Internal(QListViewItem *elt,
                                      QString title, QString xmlUrl, QString htmlUrl,
                                      QString description, bool isLiveJournal, QString ljUserName,
                                      Feed::LJAuthMode ljAuthMode, QString ljLogin, QString ljPassword,
                                      bool updateTitle)
{
    m_feeds.addFeed(elt);

    Feed *feed = static_cast<Feed *>(m_feeds.find(elt));

    feed->setTitle( title );
    feed->xmlUrl         = xmlUrl;
    feed->htmlUrl        = htmlUrl;
    feed->description    = description;
    feed->isLiveJournal  = isLiveJournal;
    feed->ljUserName     = ljUserName;
    feed->ljAuthMode     = ljAuthMode;
    feed->ljLogin        = ljLogin;
    feed->ljPassword     = ljPassword;
    feed->updateTitle    = updateTitle;

    connect( feed, SIGNAL(fetched(Feed* )),
             this, SLOT(slotFeedFetched(Feed *)) );

    // Read feed archive, if present
    Archive::load(feed);

    // enable when we need to update favicons, on for example systray
    //connect( feed, SIGNAL(faviconLoaded()),
    //         this, SLOT(slotFaviconLoaded()));

    return feed;
}


void aKregatorView::writeChildNodes( QListViewItem *item, QDomElement &node, QDomDocument &document)
{
  if (!item)
        item=m_tree->firstChild();
    for (QListViewItem *it = item; it; it = it->nextSibling())
    {
        FeedGroup *g = m_feeds.find(it);
        if (g)
        {
            if (g->isGroup())
            {
                QDomElement base = g->toXml( node, document );
                base.setAttribute("isOpen", it->isOpen() ? "true" : "false");

                writeChildNodes( it->firstChild(), base, document );
            } else {
                g->toXml( node, document );
            }
        }
    }
}

bool aKregatorView::event(QEvent *e)
{
    if (e->type() == QEvent::ApplicationPaletteChange)
    {
        m_articleViewer->reload();
        return true;
    }
    return QWidget::event(e);
}

void aKregatorView::slotNormalView()
{
    if (m_viewMode==NormalView)
       return;

    else if (m_viewMode==CombinedView)
    {
        m_articles->show();
        // tell articleview to redisplay+reformat
        ArticleListItem *item = static_cast<ArticleListItem *>(m_articles->currentItem());
        if (item)
        {
            Feed *feed = static_cast<Feed *>(m_feeds.find(m_tree->currentItem()));
            if (feed)
                m_articleViewer->show( feed, item->article() );
        }
    }

    m_panner2->setOrientation(QSplitter::Vertical);
    m_viewMode=NormalView;

    Settings::setViewMode( m_viewMode );
}

void aKregatorView::slotWidescreenView()
{
    if (m_viewMode==WidescreenView)
       return;
    else if (m_viewMode==CombinedView)
    {
        m_articles->show();
        // tell articleview to redisplay+reformat
        ArticleListItem *item = static_cast<ArticleListItem *>(m_articles->currentItem());
        if (item)
        {
            Feed *feed = static_cast<Feed *>(m_feeds.find(m_tree->currentItem()));
            if (feed)
                m_articleViewer->show( feed, item->article() );
        }
    }

    m_panner2->setOrientation(QSplitter::Horizontal);
    m_viewMode=WidescreenView;

    Settings::setViewMode( m_viewMode );
}

void aKregatorView::slotCombinedView()
{
    if (m_viewMode==CombinedView)
       return;

    m_articles->hide();
    m_viewMode=CombinedView;

    Feed *feed = static_cast<Feed *>(m_feeds.find(m_tree->currentItem()));
    if (feed)
        m_articleViewer->show(feed);

    Settings::setViewMode( m_viewMode );
}



void aKregatorView::slotContextMenu(KListView*, QListViewItem*, const QPoint& p)
{
   QWidget *w = m_part->factory()->container("feeds_popup", m_part);
   if (w)
      static_cast<QPopupMenu *>(w)->exec(p);
}

void aKregatorView::slotItemChanged(QListViewItem *item)
{
    FeedGroup *feed = static_cast<FeedGroup *>(m_feeds.find(item));

    if (feed->isGroup())
    {
        m_part->actionCollection()->action("feed_add")->setEnabled(true);
        m_part->actionCollection()->action("feed_add_group")->setEnabled(true);
    }
    else
    {
        m_part->actionCollection()->action("feed_add")->setEnabled(false);
        m_part->actionCollection()->action("feed_add_group")->setEnabled(false);

        slotUpdateArticleList( static_cast<Feed *>(feed) );
        if (m_viewMode==CombinedView)
            m_articleViewer->show(static_cast<Feed *>(feed) );
    }

    if (item->parent())
        m_part->actionCollection()->action("feed_remove")->setEnabled(true);
    else
        m_part->actionCollection()->action("feed_remove")->setEnabled(false);

}

void aKregatorView::slotUpdateArticleList(Feed *source)
{
    m_articles->setUpdatesEnabled(false);
    m_articles->clear(); // FIXME adding could become rather slow if we store a lot of archive items?

    m_articles->setColumnText(0, source->title());

    if (source->articles.count() > 0)
    {
        MyArticle::List::ConstIterator it;
        MyArticle::List::ConstIterator end = source->articles.end();
        for (it = source->articles.begin(); it != end; ++it)
        {
            new ArticleListItem( m_articles, (*it), source );
        }
    }
    m_articles->setUpdatesEnabled(true);
    m_articles->triggerUpdate();
}

// NOTE: feed can only be added to a feed group as a child
void aKregatorView::slotFeedAdd()
{
    if (!m_tree->currentItem() || m_feeds.find(m_tree->currentItem())->isGroup() == false)
    {
        KMessageBox::error(this, i18n("You have to choose feed group before adding feed."));
        return;
    }


    KListViewItem *elt;
    AddFeedDialog *afd = new AddFeedDialog( this, "add_feed" );
    if (afd->exec() != QDialog::Accepted) return;

    QString text=afd->feedTitle;

    FeedPropertiesDialog *dlg = new FeedPropertiesDialog( this, "edit_feed" );

    dlg->setFeedName(text);
    dlg->setUrl(afd->feedURL);
//    dlg->widget->urlEdit->hide();

    if (dlg->exec() != QDialog::Accepted) return;


    QListViewItem *lastChild = m_tree->currentItem()->firstChild();
    while (lastChild && lastChild->nextSibling())
        lastChild = lastChild->nextSibling();

    if (lastChild)
        elt = new KListViewItem(m_tree->currentItem(), lastChild, text);
    else
        elt = new KListViewItem(m_tree->currentItem(), text);

    addFeed_Internal( elt,
                      text,
                      dlg->url(),
                      "",
                      "",
                      false, //dlg->widget->ljUserChkbox->isChecked(),
                      QString::null, //dlg->widget->ljUserEdit->text(),
                      dlg->authMode(),
                      dlg->ljLogin(),
                      dlg->ljPassword(),
                      false
                    );

    m_part->setModified(true);
}

void aKregatorView::slotFeedAddGroup()
{
    if (!m_tree->currentItem() || m_feeds.find(m_tree->currentItem())->isGroup() == false)
    {
        KMessageBox::error(this, i18n("You have to choose feed group before adding subgroup."));
        return;
    }

    bool Ok;
    KListViewItem *elt;

    QString text = KInputDialog::getText(i18n("Add Feed Group"), i18n("Feed group title:"), "", &Ok);
    if (!Ok) return;

    QListViewItem *lastChild = m_tree->currentItem()->firstChild();
    while (lastChild && lastChild->nextSibling())
        lastChild = lastChild->nextSibling();

    if (lastChild)
        elt = new KListViewItem(m_tree->currentItem(), lastChild, text);
    else
        elt = new KListViewItem(m_tree->currentItem(), text);

    m_feeds.addFeedGroup(elt);
    FeedGroup *g = m_feeds.find(elt);
    if (g)
        g->setTitle( text );

    m_part->setModified(true);
}

void aKregatorView::slotFeedRemove()
{
    QListViewItem *elt = m_tree->currentItem();
    if (!elt) return;
    QListViewItem *parent = elt->parent();
    if (!parent) return; // don't delete root element! (safety valve)

    QString msg = elt->childCount() ?
        i18n("<qt>Are you sure you want to delete group<br><b>%1</b><br> and its subgroups and feeds?</qt>") :
        i18n("<qt>Are you sure you want to delete feed<br><b>%1</b>?</qt>");
    if (KMessageBox::warningContinueCancel(0, msg.arg(elt->text(0)),i18n("Delete Feed"),KGuiItem(i18n("&Delete"),"editdelete")) == KMessageBox::Continue)
    {
        m_feeds.removeFeed(elt);
        // FIXME: kill children? (otoh - auto kill)
/*        if (!Notes.count())
            slotActionUpdate();
        if (!parent)
            parent = Items->firstChild();
        Items->prevItem = 0;
        slotNoteChanged(parent);*/

        m_part->setModified(true);
    }
}

void aKregatorView::slotFeedModify()
{
    kdDebug() << k_funcinfo << "BEGIN" << endl;

    FeedGroup *g = m_feeds.find(m_tree->currentItem());
    if (g->isGroup())
    {
        m_tree->currentItem()->setRenameEnabled(0, true);
        m_tree->currentItem()->startRename(0);
        return;
    }

    Feed *feed = static_cast<Feed *>(g);
    if (!feed) return;

    FeedPropertiesDialog *dlg = new FeedPropertiesDialog( this, "edit_feed" );

    dlg->setFeedName( feed->title() );
    dlg->setUrl( feed->xmlUrl );
    dlg->setAuthMode( feed->ljAuthMode );
    dlg->setLjLogin( feed->ljLogin );
    dlg->setLjPassword( feed->ljPassword );

    if (dlg->exec() != QDialog::Accepted) return;

    feed->setTitle( dlg->feedName() );
    feed->xmlUrl         = dlg->url();
    feed->ljAuthMode     = dlg->authMode();
    feed->ljLogin        = dlg->ljLogin();
    feed->ljPassword     = dlg->ljPassword();

    m_part->setModified(true);

    kdDebug() << k_funcinfo << "END" << endl;
}

void aKregatorView::slotFetchCurrentFeed()
{
    if (m_tree->currentItem())
    {
        Feed *f = static_cast<Feed *>(m_feeds.find(m_tree->currentItem()));
        if (f && !f->isGroup())
        {
            kdDebug() << "Fetching item " << f->title() << endl;
            f->fetch();
        }
        else // its a feed group, need to fetch from all its children, recursively
        {
            FeedGroup *g = m_feeds.find(m_tree->currentItem());
            if (!g) {
                KMessageBox::error( this, i18n( "Internal error, feeds tree inconsistent." ) );
                return;
            }

            for (QListViewItemIterator it(m_tree->currentItem()); it.current(); ++it)
            {
                kdDebug() << "Fetching subitem " << (*it)->text(0) << endl;
                Feed *f = static_cast<Feed *>(m_feeds.find(*it));
                if (f && !f->isGroup())
                    f->fetch();
            }
        }
    }
}

void aKregatorView::slotFetchAllFeeds()
{
    for (QListViewItemIterator it(m_tree->firstChild()); it.current(); ++it)
    {
        kdDebug() << "Fetching subitem " << (*it)->text(0) << endl;
        Feed *f = static_cast<Feed *>(m_feeds.find(*it));
        if (f && !f->isGroup())
            f->fetch();
    }
}


void aKregatorView::slotFeedFetched(Feed *feed)
{
    // Feed finished fetching
    // If its a currenly selected feed, update view
    kdDebug() << k_funcinfo << "BEGIN" << endl;
    if (feed->item() == m_tree->currentItem())
    {
        kdDebug() << k_funcinfo << "Updating article list" << endl;
        slotUpdateArticleList(feed);
    }

    // Also, update unread counts

    // Store archive
    Archive::save(feed);

//    m_part->setModified(true); // FIXME reenable when article storage is implemented

    kdDebug() << k_funcinfo << "END" << endl;
}

void aKregatorView::slotArticleSelected(QListViewItem *i)
{
    if (m_viewMode==CombinedView) return; // shouldn't ever happen

    ArticleListItem *item = static_cast<ArticleListItem *>(i);
    if (!item) return;
    Feed *feed = static_cast<Feed *>(m_feeds.find(m_tree->currentItem()));
    if (!feed) return;
    m_articleViewer->show( feed, item->article() );
}

void aKregatorView::slotArticleDoubleClicked(QListViewItem *i, const QPoint &, int)
{
    ArticleListItem *item = static_cast<ArticleListItem *>(i);
    if (!item) return;
    if (!item->article().link().isValid()) return;
    // TODO : make this configurable....
    KRun::runURL(item->article().link(), "text/html", false, false);

}


void aKregatorView::slotItemRenamed( QListViewItem *item )
{
    QString text = item->text(0);
    kdDebug() << "Item renamed to " << text << endl;

    Feed *feed = static_cast<Feed *>(m_feeds.find(item));
    if (feed)
    {
        feed->setTitle( text );
        if (!feed->isGroup())
            feed->updateTitle = false; // if user edited title by hand, do not update it automagically

        m_part->setModified(true);
    }
}

void aKregatorView::slotMouseOverInfo(const KFileItem *kifi)
{
    if (kifi)
    {
        KFileItem *k=(KFileItem*)kifi;
        m_part->setStatusBar(k->url().prettyURL());//getStatusBarInfo());
    }
    else
    {
    m_part->setStatusBar(QString::null);
    }
}

#include "akregator_view.moc"
