/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2001,2002,2003,2004 by Adriaan de Groot
**
** This is the main program for kpilotTest, which shows a SyncLog and
** exercises the KPilotDeviceLink class. It's intended to test if the
** Palm hardware and the KPilot software are functioning correctly to
** some extent.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/

#include "options.h"

#include <stdlib.h>
#include <time.h>
#include <iostream>

#include <qpushbutton.h>
#include <q3hbox.h>
#include <qtimer.h>

#include <kapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kservice.h>
#include <kservicetype.h>
#include <KServiceTypeTrader>
#include <pi-version.h>

#include "actionQueue.h"
#include "actions.h"
#include "kpilotdevicelink.h"
#include "kpilotlocallink.h"
#include "pilot.h"

#include "kpilotConfig.h"
#include "hotSync.h"


/**
*** Conduits - sync actions - for testing specific scenarios.
**/



KPilotLink *createLink( bool local )
{
	FUNCTIONSETUP;
	if (!local)
	{
		return new KPilotDeviceLink(0, "deviceLink");
	}
	else
	{
		return new KPilotLocalLink(0, "localLink");
	}
}

/** If @p loop is true, then instead of quitting at end of
*   sync, wait for a new sync just like the real daemon does.
*/
void connectStack( KPilotLink *l, ActionQueue *a, bool loop = false )
{
	FUNCTIONSETUP;

	if (l && a)
	{
		QObject::connect(a, SIGNAL(syncDone(SyncAction *)),
			l, SLOT(close()));
		if (!loop)
		{
			QObject::connect(a, SIGNAL(syncDone(SyncAction *)),
				kapp, SLOT(quit()));
		}
		else
		{
			QObject::connect(a, SIGNAL(syncDone(SyncAction *)),
				l, SLOT(reset()));
		}
		QObject::connect(l, SIGNAL(deviceReady(KPilotLink*)),
			a, SLOT(execConduit()));
	}
}



int exec(const QString &device, const QString &what, KCmdLineArgs *p)
{
	FUNCTIONSETUP;

	// get --exec-conduit value
	if (what.isEmpty()) return 1;
	QStringList l;
	l.append(what);

	SyncAction::SyncMode::Mode syncMode = SyncAction::SyncMode::eHotSync;
	if (p->isSet("HHtoPC")) syncMode = SyncAction::SyncMode::eCopyHHToPC;
	if (p->isSet("PCtoHH")) syncMode = SyncAction::SyncMode::eCopyPCToHH;
	SyncAction::SyncMode mode(syncMode,p->isSet("test"),p->isSet("local"));

	KPilotLink *link = createLink( p->isSet("local") );
	ActionQueue *syncStack = new ActionQueue( link );
	syncStack->queueInit();
	syncStack->addAction(new CheckUser( link ));
	syncStack->queueConduits(l,mode);
	syncStack->queueCleanup();
	connectStack(link,syncStack);
	link->reset(device);
	return kapp->exec();
}

int backup(const QString &device, const QString &what, KCmdLineArgs *p)
{
	FUNCTIONSETUP;
	KPilotLink *link = createLink( p->isSet("local") );
	ActionQueue *syncStack = new ActionQueue( link );
	syncStack->queueInit();
	BackupAction *ba = new BackupAction( link, true /* full backup */ );
	ba->setDirectory( what );
	syncStack->addAction( ba );
	syncStack->queueCleanup();
	connectStack(link,syncStack);
	link->reset(device);
	return kapp->exec();
}

int restore(const QString &device, const QString &what, KCmdLineArgs *p)
{
	FUNCTIONSETUP;
	KPilotLink *link = createLink( p->isSet("local") );
	ActionQueue *syncStack = new ActionQueue( link );
	syncStack->queueInit();
	RestoreAction *ra = new RestoreAction( link );
	ra->setDirectory( what );
	syncStack->addAction( ra );
	syncStack->queueCleanup();
	connectStack(link,syncStack);
	link->reset(device);
	return kapp->exec();
}

int listDB(const QString &device, KCmdLineArgs *p)
{
	FUNCTIONSETUP;
	KPilotLink *link = createLink( p->isSet("local") );
	ActionQueue *syncStack = new ActionQueue( link );
	syncStack->queueInit();
	syncStack->addAction( new TestLink( link ) );
	syncStack->queueCleanup();
	connectStack(link,syncStack, p->isSet("loop") );
	link->reset(device);
	return kapp->exec();
}

int check( const QString &device, const QString &what, KCmdLineArgs *p )
{
	FUNCTIONSETUP;

	if ( "help" == what )
	{
		std::cout <<
"You can use the --check option to kpilotTest to run various\n"
"small checks that require the use of the device. These are:\n"
"\thelp - show this help\n"
"\tuser - check the user name on the handheld\n"
		<< std::endl;
		return 0;
	}

	if ( "user" == what )
	{
		KPilotLink *link = createLink( p->isSet("local") );
		ActionQueue *syncStack = new ActionQueue( link );
		syncStack->queueInit();
		syncStack->addAction( new CheckUser( link ) );
		syncStack->queueCleanup();
		connectStack(link,syncStack);
		link->reset(device);
		return kapp->exec();
	}

	return 0;
}

void listConduits()
{
	FUNCTIONSETUP;
        KService::List offers = KServiceTypeTrader::self()->query( CSL1("KPilotConduit") );
        // Now actually fill the two list boxes, just make
        // sure that nothing gets listed in both.
        //
        //   
        KService::List::ConstIterator e = offers.end();
        for (KService::List::ConstIterator i = offers.begin(); i!=e; ++i)
        {
                const KService::Ptr o = *i;

		std::cout << "File:   " << o->desktopEntryName() << std::endl;
		std::cout << "  Desc: " << o->name()  << std::endl;
		
                if (!o->library().isEmpty())
                {
			std::cout << "  Lib : "
				<< o->library()
				<< std::endl;
                }
        }



}

