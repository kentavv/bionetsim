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

#include <qapplication.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qprinter.h>
#include <qsettings.h>
#include <qstatusbar.h>
#include <qvbox.h>
#include <qworkspace.h>

#include "network_evolve_app.h"
#include "pathway_editor.h"
#include "ode_gen.h"
#include "splash.h"
#include "version.h"
#include "document.xpm"
#include "fileopen.xpm"
#include "filesave.xpm"
#include "fileprint.xpm"

const QString WINDOWS_REGISTRY = "/ConfluenceRD/Bioinformatics";
const QString APP_KEY = "/Network Evolve/";

NetworkEvolveApp::NetworkEvolveApp()
  : QMainWindow(NULL),
    printer_(NULL)
{
  setupMenuBar();
  setupStatusBar();

  QVBox* vb = new QVBox(this);
  vb->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  ws_ = new QWorkspace(vb);
  setCentralWidget(vb);

  {
    QSettings settings;
    settings.insertSearchPath(QSettings::Windows, WINDOWS_REGISTRY);
    int windowWidth = settings.readNumEntry(APP_KEY + "WindowWidth", 650);
    int windowHeight = settings.readNumEntry(APP_KEY + "WindowHeight", 400);
    int windowX = settings.readNumEntry(APP_KEY + "WindowX", -1);
    int windowY = settings.readNumEntry(APP_KEY + "WindowY", -1);
    resize(windowWidth, windowHeight);
    if(windowX != -1 || windowY != -1) {
      move(windowX, windowY);
    }
  }
}

NetworkEvolveApp::~NetworkEvolveApp() {
  if(printer_) delete printer_;
}

PathwayEditor *
NetworkEvolveApp::newDoc() {
  PathwayEditor *pe = new PathwayEditor(ws_, 0, WDestructiveClose);
  connect(pe, SIGNAL(statusMessage(const QString &, int)),
	  SLOT(updateStatusMessage(const QString &, int)));
  pe->setCaption("unnamed document");
  pe->setIcon(QPixmap(document_xpm));
  // show the very first window in maximized mode
  if(ws_->windowList().isEmpty())
    pe->showMaximized();
  else
    pe->show();
  return pe;
}

void
NetworkEvolveApp::fileNew() {
  newDoc();
}

void
NetworkEvolveApp::fileOpen() {
  QString fn = QFileDialog::getOpenFileName(QString::null, "Network Files (*.net);;All Files (*)", this);
  open(fn);
}

void
NetworkEvolveApp::fileSave() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) pe->fileSave();
}

void
NetworkEvolveApp::fileSaveAs() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) pe->fileSaveAs();
}

void
NetworkEvolveApp::fileSaveSnapshot() {
  QString fn = QFileDialog::getSaveFileName(QString::null, "PNG Files (*.png);;All Files (*)", this);
  if(fn.isEmpty()) return;

  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    if(!pe->saveSnapshot(fn)) {
      QMessageBox::critical(NULL, appname, QString("Unable to save as ") + fn);
    }
  }
}

void
NetworkEvolveApp::fileExport() {
  QFileDialog d(QString::null, "Matrices (*.txt);;Matlab (*.mat);;C++ (*.cpp);;CVODE (*.cpp)", this, NULL, true);
  d.setMode(QFileDialog::AnyFile);
  if(d.exec() != QFileDialog::Accepted) return;
  
  QString fn = d.selectedFile();
  if(fn.isEmpty()) return;

  ode_format_t format;
  if(d.selectedFilter() == "Matrices (*.txt)") {
    format = Matrices;
  } else if(d.selectedFilter() == "Matlab (*.mat)") {
    format = Matlab;
  } else if(d.selectedFilter() == "C++ (*.cpp)") {
    format = CPP;
  } else if(d.selectedFilter() == "CVODE (*.cpp)") {
    format = CVODE;
  } else {
    QMessageBox::critical(NULL, appname, QString("Unknown format for ") + fn);
    return;
  }
  
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    if(!pe->exportAs(fn, format)) {
      QMessageBox::critical(NULL, appname, QString("Unable to export to ") + fn);
    }
  }
}

