// -*- mode: C++; c-file-style: "gnu" -*-

#ifndef KMReaderMainWin_h
#define KMReaderMainWin_h

#include "secondarywindow.h"

#include <kurl.h>

#include <boost/scoped_ptr.hpp>

class KMReaderWin;
class KMMessage;
class KMMessagePart;
class KAction;
class KActionMenu;
class KFontAction;
class KFontSizeAction;
class CustomTemplatesMenu;
template <typename T, typename S> class QMap;

namespace KMail {
class MessageActions;
}

class KMReaderMainWin : public KMail::SecondaryWindow
{
  Q_OBJECT

public:
  KMReaderMainWin( bool htmlOverride, bool htmlLoadExtOverride, char *name = 0 );
  KMReaderMainWin( char *name = 0 );
  KMReaderMainWin(KMMessagePart* aMsgPart,
    bool aHTML, const QString& aFileName, const QString& pname,
    const QString & encoding, char *name = 0 );
  virtual ~KMReaderMainWin();

  void setUseFixedFont( bool useFixedFont );

  /**
   * take ownership of and show @param msg
   *
   * The last two parameters, serNumOfOriginalMessage and nodeIdOffset, are needed when @p msg
   * is derived from another message, e.g. the user views an encapsulated message in this window.
   * Then, the reader needs to know about that original message, so those to parameters are passed
   * onto setOriginalMsg() of KMReaderWin.
   */
  void showMsg( const QString & encoding, KMMessage *msg,
                unsigned long serNumOfOriginalMessage = 0, int nodeIdOffset = -1 );

private slots:
  void slotMsgPopup(KMMessage &aMsg, const KUrl &aUrl, const QPoint& aPoint);

  /** Copy selected messages to folder with corresponding to given QAction */
  void slotCopySelectedMessagesToFolder( QAction* );
  void slotTrashMsg();
  void slotPrintMsg();
  void slotForwardInlineMsg();
  void slotForwardAttachedMsg();
  void slotRedirectMsg();
  void slotShowMsgSrc();
  void slotFontAction(const QString &);
  void slotSizeAction(int);
  void slotCreateTodo();
  void slotCustomReplyToMsg( const QString &tmpl );
  void slotCustomReplyAllToMsg( const QString &tmpl );
  void slotCustomForwardMsg( const QString &tmpl );

  void slotEditToolbars();
  void slotConfigChanged();
  void slotUpdateToolbars();

  void slotFolderRemoved( QObject* folderPtr );

private:
  void initKMReaderMainWin();
  void setupAccel();
  void updateMessageMenu();
  void updateCustomTemplateMenus();

  KMReaderWin *mReaderWin;
  KMMessage *mMsg;
  KUrl mUrl;
  // a few actions duplicated from kmmainwidget
  KAction *mTrashAction, *mPrintAction, *mSaveAsAction, *mSaveAtmAction,
          *mViewSourceAction;
  KActionMenu *mCopyActionMenu;
  KFontAction *fontAction;
  KFontSizeAction *fontSizeAction;
  KMail::MessageActions *mMsgActions;

  // Custom template actions menu
  boost::scoped_ptr<CustomTemplatesMenu> mCustomTemplateMenus;
};

#endif /*KMReaderMainWin_h*/
