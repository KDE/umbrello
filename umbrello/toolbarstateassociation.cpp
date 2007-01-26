/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstateassociation.h"

// kde includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

// app includes
#include "assocrules.h"
#include "association.h"
#include "associationwidget.h"
#include "classifierwidget.h"
#include "folder.h"
#include "model_utils.h"
#include "uml.h"
#include "umlobject.h"
#include "umlview.h"
#include "umllistview.h"
#include "umldoc.h"
#include "umlwidget.h"

using namespace Uml;

ToolBarStateAssociation::ToolBarStateAssociation(UMLView *umlView) : ToolBarStatePool(umlView) {
    m_firstWidget = 0;
    m_associationLine = 0;
}

ToolBarStateAssociation::~ToolBarStateAssociation() {
    delete m_associationLine;
}

void ToolBarStateAssociation::init() {
    ToolBarStatePool::init();

    cleanAssociation();
}

void ToolBarStateAssociation::cleanBeforeChange() {
    ToolBarStatePool::cleanBeforeChange();

    cleanAssociation();
}

void ToolBarStateAssociation::mouseMove(QMouseEvent* ome) {
    ToolBarStatePool::mouseMove(ome);

    if (m_associationLine) {
        QPoint sp = m_associationLine->startPoint();
        m_associationLine->setPoints(sp.x(), sp.y(), m_pMouseEvent->x(), m_pMouseEvent->y());
    }
}

void ToolBarStateAssociation::slotWidgetRemoved(UMLWidget* widget) {
    ToolBarState::slotWidgetRemoved(widget);

    if (widget == m_firstWidget) {
        cleanAssociation();
    }
}

void ToolBarStateAssociation::mouseReleaseAssociation() {
    if (m_pMouseEvent->button() != Qt::LeftButton ||
            !m_firstWidget || m_firstWidget->getBaseType() != Uml::wt_Class) {
        cleanAssociation();
        return;
    }

    getCurrentAssociation()->createAssocClassLine(
            static_cast<ClassifierWidget*>(m_firstWidget),
            getCurrentAssociation()->getLinePath()->onLinePath(m_pMouseEvent->pos()));
    cleanAssociation();
}

void ToolBarStateAssociation::mouseReleaseWidget() {
    if (m_pMouseEvent->button() != Qt::LeftButton) {
        cleanAssociation();
        return;
    }

    // TODO In old code in ToolBarState there was a TODO that said: Should not
    //be called by a Sequence message Association. Here's the check for that,
    //although I don't know why it is needed, but it seems that it's not needed,
    //as the old code worked fine without it...
    if (getAssociationType() == at_Seq_Message) {
        return;
    }

    if (!m_firstWidget) {
        setFirstWidget();
    } else {
        setSecondWidget();
    }
}

void ToolBarStateAssociation::mouseReleaseEmpty() {
    cleanAssociation();
}

void ToolBarStateAssociation::setFirstWidget() {
    UMLWidget* widget = getCurrentWidget();
    Association_Type type = getAssociationType();

    if (!AssocRules::allowAssociation(type, widget)) {
        //TODO improve error feedback: tell the user what are the valid type of associations for
        //that widget
        KMessageBox::error(0, i18n("Incorrect use of associations."), i18n("Association Error"));
        return;
    }
    //set up position
    QPoint pos;
    pos.setX(widget->getX() + (widget->getWidth() / 2));
    pos.setY(widget->getY() + (widget->getHeight() / 2));
    //TODO why is this needed?
    m_pUMLView->setPos(pos);

    m_firstWidget = widget;

    m_associationLine = new QCanvasLine(m_pUMLView->canvas());
    m_associationLine->setPoints(pos.x(), pos.y(), pos.x(), pos.y());
    m_associationLine->setPen(QPen(m_pUMLView->getLineColor(), m_pUMLView->getLineWidth(), Qt::DashLine));

    m_associationLine->setVisible(true);

    m_pUMLView->viewport()->setMouseTracking(true);
}

