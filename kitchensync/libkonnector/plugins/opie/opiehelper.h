

#ifndef opiehelper_h
#define opiehelper_h

#include <qptrlist.h>

#include <ksyncentry.h>
#include <kalendarsyncentry.h>
#include <kaddressbooksyncentry.h>
#include <opiedesktopsyncentry.h>
#include "opiecategories.h"
#include "categoryedit.h"

class OpieHelperClass {
 public:
  OpieHelperClass() {};
  ~OpieHelperClass()  {};
  void toOpieDesktopEntry(  const QString &,
			   QPtrList<KSyncEntry> *list,
			   OpieHelper::CategoryEdit *edit );

  void toCalendar(const QString &timeStamp,
		  const QString &todo,
		  const QString &calendar ,
		  QPtrList<KSyncEntry> *list,
		  OpieHelper::CategoryEdit *edit);

  void toAddressbook( const QString &timeStamp,
		      const QString &fileName,
		      QPtrList<KSyncEntry> *list,
		      OpieHelper::CategoryEdit *edit );
  static OpieHelperClass *self();
 private:
  static OpieHelperClass *s_Self;
};

#endif

