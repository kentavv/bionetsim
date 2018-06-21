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

#ifndef _NETWORK_EDITOR_APP_H_
#define _NETWORK_EDITOR_APP_H_

#include <qlist.h>
#include <qmainwindow.h>
#include <qstring.h>
#include <qstringlist.h>

#include "pathway_canvas_items.h"

class QPrinter;
class QProgressBar;
class QWorkspace;
class NetworkEditor;

class NetworkEditorApp : public QMainWindow {
  Q_OBJECT

public:
  NetworkEditorApp(QWidget *parent=0, const char *name=NULL, WFlags f=WType_TopLevel, bool embedded=false);
  ~NetworkEditorApp();

  bool open(const QString &);
  NetworkEditor *newDoc();
  
  int numNetworks() const;
  QList<NetworkEditor> networkEditors() const;
  NetworkEditor *activeNetworkEditor();

signals:
  void changed();
  void topologyChanged();
  
public slots:
  void cascade();
  void tile();
 
protected slots:
  void fileNew();
  void fileOpen();
  void fileSave();
  void fileSaveAs();
  void fileSaveAll();
  void fileSaveSnapshot();
  void fileSaveSVG();
  void fileExport();
  void filePrint();
  void fileClose();
  void fileOpenRecent(int);

  void editUndo();
  void editCut();
  void editCopy();
  void editCopyToClipboard();
  void editPaste();
  void editClear();
  void editSelectAll();
  void editLayoutGo();
  void editLayoutRandom();  
  void editSnapToGrid();
  void editTrimBorder();

  void pathwayViewReport();
  void pathwaySliders();
	void pathwayReactionEntry();
  void pathwayEditorParams();

  void simulationParams();
  void simulationRunSimulation();
  void simulationConcentrationDialog();
  void simulationDynamicDialog();
  void simulationExportTimeSeries();

  void showLog();
  
  void fitnessCalculate();
  void fitnessParams();

  void optimizationParameterScan();
  void optimizationParameterScan2D();

  void helpHelp();
  void helpAbout();

  void updateStatusMessage(const QString &, int);

  void editEnvironments();
  
  void environmentsMenuAboutToShow();
  void environmentsMenuActivated(int);
  void windowsMenuAboutToShow();
  void windowsMenuActivated(int);
  
protected:
  QPrinter *printer_;
  QWorkspace *ws_;
  QPopupMenu *windows_, *envs_;
  QProgressBar *progress_;
  QList<PathwayCanvasItem> copied_items_;
  
  void setupMenuBar();
  void setupStatusBar();

  void closeEvent(QCloseEvent*);
  void saveOptions();

  QPopupMenu *file_menu_;
  QStringList recent_files_;
  void updateRecentFilesMenu();
  void updateRecentFiles(const QString&);

  bool embedded_;
};
  
#endif