void ToolBarStateAssociation::setSecondWidget() {
    Association_Type type = getAssociationType();
    UMLWidget* widgetA = m_firstWidget;
    UMLWidget* widgetB = getCurrentWidget();
    Widget_Type at = widgetA->getBaseType();
    bool valid = true;
    if (type == at_Generalization) {
        type = AssocRules::isGeneralisationOrRealisation(widgetA, widgetB);
    }
    if (widgetA == widgetB) {
        valid = AssocRules::allowSelf(type, at);
        if (valid && type == at_Association) {
            type = at_Association_Self;
        }
    } else {
        valid = AssocRules::allowAssociation(type, widgetA, widgetB);
    }
    if (valid) {
        AssociationWidget *temp = new AssociationWidget(m_pUMLView, widgetA, type, widgetB);
        addAssociationInViewAndDoc(temp);
        if (type == at_Containment) {
            UMLListView *lv = UMLApp::app()->getListView();
            UMLObject *newContainer = widgetA->getUMLObject();
            UMLObject *objToBeMoved = widgetB->getUMLObject();
            if (newContainer && objToBeMoved) {
                UMLListViewItem *newLVParent = lv->findUMLObject(newContainer);
                lv->moveObject(objToBeMoved->getID(),
                               Model_Utils::convert_OT_LVT(objToBeMoved),
                               newLVParent);
            }
        }
        UMLApp::app()->getDocument()->setModified();
    } else {
        //TODO improve error feedback: tell the user what are the valid type of associations for
        //the second widget using the first widget
        KMessageBox::error(0, i18n("Incorrect use of associations."), i18n("Association Error"));
    }

    cleanAssociation();
}

Association_Type ToolBarStateAssociation::getAssociationType() {
    Association_Type at;

    switch(getButton()) {
        case WorkToolBar::tbb_Anchor:                   at = at_Anchor;            break;
        case WorkToolBar::tbb_Association:              at = at_Association;       break;
        case WorkToolBar::tbb_UniAssociation:           at = at_UniAssociation;    break;
        case WorkToolBar::tbb_Generalization:           at = at_Generalization;    break;
        case WorkToolBar::tbb_Composition:              at = at_Composition;       break;
        case WorkToolBar::tbb_Aggregation:              at = at_Aggregation;       break;
        case WorkToolBar::tbb_Relationship:             at = at_Relationship;      break;
        case WorkToolBar::tbb_Dependency:               at = at_Dependency;        break;
        case WorkToolBar::tbb_Containment:              at = at_Containment;       break;
        case WorkToolBar::tbb_Seq_Message_Synchronous:
        case WorkToolBar::tbb_Seq_Message_Asynchronous: at = at_Seq_Message;       break;
        case WorkToolBar::tbb_Coll_Message:             at = at_Coll_Message;      break;
        case WorkToolBar::tbb_State_Transition:         at = at_State;             break;
        case WorkToolBar::tbb_Activity_Transition:      at = at_Activity;          break;

        default:                                        at = at_Unknown;           break;
    }

    return at;
}

void ToolBarStateAssociation::addAssociationInViewAndDoc(AssociationWidget* a) {
    // append in view
    if (m_pUMLView->addAssociation(a, false)) {
        // if view went ok, then append in document
        UMLAssociation *umla = a->getAssociation();
        if (umla == NULL) {
            // association without model representation in UMLDoc
            return;
        }
        Uml::Model_Type m = Model_Utils::convert_DT_MT(m_pUMLView->getType());
        UMLDoc *umldoc = UMLApp::app()->getDocument();
        umla->setUMLPackage(umldoc->getRootFolder(m));
        UMLApp::app()->getDocument()->addAssociation(umla);
    } else {
        kError() << "cannot addAssocInViewAndDoc(), deleting" << endl;
        delete a;
    }
}

void ToolBarStateAssociation::cleanAssociation() {
    m_firstWidget = 0;

    delete m_associationLine;
    m_associationLine = 0;
}

#include "toolbarstateassociation.moc"
