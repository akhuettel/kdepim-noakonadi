/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

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

#include "opiecategories.h"

OpieCategories::OpieCategories()
{

}
OpieCategories::OpieCategories(const QString &id, const QString &name, const QString &app )
{
    m_name = name;
    m_id = id;
    m_app = app;
}
OpieCategories::OpieCategories(const OpieCategories &op )
{
    (*this) = op;
}
QString OpieCategories::id() const
{
    return m_id;
}
QString OpieCategories::name() const
{
    return m_name;
}
QString OpieCategories::app() const
{
    return m_app;
}
OpieCategories &OpieCategories::operator=(const OpieCategories &op )
{
    m_name = op.m_name;
    m_app = op.m_app;
    m_id = op.m_id;
    return (*this);
}


bool operator== (const OpieCategories& a,  const OpieCategories &b )
{
    if ( a.id() == b.id() && a.name() == b.name() && a.app() == b.app() )
        return true;
    return false;
}





