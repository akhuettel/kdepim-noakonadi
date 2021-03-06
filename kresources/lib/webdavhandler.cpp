/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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
*/

#include <QDateTime>

#include <ksystemtimezone.h>
#include <kdebug.h>

#include "webdavhandler.h"

WebdavHandler::WebdavHandler()
{
}


KUrl WebdavHandler::toDAV( const KUrl& url )
{
  KUrl result( url );

  if ( result.protocol() == "http" ) {
    if ( result.port() == 443 ) {
      // OpenGroupware.org returns 'http://server:443' instead of 'https://server'
      result.setProtocol( "webdavs" );
      result.setPort( 0 );
    } else
      result.setProtocol( "webdav" );
  } else if ( result.protocol() == "https" )
    result.setProtocol( "webdavs" );

  return result;
}



QDomElement WebdavHandler::addElement( QDomDocument &doc, QDomNode &node,
                                       const QString &tag, const QString &value )
{
  QDomElement el = doc.createElement( tag );
  node.appendChild( el );
  if ( !value.isNull() ) {
    QDomText txt = doc.createTextNode( value );
    el.appendChild( txt );
  }
  return el;
}

QDomElement WebdavHandler::addElementNS( QDomDocument &doc, QDomNode &node,
                                       const QString &ns, const QString &tag, const QString &value )
{
  QDomElement el = doc.createElementNS( ns, tag );
  node.appendChild( el );
  if ( !value.isNull() ) {
    QDomText txt = doc.createTextNode( value );
    el.appendChild( txt );
  }
  return el;
}

QDomElement WebdavHandler::addDavElement( QDomDocument &doc, QDomNode &node,
                                          const QString &tag, const QString &value )
{
  return addElementNS( doc, node, "DAV:", tag, value );
}

bool WebdavHandler::extractBool( const QDomElement &node, const QString &entry, bool &value )
{
  QDomElement element = node.namedItem( entry ).toElement();
  if ( !element.isNull() ) {
    value = (element.text() != "0");
    return true;
  }
  return false;
}

bool WebdavHandler::extractLong( const QDomElement &node, const QString &entry, long &value )
{
  QDomElement element = node.namedItem( entry ).toElement();
  if ( !element.isNull() ) {
    value = element.text().toLong();
    return true;
  }
  return false;
}

bool WebdavHandler::extractFloat( const QDomElement &node, const QString &entry, float &value )
{
  QDomElement element = node.namedItem( entry ).toElement();
  if ( !element.isNull() ) {
    value = element.text().toFloat();
    return true;
  }
  return false;
}

bool WebdavHandler::extractDateTime( const QDomElement &node, const QString &entry, QDateTime &value )
{
  QDomElement element = node.namedItem( entry ).toElement();
  if ( !element.isNull() && !element.text().isEmpty() ) {
    value = QDateTime::fromString( element.text(), Qt::ISODate );
    return true;
  }
  return false;
}

bool WebdavHandler::extractString( const QDomElement &node, const QString &entry, QString &value )
{
  QDomElement element = node.namedItem( entry ).toElement();
  if ( !element.isNull() ) {
    value = element.text();
    return true;
  }
  return false;
}

bool WebdavHandler::extractStringList( const QDomElement &node, const QString &entry, QStringList &value )
{
  QDomElement element = node.namedItem( entry ).toElement();
  if ( !element.isNull() ) {
    value.clear();
    QDomNodeList list = element.elementsByTagNameNS( "xml:", "v" );
    for( uint i=0; i < list.length(); i++ ) {
      QDomElement item = list.item(i).toElement();
      value.append( item.text() );
    }
    return true;
  }
  return false;
}


const QString WebdavHandler::getEtagFromHeaders( const QString& headers )
{
  int start = headers.indexOf( "etag:", 0, Qt::CaseInsensitive );
  if ( start < 0 ) return QString();
  start += 6;
  return headers.mid( start, headers.indexOf( "\n", start ) - start );
}

QDateTime WebdavHandler::utcAsZone( const QDateTime& utc, const QString& timeZoneId )
{
  if ( utc.timeSpec() != Qt::UTC ) {
    kError() <<"WebdavHandler::utcAsZone(): non-UTC time supplied";
    return utc;
  }
  KTimeZone tz = KSystemTimeZones::zone( timeZoneId );
  if ( !tz.isValid() ) {
    // Default to no conversion when time zone not recognized
    QDateTime local = utc;
    local.setTimeSpec( Qt::LocalTime );
    return local;
  }
  return tz.toZoneTime( utc );
}

QDateTime WebdavHandler::zoneAsUtc( const QDateTime& zone, const QString& timeZoneId )
{
  if ( zone.timeSpec() == Qt::UTC ) {
    kError() <<"WebdavHandler::zoneAsUtc(): UTC time supplied";
    return zone;
  }
  KTimeZone tz = KSystemTimeZones::zone( timeZoneId );
  if ( !tz.isValid() ) {
    // Default to no conversion when time zone not recognized
    QDateTime utc = zone;
    utc.setTimeSpec( Qt::UTC );
    return utc;
  }
  return tz.toUtc( zone );
}
