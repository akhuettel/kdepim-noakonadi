/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __KPILOT_MEMO_H
#define __KPILOT_MEMO_H

#include <pi-macros.h>
#include <qstring.h>
#include "pilotAppCategory.h"
#include "pilotRecord.h"

class PilotMemo : public PilotAppCategory
{
public:
  PilotMemo(void) : PilotAppCategory() { fText = NULL; fSize = 0; }
  PilotMemo(PilotRecord* rec);
  PilotMemo(void *buf) : PilotAppCategory() { unpack(buf, 1); }
  PilotMemo(void *buf, int attr, recordid_t id, int category)
    : PilotAppCategory(attr, id, category) { unpack(buf, 1); }
  ~PilotMemo() { if (fText) delete fText; if (fTitle) delete fTitle;}
  
  const char *text(void) const { return fText; }
  void setText(const char* text) { unpack(text, 0); }
  const char* getTitle(void) const { return fTitle; }
  PilotRecord* pack() { return PilotAppCategory::pack(); }
  
	typedef enum { MAX_MEMO_LEN=8192 } Constants ;

	/**
	* Return a "short but sensible" title. getTitle() returns the
	* first line of the memo, which may be very long
	* and inconvenient. sensibleTitle() returns about 30
	* characters.
	*/
	QString shortTitle() const;

	/**
	* Returns a (complete) title if there is one and [unknown]
	* otherwise.
	*/
	QString sensibleTitle() const;

protected:
  //     void *pack(int *i);
  void *pack(void *, int *);
  void unpack(const void *, int = 0);
  
private:
  char *fText;
  int fSize;
  char* fTitle;
  void *internalPack(unsigned char *);
};



#endif
