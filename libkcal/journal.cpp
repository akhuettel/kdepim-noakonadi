/*
    This file is part of libkcal.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include "journal.h"

using namespace KCal;

Journal::Journal()
{
}

Journal::~Journal()
{
}

Journal *Journal::clone()
{
  return new Journal( *this );
}


bool Journal::operator==( const Journal& j2 ) const
{
    return static_cast<const Incidence&>(*this) == static_cast<const Incidence&>(j2);
}

