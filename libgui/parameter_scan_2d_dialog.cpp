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
#include "scaled_matrix_view.h"
#include "parameter_scan_2d_dialog.h"
#include "pathway_matrix.h"
#include "version.h"

ParameterScan2DDialog::ParameterScan2DDialog(QWidget *parent,
					     const PathwayMatrix &pem,
					     const Fitness &fitness)
  : QDialog(parent, NULL, false, Qt::WStyle_MinMax),
    pem_(pem),
    fitness_(fitness)
{
  QVBoxLayout *vbox = new QVBoxLayout(this,8);
  vbox->setAutoAdd(true);

  QHBox *hbox;
  QLabel *label;

  label = new QLabel("2D Parameter Scan", this);

  mv_ = new ScaledMatrixView(this);

  for(int i=0; i<2; i++) {
    QGroupBox *group = new QGroupBox(2, Vertical, i == 0 ? "Rows" : "Columns", this);

    hbox = new QHBox(group);
    label = new QLabel("Parameter: ", hbox);
    label->setFixedSize(label->sizeHint());
    param_[i] = new QComboBox(false, hbox);
    param_[i]->insertStringList(pem_.moleculeNames());
    param_[i]->insertStringList(pem_.rxn_equations);
    
    hbox = new QHBox(group);
    label = new QLabel("Start: ", hbox);
    label->setFixedSize(label->sizeHint());
    start_[i] = new QLineEdit("0.0", hbox);
    
    label = new QLabel("End: ", hbox);
    label->setFixedSize(label->sizeHint());
    end_[i] = new QLineEdit("100.0", hbox);
    
		label = new QLabel("#Samples: ", hbox);
    label->setFixedSize(label->sizeHint());
    step_[i] = new QLineEdit("10", hbox);
  }

  hbox = new QHBox(this);
  label = new QLabel("Scale: ", hbox);
  label->setFixedSize(label->sizeHint());
  scale_ = new QLineEdit("10", hbox);
  
  label = new QLabel("Update Rate: ", hbox);
  label->setFixedSize(label->sizeHint());
  update_rate_ = new QLineEdit("10", hbox);

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
ParameterScan2DDialog::go() {
  if(start_stop_->text() == "Start") {
    for(int i=0; i<2; i++) {
      QString label = param_[i]->currentText();
      kc_mod_[i] = true;
      ind_[i] = pem_.rxn_equations.findIndex(label);
      if(ind_[i] == -1) {
	kc_mod_[i] = false;
	ind_[i] = pem_.moleculeNames().findIndex(label);
      }
      if(ind_[i] == -1) {
	QMessageBox::critical(this, appname,
			      QString("Unable to locate molecule or reaction named: ") + label);
	return;
      }
      vstep_[i]  = step_[i]->text().toInt();
      vstart_[i] = start_[i]->text().toDouble();
      vend_[i]   = end_[i]->text().toDouble();
      pos_[i]    = 0;
    }

		if(vstep_[0] <= 0 || vstep_[1] <= 0) {
	QMessageBox::critical(this, appname,
			      QString("Number of steps must be a positive integer"));
	return;
		}

    rate_ = update_rate_->text().toInt();
    
    //printf("ind: %d\n", ind_);
    
    progress_->setTotalSteps(vstep_[0]*vstep_[1]);
    progress_->setProgress(0);
    m_.resize(vstep_[0], vstep_[1]);
    m_.fill(0);
    
    mv_->setMatrix(m_);
    mv_->setScale(scale_->text().toInt());
    mv_->showMarkers(false);
    
    start_stop_->setText("Stop");
    timer_->start(100);
  } else {
    start_stop_->setText("Start");
    timer_->stop();
    progress_->reset();
  }
}

void
ParameterScan2DDialog::step() {
  double tmp[2], tmp_v[2][2];
  int i;

  for(i=0; i<2; i++) {
    double x = pos_[i]/(double)vstep_[i] * (vend_[i] - vstart_[i]) + vstart_[i];
    if(kc_mod_[i]) {
      tmp[i] = pem_.kc(ind_[i], 0);
      pem_.kc(ind_[i], 0) = x;
    } else {
      if(ind_[i] < (int)pem_.cvs.count()) {
	((SquareWaveControlVariableItem*)pem_.cvs[ind_[i]])->values(tmp_v[i]);
	((SquareWaveControlVariableItem*)pem_.cvs[ind_[i]])->setValues(x, tmp_v[i][1]);
      } else {
	tmp[i] = pem_.y0(ind_[i], 0);
	pem_.y0(ind_[i], 0) = x;
      }
    }
  }
  
  double fitness = -1.0;
  if(!fitness_.calculate(pem_, fitness)) {
  }
  m_(pos_[0], pos_[1]) = fitness;

  pos_[1]++;
  if(pos_[1] == vstep_[1]) {
    pos_[0]++;
    pos_[1] = 0;
  }
  
  for(i=0; i<2; i++) {
    //double x = pos_[i]/(double)vstep_[i] * (vend_[i] - vstart_[i]) + vstart_[i];
    if(kc_mod_[i]) {
      pem_.kc(ind_[i], 0) = tmp[i];
    } else {
      if(ind_[i] < (int)pem_.cvs.count()) {
	((SquareWaveControlVariableItem*)pem_.cvs[ind_[i]])->setValues(tmp_v[i][0], tmp_v[i][1]);
      } else {
	pem_.y0(ind_[i], 0) = tmp[i];
      }
    }
  }

  i = progress_->progress() + 1;

  if((i % rate_) == 0 || i == progress_->totalSteps()) {
    mv_->setMatrix(m_);
  }

  progress_->setProgress(i);
  if(i == progress_->totalSteps()) go();
}
