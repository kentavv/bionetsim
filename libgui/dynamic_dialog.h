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

#ifndef _DYNAMIC_DIALOG_H_
#define _DYNAMIC_DIALOG_H_

#include <qdialog.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "matrix.h"
#include "pathway_canvas_items.h"
#include "result.h"

class VCRControls;
class QComboBox;
class QSlider;

class DynamicDialog : public QDialog {
  Q_OBJECT
public:
  DynamicDialog(QWidget *parent);
  ~DynamicDialog();

  void setData(const QValueList<Result> &results_,
	       QList<PathwayCanvasItem> *pitems,
	       QWidget *w);

public slots:
  void hide();
  void show();
 
protected slots:
  void changedTime(int);
  void colorModeChanged(int);
  void normalizeModeChanged(int);
  void done(int);
	void sliderMoved(int);
  
protected:
  Matrix m_;
  QList<PathwayCanvasItem> *pitems_;
  int color_mode_, norm_mode_;
  QWidget *w_;
  VCRControls *vcr_;
	QSlider *slider_;

  void stop();
};

#endif
