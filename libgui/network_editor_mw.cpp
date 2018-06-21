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
#include <qdesktopwidget.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qprinter.h>
#include <qprogressbar.h>
#include <qsettings.h>
#include <qstatusbar.h>
#include <qtextedit.h>
#include <qvbox.h>
#include <qworkspace.h>

#include "log.h"
#include "pathway_canvas_items.h"
#include "network_editor_mw.h"
#include "network_editor.h"
#include "ode_gen.h"
#include "splash_screen.h"
#include "version.h"

#include "document.xpm"
#include "fileopen.xpm"
#include "filesave.xpm"
#include "fileprint.xpm"

static int max_recent_files = 5;

inline bool
isNetworkEditor(QWidget *w) {
  return strcmp(w->className(), "NetworkEditor") == 0;
}

NetworkEditorApp::NetworkEditorApp(QWidget *parent, const char *name, WFlags f, bool embedded)
  : QMainWindow(parent, name, f),
    printer_(NULL),
    embedded_(embedded)
{
  setupMenuBar();
  setupStatusBar();

  QVBox* vb = new QVBox(this);
  vb->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  ws_ = new QWorkspace(vb);
  setCentralWidget(vb);

  if(!embedded_) {
    // Log has already been created if this is an embedded object
    QTextEdit *log = createLog(NULL);
	log->hide();
  }
  
  {
    QSettings settings;
    settings.insertSearchPath(QSettings::Windows, registry);
    int windowWidth = settings.readNumEntry(app_key + "WindowWidth", 650);
    int windowHeight = settings.readNumEntry(app_key + "WindowHeight", 400);
    int windowX = settings.readNumEntry(app_key + "WindowX", -1);
    int windowY = settings.readNumEntry(app_key + "WindowY", -1);
    bool maximized = settings.readBoolEntry(app_key + "Maximized", false);
    QRect geo(windowX, windowY, windowWidth, windowHeight);
    
    QDesktopWidget d;
    QRect sg = d.screenGeometry();
    if(!sg.contains(geo)) {
      pneDebug("Resetting crazy geometry settings");
      geo.setRect(0, 0, 400, 400);
    }
		   
    resize(geo.width(), geo.height());
    move(geo.x(), geo.y());
    if(maximized) showMaximized();

    for(int i=0; i<max_recent_files; i++) {
      QString fn = settings.readEntry(app_key + "File" +
				      QString::number(i + 1));
      if(fn.isEmpty()) break;
      recent_files_.push_back(fn);
    }
    if(!recent_files_.isEmpty()) {
      updateRecentFilesMenu();
    }
  }

  copied_items_.setAutoDelete(true);
}

NetworkEditorApp::~NetworkEditorApp() {
  if(printer_) delete printer_;
}

NetworkEditor *
NetworkEditorApp::newDoc() {
  NetworkEditor *pe = new NetworkEditor(ws_, 0, WDestructiveClose);
  pe->setProgressBar(progress_);
  connect(pe, SIGNAL(statusMessage(const QString &, int)),
	  SLOT(updateStatusMessage(const QString &, int)));
  pe->setCaption("unnamed document");
  pe->setIcon(QPixmap(document_xpm));
  // show the very first window (excluding the log window) in maximized mode
  if(ws_->windowList().count() <= 1)
    pe->showMaximized();
  else
    pe->show();
  connect(pe, SIGNAL(changed()), SIGNAL(changed()));
  connect(pe, SIGNAL(topologyChanged()), SIGNAL(topologyChanged()));
  return pe;
}

int
NetworkEditorApp::numNetworks() const {
  return networkEditors().count();
}

QList<NetworkEditor>
NetworkEditorApp::networkEditors() const {
  QList<NetworkEditor> nes;

  QWidgetList wl = ws_->windowList();

  for(QWidget *w=wl.first(); w; w=wl.next()) {
    if(isNetworkEditor(w)) {
      nes.append((NetworkEditor*)w); 
    }
  }

  return nes;
}

