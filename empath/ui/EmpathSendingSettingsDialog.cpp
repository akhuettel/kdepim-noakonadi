/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathSendingSettingsDialog.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "RikGroupBox.h"
		
EmpathSendingSettingsDialog::EmpathSendingSettingsDialog(
		QWidget * parent,
		const char * name)
	:	QWidget(parent, name)
{
	empathDebug("ctor");
	
	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();

	rgb_server_	= new RikGroupBox(i18n("Server"), 8, this, "rgb_server");
	CHECK_PTR(rgb_server_);
	
	rgb_copies_		= new RikGroupBox(i18n("Copies of outgoing messages"), 8, this, "rgb_copies");
	CHECK_PTR(rgb_copies_);

	w_server_	= new QWidget(rgb_server_,	"w_server");
	CHECK_PTR(w_server_);
	
	w_copies_		= new QWidget(rgb_copies_,	"w_copies");
	CHECK_PTR(w_copies_);
	
	rgb_server_->setMinimumHeight(h * 4 + (6 * 10));
	rgb_copies_->setMinimumHeight(h * 3 + (5 * 10));
	
	rgb_server_->setWidget(w_server_);
	rgb_copies_->setWidget(w_copies_);

	// Server

	serverButtonGroup_	=
		new QButtonGroup(this, "serverButtonGroup");
	CHECK_PTR(serverButtonGroup_);

	serverButtonGroup_->hide();
	serverButtonGroup_->setExclusive(true);

	rb_sendmail_		=
		new QRadioButton("Sendmail", w_server_,
				"rb_sendmail");
	CHECK_PTR(rb_sendmail_);

	rb_sendmail_->setMaximumHeight(h);
	rb_sendmail_->setMinimumHeight(h);

	le_sendmail_		=
		new QLineEdit(w_server_, "le_sendmail");
	CHECK_PTR(le_sendmail_);
	
	le_sendmail_->setMaximumHeight(h);
	le_sendmail_->setMinimumHeight(h);

	pb_sendmailBrowse_	=
		new QPushButton("...", w_server_, "pb_sendmailBrowse");
	CHECK_PTR(pb_sendmailBrowse_);
	
	pb_sendmailBrowse_->setMaximumHeight(h);
	pb_sendmailBrowse_->setMinimumHeight(h);

	QObject::connect(rb_sendmail_, SIGNAL(toggled(bool)),
			le_sendmail_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_sendmail_, SIGNAL(toggled(bool)),
			pb_sendmailBrowse_, SLOT(setEnabled(bool)));
	
	rb_qmail_		=
		new QRadioButton("Qmail", w_server_,
				"rb_qmail");
	CHECK_PTR(rb_qmail_);
	
	rb_qmail_->setMaximumHeight(h);
	rb_qmail_->setMinimumHeight(h);

	le_qmail_		=
		new QLineEdit(w_server_, "le_qmail");
	CHECK_PTR(le_qmail_);
	
	le_qmail_->setMaximumHeight(h);
	le_qmail_->setMinimumHeight(h);

	pb_qmailBrowse_	=
		new QPushButton("...", w_server_, "pb_qmailBrowse");
	CHECK_PTR(pb_qmailBrowse_);
	
	pb_qmailBrowse_->setMaximumHeight(h);
	pb_qmailBrowse_->setMinimumHeight(h);

	QObject::connect(rb_qmail_, SIGNAL(toggled(bool)),
			le_qmail_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_qmail_, SIGNAL(toggled(bool)),
			pb_qmailBrowse_, SLOT(setEnabled(bool)));
	
	rb_smtp_		=
		new QRadioButton("SMTP", w_server_,
				"rb_smtp");
	CHECK_PTR(rb_smtp_);
	
	rb_smtp_->setMaximumHeight(h);
	rb_smtp_->setMinimumHeight(h);

	le_smtpServer_		=
		new QLineEdit(w_server_, "le_smtp");
	CHECK_PTR(le_smtpServer_);
	
	le_smtpServer_->setMaximumHeight(h);
	le_smtpServer_->setMinimumHeight(h);

	l_smtpServerPort_	=
		new QLabel(i18n("Port:"), w_server_, "l_smtpServerPort");
	CHECK_PTR(l_smtpServerPort_);
	
	l_smtpServerPort_->setMaximumHeight(h);
	l_smtpServerPort_->setMinimumHeight(h);

#ifdef EMPATH_QT_BUILD
	sb_smtpPort_	=
		new QSpinBox(1, 99999999, 1, w_server_, "sb_smtpPort");
	CHECK_PTR(sb_smtpPort_);
#else
	sb_smtpPort_	=
		new KNumericSpinBox(w_server_, "sb_smtpPort");
	CHECK_PTR(sb_smtpPort_);
	sb_smtpPort_->setEditable(true);
	sb_smtpPort_->setRange(1, 99999999);
	sb_smtpPort_->setValue(25);
#endif

	sb_smtpPort_->setMaximumHeight(h);
	sb_smtpPort_->setMinimumHeight(h);

	QObject::connect(rb_smtp_, SIGNAL(toggled(bool)),
			le_smtpServer_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_smtp_, SIGNAL(toggled(bool)),
			l_smtpServerPort_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_smtp_, SIGNAL(toggled(bool)),
			sb_smtpPort_, SLOT(setEnabled(bool)));
	
	rb_qmtp_		=
		new QRadioButton("QMTP", w_server_,
				"rb_qmtp");
	CHECK_PTR(rb_qmtp_);
	
	rb_qmtp_->setMaximumHeight(h);
	rb_qmtp_->setMinimumHeight(h);

	le_qmtpServer_		=
		new QLineEdit(w_server_, "le_qmtp");
	CHECK_PTR(le_qmtpServer_);
	
	le_qmtpServer_->setMaximumHeight(h);
	le_qmtpServer_->setMinimumHeight(h);

	l_qmtpServerPort_	=
		new QLabel(i18n("Port:"), w_server_, "l_qmtpServerPort");
	CHECK_PTR(l_qmtpServerPort_);
	
	l_qmtpServerPort_->setMaximumHeight(h);
	l_qmtpServerPort_->setMinimumHeight(h);

#ifdef EMPATH_QT_BUILD
	sb_qmtpPort_	=
		new QSpinBox(1, 9999999999, 1, w_server_, "sb_qmtpPort");
	CHECK_PTR(sb_qmtpPort_);
#else
	sb_qmtpPort_	=
		new KNumericSpinBox(w_server_, "sb_qmtpPort");
	CHECK_PTR(sb_qmtpPort_);
	sb_qmtpPort_->setEditable(true);
	sb_qmtpPort_->setRange(1, 99999999);
	sb_qmtpPort_->setValue(209);
#endif
	
	sb_qmtpPort_->setMaximumHeight(h);
	sb_qmtpPort_->setMinimumHeight(h);

	QObject::connect(rb_qmtp_, SIGNAL(toggled(bool)),
			le_qmtpServer_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_qmtp_, SIGNAL(toggled(bool)),
			l_qmtpServerPort_, SLOT(setEnabled(bool)));
	
	QObject::connect(rb_qmtp_, SIGNAL(toggled(bool)),
			sb_qmtpPort_, SLOT(setEnabled(bool)));
	
	serverButtonGroup_->insert(rb_sendmail_,	0);
	serverButtonGroup_->insert(rb_qmail_,		1);
	serverButtonGroup_->insert(rb_smtp_,		2);
	serverButtonGroup_->insert(rb_qmtp_,		3);
	
	rb_sendmail_->setChecked(true);	
	le_qmail_->setEnabled(false);
	pb_qmailBrowse_->setEnabled(false);
	le_smtpServer_->setEnabled(false);
	l_smtpServerPort_->setEnabled(false);
	sb_smtpPort_->setEnabled(false);
	le_qmtpServer_->setEnabled(false);
	l_qmtpServerPort_->setEnabled(false);
	sb_qmtpPort_->setEnabled(false);
	

	// Copies
	
	cb_copySelf_	=
		new QCheckBox(i18n("Send a copy to &me"), w_copies_, "cb_copySelf");
	CHECK_PTR(cb_copySelf_);

	cb_copySelf_->setMaximumHeight(h);
	cb_copySelf_->setMinimumHeight(h);
	cb_copySelf_->setMinimumWidth(cb_copySelf_->sizeHint().width());
	
	cb_copyOther_	=
		new QCheckBox(i18n("Send a copy to other &address:"), w_copies_,
				"cb_copyOther");
	CHECK_PTR(cb_copyOther_);
	
	cb_copyOther_->setMaximumHeight(h);
	cb_copyOther_->setMinimumHeight(h);
	cb_copyOther_->setMinimumWidth(cb_copyOther_->sizeHint().width());

	le_copyOther_	=
		new QLineEdit(w_copies_, "le_copyOther");
	CHECK_PTR(le_copyOther_);
	
	le_copyOther_->setMaximumHeight(h);
	le_copyOther_->setMinimumHeight(h);
	
	pb_browseCopyOther_	=
		new QPushButton("...", w_copies_, "pb_browseCopyOther");
	CHECK_PTR(pb_browseCopyOther_);
	
	pb_browseCopyOther_->setMaximumHeight(h);
	pb_browseCopyOther_->setMinimumHeight(h);
	
	QObject::connect(cb_copyOther_, SIGNAL(toggled(bool)),
			le_copyOther_, SLOT(setEnabled(bool)));
	
	QObject::connect(cb_copyOther_, SIGNAL(toggled(bool)),
			pb_browseCopyOther_, SLOT(setEnabled(bool)));
	
	le_copyOther_->setEnabled(false);
	pb_browseCopyOther_->setEnabled(false);
	
	cb_copyFolder_	=
		new QCheckBox(i18n("Place a copy in &folder: "), w_copies_,
				"cb_copyFolder");
	CHECK_PTR(cb_copyFolder_);
	
	cb_copyFolder_->setMaximumHeight(h);
	cb_copyFolder_->setMinimumHeight(h);
	cb_copyFolder_->setMinimumWidth(cb_copyFolder_->sizeHint().width());
	
	cmb_copyFolder_	=
		new QComboBox(w_copies_, "cmb_copyFolder");
	CHECK_PTR(cmb_copyFolder_);
	
	cmb_copyFolder_->setMaximumHeight(h);
	cmb_copyFolder_->setMinimumHeight(h);
	
	QObject::connect(cb_copyFolder_, SIGNAL(toggled(bool)),
			cmb_copyFolder_, SLOT(setEnabled(bool)));

	cmb_copyFolder_->setEnabled(false);
	
	// Layouts
	
	topLevelLayout_				= new QGridLayout(this, 2, 1, 10, 10);
	CHECK_PTR(topLevelLayout_);

	topLevelLayout_->setRowStretch(0, 4);
	topLevelLayout_->setRowStretch(1, 3);

	serverGroupLayout_		= new QGridLayout(w_server_,	4, 4, 0, 10);
	CHECK_PTR(serverGroupLayout_);
	
	serverGroupLayout_->setColStretch(0, 2);
	serverGroupLayout_->setColStretch(1, 2);
	serverGroupLayout_->setColStretch(2, 1);
	serverGroupLayout_->setColStretch(3, 1);
	
	copiesGroupLayout_		= new QGridLayout(w_copies_,	3, 3, 0, 10);
	CHECK_PTR(copiesGroupLayout_);

//	copiesGroupLayout_->setColStretch(0, 6);
	copiesGroupLayout_->setColStretch(1, 4);
	copiesGroupLayout_->setColStretch(2, 1);

	topLevelLayout_->addWidget(rgb_server_,		0, 0);
	topLevelLayout_->addWidget(rgb_copies_,		1, 0);

	serverGroupLayout_->addWidget(rb_sendmail_,		0, 0);
	serverGroupLayout_->addWidget(rb_qmail_,		1, 0);
	serverGroupLayout_->addWidget(rb_smtp_,			2, 0);
	serverGroupLayout_->addWidget(rb_qmtp_,			3, 0);

	serverGroupLayout_->addMultiCellWidget(le_sendmail_,	0, 0, 1, 2);
	serverGroupLayout_->addMultiCellWidget(le_qmail_,		1, 1, 1, 2);
	serverGroupLayout_->addWidget(le_smtpServer_,			2, 1);
	serverGroupLayout_->addWidget(le_qmtpServer_,			3, 1);
	
	serverGroupLayout_->addWidget(pb_sendmailBrowse_,	0, 3);
	serverGroupLayout_->addWidget(pb_qmailBrowse_,		1, 3);

	serverGroupLayout_->addWidget(l_smtpServerPort_,	2, 2);
	serverGroupLayout_->addWidget(l_qmtpServerPort_,	3, 2);
	
	serverGroupLayout_->addWidget(sb_smtpPort_,			2, 3);
	serverGroupLayout_->addWidget(sb_qmtpPort_,			3, 3);

	serverGroupLayout_->activate();
	
	copiesGroupLayout_->addMultiCellWidget(cb_copySelf_,	0, 0, 0, 2);
	copiesGroupLayout_->addWidget(cb_copyOther_,			1, 0);
	copiesGroupLayout_->addWidget(le_copyOther_,			1, 1);
	copiesGroupLayout_->addWidget(pb_browseCopyOther_,		1, 2);
	copiesGroupLayout_->addWidget(cb_copyFolder_,			2, 0);
	copiesGroupLayout_->addMultiCellWidget(cmb_copyFolder_,	2, 2, 1, 2);
	
	copiesGroupLayout_->activate();
	
	topLevelLayout_->activate();
}

	void
