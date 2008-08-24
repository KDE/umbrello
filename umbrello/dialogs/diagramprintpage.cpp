/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "diagramprintpage.h"

// local includes
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlviewlist.h"
#include "umlnamespace.h"
#include "model_utils.h"

// kde includes
#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>

// qt includes
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QListWidget>
#include <QtGui/QRadioButton>
#include <QtGui/QGroupBox>
#include "umlscene.h"

DiagramPrintPage::DiagramPrintPage(QWidget * parent, UMLDoc * m_pDoc) : QWidget(parent), m_pDoc(m_pDoc)
{
    int margin = fontMetrics().height();
    setWindowTitle(i18n("&Diagrams"));
    QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setMargin(margin);

    m_pFilterGB = new QGroupBox(i18n("Filter"), this);
    mainLayout->addWidget(m_pFilterGB);

    QVBoxLayout * filter = new QVBoxLayout(m_pFilterGB);
    filter->setSpacing(10);
    filter->setMargin(margin);

    m_pCurrentRB = new QRadioButton(i18n("&Current diagram"), m_pFilterGB);
    filter->addWidget(m_pCurrentRB);
    m_pCurrentRB->setChecked(true);

    m_pAllRB = new QRadioButton(i18n("&All diagrams"), m_pFilterGB);
    filter->addWidget(m_pAllRB);

    m_pSelectRB = new QRadioButton(i18n("&Select diagrams"), m_pFilterGB);
    filter->addWidget(m_pSelectRB);

    m_pTypeRB = new QRadioButton(i18n("&Type of diagram"), m_pFilterGB);
    filter->addWidget(m_pTypeRB);

    m_pSelectGB = new QGroupBox(i18nc("diagram selection for printing", "Selection"), this);
    mainLayout->addWidget(m_pSelectGB);

    QVBoxLayout * select = new QVBoxLayout(m_pSelectGB);
    select->setSpacing(10);
    select->setMargin(margin);

    m_pTypeCB = new KComboBox(m_pSelectGB);
    select->addWidget(m_pTypeCB);
    m_pTypeCB->setEnabled(false);

    m_pSelectLW = new QListWidget(m_pSelectGB);
    select->addWidget(m_pSelectLW);
    m_pSelectLW->setEnabled(false);
    m_pSelectLW->setSelectionMode(QAbstractItemView::MultiSelection);
    m_pSelectLW->addItem(UMLApp::app()->getCurrentView()->umlScene()->getName());
    m_pSelectLW->setCurrentRow(0);
    m_nIdList.clear();
    m_nIdList.append(UMLApp::app()->getCurrentView()->umlScene()->getID());

    m_ViewType = Uml::dt_Class;
    connect(m_pAllRB, SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(m_pCurrentRB, SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(m_pSelectRB, SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(m_pTypeRB, SIGNAL(clicked()), this, SLOT(slotClicked()));

    connect(m_pTypeCB, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));

    QStringList types;
    // diagramNo 1 is Uml::dt_Class
    // digaramNo 9 is Uml::dt_EntityRelationship
    for (int diagramNo = 1; diagramNo < Uml::N_DIAGRAMTYPES; diagramNo++) {
        types << Model_Utils::diagramTypeToString( ( Uml::Diagram_Type )diagramNo ) ;
    }

    m_pTypeCB->insertItems(0, types);
}

DiagramPrintPage::~DiagramPrintPage()
{
}

int DiagramPrintPage::printUmlCount()
{
    QList<QListWidgetItem *> selectedItems = m_pSelectLW->selectedItems();
    return selectedItems.count();
}

QString DiagramPrintPage::printUmlDiagram(int sel)
{
    int count = 0;

    for (int i = 0; i < m_pSelectLW->count(); i++) {
        if (isSelected(i)) {
            if (count == sel) {
                UMLView *view = (UMLView *)m_pDoc->findView(m_nIdList[i]);
                QString sID = QString("%1").arg(ID2STR(view->umlScene()->getID()));
                return sID;
            }
            count++;
        }
    }
    return QString();
}

bool DiagramPrintPage::isValid( QString& msg )
{
    int listCount = m_pSelectLW->count();
    bool sel = false;
    for(int i = 0; i < listCount; i++) {
        if (isSelected(i)) {
            sel = true;
            i = listCount;
        }
    }
    msg = i18n("No diagrams selected.");
    return sel;
}

bool DiagramPrintPage::isSelected(int index)
{
    QList<QListWidgetItem *> selectedItems = m_pSelectLW->selectedItems();
    QListWidgetItem* itemAtIndex = m_pSelectLW->item(index);
    if (selectedItems.contains(itemAtIndex)) {
        return true;
    }
    else {
        return false;
    }
}

void DiagramPrintPage::slotClicked()
{
    UMLViewList list = m_pDoc->getViewIterator();
    QString type;

    // clear list with diagrams to print
    m_nIdList.clear();

    UMLScene *currentScene = UMLApp::app()->getCurrentView()->umlScene();
    if (m_pCurrentRB->isChecked()) {
        m_pTypeCB->setEnabled(false);
        m_pSelectLW->setEnabled(false);
        m_pSelectLW->clear();
        m_pSelectLW->addItem(currentScene->getName());
        m_pSelectLW->setCurrentRow(0);
        m_nIdList.append(currentScene->getID());
    }

    if (m_pAllRB->isChecked()) {
        m_pTypeCB->setEnabled(false);
        m_pSelectLW->setEnabled(false);
        m_pSelectLW->clear();
        foreach ( UMLView * view , list ) {
            m_pSelectLW->addItem(view->umlScene()->getName());
            m_nIdList.append(view->umlScene()->getID());
        }
        m_pSelectLW->selectAll();
    }

    if (m_pSelectRB->isChecked()) {
        m_pTypeCB->setEnabled(false);
        m_pSelectLW->setEnabled(true);
        m_pSelectLW->clear();
        foreach ( UMLView * view , list) {
            m_pSelectLW->addItem(view->umlScene()->getName());
            m_nIdList.append(view->umlScene()->getID());
        }
    }

    if (m_pTypeRB->isChecked()) {
        m_pTypeCB->setEnabled(true);
        m_pSelectLW->setEnabled(true);
        m_pSelectLW->clear();
        foreach ( UMLView * view , list) {
            if(view->umlScene()->getType() == m_ViewType) {
                m_pSelectLW->addItem(view->umlScene()->getName());
                m_nIdList.append(view->umlScene()->getID());
            }
        }
        m_pSelectLW->selectAll();
    }
}

void DiagramPrintPage::slotActivated(int index)
{
    UMLViewList list = m_pDoc->getViewIterator();

    // combo box entries start from 0 index
    // valid diagram_type enum values start from 1
    m_ViewType = ( Uml::Diagram_Type )( index + 1 );

    m_pSelectLW->clear();

    m_nIdList.clear();
    foreach (UMLView * view , list) {
        if (view->umlScene()->getType() == m_ViewType) {
            m_pSelectLW->addItem(view->umlScene()->getName());
            m_nIdList.append(view->umlScene()->getID());
        }
    }
    m_pSelectLW->selectAll();
}

#include "diagramprintpage.moc"
