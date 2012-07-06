/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "objectnodewidget.h"

// app includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "objectnodedialog.h"
#include "listpopupmenu.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

// qt includes
#include <QPointer>
#include <QPainter>

const QSizeF ObjectNodeWidget::MinimumSize(30, 10);

/**
 * Creates a Object Node widget.
 *
 * @param objectNodeType   The type of object node
 * @param id               The ID to assign (-1 will prompt a new ID.)
 */
ObjectNodeWidget::ObjectNodeWidget(ObjectNodeType objectNodeType, Uml::IDType id)
  : UMLWidget(WidgetBase::wt_ObjectNode, id)
{
    m_objectNodeType = objectNodeType;
    createTextItemGroup();
}

/**
 * Destructor.
 */
ObjectNodeWidget::~ObjectNodeWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void ObjectNodeWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());
    painter->drawRect(rect());
    painter->drawLine(m_objectFlowLine);
}

/**
 * Helper to convert a string to ObjectNodeType.
 */
ObjectNodeWidget::ObjectNodeType ObjectNodeWidget::stringToObjectNodeType(const QString& objectNodeType)
{
    if (objectNodeType == "Central buffer")
       return ObjectNodeWidget::Buffer;
    if (objectNodeType == "Data store")
       return ObjectNodeWidget::Data;
    if (objectNodeType == "Object Flow")
       return ObjectNodeWidget::Flow;
    // Shouldn't happen
    Q_ASSERT(0);
    return ObjectNodeWidget::Flow;
}

/**
 * Returns the type of object node.
 */
ObjectNodeWidget::ObjectNodeType ObjectNodeWidget::objectNodeType() const
{
    return m_objectNodeType;
}

/**
 * Sets the object node type and updates texts.
 */
void ObjectNodeWidget::setObjectNodeType(ObjectNodeType objectNodeType)
{
    m_objectNodeType = objectNodeType;
    updateTextItemGroups();
}

/**
 * Sets the state of an object node when it's an objectflow.
 */
void ObjectNodeWidget::setState(const QString& state)
{
    m_state = state;
    updateTextItemGroups();
}

/**
 * @return the state of object node. (when objectFlow)
 */
QString ObjectNodeWidget::state() const
{
    return m_state;
}

/**
 * Reimplemented from UMLWidget::showPropertiesDialog to show
 * appropriate dialog for objectnode widget.
 */
void ObjectNodeWidget::showPropertiesDialog()
{
    umlScene()->updateDocumentation(false);

    QPointer<ObjectNodeDialog> dialog = new ObjectNodeDialog(umlScene()->activeView(), this);
    if (dialog->exec() && dialog->getChangesMade()) {
        umlScene()->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
    }
    delete dialog;
}

/**
 * Open a dialog box to input the state of the widget This box is
 * shown only if m_objectNodeType = Flow
 */
void ObjectNodeWidget::askStateForWidget()
{
    bool pressedOK = false;
    QString state = KInputDialog::getText(i18n("Enter Object Flow State"),i18n("Enter State (keep '-' if there is no state for the object) "),i18n("-"), &pressedOK, UMLApp::app());

    if (pressedOK) {
        setState(state);
    }
}

/**
 * Open a dialog box to select the objectNode type (Data, Buffer or
 * Flow)
 */
void ObjectNodeWidget::askForObjectNodeType(UMLWidget* &targetWidget)
{
    bool pressedOK = false;
    int current = 0;
    const QStringList list = QStringList() << "Central buffer" << "Data store" <<"Object Flow";

    QString typeString = KInputDialog::getItem ( i18n("Select Object node type"),
                                                 i18n("Select the object node type"),
                                                 list,current, false, &pressedOK,
                                                 UMLApp::app());
    ObjectNodeType type = ObjectNodeWidget::stringToObjectNodeType(typeString);

    if (pressedOK) {
       // QString type = result.join("");
        dynamic_cast<ObjectNodeWidget*>(targetWidget)->setObjectNodeType(type);
        if (typeString == "Data store") {
            Dialog_Utils::askNameForWidget(targetWidget,
                                           i18n("Enter the name of the data store node"),
                                           i18n("Enter the name of the data store node"),
                                           i18n("data store name"));
        }
        else if (typeString == "Central buffer") {
            Dialog_Utils::askNameForWidget(targetWidget,
                                           i18n("Enter the name of the buffer node"),
                                           i18n("Enter the name of the buffer"),
                                           i18n("centralBuffer"));
        }
        if (typeString == "Object Flow") {
            Dialog_Utils::askNameForWidget(targetWidget,
                                           i18n("Enter the name of the object flow"),
                                           i18n("Enter the name of the object flow"),
                                           i18n("object flow"));

            askStateForWidget();
        }
    }
    else {
        delete targetWidget;
        targetWidget = 0;
    }
}

