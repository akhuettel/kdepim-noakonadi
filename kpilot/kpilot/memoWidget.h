/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __MEMO_WIDGET_H
#define __MEMO_WIDGET_H

#include "pilotComponent.h"
#include <qmlined.h>
#include <qcombo.h>
#include <time.h>
#include "pi-memo.h"
#include "pilotMemo.h"
#include "kpilotlink.h"

class KPilotInstaller;
class QListBox;

class MemoWidget : public PilotComponent
{
  Q_OBJECT
  
public:
  MemoWidget(KPilotInstaller* installer, QWidget* parent);
  ~MemoWidget();
  
    // Pilot Component Methods:
  void initialize();
  void preHotSync(char*);
  void postHotSync();
  bool saveData();
  
  static int MAX_MEMO_LEN;

	// int findSelectedCategory(bool AllIsUnfiled=false);

protected:
	void initializeCategories(PilotDatabase *);
	void initializeMemos(PilotDatabase *);
  
 public slots:
	/**
	* Called whenever the selected memo changes to indicate
	* which buttons are active, mostly to prevent the delete
	* button from being active when it can't do anything.
	*/
	void slotUpdateButtons();
 void slotShowMemo(int);
  void slotTextChanged();
  void slotImportMemo();
  void slotExportMemo();
  void slotDeleteMemo(); // Delets the currently selected memo
  void slotSetCategory(int);

private:
  void setupWidget();
  void updateWidget(); // Called with the lists have changed..
  void writeMemo(PilotMemo* which);
  QComboBox* fCatList;
  
  QMultiLineEdit*    fTextWidget;
  struct MemoAppInfo fMemoAppInfo;
  QList<PilotMemo>   fMemoList;
  unsigned int       fLookupTable[1000]; // Used to index the listbox into the memolist
  QListBox *          fListBox;

	QPushButton *fExportButton,*fDeleteButton;
};

#endif
