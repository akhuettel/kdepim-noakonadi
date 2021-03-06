/*
    This file is part of libkdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KDEPIM_KCMDESIGNERFIELDS_H
#define KDEPIM_KCMDESIGNERFIELDS_H

#include "kdepim_export.h"

#define KDE3_SUPPORT
#include <KCModule>
#undef KDE3_SUPPORT

class KComponentData;
class K3ListView;

class QLabel;
class Q3ListViewItem;
class QPushButton;
class QStringList;

namespace KPIM {

class KDEPIM_EXPORT KCMDesignerFields : public KCModule
{
  Q_OBJECT

  public:
    explicit KCMDesignerFields( const KComponentData &instance, QWidget *parent=0, const QStringList &args=QStringList() );

    virtual void load();
    virtual void save();
    virtual void defaults();

  protected:
    void        loadUiFiles();
    void        loadActivePages(const QStringList&);
    QStringList saveActivePages();

    virtual QString localUiDir() = 0;
    virtual QString uiPath() = 0;
    virtual void writeActivePages( const QStringList & ) = 0;
    virtual QStringList readActivePages() = 0;
    virtual QString applicationName() = 0;

  private Q_SLOTS:
    void updatePreview( Q3ListViewItem* );
    void itemClicked( Q3ListViewItem* );
    void startDesigner();
    void rebuildList();
    void deleteFile();
    void importFile();
    void delayedInit();
    void showWhatsThis(const QString &href);

  private:
    void initGUI();

    K3ListView *mPageView;
    QLabel *mPagePreview;
    QLabel *mPageDetails;
    QPushButton *mDeleteButton;
    QPushButton *mImportButton;
    QPushButton *mDesignerButton;
};

}

#endif
