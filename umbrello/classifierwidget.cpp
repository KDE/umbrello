 /*
  *  copyright (C) 2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>

#include "classifierwidget.h"
#include "classifier.h"
#include "operation.h"
#include "umlview.h"

ClassifierWidget::ClassifierWidget(UMLView * view, UMLClassifier *c, Uml::Widget_Type wt)
  : UMLWidget(view, c) {
	init(wt);
}

ClassifierWidget::~ClassifierWidget() {}

void ClassifierWidget::init(Uml::Widget_Type wt) {
	UMLWidget::setBaseType(wt);

	const Settings::OptionState& ops = m_pView->getOptionState();
	m_bShowScope = ops.classState.showScope;
	m_bShowOperations = ops.classState.showOps;
	m_bShowPublicOnly = false;
	m_bShowPackage = ops.classState.showPackage;
	/* setShowOpSigs( ops.classState.showOpSig );
	  Cannot do that because we get "pure virtual method called". Open code:
	 */
	if( !ops.classState.showOpSig ) {
		if (m_bShowScope)
			m_ShowOpSigs = Uml::st_NoSig;
		else
			m_ShowOpSigs = Uml::st_NoSigNoScope;

	} else if (m_bShowScope)
		m_ShowOpSigs = Uml::st_ShowSig;
	else
		m_ShowOpSigs = Uml::st_SigNoScope;
}

void ClassifierWidget::updateSigs() {
	//turn on scope
	if (m_bShowScope) {
		if (m_ShowOpSigs == Uml::st_NoSigNoScope) {
			m_ShowOpSigs = Uml::st_NoSig;
		} else if (m_ShowOpSigs == Uml::st_SigNoScope) {
			m_ShowOpSigs = Uml::st_ShowSig;
		}
	} else { //turn off scope
		if (m_ShowOpSigs == Uml::st_ShowSig) {
			m_ShowOpSigs = Uml::st_SigNoScope;
		} else if (m_ShowOpSigs == Uml::st_NoSig) {
			m_ShowOpSigs = Uml::st_NoSigNoScope;
		}
	}
	//To be done by inheriting classes:
	// calculateSize();
	// update();
}

bool ClassifierWidget::getShowOps() const {
	return m_bShowOperations;
}

void ClassifierWidget::setShowOps(bool _show) {
	m_bShowOperations = _show;
	updateSigs();
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowOps() {
	m_bShowOperations = !m_bShowOperations;
	updateSigs();
	calculateSize();
	update();
}

bool ClassifierWidget::getShowPublicOnly() const {
	return m_bShowPublicOnly;
}

void ClassifierWidget::setShowPublicOnly(bool _status) {
	m_bShowPublicOnly = _status;
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowPublicOnly() {
	m_bShowPublicOnly = !m_bShowPublicOnly;
	calculateSize();
	update();
}

bool ClassifierWidget::getShowScope() const {
	return m_bShowScope;
}

void ClassifierWidget::setShowScope(bool _scope) {
	m_bShowScope = _scope;
	updateSigs();
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowScope() {
	m_bShowScope = !m_bShowScope;
	updateSigs();
	calculateSize();
	update();
}

Uml::Signature_Type ClassifierWidget::getShowOpSigs() const {
	return m_ShowOpSigs;
}

void ClassifierWidget::setShowOpSigs(bool _status) {
	if( !_status ) {
		if (m_bShowScope)
			m_ShowOpSigs = Uml::st_NoSig;
		else
			m_ShowOpSigs = Uml::st_NoSigNoScope;
	} else if (m_bShowScope)
		m_ShowOpSigs = Uml::st_ShowSig;
	else
		m_ShowOpSigs = Uml::st_SigNoScope;
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowOpSigs() {
	if (m_ShowOpSigs == Uml::st_ShowSig || m_ShowOpSigs == Uml::st_SigNoScope) {
		if (m_bShowScope) {
			m_ShowOpSigs = Uml::st_NoSig;
		} else {
			m_ShowOpSigs = Uml::st_NoSigNoScope;
		}
	} else if (m_bShowScope) {
		m_ShowOpSigs = Uml::st_ShowSig;
	} else {
		m_ShowOpSigs = Uml::st_SigNoScope;
	}
	calculateSize();
	update();
}

bool ClassifierWidget::getShowPackage() const {
	return m_bShowPackage;
}

void ClassifierWidget::setShowPackage(bool _status) {
	m_bShowPackage = _status;
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowPackage() {
	m_bShowPackage = !m_bShowPackage;
	updateSigs();
	calculateSize();
	update();
}

void ClassifierWidget::setOpSignature(Uml::Signature_Type sig) {
	m_ShowOpSigs = sig;
	updateSigs();
	calculateSize();
	update();
}

int ClassifierWidget::displayedMembers(Uml::Object_Type ot) {
	UMLClassifier *c = static_cast<UMLClassifier*>(m_pObject);
	int count = 0;
	UMLClassifierListItemList list = c->getFilteredList(ot);
	for (UMLClassifierListItem *m = list.first(); m; m = list.next()) {
		if (!(m_bShowPublicOnly && m->getScope() != Uml::Public))
			count++;
	}
	return count;
}

int ClassifierWidget::displayedOperations() {
	if (!m_bShowOperations)
		return 0;
	return displayedMembers(Uml::ot_Operation);
}

bool ClassifierWidget::activate(IDChangeLog* ChangeLog /* = 0 */) {
	bool status = UMLWidget::activate(ChangeLog);
	if (status) {
		calculateSize();
	}
	return status;
}

