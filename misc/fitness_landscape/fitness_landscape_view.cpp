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

#include <stdio.h>

#include <qpainter.h>
#include <qscrollview.h>

#include "fitness_landscape_view.h"

//----------------------------------------------------------------------

static void
HSVtoRGB(float h, float s, float v, float rgb[3]) {
  if(s == 0.0) {
    rgb[0] = v;
    rgb[1] = v;
    rgb[2] = v;
  } else {
    int i;
    float f, p, q, t;
    if(h == 360) h = 0;
    h /= 60;
    i = (int)h;
    f = h - i;
    p = v * (1 - s);
    q = v * (1 - (s * f));
    t = v * (1 - (s * (1 - f)));
    switch(i) {
    case 0: rgb[0] = v; rgb[1] = t; rgb[2] = p; break;
    case 1: rgb[0] = q; rgb[1] = v; rgb[2] = p; break;
    case 2: rgb[0] = p; rgb[1] = v; rgb[2] = t; break;
    case 3: rgb[0] = p; rgb[1] = q; rgb[2] = v; break;
    case 4: rgb[0] = t; rgb[1] = p; rgb[2] = v; break;
    case 5: rgb[0] = v; rgb[1] = p; rgb[2] = q; break;
 //   default: abort();
    }
  }
}

FitnessLandscapeView::FitnessLandscapeView(QWidget* parent)
  : QScrollView(parent),
    matrix_(NULL),
    upper_co_(0.0),
    lower_co_(1.0),
    x_axis_(-1),
    y_axis_(-1),
    w_(0),
    h_(0),
    step_(0)
{
  //viewport()->setBackgroundMode(NoBackground);
  viewport()->setMouseTracking(true);
  setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

  setHScrollBarMode(AlwaysOn);
  setVScrollBarMode(AlwaysOn);
}

FitnessLandscapeView::~FitnessLandscapeView() {
  if(matrix_) delete[] matrix_;
}

bool
FitnessLandscapeView::loadData(int x_axis, int y_axis) {
  x_axis_ = x_axis;
  y_axis_ = y_axis;

  bool reuse = true;
  
  if(x_axis != y_axis) {
    QString filename;
    bool rotate = x_axis > y_axis;
    
    if(rotate) {
      filename.sprintf("%02d_%02d.raw", y_axis, x_axis);
    } else {
      filename.sprintf("%02d_%02d.raw", x_axis, y_axis);
    }
    
    FILE *file=fopen(filename, "rb");
    if(!file) return false;

    int w, h;
    fread(&w, sizeof(w), 1, file);
    fread(&h, sizeof(h), 1, file);
    step_ = 2;

    if(w*h != w_*h_ || !matrix_) {
      if(matrix_) delete[] matrix_;
      matrix_ = new float[w*h];
      reuse = false;
    }
    w_ = w;
    h_ = h;
    
    fread(matrix_, sizeof(float), w_*h_, file);
  
    fclose(file);

    if(!rotate) {
      float *matrix2 = new float[w_*h_];
      for(int y=0; y<h_; y++) {
	for(int x=0; x<w_; x++) {
	  matrix2[x*h_+y] = matrix_[y*w_+x];
	}
      }
      memcpy(matrix_, matrix2, sizeof(float)*w_*h_);
      delete[] matrix2;
      int tmp = w_;
      w_ = h_;
      h_ = tmp;
    }
  } else {
    if(matrix_) {
      memset(matrix_, 0, sizeof(float)*w_*h_);
    }
  }

#if 0
  {
    for(int y=0; y<h_; y++) {
      for(int x=0; x<w_; x++) {
	float z = matrix_[y*w_+x];
	if(z > 256) z = 256;
	z = 1 - (z - 0) / (256 - 0);
	matrix_[y*w_+x] = z;
      }
    }
  }
 
#endif
 
  genImage(reuse);

  viewport()->update();
  
  return true;
}

