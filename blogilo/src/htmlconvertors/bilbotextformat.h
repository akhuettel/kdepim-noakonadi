/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#ifndef BILBOTEXTFORMAT_H
#define BILBOTEXTFORMAT_H

#include <qtextformat.h>

/**
 * Adds some textformat attributes which don't exist in QTextFormat class.
 * this class may be removed in future, if all editor related staff be ordered as a lib.
 *
 @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 @author Golnaz Nilieh <g382nilieh@gmail.com>
*/
// class BilboTextCharFormat : public QTextCharFormat

class BilboTextFormat
{
public:

    enum Property {
        /// Anchor properties
        AnchorTitle = 0x100010,
        AnchorTarget = 0x100011,

        /// Image properties
        ImageTitle = 0x100020,
        ImageAlternateText = 0x100021,
        
        HasCodeStyle = 0x100030,

        /// Block Properties
        HtmlHeading = 0x100040, //zero if block is not in heading format, 1 for Heading 1, and so on.
        IsBlockQuote = 0x100041,

        IsHtmlTagSign = 0x100042
    };

};

#endif