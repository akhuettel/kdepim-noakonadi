// knotes-conduit.cc
//
// Copyright (C) 2000 by Dan Pilone, Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
//
//
// The KNotes conduit copies memos from the Pilot's memo pad to KNotes
// and vice-versa. It complements or replaces the builtin memo conduit
// in KPilot.
//
//



#include "options.h"

// Only include what we really need:
// First UNIX system stuff, then std C++, 
// then Qt, then KDE, then local includes.
//
//
#include <unistd.h>
#include <assert.h>
#include <stream.h>
#include <qdir.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <kdebug.h>
#include <krun.h>

// kpilot includes
#include "abbrowser-conduit.h"
#include "conduitApp.h"
//#include "kpilotlink.h"
#include "setupDialog.h"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *id=
	"$Id$";


// This is a generic main() function, all
// conduits look basically the same,
// except for the name of the conduit.
//
//
int main(int argc, char* argv[])
    {
    ConduitApp a(argc,argv,"abbrowser",
		 I18N_NOOP("Abbrowser Conduit"),
		 "0.1");
    
    a.addAuthor("Gregory Stern",
		"Abbrowser Conduit author",
		"stern@enews.nrl.navy.mil");
    
    AbbrowserConduit conduit(a.getMode(), a.getDBSource());
    a.setConduit(&conduit);
    cout << "AbbrowserConduit about to call exec" << endl;
    return a.exec(true /* with DCOP support */, false);
    }

AbbrowserConduit::AbbrowserConduit(BaseConduit::eConduitMode mode,
				   BaseConduit::DatabaseSource source)
      : BaseConduit(mode, source),
	fDcop(NULL),
	fAddressAppInfo()
    {
    if (source == Local)
	qDebug("AbbrowserConduit::AbbrowserConduit use local");
    else if (source == ConduitSocket)
	qDebug("AbbrowserConduit::AbbrowserConduit conduit socket");
    else
	qDebug("AbbrowserConduit::AbbrowserConduit undefined source");
      
    }

AbbrowserConduit::~AbbrowserConduit()
    {
    }

// aboutAndSetup is pretty much the same
// on all conduits as well.
//
//
QWidget* AbbrowserConduit::aboutAndSetup()
    {
    FUNCTIONSETUP;
    
    //return new KNotesOptions(0L);
    return NULL;
    }

const char * AbbrowserConduit::dbInfo()
    {
    return "AddressDB";
    }

void AbbrowserConduit::_startAbbrowser()
    {
    FUNCTIONSETUP;

    QByteArray sendData;
    QByteArray replyData;
    QCString replyTypeStr;
    if (!fDcop->call("abbrowser", "AbBrowserIface", "interfaces()",
		       sendData, replyTypeStr, replyData))
	{
	// abbrowser not running, start it
	KURL::List noargs;
	KRun::run("abbrowser", noargs);

	qDebug("Waiting to run abbrowser");
	sleep(5000);
	}

    if (!fDcop->call("abbrowser", "AbBrowserIface", "interfaces()",
		       sendData, replyTypeStr, replyData))
	{
	qDebug("AbbrowserConduit:: unable to connect to abbrowser through dcop; autostart failed");
	KApplication::kApplication()->exit(1);
	}
    }

void AbbrowserConduit::
_mapContactsToPilot(const QDict<ContactEntry> &contacts,
		    QMap<recordid_t, ContactEntry *> &idContactMap,
		    QList<ContactEntry> &newContacts) const
    {
    idContactMap.clear();
    newContacts.clear();
    for (QDictIterator<ContactEntry> contactIter(contacts);
	 contactIter.current();++contactIter)
	{
	ContactEntry *aContact = contactIter.current();
	if (aContact->isNew())
	    newContacts.append(aContact);
	else
	    {
	    QString idStr = aContact->getCustomField("KPILOT_ID");
	    if (idStr != QString::null)
		{
		recordid_t id = idStr.toULong();
		idContactMap.insert(id, aContact);
		}
	    else
		{
		qDebug("AbbrowserConduit::_mapContactsToPilot contact is new but KPILOT_ID is not found in abbrowser contact; BUG!");
		newContacts.append(aContact);
		}
	    }
	}
    }

