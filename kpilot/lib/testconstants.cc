/* testconstants KPilot
**
** Copyright (C) 2005 by Adriaan de Groot <groot@kde.org)
**
** Checks that various data structures are sized properly.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#define private public
#define protected public

#include "pilotDatabase.h"

#include <pi-appinfo.h>

int main(int argc, char **argv)
{
	PilotAppInfoBase info;

	kdDebug() << "*** Sizes of structures" << endl;
	kdDebug() << "AppInfoBase: " << sizeof(PilotAppInfoBase) << endl;
	kdDebug() << "CategoryInfo: " << sizeof(info.categoryInfo()) << endl;
	kdDebug() << "CategoryInfo: " << sizeof(*info.categoryInfo()) << endl;
	kdDebug() << "Category names: " << sizeof(info.categoryInfo()->name) << endl;
	kdDebug() << "Single category: " << sizeof(info.categoryInfo()->name[0]) << endl;

	if ( sizeof(info.categoryInfo()->name[0]) != 16 ) return 1;
	if ( sizeof(info.categoryInfo()->name) / sizeof(info.categoryInfo()->name[0]) != 16 ) return 1;
	return 0;
}


