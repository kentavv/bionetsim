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

#ifndef _SCALED_MATRIX_VIEW_
#define _SCALED_MATRIX_VIEW_

#include <qscrollview.h>
#include <qimage.h>
#include <qpixmap.h>

#include "matrix.h"

class ScaledMatrixView : public QScrollView {
  Q_OBJECT
public:
  ScaledMatrixView(QWidget* parent);
  ~ScaledMatrixView();

  bool setMatrix(const Matrix&);
  void setDefault(float, float);
  void setCurrent(float, float);
  void setCutoffs(float, float);
  void setScale(int);

  void showMarkers(bool);
  
signals:
  void valueSelected(int, int);
  void statusMessage(const QString &, int);
  
protected:
  void drawContents(QPainter*, int, int, int, int);
  void contentsMousePressEvent(QMouseEvent*);
  void contentsMouseMoveEvent(QMouseEvent*);

private:
  Matrix matrix_;
  int scale_;
  float upper_co_, lower_co_;
  int w_, h_, step_;
  int def_x_, def_y_;
  int cur_x_, cur_y_;
  bool show_markers_;
  
  QImage image_;
  QPixmap pix_;

  void genImage(bool);
};

#endif