NetworkEditor *
NetworkEditorApp::activeNetworkEditor() {
  NetworkEditor *pe = (NetworkEditor*)ws_->activeWindow();
  return (pe && isNetworkEditor(pe)) ? pe : NULL;
}

void
NetworkEditorApp::fileNew() {
  newDoc();
}

void
NetworkEditorApp::fileOpen() {
  QString fn = QFileDialog::getOpenFileName(QString::null, "Network Files (*.net);;All Files (*)", this);
  if(open(fn)) emit topologyChanged();
}

void
NetworkEditorApp::fileSave() {
  QWidget *w = ws_->activeWindow();
  if(w) {
    if(isNetworkEditor(w)) {
      if(((NetworkEditor*)w)->fileSave()) {
	updateRecentFiles(((NetworkEditor*)w)->filename());
      }
    } else if(w->className() == "QTextEdit") {
    }
  }
}

void
NetworkEditorApp::fileSaveAs() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    if(pe->fileSaveAs()) {
      updateRecentFiles(pe->filename());
    }
  }
}

void
NetworkEditorApp::fileSaveAll() {
  QWidgetList wl = ws_->windowList();

  NetworkEditor *pe;
  for(pe=(NetworkEditor*)(wl.first()); pe; pe=(NetworkEditor*)(wl.next())) {
    if(isNetworkEditor(pe) && pe->fileSave()) {
      updateRecentFiles(pe->filename());
    }
  }
}

void
NetworkEditorApp::fileSaveSnapshot() {
  QString fn = QFileDialog::getSaveFileName(QString::null, "PNG Files (*.png);;All Files (*)", this);
  if(fn.isEmpty()) return;

  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    if(!pe->saveSnapshot(fn)) {
      QMessageBox::critical(NULL, appname, QString("Unable to save as ") + fn);
    }
  }
}

void
NetworkEditorApp::fileSaveSVG() {
  QString fn = QFileDialog::getSaveFileName(QString::null, "SVG Files (*.svg);;All Files (*)", this);
  if(fn.isEmpty()) return;

  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    if(!pe->saveSVG(fn)) {
      QMessageBox::critical(NULL, appname, QString("Unable to save as ") + fn);
    }
  }
}

void
NetworkEditorApp::fileExport() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    QString filter;
    QString fn = QFileDialog::getSaveFileName(QString::null, "Matrices (*.txt);;Matlab (*.mat);;C++ (*.cpp);;CVODE (*.cpp)", this, NULL, QString::null, &filter);

    if(fn.isEmpty()) return;
    
    ode_format_t format;
    if(filter == "Matrices (*.txt)") {
      format = Matrices;
    } else if(filter == "Matlab (*.mat)") {
      format = Matlab;
    } else if(filter == "C++ (*.cpp)") {
      format = CPP;
    } else if(filter == "CVODE (*.cpp)") {
      format = CVODE;
    } else {
      QMessageBox::critical(NULL, appname, QString("Unknown format for ") + fn);
      return;
    }
  
    if(!pe->exportAs(fn, format)) {
      QMessageBox::critical(NULL, appname, QString("Unable to export to ") + fn);
    }
  }
}

void
NetworkEditorApp::filePrint() {
  if(!printer_) {
    printer_ = new QPrinter(QPrinter::PrinterResolution);
    printer_->setPageSize(QPrinter::Letter);
  }
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    if(printer_->setup(this)) {
      QPaintDeviceMetrics m(printer_);
      QPainter pp(printer_);
      
      double sw = m.width()  / (double)pe->contentsWidth();
      double sh = m.height() / (double)pe->contentsHeight();
      if(pe->contentsWidth() * sh <= m.width()) {
	pp.scale(sh, sh);
#ifdef WIN32
	QFont f2 = font();
	f2.setPointSizeFloat(f2.pointSizeFloat() / sh);
	pp.setFont(f2);
#endif
      } else {
	pp.scale(sw, sw);
#ifdef WIN32
	QFont f2 = font();
	f2.setPointSizeFloat(f2.pointSizeFloat() / sw);
	pp.setFont(f2);
#endif
      }

      pe->draw(&pp);
      pp.end();
    }
  }
}