int show( const QString &what )
{
	FUNCTIONSETUP;

	if ( "help" == what )
	{
		std::cout <<
"Displays various bits of KPilot's internal settings. This\n"
"does not require a device connection or a running KDE desktop.\n"
"No change to data takes place. The following options are available\n"
"for display:\n"
"\thelp     - displays this help\n"
"\tconduits - displays the list of available conduits\n"
"\tuser     - displays the user name KPilot expects\n"
"\tdevice   - displays the device settings in KPilot\n"
"\tdebug    - displays internal numbers\n"
		<< std::endl;
		return 0;
	}

	if ( "conduits" == what )
	{
		listConduits();
		return 0;
	}

	if ( "user" == what )
	{
		std::cout << "User: " << KPilotSettings::userName() << std::endl;
		return 0;
	}

	if ( "device" == what )
	{
		std::cout << "Device:   " << KPilotSettings::pilotDevice()
			<< "\nSpeed:    " << KPilotSettings::pilotSpeed()
			<< "\nEncoding: " << KPilotSettings::encoding()
			<< "\nQuirks:   " << KPilotSettings::workarounds()
			<< std::endl;
		return 0;
	}

	if ( "debug" == what )
	{
		std::cout << "Debug:  " << KPilotSettings::debug()
			<< "\nConfig: " << KPilotSettings::configVersion()
			<< std::endl;
		return 0;
	}

	std::cerr << "Unknown --show argument, use --show help for help.\n";
	return 1;
}

int main(int argc, char **argv)
{
	debug_level = 1;
	FUNCTIONSETUP;
	KAboutData about("kpilotTest", 0,
		ki18n("KPilotTest"),
		KPILOT_VERSION,
		ki18n("KPilot Tester"),
		KAboutData::License_GPL, ki18n("(C) 2001-2004, Adriaan de Groot"));
	KPILOT_ABOUT_INIT(about);

	KCmdLineArgs::init(argc, argv, &about);

	KCmdLineOptions generalOptions;
	generalOptions.add("p");
	generalOptions.add("port <device>", ki18n("Path to Pilot device node"), "/dev/pilot");
	generalOptions.add("l");
	generalOptions.add("list", ki18n("List DBs"));
	generalOptions.add("b");
	generalOptions.add("backup <dest dir>", ki18n("Backup Pilot to <dest dir>"));
	generalOptions.add("r");
	generalOptions.add("restore <src dir>", ki18n("Restore Pilot from backup"));
	generalOptions.add("e");
	generalOptions.add("exec <filename>", ki18n("Run conduit from desktop file <filename>"));
	generalOptions.add("c");
	generalOptions.add("check <what>", ki18n("Run a specific check (with the device)"), "help");
	generalOptions.add("s");
	generalOptions.add("show <what>", ki18n("Show KPilot configuration information"), "help");
#ifdef DEBUG
	generalOptions.add("debug <level>", ki18n("Set the debug level"), "1");
#endif
	KCmdLineArgs::addCmdLineOptions(generalOptions, ki18n("General"));

	KCmdLineOptions conduitOptions;
	conduitOptions.add("T");
	conduitOptions.add("notest", ki18n("*Really* run the conduit, not in test mode."));
	conduitOptions.add("F");
	conduitOptions.add("local", ki18n("Run the conduit in file-test mode."));
	conduitOptions.add("HHtoPC", ki18n("Copy Pilot to Desktop."));
	conduitOptions.add("PCtoHH", ki18n("Copy Desktop to Pilot."));
	conduitOptions.add("loop", ki18n("Repeated perform action - only useful for --list"));
	KCmdLineArgs::addCmdLineOptions(conduitOptions,
		ki18n("Conduit Actions"),"conduit");

	KCmdLineArgs::addStdCmdLineOptions();

	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();

	bool needGUI = false;

	// Some versions need a GUI
	needGUI |= (p->isSet("check"));
	needGUI |= (p->isSet("exec")); // assume worst wrt. conduits
	needGUI |= (p->isSet("restore"));

	KApplication a(needGUI);
	KPilotConfig::getDebugLevel(p);
	DEBUGKPILOT  << "Created KApplication.";

	Pilot::setupPilotCodec(KPilotSettings::encoding());

	QString device( "/dev/pilot" );

	if ( p->isSet("port") )
	{
		device = p->getOption("port");
	}

	if ( p->isSet("check") )
	{
		return check( device, p->getOption("check"),
			KCmdLineArgs::parsedArgs("conduit") );
	}

	if ( p->isSet("show") )
	{
		return show( p->getOption("show") );
	}

	if ( p->isSet("exec") )
	{
		return exec( device, p->getOption("exec"),
			KCmdLineArgs::parsedArgs("conduit") );
	}

	if ( p->isSet("list") )
	{
		return listDB( device,
			KCmdLineArgs::parsedArgs("conduit") );
	}

	if ( p->isSet("backup") )
	{
		return backup( device, p->getOption("backup"),
			KCmdLineArgs::parsedArgs("conduit") );
	}

	if ( p->isSet("restore") )
	{
		return restore( device, p->getOption("restore"),
			KCmdLineArgs::parsedArgs("conduit") );
	}



	std::cout <<
"Usage: kpilotTest [--port devicename] action\n\n"
"Where action can be one of:\n"
"\t--list - list the databases on the handheld\n"
"\t--show (help | conduits | ...) - show configuration\n"
"\t--check (help | user | ...) - check device\n"
"\t--exec conduit - run a single conduit\n"
"\t--backup - backup the device\n"
"\t--restore - restore the device from backup\n"
	<< std::endl;
	return 1;
}