void
NetworkEvolveApp::filePrint() {
  if(!printer_) {
    printer_ = new QPrinter(QPrinter::PrinterResolution);
    printer_->setPageSize(QPrinter::Letter);
  }
  if(printer_->setup(this)) {
    PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
    if(pe) {
      QPaintDeviceMetrics m(printer_);
      QPainter pp(printer_);

      double sw = m.width()  / (double)pe->contentsWidth();
      double sh = m.height() / (double)pe->contentsHeight();
      if(pe->contentsWidth() * sh <= m.width()) {
	pp.scale(sh, sh);
      } else {
	pp.scale(sw, sw);
      }
	       
      pe->draw(&pp);
      pp.end();
    }
  }
}

void
NetworkEvolveApp::fileClose() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) pe->close();
}

bool
NetworkEvolveApp::closeAllWindows() {
  QWidgetList wl = ws_->windowList();

  PathwayEditor *pe;
  for(pe=(PathwayEditor*)(wl.first()); pe; pe=(PathwayEditor*)(wl.next())) {
    if(!pe->close()) return false;
  }

  return true;
}

void
NetworkEvolveApp::saveOptions() {
  QSettings settings;
  settings.insertSearchPath(QSettings::Windows, WINDOWS_REGISTRY);
  settings.writeEntry(APP_KEY + "WindowWidth", width());
  settings.writeEntry(APP_KEY + "WindowHeight", height());
  settings.writeEntry(APP_KEY + "WindowX", x());
  settings.writeEntry(APP_KEY + "WindowY", y());
}

void
NetworkEvolveApp::fileQuit() {
  if(!closeAllWindows()) return;
  close();
}

void
NetworkEvolveApp::editSelectAll() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    pe->editSelectAll();
  }  
}

void
NetworkEvolveApp::editLayoutGo() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    pe->editLayoutGo(true);
  }  
}

void
NetworkEvolveApp::editLayoutRandom() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    pe->editLayoutRandom();
  }  
}

void
NetworkEvolveApp::pathwayViewReport() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    pe->viewReport();
  }  
}

void
NetworkEvolveApp::pathwayEditorParams() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    pe->editEditorParams();
  }
}

void
NetworkEvolveApp::pathwaySimulationParams() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    pe->editSimulationParams();
  }
}

void
NetworkEvolveApp::simulationRunSimulation() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    pe->runSimulation();
  }
}

void
NetworkEvolveApp::simulationConcentrationDialog() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    pe->displayConcentrationDialog();
  }
}

void
NetworkEvolveApp::simulationDynamicDialog() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    pe->displayDynamicDialog();
  }
}

void
NetworkEvolveApp::simulationExportTimeSeries() {
  QString fn = QFileDialog::getSaveFileName(QString::null, "Text Files (*.txt);;All Files (*)", this);
  if(fn.isEmpty()) return;

  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    if(!pe->exportTimeSeries(fn)) {
      QMessageBox::critical(NULL, appname, QString("Unable to save as ") + fn);
    }
  }
}

void
NetworkEvolveApp::optimizationParameterScan() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    pe->displayParameterScanDialog();
  }
}

void
NetworkEvolveApp::optimizationEvolveGo() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    pe->optimizationEvolveGo(true);
  }  
}

void
NetworkEvolveApp::optimizationEvolveTracesDialog() {
  PathwayEditor *pe = (PathwayEditor*)ws_->activeWindow();
  if(pe) {
    pe->displayEvolveTracesDialog();
  }  
}

void
NetworkEvolveApp::helpHelp() {
}

void
NetworkEvolveApp::helpAbout() {
  showSplashScreen(0);
}

