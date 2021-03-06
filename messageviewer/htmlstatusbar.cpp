/*  -*- c++ -*-
    htmlstatusbar.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2002 Ingo Kloecker <kloecker@kde.org>
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "htmlstatusbar.h"
#include "global.h"
#include "globalsettings.h"

#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kapplication.h>

#include <QColor>
#include <QString>
#include <QLabel>

HtmlStatusBar::HtmlStatusBar( QWidget * parent, const char * name, Qt::WFlags f )
  : QLabel( parent, f ),
    mMode( Normal )
{
  setObjectName( name );
  setAlignment( Qt::AlignHCenter | Qt::AlignTop );
  // Don't force a minimum height to the reader widget
  setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Ignored ) );
  setAutoFillBackground( true );
  update();
}

HtmlStatusBar::~HtmlStatusBar() {}

void HtmlStatusBar::update() {
  QPalette pal = palette();
  pal.setColor( backgroundRole(), bgColor() );
  pal.setColor( foregroundRole(), fgColor() );
  setPalette( pal );
  setText( message() );
}

void HtmlStatusBar::setNormalMode() {
  setMode( Normal );
}

void HtmlStatusBar::setHtmlMode() {
  setMode( Html );
}

void HtmlStatusBar::setNeutralMode() {
  setMode( Neutral );
}

void HtmlStatusBar::setMode( Mode m ) {
  if ( m == mode() )
    return;
  mMode = m;
  update();
}

QString HtmlStatusBar::message() const {
  switch ( mode() ) {
  case Html: // bold: "HTML Message"
    return i18n( "<qt><b><br />H<br />T<br />M<br />L<br /> "
                 "<br />M<br />e<br />s<br />s<br />a<br />g<br />e</b></qt>" );
  case Normal: // normal: "No HTML Message"
    return i18n( "<qt><br />N<br />o<br /> "
                 "<br />H<br />T<br />M<br />L<br /> "
                 "<br />M<br />e<br />s<br />s<br />a<br />g<br />e</qt>" );
  default:
  case Neutral:
    return QString();
  }
}


QColor HtmlStatusBar::fgColor() const {
  KConfigGroup conf( Global::instance()->config(), "Reader" );
  QColor defaultColor, color;
  switch ( mode() ) {
  case Html:
    defaultColor = Qt::white;
    color = defaultColor;
    if ( !GlobalSettings::self()->useDefaultColors() ) {
      color = conf.readEntry( "ColorbarForegroundHTML", defaultColor );
    }
    return color;
  case Normal:
    defaultColor = Qt::black;
    color = defaultColor;
    if ( !GlobalSettings::self()->useDefaultColors() ) {
      color = conf.readEntry( "ColorbarForegroundPlain", defaultColor );
    }
    return color;
  default:
  case Neutral:
    return Qt::black;
  }
}

QColor HtmlStatusBar::bgColor() const {
  KConfigGroup conf( Global::instance()->config(), "Reader" );

  QColor defaultColor, color;
  switch ( mode() ) {
  case Html:
    defaultColor = Qt::black;
    color = defaultColor;
    if ( !GlobalSettings::self()->useDefaultColors() ) {
      color = conf.readEntry( "ColorbarBackgroundHTML", defaultColor );
    }
    return color;
  case Normal:
    defaultColor = Qt::lightGray;
    color = defaultColor;
    if ( !GlobalSettings::self()->useDefaultColors() ) {
      color = conf.readEntry( "ColorbarBackgroundPlain", defaultColor );
    }
    return color;
  default:
  case Neutral:
    return Qt::white;
  }
}

#include "htmlstatusbar.moc"
