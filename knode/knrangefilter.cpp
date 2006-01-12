/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <QHBoxLayout>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <knuminput.h>
#include <ksimpleconfig.h>

#include "knrangefilter.h"

using namespace KNode;

bool KNode::RangeFilter::doFilter( int a )
{
  bool ret=true;
  if(enabled) {
    switch (op1) {
      case gt:
      case gtoeq:
          if (op2 != dis)
            ret=( matchesOp(val1,op1,a) && matchesOp(a,op2,val2) );
          else
            ret = matchesOp(val1,op1,a);
          break;
      case eq:
      case lt:
      case ltoeq:
          ret = matchesOp(val1,op1,a);
          break;
      default:
          ret = false;
    }
  }

  return ret;
}



bool KNode::RangeFilter::matchesOp( int v1, Op o, int v2 )
{
  bool ret=false;

  switch(o) {
    case eq:      ret=(v1==v2);   break;
    case gt:      ret=(v1<v2);    break;
    case gtoeq:   ret=(v1<=v2);   break;
    case ltoeq:   ret=(v1>=v2);   break;
    case lt:      ret=(v1>v2);    break;
    default:      ret=false;      break;
  };

  return ret;
}



void KNode::RangeFilter::load( KSimpleConfig *conf )
{
  enabled=conf->readEntry("enabled", false);
  val1=conf->readEntry("val1",0);
  op1=(Op) conf->readEntry("op1",0);
  val2=conf->readEntry("val2",0);
  op2=(Op) conf->readEntry("op2",0);
}



void KNode::RangeFilter::save( KSimpleConfig *conf )
{
  conf->writeEntry("enabled", enabled);
  conf->writeEntry("val1", val1);
  conf->writeEntry("op1", (int)op1);
  conf->writeEntry("op2", (int)op2);
  conf->writeEntry("val2", val2);
}




//=====================================================================================

KNode::RangeFilterWidget::RangeFilterWidget( const QString& value, int min, int max,
                                             QWidget* parent, const QString &unit )
  : QGroupBox( value, parent )
{
  QHBoxLayout *layout = new QHBoxLayout( this );

  enabled = new QCheckBox( this );
  layout->addWidget( enabled );

  val1 = new KIntSpinBox( min, max, 1, min, this );
  val1->setSuffix( unit );
  layout->addWidget( val1 );

  op1 = new QComboBox( this );
  op1->insertItem("<");
  op1->insertItem("<=");
  op1->insertItem("=");
  op1->insertItem(">=");
  op1->insertItem(">");
  layout->addWidget( op1 );

  des = new QLabel( value, this );
  des->setAlignment( Qt::AlignCenter );
  layout->addWidget( des );

  op2 = new QComboBox( this );
  op2->insertItem("");
  op2->insertItem("<");
  op2->insertItem("<=");
  layout->addWidget( op2 );

  val2 = new KIntSpinBox( min, max, 1, min, this );
  val2->setSuffix( unit );
  layout->addWidget( val2 );

  connect(op1, SIGNAL(activated(int)), SLOT(slotOp1Changed(int)));
  connect(op2, SIGNAL(activated(int)), SLOT(slotOp2Changed(int)));
  connect(enabled, SIGNAL(toggled(bool)), SLOT(slotEnabled(bool)));

  slotEnabled(false);
}



KNode::RangeFilterWidget::~RangeFilterWidget()
{
}



RangeFilter KNode::RangeFilterWidget::filter()
{
  RangeFilter r;
  r.val1=val1->value();
  r.val2=val2->value();

  r.op1=(RangeFilter::Op) op1->currentItem();
  if (op2->currentText().isEmpty())
    r.op2 = RangeFilter::dis;
  else if (op2->currentText()=="<")
    r.op2 = RangeFilter::gt;
  else if (op2->currentText()=="<=")
    r.op2 = RangeFilter::gtoeq;

  r.enabled=enabled->isChecked();

  return r;
}



void KNode::RangeFilterWidget::setFilter( RangeFilter &f )
{
  val1->setValue(f.val1);
  val2->setValue(f.val2);

  op1->setCurrentItem((int)f.op1);
  if ( f.op2 == RangeFilter::dis )
    op2->setCurrentItem(0);
  else if ( f.op2 == RangeFilter::gt )
    op2->setCurrentItem(1);
  else if ( f.op2 == RangeFilter::gtoeq )
    op2->setCurrentItem(2);

  enabled->setChecked(f.enabled);
}



void KNode::RangeFilterWidget::clear()
{
  val1->setValue(val1->minValue());
  val2->setValue(val2->minValue());
  enabled->setChecked(false);
}



void KNode::RangeFilterWidget::slotOp1Changed( int id )
{
  bool state = op1->isEnabled() && id < 2;
  op2->setEnabled(state);
  des->setEnabled(state);
  slotOp2Changed(op2->currentItem());
}



void KNode::RangeFilterWidget::slotOp2Changed( int id )
{
  val2->setEnabled(op1->isEnabled() && (op1->currentItem()<2) && (id>0));
}



void KNode::RangeFilterWidget::slotEnabled( bool e )
{
  op1->setEnabled(e);
  val1->setEnabled(e);
  des->setEnabled(e);
  slotOp1Changed(op1->currentItem());
}

// -----------------------------------------------------------------------------

#include "knrangefilter.moc"
