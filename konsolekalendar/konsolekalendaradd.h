#ifndef _KONSOLEKALENDAR_ADD_H_
#define _KONSOLEKALENDAR_ADD_H_

/***************************************************************************
        konsolekaledaradd.h  -  description
           -------------------
    begin                : Sun May 25 2003
    copyright            : (C) 2003 by Tuukka Pasanen
    email                : illuusio@mailcity.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "konsolekalendarvariables.h"
namespace KCal
{

class KonsoleKalendarAdd
{
public:
    KonsoleKalendarAdd( KonsoleKalendarVariables *variables );
    ~KonsoleKalendarAdd();

   bool addEvent();
   
private:
   KonsoleKalendarVariables *m_variables;
   
};

}
#endif