EmpathSendingSettingsDialog::saveData()
{
	KConfig * c	= kapp->getConfig();
	c->setGroup(GROUP_SENDING);
	
#define CWE c->writeEntry

	// Get server type
	int servType = (int)(
		(rb_sendmail_->isChecked()	? Sendmail	: 0) |
		(rb_qmail_->isChecked()		? Qmail		: 0) |
		(rb_smtp_->isChecked()		? SMTP		: 0) |
		(rb_qmtp_->isChecked()		? QMTP		: 0));
	
	CWE( KEY_OUTGOING_SERVER_TYPE,	servType						);
	
	CWE( KEY_SENDMAIL_LOCATION,		le_sendmail_->text()			);
	CWE( KEY_QMAIL_LOCATION,		le_qmail_->text()				);
	
	CWE( KEY_SMTP_SERVER_LOCATION,	le_smtpServer_->text()			);
	CWE( KEY_QMTP_SERVER_LOCATION,	le_qmtpServer_->text()			);

	CWE( KEY_SMTP_SERVER_PORT,		sb_smtpPort_->getValue()		);
	CWE( KEY_QMTP_SERVER_PORT,		sb_qmtpPort_->getValue()		);
	
	CWE( KEY_CC_ME,					cb_copySelf_->isChecked()		);
	CWE( KEY_CC_OTHER,				cb_copyOther_->isChecked()		);
	CWE( KEY_CC_OTHER_ADDRESS,		le_copyOther_->text()			);
	
	CWE( KEY_COPY_FOLDER,			cb_copyFolder_->isChecked()		);
	CWE( KEY_COPY_FOLDER_NAME,		cmb_copyFolder_->currentText()	);
	
	empath->updateOutgoingServer();
	
#undef CWE
}

	void