/**
 * Reimplemented from UMLWidget::loadFromXMI to load
 * ObjectNodeWidget from XMI.
 */
bool ObjectNodeWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    setName(qElement.attribute( "objectnodename", "" ));
    setDocumentation(qElement.attribute( "documentation", "" ));
    m_state = qElement.attribute("objectnodestate","");

    QString type = qElement.attribute( "objectnodetype", "1" );
    setObjectNodeType( (ObjectNodeType)type.toInt() );

    return true;
}

/**
 * Reimplemented from UMLWidget::saveToXMI to save
 * ObjectNodeType info to XMI.
 */
void ObjectNodeWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement objectNodeElement = qDoc.createElement( "objectnodewidget" );
    UMLWidget::saveToXMI( qDoc, objectNodeElement );
    objectNodeElement.setAttribute( "objectnodename", name() );
    objectNodeElement.setAttribute( "documentation", documentation());
    objectNodeElement.setAttribute( "objectnodetype", m_objectNodeType );
    objectNodeElement.setAttribute( "objectnodestate", m_state );
    qElement.appendChild( objectNodeElement );
}

/**
 * Reimplemented from UMLWidget::slotMenuSelection to handle
 * particular menu actions.
 */
void ObjectNodeWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString text = name();

    ListPopupMenu *menu = ListPopupMenu::menuFromAction(action);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }
    ListPopupMenu::MenuType sel = menu->getMenuType(action);

    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        text = KInputDialog::getText( i18n("Enter Object Node Name"),
                                      i18n("Enter the name of the object node :"),
                                      name(), &ok );
        if (ok && !text.isEmpty()) {
            setName(text);
        }
        break;

    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Reimplemented from UMLWidget::updateGeometry to
 * calculate minimumSize for this widget.
 */
void ObjectNodeWidget::updateGeometry()
{
    QSizeF minSize = textItemGroupAt(GroupIndex)->minimumSize();
    minSize = minSize.expandedTo(ObjectNodeWidget::MinimumSize);
    setMinimumSize(minSize);

    UMLWidget::updateGeometry();
}

/**
 * Reimplemented from UMLWidget::updateTextItemGroups to update
 * texts of this widget based on current state.
 */
void ObjectNodeWidget::updateTextItemGroups()
{
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    grp->setTextItemCount(2);

    QString first;
    QString second = name();

    bool visible = true;

    if(m_objectNodeType == ObjectNodeWidget::Buffer) {
        first = QLatin1String("<< centralBuffer >>");
    }
    else if(m_objectNodeType == ObjectNodeWidget::Data) {
        first = QLatin1String("<< datastore >>");
    }
    else if(m_objectNodeType == ObjectNodeWidget::Flow) {
        first = name();
        QString s = state();
        if(s.isEmpty() || s == "-") {
            second = " ";
        }
        else {
            second = s.prepend('[').append(']');
        }
    }
    else if(m_objectNodeType == ObjectNodeWidget::Normal) {
        visible = false;
    }

    grp->textItemAt(0)->setText(first);
    grp->textItemAt(0)->setExplicitVisibility(visible);

    grp->textItemAt(1)->setText(second);
    grp->textItemAt(1)->setExplicitVisibility(visible);

    UMLWidget::updateTextItemGroups();
}

/**
 * Reimplemented form UMLWidget::attributeChange to handle @ref
 * SizeHasChanged notification by placing text at right place and also
 * calculating the line (in case of Flow).
 */
QVariant ObjectNodeWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if(change == SizeHasChanged) {
        TextItemGroup *grp = textItemGroupAt(GroupIndex);
        grp->setGroupGeometry(rect());

        // Calculate only if text items are properly intialized (which
        // is not during construction of this Widget)
        if (grp->textItemCount() > 0) {
            if (m_objectNodeType == ObjectNodeWidget::Flow) {
                const TextItem *item = grp->textItemAt(0);
                const QPointF bottomLeft = item->mapToParent(item->boundingRect().bottomLeft());
                const qreal y = bottomLeft.y();
                m_objectFlowLine.setLine(0, y, size().width() - 1, y);
            }
            else {
                m_objectFlowLine = QLineF();
            }
        }

    }
    return UMLWidget::attributeChange(change, oldValue);
}

#include "objectnodewidget.moc"