bool AbbrowserConduit::_getAbbrowserContacts(QDict<ContactEntry> &contacts)
    {
    QDict<ContactEntry> entryDict;

    QByteArray noParamData;
    QByteArray replyDictData;
    QCString replyTypeStr;
    if (!fDcop->call("abbrowser", "AbBrowserIface", "getEntryDict()",
		       noParamData, replyTypeStr, replyDictData))
	{
	kdWarning() << "AbBrowserIface::Unable to call abbrowser getEntryDict()" << endl;
	return false;
	}
    assert(replyTypeStr == "QDict<ContactEntry>");

    QDataStream dictStream(replyDictData, IO_ReadOnly);
    dictStream >> contacts;
    return true;
    }

void AbbrowserConduit::showContactEntry(const ContactEntry &abAddress)
     {
     qDebug("\tAbbrowser Contact Entry");
     qDebug("\t\tLast name = %s", abAddress.getLastName().latin1());
     qDebug("\t\tFirst name = %s", abAddress.getFirstName().latin1());
     qDebug("\t\tCompany = %s", abAddress.getCompany().latin1());
     /*
     qDebug("\tJob Title = %s", abAddress.getJobTitle().latin1());
     qDebug("\tNote = %s", abAddress.getNote().latin1());
     qDebug("\tHome phone = %s", abAddress.getHomePhone().latin1());
     qDebug("\tWork phone = %s", abAddress.getBusinessPhone().latin1());
     qDebug("\tMobile phone = %s", abAddress.getMobilePhone().latin1());
     qDebug("\tEmail = %s", abAddress.getEmail().latin1());
     qDebug("\tFax = %s", abAddress.getBusinessFax().latin1());
     qDebug("\tPager = %s", abAddress.getPager().latin1());
     */
     }


void AbbrowserConduit::showPilotAddress(const PilotAddress &pilotAddress) 
     {
     qDebug("\tPilot Address");
     qDebug("\t\tLast name = %s", pilotAddress.getField(entryLastname));
     qDebug("\t\tFirst name = %s", pilotAddress.getField(entryFirstname));
     qDebug("\t\tCompany = %s", pilotAddress.getField(entryCompany));
     /*qDebug("\tJob Title = %s", pilotAddress.getField(entryTitle));
     qDebug("\tNote = %s", pilotAddress.getField(entryNote));
     qDebug("\tHome phone = %s",
	    pilotAddress.getPhoneField(PilotAddress::eHome));
     qDebug("\tWork phone = %s",
	    pilotAddress.getPhoneField(PilotAddress::eHome));
     qDebug("\tMobile phone = %s",
	    pilotAddress.getPhoneField(PilotAddress::eWork));
     qDebug("\tEmail = %s",
	    pilotAddress.getPhoneField(PilotAddress::eEmail));
     qDebug("\tFax = %s",
	    pilotAddress.getPhoneField(PilotAddress::eFax));
     qDebug("\tPager = %s",
	    pilotAddress.getPhoneField(PilotAddress::ePager));
     */
     }

