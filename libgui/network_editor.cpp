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

#include <cfloat>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qcstring.h>
#include <qcursor.h>
#include <qdatetime.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpicture.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qprogressbar.h>
#include <qscrollview.h>
#include <qsocketdevice.h>
#include <qsocketnotifier.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qvaluevector.h>
#include <qwidget.h>

#include "network_editor.h"
#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "pathway_load.h"
#include "pathway_save.h"
#include "editor.h"
#include "log.h"
#include "simulation.h"
#include "report_dialog.h"
#include "matrix.h"
#include "calculate.h"
#include "concentration_dialog.h"
#include "histogram_dialog.h"
#include "editor_params_dialog.h"
#include "environments_dialog.h"
#include "fitness_params_dialog.h"
#include "net_utils.h"
#include "simulation_params_dialog.h"
#include "dynamic_dialog.h"
#include "version.h"
#include "parameter_scan_dialog.h"
#include "parameter_scan_2d_dialog.h"
#include "slider_dialog.h"
#include "image_utils.h"
#include "reaction_entry_dialog.h"

#ifdef WIN32
#define random rand
#undef max
#endif

static const int n_menu_names = 15;
static const char *menu_names[n_menu_names] = {
	"Mass-Action Reaction",
	"---",
	"DNA Molecule",
	"mRNA Molecule",
	"Enzyme Molecule",
	"Metabolite Molecule",
	"Transcription Factor Molecule",
	"Complex Molecule",
	"---",
	"Interpolated Control Variable",
	"Square Wave Control Variable",
	"---",
	"Annotation",
	"---",
	"Subnetwork"
};

static int undo_limit = 50;

PETip::PETip(QWidget *parent)
: QToolTip(parent)
{}

void
PETip::maybeTip(const QPoint &p) {
	if(!parentWidget()->inherits("NetworkEditor"))
		return;

	QRect r;
	QString str;

	((NetworkEditor*)parentWidget())->tip(p, r, str);
	if(!r.isValid()) return;

	tip(r, str);
}

PEState::PEState()
: lb_com_(-1)
{}

PEState::PEState(PEState &pe)
: editor_(pe.editor_),
fitness_(pe.fitness_),
simulation_(pe.simulation_),
lb_com_(pe.lb_com_)
{
	items_ = copy(pe.items_);
	items_.setAutoDelete(true);
}

PEState::~PEState() {
}

PEState &
PEState::operator = (PEState &pe) {
	editor_ = pe.editor_;
	fitness_ = pe.fitness_;
	simulation_ = pe.simulation_;
	items_ = copy(pe.items_);
	items_.setAutoDelete(true);

	lb_com_ = pe.lb_com_;

	return *this;
}

NetworkEditor::NetworkEditor(QWidget *p, const char *name, WFlags f)
: QScrollView(p, name, f),
filename_(""),
version_(0),
changed_(false),
is_running_(false),
single_selection_(false),
window_selection_(false),
dragging_(false),
moving_(NULL),
edge_(NULL),
epd_(NULL),
ed_(NULL),
fpd_(NULL),
spd_(NULL),
cd_(NULL),
dd_(NULL),
hd_(NULL),
sl_(NULL),
rxned_(NULL),
sn_(NULL),
layout_was_active_(false),
remote_cpu_socket_(NULL),
remote_cpu_socket_notifier_(NULL),
remote_cpu_rep_(0),
remote_cpu_net_id_(-1)
{
	setHScrollBarMode(AlwaysOn);
	setVScrollBarMode(AlwaysOn);
	enforceBorder();  

	viewport()->setBackgroundMode(NoBackground);
	//viewport()->setBackgroundMode(FixedColor);
	//viewport()->setBackgroundColor(gray);
	setFocusPolicy(QWidget::StrongFocus);
	setFocus();
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	viewport()->setMouseTracking(true);

	insert_menu_ = new QPopupMenu(this);
	for(int i=0; i<n_menu_names; i++) {
		if(strcmp(menu_names[i], "---") == 0) {
			insert_menu_->insertSeparator();
		} else {
			insert_menu_->insertItem(menu_names[i], i);
		}
	}
	library_menu_ = new QPopupMenu(this);
	insert_menu_->insertItem("Library", library_menu_);

	connect(insert_menu_, SIGNAL(activated(int)), SLOT(insertMenu(int)));
	connect(library_menu_, SIGNAL(activated(int)), SLOT(libraryMenu(int)));
	connect(library_menu_, SIGNAL(aboutToShow()), SLOT(libraryMenuAboutToShow()));
	connect(this, SIGNAL(changed()), SLOT(setChanged()));
	connect(this, SIGNAL(parametersChanged()), SLOT(setParametersChanged()));
	connect(this, SIGNAL(topologyChanged()), SLOT(setTopologyChanged()));

	setPathwayCanvasItemsFont(font());

	state_.items_.setAutoDelete(true);

	layout_timer_ = new QTimer(this);
	connect(layout_timer_, SIGNAL(timeout()), SLOT(layoutStep()));

	pet_ = new PETip(this);

	undo_.setAutoDelete(true);
	//results_.tel.setAutoDelete(true);
}

NetworkEditor::~NetworkEditor() {
	delete pet_;
	if(sn_) delete sn_;
	remoteCPUShutdown();
	clearResults();
}

void
NetworkEditor::editUndo() {
	puts("Undo");
	popUndo();
}

void
NetworkEditor::editCut(QList<PathwayCanvasItem> &copied_items) {
	copied_items = copy(state_.items_, true);

	if(!copied_items.isEmpty()) {
		deleteSelected();
	}
}

void
NetworkEditor::editCopy(QList<PathwayCanvasItem> &copied_items) {
	copied_items = copy(state_.items_, true);
}

void
NetworkEditor::editPaste(QList<PathwayCanvasItem> &copied_items) {
	QList<PathwayCanvasItem> ci = copy(copied_items);
	if(ci.isEmpty()) return;

	unselectAll();

	PathwayCanvasItem *i;
	for(i=ci.first(); i; i=ci.next()) {
		i->setSelected(true);
		if(!isEdge(i)) {
			((MolRxnBaseItem*)i)->move(state_.editor_.gsx_, state_.editor_.gsy_);
		}
		{
			if(isMolRxn(i) || isSubnetwork(i)) {
				QString ostr = ((MolRxnBaseItem*)i)->label();
				if(!uniqueLabel(ostr)) {
					QString str;
					int tries = 0;
					do {
						str = ostr + " " + randomLabel();
						if(++tries > 128) {
							pneWarning("Unable to construct unique name while pasting: %s", (const char*)str);
							break;
						}
					} while(!uniqueLabel(str));
					((MolRxnBaseItem*)i)->setLabel(str);
				}
			}
		}
		if(isSubnetworkEdge(i)) pneDebug("Subnetwork Edge being pasted");
		state_.items_.append(i);
	}

	emit topologyChanged();
	viewport()->update();
}

void
NetworkEditor::editClear() {
	deleteSelected();
}

void
NetworkEditor::editSelectAll() {
	selectAll();
	viewport()->update();
}

void
NetworkEditor::editSelectNone() {
	unselectAll();
	viewport()->update();
}

void
NetworkEditor::editLayoutGo(bool /*s*/) {
	layoutGo(!layout_timer_->isActive());
	//layoutGo(s);
}

void
NetworkEditor::editLayoutRandom() {
	layoutRandom();
}

void
NetworkEditor::editSnapToGrid() {
	bool update = false;
	double sf = pathwayCanvasItemsScalingFactor();

	PathwayCanvasItem *i;
	for(i=state_.items_.first(); i; i=state_.items_.next()) {
		if(i->selected()) {
			if(isMovable(i)) {
				int ox = ((MolRxnBaseItem*)i)->x()*sf;
				int oy = ((MolRxnBaseItem*)i)->y()*sf;
				int nx = ox;
				int ny = oy;
				snapToNearestGridPoint(nx, ny);
				if(ox != nx || oy != ny) {
					((MolRxnBaseItem*)i)->moveTo(nx, ny);
					update = true;
				}
			}
		}
	}

	if(update) {
		emit changed();
		viewport()->update();
	}
}

