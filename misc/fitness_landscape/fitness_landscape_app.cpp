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
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif

#include <qapplication.h>
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qstatusbar.h>
#include <qtable.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qcheckbox.h>
#include <qvector.h>
#include <qlineedit.h>
#include <qslider.h>

#include "fitness_landscape_app.h"
#include "fitness_landscape_view.h"
#include "line_graph_view.h"
#include "matrix.h"
#include "version.h"
#include "calculate.h"
#include "fitness_functions.h"
#include "pathway_load.h"
#include "fileopen.xpm"
#include "filesave.xpm"
#include "fileprint.xpm"

static char *
str_clean_rn(char *str) {
  char *p;

  if((p=strrchr(str, '\r'))) *p='\0';
  if((p=strrchr(str, '\n'))) *p='\0';

  return str;
}

FitnessLandscapeApp::FitnessLandscapeApp()
  : QMainWindow(NULL),
    filename_(""),
    changed_(false),
    x_ind_(-1),
    y_ind_(-1),
    printer_(NULL)
{
  setupMenuBar();
  setupStatusBar();
  setupView();
  setDir("data");
  //setDir("pheno");
}

FitnessLandscapeApp::~FitnessLandscapeApp() {
  if(printer_) delete printer_;
}

void
FitnessLandscapeApp::fileSetDir() {
  QString fn = QFileDialog::getExistingDirectory(QString::null, this);
  if(fn.isEmpty()) return;

  if(!setDir(fn)) {
    QMessageBox::critical(NULL, appname, QString("Unable to load ") + fn);
    return;
  }
  filename_ = fn;
}

void
FitnessLandscapeApp::fileSaveImage() {
  QString fn = QFileDialog::getSaveFileName(filename_, "Text Files (*.txt);;All Files (*)", this);
  if(fn.isEmpty()) return;

  if(saveImage(filename_)) {
  } else {
    QMessageBox::critical(NULL, appname, QString("Unable to save ") + filename_);
  }
}

void
FitnessLandscapeApp::fileSaveParameters() {
  QString fn = QFileDialog::getSaveFileName(filename_, "Text Files (*.txt);;All Files (*)", this);
  if(fn.isEmpty()) return;

  filename_ = fn;
  
  if(saveParameters(filename_)) {
    changed_ = false;
  } else {
    QMessageBox::critical(NULL, appname, QString("Unable to save ") + filename_);
  }
}

void
FitnessLandscapeApp::filePrint() {
  if(!printer_) {
    printer_ = new QPrinter;
    printer_->setPageSize(QPrinter::Letter);
  }
  if(printer_->setup(this)) {
    //QPainter pp(printer);
    //canvas.drawArea(QRect(0,0,canvas.width(),canvas.height()),&pp,FALSE);
  }
}

void
FitnessLandscapeApp::fileQuit() {
  if(changed_) {
    switch(QMessageBox::warning(this, appname,
				"Current pathway is unsaved.\n"
				"Do you wish to save before quiting?\n"
				"Yes", "No", 0,
				0, 1 )) {
    case 0:
      fileSaveParameters();
      // If save failed, give the user another chance
      while(changed_) {
	fileSaveParameters();
      }
      break;
    case 1:
      break;
    }
  }
  qApp->quit();
}

void
FitnessLandscapeApp::helpHelp() {
}

void
FitnessLandscapeApp::helpAbout() {
  QString about =
    tr("<center><b>%1 %2</b><br>"
       "<br>"
       "Copyright &copy; 2002<br>"
       "Kent A. Vander Velden<br>"
       "<br>"
       "Compiled on " __DATE__ " at " __TIME__).arg(appname).arg(appversion);
  
  QMessageBox::about(this, appname, about);
}

void
FitnessLandscapeApp::setupMenuBar() {
  QPopupMenu *file = new QPopupMenu(this);
  menuBar()->insertItem("&File", file);

  file->insertItem(QPixmap(fileopen), "Open", this, SLOT(fileSetDir()), CTRL + Key_O);
  file->insertSeparator();
  file->insertItem(QPixmap(filesave), "Save Image", this, SLOT(fileSaveImage()), CTRL + Key_I);
  file->insertItem(QPixmap(filesave), "Save Parameters", this, SLOT(fileSaveParameters()), CTRL + Key_S);
  file->insertSeparator();
  file->insertItem(QPixmap(fileprint), "Print...", this, SLOT(filePrint()), CTRL + Key_P);
  file->insertSeparator();
  file->insertItem("Quit", this, SLOT(fileQuit()), CTRL + Key_Q);
  
  QPopupMenu *help = new QPopupMenu(this);
  menuBar()->insertItem("&Help", help);

  help->insertItem("&Help", this, SLOT(helpHelp()));
  help->insertItem("&About", this, SLOT(helpAbout()));
}

void
FitnessLandscapeApp::setupStatusBar() {
  statusBar()->setSizeGripEnabled(false);
  statusBar()->message("Welcome to BioNetSim Landscape", 10000);
}