void AbbrowserConduit::_copy(PilotAddress &toPilotAddr,
			     const ContactEntry &fromAbEntry)
    {
    // don't do a reset since this could wipe out non copied info 
    //toPilotAddr.reset();
    toPilotAddr.setField(entryLastname, fromAbEntry.getLastName().latin1());
    toPilotAddr.setField(entryFirstname, fromAbEntry.getFirstName().latin1());
    toPilotAddr.setField(entryCompany, fromAbEntry.getCompany().latin1());
    toPilotAddr.setField(entryTitle, fromAbEntry.getJobTitle().latin1());
    toPilotAddr.setField(entryNote, fromAbEntry.getNote().latin1());
    
    // do email first, to ensure its gets stored
    toPilotAddr.setField(PilotAddress::eEmail,
			 fromAbEntry.getEmail().latin1());
    toPilotAddr.setField(PilotAddress::eWork,
			 fromAbEntry.getBusinessPhone().latin1());
    toPilotAddr.setField(PilotAddress::eHome,
			 fromAbEntry.getHomePhone().latin1());
    toPilotAddr.setField(PilotAddress::eMobile,
			 fromAbEntry.getMobilePhone().latin1());
    toPilotAddr.setField(PilotAddress::eFax,
			 fromAbEntry.getBusinessFax().latin1());
    toPilotAddr.setField(PilotAddress::ePager,
			 fromAbEntry.getPager().latin1());
    
    // in future, may want prefs that will map from abbrowser entries
    // to the pilot phone entries so they can do the above assignment and
    // assign the Other entry which is currenty unused
    ContactEntry::Address *homeAddress = fromAbEntry.getHomeAddress();
    if (!homeAddress->isEmpty())
	_setPilotAddress(toPilotAddr, *homeAddress);
    else
	{
	// no home address, try work address
	ContactEntry::Address *workAddress = fromAbEntry.getBusinessAddress();
	if (!workAddress->isEmpty())
	    _setPilotAddress(toPilotAddr, *workAddress);
	delete workAddress;
	workAddress = NULL;
	}
    delete homeAddress;
    homeAddress = NULL;
    }

void AbbrowserConduit::_setPilotAddress(PilotAddress &toPilotAddr,
					const ContactEntry::Address &abAddress)
    {
    toPilotAddr.setField(entryAddress, abAddress.getStreet().latin1());
    toPilotAddr.setField(entryCity, abAddress.getCity().latin1());
    toPilotAddr.setField(entryState, abAddress.getState().latin1());
    toPilotAddr.setField(entryZip, abAddress.getZip().latin1());
    toPilotAddr.setField(entryCountry, abAddress.getCountry().latin1());
    }

void AbbrowserConduit::_copy(ContactEntry &toAbEntry,
			     const PilotAddress &fromPiAddr)
    {
    // copy straight forward values
    toAbEntry.setLastName( fromPiAddr.getField(entryLastname) );
    toAbEntry.setFirstName( fromPiAddr.getField(entryFirstname) );
    toAbEntry.setCompany( fromPiAddr.getField(entryCompany) );
    toAbEntry.setJobTitle( fromPiAddr.getField(entryTitle) );
    toAbEntry.setNote( fromPiAddr.getField(entryNote) );

    // copy the phone stuff
    toAbEntry.setEmail( fromPiAddr.getPhoneField(PilotAddress::eEmail));
    toAbEntry.setHomePhone( fromPiAddr.getPhoneField(PilotAddress::eHome));
    toAbEntry.setBusinessPhone( fromPiAddr.getPhoneField(PilotAddress::eWork));
    toAbEntry.setMobilePhone( fromPiAddr.getPhoneField(PilotAddress::eMobile));
    toAbEntry.setBusinessFax( fromPiAddr.getPhoneField(PilotAddress::eFax));
    toAbEntry.setPager( fromPiAddr.getPhoneField(PilotAddress::ePager));

    //in future, probably the address assigning to work or home should
    // be a prefs option
    // for now, just assign to home since that's what I'm using it for
    ContactEntry::Address *homeAddress = toAbEntry.getHomeAddress();
    homeAddress->setStreet(fromPiAddr.getField(entryAddress));
    homeAddress->setCity(fromPiAddr.getField(entryCity));
    homeAddress->setState(fromPiAddr.getField(entryState));
    homeAddress->setZip(fromPiAddr.getField(entryZip));
    homeAddress->setCountry(fromPiAddr.getField(entryCountry));
    delete homeAddress;
    homeAddress = NULL;
    
    // copy the fromPiAddr pilot id to the custom field KPilot_Id;
    // pilot id may be zero (since it could be new) but couldn't hurt
    // to even assign it to zero; let's us know what state the
    // toAbEntry is in
    toAbEntry.setCustomField("KPILOT_ID", QString::number(fromPiAddr.getID()));
    }