void
NetworkEditor::editTrimBorder() {
	enforceBorder(true);
}

void
NetworkEditor::copyCanvas(QList<PathwayCanvasItem> &copied_items) {
	copied_items = copy(state_.items_);
}

void
NetworkEditor::replaceCanvas(QList<PathwayCanvasItem> &items) {
	state_.items_ = copy(items);

	emit topologyChanged();
	viewport()->update();
}

bool
NetworkEditor::open(const QString &filename) {
	if(!pathwayLoad((const char *)filename, state_.editor_,
		state_.fitness_, state_.simulation_,
		state_.items_, version_, viewport())) {
			return false;
	}

	changed_ = false;
	filename_ = filename;

	setCaption(filename);
	enforceBorder();

	viewport()->update();

	return true;
}

bool
NetworkEditor::fileSave() {
	if(filename_.isEmpty()) {
		return fileSaveAs();
	} else {
		if(!save()) {
			QMessageBox::critical(NULL, appname, QString("Unable to save ") + filename_);
			return false;
		}
	}
	return true;
}

bool
NetworkEditor::fileSaveAs() {
	QString fn = QFileDialog::getSaveFileName(QString::null, "Network Files (*.net);;All Files (*)", this);
	if(fn.isEmpty()) return false;

	if(QFile::exists(fn) &&
		QMessageBox::question(this, appname,
		tr("The file %1 already exists.\n"
		"Do you want to replace it?")
		.arg(fn),
		QMessageBox::Yes, (QMessageBox::No|
		QMessageBox::Default|
		QMessageBox::Escape)) ==
		QMessageBox::No) {
			return false;
	}

	if(!saveAs(fn)) {
		QMessageBox::critical(NULL, appname, QString("Unable to save as ") + fn);
		return false;
	}
	return true;
}

bool
NetworkEditor::save() {
	return saveAs(filename_);
}

bool
NetworkEditor::saveAs(const QString &filename) {
	version_++;

	if(!pathwaySave((const char *)filename, appname, appversion, state_.editor_,
		state_.fitness_, state_.simulation_, state_.items_, version_)) {
			return false;
	}

	changed_ = false;
	filename_ = filename;

	setCaption(filename);

	return true;
}

bool
NetworkEditor::saveSnapshot(const QString &filename) {
	QPixmap pix(contentsWidth(), contentsHeight());
	QPainter p(&pix);
	draw(&p);
	p.end();

	QImage img = pix.convertToImage();
	cropImageWhiteSpace(img, 2);

	return img.save(filename, "PNG", 0);
}

bool
NetworkEditor::saveSVG(const QString &filename) {
	QPicture pic;
	QPainter p(&pic);
	draw(&p);
	p.end();
	return pic.save(filename, "svg");
}

void
NetworkEditor::copyToClipboard() {
	QClipboard *cb = QApplication::clipboard();
	if(!cb) return;

	QPixmap pix(contentsWidth(), contentsHeight());
	QPainter p(&pix);
	draw(&p);
	p.end();

	QImage img = pix.convertToImage();
	cropImageWhiteSpace(img, 2);

	if(pix.convertFromImage(img)) {
		cb->setPixmap(pix, QClipboard::Clipboard);
	}
}

bool
NetworkEditor::exportAs(const QString &filename, ode_format_t format) {
	pem_.setup(state_.editor_, state_.simulation_, state_.items_);
	return gen_ode((const char*)filename, pem_, format);
}

bool
NetworkEditor::exportTimeSeries(const QString &filename) const {
	FILE *fp = fopen((const char*)filename, "w");
	if(!fp) return false;

	QValueList<Result>::const_iterator i;
	for(i = results_.begin(); i != results_.end(); ++i) {
		fprintf(fp, "%s\n", (const char*)(*i).env);
		fprintf(fp, "Time");
		for(QStringList::ConstIterator j = (*i).column_labels.begin(); j != (*i).column_labels.end(); ++j) {
			fprintf(fp, "\t%s", (const char*)*j);
		}
		fputc('\n', fp);

		(*i).m.print(fp);

		fputc('\n', fp);
		fputc('\n', fp);
	}

	fclose(fp);

	return true;
}

QString
NetworkEditor::filename() const {
	return filename_;
}

bool
NetworkEditor::hasChanged() const {
	return changed_;
}

void
NetworkEditor::setChanged() {
	//fprintf(stderr, "setChanged()\n");
	PathwayCanvasItem *item;
	for(item=state_.items_.first(); item;) {
		//printf("%p\n", item);
		if(isEdge(item) && !isCompleteEdge(item) && item != edge_) {
			fprintf(stderr, "Invalid item found\n");
			state_.items_.remove();
			fprintf(stderr, "Invalid item removed\n");
			item = state_.items_.current();
		} else {
			item = state_.items_.next();
		}
	}
	for(item=state_.items_.first(); item; item=state_.items_.next()) {
		item->updatePropertiesDialog(state_.simulation_);
	}

	changed_ = true;

	if(layout_timer_->isActive()) {
		layoutInit();
	}
}

void
NetworkEditor::setParametersChanged() {
	setChanged();
}

void
NetworkEditor::setTopologyChanged() {
	setChanged();
	if(sl_ && sl_->isVisible()) sl_->setItems(state_.items_);  
}

void
NetworkEditor::editorParametersChanged() {
	if(epd_) {
		pushUndo();

		epd_->getValues(state_.editor_);

		unselectAll();
		emit changed();
		viewport()->update();
	}
}

void
NetworkEditor::environmentsChanged() {
	if(ed_) {
		pushUndo();

		ed_->getValues(state_.simulation_);

		setAllControlVariablesEnvironment(state_.items_,
			state_.simulation_.curEnvironment(),
			state_.simulation_.numEnvironment());

		state_.fitness_.setNumEnvironment(state_.simulation_.numEnvironment());

		emit topologyChanged();
	}
}

void
NetworkEditor::fitnessParametersChanged() {
	if(fpd_) {
		pushUndo();

		fpd_->getValues(state_.fitness_);

		emit changed();
	}
}

void
NetworkEditor::simulationParametersChanged() {
	if(spd_) {
		pushUndo();

		spd_->getValues(state_.simulation_);

		emit topologyChanged();
	}
}

void
NetworkEditor::fitnessCalculate() {
	QTime qt;
	double t;
	QString str;

	emit statusMessage("Fitness calculation has begun... (press Esc to abort)", 30000);

	qt.start();

	pem_.setup(state_.editor_, state_.simulation_, state_.items_, progress_);

	double fitness;
	if(state_.fitness_.calculate(pem_, fitness)) {
		pneDebug(tr("Fitness: %1\n").arg(fitness));
		str.sprintf("Fitness calculation complete - Fitness: %f", fitness);
	} else {
		pneDebug("Failed to compute fitness\n");
		str.sprintf("Fitness calculation failed - ");
	}

	t = qt.elapsed() / 1000.0;

	QString str2;
	str2.sprintf(" Time Required: %0.3fs", t);

	str += str2;

	emit statusMessage(str, 30000);
}

void
NetworkEditor::fitnessParams() {
	if(!fpd_) {
		fpd_ = new FitnessParametersDialog(this,
			moleculeNames(state_.items_),
			state_.simulation_.environmentDescriptions(),
			state_.fitness_);
		connect(fpd_, SIGNAL(changed()), SLOT(fitnessParametersChanged()));
		connect(fpd_, SIGNAL(topologyChanged()), SLOT(fitnessParametersChanged()));
		connect(fpd_, SIGNAL(calculate()), SLOT(fitnessCalculate()));
		fpd_->show();
	} else {
		fpd_->setValues(moleculeNames(state_.items_),
			state_.simulation_.environmentDescriptions(),
			state_.fitness_);
		fpd_->show();
	}
}

