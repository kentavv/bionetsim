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

#include <qcombobox.h>
#include <qdialog.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qtable.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <qmessagebox.h>

#include "reaction_entry_dialog.h"
#include "rxn_parser.h"
#include "pathway_canvas_utils.h"
#include "log.h"

ReactionEntryDialog::ReactionEntryDialog(QWidget *parent)
: QDialog(parent, NULL, false, Qt::WStyle_MinMax),
changed_(false),
parser_successful_(false)
{
	QVBoxLayout *vbox = new QVBoxLayout(this,8);
	vbox->setAutoAdd(true);

	QHBox *hbox;

	//dbg_ = new QCheckBox("Enable parser debugging", this);
	//dbg_->setChecked(reactionParserDebug() > 0);

	text_ = new QTextEdit(this);
	//text_->setTabStopWidth(2);
	//text_->setFamily("Courier");
	text_->setCurrentFont(QFont("Courier", 10));
	//text_->setFont(QFont("Courier", 8));
	text_->setTextFormat(Qt::PlainText);

	hbox = new QHBox(this);
	QPushButton *cancel = new QPushButton("Cancel", hbox);
	apply_ = new QPushButton("Apply", hbox);
	QPushButton *ok = new QPushButton("OK", hbox);
	ok->setDefault(true);
	apply_->setEnabled(false);

	// setValues(mol_names, env_names, fitness);

	connect(text_, SIGNAL(textChanged()), SLOT(setChanged()));
	connect(text_, SIGNAL(returnPressed()), SLOT(apply()));

	connect(apply_, SIGNAL(clicked()), SLOT(apply()));
	connect(ok, SIGNAL(clicked()), SLOT(apply()));
	connect(ok, SIGNAL(clicked()), SLOT(accept()));
	connect(cancel, SIGNAL(clicked()), SLOT(reject()));

	//connect(dbg_, SIGNAL(toggled(bool)), SLOT(enableDebug(bool)));
}

void
ReactionEntryDialog::setItems(const QList<PathwayCanvasItem> &items) {
	QString text;
	itemsToText(items, text);
	text_->setText(text);
}

void 
ReactionEntryDialog::getItems(QList<PathwayCanvasItem> &items) {
	items = items_;
}

void
ReactionEntryDialog::apply() {
	QList<PathwayCanvasItem> tmp;
	parser_successful_ = reaction_parse(text_->text(), tmp);
	if(!parser_successful_) {
		QTextEdit *log = pneLog();
		if(log) {
			log->show();
			//log->raise();
			//qApp->processEvents();
			//setFocus();
			//raise();
		}
		return;
	}
	changed_ = true;
	items_ = tmp;

	if(changed_ && parser_successful_) emit ready();
	changed_ = false;
	apply_->setEnabled(false);
}

void
ReactionEntryDialog::setChanged() {
	changed_ = true;
	apply_->setEnabled(true);
}

void
ReactionEntryDialog::enableDebug(bool dbg) {
	setReactionParserDebug(dbg ? 1 : 0);
}

void
ReactionEntryDialog::itemsToText(const QList<PathwayCanvasItem> &items, QString &text) {
	PathwayCanvasItem *i;
	QListIterator<PathwayCanvasItem> it(items);
	//QStringList names;
	QStringList::Iterator nit;

		for(it.toFirst(); it.current(); ++it) {
			i = it.current();
			if(isSubnetwork(i)) {
			QMessageBox::warning(this, "Manual Reaction Entry", "Manual reaction entry and subnetworks are not currently compatible.\n"
				"Subnetworks have been detected in current model.\nIf changes are applied, the subnetworks will be lost.");
		break;
							}
		}

	int dnum = 0;
	for(it.toFirst(); it.current(); ++it) {
		i = it.current();
		if(isReaction(i)) {
			QString str;

			ReactionItem *p = (ReactionItem*)i;
			QString label = p->label();
			      if(label.find(' ') != -1) {
	label = QString("(%1)").arg(label);
      }
			str = label + ":  " + p->equation();
			str += QObject::tr("  [%1, %2]").arg(((MassActionReactionItem*)p)->Kf()).arg(((MassActionReactionItem*)p)->Kr());
			str += "\n";
			text += (const char*)str;
		} else if(isMolecule(i) && ((MoleculeItem*)i)->decays()) {
			QString str;

			MoleculeItem *p = (MoleculeItem*)i;
			str = QObject::tr("D%1:  ").arg(++dnum) + p->label() + QObject::tr(" --> 0 [%1]\n").arg(p->decayRate());

			text += (const char*)str;
		} else if(isSubnetwork(i)) {
			//abort();
			//return;
		}
	}

	if(!text.isEmpty()) {
	text += "\n\n";
	}

	for(it.toFirst(); it.current(); ++it) {
		i = it.current();
		if(isMolecule(i)) {
			QString str;

			MoleculeItem *p = (MoleculeItem*)i;
			QString label = p->label();
			      if(label.find(' ') != -1) {
	label = QString("(%1)").arg(label);
      }
			str = label + " = " + QObject::tr("%1").arg(p->initialConcentration()) + "\n";

			text += (const char*)str;
		}
	}


		//	names.sort();
		//	for(nit = names.begin(); nit != names.end(); ++nit) {
		//	/	text += *nit;
		//}

#if 0
		for(it.toFirst(); it.current(); ++it) {
			i = it.current();
			if(isSubnetwork(i)) {
				text += (indent(ws) + ((SubnetworkItem*)i)->label() +
					"  (" + moleculeBindings((SubnetworkItem*)i) + "):\n");
				buildReactionList(((SubnetworkItem*)i)->items(), text, ws+2);
			}
		}
#endif
	}