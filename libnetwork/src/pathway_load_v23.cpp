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

#include <stdio.h>

#include <qdom.h>

#include "log.h"
#include "pathway_load_v23.h"
#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "editor.h"
#include "fitness.h"
#include "simulation.h"

/*
  Would be nice to use id and idref along with elementById() instead of
  <Id>...</Id>, but Qt does not current support Id lookups.  The method
  is there, but without a means to tell the QDomClasses which attributes
  are element Ids, the method always returns the null element. (see
  QDomDocument::elementById() for information) - v3.3.0
*/

static bool parseEditor(QDomNode &editor_node, Editor &editor);
static bool parseGenericFitnessTarget(QDomNode &generic_target_node, Fitness &fitness);
static bool parseGenericFitness(QDomNode &generic_node, Fitness &fitness);
static bool parseFitness(QDomNode &editor_node, Fitness &fitness);
static bool parseSimulation(QDomNode &simulation_node, Simulation &simulation);
static bool parseEnvironments(QDomNode &environments_node, Simulation &simulation);
static bool parseMolRxnBase(QDomElement &e, MolRxnBaseItem *molrxn);
static bool parseMoleculeBase(QDomElement &e, MoleculeItem *mol);
static bool parseAnnotationItem(QDomNode &ann_node, AnnotationItem *ann);
static bool parseMatrix(QDomNode &matrix_node, Matrix &m);
static bool parseInterpolatedControlVariableValues(QDomNode &values_node, InterpolatedControlVariableItem *cv);
static bool parseInterpolatedControlVariableParameters(QDomNode &param_node, InterpolatedControlVariableItem *cv);
static bool parseInterpolatedControlVariableItem(QDomNode &cv_node, InterpolatedControlVariableItem *cv, int ce, int ne);
static bool parseSquareWaveControlVariableParameters(QDomNode &param_node, SquareWaveControlVariableItem *cv);
static bool parseSquareWaveControlVariableItem(QDomNode &cv_node, SquareWaveControlVariableItem *cv, int ce, int ne);
static bool parseMoleculeItem(QDomNode &mol_node, MoleculeItem *mol);
static bool parseMolecules(QDomNode &molecules_node, QList<PathwayCanvasItem> &items, int ce, int ne);
static bool parseReactionEdge(QDomNode &edge_node, QList<PathwayCanvasItem> &items, MassActionReactionItem *rxn, ReactionEdgeItem *edge);
static bool parseReactionEdges(QDomNode &edges_node, QList<PathwayCanvasItem> &items, MassActionReactionItem *rxn);
static bool parseMassActionReactionItem(QDomNode &rxn_node, QList<PathwayCanvasItem> &items, MassActionReactionItem *rxn);
static bool parseReactions(QDomNode &reactions_node, QList<PathwayCanvasItem> &items);
static bool parseNetwork(QDomNode &net_node, QList<PathwayCanvasItem> &items, int ce, int ne);
static bool parseSubnetworkEdge(QDomNode &edge_node, QList<PathwayCanvasItem> &items, SubnetworkItem *sn, SubnetworkEdgeItem *edge);
static bool parseSubnetworkEdges(QDomNode &edges_node, QList<PathwayCanvasItem> &items, SubnetworkItem *sn);
static bool parseSubnetworkItem(QDomNode &sn_node, QList<PathwayCanvasItem> &items, SubnetworkItem *sn, int ce, int ne);
static bool parseSubnetworks(QDomNode &subnetworks_node, QList<PathwayCanvasItem> &items, int ce, int ne);