void
NetworkEditor::editEditorParams() {
	if(!epd_) {
		epd_ = new EditorParametersDialog(this, state_.editor_);
		connect(epd_, SIGNAL(changed()), SLOT(editorParametersChanged()));
		connect(epd_, SIGNAL(topologyChanged()), SLOT(editorParametersChanged()));
		epd_->show();
	} else {
		epd_->setValues(state_.editor_);
		epd_->show();
	}
}

void
NetworkEditor::editEnvironments() {
	if(!ed_) {
		ed_ = new EnvironmentsDialog(this, state_.simulation_);
		connect(ed_, SIGNAL(changed()), SLOT(environmentsChanged()));
		connect(ed_, SIGNAL(topologyChanged()), SLOT(environmentsChanged()));
		ed_->show();
	} else {
		ed_->setValues(state_.simulation_);
		ed_->show();
	}
}

void
NetworkEditor::editSimulationParams() {
	if(!spd_) {
		spd_ = new SimulationParametersDialog(this, state_.simulation_);
		connect(spd_, SIGNAL(changed()), SLOT(simulationParametersChanged()));
		connect(spd_, SIGNAL(topologyChanged()), SLOT(simulationParametersChanged()));
		spd_->show();
	} else {
		spd_->setValues(state_.simulation_);
		spd_->show();
	}
}

void
NetworkEditor::displayConcentrationDialog() {
	if(state_.simulation_.simulationMethod() == 0 ||
		state_.simulation_.singleTrajectory()) {
			if(!cd_) {
				cd_ = new ConcentrationDialog(this);
				cd_->resize(560, 400);
				//connect(this, SIGNAL(topologyChanged()), cd_, SLOT(hide()));
			}

			cd_->setResults(results_);
			cd_->show();
			if(hd_) hd_->hide();
	} else {
		if(!hd_) {
			hd_ = new HistogramDialog(this);
			hd_->resize(560, 400);
			//connect(this, SIGNAL(topologyChanged()), hd_, SLOT(hide()));
		}
		hd_->setResults(results_);
		hd_->show();
		if(cd_) cd_->hide();
	}
}

void
NetworkEditor::displayDynamicDialog() {
	if(!dd_) {
		dd_ = new DynamicDialog(this);
		connect(this, SIGNAL(topologyChanged()), dd_, SLOT(hide()));
	}
	dd_->setData(results_, &state_.items_, viewport());
	dd_->show();  
}

void
NetworkEditor::displayParameterScanDialog() {
	pem_.setup(state_.editor_, state_.simulation_, state_.items_);
	ParameterScanDialog ps(this, pem_, state_.fitness_);
	ps.exec();
}

void
NetworkEditor::displayParameterScan2DDialog() {
	pem_.setup(state_.editor_, state_.simulation_, state_.items_);
	ParameterScan2DDialog ps(this, pem_, state_.fitness_);
	ps.exec();
}

#define CLIENT_SIMULATE     0x1001
#define CLIENT_ABORT        0x1002
#define CLIENT_CLOSE        0x1004

void
NetworkEditor::remoteCPUShutdown() {
	if(remote_cpu_socket_) {
		if(remote_cpu_socket_->isValid() &&
			remote_cpu_socket_->error() == QSocketDevice::NoError) {
				unsigned int command = CLIENT_CLOSE;
				int fd = remote_cpu_socket_->socket();
				send(fd, (void *)&command, sizeof(command));
		}

		delete remote_cpu_socket_;
		delete remote_cpu_socket_notifier_;

		remote_cpu_socket_ = NULL;
		remote_cpu_socket_notifier_ = NULL;
	}
}

void
NetworkEditor::remoteCPUReadyRead() {
	remote_cpu_socket_notifier_->setEnabled(false);

	//emit statusMessage("Receiving results from remote CPU...", 30000);
	Result result;
	int net_id;
	int fd = remote_cpu_socket_->socket();
	if(!receive(fd, (void*)&net_id, sizeof(net_id)) ||
		!receive(fd, result)) {
			emit statusMessage("Error while receiving results from remote CPU...", 30000);
			remoteCPUShutdown();
			return;
	}

	if(net_id != remote_cpu_net_id_) {
		emit statusMessage("Ignoring old results from remote CPU...", 30000);
		remoteCPUShutdown();
		return;
	}

	if(state_.simulation_.simulationMethod() == 0 ||
		state_.simulation_.singleTrajectory()) {
			results_.append(result);

			if(cd_ && cd_->isVisible()) displayConcentrationDialog();
			if(dd_ && dd_->isVisible()) displayDynamicDialog();
	} else {
		if(remote_cpu_rep_ == 0) {
			results_.append(result);
		} else {
			QValueList<Result>::iterator i;
			for(i = results_.begin(); i != results_.end(); ++i) {
				if((*i).env == result.env) {
					(*i).tel.append(new Matrix(result.m));
					break;
				}
			}
			if(i == results_.end()) {
				emit statusMessage("Ignoring results from unknown environment...", 30000);
				remoteCPUShutdown();
				return;
			}
		}

		remote_cpu_rep_++;

		if(remote_cpu_rep_ > 0 && state_.simulation_.histogramUpdateRate() > 0 &&
			(remote_cpu_rep_ % state_.simulation_.histogramUpdateRate()) == 0) {
				displayConcentrationDialog();
		}
		if(remote_cpu_rep_ == state_.simulation_.numReplicates()) {
			if(hd_ && hd_->isVisible()) displayConcentrationDialog();
			if(dd_ && dd_->isVisible()) displayDynamicDialog();
		}
	}

	QString str;

	if(state_.simulation_.simulationMethod() == 0 ||
		state_.simulation_.singleTrajectory() ||
		remote_cpu_rep_ >= state_.simulation_.numReplicates()) {
			str.sprintf("Remote CPU simulation complete after %0.3fs", remote_cpu_qt_.elapsed() / 1000.0);
			remoteCPUShutdown();
	} else {
		str.sprintf("Remote CPU simulation in progress - %d of %d replicates completed...",
			remote_cpu_rep_, state_.simulation_.numReplicates());
		remote_cpu_socket_notifier_->setEnabled(true);
	}

	emit statusMessage(str, 30000);
}

void
NetworkEditor::clearResults() {
	// The graphs share the same copies of the matrices as the ones that are about to be deleted.
	if(cd_) cd_->clearResults();
	if(hd_) hd_->clearResults();

	// set auto delete so the following clear will delete the matrices
	QValueList<Result>::iterator i;
	for(i = results_.begin(); i != results_.end(); ++i) {
		(*i).tel.setAutoDelete(true);
	}

	results_.clear();
}

