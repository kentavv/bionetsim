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

#ifndef _FITNESS_PARAMS_DIALOG_H_
#define _FITNESS_PARAMS_DIALOG_H_

#include <qdialog.h>
#include <qstringlist.h>

#include "fitness.h"

class QCheckBox;
class QComboBox;
class QGroupBox;
class QPushButton;
class QTable;
class QTextEdit;

class FitnessParametersDialog : public QDialog {
  Q_OBJECT
public:
  FitnessParametersDialog(QWidget *parent,
			  const QStringList &mol_names,
			  const QStringList &env_names,
			  const Fitness &fitness);
  
  void getValues(Fitness &fitness);
  void setValues(const QStringList &mol_names,
		 const QStringList &env_names,
		 const Fitness &fitness);
  
signals:
  void changed();
  void topologyChanged();
  void calculate();
  
protected slots:
  void apply();
  void setChanged();
  void setTopologyChanged();

  void addTarget();
  void removeTarget();
  void functionChanged(const QString &);
  void enableDebug(bool);

protected:
  QComboBox *ff_;
  QGroupBox *generic_group_;
  QTable *generic_target_;
  QPushButton *apply_;
  QTextEdit *text_;
	QCheckBox *dbg_;

  QStringList mol_names_;
  
  bool changed_;
  bool topology_changed_;
};

#endif
