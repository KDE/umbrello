/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "objectnodewidget.h"

// app includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "listpopupmenu.h"
#include "objectnodedialog.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"

// kde includes
#include <klocale.h>
#include <kinputdialog.h>
#include <kdialog.h>

// qt includes
#include <QPainter>
#include <QPointer>

#define OBJECTNODE_MARGIN 5
#define OBJECTNODE_WIDTH 30
#define OBJECTNODE_HEIGHT 10

/**
 * Creates a Object Node widget.
 *
 * @param scene            The parent of the widget.
 * @param objectNodeType   The type of object node
 * @param id               The ID to assign (-1 will prompt a new ID.)
 */
ObjectNodeWidget::ObjectNodeWidget(UMLScene * scene, ObjectNodeType objectNodeType, Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_ObjectNode, id)
{
    setObjectNodeType(objectNodeType);
    setState("");
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
void ObjectNodeWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    int w = width();
    int h = height();

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    int textStartY = (h / 2) - (fontHeight / 2);

    setPenFromSettings(painter);

    if (UMLWidget::useFillColor()) {
        painter->setBrush(UMLWidget::fillColor());
    }

    painter->drawRect(0, 0, w, h);
    painter->setFont(UMLWidget::font());

    switch (m_objectNodeType)
    {
    case Normal : break;
    case Buffer :
        {
            painter->setPen(textColor());
            painter->drawText(OBJECTNODE_MARGIN, (textStartY/2), w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, "<< centralBuffer >>");
            painter->drawText(OBJECTNODE_MARGIN, (textStartY/2) + fontHeight + 5, w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, name());
        }
        break;
    case Data :
        {
            painter->setPen(textColor());
            painter->drawText(OBJECTNODE_MARGIN, (textStartY/2), w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, "<< datastore >>");
            painter->drawText(OBJECTNODE_MARGIN, (textStartY/2) + fontHeight + 5, w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, name());
        }
        break;
    case Flow :
        {
            QString objectflow_value;
            if(state() == "-" || state() == NULL)
            {
                objectflow_value = ' ';
            }
            else
            {
                objectflow_value = '[' + state() + ']';
            }

            painter->drawLine(10, h/2, w-10, h/2);
            painter->setPen(textColor());
            painter->setFont(UMLWidget::font());
            painter->drawText(OBJECTNODE_MARGIN, textStartY/2 - OBJECTNODE_MARGIN, w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, name());
            painter->drawText(OBJECTNODE_MARGIN, textStartY/2 + textStartY + OBJECTNODE_MARGIN, w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, objectflow_value);
        }
        break;
    }

    if(m_selected)
        paintSelected(painter);

}

/**
 * Overrides method from UMLWidget.
 */
UMLSceneSize ObjectNodeWidget::minimumSize()
{
    int widthtmp = 10, height = 10,width=10;
    if (m_objectNodeType == Buffer) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const int textWidth = fm.width("<< centrelBuffer >>");
        const int namewidth = fm.width(name());
        height = fontHeight * 2;
        widthtmp = textWidth > OBJECTNODE_WIDTH ? textWidth : OBJECTNODE_WIDTH;
        width = namewidth > widthtmp ? namewidth : widthtmp;
        height = height > OBJECTNODE_HEIGHT ? height : OBJECTNODE_HEIGHT;
        width += OBJECTNODE_MARGIN * 2;
        height += OBJECTNODE_MARGIN * 2 + 5;
    } else if (m_objectNodeType == Data) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const int textWidth = fm.width("<< datastore >>");
        const int namewidth = fm.width(name());
        height = fontHeight * 2;
        widthtmp = textWidth > OBJECTNODE_WIDTH ? textWidth : OBJECTNODE_WIDTH;
        width = namewidth > widthtmp ? namewidth : widthtmp;
        height = height > OBJECTNODE_HEIGHT ? height : OBJECTNODE_HEIGHT;
        width += OBJECTNODE_MARGIN * 2;
        height += OBJECTNODE_MARGIN * 2 + 5;
    } else if (m_objectNodeType == Flow) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const int textWidth = fm.width('[' + state() + ']');
        const int namewidth = fm.width(name());
        height = fontHeight * 2;
        widthtmp = textWidth > OBJECTNODE_WIDTH ? textWidth : OBJECTNODE_WIDTH;
        width = namewidth > widthtmp ? namewidth : widthtmp;
        height = height > OBJECTNODE_HEIGHT ? height : OBJECTNODE_HEIGHT;
        width += OBJECTNODE_MARGIN * 2;
        height += OBJECTNODE_MARGIN * 4;
    }

    return UMLSceneSize(width, height);
}

/**
 * Returns the type of object node.
 */
ObjectNodeWidget::ObjectNodeType ObjectNodeWidget::objectNodeType() const
{
    return m_objectNodeType;
}

/**
 * Returns the type of object node.
 * TODO: static, rename to "toObjectNodeType(...).
 */