void
NetworkEditor::runSimulation() {
	if(is_running_) {
		emit statusMessage("Simulation already running...", 30000);
		return;
	}

	QTime qt;
	QString str;

	emit statusMessage("Simulation has begun...", 3000000);

	qt.start();

	pem_.setup(state_.editor_, state_.simulation_, state_.items_, progress_);

	clearResults();

	if(!state_.simulation_.useRemoteCPU()) {
		is_running_ = true;
		bool rv = run_simulation(pem_, results_);
		QString str2;
		str2.sprintf("%0.3fs", qt.elapsed() / 1000.0);	
		str = QString("Simulation ") + (rv ? (pem_.quit ? "aborted" : "complete") : "failed") + " after " + str2;

#if 0
		if(i > 0 && state_.simulation_.histogramUpdateRate() > 0 &&
			(i % state_.simulation_.histogramUpdateRate()) == 0) {
				displayConcentrationDialog();
		}

		QString str;
		str.sprintf("Simulation in progress - %d of %d replicates completed...",
			i+1, state_.simulation_.numReplicates());

		emit statusMessage(str, 3000000);
#endif
		is_running_ = false;
	} else {
		if(remote_cpu_socket_) {
			str = "Remote CPU calculation in progress, abort before starting another";
		} else {
			remote_cpu_socket_ = new QSocketDevice;

			remote_cpu_qt_.start();

			in_addr ip;
			if(!resolve_hostname((const char*)state_.simulation_.remoteCPU(), ip)) {
				str = "Failed to resolve hostname of remote CPU " + state_.simulation_.remoteCPU();
				str.sprintf("%d", remote_cpu_socket_->error());
				remoteCPUShutdown();
				emit statusMessage(str, 30000);
				return;
			}
			if(!remote_cpu_socket_->connect(ntohl(ip.s_addr), DS_CLIENT_PORT)) {
				str = "Failed to connect to remote CPU " + state_.simulation_.remoteCPU();
				remoteCPUShutdown();
			} else {
				str = "Connected to remote CPU " + state_.simulation_.remoteCPU();

				int fd = remote_cpu_socket_->socket();

				QString remote_netformat;
				if(!receive(fd, remote_netformat)) {
					emit statusMessage("Failed to obtain remote net format", 30000);
					remoteCPUShutdown();
					return;
				}

				if(remote_netformat != netformat) {
					emit statusMessage("Remote net format incompatiable with this client's", 30000);
					remoteCPUShutdown();
					return;
				}

				QByteArray buf;
				if(!pathwaySaveBuffer(buf, appname, appversion, state_.editor_,
					state_.fitness_, state_.simulation_, state_.items_, version_)) {
						emit statusMessage("Unable to serialize network for remote CPU closed", 30000);
						remoteCPUShutdown();
						return;
				}

				const char *net_txt = (const char*)buf;
				long net_txt_len = buf.size()-1; // Subtract off the \0

				unsigned int command = CLIENT_SIMULATE;
				if(!send(fd, (void *)&command, sizeof(command)) ||
					!send(fd, (void *)&net_txt_len, sizeof(net_txt_len)) ||
					!send(fd, (void *)net_txt, net_txt_len) ||
					!receive(fd, (void *)&remote_cpu_net_id_, sizeof(remote_cpu_net_id_))) {
						emit statusMessage("Error while during initial transaction with remote CPU", 30000);
						remoteCPUShutdown();
						return;
				}

				remote_cpu_socket_notifier_ = new QSocketNotifier(remote_cpu_socket_->socket(),
					QSocketNotifier::Read, this);
				connect(remote_cpu_socket_notifier_, SIGNAL(activated(int)), SLOT(remoteCPUReadyRead()));

				remote_cpu_rep_ = 0;
			}
		}
	}

	emit statusMessage(str, 30000);

	if((cd_ && cd_->isVisible()) || (hd_ && hd_->isVisible())) displayConcentrationDialog();
	if(dd_ && dd_->isVisible()) displayDynamicDialog();
}

void
NetworkEditor::viewReport() {
	ReportDialog rd(this, NULL, true);
	rd.setup(state_.simulation_, state_.items_);  
	rd.resize(400, 400);
	rd.exec();
}

void
NetworkEditor::sliders() {
	if(!sl_) {
		sl_ = new SliderDialog(this);
		sl_->resize(450, 400);
		connect(sl_, SIGNAL(parametersChanged()), SLOT(setParametersChanged()));
	}

	sl_->setItems(state_.items_);
	sl_->show();
}

void
NetworkEditor::reactionEntry() {
	if(!rxned_) {
		rxned_ = new ReactionEntryDialog(this);
		rxned_->resize(450, 400);
		connect(rxned_, SIGNAL(ready()), SLOT(getItemsFromReactionEntryDialog()));
	}

	rxned_->setItems(state_.items_);
	rxned_->show();
}

bool
NetworkEditor::uniqueLabel(const QString &str) const {
	QListIterator<PathwayCanvasItem> i(state_.items_);
	for(; i.current(); ++i) {
		if((isMolRxn(i.current()) || isSubnetwork(i.current())) &&
			((MolRxnBaseItem*)i.current())->label() == str) {
				return false;
		}
	}
	return true;
}

QString
NetworkEditor::randomLabel() const {
	QString str;
	for(int i=0; i<4; i++) {
		char c;
		int rn = random() % 62;
		if(rn < 26) {
			c = 'A' + rn;
		} else if(rn < 52) {
			c = 'a' + (rn - 26);
		} else {
			c = '0' + (rn - 52);
		}
		str += c;
	}
	return str;
}

QString
NetworkEditor::nextItemName(const QString &base) const {
	QString str;
	int n = 0;
	if(base == "CV") {
		n = numControlVariables(state_.items_) + 1;
	} else if(base == "Mol") {
		n = numMolecules(state_.items_) + 1;
	} else if(base == "Rxn") {
		n = numReactions(state_.items_) + 1;
	} else if(base == "SN") {
		n = numSubnetworks(state_.items_) + 1;
	} else {
		pneWarning("Unknown base type passed to nextItemName: %s", (const char*)base);
	}
	int ntries;
	for(ntries=0; ntries<256; ntries++) {
		QString rn;
		if(ntries < 128) {
			rn.setNum(n + ntries);
		} else {
			rn = randomLabel();
		}
		str = base + " " + rn;
		if(uniqueLabel(str)) break;
	}
	if(ntries == 256) {
		pneWarning("Unable to construct unique item name: %s", (const char*)str);
	}
	return str;
}

QString
NetworkEditor::nextControlVariableName() const {
	return nextItemName("CV");
}

QString
NetworkEditor::nextMoleculeName() const {
	return nextItemName("Mol");
}

QString
NetworkEditor::nextReactionName() const {
	return nextItemName("Rxn");
}

QString
NetworkEditor::nextSubnetworkName() const {
	return nextItemName("SN");
}

void
NetworkEditor::viewportPaintEvent(QPaintEvent *pe) {
	QWidget* vp = viewport();
	//QPainter p(vp);
	QRect r = pe->rect();

	//printf("Updated needed: %d %d %d %d\n", r.x(), r.y(), r.width(), r.height());

	int ex2 = r.x();
	int ey2 = r.y();
	int ew2 = r.width();
	int eh2 = r.height();

	QPixmap pix(ew2, eh2);
	QPainter p2(&pix);
	p2.fillRect(0, 0, ew2, eh2, backgroundColor());

	r &= vp->rect();
	int ex = r.x();
	int ey = r.y();
	int ew = r.width();
	int eh = r.height();
	//printf("r&vp: %d %d %d %d\n", ex, ey, ew ,eh);

	//r.setX(r.x() + contentsX());
	//r.setY(r.y() + contentsY());

	r &= QRect(0, 0, contentsWidth(), contentsHeight());
	//printf("ra&p: %d %d %d %d\n", r.x(), r.y(), r.width(), r.height());
	if(r.isValid()) {
		ex = r.x();
		ey = r.y();
		ew = r.width();
		eh = r.height();

		//printf("r2&p: %d %d %d %d\n", ex, ey, ew, eh);

		p2.setClipRect(ex-ex2, ey-ey2, ew, eh);
		//printf("Clip: %d %d %d %d\n", ex, ey, ew, eh);

		p2.translate(-(ex + contentsX()), -(ey + contentsY()));

		drawContents(&p2, (ex + contentsX()), (ey + contentsY()), ew, eh);

		p2.end();
	} else {
		p2.end();
		//puts("Invalid");
	}
	bitBlt(vp, ex2, ey2, &pix);
}

void
NetworkEditor::draw(QPainter *p) {
	drawContents(p, 0, 0, contentsWidth(), contentsHeight());
}

