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

#ifndef _FITNESS_LANDSCAPE_APP_H_
#define _FITNESS_LANDSCAPE_APP_H_

#include <qmainwindow.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qvector.h>

#include "pathway_matrix.h"

class FitnessLandscapeView;
class LineGraphView;
class QPrinter;
class QTable;
class QComboBox;
class QCheckBox;
class QSlider;
class QLabel;

class FitnessLandscapeApp : public QMainWindow {
  Q_OBJECT

public:
  FitnessLandscapeApp();
  ~FitnessLandscapeApp();

  bool setDir(QString directory);
  bool saveImage(QString filename);
  bool saveParameters(QString filename);

protected slots:
  void fileSetDir();
  void fileSaveImage();
  void fileSaveParameters();
  void filePrint();
  void fileQuit();

  void helpHelp();
  void helpAbout();

  void updateStatusMessage(const QString &str, int delay);

  void setXAxis(int);
  void setYAxis(int);

  void calculate();
  void calculateUser();

  void cutoffSliderChanged();
  void paramChanged(int, int);
  void molChanged(int, int);
  void valueUpdated(int, int);
  
protected:
  QString filename_;
  bool changed_;
  int x_ind_, y_ind_;

  PathwayMatrix pem_;

  QPrinter *printer_;

  FitnessLandscapeView *view_;
  QTable *parameters_;
  QTable *molecules_;
  LineGraphView *lg_view_;
  QSlider *upper_cos_;
  QSlider *lower_cos_;
  QLabel *upper_label_;
  QLabel *lower_label_;
  QComboBox *x_axis_;
  QComboBox *y_axis_;

  QStringList kc_labels_;
  QValueList<float> kc_values_;
  QValueList<float> kc_user_values_;

  QStringList mol_labels_;
  QValueList<float> mol_values_;
  QValueList<float> mol_user_values_;

  void setupMenuBar();
  void setupStatusBar();
  void setupView();

  bool loadValuesLabels(QString, QValueList<float>&, QStringList&);
  bool loadData(QString);
  void setAxies(int x, int y);
};
  
#endif
