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

#include <qdialog.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcombobox.h>

#include "lgv_config_dialog.h"
#include "line_graph_view.h"

LGVConfigDialog::LGVConfigDialog(QWidget *parent, LineGraphView *lgv)
  : QDialog(parent),
    lgv_(lgv)
{
  QVBoxLayout *vbox = new QVBoxLayout(this, 8);
  vbox->setAutoAdd(true);

  QHBox *hbox;

  hbox = new QHBox(this);
  QPushButton *cancel = new QPushButton("Cancel", hbox);
  /*QPushButton *apply = */ new QPushButton("Apply", hbox);
  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);

  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}