EmpathSendingSettingsDialog::loadData()
{
	KConfig * c	= kapp->getConfig();
	c->setGroup(GROUP_SENDING);
	
	OutgoingServerType t =
		(OutgoingServerType)(c->readNumEntry(KEY_OUTGOING_SERVER_TYPE));
	
	rb_sendmail_->setChecked(	t == Sendmail);
	rb_qmail_->setChecked(		t == Qmail);
	rb_smtp_->setChecked(		t == SMTP);
	rb_qmtp_->setChecked(		t == QMTP);
	
	le_sendmail_->setText(		c->readEntry(KEY_SENDMAIL_LOCATION));
	
	le_qmail_->setText(			c->readEntry(KEY_QMAIL_LOCATION));
	
	le_smtpServer_->setText(	c->readEntry(KEY_SMTP_SERVER_LOCATION));
	sb_smtpPort_->setValue(		c->readNumEntry(KEY_SMTP_SERVER_PORT));
	
	le_qmtpServer_->setText(	c->readEntry(KEY_QMTP_SERVER_LOCATION));
	sb_qmtpPort_->setValue(		c->readNumEntry(KEY_QMTP_SERVER_PORT));
	
	cb_copySelf_->setChecked(	c->readBoolEntry(KEY_CC_ME));
	cb_copyOther_->setChecked(	c->readBoolEntry(KEY_CC_OTHER));
	le_copyOther_->setText(		c->readEntry(KEY_CC_OTHER_ADDRESS));
	
	cb_copyFolder_->setChecked(	c->readEntry(KEY_COPY_FOLDER));
//	c->setCopyFolderName(cmb_copyFolder_->currentText());
	
	empath->updateOutgoingServer();
}

