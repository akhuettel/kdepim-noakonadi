/*
    knode.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/
#include "knode.h"


#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <kmenubar.h>
#include <kiconloader.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kapplication.h>

//GUI
#include "knmainwidget.h"
#include "knarticlewidget.h"
#include "knarticlewindow.h"
#include "kncollectionviewitem.h"
#include "knhdrviewitem.h"

KNMainWindow::KNMainWindow( QWidget* pWidget )
  : KMainWindow(pWidget,"mainWindow")
{
  //setupStatusBar();
  createStandardStatusBarAction();
  setStandardToolBarMenuEnabled(true);

  //config stuff
  KStdAction::quit(kapp, SLOT(closeAllWindows()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()),
                                actionCollection(), "knode_configure_toolbars" );

  m_mainWidget = new KNMainWidget( this, this, 0 );
  connect( m_mainWidget, SIGNAL(signalCaptionChangeRequest(const QString&)),
           SLOT( setCaption(const QString&)) );
  setCentralWidget( m_mainWidget );
  createGUI( "knodeui.rc" );
}

KNMainWindow::~KNMainWindow()
{
  saveMainWindowSettings(KGlobal::config(),"mainWindow_options");
}

void KNMainWindow::openURL( const KURL& url )
{
  m_mainWidget->openURL( url );
}

void KNMainWindow::slotConfToolbar()
{
  saveMainWindowSettings(KGlobal::config(),"mainWindow_options");
  KEditToolbar dlg(actionCollection(), "knodeui.rc");
  connect(&dlg,SIGNAL( newToolbarConfig() ), this, SLOT( slotNewToolbarConfig() ));
  dlg.exec();
}

void KNMainWindow::slotNewToolbarConfig()
{
  createGUI("knodeui.rc");
  //initPopups();
  applyMainWindowSettings(KGlobal::config(),"mainWindow_options");
}

#include "knode.moc"
