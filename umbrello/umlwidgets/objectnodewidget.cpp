/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "objectnodewidget.h"

// app includes
#include "debug_utils.h"
#include "docwindow.h"
#include "dialog_utils.h"
#include "listpopupmenu.h"
#include "objectnodedialog.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "widget_utils.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QPainter>
#include <QPointer>
#include <QXmlStreamWriter>

#define OBJECTNODE_MARGIN  5
#define OBJECTNODE_WIDTH  30
#define OBJECTNODE_HEIGHT 10

DEBUG_REGISTER_DISABLED(ObjectNodeWidget)

/**
 * Creates an Object Node widget.
 *
 * @param scene            The parent of the widget.
 * @param objectNodeType   The type of object node
 * @param id               The ID to assign (-1 will prompt a new ID.)
 */
ObjectNodeWidget::ObjectNodeWidget(UMLScene * scene, ObjectNodeType objectNodeType, Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_ObjectNode, id)
{
    setObjectNodeType(objectNodeType);
    setState(QString());
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

    if (m_objectNodeType == Flow) {
        QString objectflow_value;
        if (state() == QLatin1String("-") || state().isEmpty()) {
            objectflow_value = QLatin1Char(' ');
        } else {
            objectflow_value = QLatin1Char('[') + state() + QLatin1Char(']');
        }
        painter->drawLine(10, h/2, w-10, h/2);
        painter->setPen(textColor());
        painter->setFont(UMLWidget::font());
        painter->drawText(OBJECTNODE_MARGIN, textStartY/2 - OBJECTNODE_MARGIN,
                          w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, name());
        painter->drawText(OBJECTNODE_MARGIN, textStartY/2 + textStartY + OBJECTNODE_MARGIN,
                          w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, objectflow_value);
    } else {
        painter->setPen(textColor());
        const QString stereoType = (m_objectNodeType == Normal ? QLatin1String("object") :
                                    m_objectNodeType == Buffer ? QLatin1String("centralBuffer")
                                                               : QLatin1String("datastore"));
        painter->drawText(OBJECTNODE_MARGIN, textStartY / 2, w - OBJECTNODE_MARGIN * 2, fontHeight,
                          Qt::AlignHCenter, Widget_Utils::adornStereo(stereoType));
        painter->drawText(OBJECTNODE_MARGIN, (textStartY / 2) + fontHeight + 5,
                          w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, name());
    }

    UMLWidget::paint(painter, option, widget);
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF ObjectNodeWidget::minimumSize() const
{
    int widthtmp = 10, height = 10, width=10;
    if (m_objectNodeType == Buffer) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const int textWidth = fm.width(Widget_Utils::adornStereo(QLatin1String("centralBuffer")));
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
        const int textWidth = fm.width(Widget_Utils::adornStereo(QLatin1String("datastore")));
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
        const int textWidth = fm.width(QLatin1Char('[') + state() + QLatin1Char(']'));
        const int namewidth = fm.width(name());
        height = fontHeight * 2;
        widthtmp = textWidth > OBJECTNODE_WIDTH ? textWidth : OBJECTNODE_WIDTH;
        width = namewidth > widthtmp ? namewidth : widthtmp;
        height = height > OBJECTNODE_HEIGHT ? height : OBJECTNODE_HEIGHT;
        width += OBJECTNODE_MARGIN * 2;
        height += OBJECTNODE_MARGIN * 4;
    }

    return QSizeF(width, height);
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
 */
ObjectNodeWidget::ObjectNodeType ObjectNodeWidget::toObjectNodeType(const QString& type)
{
    if (type == QLatin1String("Central buffer"))
       return ObjectNodeWidget::Buffer;
    if (type == QLatin1String("Data store"))
       return ObjectNodeWidget::Data;
    if (type == QLatin1String("Object Flow"))
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
   setObjectNodeType(ObjectNodeWidget::toObjectNodeType(type));
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
            QString text = name();
            bool ok = Dialog_Utils::askName(i18n("Enter Object Node Name"),
                                            i18n("Enter the name of the object node :"),
                                            text);
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
bool ObjectNodeWidget::showPropertiesDialog()
{
    UMLApp::app()->docWindow()->updateDocumentation(false);

    bool result = false;
    QPointer<ObjectNodeDialog> dialog = new ObjectNodeDialog(UMLApp::app()->currentView(), this);
    if (dialog->exec() && dialog->getChangesMade()) {
        UMLApp::app()->docWindow()->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
        result = true;
    }
    delete dialog;
    return result;
}

/**
 * Saves the widget to the "objectnodewidget" XMI element.
 */
void ObjectNodeWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("objectnodewidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeAttribute(QLatin1String("objectnodename"), m_Text);
    writer.writeAttribute(QLatin1String("documentation"), m_Doc);
    writer.writeAttribute(QLatin1String("objectnodetype"), QString::number(m_objectNodeType));
    writer.writeAttribute(QLatin1String("objectnodestate"), m_state);
    writer.writeEndElement();
}

/**
 * Loads the widget from the "objectnodewidget" XMI element.
 */
bool ObjectNodeWidget::loadFromXMI(QDomElement& qElement)
{
    if(!UMLWidget::loadFromXMI(qElement) )
        return false;
    m_Text = qElement.attribute(QLatin1String("objectnodename"));
    m_Doc = qElement.attribute(QLatin1String("documentation"));
    QString type = qElement.attribute(QLatin1String("objectnodetype"), QLatin1String("1"));
    m_state = qElement.attribute(QLatin1String("objectnodestate"));
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
    const QStringList list = QStringList()
                             << QLatin1String("Central buffer")
                             << QLatin1String("Data store")
                             << QLatin1String("Object Flow");

#if QT_VERSION >= 0x050000
    QString type = QInputDialog::getItem (UMLApp::app(),
                                          i18n("Select Object node type"),  i18n("Select the object node type"),
                                          list, current, false, &pressedOK);

#else
    QString type = KInputDialog::getItem (i18n("Select Object node type"),  i18n("Select the object node type"), list, current, false, &pressedOK, UMLApp::app());
#endif

    if (pressedOK) {
        targetWidget->asObjectNodeWidget()->setObjectNodeType(type);
        if (type == QLatin1String("Data store"))
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the name of the data store node"), i18n("Enter the name of the data store node"), i18n("data store name"));
        if (type == QLatin1String("Central buffer"))
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the name of the buffer node"), i18n("Enter the name of the buffer"), i18n("centralBuffer"));
        if (type == QLatin1String("Object Flow")) {
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the name of the object flow"), i18n("Enter the name of the object flow"), i18n("object flow"));
            askStateForWidget();
        }
    } else {
        targetWidget->cleanup();
        delete targetWidget;
        targetWidget = 0;
    }
}

/**
 * Open a dialog box to input the state of the widget.
 * This box is shown only if m_objectNodeType = Flow.
 */
void ObjectNodeWidget::askStateForWidget()
{
    QString state = i18n("-");
    bool pressedOK = Dialog_Utils::askName(i18n("Enter Object Flow State"),
                                           i18n("Enter State (keep '-' if there is no state for the object) "),
                                           state);
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


