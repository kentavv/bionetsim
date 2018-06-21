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

#ifndef _PLOT_DIALOG_H_
#define _PLOT_DIALOG_H_

#include <qdialog.h>
#include <qlist.h>
#include <qlistview.h>
#include <qstringlist.h>

#include "plot_view.h"
#include "result.h"

class QListViewItem;
class QScrollView;
class QSplitter;

// A helper class that emits a signal on a resize - required to redo layout
// of plots after a splitter resize
class ListViewHelper : public QListView {
  Q_OBJECT
public:
  ListViewHelper(QWidget *parent=0, const char *name=0, WFlags f=0)
    : QListView(parent, name, f) {}

signals:
  void resized();

protected:
  virtual void resizeEvent(QResizeEvent *e) {
    QListView::resizeEvent(e);
    emit resized();
  }
};

class PlotDialog : public QDialog {
  Q_OBJECT
public:
  PlotDialog(QWidget *parent=NULL, const char *name=NULL, bool modal=false, WFlags f=0);
  ~PlotDialog();

  virtual void clear();

  virtual void show();
  void setResults(const QValueList<Result> &results);
	void clearResults();

protected slots:
  virtual void print();
  virtual void saveSnapshot();
  virtual void copyToClipboard();
  virtual void fullScreen(PlotView *);
  void graphSelected(QListViewItem*);
  void updateGraphLayout();

protected:
  QSplitter *split_;
  QScrollView *sv_;
  PlotView *fullscreen_;
  QPtrList<PlotView> wl_;

  QListView *lv_;  

  QStringList labels_;
  QValueList<Result> results_;

  virtual void resizeEvent(QResizeEvent *);
  virtual void genPixmap(QPixmap &);
  virtual void setData(const Result &r) = 0;
  void setGraph(int);

  // Height and width of the mini-graphs used in the collage
  static const int mgw;
  static const int mgh;
};

#endif
