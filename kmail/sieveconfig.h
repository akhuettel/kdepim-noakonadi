/*  -*- c++ -*-
    sieveconfig.h

    KMail, the KDE mail client.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef __KMAIL_SIEVECONFIG_H__
#define __KMAIL_SIEVECONFIG_H__

#include <QWidget>

#include <kurl.h>

class QCheckBox;
class QLineEdit;
class KIntSpinBox;
class KConfigBase;

namespace KMail {

  class SieveConfig {
  public:
    SieveConfig( bool managesieveSupported=false, bool reuseConfig=true,
		 unsigned int port=2000, const KUrl & alternateURL=KUrl() )
      : mManagesieveSupported( managesieveSupported ),
	mReuseConfig( reuseConfig ),
	mPort( port ),
	mAlternateURL( alternateURL ) {}

    SieveConfig( const SieveConfig & other )
      : mManagesieveSupported( other.managesieveSupported() ),
	mReuseConfig( other.reuseConfig() ),
	mPort( other.port() ),
	mAlternateURL( other.alternateURL() ),
	mVacationFileName( other.vacationFileName() ) {}

    bool managesieveSupported() const {
      return mManagesieveSupported;
    }
    void setManagesieveSupported( bool enable ) {
      mManagesieveSupported = enable;
    }

    bool reuseConfig() const {
      return mReuseConfig;
    }
    void setReuseConfig( bool reuse ) {
      mReuseConfig = reuse;
    }

    unsigned short port() const {
      return mPort;
    }
    void setPort( unsigned short port ) {
      mPort = port;
    }

    KUrl alternateURL() const {
      return mAlternateURL;
    }
    void setAlternateURL( const KUrl & url ) {
      mAlternateURL = url;
    }

    QString vacationFileName() const { return mVacationFileName; }

    void readConfig( const KConfigBase & config );
    void writeConfig( KConfigBase & config ) const;

  protected:
    bool mManagesieveSupported;
    bool mReuseConfig;
    unsigned short mPort;
    KUrl mAlternateURL;
    QString mVacationFileName;
  };

  class SieveConfigEditor : public QWidget {
    Q_OBJECT
  public:
    SieveConfigEditor( QWidget * parent=0 );

    bool managesieveSupported() const;
    virtual void setManagesieveSupported( bool enable );

    bool reuseConfig() const;
    virtual void setReuseConfig( bool reuse );

    unsigned short port() const;
    virtual void setPort( unsigned short port );

    KUrl alternateURL() const;
    virtual void setAlternateURL( const KUrl & url );

    SieveConfig config() const {
      return SieveConfig( managesieveSupported(), reuseConfig(),
			  port(), alternateURL() );
    }

    virtual void setConfig( const SieveConfig & config );

  protected slots:
    void slotEnableWidgets();

  protected:
    QCheckBox * mManagesieveCheck;
    QCheckBox * mSameConfigCheck;
    KIntSpinBox * mPortSpin;
    QLineEdit * mAlternateURLEdit;
  };

} // namespace KMail

#endif // __KMAIL_SIEVECONFIG_H__