void
NetworkEditorApp::fileClose() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->close();
  }
}

void
NetworkEditorApp::saveOptions() {
  QSettings settings;
  settings.insertSearchPath(QSettings::Windows, registry);
  settings.writeEntry(app_key + "WindowWidth", width());
  settings.writeEntry(app_key + "WindowHeight", height());
  settings.writeEntry(app_key + "WindowX", x());
  settings.writeEntry(app_key + "WindowY", y());
  settings.writeEntry(app_key + "Maximized", isMaximized());
  for(int i=0; i<(int)recent_files_.count(); i++) {
    settings.writeEntry(app_key + "File" + QString::number(i + 1),
			recent_files_[i]);
  }
}

void
NetworkEditorApp::editUndo() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editUndo();
  }  
}

void
NetworkEditorApp::editCut() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editCut(copied_items_);
  }  
}

void
NetworkEditorApp::editCopy() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editCopy(copied_items_);
  }  
}

void
NetworkEditorApp::editCopyToClipboard() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->copyToClipboard();
  }
}

void
NetworkEditorApp::editPaste() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editPaste(copied_items_);
  }  
}

void
NetworkEditorApp::editClear() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editClear();
  }  
}

void
NetworkEditorApp::editSelectAll() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editSelectAll();
  }  
}

void
NetworkEditorApp::editLayoutGo() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editLayoutGo(true);
  }  
}

void
NetworkEditorApp::editLayoutRandom() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editLayoutRandom();
  }  
}

void
NetworkEditorApp::editSnapToGrid() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editSnapToGrid();
  }
}

void
NetworkEditorApp::editTrimBorder() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editTrimBorder();
  }
}

void
NetworkEditorApp::pathwayViewReport() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->viewReport();
  }  
}

void
NetworkEditorApp::pathwaySliders() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->sliders();
  }  
}

void
NetworkEditorApp::pathwayReactionEntry() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->reactionEntry();
  }  
}

void
NetworkEditorApp::pathwayEditorParams() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editEditorParams();
  }
}

void
NetworkEditorApp::simulationParams() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editSimulationParams();
  }
}

void
NetworkEditorApp::simulationRunSimulation() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->runSimulation();
  }
}

void
NetworkEditorApp::simulationConcentrationDialog() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->displayConcentrationDialog();
  }
}

void
NetworkEditorApp::simulationDynamicDialog() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->displayDynamicDialog();
  }
}

void
NetworkEditorApp::simulationExportTimeSeries() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    QString fn = QFileDialog::getSaveFileName(QString::null, "Text Files (*.txt);;All Files (*)", this);
    if(fn.isEmpty()) return;
    
    if(!pe->exportTimeSeries(fn)) {
      QMessageBox::critical(NULL, appname, QString("Unable to save as ") + fn);
    }
  }
}

void
NetworkEditorApp::fitnessCalculate() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->fitnessCalculate();
  }
}

void
NetworkEditorApp::fitnessParams() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->fitnessParams();
  }
}

void
NetworkEditorApp::optimizationParameterScan() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->displayParameterScanDialog();
  }
}

void
NetworkEditorApp::optimizationParameterScan2D() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->displayParameterScan2DDialog();
  }
}

void
NetworkEditorApp::helpHelp() {
}

void
NetworkEditorApp::helpAbout() {
  QPixmap pix = QPixmap::fromMimeSource("SplashScreen");
  SplashScreen *splash = new SplashScreen("", pix, 0);
  splash->show();
}