void
NetworkEditor::drawContents(QPainter *p, int cx, int cy, int cw, int ch) {
	p->fillRect(cx, cy, cw, ch, white);

	if(state_.editor_.display_grid_) {
		int sgcx = (cx / state_.editor_.gsx_) * state_.editor_.gsx_ - 2*state_.editor_.gsx_;
		int sgcy = (cy / state_.editor_.gsy_) * state_.editor_.gsy_ - 2*state_.editor_.gsy_;
		int sgcw = (cw / state_.editor_.gsx_) * state_.editor_.gsx_ + 4*state_.editor_.gsx_;
		int sgch = (ch / state_.editor_.gsy_) * state_.editor_.gsy_ + 4*state_.editor_.gsy_;
		p->setPen(lightGray);
		for(int x=0; x<sgcw; x+=state_.editor_.gsx_) {
			p->drawLine(x+sgcx, sgcy, x+sgcx, sgcy+sgch);
		}
		for(int y=0; y<sgch; y+=state_.editor_.gsy_) {
			p->drawLine(sgcx, y+sgcy, sgcx+sgcw, y+sgcy);
		}
	}

	PathwayCanvasItem *i;
	for(i=state_.items_.first(); i; i=state_.items_.next()) {
		if(isEdge(i) || isNode(i)) {
			//i->save(stdout);
			//puts("========================");
			//fflush(NULL);
			i->draw(p);
		}
	}
	for(i=state_.items_.last(); i; i=state_.items_.prev()) {
		if(!(isEdge(i) || isNode(i))) {
			//i->save(stdout);
			//puts("========================");
			//fflush(NULL);
			i->draw(p);
		}
	}

	if(window_selection_) {
		p->setPen(QPen(black, 0, DotLine));
		p->drawRect(QRect(m_start_, m_cur_));
	}
}

void
NetworkEditor::insertItem(const QPoint &pt) {
	//printf("New %d %d\n", pt.x(), pt.y());
	MolRxnBaseItem *p = NULL;
	switch(state_.lb_com_) {
	case 0:
		p = new MassActionReactionItem(viewport());
		break;
	case 2:
		p = new DNAMoleculeItem(viewport());
		break;
	case 3:
		p = new mRNAMoleculeItem(viewport());
		break;
	case 4:
		p = new EnzymeMoleculeItem(viewport());
		break;
	case 5:
		p = new MetaboliteMoleculeItem(viewport());
		break;
	case 6:
		p = new TranscriptionFactorMoleculeItem(viewport());
		break;
	case 7:
		p = new ComplexMoleculeItem(viewport());
		break;
	case 9:
		p = new InterpolatedControlVariableItem(viewport());
		((InterpolatedControlVariableItem*)p)->setNumEnvironment(state_.simulation_.numEnvironment());
		((InterpolatedControlVariableItem*)p)->setCurEnvironment(state_.simulation_.curEnvironment());
		break;
	case 10:
		p = new SquareWaveControlVariableItem(viewport());
		if(state_.simulation_.numEnvironment() == 0) {
			pneWarning("Inserting default environment before control variable");
			state_.simulation_.setNumEnvironment(1);
		}
		((SquareWaveControlVariableItem*)p)->setNumEnvironment(state_.simulation_.numEnvironment());
		((SquareWaveControlVariableItem*)p)->setCurEnvironment(state_.simulation_.curEnvironment());
		break;
	case 12:
		p = new AnnotationItem(viewport());
		break;
	case 14:
		p = new SubnetworkItem(viewport());
		break;
	case 99:
		p = (SubnetworkItem*)sn_->clone();
		break;
	}
	if(p) {
		if(isMolecule(p)) {
			if(isControlVariable(p)) {
				p->setLabel(nextControlVariableName());
			} else {
				p->setLabel(nextMoleculeName());
			}
		} else if(isReaction(p)) {
			p->setLabel(nextReactionName());
		} else if(isAnnotation(p)) {
		} else if(isSubnetwork(p)) {
			p->setLabel(nextSubnetworkName());
		}
		if(state_.editor_.use_grid_) {
			int x = pt.x();
			int y = pt.y();
			snapToNearestGridPoint(x, y);
			p->moveTo(x, y);
		} else {
			p->moveTo(pt.x(), pt.y());
		}

		state_.items_.append(p);
		viewport()->update();

		enforceBorder();
	}
}

void
NetworkEditor::unselectAll() {
	PathwayCanvasItem *i;
	for(i=state_.items_.first(); i; i=state_.items_.next()) {
		i->setSelected(false);
	}
}

void
NetworkEditor::selectAll() {
	PathwayCanvasItem *i;
	for(i=state_.items_.first(); i; i=state_.items_.next()) {
		i->setSelected(true);
	}
}

void
NetworkEditor::contentsMousePressEvent(QMouseEvent *e) {
	if(e->pos().x() < contentsX() || contentsWidth()  <= e->pos().x() ||
		e->pos().y() < contentsY() || contentsHeight() <= e->pos().y()) {
			return;
	}

	// Ignore this mouse press event if another button is already held down
	if(e->state() & (LeftButton | RightButton | MidButton)) {
		return;
	}

	if(layout_timer_->isActive()) {
		layout_timer_->stop();
		layout_was_active_ = true;
	}

	if(!moving_ && !edge_) {
		pushUndo();
	}

	PathwayCanvasItem *i;

	if(e->button() == Qt::LeftButton) {
		if(edge_) return;
		if(state_.lb_com_ != -1) {
			insertItem(e->pos());
			emit topologyChanged();
		}
		if(state_.lb_com_ == -1) {
			bool found = false;
			for(i=state_.items_.first(); i; i=state_.items_.next()) {
				if(i->contains(e->pos())) {
					found = true;
					break;
				}
			}

			if(found) {
				//puts("found");
				if(!i->selected()) {
					if(!(e->state() & (ShiftButton | ControlButton))) unselectAll();
					//i->setSelected(true);
					emit changed();
				}	      
				m_start_ = m_cur_ = m_prev_ = e->pos();
				single_selection_ = true;
				dragging_ = true;
				return;
			} else {
				if(!(e->state() & (ShiftButton | ControlButton))) {
					unselectAll();
					emit changed();
				}	
				m_start_ = m_cur_ = m_prev_ = e->pos();
				window_selection_ = true;
				return;
			}
		}
	}
	if(e->button() == Qt::RightButton) {
		if(edge_) {
			for(i=state_.items_.first(); i; i=state_.items_.next()) {
				if(((isReactionEdge(edge_) &&
					(isMolecule(i) && ((ReactionEdgeItem*)edge_)->reaction() ||
					isReaction(i) && ((ReactionEdgeItem*)edge_)->molecule())) ||
					(isSubnetworkEdge(edge_) && isMolecule(i))) &&
					i->contains(e->pos())) {
						viewport()->unsetCursor();
						edge_->addPoint((MolRxnBaseItem*)i);
						edge_ = NULL;
						emit topologyChanged();
						break;
				}
			}

			if(!i) {
				state_.items_.append(moving_);
				moving_ = edge_->addPoint(moving_);
				moving_->moveTo(e->pos().x(), e->pos().y());
				emit changed();
			}
		} else {
			state_.lb_com_ = -1;
			viewport()->unsetCursor();
			moving_ = NULL;
			for(i=state_.items_.first(); i; i=state_.items_.next()) {
				if(i->contains(e->pos())) {
					if(isMolRxn(i) || isSubnetwork(i)) {
						viewport()->setCursor(crossCursor);
						if(isMolRxn(i)) {
							edge_ = new ReactionEdgeItem(viewport());
							moving_ = edge_->addPoint((MolRxnBaseItem*)i);
						} else if(isSubnetwork(i)) {
							edge_ = new SubnetworkEdgeItem(viewport());
							moving_ = ((SubnetworkEdgeItem*)edge_)->addPoint((SubnetworkItem*)i, e->pos());
						}
						if(moving_) {
							moving_->moveTo(e->pos().x(), e->pos().y());
							state_.items_.append(edge_);
							emit topologyChanged();
							break;
						} else {
							delete edge_;
							edge_ = NULL;
						}
					}
				}
			}

			if(!i) {
				viewport()->unsetCursor();
				insert_menu_->popup(QCursor::pos());
			}
		}
		viewport()->update();
	}
}

//----------------------------------------------------------------------

