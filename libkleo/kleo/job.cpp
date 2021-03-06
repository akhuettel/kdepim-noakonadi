/*
    job.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004,2005 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/


#include "job.h"

#include "keylistjob.h"
#include "encryptjob.h"
#include "decryptjob.h"
#include "decryptverifyjob.h"
#include "signjob.h"
#include "signkeyjob.h"
#include "signencryptjob.h"
#include "verifydetachedjob.h"
#include "verifyopaquejob.h"
#include "keygenerationjob.h"
#include "importjob.h"
#include "importfromkeyserverjob.h"
#include "exportjob.h"
#include "changeexpiryjob.h"
#include "changeownertrustjob.h"
#include "downloadjob.h"
#include "deletejob.h"
#include "refreshkeysjob.h"
#include "adduseridjob.h"
#include "specialjob.h"

#include <gpgme++/error.h>

#include <QCoreApplication>
#include <kdebug.h>

#include <gpg-error.h>

Kleo::Job::Job( QObject * parent )
  : QObject( parent )
{
    if ( QCoreApplication * app = QCoreApplication::instance() )
	connect( app, SIGNAL(aboutToQuit()), SLOT(slotCancel()) );
}

Kleo::Job::~Job() {

}

void Kleo::Job::showErrorDialog( QWidget *, const QString & ) const {
  kDebug(5150) <<"Kleo::Job::showErrorDialog() should be reimplemented in Kleo::Job subclasses!";
}

QString Kleo::Job::auditLogAsHtml() const {
    kDebug(5150) << "Kleo::Job::auditLogAsHtml() should be reimplemented in Kleo::Job subclasses!" << endl;
    return QString();
}

GpgME::Error Kleo::Job::auditLogError() const {
    kDebug(5150) << "Kleo::Job::auditLogError() should be reimplemented in Kleo::Job subclasses!" << endl;
    return GpgME::Error( gpg_error( GPG_ERR_NOT_IMPLEMENTED ) );
}

bool Kleo::Job::isAuditLogSupported() const {
    return auditLogError().code() != GPG_ERR_NOT_IMPLEMENTED ;
}

#define make_job_subclass_ext(x,y)                \
  Kleo::x::x( QObject * parent ) : y( parent ) {} \
  Kleo::x::~x() {}

#define make_job_subclass(x) make_job_subclass_ext(x,Job)

make_job_subclass(KeyListJob)
make_job_subclass(EncryptJob)
make_job_subclass(DecryptJob)
make_job_subclass(DecryptVerifyJob)
make_job_subclass(SignJob)
make_job_subclass(SignEncryptJob)
make_job_subclass(SignKeyJob)
make_job_subclass(VerifyDetachedJob)
make_job_subclass(VerifyOpaqueJob)
make_job_subclass(KeyGenerationJob)
make_job_subclass(AbstractImportJob)
make_job_subclass_ext(ImportJob,AbstractImportJob)
make_job_subclass_ext(ImportFromKeyserverJob,AbstractImportJob)
make_job_subclass(ExportJob)
make_job_subclass(ChangeExpiryJob)
make_job_subclass(ChangeOwnerTrustJob)
make_job_subclass(DownloadJob)
make_job_subclass(DeleteJob)
make_job_subclass(RefreshKeysJob)
make_job_subclass(AddUserIDJob)
make_job_subclass(SpecialJob)

#undef make_job_subclass

#include "job.moc"

#include "keylistjob.moc"
#include "encryptjob.moc"
#include "decryptjob.moc"
#include "decryptverifyjob.moc"
#include "signjob.moc"
#include "signencryptjob.moc"
#include "signkeyjob.moc"
#include "verifydetachedjob.moc"
#include "verifyopaquejob.moc"
#include "keygenerationjob.moc"
#include "abstractimportjob.moc"
#include "importjob.moc"
#include "importfromkeyserverjob.moc"
#include "exportjob.moc"
#include "changeexpiryjob.moc"
#include "changeownertrustjob.moc"
#include "downloadjob.moc"
#include "deletejob.moc"
#include "refreshkeysjob.moc"
#include "adduseridjob.moc"
#include "specialjob.moc"
