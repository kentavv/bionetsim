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

#ifndef _PARAMETER_SCAN_DIALOG_H_
#define _PARAMETER_SCAN_DIALOG_H_

#include <qdialog.h>
#include <qstringlist.h>
#include "fitness.h"
#include "matrix.h"
#include "pathway_matrix.h"

class QComboBox;
class QLineEdit;
class QProgressBar;
class QTimer;

class LineGraphView;

class ParameterScanDialog : public QDialog {
  Q_OBJECT
public:
  ParameterScanDialog(QWidget *parent,
		      const PathwayMatrix &pem,
		      const Fitness &fitness);

protected slots:
  void go();
  void step();
 
protected:
  PathwayMatrix pem_;
  Fitness fitness_;
  
  LineGraphView *lg_;
  QComboBox *param_;
  QLineEdit *start_;
  QLineEdit *end_;
  QLineEdit *step_;
  QTimer *timer_;
  QProgressBar *progress_;
  QPushButton *start_stop_;

  Matrix m_;
  QStringList labels_;
  bool kc_mod_;
  int ind_;
};

#endif
