#ifndef _KPILOT_DBSELECTIONDIALOG_H
#define _KPILOT_DBSELECTIONDIALOG_H
/* KPilot
**
** Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines a dialog box that lets the
** user select a set of databases (e.g. which databases
** should be ignored  when doing a backup)
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <kdialog.h>

class KPilotDBSelectionWidget;

class KPilotDBSelectionDialog : public KDialog
{
Q_OBJECT
public:
	KPilotDBSelectionDialog(const QStringList &selectedDBs,
		const QStringList &deviceDBs,
		const QStringList &addedDBs,  QWidget *, const char *);
	virtual ~KPilotDBSelectionDialog();

	QStringList getSelectedDBs();
	QStringList getAddedDBs() const {return fAddedDBs; };
private:
	QStringList fSelectedDBs;
	QStringList fAddedDBs;
	QStringList fDeviceDBs;
protected slots:
	void addDB();
	void removeDB();
	void textChanged( const QString& dbname);
	void dbSelectionChanged( int );
private:
	KPilotDBSelectionWidget *fSelectionWidget;
} ;

#endif
