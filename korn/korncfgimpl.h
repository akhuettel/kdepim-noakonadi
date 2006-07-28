/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef MK_KORNCFGIMPL_H
#define MK_KORNCFGIMPL_H

/**
 * @file
 *
 * This file contains the class KornCfgImpl
 */

#include <QWidget>

class Settings;

class KConfig;
#include "ui_korncfg.h"

class KDialog;

class QObject;
class QModelIndex;
class QString;

/**
 * This class implements the KOrn's configuration dialog.
 * The dialog itself is created form a ui-file.
 */
class KornCfgImpl : public QWidget, public Ui_KornCfgWidget
{ Q_OBJECT
public:
	/**
	 * Constructor
	 *
	 * @param parent the parent of this object
	 */
	KornCfgImpl( QWidget * parent = 0 );
	/**
	 * Destructor
	 */
	virtual ~KornCfgImpl();

private slots:
	void slotDialogDestroyed();
	
	void slotEditBox();
	void slotActivated( const QModelIndex& );

	void slotOK();
	void slotCancel();
	void slotApply();

	void slotAdd();
	void slotRemove();
	void slotMoveUp();
	void slotMoveDown();
	
private:
	void readConfig();
	void writeConfig();

	void rewritePasswords();
	
	KConfig *m_config;
	Settings *m_settings;
	KDialog *m_base;
};


#endif //MK_KORNCFGIMPL_H