void
FitnessLandscapeApp::setupView() {
  QVBox *vbox = new QVBox(this);

  QSplitter *splitter3 = new QSplitter(vbox);
  splitter3->setOrientation(QSplitter::Vertical);
  
  QSplitter *splitter1 = new QSplitter(splitter3);
  splitter1->setOrientation(QSplitter::Horizontal);
  view_ = new FitnessLandscapeView(splitter1);
  splitter1->setResizeMode(view_, QSplitter::Stretch);
  QSplitter *splitter2 = new QSplitter(splitter1);
  splitter2->setOrientation(QSplitter::Vertical);
  parameters_ = new QTable(1, 3, splitter2);
  //hbox->setStretchFactor(parameters_, 25);
  //Enable for Qt3
  parameters_->setColumnReadOnly(0, true);
  parameters_->setColumnReadOnly(1, true);
  parameters_->horizontalHeader()->setLabel(0, "Reaction");
  parameters_->horizontalHeader()->setLabel(1, "Wild Type");
  parameters_->horizontalHeader()->setLabel(2, "User Selected");

  molecules_ = new QTable(1, 3, splitter2);
  molecules_->setColumnReadOnly(0, true);
  molecules_->setColumnReadOnly(1, true);
  molecules_->horizontalHeader()->setLabel(0, "Molecule");
  molecules_->horizontalHeader()->setLabel(1, "Wild Type");
  molecules_->horizontalHeader()->setLabel(2, "User Selected");
  splitter1->setResizeMode(splitter2, QSplitter::KeepSize);

  lg_view_ = new LineGraphView(splitter3);

  QGrid *grid;
  QLabel *label;
  
  grid = new QGrid(2, QGrid::Horizontal, vbox);
  upper_label_ = new QLabel("Upper:", grid);
  upper_label_->setFixedWidth(upper_label_->width());
  upper_cos_ = new QSlider(0, 200, 1, 0, Horizontal, grid);
  lower_label_ = new QLabel("Lower:", grid);
  lower_label_->setFixedWidth(lower_label_->width());
  lower_cos_ = new QSlider(0, 200, 1, 200, Horizontal, grid);
  
  grid = new QGrid(2, QGrid::Horizontal, vbox);
  label = new QLabel("X Axis:", grid);
  label->setFixedWidth(label->width());
  x_axis_ = new QComboBox(false, grid);
  label =new QLabel("Y Axis:", grid);
  label->setFixedWidth(label->width());
  y_axis_ = new QComboBox(false, grid);

  connect(view_, SIGNAL(valueSelected(int, int)), SLOT(valueUpdated(int, int)));
  connect(view_, SIGNAL(statusMessage(const QString&, int)),
          SLOT(updateStatusMessage(const QString&, int)));
  connect(parameters_, SIGNAL(valueChanged(int, int)), SLOT(paramChanged(int, int)));
  connect(molecules_, SIGNAL(valueChanged(int, int)), SLOT(molChanged(int, int)));
  connect(upper_cos_, SIGNAL(valueChanged(int)), SLOT(cutoffSliderChanged()));
  connect(lower_cos_, SIGNAL(valueChanged(int)), SLOT(cutoffSliderChanged()));
  connect(x_axis_, SIGNAL(activated(int)), SLOT(setXAxis(int)));
  connect(y_axis_, SIGNAL(activated(int)), SLOT(setYAxis(int)));

  setCentralWidget(vbox);
}

void
FitnessLandscapeApp::cutoffSliderChanged() {
  double upper = upper_cos_->value() / 200.0;
  double lower = lower_cos_->value() / 200.0;
  QString str;
  str.sprintf("Upper (%0.3f):", upper);
  upper_label_->setText(str);
  str.sprintf("Lower (%0.3f):", lower);
  lower_label_->setText(str);
  view_->setCutoffs(upper, lower);
}

void
FitnessLandscapeApp::paramChanged(int r, int c) {
  if(c != 2) return;
  if(r >= 1) {
    kc_user_values_[r-1] = parameters_->text(r, c).toFloat();
    calculateUser();
    if(r-1 == x_ind_ || r-1 == y_ind_) {
      view_->setCurrent(kc_user_values_[x_ind_], kc_user_values_[y_ind_]);
    }
  } else {
    calculateUser();
  }
}

void
FitnessLandscapeApp::molChanged(int r, int c) {
  if(c != 2) return;
  if(r >= 1) {
    //mol_user_values_[r-1] = molecules_->text(r, c).toFloat();
    calculateUser();
  } else {
    calculateUser();
  }
}

void
FitnessLandscapeApp::valueUpdated(int x, int y) {
  QString str;

  str.setNum(x);
  parameters_->setText(x_ind_+1, 2, str);  
  kc_user_values_[x_ind_] = x;
  
  str.setNum(y);
  parameters_->setText(y_ind_+1, 2, str);  
  kc_user_values_[y_ind_] = y;

  view_->setCurrent(x, y);

  calculateUser();
}

