/* perl-conduit.cc			KPilot
**
** Copyright (C) 2004 by Adriaan de Groot
**
** This file is part of the Perl conduit, a conduit for KPilot that
** is intended to showcase how to use perl code inside a conduit.
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

#ifdef DEBUG
#undef DEBUG
#define DEBUG (1)
#else
#define DEBUG (1)
#endif

#include "options.h"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *perl_conduit_id=
	"$Id$";

#include "perl-conduit.h"  // The Conduit action
#include "perlconduit.h"   // The settings class

#include <qthread.h>
#include <qapplication.h>

#include <EXTERN.h>
#include <perl.h>

class PerlThread : public QThread
{
public:
	PerlThread(QObject *parent) : fParent(parent) { } ;
	virtual void run();

	QString result() const { return fResult; } ;

protected:
	QObject *fParent;
	PerlInterpreter *fPerl;

	QString fResult;
} ;


PerlConduit::PerlConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &l) :
	ConduitAction(d,n,l)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT<<perl_conduit_id<<endl;
#endif
	fConduitName=i18n("Perl");

	(void) perl_conduit_id;
}

PerlConduit::~PerlConduit()
{
	FUNCTIONSETUP;
}

/* virtual */ bool PerlConduit::exec()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": In exec() @" << (unsigned long) this << endl;
#endif

	fThread = new PerlThread(this) ;
	fThread->start();
	return true;
}

/* virtual */ bool PerlConduit::event(QEvent *e)
{
	FUNCTIONSETUP;

	if (e->type() == QEvent::User)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Perl thread done." << endl;
#endif
		QString r;
		addSyncLogEntry(i18n("Perl returned %1.").arg(fThread->result()));
		delayDone();
		return true;
	}
	else return ConduitAction::event(e);
}

static const char *perl_args[] = { "", "-e", "0" } ;

void PerlThread::run()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Thread starting." << endl;
#endif

	fPerl = perl_alloc();
	perl_construct(fPerl);
	perl_parse(fPerl, NULL, 3, const_cast<char **>(perl_args), NULL);
	perl_run(fPerl);

	eval_pv(PerlConduitSettings::expression().latin1(),TRUE);

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Thread woken with " << SvIV(get_sv("a",FALSE)) << endl;
#endif

	fResult.setNum(SvIV(get_sv("a",FALSE)));

	perl_destruct(fPerl);
	perl_free(fPerl);

	QApplication::postEvent(fParent,new QEvent(QEvent::User));
}