void
NetworkEditorApp::setupMenuBar() {
  int id;
  
  file_menu_ = new QPopupMenu(this);
  menuBar()->insertItem("&File", file_menu_);

  file_menu_->insertItem("New", this, SLOT(fileNew()), CTRL + Key_N);
  file_menu_->insertItem(QPixmap(fileopen), "Open", this, SLOT(fileOpen()), CTRL + Key_O);
  file_menu_->insertItem("Close", this, SLOT(fileClose()), CTRL + Key_W);
  file_menu_->insertSeparator();
  file_menu_->insertItem(QPixmap(filesave), "Save", this, SLOT(fileSave()), CTRL + Key_S);
  file_menu_->insertItem("Save As...", this, SLOT(fileSaveAs()));
  file_menu_->insertItem("Save All...", this, SLOT(fileSaveAll()));
  file_menu_->insertItem("Save Snapshot...", this, SLOT(fileSaveSnapshot()));
  file_menu_->insertItem("Save SVG...", this, SLOT(fileSaveSVG()));
  file_menu_->insertItem("Export...", this, SLOT(fileExport()));
  file_menu_->insertSeparator();
  file_menu_->insertItem(QPixmap(fileprint), "Print...", this, SLOT(filePrint()), CTRL + Key_P);
  file_menu_->insertSeparator();
  file_menu_->insertSeparator();
  id = file_menu_->insertItem("Quit", qApp, SLOT(closeAllWindows()), CTRL + Key_Q);
  file_menu_->setItemEnabled(id, !embedded_);
  
  QPopupMenu *edit = new QPopupMenu(this);
  menuBar()->insertItem("&Edit", edit);

  edit->insertItem("&Undo", this, SLOT(editUndo()), CTRL + Key_Z);
  edit->insertItem("Cu&t", this, SLOT(editCut()), CTRL + Key_X);
  edit->insertItem("&Copy", this, SLOT(editCopy()), CTRL + Key_C);
  edit->insertItem("Copy to Clipboard", this, SLOT(editCopyToClipboard()));
  edit->insertItem("&Paste", this, SLOT(editPaste()), CTRL + Key_V);
  edit->insertItem("Clear", this, SLOT(editClear()));

  edit->insertItem("Select &All", this, SLOT(editSelectAll()), CTRL + Key_A);
  edit->insertSeparator();
  QPopupMenu *layout = new QPopupMenu(this);
  layout->insertItem("Dynamic Layout", this, SLOT(editLayoutGo()));
  layout->insertItem("Random Layout", this, SLOT(editLayoutRandom()));
  edit->insertItem("Layout", layout);
  edit->insertSeparator();
  edit->insertItem("&Snap To Grid", this, SLOT(editSnapToGrid()));
  edit->insertItem("&Trim Border", this, SLOT(editTrimBorder()));
  
  QPopupMenu *pathway = new QPopupMenu(this);
  menuBar()->insertItem("&Network", pathway);
  pathway->insertItem("View &Report", this, SLOT(pathwayViewReport()), CTRL + Key_R);
  pathway->insertItem("Sliders", this, SLOT(pathwaySliders()));
  pathway->insertItem("&Manual Reaction Entry", this, SLOT(pathwayReactionEntry()));
  pathway->insertSeparator();
  pathway->insertItem("&Options...", this, SLOT(pathwayEditorParams()));

  QPopupMenu *sim = new QPopupMenu(this);
  menuBar()->insertItem("&Simulation", sim);
  sim->insertItem("&Execute", this, SLOT(simulationRunSimulation()), CTRL + Key_E);
  sim->insertSeparator();
  sim->insertItem("&Time Series Dialog", this, SLOT(simulationConcentrationDialog()), CTRL + Key_T);
  sim->insertItem("&Dynamic Dialog", this, SLOT(simulationDynamicDialog()));
  sim->insertItem("Export Time &Series", this, SLOT(simulationExportTimeSeries()));
  sim->insertSeparator();
  sim->insertItem("&Options...", this, SLOT(simulationParams()));
  envs_ = new QPopupMenu(this);
  sim->insertItem("Environment", envs_);
  connect(envs_, SIGNAL(aboutToShow()), SLOT(environmentsMenuAboutToShow()));
  
  QPopupMenu *opt = new QPopupMenu(this);
  menuBar()->insertItem("&Optimization", opt);
  QPopupMenu *fitness = new QPopupMenu(this);
  fitness->insertItem("&Calculate", this, SLOT(fitnessCalculate()), CTRL + Key_F);
  fitness->insertItem("Options...", this, SLOT(fitnessParams()));
  opt->insertItem("Fitness Function", fitness);
  opt->insertSeparator();
  opt->insertItem("&Parameter Scan", this, SLOT(optimizationParameterScan()));
  opt->insertItem("&Parameter Scan 2D", this, SLOT(optimizationParameterScan2D()));

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
NetworkEditorApp::setupStatusBar() {
  statusBar()->setSizeGripEnabled(false);
  statusBar()->message("Welcome to GenoDYN", 10000);
  progress_ = new QProgressBar(statusBar());
  progress_->setFixedWidth(200);
  progress_->setPercentageVisible(false);
  statusBar()->addWidget(progress_, 0, true);
}

bool
NetworkEditorApp::open(const QString &filename) {
  if(!filename.isEmpty()) {
    NetworkEditor *pe = newDoc();
    if(!pe->open(filename)) {
      delete pe;
      QMessageBox::critical(NULL, appname, QString("Unable to load ") + filename);
      return false;
    }
    updateRecentFiles(QFileInfo(filename).absFilePath());
    return true;
  } else {
    statusBar()->message("Loading aborted", 5000);
  }
  return false;
}

void
NetworkEditorApp::updateStatusMessage(const QString &str, int delay) {
  statusBar()->message(str, delay);
}

void
NetworkEditorApp::editEnvironments() {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    pe->editEnvironments();
  }
}

