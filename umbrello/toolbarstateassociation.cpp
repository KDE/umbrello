/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "toolbarstateassociation.h"

// app includes
#include "assocrules.h"
#include "association.h"
#include "associationline.h"
#include "associationwidget.h"
#include "classifier.h"
#include "classifierwidget.h"
#include "cmds/widget/cmdcreatewidget.h"
#include "floatingtextwidget.h"
#include "debug_utils.h"
#include "folder.h"
#include "model_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

/**
 * Creates a new ToolBarStateAssociation.
 *
 * @param umlScene The UMLScene to use.
 */
ToolBarStateAssociation::ToolBarStateAssociation(UMLScene *umlScene)
  : ToolBarStatePool(umlScene),
    m_firstWidget(0),
    m_associationLine(0)
{
}

/**
 * Destroys this ToolBarStateAssociation.
 * Deletes the association line.
 */
ToolBarStateAssociation::~ToolBarStateAssociation()
{
    cleanAssociation();
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
        m_associationLine->setLine(sp.x(), sp.y(), m_pMouseEvent->scenePos().x(), m_pMouseEvent->scenePos().y());
    }
}

/**
 * A widget was removed from the UMLScene.
 * If the widget removed was the current widget, the current widget is set
 * to 0.
 * Also, if it was the first widget, the association is cleaned.
 */
void ToolBarStateAssociation::slotWidgetRemoved(UMLWidget* widget)
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
            !m_firstWidget || !m_firstWidget->isClassWidget()) {
        cleanAssociation();
        return;
    }

    currentAssociation()->createAssocClassLine(
            static_cast<ClassifierWidget*>(m_firstWidget),
            currentAssociation()->associationLine().closestSegmentIndex(m_pMouseEvent->scenePos()));
    m_firstWidget->addAssoc(currentAssociation());
    cleanAssociation();
}

/**
 * Called when the release event happened on a widget.
 * If the button pressed isn't left button, the association is cleaned. If
 * it is left button, sets the first widget or the second, depending on
 * whether the first widget is already set or not.
 */
