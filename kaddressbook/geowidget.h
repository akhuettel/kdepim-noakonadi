/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#ifndef GEOWIDGET_H
#define GEOWIDGET_H

#include <kdialog.h>

#include "contacteditorwidget.h"

namespace KABC { class Geo; }

class GeoMapWidget;

class QCheckBox;
class QDoubleSpinBox;
class QMouseEvent;
class QPaintEvent;
class QPushButton;
class QSpinBox;

class KComboBox;

typedef struct {
  double latitude;
  double longitude;
  QString country;
} GeoData;

class GeoWidget : public KAB::ContactEditorWidget
{
  Q_OBJECT

  public:
    GeoWidget( KABC::AddressBook *ab, QWidget *parent );
    ~GeoWidget();

    void loadContact( KABC::Addressee *addr );
    void storeContact( KABC::Addressee *addr );

    void setReadOnly( bool readOnly );

  private Q_SLOTS:
    void editGeoData();

  private:
    QDoubleSpinBox *mLatitudeBox;
    QDoubleSpinBox *mLongitudeBox;

    QCheckBox *mGeoIsValid;
    QPushButton *mExtendedButton;

    bool mReadOnly;
};

class GeoDialog : public KDialog
{
  Q_OBJECT

  public:
    GeoDialog( QWidget *parent );
    ~GeoDialog();

    void setLatitude( double latitude );
    double latitude() const;

    void setLongitude( double longitude );
    double longitude() const;

  private Q_SLOTS:
    void updateInputs();

    void sexagesimalInputChanged();
    void geoMapChanged();
    void cityInputChanged();

  private:
    void loadCityList();
    double calculateCoordinate( const QString& ) const;
    int nearestCity( double, double ) const;

    GeoMapWidget *mMapWidget;
    KComboBox *mCityCombo;

    QSpinBox *mLatDegrees;
    QSpinBox *mLatMinutes;
    QSpinBox *mLatSeconds;
    KComboBox *mLatDirection;

    QSpinBox *mLongDegrees;
    QSpinBox *mLongMinutes;
    QSpinBox *mLongSeconds;
    KComboBox *mLongDirection;

    double mLatitude;
    double mLongitude;
    QMap<QString, GeoData> mGeoDataMap;
    bool mUpdateSexagesimalInput;
};

class GeoMapWidget : public QWidget
{
  Q_OBJECT

  public:
    GeoMapWidget( QWidget *parent );
    ~GeoMapWidget();

    void setLatitude( double latitude );
    double latitude()const;

    void setLongitude( double longitude );
    double longitude()const;

  Q_SIGNALS:
    void changed();

  protected:
    virtual void mousePressEvent( QMouseEvent* );
    virtual void paintEvent( QPaintEvent* );

  private:
    double mLatitude;
    double mLongitude;
};

class GeoWidgetFactory : public KAB::ContactEditorWidgetFactory
{
  public:
    KAB::ContactEditorWidget *createWidget( KABC::AddressBook *ab, QWidget *parent )
    {
      return new GeoWidget( ab, parent );
    }

    QString pageIdentifier() const { return "misc"; }
};

#endif
