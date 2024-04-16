/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "enumwidget.h"

// app includes
#include "classifier.h"
#include "classifierlistitem.h"
#include "debug_utils.h"
#include "enum.h"
#include "enumliteral.h"
#include "listpopupmenu.h"
#include "object_factory.h"
#include "uml.h"
#include "umlclassifierlistitemlist.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// qt includes
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(EnumWidget)

/**
 * Constructs an instance of EnumWidget.
 *
 * @param scene   The parent of this EnumWidget.
 * @param o       The UMLObject this will be representing.
 */
EnumWidget::EnumWidget(UMLScene *scene, UMLObject* o)
  : UMLWidget(scene, WidgetBase::wt_Enum, o),
    m_showPackage(false)
{
    setSize(100, 30);
    //set defaults from m_scene
    if (m_scene) {
        //check to see if correct
        const Settings::OptionState& ops = m_scene->optionState();
        m_showPackage = ops.classState.showPackage;
    } else {
        // For completeness only. Not supposed to happen.
        m_showPackage = false;
    }
}

/**
 * Destructor.
 */
EnumWidget::~EnumWidget()
{
}

/**
 * Returns the status of whether to show Package.
 *
 * @return  True if package is shown.
 */
bool EnumWidget::showPackage() const
{
    return m_showPackage;
}

/**
 * Set the status of whether to show Package.
 *
 * @param _status             True if package shall be shown.
 */
void EnumWidget::setShowPackage(bool _status)
{
    m_showPackage = _status;
    updateGeometry();
    update();
}

/**
 * Toggles the status of whether to show package.
 */
void EnumWidget::toggleShowPackage()
{
    m_showPackage = !m_showPackage;
    updateGeometry();
    update();
}

/**
 * Draws the enum as a rectangle with a box underneith with a list of literals
 * Reimplemented from UMLWidget::paint
 */
void EnumWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    setPenFromSettings(painter);
    if(UMLWidget::useFillColor())
        painter->setBrush(UMLWidget::fillColor());
    else
        painter->setBrush(m_scene->backgroundColor());

    const int w = width();
    const int h = height();

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    QString name;
    if (m_showPackage) {
        name = m_umlObject->fullyQualifiedName();
    } else {
        name = this->name();
    }

    painter->drawRect(0, 0, w, h);
    painter->setPen(textColor());

    QFont font = UMLWidget::font();
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(ENUM_MARGIN, 0,
               w - ENUM_MARGIN * 2, fontHeight,
               Qt::AlignCenter, m_umlObject->stereotype(true));

    font.setItalic(m_umlObject->isAbstract());
    painter->setFont(font);
    painter->drawText(ENUM_MARGIN, fontHeight,
               w - ENUM_MARGIN * 2, fontHeight, Qt::AlignCenter, name);
    font.setBold(false);
    font.setItalic(false);
    painter->setFont(font);

    int y = fontHeight * 2;

    setPenFromSettings(painter);

    painter->drawLine(0, y, w, y);

    QFontMetrics fontMetrics(font);
    const UMLClassifier *classifier = m_umlObject->asUMLClassifier();
    UMLClassifierListItem* enumLiteral = 0;
    UMLClassifierListItemList list = classifier->getFilteredList(UMLObject::ot_EnumLiteral);
    for(enumLiteral: list) {
        QString text = enumLiteral->toString();
        painter->setPen(textColor());
        painter->drawText(ENUM_MARGIN, y,
                   fontMetrics.width(text), fontHeight, Qt::AlignVCenter, text);
        y+=fontHeight;
    }

    UMLWidget::paint(painter, option, widget);
}

/**
 * Loads from an "enumwidget" XMI element.
 */
bool EnumWidget::loadFromXMI(QDomElement & qElement)
{
    if (!UMLWidget::loadFromXMI(qElement)) {
        return false;
    }
    QString showpackage = qElement.attribute(QStringLiteral("showpackage"), QStringLiteral("0"));

    m_showPackage = (bool)showpackage.toInt();

    return true;
}

/**
 * Saves to the "enumwidget" XMI element.
 */
void EnumWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("enumwidget"));
    UMLWidget::saveToXMI(writer);

    writer.writeAttribute(QStringLiteral("showpackage"), QString::number(m_showPackage));
    writer.writeEndElement();
}

/**
 * Will be called when a menu selection has been made from the
 * popup menu.
 *
 * @param action   The action that has been selected.
 */
void EnumWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    if (sel == ListPopupMenu::mt_EnumLiteral) {
        if (Object_Factory::createChildObject(m_umlObject->asUMLClassifier(),
                                              UMLObject::ot_EnumLiteral))  {
            /* I don't know why it works without these calls:
            updateComponentSize();
            update();
             */
            UMLApp::app()->document()->setModified();
        }
        return;
    }
    UMLWidget::slotMenuSelection(action);
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF EnumWidget::minimumSize() const
{
    if (!m_umlObject) {
        return UMLWidget::minimumSize();
    }

    int width, height;
    QFont font = UMLWidget::font();
    font.setItalic(false);
    font.setUnderline(false);
    font.setBold(false);
    const QFontMetrics fm(font);

    const int fontHeight = fm.lineSpacing();

    int lines = 1;//always have one line - for name
    lines++; //for the stereotype

    const int numberOfEnumLiterals = m_umlObject->asUMLEnum()->enumLiterals();

    height = width = 0;
    //set the height of the enum

    lines += numberOfEnumLiterals;
    if (numberOfEnumLiterals == 0) {
        height += fontHeight / 2; //no enum literals, so just add a bit of space
    }

    height += lines * fontHeight;

    //now set the width of the classifier
    //set width to name to start with
    if (m_showPackage)  {
        width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(m_umlObject->fullyQualifiedName()).width();
    } else {
        width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(name()).width();
    }
    int w = getFontMetrics(FT_BOLD).boundingRect(m_umlObject->stereotype(true)).width();


    width = w > width?w:width;

    const UMLClassifier *classifier = m_umlObject->asUMLClassifier();
    UMLClassifierListItemList list = classifier->getFilteredList(UMLObject::ot_EnumLiteral);
    UMLClassifierListItem *listItem = nullptr;
    for(listItem: list) {
        int w = fm.width(listItem->toString());
        width = w > width?w:width;
    }

    //allow for width margin
    width += ENUM_MARGIN * 2;

    return QSizeF(width, height);
}
