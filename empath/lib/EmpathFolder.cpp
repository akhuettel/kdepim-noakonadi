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

// Local includes
#include "Empath.h"
#include "EmpathFolderList.h"
#include "EmpathFolder.h"
#include "EmpathDefines.h"
#include "EmpathMailbox.h"
#include "EmpathIndex.h"
#include "EmpathUtilities.h"

uID EmpathFolder::ID = 0;

EmpathFolder::EmpathFolder()
	:	QObject(),
		messageCount_(0),
		unreadMessageCount_(0),
		url_("")
{
	empathDebug("default ctor !");
	id_ = ID++;
}

EmpathFolder::EmpathFolder(const EmpathFolder & f)
	:	QObject()
{
	empathDebug("copy ctor !");
	// FIXME !
	id_ = ID++;
}

EmpathFolder::EmpathFolder(const EmpathURL & url)
	:	QObject(),
		messageCount_(0),
		unreadMessageCount_(0),
		url_(url)
{
	empathDebug("ctor");
	messageList_.setFolder(this);
	id_ = ID++;
	QObject::connect(this, SIGNAL(countUpdated(int, int)),
		empath->mailbox(url_), SLOT(s_countUpdated(int, int)));
}

	bool
EmpathFolder::operator == (const EmpathFolder & other) const
{
	empathDebug("operator == () called");
	return id_ == other.id_;
}

EmpathFolder::~EmpathFolder()
{
	empathDebug("dtor");
}

	void
EmpathFolder::setPixmap(const QPixmap & p)
{
	pixmap_ = p;
}

	void
EmpathFolder::addMessage(EmpathIndexRecord & messageDesc)
{
//	messageList_[messageDesc.id()] = messageDesc;
	//return (messageList_.append(message) && mailbox_->writeMessage(message));
}

	bool
EmpathFolder::removeMessage(const RMessageID & id)
{
	if (!messageList_.remove(id)) return false;
	EmpathMailbox * m = empath->mailbox(url_);
	return (m != 0 &&
		m->removeMessage(EmpathURL(url_.asString() + "/" + QString(id.asString()))));
}

	const EmpathIndexRecord *
EmpathFolder::messageDescription(const RMessageID & id) const
{
	empathDebug("messageWithID(" + id.asString() + ") called");
	return messageList_.messageDescription(id);
}

	RMessage *
EmpathFolder::message(const RMessageID & id)
{
	EmpathMailbox * m = empath->mailbox(url_);
	if (m == 0) return 0;
	return m->message(EmpathURL(url_.asString() + "/" + QString(id.asString())));
}

	void
EmpathFolder::update()
{
	empathDebug("update() called");
	EmpathMailbox * m = empath->mailbox(url_);
	if (m == 0) return;
	empathDebug("mailbox name = " + m->name());
	m->readMailForFolder(this);
	emit(countUpdated(messageList_.countUnread(), messageList_.count()));
}

	bool
EmpathFolder::writeMessage(const RMessage & message)
{
	EmpathMailbox * m = empath->mailbox(url_);
	return (m != 0 && m->writeMessage(this, message));
}

	EmpathFolder *
EmpathFolder::parent() const
{
	// FIXME !
	return 0;
}

