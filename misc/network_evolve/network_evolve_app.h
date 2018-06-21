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

#ifndef _NETWORK_EVOLVE_APP_H_
#define _NETWORK_EVOLVE_APP_H_

#include <qmainwindow.h>
#include <qstring.h>

class QPrinter;
class QWorkspace;
class PathwayEditor;

class NetworkEvolveApp : public QMainWindow {
  Q_OBJECT

public:
  NetworkEvolveApp();
  ~NetworkEvolveApp();

  bool open(const QString &);
  
protected slots:
  void fileNew();
  void fileOpen();
  void fileSave();
  void fileSaveAs();
  void fileSaveSnapshot();
  void fileExport();
  void filePrint();
  void fileClose();
  void fileQuit();

  void editSelectAll();
  void editLayoutGo();
  void editLayoutRandom();  

  void pathwayViewReport();
  void pathwayEditorParams();
  void pathwaySimulationParams();

  void simulationRunSimulation();
  void simulationConcentrationDialog();
  void simulationDynamicDialog();
  void simulationExportTimeSeries();

  void optimizationParameterScan();
  void optimizationEvolveGo();  
  void optimizationEvolveTracesDialog();

  void helpHelp();
  void helpAbout();

  void updateStatusMessage(const QString &, int);

  void windowsMenuAboutToShow();
  void windowsMenuActivated(int);
  
protected:
  QPrinter *printer_;
  QWorkspace *ws_;
  QPopupMenu *windows_;
  
  void setupMenuBar();
  void setupStatusBar();
  PathwayEditor *newDoc();

  void closeEvent(QCloseEvent*);
  bool closeAllWindows();
  void saveOptions();
};
  
#endif
