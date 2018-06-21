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

#ifndef _SLIDER_DIALOG_H_
#define _SLIDER_DIALOG_H_

#include <qdialog.h>
#include <qwidget.h>
#include <qlist.h>
#include <qscrollview.h>
#include <qstring.h>

#include "pathway_canvas_items.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QMouseEvent;
class QPopupMenu;
class QScrollView;
class QSlider;
class MolRxnBaseItem;

class SliderItem : public QWidget {
  Q_OBJECT
public:
  SliderItem(QWidget *parent, MolRxnBaseItem *item, const QString &comp);
  ~SliderItem();

signals:
  void parametersChanged();

protected slots:
  void updateFields();
  void sliderChanged(int);
  void sliderLimitsChanged();
  void curChanged();

protected:
  QLineEdit *min_, *max_, *cur_;
  QSlider *slider_;
  QLabel *name_, *rxn_;
  MolRxnBaseItem *item_;
  QString comp_;
};

class SliderScrollView : public QScrollView {
  Q_OBJECT
public:
  SliderScrollView(QWidget *parent);
  ~SliderScrollView();

  void setItems(const QList<PathwayCanvasItem> &items);

signals:
  void parametersChanged();

protected slots:  
  void sliderMenu(int);
  void updateSliderMenu();

protected:
  QList<PathwayCanvasItem> items_;
  QPopupMenu *slider_menu_;
  QList<SliderItem> sliders_;

  void mousePressEvent(QMouseEvent*);
};

class SliderDialog : public QDialog {
  Q_OBJECT
public:
  SliderDialog(QWidget *parent);
  ~SliderDialog();
  
  void setItems(const QList<PathwayCanvasItem> &items);

signals:
  void parametersChanged();

protected slots:
  void apply();
  void setParametersChanged();

protected:
  QPushButton *apply_;
  SliderScrollView *sv_;
};

#endif