static bool
parseInfo(QDomNode &info_node, int &version) {
  QDomNode n = info_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "Program") {
      } else if(e.tagName() == "Format") {
      } else if(e.tagName() == "SaveDateTime") {
      } else if(e.tagName() == "Version") {
	version = e.text().toInt();
      } else {
	pneDebug("Ignoring unknown Info tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseEditor(QDomNode &editor_node, Editor &editor) {
  QDomNode n = editor_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "GridSpacing") {
	QString str = e.text().simplifyWhiteSpace();
	editor.gsx_ = str.section(' ', 0, 0).toInt();
	editor.gsy_ = str.section(' ', 1, 1).toInt();
      } else if(e.tagName() == "SnapToGrid") {
	editor.use_grid_ = true;
      } else if(e.tagName() == "Grid") {
	editor.display_grid_ = (e.attribute("display", "false") == "true");
      } else if(e.tagName() == "Icons") {
      } else if(e.tagName() == "Labels") {
      } else if(e.tagName() == "UpdateSimulationAutomatically") {
      } else {
	pneDebug("Ignoring unknown Editor tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseGenericFitnessTarget(QDomNode &generic_target_node, Fitness &fitness) {
  QDomNode n = generic_target_node.firstChild();
  QString name;
  QStringList values;
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "Molecule") {
	name = e.text();
      } else if(e.tagName() == "Values") {
	values = QStringList::split(' ', e.text().simplifyWhiteSpace());
      } else {
	pneDebug("Ignoring unknown GenericFitnessTarget tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  if(!name.isEmpty() && !values.isEmpty()) {
    QStringList target_names;
    Matrix target_values;
    fitness.genericTarget(target_names, target_values);
    int nr = target_values.nrows();
    int nc = target_values.ncolumns();

    if(target_values.empty()) {
      nc = values.count();
    } else if(nc != (int)values.count()) {
      pneDebug("Incorrect number of target values in generic target - target ignored");
      return false;
    }

    target_names.append(name);
    Matrix new_values(nr+1, nc);

    int i, j;
    
    for(i=0; i<nr; i++) {
      for(j=0; j<nc; j++) {
	new_values(i, j) = target_values(i, j);
      }
    }

    for(i=0; i<nc; i++) {
      new_values(nr, i) = values[i].toDouble();
    }

    fitness.setGeneric(target_names, new_values);
  }
  return true;
}

static bool
parseGenericFitness(QDomNode &generic_node, Fitness &fitness) {
  QDomNode n = generic_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "Target") {
	if(!parseGenericFitnessTarget(n, fitness)) return false;
      } else {
	pneDebug("Ignoring unknown GenericFitness tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseFitness(QDomNode &fitness_node, Fitness &fitness) {
  QDomNode n = fitness_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "Function") {
	fitness.setFunction(e.text());
      } else if(e.tagName() == "Generic") {
	if(!parseGenericFitness(n, fitness)) return false;
      } else {
	pneDebug("Ignoring unknown Fitness tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseSimulation(QDomNode &simulation_node, Simulation &simulation) {
  QDomNode n = simulation_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "SimulationMethod") {
	simulation.setSimulationMethod(e.text());
      } else if(e.tagName() == "TimeBegin") {
	simulation.setTimeBegin(e.text().toDouble());
      } else if(e.tagName() == "TimeEnd") {
	simulation.setTimeEnd(e.text().toDouble());
      } else if(e.tagName() == "TimeStepSize") {
	simulation.setTimeStepSize(e.text().toDouble());
      } else if(e.tagName() == "SimulateToSteadyState") {
	simulation.setSimulateToSteadyState(true);
      } else if(e.tagName() == "SteadyStateTolerance") {
	simulation.setSteadyStateTolerance(e.text().toDouble());
      } else if(e.tagName() == "AbsoluteTolerance") {
	simulation.setAbsoluteTolerance(e.text().toDouble());
      } else if(e.tagName() == "RelativeTolerance") {
	simulation.setRelativeTolerance(e.text().toDouble());
      } else if(e.tagName() == "MaxStep") {
	simulation.setMaxStep(e.text().toDouble());
      } else if(e.tagName() == "MinStep") {
	simulation.setMinStep(e.text().toDouble());
      } else if(e.tagName() == "NumReplicates") {
	simulation.setNumReplicates(e.text().toInt());
      } else if(e.tagName() == "HistogramUpdateRate") {
	simulation.setHistogramUpdateRate(e.text().toInt());
      } else if(e.tagName() == "Environments") {
	parseEnvironments(n, simulation);
      } else if(e.tagName() == "SingleTrajectory") {
	simulation.setSingleTrajectory(true);
      } else if(e.tagName() == "UseRemoteCPU") {
	simulation.setUseRemoteCPU(true);
      } else if(e.tagName() == "RemoteCPU") {
	simulation.setRemoteCPU(e.text());
      } else {
	pneDebug("Ignoring unknown Simulation tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseEnvironments(QDomNode &environments_node, Simulation &simulation) {
  QDomNode n = environments_node.firstChild();
  QStringList envs;
  QString cur;
  
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();
      
      if(e.tagName() == "Environment") {
	envs += e.text();
	if(e.attribute("current") == "true") {
	  cur = e.text();
	}
      } else {
	pneDebug("Ignoring unknown Environments tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }

  simulation.setEnvironmentDescriptions(envs);
  simulation.setCurEnvironment(cur);
  
  return true;
}

static bool
parseMolRxnBase(QDomElement &e, MolRxnBaseItem *molrxn) {
  if(e.tagName() == "Label") {
    molrxn->setLabel(e.text());
  } else if(e.tagName() == "Id") {
    molrxn->setId(e.text().toInt());
  } else if(e.tagName() == "Pos") {
    QString str = e.text().simplifyWhiteSpace();
    int x = str.section(' ', 0, 0).toInt();
    int y = str.section(' ', 1, 1).toInt();
    molrxn->move(x, y);
  } else if(e.tagName() == "Notes") {
    molrxn->setNotes(e.text());
  } else if(e.tagName() == "NotInitialized") {
    molrxn->setInitialized(false);
  } else {
    return false;
  }
  return true;
}

static bool
parseMoleculeBase(QDomElement &e, MoleculeItem *mol) {
  if(e.tagName() == "DefaultCoefficient") {
  } else if(e.tagName() == "Exported") {
    mol->setExported(true);
  } else if(e.tagName() == "NotPlotted") {
    mol->setPlotted(false);
  } else {
    return false;
  }
  return true;
}

static bool
parseAnnotationItem(QDomNode &ann_node, AnnotationItem *ann) {
  ann->setInitialized(true);
  QDomNode n = ann_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(parseMolRxnBase(e, ann)) {
      } else {
	pneDebug("Ignoring unknown Annotation tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseMatrix(QDomNode &matrix_node, Matrix &m) {
  QDomElement e = matrix_node.toElement();
  int nr = e.attribute("nrows", "0").toInt();
  int nc = e.attribute("ncolumns", "0").toInt();
  m.resize(nr, nc);
  if(m.empty()) return true;

  int r=0, c=0;
  
  QDomNode n = matrix_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "r") {
	QDomNode n2 = n.firstChild();
	while(!n2.isNull()) {
	  if(n2.isElement()) {
	    QDomElement e2 = n2.toElement();
	    
	    if(e2.tagName() == "c") {
	      m(r, c) = e2.text().toDouble();
	      c++;
	    } else {
	      pneDebug("Ignoring unknown Matrix tag: " + e2.tagName());
	    }
	  }
	  n2 = n2.nextSibling();
	}
	c=0; r++;
      } else {
	pneDebug("Ignoring unknown Matrix tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseInterpolatedControlVariableValues(QDomNode &values_node, InterpolatedControlVariableItem *cv) {
  QDomNode n = values_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();
      if(e.tagName() == "Matrix") {
	Matrix values;
	if(parseMatrix(n, values)) {
	  cv->setValues(values);
	}
      } else {
	pneDebug("Ignoring unknown InterpolatedControlVariable Values tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseInterpolatedControlVariableParameters(QDomNode &param_node, InterpolatedControlVariableItem *cv) {
  QDomNode n = param_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "Values") {
	parseInterpolatedControlVariableValues(n, cv);
      } else {
	pneDebug("Ignoring unknown InterpolatedControlVariable Parameter tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseInterpolatedControlVariableItem(QDomNode &cv_node, InterpolatedControlVariableItem *cv, int ce, int ne) {
  cv->setInitialized(true);
  cv->setNumEnvironment(ne);
  cv->setCurEnvironment(ce);
  QDomNode n = cv_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(parseMolRxnBase(e, cv)) {
      } else if(parseMoleculeBase(e, cv)) {
      } else if(e.tagName() == "Parameters") {
	if(!parseInterpolatedControlVariableParameters(n, cv)) return false;
      } else {
	pneDebug("Ignoring unknown InterpolatedControlVariable tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseSquareWaveControlVariableParameters(QDomNode &param_node, SquareWaveControlVariableItem *cv) {
  int env = param_node.toElement().elementsByTagName("Number").item(0).toElement().text().toInt();
  cv->setCurEnvironment(env);

  QDomNode n = param_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "Number") {
      } else if(e.tagName() == "Values") {
	QString str = e.text().simplifyWhiteSpace();
	double v1 = str.section(' ', 0, 0).toDouble();
	double v2 = str.section(' ', 1, 1).toDouble();
	cv->setValues(v1, v2);
      } else if(e.tagName() == "Duration") {
	QString str = e.text().simplifyWhiteSpace();
	double v1 = str.section(' ', 0, 0).toDouble();
	double v2 = str.section(' ', 1, 1).toDouble();
	cv->setDuration(v1, v2);
      } else if(e.tagName() == "TransitionTime") {
	QString str = e.text().simplifyWhiteSpace();
	double v1 = str.section(' ', 0, 0).toDouble();
	double v2 = str.section(' ', 1, 1).toDouble();
	cv->setTransitionTime(v1, v2);
      } else if(e.tagName() == "PhaseShift") {
	cv->setPhaseShift(e.text().toDouble());
      } else if(e.tagName() == "Noise") {
	cv->setNoise(e.text().toDouble());
      } else {
	pneDebug("Ignoring unknown SquareWaveControlVariable Parameter tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseSquareWaveControlVariableItem(QDomNode &cv_node, SquareWaveControlVariableItem *cv, int ce, int ne) {
  cv->setInitialized(true);
  cv->setNumEnvironment(ne);
  cv->setCurEnvironment(ce);
  QDomNode n = cv_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(parseMolRxnBase(e, cv)) {
      } else if(parseMoleculeBase(e, cv)) {
      } else if(e.tagName() == "Parameters") {
	if(!parseSquareWaveControlVariableParameters(n, cv)) return false;
      } else {
	pneDebug("Ignoring unknown SquareWaveControlVariable tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseMoleculeItem(QDomNode &mol_node, MoleculeItem *mol) {
  mol->setInitialized(true);
  QDomNode n = mol_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(parseMolRxnBase(e, mol)) {
      } else if(parseMoleculeBase(e, mol)) {
      } else if(e.tagName() == "InitialConcentration") {
	mol->setInitialConcentration(e.text().toDouble());
      } else if(e.tagName() == "DecayRate") {
	mol->setDecayRate(e.text().toDouble());
      } else if(e.tagName() == "Decays") {
	mol->setDecays(true);
      } else {
	pneDebug("Ignoring unknown Molecule tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseMolecules(QDomNode &molecules_node, QList<PathwayCanvasItem> &items, int ce, int ne) {
  QDomNode n = molecules_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "Annotation") {
	AnnotationItem *ann = new AnnotationItem(NULL);
	if(!parseAnnotationItem(n, ann)) return false;
	items.append(ann);
      } else if(e.tagName() == "InterpolatedControlVariable") {
	InterpolatedControlVariableItem *cv = new InterpolatedControlVariableItem(NULL);
	if(!parseInterpolatedControlVariableItem(n, cv, ce, ne)) return false;
	items.append(cv);
      } else if(e.tagName() == "SquareWaveControlVariable") {
	SquareWaveControlVariableItem *cv = new SquareWaveControlVariableItem(NULL);
	if(!parseSquareWaveControlVariableItem(n, cv, ce, ne)) return false;
	items.append(cv);
      } else if(e.tagName() == "DNAMolecule") {
	MoleculeItem *mol = new DNAMoleculeItem(NULL);
	if(!parseMoleculeItem(n, mol)) return false;
	items.append(mol);
      } else if(e.tagName() == "mRNAMolecule") {
	MoleculeItem *mol = new mRNAMoleculeItem(NULL);
	if(!parseMoleculeItem(n, mol)) return false;
	items.append(mol);
      } else if(e.tagName() == "EnzymeMolecule") {
	MoleculeItem *mol = new EnzymeMoleculeItem(NULL);
	if(!parseMoleculeItem(n, mol)) return false;
	items.append(mol);
      } else if(e.tagName() == "MetaboliteMolecule") {
	MoleculeItem *mol = new MetaboliteMoleculeItem(NULL);
	if(!parseMoleculeItem(n, mol)) return false;
	items.append(mol);
      } else if(e.tagName() == "TranscriptionFactorMolecule") {
	MoleculeItem *mol = new TranscriptionFactorMoleculeItem(NULL);
	if(!parseMoleculeItem(n, mol)) return false;
	items.append(mol);
      } else if(e.tagName() == "ComplexMolecule") {
	MoleculeItem *mol = new ComplexMoleculeItem(NULL);
	if(!parseMoleculeItem(n, mol)) return false;
	items.append(mol);
      } else {
	pneDebug("Ignoring unknown Molecules tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseReactionEdge(QDomNode &edge_node, QList<PathwayCanvasItem> &items, MassActionReactionItem *rxn, ReactionEdgeItem *edge) {
  bool reactant_found = false;  
  QDomNodeList nl;
  MolRxnBaseItem *in = NULL;

  {
    nl = edge_node.toElement().elementsByTagName("Reactant");
    if(nl.count() > 0) {
      reactant_found = true;
      QString label = nl.item(0).toElement().elementsByTagName("Label").item(0).toElement().text();
      int id = nl.item(0).toElement().elementsByTagName("Id").item(0).toElement().text().toInt();
      PathwayCanvasItem *i;
      for(i=items.first(); i; i=items.next()) {
	if(isMolecule(i)) {
	  if(((MolRxnBaseItem*)i)->id() == id) {
	    in = edge->addPoint((MoleculeItem*)i);
	    break;
	  }
	}
      }
      if(!i) {
	pneDebug(QString("Unable to find Reactant id: %1").arg(id));
	return false;
      }
    } else {
      // Assume there is going to be a product
      in = edge->addPoint(rxn);
    }
  }

  {
    nl = edge_node.toElement().elementsByTagName("InternalNodes");
    if(nl.count() > 0) {
      QDomNode n = nl.item(0).firstChild();
      while(!n.isNull()) {
	if(n.isElement()) {
	  QDomElement e = n.toElement();
	  
	  if(e.tagName() == "InternalNode") {
	    int id = e.elementsByTagName("Id").item(0).toElement().text().toInt();
	    if(id != 0) {
	      QString str = e.elementsByTagName("Pos").item(0).toElement().text();
	      int x = str.section(' ', 0, 0).toInt();
	      int y = str.section(' ', 1, 1).toInt();
	      in->setId(id);
	      in->move(x, y);
	      items.append(in);
	      in = edge->addPoint(in);
	    } else {
	      pneDebug("Ignoring corrupt InternalNode");
	    }
	  } else {
	    pneDebug("Ignoring unknown InternalNodes tag: " + e.tagName());
	  }
	}
	n = n.nextSibling();
      }
    }
  }

  {
    nl = edge_node.toElement().elementsByTagName("Product");
    if(nl.count() > 0) {
      if(reactant_found) {
	pneDebug("Reactant and product found in same edge");
	return false;
      }
      QString label = nl.item(0).toElement().elementsByTagName("Label").item(0).toElement().text();
      int id = nl.item(0).toElement().elementsByTagName("Id").item(0).toElement().text().toInt();
      PathwayCanvasItem *i;
      for(i=items.first(); i; i=items.next()) {
	if(isMolecule(i)) {
	  if(((MolRxnBaseItem*)i)->id() == id) {
	    in = edge->addPoint((MoleculeItem*)i);
	    break;
	  }
	}
      }
      if(!i) {
	pneDebug(QString("Unable to find Reactant id: %1").arg(id));
	return false;
      }
    } else {
      if(reactant_found) {
	in = edge->addPoint(rxn);
      } else {
	pneDebug("No product nor reactant found for edge");
	return false;
      }
    }
  }

  if(in) {
    pneDebug("\"in\" was deleted during load...");
    delete in;
    in = NULL;
  }

  QDomNode n = edge_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "Id") {
	edge->setId(e.text().toInt());
      } else if(e.tagName() == "Product") {
      } else if(e.tagName() == "Reactant") {
      } else if(e.tagName() == "Coefficient") {
	edge->setCoefficient(e.text().toInt());
      } else if(e.tagName() == "InternalNodes") {
      } else {
	pneDebug("Ignoring unknown ReactionEdge tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseReactionEdges(QDomNode &edges_node, QList<PathwayCanvasItem> &items, MassActionReactionItem *rxn) {
  QDomNode n = edges_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "ReactionEdge") {
	// Must append the edge before parsing else the internal nodes will
	// be deleted first and creating a seg fault when the edge tries to
	// disassociate with them.  
	ReactionEdgeItem *edge = new ReactionEdgeItem(NULL);
	items.append(edge);
	if(!parseReactionEdge(n, items, rxn, edge)) return false;
	//items.append(edge);
      } else {
	pneDebug("Ignoring unknown Edges tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseMassActionReactionItem(QDomNode &rxn_node, QList<PathwayCanvasItem> &items, MassActionReactionItem *rxn) {
  rxn->setInitialized(true);
  QDomNode n = rxn_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(parseMolRxnBase(e, rxn)) {
      } else if(e.tagName() == "Kf") {
	rxn->setKf(e.text().toDouble());
      } else if(e.tagName() == "Kr") {
	rxn->setKr(e.text().toDouble());
      } else if(e.tagName() == "Edges") {
	parseReactionEdges(n, items, rxn);
      } else if(e.tagName() == "Reversible") {
	rxn->setReversible(true);
      } else {
	pneDebug("Ignoring unknown MassActionReaction tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseReactions(QDomNode &reactions_node, QList<PathwayCanvasItem> &items) {
  QDomNode n = reactions_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "MassActionReaction") {
	MassActionReactionItem *rxn = new MassActionReactionItem(NULL);
	if(!parseMassActionReactionItem(n, items, rxn)) return false;
	items.append(rxn);
      } else {
	pneDebug("Ignoring unknown Reactions tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseNetwork(QDomNode &net_node, QList<PathwayCanvasItem> &items, int ce, int ne) {
  QDomNode n = net_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();
      
      if(e.tagName() == "Molecules") { // Need to load molecules before next 2!
	if(!parseMolecules(n, items, ce, ne)) return false;
      } else if(e.tagName() == "Subnetworks") {
	if(!parseSubnetworks(n, items, ce, ne)) return false;
      } else if(e.tagName() == "Reactions") {
	if(!parseReactions(n, items)) return false;
      } else {
	pneDebug("Ignoring unknown Network tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseSubnetworkEdge(QDomNode &edge_node, QList<PathwayCanvasItem> &items, SubnetworkItem *sn, SubnetworkEdgeItem *edge) {
  bool reactant_found = false;  
  QDomNodeList nl;
  MolRxnBaseItem *in = NULL;

  {
    nl = edge_node.toElement().elementsByTagName("SubnetworkMolecule");
    if(nl.count() > 0) {
      reactant_found = true;
      QString label = nl.item(0).toElement().elementsByTagName("Label").item(0).toElement().text();
      int id = nl.item(0).toElement().elementsByTagName("Id").item(0).toElement().text().toInt();
      MoleculeItem *m = sn->exportedMolecule(label);
      if(m) {
	in = edge->addPoint(sn, m->label());
      } else {
	pneDebug(QString("Unable to find SubnetworkMolecule id: %1").arg(id));
	return false;
      }
    } else {
      pneDebug(QString("No SubnetworkMolecule specified"));
      return false;
    }
  }

  {
    nl = edge_node.toElement().elementsByTagName("InternalNodes");
    if(nl.count() > 0) {
      QDomNode n = nl.item(0).firstChild();
      while(!n.isNull()) {
	if(n.isElement()) {
	  QDomElement e = n.toElement();
	  
	  if(e.tagName() == "InternalNode") {
	    int id = e.elementsByTagName("Id").item(0).toElement().text().toInt();
	    if(id != 0) {
	      QString str = e.elementsByTagName("Pos").item(0).toElement().text();
	      int x = str.section(' ', 0, 0).toInt();
	      int y = str.section(' ', 1, 1).toInt();
	      in->setId(id);
	      in->move(x, y);
	      items.append(in);
	      in = edge->addPoint(in);
	    } else {
	      pneDebug("Ignoring corrupt InternalNode");
	    }
	  } else {
	    pneDebug("Ignoring unknown SubnetworkInternalNodes tag: " + e.tagName());
	  }
	}
	n = n.nextSibling();
      }
    }
  }

  {
    nl = edge_node.toElement().elementsByTagName("Molecule");
    if(nl.count() > 0) {
      QString label = nl.item(0).toElement().elementsByTagName("Label").item(0).toElement().text();
      int id = nl.item(0).toElement().elementsByTagName("Id").item(0).toElement().text().toInt();
      PathwayCanvasItem *i;
      for(i=items.first(); i; i=items.next()) {
	if(isMolecule(i)) {
	  if(((MolRxnBaseItem*)i)->id() == id) {
	    in = edge->addPoint((MoleculeItem*)i);
	    break;
	  }
	}
      }
      if(!i) {
	pneDebug(QString("Unable to find Molecule id: %1").arg(id));
	return false;
      }
    } else {
      pneDebug(QString("No Molecule specified"));
      return false;
    }
  }

  if(in) {
    pneDebug("\"in\" was deleted during subnetwork load...");
    delete in;
    in = NULL;
  }

  QDomNode n = edge_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "Id") {
	edge->setId(e.text().toInt());
      } else if(e.tagName() == "SubnetworkMolecule") {
      } else if(e.tagName() == "Molecule") {
      } else if(e.tagName() == "InternalNodes") {
      } else {
	pneDebug("Ignoring unknown SubnetworkEdge tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseSubnetworkEdges(QDomNode &edges_node, QList<PathwayCanvasItem> &items, SubnetworkItem *sn) {
  QDomNode n = edges_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "SubnetworkEdge") {
	// Must append the edge before parsing else the internal nodes will
	// be deleted first and creating a seg fault when the edge tries to
	// disassociate with them.  
	SubnetworkEdgeItem *edge = new SubnetworkEdgeItem(NULL);
	items.append(edge);
	if(!parseSubnetworkEdge(n, items, sn, edge)) return false;
	//items.append(edge);
      } else {
	pneDebug("Ignoring unknown SubnetworkEdges tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseSubnetworkItem(QDomNode &sn_node, QList<PathwayCanvasItem> &items, SubnetworkItem *sn, int ce, int ne) {
  sn->setInitialized(true);
  QDomNode n = sn_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(parseMolRxnBase(e, sn)) {
      } else if(e.tagName() == "Network") {  // Must be loaded before edges
	QList<PathwayCanvasItem> sn_items;
	if(!parseNetwork(n, sn_items, ce, ne)) return false;
	sn->setItems(sn_items);
      } else if(e.tagName() == "Edges") {
	parseSubnetworkEdges(n, items, sn);
      } else {
	pneDebug("Ignoring unknown Subnetwork tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

static bool
parseSubnetworks(QDomNode &subnetworks_node, QList<PathwayCanvasItem> &items, int ce, int ne) {
  QDomNode n = subnetworks_node.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QDomElement e = n.toElement();

      if(e.tagName() == "Subnetwork") {
	SubnetworkItem *sn = new SubnetworkItem(NULL);
	if(!parseSubnetworkItem(n, items, sn, ce, ne)) return false;
	items.append(sn);
      } else {
	pneDebug("Ignoring unknown Subnetworks tag: " + e.tagName());
      }
    }
    n = n.nextSibling();
  }
  return true;
}

bool
pathwayLoad_v23(QDomDocument &doc, Editor &editor, Fitness &fitness, Simulation &simulation,
		QList<PathwayCanvasItem> &items, int &version) {
  QDomElement root = doc.documentElement();
  if(root.tagName() != "Network") return false;

  QDomNode n = root.firstChild();
  while(!n.isNull()) {
    if(n.isElement()) {
      QString tag = n.toElement().tagName();
      
      if(tag == "Info") {
	if(!parseInfo(n, version)) return false;
      } else if(tag == "Editor") {
	if(!parseEditor(n, editor)) return false;
      } else if(tag == "Fitness") {
	if(!parseFitness(n, fitness)) return false;
      } else if(tag == "Simulation") {
	if(!parseSimulation(n, simulation)) return false;
      } else if(tag == "Molecules") { // Need to load molecules before next 2!
	if(!parseMolecules(n, items, simulation.curEnvironment(), simulation.numEnvironment())) return false;
      } else if(tag == "Subnetworks") {
	if(!parseSubnetworks(n, items, simulation.curEnvironment(), simulation.numEnvironment())) return false;
      } else if(tag == "Reactions") {
	if(!parseReactions(n, items)) return false;
      }
    }
    n = n.nextSibling();
  }  
  return true;
}
