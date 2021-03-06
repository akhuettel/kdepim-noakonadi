/*******************************************************************************
**
** Filename   : util
** Created on : 03 April, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : <adam@kde.org>
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   It is distributed in the hope that it will be useful, but
**   WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**   General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
**
*******************************************************************************/

#include "util.h"

#include <kcharsets.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kdebug.h>

#include <QTextCodec>
#include <QWidget>

bool Util::checkOverwrite( const KUrl &url, QWidget *w )
{
  if ( KIO::NetAccess::exists( url, KIO::NetAccess::DestinationSide, w ) ) {
    if ( KMessageBox::Cancel == KMessageBox::warningContinueCancel(
         w,
         i18n( "A file named \"%1\" already exists. "
             "Are you sure you want to overwrite it?", url.prettyUrl() ),
             i18n( "Overwrite File?" ),
                   KStandardGuiItem::overwrite() ) )
      return false;
  }
  return true;
}

#ifdef Q_WS_MACX
#include <QDesktopServices>
#endif

bool Util::handleUrlOnMac( const KUrl& url )
{
#ifdef Q_WS_MACX
  QDesktopServices::openUrl( url );
  return true;
#else
  Q_UNUSED( url );
  return false;
#endif
}

QStringList Util::supportedEncodings(bool usAscii)
{
  // cberzan: replaced by KCodecAction in CodecManager
  QStringList encodingNames = KGlobal::charsets()->availableEncodingNames();
  QStringList encodings;
  QMap<QString,bool> mimeNames;
  for (QStringList::Iterator it = encodingNames.begin();
    it != encodingNames.end(); ++it)
  {
    QTextCodec *codec = KGlobal::charsets()->codecForName(*it);
//     kDebug() << "name" << *it << "codec" << codec << "name" << (codec ? codec->name() : "NULL");
    QString mimeName = (codec) ? QString(codec->name()).toLower() : (*it);
    if (!mimeNames.contains(mimeName) )
    {
      encodings.append( KGlobal::charsets()->descriptionForEncoding(*it) );
      mimeNames.insert( mimeName, true );
//       kDebug() << "added" << mimeName;
    }
  }
  encodings.sort();
  if (usAscii)
    encodings.prepend(KGlobal::charsets()->descriptionForEncoding("us-ascii") );
  return encodings;
}

//-----------------------------------------------------------------------------
QString Util::fixEncoding( const QString &encoding )
{
  QString returnEncoding = encoding;
  // According to http://www.iana.org/assignments/character-sets, uppercase is
  // preferred in MIME headers
  if ( returnEncoding.toUpper().contains( "ISO " ) ) {
    returnEncoding = returnEncoding.toUpper();
    returnEncoding.replace( "ISO ", "ISO-" );
  }
  return returnEncoding;
}

//-----------------------------------------------------------------------------
QString Util::encodingForName( const QString &descriptiveName )
{
  QString encoding = KGlobal::charsets()->encodingForName( descriptiveName );
  return Util::fixEncoding( encoding );
}

