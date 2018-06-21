// This file is part of GenoDYN.
//
// GenoDYN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// GenoDYN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with GenoDYN.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2008 Kent A. Vander Velden

#include <qcombobox.h>
#include <qdialog.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qtimer.h>

#include "calculate.h"
#include "line_graph_view.h"
#include "parameter_scan_dialog.h"
#include "pathway_matrix.h"
#include "version.h"
#include "fitness_lang.h"

ParameterScanDialog::ParameterScanDialog(QWidget *parent,
					 const PathwayMatrix &pem,
					 const Fitness &fitness)
  : QDialog(parent, NULL, false, Qt::WStyle_MinMax),
    pem_(pem),
    fitness_(fitness),
    kc_mod_(true),
    ind_(0)
{
  QVBoxLayout *vbox = new QVBoxLayout(this,8);
  vbox->setAutoAdd(true);

  QHBox *hbox;
  QLabel *label;

  label = new QLabel("Parameter Scan", this);

  lg_ = new LineGraphView(this);
  labels_.append("Full data");
  labels_.append("Data ignoring error");

  hbox = new QHBox(this);
  label = new QLabel("Parameter: ", hbox);
  label->setFixedSize(label->sizeHint());
  param_ = new QComboBox(false, hbox);
  param_->insertStringList(pem_.moleculeNames());
  param_->insertStringList(pem_.rxn_equations);

  hbox = new QHBox(this);
  label = new QLabel("Start: ", hbox);
  label->setFixedSize(label->sizeHint());
  start_ = new QLineEdit("0.0", hbox);
  
  label = new QLabel("End: ", hbox);
  label->setFixedSize(label->sizeHint());
  end_ = new QLineEdit("100.0", hbox);
  
	label = new QLabel("#Samples: ", hbox);
  label->setFixedSize(label->sizeHint());
  step_ = new QLineEdit("10", hbox);

  progress_ = new QProgressBar(this);
  
  hbox = new QHBox(this);
  QPushButton *cancel = new QPushButton("Close", hbox);
  start_stop_ = new QPushButton("Start", hbox);
  start_stop_->setDefault(true);

  timer_ = new QTimer(this);

  connect(timer_, SIGNAL(timeout()), SLOT(step()));
  connect(start_stop_, SIGNAL(clicked()), SLOT(go()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}

void
ParameterScanDialog::go() {
  if(start_stop_->text() == "Start") {
    QString label = param_->currentText();

    kc_mod_ = true;
    ind_ = pem_.rxn_equations.findIndex(label);
    if(ind_ == -1) {
      kc_mod_ = false;
      ind_ = pem_.moleculeNames().findIndex(label);
    }
    if(ind_ == -1) {
      QMessageBox::critical(this, appname,
			    QString("Unable to locate reaction ") + label);
      return;
    }
    //printf("ind: %d\n", ind_);
    
    int step = step_->text().toInt();
    double start = start_->text().toDouble();
    double end = end_->text().toDouble();

    progress_->setTotalSteps(step+1);
    progress_->setProgress(0);
    m_.resize(step+1, 3);

    for(int i=0; i<=step; i++) {
      double x = i/(double)step * (end - start) + start;
      m_(i, 0) = x;
      m_(i, 1) = 0.0;
      m_(i, 2) = 0.0;
    }

    start_stop_->setText("Stop");
    timer_->start(100);
  } else {
    start_stop_->setText("Start");
    timer_->stop();
    progress_->reset();
  }
}

void
ParameterScanDialog::step() {
  int i = progress_->progress();
  double tmp, tmp_v[2];
  if(kc_mod_) {
    tmp = pem_.kc(ind_, 0);
    pem_.kc(ind_, 0) = m_(i, 0);
  } else {
    if(ind_ < (int)pem_.cvs.count()) {
      ((SquareWaveControlVariableItem*)pem_.cvs[ind_])->values(tmp_v);
      ((SquareWaveControlVariableItem*)pem_.cvs[ind_])->setValues(m_(i, 0), tmp_v[1]);
    } else {
      tmp = pem_.y0(ind_, 0);
      pem_.y0(ind_, 0) = m_(i, 0);
    }
  }
  double fitness = -1.0;
  Matrix results;
  if(!fitness_.calculate(pem_, fitness)) {
  }
  m_(i, 1) = fitness;
  if(fitness == -1.0) {
    m_(i, 2) = (i == 0) ? 0.0 : m_(i-1, 2);
  } else {
    m_(i, 2) = fitness;
  }

  if(kc_mod_) {
    pem_.kc(ind_, 0) = tmp;
  } else {
    if(ind_ < (int)pem_.cvs.count()) {
      ((SquareWaveControlVariableItem*)pem_.cvs[ind_])->setValues(tmp_v[0], tmp_v[1]);
    } else {
      pem_.y0(ind_, 0) = tmp;
    }
  }

  lg_->setData(m_, labels_);

  i++;
  progress_->setProgress(i);
  if(progress_->progress() == progress_->totalSteps()) go();
}
