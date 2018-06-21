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

#ifndef _EDITOR_PARAMS_DIALOG_H_
#define _EDITOR_PARAMS_DIALOG_H_

#include <qdialog.h>
#include <qstringlist.h>

#include "editor.h"

class QCheckBox;
class QComboBox;
class QPushButton;
class QSpinBox;

class EditorParametersDialog : public QDialog {
  Q_OBJECT
public:
  EditorParametersDialog(QWidget *parent,
			 const Editor &editor);
  
  void getValues(Editor &editor);
  void setValues(const Editor &editor);
  
signals:
  void changed();
  void topologyChanged();
  
protected slots:
  void apply();
  void setChanged();
  void setTopologyChanged();
  
protected:
  QSpinBox *sim_delay_;
  QSpinBox *gsx_, *gsy_;
  QCheckBox *use_grid_;
  QCheckBox *display_grid_;
  QPushButton *apply_;

  bool changed_;
  bool topology_changed_;
};

#endif
