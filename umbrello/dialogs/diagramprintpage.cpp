/***************************************************************************
 *                                                                         *
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

// qt/kde includes
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLayout>
#include <q3listbox.h>
#include <QtGui/QRadioButton>
#include <q3buttongroup.h>
#include <q3groupbox.h>

#include <kcombobox.h>
#include <klocale.h>

// local includes
#include "../uml.h"
#include "../umldoc.h"
#include "../umlview.h"
#include "../umlviewlist.h"
#include "../umlnamespace.h"
#include "../model_utils.h"
#include "umlscene.h"

DiagramPrintPage::DiagramPrintPage(QWidget * parent, UMLDoc * m_pDoc) : QWidget(parent), m_pDoc(m_pDoc) {
    int margin = fontMetrics().height();
    setWindowTitle(i18n("&Diagrams"));
    QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout -> setSpacing(10);
    mainLayout -> setMargin(margin);

    m_pFilterBG = new Q3ButtonGroup(i18n("Filter"), this);
    mainLayout -> addWidget(m_pFilterBG);
    m_pFilterBG -> setExclusive(true);

    QVBoxLayout * filter = new QVBoxLayout(m_pFilterBG);
    filter -> setSpacing(10);
    filter-> setMargin(margin);

    m_pCurrentRB = new QRadioButton(i18n("&Current diagram"), m_pFilterBG);
    filter -> addWidget(m_pCurrentRB);
    m_pCurrentRB -> setChecked(true);
    m_pFilterBG -> insert(m_pCurrentRB, Current);

    m_pAllRB = new QRadioButton(i18n("&All diagrams"), m_pFilterBG);
    filter -> addWidget(m_pAllRB);
    m_pFilterBG -> insert(m_pAllRB, All);

    m_pSelectRB = new QRadioButton(i18n("&Select diagrams"), m_pFilterBG);
    filter -> addWidget(m_pSelectRB);
    m_pFilterBG -> insert(m_pSelectRB, Select);

    m_pTypeRB = new QRadioButton(i18n("&Type of diagram"), m_pFilterBG);
    filter -> addWidget(m_pTypeRB);
    m_pFilterBG -> insert(m_pTypeRB, Type);

    m_pSelectGB = new Q3GroupBox(i18nc("diagram selection for printing", "Selection"), this);
    mainLayout -> addWidget(m_pSelectGB);

    QVBoxLayout * select = new QVBoxLayout(m_pSelectGB);
    select -> setSpacing(10);
    select-> setMargin(margin);

    m_pTypeCB = new KComboBox(m_pSelectGB);
    select -> addWidget(m_pTypeCB);
    m_pTypeCB -> setEnabled(false);

    m_pSelectLB = new Q3ListBox(m_pSelectGB);
    select -> addWidget(m_pSelectLB);
    m_pSelectLB -> setEnabled(false);
    m_pSelectLB -> setSelectionMode(Q3ListBox::Multi);
    m_pSelectLB -> insertItem(UMLApp::app()->getCurrentView()->umlScene()->getName());
    m_pSelectLB -> setSelected(0, true);
    m_nIdList.clear();
    m_nIdList.append(UMLApp::app()->getCurrentView()->umlScene()->getID());



    m_ViewType = Uml::dt_Class;
    connect(m_pFilterBG, SIGNAL(clicked(int)), this, SLOT(slotClicked(int)));
    connect(m_pTypeCB, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));

    QStringList types;
    // diagramNo 1 is Uml::dt_Class
    // digaramNo 9 is Uml::dt_EntityRelationship
    for (int diagramNo=1; diagramNo < 10; diagramNo++) {
        types<< Model_Utils::diagramTypeToString( ( Uml::Diagram_Type )diagramNo ) ;
    }

    m_pTypeCB -> insertItems(0, types);
}

DiagramPrintPage::~DiagramPrintPage()
{
}

int DiagramPrintPage::printUmlCount() {
    int listCount = m_pSelectLB -> count();
    int count = 0;

    for(int     i=0;i<listCount;i++) {
        if(m_pSelectLB -> isSelected(i)) {
            count++;
        }
    }
    return count;
}

QString DiagramPrintPage::printUmlDiagram(int sel){
    int listCount = m_pSelectLB -> count();
    int count = 0;

    for(int i = 0; i < listCount; i++) {
        if(m_pSelectLB -> isSelected(i)) {
            if(count==sel) {
                UMLView *view = (UMLView *)m_pDoc -> findView(m_nIdList[i]);
                QString sID = QString("%1").arg(ID2STR(view->umlScene()->getID()));
                return sID;
            }
            count++;
        }
    }
    return QString();
}

bool DiagramPrintPage::isValid( QString& msg ) {
    int listCount = m_pSelectLB -> count();
    bool sel = false;
    for(int i =0;i<listCount;i++) {
        if(m_pSelectLB -> isSelected(i)) {
            sel = true;
            i = listCount;
        }
    }
    msg = i18n("No diagrams selected.");
    return sel;
}

void DiagramPrintPage::slotClicked(int id) {
    UMLViewList list = m_pDoc -> getViewIterator();
    QString type;

    // clear list with diagrams to print
    m_nIdList.clear();

    UMLScene *currentScene = UMLApp::app()->getCurrentView()->umlScene();
    switch(id) {
    case Current:
        m_pTypeCB -> setEnabled(false);
        m_pSelectLB -> setEnabled(false);
        m_pSelectLB -> clear();
        m_pSelectLB -> insertItem(currentScene->getName());
        m_pSelectLB -> setSelected(0, true);
        m_nIdList.append(currentScene->getID());
        break;

    case All:

        m_pTypeCB -> setEnabled(false);
        m_pSelectLB -> setEnabled(false);
        m_pSelectLB -> clear();
        foreach ( UMLView * view , list ) {
            m_pSelectLB -> insertItem(view->umlScene()->getName());
            m_nIdList.append(view->umlScene()->getID());
        }
        m_pSelectLB -> selectAll(true);
        break;

    case Select:
        m_pTypeCB -> setEnabled(false);
        m_pSelectLB -> setEnabled(true);
        m_pSelectLB -> clear();
        foreach ( UMLView * view , list) {
            m_pSelectLB -> insertItem(view->umlScene()->getName());
            m_nIdList.append(view->umlScene()->getID());
        }
        break;

    case Type:
        m_pTypeCB -> setEnabled(true);
        m_pSelectLB -> setEnabled(true);
        m_pSelectLB -> clear();
        foreach ( UMLView * view , list) {
            if(view->umlScene()->getType() == m_ViewType) {
                m_pSelectLB -> insertItem(view->umlScene()->getName());
                m_nIdList.append(view->umlScene()->getID());
            }
        }
        m_pSelectLB -> selectAll(true);
        break;
    }
}

void DiagramPrintPage::slotActivated(int index) {
    UMLViewList list = m_pDoc -> getViewIterator();

    // combo box entries start from 0 index
    // valid diagram_type enum values start from 1
    m_ViewType = ( Uml::Diagram_Type )( index + 1 );

    m_pSelectLB -> clear();

    m_nIdList.clear();
    foreach ( UMLView * view , list) {
        if(view->umlScene()->getType() == m_ViewType) {
            m_pSelectLB -> insertItem(view->umlScene()->getName());
            m_nIdList.append(view->umlScene()->getID());
        }
    }
    m_pSelectLB -> selectAll(true);
}

#include "diagramprintpage.moc"
