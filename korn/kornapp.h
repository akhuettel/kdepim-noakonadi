/*
* kornapp.h -- Declaration of class KornApp.
* Generated by newclass on Sun Apr 22 23:50:49 EST 2001.
*/
#ifndef SSK_KORNAPP_H
#define SSK_KORNAPP_H

#include <kuniqueapplication.h>

class KornShell;

/**
* @short KornApp
* @author Sirtaj Singh Kang (taj@kde.org)
* @version $Id$
*/
class KornApp : public KUniqueApplication
{
	Q_OBJECT

public:
	/**
	* KornApp Constructor
	*/
	KornApp() : KUniqueApplication(), _shell( 0 ), _instanceCount( 0 ) {}

	/**
	* KornApp Destructor
	*/
	virtual ~KornApp(){}

	/**
	 * This function handles a new instance of KOrn.
	 * If KOrn already is started, it displays the option dialog; elsewise it starts KOrn.
	 *
	 * @return always 0
	 */
	virtual int newInstance();
	/**
	 * This function sets the KornShell.
	 *
	 * @param shell the new KornShell
	 */
	void setShell( KornShell *shell ){ _shell = shell; }
	
private:
	KornShell *_shell;
	int _instanceCount;

	KornApp& operator=( const KornApp& );
	KornApp( const KornApp& );
};

#endif // SSK_KORNAPP_H
