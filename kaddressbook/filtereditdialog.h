/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef FILTEREDITDIALOG_H
#define FILTEREDITDIALOG_H

#include <kdialog.h>

#include "filter.h"

class QButtonGroup;
class QString;
class QWidget;

class KLineEdit;
class KListWidget;

namespace KPIM {
class CategorySelectWidget;
}

class FilterDialog : public KDialog
{
  Q_OBJECT

  public:
    FilterDialog( QWidget *parent );
    ~FilterDialog();

    void setFilters( const Filter::List &list );
    Filter::List filters() const;

  protected Q_SLOTS:
    void add();
    void edit();
    void remove();
    void selectionChanged();

  private:
    void initGUI();
    void refresh();

    Filter::List mFilterList;
    Filter::List mInternalFilterList;

    KListWidget *mFilterListBox;
    QPushButton *mAddButton;
    QPushButton *mEditButton;
    QPushButton *mRemoveButton;
};

class FilterEditDialog : public KDialog
{
    Q_OBJECT
  public:
    FilterEditDialog( QWidget *parent );
    ~FilterEditDialog();

    void setFilter( const Filter &filter );
    Filter filter();

  protected Q_SLOTS:
    void filterNameTextChanged( const QString& );
    void slotHelp();

  private:
    void initGUI();

    Filter mFilter;

    KLineEdit *mNameEdit;
    KPIM::CategorySelectWidget *mCategoriesView;
    QButtonGroup *mMatchRuleGroup;
    QPushButton *mEditButton;
    QPushButton *mRemoveButton;
};

#endif