void
NetworkEditor::contentsMouseMoveEvent(QMouseEvent *e) {
	//printf("move event  %d %d %d %p\n", single_selection_, window_selection_, dragging_, edge_);

	if(edge_) {
		int x = e->pos().x();
		int y = e->pos().y();
		if(state_.editor_.use_grid_) {
			bool found = false;
			PathwayCanvasItem *i;
			for(i=state_.items_.first(); i; i=state_.items_.next()) {
				if(!isEdge(i) && !isNode(i) && i->contains(e->pos())) {
					found = true;
					break;
				}
			}
			if(!found) {
				snapToNearestGridPoint(x, y);
			}
		}
		QRect bb = moving_->boundingRect();
		int min_x = bb.width()/2;
		int min_y = bb.height()/2;
		int max_x = contentsWidth() - min_x;
		int max_y = contentsHeight() - min_y;
		x = GenoDYN::clamp(x, min_x, max_x);
		y = GenoDYN::clamp(y, min_x, max_y);

		moving_->moveTo(x, y);
		ensureVisible(x, y, 10, 10);
		viewport()->update();
	} else {
		if(e->state() & Qt::LeftButton) {
			if(window_selection_) {
				m_cur_ = e->pos();
				viewport()->update();
			} else {
				if(dragging_) {
					if(single_selection_) {
						bool found = false;
						PathwayCanvasItem *i;
						for(i=state_.items_.first(); i; i=state_.items_.next()) {
							if(!isEdge(i) && i->contains(m_start_)) {
								i->setSelected(true);
								found = true;
								break;
							}
						}
						if(!found) {
							for(i=state_.items_.first(); i; i=state_.items_.next()) {
								if(isEdge(i) && i->contains(m_start_)) {
									i->setSelected(true);
									found = true;
									break;
								}
							}
						}
						if(found) viewport()->update();
						single_selection_ = false;
					}
					m_cur_ = e->pos();

					int dx = m_cur_.x() - m_prev_.x();
					int dy = m_cur_.y() - m_prev_.y();

					if(state_.editor_.use_grid_ && !(e->state() & ControlButton)) {
						dx = (dx / state_.editor_.gsx_) * state_.editor_.gsx_;
						dy = (dy / state_.editor_.gsy_) * state_.editor_.gsy_;
					}

					if(dx != 0 || dy != 0) {
						moveSelected(dx, dy);
						m_prev_ += QPoint(dx, dy);
						ensureVisible(m_prev_.x(), m_prev_.y(), 10, 10);
					}
				} else {
				}
			}
		} else {
		}
	}
}

//----------------------------------------------------------------------

void
NetworkEditor::contentsMouseReleaseEvent(QMouseEvent *e) {
	//printf("release event  %d %d %d\n", single_selection_, window_selection_, dragging_);

	if(e->button() == Qt::LeftButton) {
		if(single_selection_) {
			if(m_start_ == e->pos()) {
				bool found = false;
				PathwayCanvasItem *i;
				for(i=state_.items_.first(); i; i=state_.items_.next()) {
					if(!isEdge(i) && i->contains(m_start_)) {
						found = true;
						break;
					}
				}
				if(!found) {
					for(i=state_.items_.first(); i; i=state_.items_.next()) {
						if(isEdge(i) && i->contains(m_start_)) {
							break;
						}
					}
				}

				if(i) {
					if(e->state() & (ShiftButton | ControlButton)) {
						i->setSelected(!i->selected());
					} else {
						unselectAll();
						i->setSelected(true);
					}
					viewport()->update();
				}
			}
		} else if(window_selection_) {
			double sf = pathwayCanvasItemsScalingFactor();
			m_cur_ = e->pos();
			QRect win(m_start_/sf, m_cur_/sf);
			win = win.normalize();
			PathwayCanvasItem *i;
			for(i=state_.items_.first(); i; i=state_.items_.next()) {
				if(win.contains(i->boundingRect())) {
					i->setSelected(true);
				}
			}
			viewport()->update();      
		}
		single_selection_ = false;
		window_selection_ = false;
		dragging_ = false;
	}

	if(!edge_) {
		moving_ = NULL;
	}

	if(layout_was_active_) {
		layout_was_active_ = false;
		layoutInit();
		layout_timer_->start(100);
	}  
}

void
NetworkEditor::contentsMouseDoubleClickEvent(QMouseEvent *e) {
	//puts("double click event");

	if(e->button() == Qt::LeftButton) {
		if(!edge_) {
			PathwayCanvasItem *i;
			for(i=state_.items_.first(); i; i=state_.items_.next()) {
				if(i->contains(e->pos())) {
					if(!isEdge(i)) {
						QWidget *pd = i->propertiesDialog(this, state_.simulation_);
						if(pd) {
							connect(pd, SIGNAL(changed()), SIGNAL(changed()));
							connect(pd, SIGNAL(topologyChanged()), SIGNAL(topologyChanged()));
						}
						return;
					}
				}
			}
			for(i=state_.items_.first(); i; i=state_.items_.next()) {
				if(i->contains(e->pos())) {
					if(isEdge(i)) {
						pushUndo();

						InternalNodeItem *p = new InternalNodeItem(viewport());
						int x = e->pos().x();
						int y = e->pos().y();
						if(state_.editor_.use_grid_) {
							snapToNearestGridPoint(x, y);
						}
						p->moveTo(x, y);
						((ReactionEdgeItem*)i)->insertPoint(p);
						state_.items_.append(p);
						unselectAll();
						p->setSelected(true);
						viewport()->update();
						emit changed();

						return;
					}
				}
			}
		}
	}
}

void
NetworkEditor::closeEvent(QCloseEvent *e) {
	if(hasChanged()) {
		setFocus();
		showMaximized();
		switch(QMessageBox::warning(this, appname,
			"Current pathway is unsaved.\n"
			"Do you wish to save before quiting?\n",
			"Yes", "No", "Cancel", 0, 2)) {
		case 0:
			if(fileSave()) e->accept();
			else e->ignore();
			break;
		case 1:
			e->accept();
			break;
		case 2:
			e->ignore();
			break;
		}
	} else {
		e->accept();
	}
}

void
NetworkEditor::insertMenu(int id) {
	viewport()->setCursor(pointingHandCursor);
	state_.lb_com_ = id;
	emit statusMessage(tr("Placing %1").arg(menu_names[id]), 5000);
}

void
NetworkEditor::libraryMenu(int id) {
	if(sn_) delete sn_;
	sn_ = NULL;
	QString fn = library_menu_->text(id);
	if(fn != QString::null) {
		//printf("Loading subnet %s\n", (const char*)fn);
		QList<PathwayCanvasItem> items;
		if(pathwayLoad(fn, items)) {
			sn_ = new SubnetworkItem(viewport());
			sn_->setItems(items);
			viewport()->setCursor(pointingHandCursor);
			state_.lb_com_ = 99;
			emit statusMessage(tr("Placing subnetwork %1").arg(fn), 5000);
		} else {
			QMessageBox::warning(this, appname,
				tr("Unable to import subnetwork %1.\n").arg(fn),
				"Ok", 0);
		}
	}
}

void
NetworkEditor::libraryMenuAboutToShow() {
	library_menu_->clear();

	QDir d("networks/subnetwork_library");
	d.setFilter(QDir::Files);
	d.setNameFilter("*.net");
	const QFileInfoList *list = d.entryInfoList();
	if(list) {
		QFileInfoListIterator it(*list);
		QFileInfo *fi;
		while((fi = it.current())) {
			library_menu_->insertItem(fi->filePath());
			++it;
		}
	}
}

void
NetworkEditor::deleteEdge(EdgeBaseItem *edge) {
	QList<MolRxnBaseItem> nodes = edge->nodes();
	QListIterator<MolRxnBaseItem> i(nodes);
	for(; i.current();) {
		MolRxnBaseItem *p = (MolRxnBaseItem*)i.current();
		if(isNode(p)) {
			edge->removeNode(p);
			nodes.remove(p);
			state_.items_.remove(p);
		} else {
			++i;
		}
	}
	state_.items_.remove(edge);
}