void ToolBarStateAssociation::mouseReleaseWidget()
{
    if (m_pMouseEvent->button() != Qt::LeftButton) {
        cleanAssociation();
        return;
    }

    // TODO In old code in ToolBarState there was a TODO that said: Should not
    //be called by a Sequence message Association. Here's the check for that,
    //although I don't know why it is needed, but it seems that it's not needed,
    //as the old code worked fine without it...
    if (getAssociationType() == Uml::AssociationType::Seq_Message) {
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
void ToolBarStateAssociation::mouseReleaseEmpty()
{
    cleanAssociation();
}

/**
 * Sets the first widget in the association using the current widget.
 * If the widget can't be associated using the current type of association,
 * an error is shown and the widget isn't set.
 * Otherwise, the temporary visual association is created and the mouse
 * tracking is enabled, so move events will be delivered.
 */
void ToolBarStateAssociation::setFirstWidget()
{
    UMLWidget* widget = currentWidget();
    Uml::AssociationType::Enum type = getAssociationType();

    if (!AssocRules::allowAssociation(type, widget)) {
        //TODO improve error feedback: tell the user what are the valid type of associations for
        //that widget
        KMessageBox::error(0, i18n("Incorrect use of associations."), i18n("Association Error"));
        return;
    }
    //set up position
    QPoint pos;
    pos.setX(widget->scenePos().x() + (widget->width() / 2));
    pos.setY(widget->scenePos().y() + (widget->height() / 2));
    //TODO why is this needed?
    m_pUMLScene->setPos(pos);

    cleanAssociation();
    m_firstWidget = widget;

    // preliminary line
    m_associationLine = new QGraphicsLineItem();
    m_pUMLScene->addItem(m_associationLine);
    m_associationLine->setLine(pos.x(), pos.y(), pos.x(), pos.y());
    m_associationLine->setPen(QPen(m_pUMLScene->lineColor(), m_pUMLScene->lineWidth(), Qt::DashLine));
    m_associationLine->setVisible(true);

    m_pUMLScene->activeView()->viewport()->setMouseTracking(true);
}

/**
 * Sets the second widget in the association using the current widget and
 * creates the association.
 * If the association between the two widgets using the current type of
 * association is illegitimate, an error is shown and the association cancelled.
 * Otherwise, the association is created and added to the scene, and the tool
 * is changed to the default tool.
 *
 * @todo Why change to the default tool? Shouldn't it better to stay on
 *       association and let the user change with a right click? The tool to
 *       create widgets doesn't change to default after creating a widget
 */
void ToolBarStateAssociation::setSecondWidget()
{
    Uml::AssociationType::Enum type = getAssociationType();
    UMLWidget* widgetA = m_firstWidget;
    UMLWidget* widgetB = currentWidget();
    WidgetBase::WidgetType at = widgetA->baseType();
    bool valid = true;
    if (type == Uml::AssociationType::Generalization) {
        type = AssocRules::isGeneralisationOrRealisation(widgetA, widgetB);
    }
    if (widgetA == widgetB) {
        valid = AssocRules::allowSelf(type, at);
        if (valid && type == Uml::AssociationType::Association) {
            type = Uml::AssociationType::Association_Self;
        }
    } else {
        valid = AssocRules::allowAssociation(type, widgetA, widgetB);
    }
    if (valid) {
        if (widgetA->changesShape())
            widgetA->updateGeometry();
        if (widgetB->changesShape())
            widgetB->updateGeometry();
        if (widgetA->isClassWidget() && widgetB->isClassWidget()) {
            if (type == Uml::AssociationType::Composition) {
                UMLClassifier *c = widgetA->umlObject()->asUMLClassifier();
                UMLAttribute *attr = new UMLAttribute(c, c->uniqChildName(UMLObject::ot_Attribute));
                attr->setType(widgetB->umlObject());
                c->addAttribute(attr);
                cleanAssociation();
                emit finished();
                return;
            }
            /*  Activating this code will produce a DataType named as the Role B type
                with a suffixed "*" to indicate it is a pointer type.
                However, this is a non standard, C++ specific notation.
                Further, if this code is activated then no Aggregation association to the
                role B type will be shown. Instead, if the "B*" pointer type is dragged
                to the diagram then a Composition to that type will be shown.
            else if (type == Uml::AssociationType::Aggregation) {
                UMLClassifier *c = widgetA->umlObject()->asUMLClassifier();
                UMLAttribute *attr = new UMLAttribute(c, c->uniqChildName(UMLObject::ot_Attribute));
                attr->setTypeName(QString(QLatin1String("%1*")).arg(widgetB->umlObject()->name()));
                c->addAttribute(attr);
                cleanAssociation();
                emit finished();
                return;
            } */
        }
        AssociationWidget *temp = AssociationWidget::create(m_pUMLScene, widgetA, type, widgetB);
        if (widgetA->baseType() == UMLWidget::wt_Port) {
            QPointF lineStart = widgetA->getPos();
            uDebug() << "ToolBarStateAssociation::setSecondWidget : lineStart = " << lineStart;
            temp->associationLine().setPoint(0, lineStart);
        }
        FloatingTextWidget *wt = temp->textWidgetByRole(Uml::TextRole::Coll_Message);
        if (wt)
            wt->showOperationDialog();
        UMLApp::app()->executeCommand(new Uml::CmdCreateWidget(temp));
    } else {
        //TODO improve error feedback: tell the user what are the valid type of associations for
        //the second widget using the first widget
        KMessageBox::error(0, i18n("Incorrect use of associations."), i18n("Association Error"));
    }

    cleanAssociation();
    emit finished();
}

/**
 * Returns the association type of this tool.
 *
 * @return The association type of this tool.
 */
Uml::AssociationType::Enum ToolBarStateAssociation::getAssociationType()
{
    Uml::AssociationType::Enum at;

    switch(getButton()) {
        case WorkToolBar::tbb_Anchor:                   at = Uml::AssociationType::Anchor;            break;
        case WorkToolBar::tbb_Association:              at = Uml::AssociationType::Association;       break;
        case WorkToolBar::tbb_UniAssociation:           at = Uml::AssociationType::UniAssociation;    break;
        case WorkToolBar::tbb_Generalization:           at = Uml::AssociationType::Generalization;    break;
        case WorkToolBar::tbb_Composition:              at = Uml::AssociationType::Composition;       break;
        case WorkToolBar::tbb_Aggregation:              at = Uml::AssociationType::Aggregation;       break;
        case WorkToolBar::tbb_Relationship:             at = Uml::AssociationType::Relationship;      break;
        case WorkToolBar::tbb_Dependency:               at = Uml::AssociationType::Dependency;        break;
        case WorkToolBar::tbb_Containment:              at = Uml::AssociationType::Containment;       break;
        case WorkToolBar::tbb_Seq_Message_Creation:
        case WorkToolBar::tbb_Seq_Message_Destroy:
        case WorkToolBar::tbb_Seq_Message_Synchronous:
        case WorkToolBar::tbb_Seq_Combined_Fragment:
        case WorkToolBar::tbb_Seq_Precondition:
        case WorkToolBar::tbb_Seq_Message_Asynchronous: at = Uml::AssociationType::Seq_Message;       break;
        case WorkToolBar::tbb_Coll_Mesg_Sync: at = Uml::AssociationType::Coll_Mesg_Sync;      break;
        case WorkToolBar::tbb_Coll_Mesg_Async: at = Uml::AssociationType::Coll_Mesg_Async;      break;
        case WorkToolBar::tbb_State_Transition:         at = Uml::AssociationType::State;             break;
        case WorkToolBar::tbb_Activity_Transition:      at = Uml::AssociationType::Activity;          break;
        case WorkToolBar::tbb_Exception:                at = Uml::AssociationType::Exception;         break;
        case WorkToolBar::tbb_Category2Parent:          at = Uml::AssociationType::Category2Parent;   break;
        case WorkToolBar::tbb_Child2Category:           at = Uml::AssociationType::Child2Category;    break;

        default:                                        at = Uml::AssociationType::Unknown;           break;
    }

    return at;
}

/**
 * Adds an AssociationWidget to the association list and creates the
 * corresponding UMLAssociation in the current UMLDoc.
 * If the association can't be added, is deleted.
 *
 * @param assoc The AssociationWidget to add.
 * @return      True on success
 */
bool ToolBarStateAssociation::addAssociationInViewAndDoc(AssociationWidget* assoc)
{
    // append in view
    if (m_pUMLScene->addAssociation(assoc, false)) {
        // if view went ok, then append in document
        UMLAssociation *umla = assoc->association();
        if (umla) {
            // association with model representation in UMLDoc
            Uml::ModelType::Enum m = Model_Utils::convert_DT_MT(m_pUMLScene->type());
            UMLDoc *umldoc = UMLApp::app()->document();
            umla->setUMLPackage(umldoc->rootFolder(m));
            umldoc->addAssociation(umla);
        }
        return true;
    } else {
        uError() << "cannot addAssocInViewAndDoc(), deleting";
        delete assoc;
        return false;
    }
}

/**
 * Cleans the first widget and the temporary association line, if any.
 * Both are set to null, and the association line is also deleted.
 */
void ToolBarStateAssociation::cleanAssociation()
{
    m_firstWidget = 0;

    delete m_associationLine;
    m_associationLine = 0;
}

