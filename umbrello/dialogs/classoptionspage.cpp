/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <klocale.h>

#include "../umlview.h"
#include "../conceptwidget.h"
#include "classoptionspage.h"

ClassOptionsPage::ClassOptionsPage(QWidget * pParent, UMLWidget * pWidget ) : QWidget( pParent ) {
	m_pWidget = dynamic_cast<ConceptWidget *>( pWidget );
	int margin = fontMetrics().height();

	bool sig = false;
	Uml::Signature_Type sigtype;

	QVBoxLayout * topLayout = new QVBoxLayout(this);
	topLayout -> setSpacing(6);
	m_pVisibilityGB = new QGroupBox(i18n("Show"), this);
	topLayout -> addWidget(m_pVisibilityGB);
	QGridLayout * visibilityLayout = new QGridLayout(m_pVisibilityGB);
	visibilityLayout -> setSpacing(10);
	visibilityLayout -> setMargin(margin);

	m_pShowScopeCB = new QCheckBox(i18n("Visibility"), m_pVisibilityGB);
	m_pShowScopeCB -> setChecked(m_pWidget -> getShowScope());
	visibilityLayout -> addWidget(m_pShowScopeCB, 0, 0);

	m_pShowAttsCB = new QCheckBox(i18n("Attributes"), m_pVisibilityGB);
	m_pShowAttsCB -> setChecked(m_pWidget -> getShowAtts());
	visibilityLayout -> addWidget(m_pShowAttsCB, 0, 1);

	m_pShowOpsCB = new QCheckBox(i18n("Operations"), m_pVisibilityGB);
	m_pShowOpsCB -> setChecked(m_pWidget -> getShowOps());
	visibilityLayout -> addWidget(m_pShowOpsCB, 1, 0);

	m_pShowStereotypeCB = new QCheckBox(i18n("Stereotype"), m_pVisibilityGB);
	m_pShowStereotypeCB -> setChecked(m_pWidget -> getShowStereotype());
	visibilityLayout -> addWidget(m_pShowStereotypeCB, 1, 1);

	m_pShowAttSigCB = new QCheckBox(i18n("Attribute signature"), m_pVisibilityGB);
	sigtype = m_pWidget -> getShowAttSigs();
	if(sigtype == Uml::st_NoSig || sigtype == Uml::st_NoSigNoScope)
		sig = false;
	else
		sig = true;
	m_pShowAttSigCB -> setChecked(sig);
	visibilityLayout -> addWidget(m_pShowAttSigCB, 2, 0);

	m_pShowPackageCB = new QCheckBox(i18n("Package"), m_pVisibilityGB);
	m_pShowPackageCB -> setChecked(m_pWidget -> getShowPackage());
	visibilityLayout -> addWidget(m_pShowPackageCB,2, 1);

	sigtype = m_pWidget -> getShowOpSigs();
	if(sigtype == Uml::st_NoSig || sigtype == Uml::st_NoSigNoScope)
		sig = false;
	else
		sig = true;
	m_pShowOpSigCB = new QCheckBox(i18n("Operation signature"), m_pVisibilityGB);
	m_pShowOpSigCB -> setChecked(sig);
	visibilityLayout -> addWidget(m_pShowOpSigCB, 3, 0);
	visibilityLayout -> setRowStretch(3, 1);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassOptionsPage::~ClassOptionsPage() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassOptionsPage::updateUMLWidget() {
	m_pWidget -> setShowScope(m_pShowScopeCB -> isChecked());
	m_pWidget -> setShowAtts(m_pShowAttsCB -> isChecked());
	m_pWidget -> setShowOps(m_pShowOpsCB -> isChecked());
	m_pWidget -> setShowStereotype(m_pShowStereotypeCB -> isChecked());
	m_pWidget -> setShowPackage(m_pShowPackageCB -> isChecked());
	m_pWidget -> setShowAttSigs( m_pShowAttSigCB -> isChecked() );
	m_pWidget -> setShowOpSigs( m_pShowOpSigCB -> isChecked() );
}


