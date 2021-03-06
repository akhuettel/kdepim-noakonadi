/*
    This file is part of KAddressbook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef CUSTOMFIELDSWIDGET_H
#define CUSTOMFIELDSWIDGET_H

#include <kabc/addressee.h>
#include <kdialog.h>
#include <klocale.h>

#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtGui/QWidget>

#include "contacteditorwidget.h"

class QCheckBox;
class QFrame;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QVBoxLayout;

class KComboBox;
class KLineEdit;

typedef struct {
  QString mIdentifier;
  QString mTitle;
  bool mGlobal;

  QLabel *mLabel;
  QWidget *mWidget;
  QHBoxLayout *mLayout;
} FieldRecord;

typedef QList<FieldRecord> FieldRecordList;

class AddFieldDialog : public KDialog
{
  Q_OBJECT

  public:
    AddFieldDialog( QWidget *parent );

    QString title() const;
    QString identifier() const;
    QString type() const;
    bool isGlobal() const;

  private Q_SLOTS:
    void nameChanged( const QString& );

  private:
    KLineEdit *mTitle;
    KComboBox *mType;
    QCheckBox *mGlobal;

    QVector<QString> mTypeList;
    QVector<QString> mTypeName;
};

class FieldWidget : public QWidget
{
  Q_OBJECT

  public:
    FieldWidget( QWidget *parent );

    void addField( const QString &identifier, const QString &title,
                   const QString &type, bool isGlobal );

    void removeField( const QString &identifier );

    void loadContact( KABC::Addressee *addr );
    void storeContact( KABC::Addressee *addr );
    void setReadOnly( bool readOnly );

    FieldRecordList fields() const { return mFieldList; }

    void removeLocalFields();
    void clearFields();

  Q_SIGNALS:
    void changed();

  private:
    void recalculateLayout();

    QVBoxLayout *mGlobalLayout;
    QVBoxLayout *mLocalLayout;
    QFrame *mSeparator;

    FieldRecordList mFieldList;
};

class CustomFieldsWidget : public KAB::ContactEditorWidget
{
  Q_OBJECT

  public:
    CustomFieldsWidget( KABC::AddressBook *ab, QWidget *parent );

    void loadContact( KABC::Addressee *addr );
    void storeContact( KABC::Addressee *addr );
    void setReadOnly( bool readOnly );

  private Q_SLOTS:
    void addField();
    void removeField();

  private:
    void initGUI();

    QStringList marshallFields( bool ) const;

    QPushButton *mAddButton;
    QPushButton *mRemoveButton;
    QGridLayout *mLayout;

    FieldWidget *mFieldWidget;

    KABC::Addressee mAddressee;
};

class CustomFieldsWidgetFactory : public KAB::ContactEditorWidgetFactory
{
  public:
    KAB::ContactEditorWidget *createWidget( KABC::AddressBook *ab, QWidget *parent )
    {
      return new CustomFieldsWidget( ab, parent );
    }

    QString pageTitle() const { return i18n( "Custom Fields" ); }
    QString pageIdentifier() const { return "custom_fields"; }
};

void splitField( const QString&, QString&, QString&, QString& );

#endif