bool
FitnessLandscapeApp::setDir(QString filename) {
  if(chdir((const char*)filename) == -1) {
    return false;
  }

  QString pw_filename = "gal4.net";
  if(!pathwayLoad(pw_filename, pem_)) {
    fprintf(stderr, "Unable to parse pathway file %s\n", (const char*)pw_filename);
    return false;
  }
  
  loadValuesLabels("kc.txt", kc_values_, kc_labels_);
  kc_user_values_ = kc_values_;
  loadValuesLabels("mol.txt", mol_values_, mol_labels_);
  mol_user_values_ = mol_values_;

  if(kc_values_.count() != pem_.kc.nrows()) abort();
  
  {
    x_axis_->clear();
    x_axis_->insertStringList(kc_labels_);

    y_axis_->clear();
    y_axis_->insertStringList(kc_labels_);
  }

  {
    parameters_->setNumRows(28);
    parameters_->setText(0, 0, "Fitness");
    parameters_->setText(0, 1, "");
    parameters_->setText(0, 2, "");
    parameters_->setText(0, 3, "");

    int i;
    
    for(i=0; i<27; i++) {
      parameters_->setText(i+1, 0, kc_labels_[i]);
      QString str;
      str.setNum(kc_values_[i]);
      parameters_->setText(i+1, 1, str);
      str.setNum(kc_user_values_[i]);
      parameters_->setText(i+1, 2, str);
    }

    molecules_->setNumRows(18);
    
    for(i=0; i<18; i++) {
      molecules_->setText(i, 0, mol_labels_[i]);
      QString str;
      str.setNum(mol_values_[i]);
      molecules_->setText(i, 1, str);
      str.setNum(mol_user_values_[i]);
      molecules_->setText(i, 2, str);
    }    
  }

  calculate();
  calculateUser();

  setAxies(0, 1);
  return true;
}

bool
FitnessLandscapeApp::loadValuesLabels(QString filename,
				      QValueList<float> &values,
				      QStringList &labels) {
  FILE *file = fopen(filename, "r");
  if(!file) return false;

  values.clear();
  labels.clear();
  
  char line[1024];
  while(fgets(line, 1024, file)) {
    str_clean_rn(line);
    char *p = strchr(line, '\t');
    if(!p) continue;
    *p = '\0';
    values.append(QString(line).toFloat());
    labels.append(QString(p+1));
  }
  
  fclose(file);
  return true;  
}

void
FitnessLandscapeApp::setAxies(int x_ind, int y_ind) {
  if(x_ind < 0 || 27 <= x_ind ||
     y_ind < 0 || 27 <= y_ind) return;

  x_ind_ = x_ind;
  y_ind_ = y_ind;
  
  x_axis_->setCurrentItem(x_ind_);
  y_axis_->setCurrentItem(y_ind_);

  view_->loadData(x_ind_, y_ind_);
  view_->setDefault(kc_values_[x_ind_], kc_values_[y_ind_]);
  view_->setCurrent(kc_user_values_[x_ind_], kc_user_values_[y_ind_]);
}

void
FitnessLandscapeApp::setXAxis(int ind) {
  setAxies(ind, y_ind_);
}

void
FitnessLandscapeApp::setYAxis(int ind) {
  setAxies(x_ind_, ind);
}

void
FitnessLandscapeApp::calculate() {
  for(int i=0; i<kc_values_.count(); i++) {
    pem_.kc(i, 0) = kc_user_values_[i];
  }
  Matrix results;
  double val = -1.0;
  if(run_simulation(pem_, results)) {
    val = calculate_fitness(pem_, results);
  }
  if(val < 0.0) {
    //updateStatusMessage("WARNING: ODE Solver failed", 5000);
    QMessageBox::warning(this, appname, "ODE Solver failed");
  }
  
  QString str;
  str.setNum(val);
  parameters_->setText(0, 1, str);
}

void
FitnessLandscapeApp::calculateUser() {
  for(int i=0; i<kc_user_values_.count(); i++) {
    pem_.kc(i, 0) = kc_user_values_[i];
  }
  Matrix results;
  double val = -1.0;
  if(run_simulation(pem_, results)) {
    val = calculate_fitness(pem_, results);
  }
  if(val < 0.0) {
    //updateStatusMessage("WARNING: ODE Solver failed", 5000);
    QMessageBox::warning(this, appname, "ODE Solver failed");
  }
  lg_view_->setData(results, mol_labels_);

  QString str;
  str.setNum(val);
  parameters_->setText(0, 2, str);
}

bool
FitnessLandscapeApp::saveImage(QString filename) {
  return false;
}

bool
FitnessLandscapeApp::saveParameters(QString filename) {
  return false;
}

void
FitnessLandscapeApp::updateStatusMessage(const QString &str, int delay) {
  statusBar()->message(str, delay);
}