void
FitnessLandscapeView::genImage(bool reuse) {
  int image_w = w_+50;
  int image_h = h_+20;
  int x, y;
  float z, color[3];


  if(!reuse) image_.create(image_w, image_h, 32);

  if(!reuse || x_axis_ == y_axis_) {
    image_.fill(0);

    for(y=0; y<h_; y++) {
      z = y/(float)h_;
      HSVtoRGB(z*240, 1.0, 1.0, color);
      for(x=0; x<20; x++) {
	uint *p = (uint *)image_.scanLine(y+10) + (x+10);
	*p = qRgb((int)(color[0] * 255 + 0.5),
		  (int)(color[1] * 255 + 0.5),
		  (int)(color[2] * 255 + 0.5));
      }
    }
  }
  
  if(x_axis_ != y_axis_) {
    for(y=0; y<h_; y++) {
      for(x=0; x<w_; x++) {
	z = matrix_[y*w_+x];
	if(z < upper_co_ || z > lower_co_) {
	  color[0] = 0;
	  color[1] = 0;
	  color[2] = 0;
	} else {
	  HSVtoRGB((1.0-z)*240, 1.0, 1.0, color);
	}
	uint *p = (uint *)image_.scanLine(y+10) + (x+40);
	*p = qRgb((int)(color[0] * 255 + 0.5),
		  (int)(color[1] * 255 + 0.5),
		  (int)(color[2] * 255 + 0.5));
      }
    }
  }
 
  pix_.convertFromImage(image_);

  if(!reuse) resizeContents(pix_.width(), pix_.height());
}

void
FitnessLandscapeView::setDefault(float x, float y) {
  int p_def_x = def_x_;
  int p_def_y = def_y_;
  def_x_ = (int)(x/step_ + 0.5);
  def_y_ = (int)(y/step_ + 0.5);
  updateContents(p_def_x-6+40, p_def_y-6+10, 12, 12);
  updateContents(def_x_-6+40, def_y_-6+10, 12, 12);
}

void
FitnessLandscapeView::setCurrent(float x, float y) {
  int p_cur_x = cur_x_;
  int p_cur_y = cur_y_;
  cur_x_ = (int)(x/step_ + 0.5);
  cur_y_ = (int)(y/step_ + 0.5);
  updateContents(p_cur_x-6+40, p_cur_y-6+10, 12, 12);
  updateContents(cur_x_-6+40, cur_y_-6+10, 12, 12);
}

void
FitnessLandscapeView::setCutoffs(float upper_co, float lower_co) {
  upper_co_ = upper_co;
  lower_co_ = lower_co;
  genImage(true);
  viewport()->update();
}

void
FitnessLandscapeView::contentsMousePressEvent(QMouseEvent *e) {
  int x = (e->x() - 40);
  int y = (e->y() - 10);
  if(0 <= x && x < w_ &&
     0 <= y && y < h_) {
    emit valueSelected(x*step_, y*step_);
  }
}

void
FitnessLandscapeView::contentsMouseMoveEvent(QMouseEvent *e) {
  int x = (e->x() - 40);
  int y = (e->y() - 10);
  if(0 <= x && x < w_ &&
     0 <= y && y < h_) {
    QString str;
    str.sprintf("%d %d %f\n", x*step_, y*step_, matrix_[y*w_+x]);
    emit statusMessage(str, 5000);
  }
}

void
FitnessLandscapeView::drawContents(QPainter* p, int cx, int cy, int cw, int ch) {
  /*
    cx, cy - starting position needing update
    cw, ch - width and height of area needing update
  */
  
  //printf("c:{%d %d} cs:{%d %d}  limits:{%d %d}\n", cx, cy, cw, ch, width(), height());

  //cout << "size:" << width() << '\t' << height() << '\n';
  //cout << "tw:" << tw << '\t' << "th:" << th << '\n';
  //cout << "contents: " << contentsWidth() << '\t' << contentsHeight() << '\n';

  //p->fillRect(cx, cy, cw, ch, white);

  p->drawPixmap(cx, cy, pix_, cx, cy, cw, ch);

  p->setPen(white);
  p->setRasterOp(XorROP);
  
  p->drawLine(def_x_-5+40, def_y_+10, def_x_+5+40, def_y_+10);
  p->drawLine(def_x_+40, def_y_-5+10, def_x_+40, def_y_+5+10);

  p->drawLine(cur_x_-5+40, cur_y_-5+10, cur_x_+5+40, cur_y_+5+10);
  p->drawLine(cur_x_-5+40, cur_y_+5+10, cur_x_+5+40, cur_y_-5+10);
}

//----------------------------------------------------------------------