void AbbrowserConduit::_addToAbbrowser(const PilotAddress &address)
    {
    ContactEntry entry;
    _copy(entry, address);
    _saveAbEntry(entry);
    }

void AbbrowserConduit::_addToPalm(ContactEntry &entry)
    {
    PilotAddress pilotAddress(fAddressAppInfo);
    _copy(pilotAddress, entry);
    _savePilotAddress(pilotAddress, entry);
    }

/** There was a conflict between the two fields; either could be null,
 *  or both have been modified
 */
void AbbrowserConduit::_handleConflict(PilotAddress *pilotAddress,
				       ContactEntry *abEntry)
    {
    if (pilotAddress && abEntry)
	{
	qDebug("AbbrowserConduit::_handleConflict => Both records exist but both were changed");
	showContactEntry(*abEntry);
	showPilotAddress(*pilotAddress);
	}
    else if (pilotAddress)
	{
	qDebug("AbbrowserConduit::_handleConflict =>  ContactEntry was deleted but pilotAddress was modified");
	showPilotAddress(*pilotAddress);
	}
    else
	{
	qDebug("AbbrowserConduit::_handleConflict =>  PilotAddress was deleted but ConactEntry was modified");
	showContactEntry(*abEntry);
	}
    }

void AbbrowserConduit::_removePilotAddress(PilotAddress &address)
    {
    qDebug("AbbrowserConduit::_removePilotAddress");
    showPilotAddress(address);
    }

void AbbrowserConduit::_removeAbEntry(ContactEntry &abEntry)
    {
    qDebug("AbbrowserConduit::_removeAbEntry");
    showContactEntry(abEntry);
    }
void AbbrowserConduit::_saveAbEntry(ContactEntry &abEntry)
    {
    qDebug("AbbrowserConduit::_saveAbEntry");
    // this marks that this field has been synced
    abEntry.setModified(false);

    showContactEntry(abEntry);
    // save over kdcop to abbrowser
    }

void AbbrowserConduit::_savePilotAddress(PilotAddress &address,
					 ContactEntry &abEntry)
    {
    qDebug("AbbrowserConduit::_savePilotAddress");

    // if abEntry isn't storing the pilot record id, then store it
    // and save the ab entry
    }

void AbbrowserConduit::_setAppInfo()
    {
    // get the address application header information
    unsigned char * buffer = new unsigned char [PilotAddress::APP_BUFFER_SIZE];
    int appLen = readAppInfo(buffer);
    unpack_AddressAppInfo(&fAddressAppInfo, buffer, appLen);
    delete []buffer;
    buffer = NULL;
    }

bool AbbrowserConduit::_equal(const PilotAddress &piAddress,
			      const ContactEntry &abEntry) const
    {
    if (piAddress.getField(entryLastname) != abEntry.getLastName())
	return false;
    if (piAddress.getField(entryFirstname) != abEntry.getFirstName())
	return false;
    if (piAddress.getField(entryTitle) != abEntry.getJobTitle())
	return false;
    if (piAddress.getField(entryCompany) != abEntry.getCompany())
	return false;
    if (piAddress.getField(entryNote) != abEntry.getNote())
	return false;
    if (piAddress.getPhoneField(PilotAddress::eWork) !=
	abEntry.getBusinessPhone())
	return false;
    if (piAddress.getPhoneField(PilotAddress::eHome) != abEntry.getHomePhone())
	return false;
    if (piAddress.getPhoneField(PilotAddress::eEmail) != abEntry.getEmail())
	return false;
    if (piAddress.getPhoneField(PilotAddress::eFax) !=abEntry.getBusinessFax())
	return false;
    if (piAddress.getPhoneField(PilotAddress::eMobile)
	!= abEntry.getMobilePhone())
	return false;
    ContactEntry::Address *address = abEntry.getHomeAddress();
    if (piAddress.getField(entryAddress) != address->getStreet())
	{
	delete address;
	address = abEntry.getBusinessAddress();
	if (piAddress.getField(entryAddress) != address->getStreet())
	    {
	    delete address;
	    return false;
	    }
	}
    if (piAddress.getField(entryCity) != address->getCity())
	{
	delete address;
	return false;
	}
    if (piAddress.getField(entryState) != address->getState())
	{
	delete address;
	return false;
	}
    if (piAddress.getField(entryZip) != address->getZip())
	{
	delete address;
	return false;
	}
    if (piAddress.getField(entryCountry) != address->getCountry())
	{
	delete address;
	return false;
	}
    
    delete address;
    return true;
    }

