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

#ifndef _REACTION_ENTRY_DIALOG_H_
#define _REACTION_ENTRY_DIALOG_H_

#include <qdialog.h>
#include <qstringlist.h>

#include "fitness.h"

class QCheckBox;
class QGroupBox;
class QPushButton;
class QTable;
class QTextEdit;

class ReactionEntryDialog : public QDialog {
  Q_OBJECT
public:
  ReactionEntryDialog(QWidget *parent);

	void setItems(const QList<PathwayCanvasItem> &items);
	void getItems(QList<PathwayCanvasItem> &items);

signals:
  void ready();
  
protected slots:
  void apply();
  void setChanged();
  void enableDebug(bool);

protected:
  QPushButton *apply_;
  QTextEdit *text_;
	QCheckBox *dbg_;

	QList<PathwayCanvasItem> items_;

	bool changed_;
	bool parser_successful_;

	void itemsToText(const QList<PathwayCanvasItem> &items, QString &text);
};

#endif
