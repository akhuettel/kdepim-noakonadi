/***************************************************************************
			  main.cpp  -  description
			     -------------------
    begin                : Sun Jan  6 11:50:14 EET 2002
    copyright            : (C) 2002 by Tuukka Pasanen
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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <qdatetime.h>

#include <stdlib.h>
#include <iostream>

#include "konsolekalendar.h"

using namespace KCal;
using namespace std;

static const char *description = I18N_NOOP("KonsoleKalendar");
	
static KCmdLineOptions options[] =
{
  { "help", I18N_NOOP("Prints this help"), 0 },
  { "verbose", I18N_NOOP("Output helpful (?) debug info"), 0 },
  { "file <calendar file>", I18N_NOOP("Specify which calendar you want to use."), 0 },
  { "next", I18N_NOOP("Next activity in calendar"), 0 },
  { "date <date>", I18N_NOOP("Show selected day's calendar"), 0 },
  { "time <time>", I18N_NOOP("Show selected time at calendar"), 0 }, 
  { "start-date <start-date>", I18N_NOOP("Start from this day"), 0 },
  { "start-time <start-time>", I18N_NOOP("Start from this time [mm:hh]"), 0 },
  { "end-date <end-date>", I18N_NOOP("End to this day"), 0 },
  { "end-time <end-time>", I18N_NOOP("End to this time [mm:hh]"), 0 },
  { "epoch-end <epoch-time>", I18N_NOOP("End time in epoch format"), 0 },
  { "epoch-start <epoch-time>", I18N_NOOP("Start time in epoch format"), 0 },
  { "description <description>", I18N_NOOP("Add description to event (works with add and change)"), 0 },
  { "all", I18N_NOOP("Show all entries"), 0 },
  { "add", I18N_NOOP("Add an event"), 0 },
  { "change", I18N_NOOP("Delete an event (currently not implemented)"), 0 },
  { "delete", I18N_NOOP("Delete an event (currently not implemented)"), 0 },
  KCmdLineLastOption
};

int main(int argc, char *argv[])
{
  KAboutData aboutData( "konsolekalendar", I18N_NOOP( "KonsoleKalendar" ),
                        "0.5", description, KAboutData::License_GPL,
                        "(c) 2002-2003, Tuukka Pasanen", 0, 0,
                        "illuusio@mailcity.com");
  aboutData.addAuthor("Tuukka Pasanen",0, "illuusio@mailcity.com");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QString KalendarFile;
  QDate startdate = QDate::currentDate();
  QTime starttime(7,0);
  
  QDate enddate = QDate::currentDate();
  QTime endtime(17,0);

	QDate date = QDate::currentDate();
	QTime time = QTime::currentTime();
	
	
  QString option;

  KApplication app( false, false );
	
  KonsoleKalendarVariables variables;

  if ( args->isSet("verbose") )
  {
     variables.setVerbose(true);
  }

  /*
   *  Show next happening and exit
   *
   */
  if ( args->isSet("next") ) 
  {
    
    if(variables.isVerbose()) 
    {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Show next happening in calendar and exit" << endl;
    }
    variables.setNext(true);
  }

  /*
   *  If we like to see some date
   *
   */
  if ( args->isSet("date") ) 
  {
    option = args->getOption("date");
  
    if(variables.isVerbose()) 
    {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Show date info and exit: (" << option << ")" << endl;             
    }

    date = variables.parseDate(option);

    //variables.setDate(date);
  } 

  /*
   *  If we like to see some time
   *
   */
  if ( args->isSet("time") ) 
  {
    option = args->getOption("time");
  
    if(variables.isVerbose()) 
    {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Show date info and exit: (" << option << ")" << endl;             
    }

    time = variables.parseTime(option);

    //variables.setDate(date);
  } 

	
  /*
   *  Set starting date for calendar
   *
   */
  if ( args->isSet("start-date") ) 
  {
    option = args->getOption("start-date");
    
    if(variables.isVerbose())
    {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Start date: (" << option << ")" << endl;              
    }

    startdate = variables.parseDate(option);
	
    //variables.setStartDate(date);
  }


  /*
   *  Set starting time
   *
   */
  if ( args->isSet("start-time") ) 
  {
    option = args->getOption("start-time");
    
    if(variables.isVerbose())
    {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | Start time: (" << option << ")" << endl;              
    }

    starttime = variables.parseTime(option);
	
    //variables.setStartDate(date);
  }
	
  /*
   *  Set end date for calendar
   *
   */
  
  if ( args->isSet("end-date") ) 
  {
    QString option = args->getOption("end-date");
    
    if(variables.isVerbose()) 
    {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | End date: (" << option << ")" << endl;              
    }

    enddate = variables.parseDate(option);

    //variables.setEndDate(date);
  }

  /*
   *  Set ending time
   *
   */
  if ( args->isSet("end-time") ) 
  {
    option = args->getOption("start-time");
    
    if(variables.isVerbose())
    {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | End time: (" << option << ")" << endl;              
    }

    endtime = variables.parseTime(option);
	
    // variables.setStartDate(date);
  }

	
	

  if( args->isSet("all") ) 
  {
    variables.setAll( true );
  } 
  else 
  {
    variables.setAll( false );
  } // else

  if ( args->isSet("file") ) 
  {
    option = args->getOption("file");
    variables.setCalendarFile(option);

    if(variables.isVerbose())
    {
      kdDebug() << "main.cpp::int main(int argc, char *argv[]) | using calendar at: (" << variables.getCalendarFile() << ")" << endl;              
    } // if verbose   
  
  } 
  else 
  {
    KConfig cfg( locate( "config", "korganizerrc" ) );

    cfg.setGroup("General");
    KURL url( cfg.readEntry("Active Calendar") );
    if ( url.isLocalFile() ) 
    {
      KalendarFile = url.path();
    
      variables.setCalendarFile(KalendarFile);
	
      if(variables.isVerbose())
      {
        cout << "main.cpp::int main(int argc, char *argv[]) | Calendar file currently is " << variables.getCalendarFile().local8Bit() << endl;
      } // if verbose
	    
    } 
    else 
    {
      cout << i18n("Remote files are not supported yet.").local8Bit() << endl;
    } // else
  } // else 

  args->clear(); // Free up some memory.

  //variables->setCalendarFile(KalendarFile);

	
	
	
	QDateTime startdatetime(startdate, starttime);
	QDateTime enddatetime(enddate, endtime);
	QDateTime datetime( date, time );
	
	variables.setStartDate( startdatetime );
	variables.setEndDate( enddatetime );
	variables.setDate( datetime );
	
	
  KonsoleKalendar *konsolekalendar = new KonsoleKalendar(variables);

	  /*
	   * Opens calendar file so we can use it;)
	   * Because at this point we don't know what well
	   * Do with it..
	   * 
	   * Adds it to konsolekalendarvariables also..
	   */  
	konsolekalendar->openCalendar();

	
	konsolekalendar->showInstance();

  delete konsolekalendar;

  if(variables.isVerbose()){
    kdDebug() << "main.cpp::int main(int argc, char *argv[]) | exiting" << endl;
  }

  return 0;
}
