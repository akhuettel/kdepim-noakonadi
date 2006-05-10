/*  -*- c++ -*-
    vacationdialog.cpp

    KMail, the KDE mail client.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vacationdialog.h"

#include <kmime_header_parsing.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QByteArray>
using KMime::Types::AddrSpecList;
using KMime::Types::AddressList;
using KMime::Types::MailboxList;
using KMime::HeaderParsing::parseAddressList;

#include <knuminput.h>
#include <klocale.h>
#include <kdebug.h>
#include <kwin.h>
#include <kapplication.h>

#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <q3textedit.h>
#include <kiconloader.h>

namespace KMail {

  VacationDialog::VacationDialog( const QString & caption, QWidget * parent,
				  const char * name, bool modal )
    : KDialogBase( Plain, caption, Ok|Cancel|Default, Ok, parent, name, modal )
  {
    KWin::setIcons( winId(), qApp->windowIcon().pixmap(IconSize(K3Icon::Desktop),IconSize(K3Icon::Desktop)), qApp->windowIcon().pixmap(IconSize(K3Icon::Small),IconSize(K3Icon::Small)) );

    static const int rows = 4;
    int row = -1;

    QGridLayout * glay = new QGridLayout( plainPage() );
    glay->setSpacing( spacingHint() );
    glay->setMargin( 0 );
    glay->setColumnStretch( 1, 1 );

    // explanation label:
    ++row;
    glay->addWidget( new QLabel( i18n("Configure vacation "
					       "notifications to be sent:"),
					  plainPage() ), row, 0, 1, 2 );

    // Activate checkbox:
    ++row;
    mActiveCheck = new QCheckBox( i18n("&Activate vacation notifications"), plainPage() );
    glay->addWidget( mActiveCheck, row, 0, 1, 2 );

    // Message text edit:
    ++row;
    glay->setRowStretch( row, 1 );
    mTextEdit = new Q3TextEdit( plainPage(), "mTextEdit" );
    mTextEdit->setTextFormat( Qt::PlainText );
    glay->addWidget( mTextEdit, row, 0, 1, 2 );

    // "Resent only after" spinbox and label:
    ++row;
    mIntervalSpin = new KIntSpinBox( 1, 356, 1, 7, plainPage() );
    mIntervalSpin->setObjectName( "mIntervalSpin" );
    connect(mIntervalSpin, SIGNAL( valueChanged( int )), SLOT( slotIntervalSpinChanged( int ) ) );
    QLabel *label = new QLabel( i18n("&Resend notification only after:"), plainPage() );
    label->setBuddy( mIntervalSpin );
    glay->addWidget( label, row, 0 );
    glay->addWidget( mIntervalSpin, row, 1 );

    // "Send responses for these addresses" lineedit and label:
    ++row;
    mMailAliasesEdit = new QLineEdit( plainPage(), "mMailAliasesEdit" );
    QLabel *tmpLabel = new QLabel( i18n("&Send responses for these addresses:"), plainPage() );
    tmpLabel->setBuddy( mMailAliasesEdit );
    glay->addWidget( tmpLabel, row, 0 );
    glay->addWidget( mMailAliasesEdit, row, 1 );

    // row 5 is for stretch.
    Q_ASSERT( row == rows - 1 );
  }

  VacationDialog::~VacationDialog() {
    kDebug(5006) << "~VacationDialog()" << endl;
  }

  bool VacationDialog::activateVacation() const {
    return mActiveCheck->isChecked();
  }

  void VacationDialog::setActivateVacation( bool activate ) {
    mActiveCheck->setChecked( activate );
  }

  QString VacationDialog::messageText() const {
    return mTextEdit->text().trimmed();
  }

  void VacationDialog::setMessageText( const QString & text ) {
    mTextEdit->setText( text );
  }

  int VacationDialog::notificationInterval() const {
    return mIntervalSpin->value();
  }

  void VacationDialog::setNotificationInterval( int days ) {
    mIntervalSpin->setValue( days );
  }

  AddrSpecList VacationDialog::mailAliases() const {
    QByteArray text = mMailAliasesEdit->text().toLatin1(); // ### IMAA: !ok
    AddressList al;
    const char * s = text.begin();
    parseAddressList( s, text.end(), al );

    AddrSpecList asl;
    for ( AddressList::const_iterator it = al.begin() ; it != al.end() ; ++it ) {
      const MailboxList & mbl = (*it).mailboxList;
      for ( MailboxList::const_iterator jt = mbl.begin() ; jt != mbl.end() ; ++jt )
	asl.push_back( (*jt).addrSpec );
    }
    return asl;
  }

  void VacationDialog::setMailAliases( const AddrSpecList & aliases ) {
    QStringList sl;
    for ( AddrSpecList::const_iterator it = aliases.begin() ; it != aliases.end() ; ++it )
      sl.push_back( (*it).asString() );
    mMailAliasesEdit->setText( sl.join(", ") );
  }

  void VacationDialog::setMailAliases( const QString & aliases ) {
    mMailAliasesEdit->setText( aliases );
  }

  void VacationDialog::slotIntervalSpinChanged ( int value ) {
    mIntervalSpin->setSuffix( i18np(" day", " days", value) );
  }

} // namespace KMail

#include "vacationdialog.moc"
