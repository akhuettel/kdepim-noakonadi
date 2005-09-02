/*  -*- c++ -*-
    kpgpui.h

    Copyright (C) 2001,2002 the KPGP authors
    See file AUTHORS.kpgp for details

    This file is part of KPGP, the KDE PGP/GnuPG support library.

    KPGP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef KPGPUI_H
#define KPGPUI_H

#include <kdialogbase.h>  // base class of all dialogs here
#include <qwidget.h>      // base class of Config
#include <qcheckbox.h>    // used in inlined methods
//Added by qt3to4:
#include <QPixmap>
#include <QLabel>
#include <Q3ValueList>
#include <kdebug.h>       // used in inlined methods
#include <q3cstring.h>     // used in return-by-value
#include <qstring.h>      // is a member in KeyRequester
#include <q3valuevector.h> // used in KeyApprovalDialog
#include <q3ptrvector.h>
#include "kpgp.h"

#include <kdepimmacros.h>

class QString;
class QRegExp;
class Q3CString;
class QCheckBox;            // needed by Config, KeySelectionDialog
class Q3MultiLineEdit;       // needed by CipherTextDialog
class QComboBox;            // needed by Config
class QPixmap;              // needed by KeySelectionDialog
class QPushButton;          // needed by KeyRequester
class QTimer;               // needed by KeySelectionDialog

class KListView;            // needed by KeySelectionDialog
class KPasswordEdit;        // needed by PassphraseDialog

namespace Kpgp {

class Module;
class KeyList;              // needed by KeySelectionDialog
class Key;                  // needed by KeySelectionDialog
class KeyIDList;            // needed by KeySelectionDialog

/** the passphrase dialog */
class KDE_EXPORT PassphraseDialog : public KDialogBase
{
  Q_OBJECT

  public:
    PassphraseDialog( QWidget *parent=0, const QString &caption=QString::null,
                      bool modal=true, const QString &keyID=QString::null);
    virtual ~PassphraseDialog();

    const char * passphrase();

  private:
    KPasswordEdit *lineedit;
};


// -------------------------------------------------------------------------
/** a widget for configuring the pgp interface. Can be included into
    a tabdialog. This widget by itself does not provide an apply/cancel
    button mechanism. */
class KDE_EXPORT Config : public QWidget
{
  Q_OBJECT

  public:
    Config(QWidget *parent = 0, const char *name = 0, bool encrypt =true);
    virtual ~Config();

    virtual void setValues();
    virtual void applySettings();
    QGroupBox* optionsGroupBox() { return mpOptionsGroupBox; };
  signals:
    void changed();

  protected:
    Module *pgp;
    QCheckBox *storePass;
    QCheckBox *encToSelf;
    QCheckBox *showCipherText;
    QCheckBox *showKeyApprovalDlg;
    QComboBox *toolCombo;
    QGroupBox* mpOptionsGroupBox;
};


// -------------------------------------------------------------------------
#define KeySelectionDialogSuper KDialogBase
class KDE_EXPORT KeySelectionDialog: public KeySelectionDialogSuper
{
  Q_OBJECT

  enum TrustCheckMode { NoExpensiveTrustCheck,
                        AllowExpensiveTrustCheck,
                        ForceTrustCheck
                      };

  public:
    /** allowedKeys: see kpgp.h
     */
    KeySelectionDialog( const KeyList& keyList,
                        const QString& title,
                        const QString& text = QString::null,
                        const KeyIDList& keyIds = KeyIDList(),
                        const bool rememberChoice = false,
                        const unsigned int allowedKeys = AllKeys,
                        const bool extendedSelection = false,
                        QWidget *parent=0, const char *name=0,
                        bool modal=true );
    virtual ~KeySelectionDialog();

    /** Returns the key ID of the selected key in single selection mode.
        Otherwise it returns a null string. */
    virtual KeyID key() const;
 
    /** Returns a list of selected key IDs. */
    virtual KeyIDList keys() const
      { return mKeyIds; };

    virtual bool rememberSelection() const
      { if( mRememberCB )
          return mRememberCB->isChecked();
        else
          return false;
      };

  protected slots:
    virtual void slotRereadKeys();
    virtual void slotSelectionChanged( Q3ListViewItem* );
    virtual void slotSelectionChanged();
    virtual void slotCheckSelection( Q3ListViewItem* = 0 );
    virtual void slotRMB( Q3ListViewItem*, const QPoint&, int );
    virtual void slotRecheckKey();
    virtual void slotOk();
    virtual void slotCancel();
    virtual void slotSearch( const QString & text );
    virtual void slotFilter();

  private:
    void filterByKeyID( const QString & keyID );
    void filterByKeyIDOrUID( const QString & keyID );
    void filterByUID( const QString & uid );
    void showAllItems();
    bool anyChildMatches( const Q3ListViewItem * item, QRegExp & rx ) const;

    void initKeylist( const KeyList& keyList, const KeyIDList& keyIds );

    QString keyInfo( const Kpgp::Key* ) const;

    QString beautifyFingerprint( const Q3CString& ) const;

    // Returns the key ID of the key the given QListViewItem belongs to
    KeyID getKeyId( const Q3ListViewItem* ) const;

