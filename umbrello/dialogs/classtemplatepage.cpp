/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classtemplatepage.h"
#include "umltemplatedialog.h"
#include "../template.h"
#include "../listpopupmenu.h"
#include <kbuttonbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <qlayout.h>

ClassTemplatePage::ClassTemplatePage(QWidget* parent, UMLClass * myclass , UMLDoc* doc) : QWidget(parent) {
	m_bSigWaiting = false;
	m_pDoc = doc;
	m_pClass = myclass;
	m_pMenu = 0;
	int margin = fontMetrics().height();
	setMinimumSize(310,330);

	QVBoxLayout* mainLayout = new QVBoxLayout( this );
	mainLayout -> setSpacing(10);

	m_pTemplateGB = new QGroupBox(i18n("Templates"), this);
	QGridLayout* templateLayout = new QGridLayout(m_pTemplateGB, 2, 2);
	templateLayout->setMargin(margin);
	templateLayout->setSpacing(10);

	m_pTemplateLB = new QListBox(m_pTemplateGB);
	templateLayout->addWidget(m_pTemplateLB, 0, 0);

	QVBoxLayout* buttonLayout = new QVBoxLayout(templateLayout);
	m_pUpArrowB = new KArrowButton(m_pTemplateGB);
	m_pUpArrowB->setEnabled(false);
	buttonLayout->addWidget(m_pUpArrowB);

	m_pDownArrowB = new KArrowButton(m_pTemplateGB, Qt::DownArrow);
	m_pDownArrowB->setEnabled(false);
	buttonLayout->addWidget(m_pDownArrowB);

	KButtonBox* buttonBox = new KButtonBox(m_pTemplateGB);
	buttonBox->addButton( i18n("N&ew Template..."), this, SLOT(slotNewTemplate()) );
	m_pDeleteTemplateButton = buttonBox->addButton( i18n("&Delete"),
							this, SLOT(slotDelete()) );
	m_pPropertiesButton = buttonBox->addButton( i18n("&Properties"), this, SLOT(slotProperties()) );
	templateLayout->addMultiCellWidget(buttonBox, 1, 1, 0, 1);

	mainLayout->addWidget(m_pTemplateGB);

	m_pDocGB = new QGroupBox(i18n("Documentation"), this);
	QVBoxLayout* docLayout = new QVBoxLayout(m_pDocGB);
	m_pDocTE = new QTextEdit(m_pDocGB);
	docLayout->setMargin(margin);
	docLayout->setSpacing(10);
	docLayout->addWidget(m_pDocTE);
	mainLayout->addWidget(m_pDocGB);

	//add templates to list
	UMLTemplate* theTemplate;
	m_pTemplateList = myclass->getTemplateList();
	for (theTemplate=m_pTemplateList->first();theTemplate != 0;theTemplate=m_pTemplateList->next()) {
		m_pTemplateLB->insertItem( theTemplate->getName() );
		connect( theTemplate, SIGNAL(modified()),this,SLOT(slotTemplateRenamed()));
	}

	enableWidgets(false);//disable widgets until an att is chosen
	m_pOldTemplate = 0;
	connect( m_pTemplateLB, SIGNAL(clicked(QListBoxItem*)), this, SLOT(slotClicked(QListBoxItem*)) );

	connect( m_pTemplateLB, SIGNAL(rightButtonPressed(QListBoxItem*, const QPoint&)),
	         this, SLOT(slotRightButtonPressed(QListBoxItem*, const QPoint&)) );

	connect( m_pTemplateLB, SIGNAL(rightButtonClicked(QListBoxItem*, const QPoint&)),
	         this, SLOT(slotRightButtonClicked(QListBoxItem*, const QPoint&)) );
	connect( m_pDoc, SIGNAL(sigObjectCreated(UMLObject*)),
		 this, SLOT(slotTemplateCreated(UMLObject*)) );

	connect( m_pUpArrowB, SIGNAL( clicked() ), this, SLOT( slotUpClicked() ) );
	connect( m_pDownArrowB, SIGNAL( clicked() ), this, SLOT( slotDownClicked() ) );
	connect( m_pTemplateLB, SIGNAL( doubleClicked( QListBoxItem* ) ),
		 this, SLOT( slotDoubleClick( QListBoxItem* ) ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassTemplatePage::~ClassTemplatePage() {

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassTemplatePage::enableWidgets(bool state) {
	m_pDocTE->setEnabled(state);
	//if disabled clear contents
	if (!state) {
		m_pDocTE->setText( "" );
		m_pUpArrowB->setEnabled( false );
		m_pDownArrowB->setEnabled( false );
		m_pDeleteTemplateButton->setEnabled(false);
		m_pPropertiesButton->setEnabled(false);
		return;
	}
	/*now check the order buttons.
		Double check an item is selected
	   If only one att. in list make sure there disabled.
		If at top item,only allow down arrow to be enabled.
		If at bottom item. only allow up arrow to be enabled.
	*/
	int index = m_pTemplateLB->currentItem();
	if (m_pTemplateLB->count() == 1 || index == -1) {
		m_pUpArrowB->setEnabled(false);
		m_pDownArrowB->setEnabled(false);
	} else if (index == 0) {
		m_pUpArrowB->setEnabled(false);
		m_pDownArrowB->setEnabled(true);
	} else if ( index == (int)m_pTemplateLB->count() - 1 ) {
		m_pUpArrowB->setEnabled(true);
		m_pDownArrowB->setEnabled(false);
	} else {
		m_pUpArrowB->setEnabled(true);
		m_pDownArrowB->setEnabled(true);
	}
	m_pDeleteTemplateButton->setEnabled(true);
	m_pPropertiesButton->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassTemplatePage::slotClicked(QListBoxItem* item) {
	//make sure clicked on an item
	if (!item) {
		enableWidgets(false);
		m_pOldTemplate = 0;
		m_pTemplateLB->clearSelection();
		return;
	}

	QString name = m_pTemplateLB->currentText();
	UMLTemplate* pTemplate = m_pTemplateList->at( m_pTemplateLB->index(item) );
	//if not first time an item is highlighted
	//save old highlighted item first
	if(m_pOldTemplate) {
		m_pOldTemplate->setDoc( m_pDocTE->text() );
	}
	//now update screen
	m_pDocTE->setText( pTemplate->getDoc() );
	enableWidgets(true);
	m_pOldTemplate = pTemplate;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassTemplatePage::updateObject() {
	QListBoxItem* item = m_pTemplateLB->item( m_pTemplateLB->currentItem() );
	slotClicked(item);
	QStringList stringList;
	int count = m_pTemplateLB->count();
	for (int i = 0; i < count; i++) {
		stringList.append( m_pTemplateLB->text(i) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassTemplatePage::slotTemplateCreated(UMLObject* object) {
	if (!m_bSigWaiting) {
		return;
	}
	if( object->getBaseType() != Uml::ot_Template ||
	    object->parent() != m_pClass )
	{
		return;
	}
	int index = m_pTemplateLB->count();
	m_pTemplateLB->insertItem(object->getName(), index);
	connect(object,SIGNAL(modified()),this,SLOT(slotTemplateRenamed()));
	m_bSigWaiting = false;
	slotClicked(0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassTemplatePage::slotTemplateRenamed( ) {
	if (!m_bSigWaiting) {
		return;
	}
	const UMLObject *object = dynamic_cast<const UMLObject*>(sender());
	if( object->getBaseType() != Uml::ot_Template )
	{
		return;
	}
	int index = m_pTemplateLB->currentItem();
	m_pTemplateLB->changeItem(object->getName(), index);
	m_bSigWaiting = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassTemplatePage::slotRightButtonClicked(QListBoxItem * /*item*/, const QPoint &/* p*/) {
	if (m_pMenu) {
		m_pMenu->hide();
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassTemplatePage::slotRightButtonPressed(QListBoxItem* item, const QPoint& p) {
	ListPopupMenu::Menu_Type type = ListPopupMenu::mt_Undefined;
	if (item) { //pressed on an item
		type = ListPopupMenu::mt_Template_Selected;
	} else { //pressed into fresh air
		type = ListPopupMenu::mt_New_Template;
	}
	if(m_pMenu) {
		m_pMenu->hide();
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
	m_pMenu = new ListPopupMenu(this, type);

	m_pMenu->popup(p);
	connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassTemplatePage::slotPopupMenuSel(int id) {
	UMLTemplate* theTemplate = m_pTemplateList->at( m_pTemplateLB->currentItem() );
	if(!theTemplate && id != ListPopupMenu::mt_New_Attribute) {
		kdWarning() << "can't find template from selection" << endl;
		return;
	}
	switch(id) {
		case ListPopupMenu::mt_New_Template:
			slotNewTemplate();
			break;

		case ListPopupMenu::mt_Delete:
			slotDelete();
			break;

		case ListPopupMenu::mt_Rename:
			m_bSigWaiting = true;
			m_pDoc->renameChildUMLObject(theTemplate);
			break;

		case ListPopupMenu::mt_Properties:
			slotProperties();
			break;
	}
}

void ClassTemplatePage::slotUpClicked() {
	int count = m_pTemplateLB->count();
	int index = m_pTemplateLB->currentItem();
	//shouldn't occur, but just in case
	if( count <= 1 || index <= 0 ) {
		return;
	}

	//swap the text around ( meaning attributes )
	QString aboveString = m_pTemplateLB->text( index - 1 );
	QString currentString = m_pTemplateLB->text( index );
	m_pTemplateLB->changeItem( currentString, index -1 );
	m_pTemplateLB->changeItem( aboveString, index );
	//set the moved atttribute selected
	QListBoxItem* item = m_pTemplateLB->item( index - 1 );
	m_pTemplateLB->setSelected( item, true );
	//now change around in the list
	UMLTemplate* aboveTemplate = m_pTemplateList->at( index - 1 );
	UMLTemplate* currentTemplate = m_pTemplateList->take( index );
	m_pTemplateList->insert( m_pTemplateList->findRef( aboveTemplate ), currentTemplate );
	slotClicked( item );
}

void ClassTemplatePage::slotDownClicked() {
	int count = m_pTemplateLB->count();
	int index = m_pTemplateLB->currentItem();
	//shouldn't occur, but just in case
	if( count <= 1 || index >= count - 1 ) {
		return;
	}

	//swap the text around ( meaning attributes )
	QString belowString = m_pTemplateLB->text( index + 1 );
	QString currentString = m_pTemplateLB->text( index );
	m_pTemplateLB->changeItem( currentString, index + 1 );
	m_pTemplateLB->changeItem( belowString, index );
	//set the moved atttribute selected
	QListBoxItem* item = m_pTemplateLB->item( index + 1 );
	m_pTemplateLB->setSelected( item, true );
	//now change around in the list
	UMLTemplate* aboveTemplate = m_pTemplateList->at( index + 1 );
	UMLTemplate* currentTemplate = m_pTemplateList->take( index );
	m_pTemplateList->insert( m_pTemplateList->findRef( aboveTemplate ) + 1, currentTemplate );
	slotClicked( item );
}

void ClassTemplatePage::slotDoubleClick(QListBoxItem* item) {
	if (!item) {
		return;
	}
	QString name = item->text();
	UMLTemplate* pTemplate  = m_pTemplateList->at( m_pTemplateLB->index(item) );
	if( !pTemplate ) {
		kdWarning() << "can't find template from selection" << endl;
		return;
	}
	UMLTemplateDialog dialogue(this, pTemplate);
	if ( dialogue.exec() ) {
		m_pTemplateLB->changeItem( pTemplate->getName(), m_pTemplateLB->index(item) );
//FIXME		m_pDoc->signalChildUMLObjectUpdate(pTemplate);
	}
}

void ClassTemplatePage::slotDelete() {
	UMLTemplate* selectedTemplate = m_pTemplateList->at( m_pTemplateLB->currentItem() );
	//should really wait for signal back
	//but really shouldn't matter
	m_pDoc->removeUMLObject(selectedTemplate);
	m_pTemplateLB->removeItem( m_pTemplateLB->currentItem() );
	slotClicked(0);
}

void ClassTemplatePage::slotProperties() {
	slotDoubleClick( m_pTemplateLB->item( m_pTemplateLB->currentItem() ) );
}

void ClassTemplatePage::slotNewTemplate() {
	m_bSigWaiting = true;
	m_pDoc->createUMLObject(m_pClass, Uml::ot_Template);
}

#include "classtemplatepage.moc"
