/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "selectopdlg.h"
#include "../attribute.h"
#include "../operation.h"
#include <klocale.h>
#include <kdebug.h>
#include <qlayout.h>

SelectOpDlg::SelectOpDlg(QWidget * parent, UMLClassifier * c) : KDialogBase(Plain, i18n("Select Operation"), Ok | Cancel , Ok, parent, "_SELOPDLG_", true, true) {
	QVBoxLayout * topLayout = new QVBoxLayout(plainPage());

	m_pOpGB = new QGroupBox(i18n("Select Operation"), plainPage());
	topLayout -> addWidget(m_pOpGB);

	QGridLayout * mainLayout = new QGridLayout(m_pOpGB, 3, 2);
	mainLayout -> setSpacing(spacingHint());
	mainLayout -> setMargin(fontMetrics().height());

	m_pSeqL = new QLabel(i18n("Sequence number:"), m_pOpGB);
	mainLayout -> addWidget(m_pSeqL, 0, 0);

	m_pSeqLE = new QLineEdit(m_pOpGB);
	mainLayout -> addWidget(m_pSeqLE, 0, 1);

	m_pOpCB = new QComboBox(m_pOpGB);
	mainLayout -> addWidget(m_pOpCB, 1, 1);

	m_pOpRB = new QRadioButton(i18n("Class operation:"), m_pOpGB);
	mainLayout -> addWidget(m_pOpRB, 1, 0);

	m_pCustomRB = new QRadioButton(i18n("Custom operation:"), m_pOpGB);
	mainLayout -> addWidget(m_pCustomRB, 2, 0);

	m_pOpLE = new QLineEdit(m_pOpGB);
	mainLayout -> addWidget(m_pOpLE, 2, 1);

	m_pOpBG = new QButtonGroup(0);
	m_pOpBG -> insert(m_pOpRB, OP);
	m_pOpBG -> insert(m_pCustomRB, CUSTOM);
	m_pOpBG -> setExclusive(true);
	m_pOpBG -> setButton(OP);

	QPtrList<UMLClassifierListItem>* list = c -> getOpList();
	UMLClassifierListItem* obj=0;
	for(obj=list -> first();obj != 0;obj=list -> next())
		m_pOpCB -> insertItem(obj -> toString(Uml::st_SigNoScope));
	//disableResize();
	connect(m_pOpBG, SIGNAL(clicked(int)), this, SLOT(slotSelected(int)));
	m_nOpCount = c -> operations();
	slotSelected(OP);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
SelectOpDlg::~SelectOpDlg() {
	disconnect(m_pOpBG, SIGNAL(clicked(int)), this, SLOT(slotSelected(int)));
	delete m_pOpBG;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString SelectOpDlg::getOpText() {
	if(m_pOpRB -> isChecked())
		return m_pOpCB -> currentText();
	else
		return m_pOpLE -> text();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SelectOpDlg::slotSelected(int id) {
	if(id == OP) {
		m_pOpLE -> setEnabled(false);
		if(m_nOpCount > 0)
			m_pOpCB -> setEnabled(true);
	} else {
		m_pOpLE -> setEnabled(true);
		m_pOpCB -> setEnabled(false);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SelectOpDlg::setCustomOp(QString op) {
	m_pOpLE -> setText(op);
	if(op.length() > 0) {
		slotSelected(CUSTOM);
		m_pCustomRB -> setChecked(true);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString SelectOpDlg::getSeqNumber() {
	kdDebug() << k_funcinfo << "returning:" << m_pSeqLE->text() << endl;
	return m_pSeqLE->text();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SelectOpDlg::setSeqNumber(QString num) {
	m_pSeqLE->setText(num);
}



#include "selectopdlg.moc"
