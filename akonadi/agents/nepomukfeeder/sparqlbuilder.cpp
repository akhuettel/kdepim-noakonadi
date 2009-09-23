/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "sparqlbuilder.h"

#include <Soprano/Vocabulary/XMLSchema>
#include <QStringList>

static inline QString urlToString( const QUrl &url )
{
  return QLatin1Char( '<' ) + url.toString() + QLatin1Char( '>' );
}

void SparqlBuilder::TriplePattern::setSubject(const QUrl& subject)
{
  m_subject = urlToString( subject );
}

void SparqlBuilder::TriplePattern::setPredicate(const QUrl& predicate)
{
  m_predicate = urlToString( predicate );
}

void SparqlBuilder::TriplePattern::setObject(const QUrl& object)
{
  m_object = urlToString( object );
}

static inline QString valueToString( const QString &value, const QUrl &type )
{
  return QLatin1Char( '"' ) + value + QLatin1String( "\"^^" ) + urlToString( type );
}

void SparqlBuilder::TriplePattern::setObject(const QString& object)
{
  m_object = valueToString( object, Soprano::Vocabulary::XMLSchema::string() );
}

void SparqlBuilder::TriplePattern::setObject(const QVariant& object)
{
  switch ( object.type() ) {
    case QVariant::String:
      m_object = valueToString( object.toString(), Soprano::Vocabulary::XMLSchema::string() );
      break;
    case QVariant::Bool:
      if ( object.toBool() )
        m_object = valueToString( QLatin1String( "true" ), Soprano::Vocabulary::XMLSchema::boolean() );
      else
        m_object = valueToString( QLatin1String( "false" ), Soprano::Vocabulary::XMLSchema::boolean() );
      break;
    case QVariant::Int:
      m_object = valueToString( QString::number( object.toInt() ), Soprano::Vocabulary::XMLSchema::integer() );
      break;
    default:
      Q_ASSERT( "Type not supported (yet)" == false );
  }
}

void SparqlBuilder::TriplePattern::setObject(const SparqlBuilder::QueryVariable& variable)
{
  m_object = variable.name();
}



QString SparqlBuilder::TriplePattern::toString() const
{
  return m_subject + QLatin1Char( ' ' ) + m_predicate + QLatin1Char( ' ' ) + m_object;
}

QString SparqlBuilder::GraphPattern::toString() const
{
  const QString tmp = toStringInternal();
  if ( tmp.isEmpty() )
    return QString();

  QString graphPattern;
  if ( !m_name.isEmpty() )
    graphPattern += QString::fromLatin1( "GRAPH %1 " ).arg( m_name );
  if ( m_isOptional )
    graphPattern += QLatin1String( "OPTIONAL " );
  graphPattern += QLatin1String( "{ " ) + tmp + QLatin1String( " }" );
  return graphPattern;
}

QString SparqlBuilder::BasicGraphPattern::toStringInternal() const
{
  QStringList patterns;
  foreach ( const TriplePattern &triple, m_triples )
    patterns.append( triple.toString() );
  // TODO: consider value constraints
  return patterns.join( QLatin1String( " . " ) );
}


QString SparqlBuilder::GroupGraphPattern::toStringInternal() const
{
  return QLatin1String( "TODO" );
}



SparqlBuilder::SparqlBuilder()
{
}

SparqlBuilder::~SparqlBuilder()
{
}

QString SparqlBuilder::graphPattern() const
{
  if ( !m_graphPattern )
    return QString();
  const QString tmp = m_graphPattern->toString();
  if ( tmp.isEmpty() )
    return QString();
  return QLatin1String( "WHERE " ) + tmp;
}