/*
    knfilterdialog.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qbitarray.h>
#include <qpixmap.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "knglobals.h"
#include "knfiltermanager.h"
#include "knstatusfilter.h"
#include "knrangefilter.h"
#include "knstringfilter.h"
#include "knfilterconfigwidget.h"
#include "knarticlefilter.h"
#include "utilities.h"
#include "knfilterdialog.h"


KNFilterDialog::KNFilterDialog(KNArticleFilter *f, QWidget *parent, const char *name)
  : KDialogBase(Plain, (f->id()==-1)? i18n("New Filter"):i18n("Properties of %1").arg(f->name()),
                Ok|Cancel|Help, Ok, parent, name),
    fltr(f)
{
  QFrame* page=plainPage();

  QGroupBox *gb=new QGroupBox(page);
  QLabel *l1=new QLabel(i18n("Name"), gb);
  QLabel *l2=new QLabel(i18n("apply on"), gb);
  fname=new QLineEdit(gb);
  enabled=new QCheckBox(i18n("show in menu"), gb);
  apon=new QComboBox(gb);
  apon->insertItem(i18n("single articles"));
  apon->insertItem(i18n("whole threads"));  
      
  fw=new KNFilterConfigWidget(page);

  QGridLayout *gbL=new QGridLayout(gb, 2,4,8,5);
  gbL->addWidget(l1, 0,0);
  gbL->addMultiCellWidget(fname, 0,0,1,3);
  gbL->addWidget(enabled, 1,0);
  gbL->addWidget(l2, 1,2);
  gbL->addWidget(apon, 1,3);
  gbL->setColStretch(1,1);
  
  QVBoxLayout *topL=new QVBoxLayout(page,0,5);

  topL->addWidget(gb);
  topL->addWidget(fw,1);

  enabled->setChecked(f->isEnabled());
  apon->setCurrentItem((int) f->applyOn());
  fname->setText(f->translatedName());

  fw->status->setFilter(f->status);
  fw->lines->setFilter(f->lines);
  fw->age->setFilter(f->age);
  fw->score->setFilter(f->score);
  fw->subject->setFilter(f->subject);
  fw->from->setFilter(f->from);
  
  setFixedHeight(sizeHint().height());
  restoreWindowSize("filterDLG", this, sizeHint());

  setHelp("anc-using-filters");
}



KNFilterDialog::~KNFilterDialog()
{
  saveWindowSize("filterDLG", size());
}



void KNFilterDialog::slotOk()
{
  if (fname->text().isEmpty())
    KMessageBox::sorry(this, i18n("Please provide a name for this filter."));
  else
    if (!knGlobals.filManager->newNameIsOK(fltr,fname->text()))
      KMessageBox::sorry(this, i18n("A filter with this name exists already.\nPlease choose a different name."));
    else {
      fltr->setTranslatedName(fname->text());
      fltr->setEnabled(enabled->isChecked());
      fltr->status=fw->status->filter();
      fltr->score=fw->score->filter();
      fltr->age=fw->age->filter();
      fltr->lines=fw->lines->filter();
      fltr->subject=fw->subject->filter();
      fltr->from=fw->from->filter();
      fltr->setApplyOn(apon->currentItem());  
    
      accept();
    }
}



//--------------------------------

#include "knfilterdialog.moc"