void
NetworkEditor::deleteSelected() {
	bool push_done = false;

	QListIterator<PathwayCanvasItem> i(state_.items_), j(state_.items_);

	// Remove selected internal nodes, molecules, and rxns
	for(; i.current();) {
		if(i.current()->selected() && !isEdge(i)) {
			if(!push_done) {
				pushUndo();
				push_done = true;
			}
			// Remove selected node from any edges
			j.toFirst();
			for(; j.current(); ++j) {
				if(isEdge(j)) {
					((EdgeBaseItem*)j.current())->removeNode((MolRxnBaseItem*)(i.current()));
				}
			}
			// Actually delete selected node
			state_.items_.remove(i.current());
		} else {
			++i;
		}
	}

	// Remove selected and invalid edges (includes current edge_)
	i.toFirst();
	for(; i.current();) {
		PathwayCanvasItem *p = i.current();
		if(isEdge(p) && (p->selected() || !isCompleteEdge(p))) {
			if(!push_done) {
				pushUndo();
				push_done = true;
			}
			deleteEdge((EdgeBaseItem*)p);
		} else {
			++i;
		}
	}
	if(edge_) viewport()->unsetCursor();
	edge_ = NULL;
	moving_ = NULL;
	emit topologyChanged();
	viewport()->update();
}

#define UP    0x01
#define DOWN  0x02
#define LEFT  0x04
#define RIGHT 0x08

void
NetworkEditor::keyPressEvent(QKeyEvent *e) {
	switch(e->key()) {
	case Key_Up:
		if(e->state() & (ShiftButton | ControlButton)) {
			moveSelected(UP, bool(e->state() & ShiftButton));
		} else {
			scrollBy(0, -visibleHeight()/5);
		}
		break;
	case Key_Down:
		if(e->state() & (ShiftButton | ControlButton)) {
			moveSelected(DOWN, bool(e->state() & ShiftButton));
		} else {
			scrollBy(0, visibleHeight()/5);
		}
		break;
	case Key_Left:
		if(e->state() & (ShiftButton | ControlButton)) {
			moveSelected(LEFT, bool(e->state() & ShiftButton));
		} else {
			scrollBy(-visibleWidth()/5, 0);
		}
		break;
	case Key_Right:
		if(e->state() & (ShiftButton | ControlButton)) {
			moveSelected(RIGHT, bool(e->state() & ShiftButton));
		} else {
			scrollBy(visibleWidth()/5, 0);
		}
		break;
	case Key_PageUp:
		scrollBy(0, -visibleHeight());
		break;
	case Key_PageDown:
		scrollBy(0, visibleHeight());
		break;
	case Key_Escape:
		viewport()->unsetCursor();
		state_.lb_com_ = -1;
		if(edge_) {
			deleteEdge(edge_);
			edge_ = NULL;
			viewport()->update();
			emit topologyChanged();
		}
		pem_.quit = true;
		if(remote_cpu_socket_) {
			emit statusMessage("Aborting remote CPU simulation...", 30000);
			remoteCPUShutdown();
		}
		break;
	case Key_Delete:
		deleteSelected();
		break;
	case Key_Plus:
		if(e->state() & ControlButton) {
			double sf = pathwayCanvasItemsScalingFactor();
			setPathwayCanvasItemsScalingFactor(sf + .1);
			enforceBorder();
			emit changed();
			viewport()->update();
		}
		break;
	case Key_Minus:
		if(e->state() & ControlButton) {
			double sf = pathwayCanvasItemsScalingFactor();
			if(sf > .1) {
				setPathwayCanvasItemsScalingFactor(GenoDYN::max(.1, sf - .1));
			}
			enforceBorder();
			emit changed();
			viewport()->update();
		}
		break;
	default:
		e->ignore();
	}  
}

void
NetworkEditor::tip(QPoint p, QRect &r, QString &str) const {
	double sf = pathwayCanvasItemsScalingFactor();
	p += QPoint(contentsX(), contentsY());

	QListIterator<PathwayCanvasItem> i(state_.items_);
	for(; i.current(); ++i) {
		if(isMolRxn(i) && i.current()->contains(p)) {
			MolRxnBaseItem *mr = (MolRxnBaseItem*)i.current();
			r = mr->boundingRect();
			r = QRect(r.topLeft()*sf, r.bottomRight()*sf);
			r.moveBy(-contentsX(), -contentsY());
			str = mr->tip();
			break;
		}
	}
}

void
NetworkEditor::pushUndo() {
	printf("Pushing onto %d undos...\n", undo_.count());
	undo_.append(new PEState(state_));
	if((int)undo_.count() > undo_limit) {
		undo_.removeFirst();
	}
}

void
NetworkEditor::popUndo() {
	//printf("Popping from %d undos...\n", undo_.count());
	if(!undo_.isEmpty()) {
		state_ = *undo_.last();
		undo_.prev();
		undo_.removeLast();
		if(edge_) viewport()->unsetCursor();
		edge_ = NULL;
		moving_ = NULL;
		emit topologyChanged();
		viewport()->update();
	}
}

void
NetworkEditor::moveSelected(int dir, bool big_step) {
	pushUndo();

	int dx=1, dy=1;

	if(big_step) {
		dx = state_.editor_.gsx_;
		dy = state_.editor_.gsy_;
	}

	PathwayCanvasItem *i;
	for(i=state_.items_.first(); i; i=state_.items_.next()) {
		if(i->selected() && isMovable(i)) {
			MolRxnBaseItem *p = (MolRxnBaseItem*)i;
			if(dir & UP)    p->move(0, -dy);
			if(dir & DOWN)  p->move(0, +dy);
			if(dir & LEFT)  p->move(-dx, 0);
			if(dir & RIGHT) p->move(+dx, 0);
		}
	}
	enforceBorder();
	viewport()->update();
}

void
NetworkEditor::moveSelected(int dx, int dy) {
	PathwayCanvasItem *i;
	for(i=state_.items_.first(); i; i=state_.items_.next()) {
		if(i->selected() && isMovable(i)) {
			MolRxnBaseItem *p = (MolRxnBaseItem*)i;
			p->move(dx, dy);
		}
	}
	enforceBorder();
	viewport()->update();
}

void
NetworkEditor::layoutGo(bool s) {
	if(s && !layout_timer_->isActive()) {
		layoutInit();
		layout_timer_->start(100);
	} else {
		layout_timer_->stop();
	} 
}

void
NetworkEditor::layoutRandom() {
	int w = contentsWidth();
	int h = contentsHeight();
	PathwayCanvasItem *i;
	for(i=state_.items_.first(); i; i=state_.items_.next()) {
		if(isMovable(i) && !isAnnotation(i)) {
			MolRxnBaseItem *p = (MolRxnBaseItem*)i;
			int xi = random()%(w-20);
			int yi = random()%(h-15);
			if(state_.editor_.use_grid_) snapToNearestGridPoint(xi, yi);
			p->moveTo(xi, yi);
		}
	}
	if(layout_timer_->isActive()) layoutInit();
	viewport()->update();
}

void
NetworkEditor::layoutInit() {
	int ni = 0, j, k;
	PathwayCanvasItem *i;
	for(i=state_.items_.first(); i; i=state_.items_.next()) {
		if(isMovable(i) && !isAnnotation(i)) {
			i->setId(ni++);
		}
	}

	x_.resize(ni),  y_.resize(ni);
	dx_.resize(ni),  dy_.resize(ni);
	ddx_.resize(ni), ddy_.resize(ni);
	for(j=0; j<ni; j++) {
		dx_[j] = 0.0;
		dy_[j] = 0.0;
	}

	double sf = pathwayCanvasItemsScalingFactor();
	int w = contentsWidth()/sf;
	int h = contentsHeight()/sf;
	for(i=state_.items_.first(), j=0; i; i=state_.items_.next()) {
		if(isMovable(i) && !isAnnotation(i)) {
			MolRxnBaseItem *p = (MolRxnBaseItem*)i;
			x_[j] = p->x()/(double)w;
			y_[j] = p->y()/(double)h;
			j++;
		}
	}

	em_.resize(ni * ni);
	for(j=0; j<ni-1; j++) {
		for(k=j+1; k<ni; k++) {
			em_[j*ni + k] = -1;
		}
	}

	int ne = 0;
	for(i=state_.items_.first(), j=0; i; i=state_.items_.next()) {
		if(isEdge(i)) {
			QList<MolRxnBaseItem> nodes = ((EdgeBaseItem*)i)->nodes();

			PathwayCanvasItem *p, *q;
			for(p=nodes.first(), q=nodes.next(); q; q=nodes.next()) {
				if(p->id() <= q->id()) {
					em_[p->id() * ni + q->id()] = ne++;
				} else {
					em_[q->id() * ni + p->id()] = ne++;
				}
			}
		}
	}
}

