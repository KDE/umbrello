/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2008                                               *
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
#include "umlscene.h"
#include "umllistview.h"
#include "umldoc.h"
#include "umlwidget.h"

// qt includes
#include <QtGui/QGraphicsSceneMouseEvent>

using namespace Uml;

ToolBarStateAssociation::ToolBarStateAssociation(UMLScene *umlScene) :
    ToolBarStatePool(umlScene)
{
    m_firstWidget = 0;
    m_associationLine = 0;
}

ToolBarStateAssociation::~ToolBarStateAssociation()
{
    delete m_associationLine;
}

/**
 * Goes back to the initial state.
 */
void ToolBarStateAssociation::init()
{
    ToolBarStatePool::init();

    cleanAssociation();
}

/**
 * Called when the current tool is changed to use another tool.
 * Executes base method and cleans the association.
 */
void ToolBarStateAssociation::cleanBeforeChange()
{
    ToolBarStatePool::cleanBeforeChange();

    cleanAssociation();
}

/**
 * Called when a mouse event happened.
 * It executes the base method and then updates the position of the
 * association line, if any.
 */
void ToolBarStateAssociation::mouseMove(QGraphicsSceneMouseEvent* ome)
{
    ToolBarStatePool::mouseMove(ome);

    if (m_associationLine) {
        QPointF sp = m_associationLine->line().p1();
        qreal x = m_pMouseEvent->scenePos().x(), y = m_pMouseEvent->scenePos().y();
        m_associationLine->setLine(sp.x(), sp.y(), x, y);
    }
}

/**
 * A widget was removed from the UMLScene.
 * If the widget removed was the current widget, the current widget is set
 * to 0.
 * Also, if it was the first widget, the association is cleaned.
 */
void ToolBarStateAssociation::slotWidgetRemoved(UMLRectWidget* widget)
{
    ToolBarState::slotWidgetRemoved(widget);

    if (widget == m_firstWidget) {
        cleanAssociation();
    }
}

/**
 * Called when the release event happened on an association.
 * If the button pressed isn't left button, the association being created is
 * cleaned. If it is left button, and the first widget is set and is a
 * classifier widget, it creates an association class. Otherwise, the
 * association being created is cleaned.
 */
void ToolBarStateAssociation::mouseReleaseAssociation()
{
    if (m_pMouseEvent->button() != Qt::LeftButton ||
            !m_firstWidget || m_firstWidget->baseType() != Uml::wt_Class) {
        cleanAssociation();
        return;
    }

    getCurrentAssociation()->createAssocClassLine(
        static_cast<ClassifierWidget*>(m_firstWidget),
        getCurrentAssociation()->getLinePath()->onLinePath(m_pMouseEvent->scenePos()));
    m_firstWidget->addAssociationWidget( getCurrentAssociation() );
    cleanAssociation();
}

/**
 * Called when the release event happened on a widget.
 * If the button pressed isn't left button, the association is cleaned. If
 * it is left button, sets the first widget or the second, depending on
 * whether the first widget is already set or not.
 */
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

/**
 * Called when the release event happened on an empty space.
 * Cleans the association.
 */
void ToolBarStateAssociation::mouseReleaseEmpty() {
    cleanAssociation();
}

/**
 * Sets the first widget in the association using the current widget.
 * If the widget can't be associated using the current type of association,
 * an error is shown and the widget isn't set.
 * Otherwise, the temporal visual association is created and the mouse
 * tracking is enabled, so move events will be delivered.
 */
