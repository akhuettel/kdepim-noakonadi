/****************************************************************************
 ** Copyright (C) 2001-2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Gantt library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/
#ifndef KDGANTTABSTRACTGRID_P_H
#define KDGANTTABSTRACTGRID_P_H

#include <QPointer>
#include <QAbstractItemModel>
#include <QPersistentModelIndex>

#include "kdganttabstractgrid.h"

namespace KDGantt {
    class AbstractGrid::Private {
    public:
        QPointer<QAbstractItemModel> model;
        QPersistentModelIndex root;
    };

    inline AbstractGrid::AbstractGrid( Private* d ) : _d( d ) {}
}

#endif /* KDGANTTABSTRACTGRID_P_H */