ObjectNodeWidget::ObjectNodeType ObjectNodeWidget::objectNodeType(const QString& type) const
{
    if (type == "Central buffer")
       return ObjectNodeWidget::Buffer;
    if (type == "Data store")
       return ObjectNodeWidget::Data;
    if (type == "Object Flow")
       return ObjectNodeWidget::Flow;
    // Shouldn't happen
    Q_ASSERT(0);
    return ObjectNodeWidget::Flow;
}

/**
 * Sets the type of object node.
 */
void ObjectNodeWidget::setObjectNodeType(ObjectNodeType objectNodeType)
{
    m_objectNodeType = objectNodeType;
    UMLWidget::m_resizable = true;
}

/**
 * Sets the type of object node.
 */
void ObjectNodeWidget::setObjectNodeType(const QString& type)
{
   setObjectNodeType(ObjectNodeWidget::objectNodeType(type));
}

/**
 * Sets the state of an object node when it's an objectflow.
 */
void ObjectNodeWidget::setState(const QString& state)
{
    m_state = state;
    updateGeometry();
}

/**
 * Returns the state of object node.
 */
QString ObjectNodeWidget::state() const
{
    return m_state;
}

/**
 * Captures any popup menu signals for menus it created.
 */
void ObjectNodeWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        {
            bool ok = false;
            QString text = name();
            text = KInputDialog::getText(i18n("Enter Object Node Name"),
                                          i18n("Enter the name of the object node :"),
                                          name(), &ok);
            if (ok && !text.isEmpty()) {
                setName(text);
            }
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
 * Show a properties dialog for an ObjectNodeWidget.
 */
void ObjectNodeWidget::showPropertiesDialog()
{
    umlScene()->updateDocumentation(false);

    QPointer<ObjectNodeDialog> dialog = new ObjectNodeDialog(UMLApp::app()->currentView(), this);
    if (dialog->exec() && dialog->getChangesMade()) {
        umlScene()->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
    }
    delete dialog;
}

/**
 * Saves the widget to the "objectnodewidget" XMI element.
 */
void ObjectNodeWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement objectNodeElement = qDoc.createElement("objectnodewidget");
    UMLWidget::saveToXMI(qDoc, objectNodeElement);
    objectNodeElement.setAttribute("objectnodename", m_Text);
    objectNodeElement.setAttribute("documentation", m_Doc);
    objectNodeElement.setAttribute("objectnodetype", m_objectNodeType);
    objectNodeElement.setAttribute("objectnodestate", m_state);
    qElement.appendChild(objectNodeElement);
}

/**
 * Loads the widget from the "objectnodewidget" XMI element.
 */
bool ObjectNodeWidget::loadFromXMI(QDomElement& qElement)
{
    if(!UMLWidget::loadFromXMI(qElement) )
        return false;
    m_Text = qElement.attribute("objectnodename", "");
    m_Doc = qElement.attribute("documentation", "");
    QString type = qElement.attribute("objectnodetype", "1");
    m_state = qElement.attribute("objectnodestate","");
    setObjectNodeType((ObjectNodeType)type.toInt());
    return true;
}

/**
 * Open a dialog box to select the objectNode type (Data, Buffer or Flow).
 */
void ObjectNodeWidget::askForObjectNodeType(UMLWidget* &targetWidget)
{
    bool pressedOK = false;
    int current = 0;
    const QStringList list = QStringList() << "Central buffer" << "Data store" <<"Object Flow";

    QString type = KInputDialog::getItem (i18n("Select Object node type"),  i18n("Select the object node type"),list,current, false, &pressedOK, UMLApp::app());

    if (pressedOK) {
       // QString type = result.join("");
        dynamic_cast<ObjectNodeWidget*>(targetWidget)->setObjectNodeType(type);
        if (type == "Data store")
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the name of the data store node"), i18n("Enter the name of the data store node"), i18n("data store name"));
        if (type == "Central buffer")
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the name of the buffer node"), i18n("Enter the name of the buffer"), i18n("centralBuffer"));
        if (type == "Object Flow") {
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the name of the object flow"), i18n("Enter the name of the object flow"), i18n("object flow"));
            askStateForWidget();
        }
    } else {
        targetWidget->cleanup();
        delete targetWidget;
        targetWidget = NULL;
    }
}

/**
 * Open a dialog box to input the state of the widget.
 * This box is shown only if m_objectNodeType = Flow.
 */
void ObjectNodeWidget::askStateForWidget()
{
    bool pressedOK = false;
    QString state = KInputDialog::getText(i18n("Enter Object Flow State"),i18n("Enter State (keep '-' if there is no state for the object) "),i18n("-"), &pressedOK, UMLApp::app());

    if (pressedOK) {
        setState(state);
    } else {
        cleanup();
    }
}

void ObjectNodeWidget::slotOk()
{
     //   KDialog::accept();
}

#include "objectnodewidget.moc"

