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
#include <qstring.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qhbox.h>

#include "textviewer.h"

TextViewer::TextViewer(QWidget *parent, const char *name, bool modal)
  : QDialog(parent, name, modal, Qt::WStyle_MinMax)
{
  QVBoxLayout *vbox = new QVBoxLayout(this, 8);
  vbox->setAutoAdd(true);
  
  setCaption("Text Viewer");

  text_ = new QTextEdit(this);
  text_->setMinimumSize(400, 300);
  text_->setReadOnly(true);
  text_->setWordWrap(QTextEdit::NoWrap);
  
  QHBox *hbox = new QHBox(this);
  QPushButton *save = new QPushButton("Save", hbox);
  connect(save, SIGNAL(clicked()), SLOT(save()));

  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
}

void
TextViewer::setText(const QString &text) {
  text_->setText(text);
}

void
TextViewer::clear() {
  text_->clear();
}

void
TextViewer::save() {
  QString s(QFileDialog::getSaveFileName(QString::null, "Text Files (*.txt);;All Files (*)", NULL));
  if(!s.isEmpty()) {
    FILE *file = fopen((const char *)s, "w");
    if(!file) {
      QMessageBox::warning(NULL, "Text Viewer", "Unable to open file " + s);
      return;
    }
    fprintf(file, "%s\n", (const char*)text_->text());
    fclose(file);
  }
}
