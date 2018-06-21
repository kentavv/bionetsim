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

#ifndef _REPORT_DIALOG_H_
#define _REPORT_DIALOG_H_

#include <qdialog.h>
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include "matrix.h"
#include "simulation.h"
#include "pathway_canvas_items.h"

class QTextEdit;
class LineGraphView;

class ReportDialog : public QDialog {
  Q_OBJECT
public:
  ReportDialog(QWidget *parent=NULL, const char *name=NULL, bool modal=false);
  void setup(const Simulation &sim, const QList<PathwayCanvasItem> &items);
  
public slots:
  void save();
 
private:
  QTextEdit *text_;
  LineGraphView *lg_;

  void buildControlVariableGraph(const Simulation &, const QList<PathwayCanvasItem> &items);
};

#endif