void
NetworkEditor::layoutStep() {
	int w = contentsWidth();
	int h = contentsHeight();
	int i, j;

	const double cc = 0.0001;
	const double dampening = 0.85;
	const double nsl = 100.0 / (w > h ? w : h); // normalized spring length
	const double marginX = 20.0 / w; 
	const double marginY = 15.0 / h;  

	double rv_ = 0.25;
	double hc_ = 0.11;

	int n_ = x_.count();

	for(i=0; i<n_; i++) {
		ddx_[i] = 0.0;
		ddy_[i] = 0.0;
	}

	for(i=0; i<n_-1; i++) {
		for(j=i+1; j<n_; j++) {
			double dx = x_[i] - x_[j];
			double dx1 = dx;
			double dx2 = x_[i] + 1.0 - x_[j];
			double dx3 = -(1.0 - x_[i]) - x_[j];
			if(fabs(dx) > fabs(dx2)) dx = dx2;
			if(fabs(dx) > fabs(dx3)) dx = dx3;

			double dy = y_[i] - y_[j];
			double dy1 = dy;
			double dy2 = y_[i] + 1.0 - y_[j];
			double dy3 = -(1.0 - y_[i]) - y_[j];
			if(fabs(dy) > fabs(dy2)) dy = dy2;
			if(fabs(dy) > fabs(dy3)) dy = dy3;

			double ds = dx*dx + dy*dy;
			if(ds < 1e-6) ds = 1e-6;

			double fm_r = rv_ * cc / ds;

			double fm_s;
			if(em_[i * n_ + j] != -1) {
				double ds_s = dx1*dx1 + dy1*dy1;
				if(ds_s < 1e-6) ds_s = 1e-6;
				double dt = sqrt(ds_s);
				fm_s = hc_ * (nsl - dt);
			} else {
				fm_s = 0.0;
			}

			double dcx = dx * fm_r + dx1 * fm_s;
			ddx_[i] += dcx;
			ddx_[j] -= dcx;
			double dcy = dy * fm_r + dy1 * fm_s;
			ddy_[i] += dcy;
			ddy_[j] -= dcy;
		}
	}

	for(i=0; i<n_; i++) {
		dx_[i] = dx_[i] * dampening + ddx_[i];
		dy_[i] = dy_[i] * dampening + ddy_[i];

		if(dx_[i] > .125) dx_[i] = .125;
		else if(dx_[i] < -.125) dx_[i] = -.125;
		if(dy_[i] > .125) dy_[i] = .125;
		else if(dy_[i] < -.125) dy_[i] = -.125;

		x_[i] = x_[i] + dx_[i];
		if(x_[i] < 0.0) {
			x_[i] = 0.0;
			dx_[i] = fabs(dx_[i]);
		} else if(x_[i] > 1.0 - marginX) {
			x_[i] = 1.0 - marginX;
			dx_[i] = -fabs(dx_[i]);
		}
		y_[i] = y_[i] + dy_[i];
		if(y_[i] < 0.0) {
			y_[i] = 0.0;
			dy_[i] = fabs(dy_[i]);
		} else if(y_[i] > 1.0 - marginY) {
			y_[i] = 1.0 - marginY;
			dy_[i] = -fabs(dy_[i]);
		}
	}

	{
		int j;
		PathwayCanvasItem *i;
		for(i=state_.items_.first(), j=0; i; i=state_.items_.next()) {
			if(isMovable(i) && !isAnnotation(i)) {
				MolRxnBaseItem *p = (MolRxnBaseItem*)i;
				p->moveTo(int(x_[j]*w+0.5), int(y_[j]*h+0.5));
				j++;
			}
		}
	}

#if 1
	viewport()->update();
	//updateContents(lx1_, ly1_, lx2_ - lx1_ + 1, ly2_ - ly1_ + 1);
	//viewport()->update(lx1_, ly1_, lx2_ - lx1_ + 1, ly2_ - ly1_ + 1);
#else
	{
		static int count = 0;
		if((count % 10) == 0) {
			canvas()->update();
		}
		count++;
		printf("%d\n", count);
	}
#endif
	//static int count = 0;
	//count++;
	//printf("%d\n", count);
}

void
NetworkEditor::setProgressBar(QProgressBar *pb) {
	progress_ = pb;
}

Simulation
NetworkEditor::simulation() const {
	return state_.simulation_;
}

void
NetworkEditor::setCurEnvironment(int cur_env) {
	pushUndo();
	state_.simulation_.setCurEnvironment(cur_env);
	setAllControlVariablesEnvironment(state_.items_, cur_env);  
	emit changed();
}

void
NetworkEditor::enforceBorder(bool trim_top_left) {
	// Required border size
	int bs = 10;
	double sf = pathwayCanvasItemsScalingFactor();

	// Find bounding box of all items
	QRect bb;
	QListIterator<PathwayCanvasItem> i(state_.items_);
	for(; i.current(); ++i) {
		//QRect bb2 = i.current()->boundingRect();
		//printf("B:%d %d %d %d\n", bb2.x(), bb2.y(), bb2.width(), bb2.height());
		if(bb.isValid()) {
			//puts("2");
			bb |= i.current()->boundingRect();
		} else {
			//puts("1");
			bb = i.current()->boundingRect();
		}
	}
	//printf("bb: %d %d %d %d\n", bb.x(), bb.y(), bb.width(), bb.height());

	bb.setWidth(bb.width()*sf);
	bb.setHeight(bb.height()*sf);

	bb.setWidth(bb.width() + 2*bs);
	bb.setHeight(bb.height() + 2*bs);
	if(!trim_top_left) {
		bb.setWidth(bb.width() + bb.x()-bs);
		bb.setHeight(bb.height() + bb.y()-bs);
	}

	if(bb.width() < viewport()->width()) {
		bb.setWidth(viewport()->width());
	}
	if(bb.height() < viewport()->height()) {
		bb.setHeight(viewport()->height());
	}
	//printf("bb2: %d %d %d %d\n", bb.x(), bb.y(), bb.width(), bb.height());

	// Expand canvas to encompass all items and have a border of size 'border'
	resizeContents(bb.width(), bb.height());

	//emit changed();

	// Center items on canvas
	int dx = bs - bb.x();
	int dy = bs - bb.y();
	//printf("dx:%d dy:%d\n", dx, dy);

	if(!trim_top_left) {
		if(dx < 0) dx = 0;
		if(dy < 0) dy = 0;
	}

	if(dx != 0 || dy != 0) {
		for(i.toFirst(); i.current(); ++i) {
			if(isMovable(i.current())) {
				((MolRxnBaseItem*)i.current())->move(dx, dy);
			}
		}
	}

	viewport()->update();
}

void
NetworkEditor::viewportResizeEvent(QResizeEvent *e) {
	QScrollView::viewportResizeEvent(e);
	enforceBorder();
}

void
NetworkEditor::snapToNearestGridPoint(int &x, int &y) {
	x = ((x+state_.editor_.gsx_/2) / state_.editor_.gsx_) * state_.editor_.gsx_;
	y = ((y+state_.editor_.gsy_/2) / state_.editor_.gsy_) * state_.editor_.gsy_;
}

void
NetworkEditor::getItemsFromReactionEntryDialog() {
	if(!rxned_) return;
	rxned_->getItems(state_.items_);
	state_.items_.setAutoDelete(true);

	{
		PathwayCanvasItem *i;
		QListIterator<PathwayCanvasItem> it(state_.items_);

		for(it.toFirst(); it.current(); ++it) {
			i = it.current();
			i->setParent(this);
		}
	}

	layoutRandom();
}