    // Returns: -1 = unusable, 0 = unknown, 1 = valid, but untrusted, 2 = trusted
    int keyValidity( const Kpgp::Key* ) const;

    // Updates the given QListViewItem with the data of the given key
    void updateKeyInfo( const Kpgp::Key*, Q3ListViewItem* ) const;

    /** Checks if choosing the given key is allowed
        Returns:
        -1 = key must not be chosen,
         0 = not enough information to decide whether the give key is allowed
             or not,
         1 = key can be chosen
    */
    int keyAdmissibility( Q3ListViewItem*,
                          TrustCheckMode = NoExpensiveTrustCheck ) const;

    // Perform expensive trust checks for the given keys
    bool checkKeys( const Q3ValueList<Q3ListViewItem*>& ) const;

  private:
    KListView *mListView;
    QCheckBox *mRememberCB;
    QPixmap *mKeyGoodPix, *mKeyBadPix, *mKeyUnknownPix, *mKeyValidPix;
    KeyIDList mKeyIds;
    unsigned int mAllowedKeys;
    QTimer* mCheckSelectionTimer;
    QTimer* mStartSearchTimer;
    QString mSearchText;
    Q3ListViewItem* mCurrentContextMenuItem;

  static const int sCheckSelectionDelay;
};

class KDE_EXPORT KeyRequester: public QWidget
{
  Q_OBJECT

public:
  KeyRequester( QWidget * parent=0, bool multipleKeys=false,
		unsigned int allowedKeys=AllKeys, const char * name=0 );
  virtual ~KeyRequester();

  KeyIDList keyIDs() const;
  void setKeyIDs( const KeyIDList & keyIDs );

  QPushButton * eraseButton() const { return mEraseButton; }
  QPushButton * dialogButton() const { return mDialogButton; }

  void setDialogCaption( const QString & caption );
  void setDialogMessage( const QString & message );

  bool isMultipleKeysEnabled() const;
  void setMultipleKeysEnabled( bool enable );

  int allowedKeys() const;
  void setAllowedKeys( int allowed );

protected:
  /** Reimplement this to return a list of selected keys. */
  virtual KeyIDList keyRequestHook( Module * pgp ) const = 0;

protected:
  QLabel * mLabel;
  QPushButton * mEraseButton;
  QPushButton * mDialogButton;
  QString mDialogCaption, mDialogMessage;
  bool mMulti;
  int mAllowedKeys;
  KeyIDList mKeys;

protected slots:
  void slotDialogButtonClicked();
  void slotEraseButtonClicked();

signals:
  void changed();

private:
  class Private;
  Private * d;
protected:
  virtual void virtual_hook( int, void* );
};


class KDE_EXPORT PublicKeyRequester : public KeyRequester {
  Q_OBJECT
public:
  PublicKeyRequester( QWidget * parent=0, bool multipleKeys=false,
		      unsigned int allowedKeys=PublicKeys, const char * name=0 );
  virtual ~PublicKeyRequester();

protected:
  KeyIDList keyRequestHook( Module * pgp ) const;

private:
  typedef KeyRequester base;
  class Private;
  Private * d;
protected:
  virtual void virtual_hook( int, void* );
};


class KDE_EXPORT SecretKeyRequester : public KeyRequester {
  Q_OBJECT
public:
  SecretKeyRequester( QWidget * parent=0, bool multipleKeys=false,
		      unsigned int allowedKeys=SecretKeys, const char * name=0 );
  virtual ~SecretKeyRequester();

protected:
  KeyIDList keyRequestHook( Module * pgp ) const;

private:
  typedef KeyRequester base;
  class Private;
  Private * d;
protected:
  virtual void virtual_hook( int, void* );
};


// -------------------------------------------------------------------------
class KDE_EXPORT KeyApprovalDialog: public KDialogBase
{
  Q_OBJECT

  public:
    KeyApprovalDialog( const QStringList&,
                       const Q3ValueVector<KeyIDList>&,
                       const int allowedKeys,
                       QWidget *parent = 0, const char *name = 0,
                       bool modal = true );
    virtual ~KeyApprovalDialog() {};

    Q3ValueVector<KeyIDList> keys() const { return mKeys; };

    bool preferencesChanged() const { return mPrefsChanged; }

  protected slots:
    void slotPrefsChanged( int ) { mPrefsChanged = true; };
    void slotChangeEncryptionKey( int );
    virtual void slotOk();
    virtual void slotCancel();

  private:
    Q3ValueVector<KeyIDList> mKeys;
    int mAllowedKeys;
    int mEncryptToSelf;
    bool mPrefsChanged;
    Q3PtrVector<QLabel> mAddressLabels;
    Q3PtrVector<QLabel> mKeyIdsLabels;
    //QPtrVector<QListBox> mKeyIdListBoxes;
    Q3PtrVector<QComboBox> mEncrPrefCombos;
};


// -------------------------------------------------------------------------
class KDE_EXPORT CipherTextDialog: public KDialogBase
{
  Q_OBJECT

  public:
    CipherTextDialog( const Q3CString & text, const Q3CString & charset=0,
                      QWidget *parent=0, const char *name=0, bool modal=true );
    virtual ~CipherTextDialog() {};

  private:
    void setMinimumSize();
    Q3MultiLineEdit *mEditBox;
};

} // namespace Kpgp

#endif
