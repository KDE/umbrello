/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "assocgenpage.h"

#include <qlayout.h>
#include <qwidget.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qmultilineedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "../association.h"

AssocGenPage::AssocGenPage(QWidget *parent, UMLAssociation *assoc)
	: QWidget(parent)
{
	m_assoc = assoc;
	constructWidget();
}

AssocGenPage::~AssocGenPage() {}

void AssocGenPage::constructWidget() {

	// general configuration of the GUI
	int margin = fontMetrics().height();
	setMinimumSize(310,330);
	QVBoxLayout * topLayout = new QVBoxLayout(this);
	topLayout -> setSpacing(6);

	// group boxes for name, documentation properties
	QGroupBox *nameGB = new QGroupBox(this);
	QGroupBox *docGB = new QGroupBox(this);
	nameGB -> setTitle(i18n("Properties"));
	docGB -> setTitle(i18n("Documentation"));
	topLayout -> addWidget(nameGB);
	topLayout -> addWidget(docGB);

	QGridLayout * nameLayout = new QGridLayout(nameGB, 2, 2);
	nameLayout -> setSpacing(6);
	nameLayout -> setMargin(margin);

	//Association name
	nameLayout -> addWidget(new QLabel(i18n("Name:"),nameGB), 0, 0);
	m_pAssocNameLE = new QLineEdit(nameGB);
	nameLayout -> addWidget(m_pAssocNameLE, 0, 1);
	m_pAssocNameLE->setText( m_assoc->getName() );

	// documentation
	QHBoxLayout * docLayout = new QHBoxLayout(docGB);
	docLayout -> setMargin(margin);

	m_pDoc = new QMultiLineEdit(docGB);
	docLayout -> addWidget(m_pDoc);
	m_pDoc-> setText(m_assoc-> getDoc());

	// Association Type
	nameLayout -> addWidget(new QLabel(i18n("Type:"), nameGB), 1, 0);
	m_pTypeLE = new QLineEdit(nameGB);
	m_pTypeLE->setEnabled(false);
	nameLayout -> addWidget(m_pTypeLE, 1, 1);

	// set value in association type
	m_pTypeLE->setText(m_assoc->toString(m_assoc->getAssocType()));

	m_pDoc->setWordWrap(QMultiLineEdit::WidgetWidth);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void AssocGenPage::updateObject() {

	m_assoc->setName(m_pAssocNameLE->text());
	m_assoc->setDoc(m_pDoc->text());
}


#include "assocgenpage.moc"