void
NetworkEditorApp::environmentsMenuAboutToShow() {
  envs_->clear();
  envs_->insertItem("&Edit", this, SLOT(editEnvironments()));
  envs_->insertSeparator();
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    QStringList envs = pe->simulation().environmentDescriptions();
    int cur_env = pe->simulation().curEnvironment();
    QStringList::iterator i;
    int j;
    for(i = envs.begin(), j = 0; i != envs.end(); i++, j++) {
      int id = envs_->insertItem(*i,
				 this, SLOT(environmentsMenuActivated(int)));
      envs_->setItemParameter(id, j);
      envs_->setItemChecked(id, j == cur_env);
    }
  }
}

void
NetworkEditorApp::environmentsMenuActivated(int id) {
  NetworkEditor *pe = activeNetworkEditor();
  if(pe) {
    //printf("Setting env to %d\n", id);
    pe->setCurEnvironment(id);
  }
}

void
NetworkEditorApp::windowsMenuAboutToShow() {
  windows_->clear();
  windows_->insertItem("&Show Log Window", this, SLOT(showLog()));
  windows_->insertSeparator();
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
NetworkEditorApp::windowsMenuActivated(int id) {
  QWidget *w = ws_->windowList().at(id);
  if(w) {
    w->showNormal();
    w->setFocus();
  }
}

void
NetworkEditorApp::closeEvent(QCloseEvent *e) {
  QWidgetList wl = ws_->windowList();

  for(int i=0; i<int(wl.count()); i++) {
    QWidget *w = wl.at(i);
    if(!w->close()) {
      e->ignore();
      return;
    }
  }

  saveOptions();

  QMainWindow::closeEvent(e);
}

void
NetworkEditorApp::cascade() {
  ws_->cascade();
}

void
NetworkEditorApp::tile() {
  ws_->tile();
}

void
NetworkEditorApp::fileOpenRecent(int index) {
  open(recent_files_[index]);
}

void
NetworkEditorApp::updateRecentFilesMenu() {
  for(int i=0; i<max_recent_files; i++) {
    if(file_menu_->findItem(i)) {
      file_menu_->removeItem(i);
    }
    if(i < (int)recent_files_.count()) {
      file_menu_->insertItem(QString("&%1 %2").arg(i+1).arg(recent_files_[i]),
			     this, SLOT(fileOpenRecent(int)),
			     0, i, 13+i);
    }
  }
}

void
NetworkEditorApp::updateRecentFiles(const QString &fn) {
  if(recent_files_.find(fn) != recent_files_.end()) {
    return;
  }
  
  recent_files_.push_back(fn);
  if((int)recent_files_.count() > max_recent_files) {
    recent_files_.pop_front();
  }

  updateRecentFilesMenu();
}

void
NetworkEditorApp::showLog() {
	QTextEdit *log = pneLog();
	if(log) log->show();
}
