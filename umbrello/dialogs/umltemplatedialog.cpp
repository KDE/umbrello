/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "umltemplatedialog.h"
#include "../template.h"
#include "../class.h"

#include <qlayout.h>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

UMLTemplateDialog::UMLTemplateDialog(QWidget* pParent, UMLTemplate* pTemplate)
  : KDialogBase( Plain, i18n("Template Properties"), Help | Ok | Cancel , Ok, pParent, "_UMLTemplateDLG_", true, true) {
	m_pTemplate = pTemplate;
	setupDialog();
}

UMLTemplateDialog::~UMLTemplateDialog() {}

void UMLTemplateDialog::setupDialog() {
	int margin = fontMetrics().height();

	QVBoxLayout* mainLayout = new QVBoxLayout( plainPage() );

	m_pValuesGB = new QGroupBox(i18n("General Properties"), plainPage() );
	QGridLayout* valuesLayout = new QGridLayout(m_pValuesGB, 3, 2);
	valuesLayout->setMargin(margin);
	valuesLayout->setSpacing(10);

	m_pTypeL = new QLabel(i18n("&Type:"), m_pValuesGB);
	valuesLayout->addWidget(m_pTypeL, 0, 0);

	m_pTypeCB = new QComboBox(m_pValuesGB);
	valuesLayout->addWidget(m_pTypeCB, 0, 1);
	m_pTypeL->setBuddy(m_pTypeCB);

	m_pNameL = new QLabel(i18n("&Name:"), m_pValuesGB);
	valuesLayout->addWidget(m_pNameL, 1, 0);

	m_pNameLE = new QLineEdit(m_pValuesGB);
	valuesLayout->addWidget(m_pNameLE, 1, 1);

	m_pNameLE->setText( m_pTemplate->getName() );
	m_pNameL->setBuddy(m_pNameLE);

	m_pStereoTypeL = new QLabel(i18n("&Stereotype name:"), m_pValuesGB);
	valuesLayout -> addWidget(m_pStereoTypeL, 2, 0);

	m_pStereoTypeLE = new QLineEdit(m_pValuesGB);
	valuesLayout -> addWidget(m_pStereoTypeLE, 2, 1);

	m_pStereoTypeLE -> setText(m_pTemplate -> getStereotype());
	m_pStereoTypeL->setBuddy(m_pStereoTypeLE);

	mainLayout->addWidget(m_pValuesGB);

	//add some standard attribute types to combo box
	QString types[] = {
	                      i18n("class"), i18n("int"), i18n("long"), i18n("bool"),
			      i18n("string"), i18n("double"), i18n("float"), i18n("date")
	                  };

	for (int i=0; i<8; i++) {
		m_pTypeCB->insertItem(types[i]);
	}

	m_pTypeCB->setEditable(true);
	m_pTypeCB->setDuplicatesEnabled(false);//only allow one of each type in box
	m_pTypeCB->setAutoCompletion(true);

	//work out which one to select
	int typeBoxCount = 0;
	bool foundType = false;
	while (typeBoxCount < m_pTypeCB->count() && foundType == false) {
		QString typeBoxString = m_pTypeCB->text(typeBoxCount);
		if ( typeBoxString == m_pTemplate->getTypeName() ) {
			foundType = true;
			m_pTypeCB->setCurrentItem(typeBoxCount);
		} else {
			typeBoxCount++;
		}
	}

	if (!foundType) {
		m_pTypeCB->insertItem( m_pTemplate->getTypeName(), 0 );
		m_pTypeCB->setCurrentItem(0);
	}

	m_pNameLE->setFocus();
}

bool UMLTemplateDialog::apply() {
	UMLClass * pClass = dynamic_cast<UMLClass *>( m_pTemplate->parent() );

	m_pTemplate->setTypeName( m_pTypeCB->currentText() );
	QString name = m_pNameLE->text();
	if( name.length() == 0 ) {
		KMessageBox::error(this, i18n("You have entered an invalid template name."),
		                   i18n("Template Name Invalid"), false);
		m_pNameLE->setText( m_pTemplate->getName() );
		return false;
	}

	UMLObjectList list= pClass->findChildObject(Uml::ot_Attribute, name);
	if( list.count() != 0 && list.findRef( m_pTemplate ) ) {
		KMessageBox::error(this, i18n("The attribute name you have chosen is already being used in this operation."),
		                   i18n("Attribute Name Not Unique"), false);
		m_pNameLE->setText( m_pTemplate->getName() );
		return false;
	}
	m_pTemplate->setName(name);

	m_pTemplate->setStereotype( m_pStereoTypeLE->text() );

	return true;
}

void UMLTemplateDialog::slotApply() {
	apply();
}

void UMLTemplateDialog::slotOk() {
	if ( apply() ) {
		accept();
	}
}

#include "umltemplatedialog.moc"