void
NetworkEvolveApp::setupMenuBar() {
  QPopupMenu *file = new QPopupMenu(this);
  menuBar()->insertItem("&File", file);

  file->insertItem("New", this, SLOT(fileNew()), CTRL + Key_N);
  file->insertItem(QPixmap(fileopen), "Open", this, SLOT(fileOpen()), CTRL + Key_O);
  file->insertSeparator();
  file->insertItem(QPixmap(filesave), "Save", this, SLOT(fileSave()), CTRL + Key_S);
  file->insertItem("Save As...", this, SLOT(fileSaveAs()));
  file->insertItem("Save Snapshot...", this, SLOT(fileSaveSnapshot()));
  file->insertItem("Export...", this, SLOT(fileExport()));
  file->insertSeparator();
  file->insertItem(QPixmap(fileprint), "Print...", this, SLOT(filePrint()), CTRL + Key_P);
  file->insertSeparator();
  file->insertItem("Close", this, SLOT(fileClose()), CTRL + Key_W);
  file->insertItem("Quit", this, SLOT(fileQuit()), CTRL + Key_Q);

  QPopupMenu *edit = new QPopupMenu(this);
  menuBar()->insertItem("&Edit", edit);

  edit->insertItem("Select &All", this, SLOT(editSelectAll()), CTRL + Key_A);
  QPopupMenu *layout = new QPopupMenu(this);
  layout->insertItem("Dynamic Layout", this, SLOT(editLayoutGo()));
  layout->insertItem("Random Layout", this, SLOT(editLayoutRandom()));
  edit->insertItem("Layout", layout);
  
  QPopupMenu *pathway = new QPopupMenu(this);
  menuBar()->insertItem("&Pathway", pathway);
  pathway->insertItem("View Report", this, SLOT(pathwayViewReport()), CTRL + Key_R);
  pathway->insertItem("Editor Parameters", this, SLOT(pathwayEditorParams()));
  pathway->insertItem("Simulation Parameters", this, SLOT(pathwaySimulationParams()));

  QPopupMenu *sim = new QPopupMenu(this);
  sim->insertItem("Execute Simulation", this, SLOT(simulationRunSimulation()), CTRL + Key_E);
  sim->insertItem("Display Concentration Time Series Dialog", this, SLOT(simulationConcentrationDialog()), CTRL + Key_T);
  sim->insertItem("Display Dynamic Dialog", this, SLOT(simulationDynamicDialog()));
  sim->insertItem("Export Time Series Data", this, SLOT(simulationExportTimeSeries()));
  pathway->insertItem("Simulation", sim);

  QPopupMenu *opt = new QPopupMenu(this);
  //opt->insertItem("Parameter Scan", this, SLOT(optimizationParameterScan()));
  opt->insertItem("Evolve", this, SLOT(optimizationEvolveGo()));
  opt->insertItem("Display Evolution Time Series Dialog", this, SLOT(optimizationEvolveTracesDialog()));
  pathway->insertItem("Optimization", opt);

  windows_ = new QPopupMenu(this);
  windows_->setCheckable(this);
  connect(windows_, SIGNAL(aboutToShow()), SLOT(windowsMenuAboutToShow()));
  menuBar()->insertItem("&Windows", windows_);

  QPopupMenu *help = new QPopupMenu(this);
  menuBar()->insertItem("&Help", help);

  //help->insertItem("&Help", this, SLOT(helpHelp()));
  help->insertItem("&About", this, SLOT(helpAbout()));
}

void
NetworkEvolveApp::setupStatusBar() {
  statusBar()->setSizeGripEnabled(false);
  statusBar()->message("Welcome to BioNetSim", 10000);
}

bool
NetworkEvolveApp::open(const QString &filename) {
  if(!filename.isEmpty()) {
    PathwayEditor *pe = newDoc();
    if(!pe->open(filename)) {
      delete pe;
      QMessageBox::critical(NULL, appname, QString("Unable to load ") + filename);
      return false;
    }
    return true;
  } else {
    statusBar()->message("Loading aborted", 5000);
  }
  return false;
}

void
NetworkEvolveApp::updateStatusMessage(const QString &str, int delay) {
  statusBar()->message(str, delay);
}

void
NetworkEvolveApp::windowsMenuAboutToShow() {
  windows_->clear();
  int cascadeId = windows_->insertItem("&Cascade", ws_, SLOT(cascade()));
  int tileId = windows_->insertItem("&Tile", ws_, SLOT(tile()));
  if(ws_->windowList().isEmpty()) {
    windows_->setItemEnabled(cascadeId, false);
    windows_->setItemEnabled(tileId, false);
  }
  windows_->insertSeparator();
  QWidgetList windows = ws_->windowList();
  for (int i = 0; i < int(windows.count()); i++) {
    int id = windows_->insertItem(windows.at(i)->caption(),
				  this, SLOT(windowsMenuActivated(int)));
    windows_->setItemParameter(id, i);
    windows_->setItemChecked(id, ws_->activeWindow() == windows.at(i));
  }
}

void
NetworkEvolveApp::windowsMenuActivated(int id) {
  QWidget *w = ws_->windowList().at(id);
  if(w) {
    w->showNormal();
    w->setFocus();
  }
}

void
NetworkEvolveApp::closeEvent(QCloseEvent *e) {
  saveOptions();
  if(closeAllWindows()) e->accept();
  else e->ignore();
}