void ToolBarStateAssociation::setFirstWidget() {
    UMLRectWidget* widget = getCurrentWidget();
    Association_Type type = getAssociationType();

    if (!AssocRules::allowAssociation(type, widget)) {
        //TODO improve error feedback: tell the user what are the valid type of associations for
        //that widget
        KMessageBox::error(0, i18n("Incorrect use of associations."), i18n("Association Error"));
        return;
    }
    //set up position
    QPointF pos;
    pos.setX(widget->x() + (widget->width() / 2));
    pos.setY(widget->y() + (widget->height() / 2));
    //TODO why is this needed?
    m_pUMLScene->setPos(pos);

    m_firstWidget = widget;

    m_associationLine = new QGraphicsLineItem();
    m_pUMLScene->addItem(m_associationLine);
    m_associationLine->setLine(pos.x(), pos.y(), pos.x(), pos.y());
    m_associationLine->setPen(QPen(m_pUMLScene->getLineColor(), m_pUMLScene->getLineWidth(), Qt::DashLine));

    m_associationLine->setVisible(true);

    // [PORT]
    // m_pUMLScene->viewport()->setMouseTracking(true);
}

/**
 * Sets the second widget in the association using the current widget and
 * creates the association.
 * If the association between the two widgets using the current type of
 * association, an error is shown and the association cancelled.
 * Otherwise, the association is created and added to the scene, and the tool
 * is changed to the default tool.
 *
 * @todo Why change to the default tool? Shouldn't it better to stay on
 *       association and let the user change with a right click? The tool to
 *       create widgets doesn't change to default after creating a widget
 */
void ToolBarStateAssociation::setSecondWidget()
{
    Association_Type type = getAssociationType();
    UMLRectWidget* widgetA = m_firstWidget;
    UMLRectWidget* widgetB = getCurrentWidget();
    Widget_Type at = widgetA->baseType();
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
        AssociationWidget *temp = new AssociationWidget(m_pUMLScene, widgetA, type, widgetB);
        addAssociationInViewAndDoc(temp);
        if (type == at_Containment) {
            UMLListView *lv = UMLApp::app()->getListView();
            UMLObject *newContainer = widgetA->umlObject();
            UMLObject *objToBeMoved = widgetB->umlObject();
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

/**
 * Returns the association type of this tool.
 *
 * @return The association type of this tool.
 */
Association_Type ToolBarStateAssociation::getAssociationType()
{
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
        case WorkToolBar::tbb_Seq_Combined_Fragment:
        case WorkToolBar::tbb_Seq_Precondition:
        case WorkToolBar::tbb_Seq_Message_Asynchronous: at = at_Seq_Message;       break;
        case WorkToolBar::tbb_Coll_Message:             at = at_Coll_Message;      break;
        case WorkToolBar::tbb_State_Transition:         at = at_State;             break;
        case WorkToolBar::tbb_Activity_Transition:      at = at_Activity;          break;
        case WorkToolBar::tbb_Exception:                at = at_Exception;         break;
        case WorkToolBar::tbb_Category2Parent:          at = at_Category2Parent;   break;
        case WorkToolBar::tbb_Child2Category:           at = at_Child2Category;    break;

        default:                                        at = at_Unknown;           break;
    }

    return at;
}

/**
 * Adds an AssociationWidget to the association list and creates the
 * corresponding UMLAssociation in the current UMLDoc.
 * If the association can't be added, is deleted.
 *
 * @param association The AssociationWidget to add.
 */
void ToolBarStateAssociation::addAssociationInViewAndDoc(AssociationWidget* a)
{
    // append in view
    if (m_pUMLScene->addAssociation(a, false)) {
        // if view went ok, then append in document
        UMLAssociation *umla = a->getAssociation();
        if (umla == NULL) {
            // association without model representation in UMLDoc
            return;
        }
        Uml::Model_Type m = Model_Utils::convert_DT_MT(m_pUMLScene->getType());
        UMLDoc *umldoc = UMLApp::app()->getDocument();
        umla->setUMLPackage(umldoc->getRootFolder(m));
        UMLApp::app()->getDocument()->addAssociation(umla);
    } else {
        uError() << "cannot addAssocInViewAndDoc(), deleting" << endl;
        delete a;
    }
}

/**
 * Cleans the first widget and the temporal association line, if any.
 * Both are set to null, and the association line is also deleted.
 */
void ToolBarStateAssociation::cleanAssociation() {
    m_firstWidget = 0;

    delete m_associationLine;
    m_associationLine = 0;
}

#include "toolbarstateassociation.moc"