ContactEntry *
AbbrowserConduit::_findMatch(const QDict<ContactEntry> entries,
			     const PilotAddress &pilotAddress) const
    {
    // for now just loop throug all entries; in future, probably better
    // to create a map for first and last name, then just do O(1) calls
    for (QDictIterator<ContactEntry> iter(entries);iter.current();
	 ++iter)
	{
	ContactEntry *abEntry = iter.current();
	if (abEntry->getLastName() != QString::null
	     && abEntry->getFirstName() != QString::null)
	    {
	    if (abEntry->getLastName() == pilotAddress.getField(entryLastname)
		&& abEntry->getFirstName() ==
		pilotAddress.getField(entryFirstname))
		return abEntry;
	    }
	else
	    if (abEntry->getCompany() != QString::null &&
		abEntry->getCompany() == pilotAddress.getField(entryCompany))
		return abEntry;
	}
    return 0L;
    }


void AbbrowserConduit::doBackup()
    {
    qDebug("AbbrowserConduit::doBackup()");
    
    if (!fDcop)
	fDcop = KApplication::kApplication()->dcopClient();
    _startAbbrowser();
    _setAppInfo();
    
    // get the contacts from abbrowser
    QDict<ContactEntry> abbrowserContacts;
    if (!_getAbbrowserContacts(abbrowserContacts))
	{
	qDebug("AbbrowserConduit::doBackup() unable to get contacts");
	return;
	}

    // get the idContactMap and the newContacts
    // - the idContactMap maps Pilot unique record (address) id's to
    //   a Abbrowser ContactEntry; allows for easy lookup and comparisons
    // - created from the list of Abbrowser Contacts
    QMap<recordid_t, ContactEntry *> idContactMap;
    QList<ContactEntry> newContacts;
    _mapContactsToPilot(abbrowserContacts, idContactMap, newContacts);

    // iterate through all records in palm pilot
    int recIndex=0;
    for (PilotRecord *record = readRecordByIndex(recIndex); record != NULL; 
	 ++recIndex, record = readRecordByIndex(recIndex))
	{
	PilotAddress pilotAddress(fAddressAppInfo, record);
	
	// if already stored in the abbrowser
	if (idContactMap.contains( pilotAddress.id() ))
	    {
	    ContactEntry *abEntry = idContactMap[pilotAddress.id()];
	    assert(abEntry != NULL);

	    // if equal, do nothing since it is already there
	    if (!_equal(pilotAddress, *abEntry))
		// if not equal, let the user choose what to do
		_handleConflict( &pilotAddress, abEntry);
	    }
	else
	    {
	    // look for the possible match of names
	    ContactEntry *abEntry =
		_findMatch(abbrowserContacts, pilotAddress);
	    if (abEntry)
		{
		// if already found in abbrowser and kpilot, just assign
		// the kpilot id and save
		if (_equal(pilotAddress, *abEntry))
		    {
		    qDebug("Abbrowser::doBackup both records already exist and are equal, just assigning the KPILOT_ID to the abbrowser entry");
		    abEntry->setCustomField("KPILOT_ID", QString::number(pilotAddress.getID()));
		    _saveAbEntry(*abEntry);
		    }
		else
		    _handleConflict(&pilotAddress, abEntry);
		}
	    else  // if not found in the abbrowser contacts, add it
		{
		ContactEntry newEntry;
		_copy(newEntry, pilotAddress);
		_saveAbEntry(newEntry);
		}
	    }
	}
    }

