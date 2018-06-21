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

#ifndef _FITNESS_LANDSCAPE_VIEW_
#define _FITNESS_LANDSCAPE_VIEW_

#include <qscrollview.h>
#include <qimage.h>
#include <qpixmap.h>

class FitnessLandscapeView : public QScrollView {
  Q_OBJECT
public:
  FitnessLandscapeView(QWidget* parent);
  ~FitnessLandscapeView();

  bool loadData(int, int);
  void setDefault(float, float);
  void setCurrent(float, float);
  void setCutoffs(float, float);
  
signals:
  void valueSelected(int, int);
  void statusMessage(const QString &, int);
  
protected:
  void drawContents(QPainter*, int, int, int, int);
  void contentsMousePressEvent(QMouseEvent*);
  void contentsMouseMoveEvent(QMouseEvent*);

private:
  float *matrix_;
  float upper_co_, lower_co_;
  int x_axis_, y_axis_;
  int w_, h_, step_;
  int def_x_, def_y_;
  int cur_x_, cur_y_;
  
  QImage image_;
  QPixmap pix_;

  void genImage(bool);
};

#endif
