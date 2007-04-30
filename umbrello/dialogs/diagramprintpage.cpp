/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "diagramprintpage.h"

// qt/kde includes
#include <qlayout.h>
#include <qptrlist.h>
#include <qlistbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <klocale.h>

// local includes
#include "../uml.h"
#include "../umldoc.h"
#include "../umlview.h"
#include "../umlviewlist.h"
#include "../umlnamespace.h"


DiagramPrintPage::DiagramPrintPage(QWidget * parent, UMLDoc * m_pDoc) : KPrintDialogPage(parent), m_pDoc(m_pDoc) {
    int margin = fontMetrics().height();
    setTitle(i18n("&Diagrams"));
    QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout -> setSpacing(10);
    mainLayout -> setMargin(margin);

    m_pFilterBG = new QButtonGroup(i18n("Filter"), this);
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

    m_pSelectGB = new QGroupBox(i18n("Selection"), this);
    mainLayout -> addWidget(m_pSelectGB);

    QVBoxLayout * select = new QVBoxLayout(m_pSelectGB);
    select -> setSpacing(10);
    select-> setMargin(margin);

    m_pTypeCB = new QComboBox(m_pSelectGB);
    select -> addWidget(m_pTypeCB);
    m_pTypeCB -> setEnabled(false);

    m_pSelectLB = new QListBox(m_pSelectGB);
    select -> addWidget(m_pSelectLB);
    m_pSelectLB -> setEnabled(false);
    m_pSelectLB -> setSelectionMode(QListBox::Multi);
    m_pSelectLB -> insertItem(UMLApp::app()->getCurrentView()->getName());
    m_pSelectLB -> setSelected(0, true);
    m_nIdList.clear();
    m_nIdList.append(UMLApp::app()->getCurrentView()->getID());



    m_ViewType = Uml::dt_Class;
    connect(m_pFilterBG, SIGNAL(clicked(int)), this, SLOT(slotClicked(int)));
    connect(m_pTypeCB, SIGNAL(activated(const QString&)), this, SLOT(slotActivated(const QString&)));

    m_pTypeCB -> insertItem(i18n("Class"));
    m_pTypeCB -> insertItem(i18n("Use Case"));
    m_pTypeCB -> insertItem(i18n("Collaboration"));
    m_pTypeCB -> insertItem(i18n("Sequence"));
    m_pTypeCB -> insertItem(i18n("State"));
    m_pTypeCB -> insertItem(i18n("Activity"));
    m_pTypeCB -> insertItem(i18n("Component"));
    m_pTypeCB -> insertItem(i18n("Deployment"));
}

DiagramPrintPage::~DiagramPrintPage()
{
    disconnect(m_pFilterBG, SIGNAL(clicked(int)), this, SLOT(slotClicked(int)));
    disconnect(m_pTypeCB, SIGNAL(activated(const QString&)), this, SLOT(slotActivated(const QString&)));
}

void DiagramPrintPage::getOptions( QMap<QString,QString>& opts, bool /*incldef = false*/ ) {
    int listCount = m_pSelectLB -> count();
    int count = 0;

    QString diagram(i18n("kde-uml-Diagram"));
    for(int     i=0;i<listCount;i++) {
        if(m_pSelectLB -> isSelected(i)) {
            UMLView *view = (UMLView *)m_pDoc -> findView(m_nIdList[i]);
            QString sCount = QString("%1").arg(count);
            QString sID = QString("%1").arg(ID2STR(view -> getID()));
            opts.insert(diagram + sCount, sID);
            count++;
        }
    }
    opts.insert("kde-uml-count", QString("%1").arg(count));
}

void DiagramPrintPage::setOptions( const QMap<QString,QString>& /*opts*/ ) {}

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
    UMLView * view = 0;
    QString type;

    // clear list with diagrams to print
    m_nIdList.clear();

    switch(id) {
    case Current:
        m_pTypeCB -> setEnabled(false);
        m_pSelectLB -> setEnabled(false);
        m_pSelectLB -> clear();
        m_pSelectLB -> insertItem(UMLApp::app()->getCurrentView()->getName());
        m_pSelectLB -> setSelected(0, true);
        m_nIdList.append(UMLApp::app()->getCurrentView()->getID());
        break;

    case All:

        m_pTypeCB -> setEnabled(false);
        m_pSelectLB -> setEnabled(false);
        m_pSelectLB -> clear();
        for(view = list.first(); view; view = list.next()) {
            m_pSelectLB -> insertItem(view -> getName());
            m_nIdList.append(view -> getID());
        }
        m_pSelectLB -> selectAll(true);
        break;

    case Select:
        m_pTypeCB -> setEnabled(false);
        m_pSelectLB -> setEnabled(true);
        m_pSelectLB -> clear();
        for(view = list.first(); view; view = list.next()) {
            m_pSelectLB -> insertItem(view -> getName());
            m_nIdList.append(view -> getID());
        }
        break;

    case Type:
        m_pTypeCB -> setEnabled(true);
        m_pSelectLB -> setEnabled(true);
        m_pSelectLB -> clear();
        for(view = list.first(); view; view = list.next()) {
            if(view -> getType() == m_ViewType) {
                m_pSelectLB -> insertItem(view -> getName());
                m_nIdList.append(view -> getID());
            }
        }
        m_pSelectLB -> selectAll(true);
        break;
    }
}

void DiagramPrintPage::slotActivated(const QString & text) {
    UMLViewList list = m_pDoc -> getViewIterator();
    UMLView * view = 0;

    if(text == i18n("Class"))
        m_ViewType = Uml::dt_Class;
    else if(text == i18n("Sequence"))
        m_ViewType = Uml::dt_Sequence;
    else if(text == i18n("Use Case"))
        m_ViewType = Uml::dt_UseCase;
    else if(text == i18n("Collaboration"))
        m_ViewType = Uml::dt_Collaboration;
    else if(text == i18n("State"))
        m_ViewType = Uml::dt_State;
    else if(text == i18n("Activity"))
        m_ViewType = Uml::dt_Activity;
    else if(text == i18n("Component"))
        m_ViewType = Uml::dt_Component;
    else if(text == i18n("Deployment"))
        m_ViewType = Uml::dt_Deployment;
    m_pSelectLB -> clear();
    m_nIdList.clear();
    for(view = list.first(); view; view = list.next()) {
        if(view -> getType() == m_ViewType) {
            m_pSelectLB -> insertItem(view -> getName());
            m_nIdList.append(view -> getID());
        }
    }
    m_pSelectLB -> selectAll(true);
}

#include "diagramprintpage.moc"