void AbbrowserConduit::doSync()
    {
    if (!fDcop)
	fDcop = KApplication::kApplication()->dcopClient();
    _startAbbrowser();
    _setAppInfo();
    
    
    // get the contacts from abbrowser
    QDict<ContactEntry> abbrowserContacts;
    if (!_getAbbrowserContacts(abbrowserContacts))
	{
	qDebug("AbbrowserConduit::doSync() unable to get contacts");
	return;
	}

    // get the idContactMap and the newContacts
    // - the idContactMap maps Pilot unique record (address) id's to
    //   a Abbrowser ContactEntry; allows for easy lookup and comparisons
    // - created from the list of Abbrowser Contacts
    QMap<recordid_t, ContactEntry *> idContactMap;
    QList<ContactEntry> newContacts;
    _mapContactsToPilot(abbrowserContacts, idContactMap, newContacts);

    // perform syncing from palm to abbrowser
    
    // iterate through all records in palm pilot
    int recIndex=0;
    for (PilotRecord *record = readRecordByIndex(recIndex); record != NULL; 
	 ++recIndex, record = readRecordByIndex(recIndex))
	{
	PilotAddress pilotAddress(fAddressAppInfo, record);
	//   if record not in abbrowser
	if (!idContactMap.contains( pilotAddress.id() ))
	    {
	    //  ** I would like to use the the below algorithm, but there is
	    //  ** no way currenty to know if a address has been deleted
	    //  ** in abbrowser (unless the trash can functionality is done)
	    //if record has been deleted in abbrowser
	    //        then query user what to do
	    //     else // not been deleted, so must be new
	    //        add record to abbrowser

	    // assume that it was deleted from the abbrowser since a backup
	    // should have been done before the first sync; if the pilot
	    // address was modified, then query the user what to do?

	    if (pilotAddress.isModified())
		_handleConflict(&pilotAddress, NULL);
	    else
		_removePilotAddress(pilotAddress);
	    }
	else 
	    {
	    ContactEntry *abEntry = idContactMap[pilotAddress.id()];
	    assert(abEntry != NULL);
	    // the record exists in the abbrowser and the palm
	    if (pilotAddress.isModified() && abEntry->isModified())
		{
		// query the user what to do...
		_handleConflict(&pilotAddress, abEntry);
		}
	    else  // record is either modified in the abbrowser or the palm
		  // or not modified at all
		if (pilotAddress.isModified())
		    {
		    if (pilotAddress.isDeleted())
			_removeAbEntry(*abEntry);
		    else
			{
			// update abbrowser
			_copy(*abEntry, pilotAddress);
			_saveAbEntry( *abEntry );
			}
		    }
		else if (abEntry->isModified())
		    {
		    // update pilot
		    _copy(pilotAddress, *abEntry);
		    _savePilotAddress( pilotAddress, *abEntry );
		    }
	    // else not modified at either end, leave alone
	    }
	} // end pilot record loop

    // add all new entries from abbrowser to the palm pilot
    for (QListIterator<ContactEntry> newAbIter(newContacts);
	 newAbIter.current();++newAbIter)
	_addToPalm(*newAbIter.current());
    
    // add everything from pilot to abbrowser that is modified (or new)
    // add everything from abbrowser to pilot that is modified (or new) 
    // delete anything in pilot that is not in abbrowser
    //   (this assumes a backup has been done prior to the sync)
    }

void AbbrowserConduit::doTest()
    {
    FUNCTIONSETUP;
    fDcop = KApplication::kApplication()->dcopClient();
    if (!fDcop)
	{
	kdWarning() << fname
		    << ": Can't get DCOP client."
		    << endl;
	return;
	}
    doSync();
    }

