/*
    This file is part of ksync.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KSYNC_H
#define KSYNC_H

#include <kapplication.h>
#include <kmainwindow.h>
#include <kaccel.h>
#include <kaction.h>

class KSyncView;

/**
  * The base class for KSync application windows. It sets up the main
  * window and reads the config file as well as providing a menubar, toolbar
  * and statusbar. An instance of KSyncView creates your center view, which is connected
  * to the window's Doc object.
  * KSyncApp reimplements the methods that KMainWindow provides for main window handling and supports
  * full session management as well as using KActions.
  * @see KMainWindow
  * @see KApplication
  * @see KConfig
  *
  * @author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team.
  * @version KDevelop version 1.2 code generation
  */
class KSync : public KMainWindow
{
  Q_OBJECT

  friend class KSyncView;

  public:
    /** construtor of KSyncApp, calls all init functions to create the application.
     */
    KSync(QWidget* parent=0, const char* name=0);

    /** opens a file specified by commandline option
     */
    void openDocumentFile(const KURL& url=KURL());
    /** returns a pointer to the current document connected to the KTMainWindow instance and is used by
     * the View class to access the document object's methods
     */	

  protected:
    /** save general Options like all bar positions and status as well as the geometry and the recent file list to the configuration
     * file
     */ 	
    void saveOptions();
    /** read general Options again and initialize all variables like the recent file list
     */
    void readOptions();
    /** initializes the KActions of the application */
    void initActions();
    /** sets up the statusbar for the main window by initialzing a statuslabel.
     */
    void initStatusBar();
    /** creates the centerwidget of the KTMainWindow instance and sets it as the view
     */
    void initView();
    /** queryClose is called by KTMainWindow on each closeEvent of a window. Against the
     * default implementation (only returns true), this calles saveModified() on the document object to ask if the document shall
     * be saved if Modified; on cancel the closeEvent is rejected.
     * @see KTMainWindow#queryClose
     * @see KTMainWindow#closeEvent
     */
    virtual bool queryClose();
    /** queryExit is called by KTMainWindow when the last window of the application is going to be closed during the closeEvent().
     * Against the default implementation that just returns true, this calls saveOptions() to save the settings of the last window's	
     * properties.
     * @see KTMainWindow#queryExit
     * @see KTMainWindow#closeEvent
     */
    virtual bool queryExit();
    /** saves the window properties for each open window during session end to the session config file, including saving the currently
     * opened file by a temporary filename provided by KApplication.
     * @see KTMainWindow#saveProperties
     */
    virtual void saveProperties(KConfig *_cfg);
    /** reads the session config file and restores the application's state including the last opened files and documents by reading the
     * temporary files saved by saveProperties()
     * @see KTMainWindow#readProperties
     */
    virtual void readProperties(KConfig *_cfg);

  public slots:
    /** open a new application window by creating a new instance of KSyncApp */
    void slotFileNewWindow();
    /** clears the document in the actual view to reuse it as the new document */
    void slotFileNew();
    /** open a file and load it into the document*/
    void slotFileOpen();
    /** opens a file from the recent files menu */
    void slotFileOpenRecent(const KURL& url);
    /** save a document */
    void slotFileSave();
    /** save a document by a new filename*/
    void slotFileSaveAs();
    /** asks for saving if the file is modified, then closes the actual file and window*/
    void slotFileClose();
    /** print the actual file */
    void slotFilePrint();
    /** closes all open windows by calling close() on each memberList item until the list is empty, then quits the application.
     * If queryClose() returns false because the user canceled the saveModified() dialog, the closing breaks.
     */
    void slotFileQuit();
    /** put the marked text/object into the clipboard and remove
     *	it from the document
     */
    void slotEditCut();
    /** put the marked text/object into the clipboard
     */
    void slotEditCopy();
    /** paste the clipboard into the document
     */
    void slotEditPaste();
    /** changes the statusbar contents for the standard label permanently, used to indicate current actions.
     * @param text the text that is displayed in the statusbar
     */
    void slotStatusMsg(const QString &text);

  private:
    /** the configuration object of the application */
    KConfig *config;
    /** view is the main widget which represents your working area. The View
     * class should handle all events of the view widget.  It is kept empty so
     * you can create your view according to your application's needs by
     * changing the view class.
     */
    KSyncView *mView;

    // KAction pointers to enable/disable actions
    KAction* fileNewWindow;
    KAction* fileNew;
    KAction* fileOpen;
    KRecentFilesAction* fileOpenRecent;
    KAction* fileSave;
    KAction* fileSaveAs;
    KAction* fileClose;
    KAction* filePrint;
    KAction* fileQuit;
    KAction* editCut;
    KAction* editCopy;
    KAction* editPaste;
};
 
#endif // KSYNC_H
