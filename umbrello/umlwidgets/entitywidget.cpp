/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "entitywidget.h"

// app includes
#include "umlclassifier.h"
#include "umlclassifierlistitem.h"
#include "debug_utils.h"
#include "entity.h"
#include "entityattribute.h"
#include "foreignkeyconstraint.h"
#include "listpopupmenu.h"
#include "object_factory.h"
#include "uml.h"
#include "umlclassifierlistitemlist.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "uniqueconstraint.h"

// qt includes
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(EntityWidget)

/**
 * Constructs an EntityWidget.
 *
 * @param scene   The parent of this EntityWidget.
 * @param o       The UMLObject this will be representing.
 */
EntityWidget::EntityWidget(UMLScene *scene, UMLObject* o)
  : UMLWidget(scene, WidgetBase::wt_Entity, o)
{
    setSize(100, 30);
}

/**
 * Destructor.
 */
EntityWidget::~EntityWidget()
{
}

/**
 * calculate content related size of widget.
 *
 * @return calculated widget size
 */
QSizeF EntityWidget::calculateSize(bool withExtensions /* = true */) const
{
    Q_UNUSED(withExtensions)
    const QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
    const int fontHeight = fm.lineSpacing();
    if (!m_umlObject)
        return QSizeF(width(), height());

    qreal width = 0, height = defaultMargin;
    if (showStereotype() && !m_umlObject->stereotype().isEmpty()) {
        const QFontMetrics &bfm = UMLWidget::getFontMetrics(UMLWidget::FT_BOLD);
        const int stereoWidth = bfm.size(0, m_umlObject->stereotype(true)).width();
        if (stereoWidth > width)
            width = stereoWidth;
        height += fontHeight;
    }

    const QFontMetrics &bfm = UMLWidget::getFontMetrics(UMLWidget::FT_BOLD);
    const int nameWidth = bfm.size(0, name()).width();
    if (nameWidth > width)
        width = nameWidth;
    height += fontHeight;

    const UMLClassifier *classifier = m_umlObject->asUMLClassifier();
    UMLClassifierListItemList list = classifier->getFilteredList(UMLObject::ot_EntityAttribute);
    for(UMLClassifierListItem* entityattribute : list) {
        QString text = entityattribute->name();
        UMLEntityAttribute* umlEA = entityattribute->asUMLEntityAttribute();
        if (showAttributeSignature()) {
            text.append(QStringLiteral(" : ") + umlEA->getTypeName());
            text.append(QStringLiteral(" [") + umlEA->getAttributes() + QStringLiteral("]"));
        }
        if (showStereotype()) {
            text.append(QStringLiteral(" ") + umlEA->stereotype(true));
        }
        const int nameWidth = bfm.size(0, text).width();
        if (nameWidth > width)
            width = nameWidth;
        height += fontHeight;
    }
    return QSizeF(width + 2*defaultMargin, height);
}

/**
 * Draws the entity as a rectangle with a box underneith with a list of literals
 */
void EntityWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
    int fontHeight  = fm.lineSpacing();
    const QString name = this->name();

    painter->drawRect(0, 0, w, h);
    painter->setPen(textColor());

    QFont font = UMLWidget::font();
    font.setBold(true);
    painter->setFont(font);
    int y = 0;
    if (showStereotype() && !m_umlObject->stereotype().isEmpty()) {
        painter->drawText(ENTITY_MARGIN, 0,
                   w - ENTITY_MARGIN * 2, fontHeight,
                   Qt::AlignCenter, m_umlObject->stereotype(true));
        font.setItalic(m_umlObject->isAbstract());
        painter->setFont(font);
        painter->drawText(ENTITY_MARGIN, fontHeight,
                   w - ENTITY_MARGIN * 2, fontHeight, Qt::AlignCenter, name);
        font.setBold(false);
        font.setItalic(false);
        painter->setFont(font);
        y = fontHeight * 2;
    } else {
        font.setItalic(m_umlObject->isAbstract());
        painter->setFont(font);
        painter->drawText(ENTITY_MARGIN, 0,
                   w - ENTITY_MARGIN * 2, fontHeight, Qt::AlignCenter, name);
        font.setBold(false);
        font.setItalic(false);
        painter->setFont(font);

        y = fontHeight;
    }

    setPenFromSettings(painter);

    painter->drawLine(0, y, w, y);

    QFontMetrics fontMetrics(font);
    const UMLClassifier *classifier = m_umlObject->asUMLClassifier();
    
    UMLClassifierListItemList list = classifier->getFilteredList(UMLObject::ot_EntityAttribute);
    for(UMLClassifierListItem *entityattribute: list) {
        QString text = entityattribute->name();
        painter->setPen(textColor());
        const UMLEntityAttribute* umlEA = entityattribute->asUMLEntityAttribute();
        if (showAttributeSignature()) {
            text.append(QStringLiteral(" : ") + umlEA->getTypeName());
            text.append(QStringLiteral(" [") + umlEA->getAttributes() + QStringLiteral("]"));
        }
        if (showStereotype()) {
            text.append(QStringLiteral(" ") + umlEA->stereotype(true));
        }
        if (umlEA && umlEA->indexType() == UMLEntityAttribute::Primary)
        {
            font.setUnderline(true);
            painter->setFont(font);
            font.setUnderline(false);
        }
        painter->drawText(ENTITY_MARGIN, y,
                   fontMetrics.horizontalAdvance(text), fontHeight, Qt::AlignVCenter, text);
        painter->setFont(font);
        y+=fontHeight;
    }

    UMLWidget::paint(painter, option, widget);
}

bool EntityWidget::loadFromXMI(QDomElement & qElement)
{
    if (!UMLWidget::loadFromXMI(qElement))
        return false;
    QString showAttributeSignatures = qElement.attribute(QStringLiteral("showattsigs"), QStringLiteral("0"));
    m_showAttributeSignatures = (bool)showAttributeSignatures.toInt();
    return true;
}

/**
 * Saves to the "entitywidget" XMI element.
 */
void EntityWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("entitywidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeAttribute(QStringLiteral("showattsigs"), QString::number(m_showAttributeSignatures));

    writer.writeEndElement();
}

/**
 * Will be called when a menu selection has been made from the popup
 * menu.
 *
 * @param action   The action that has been selected.
 */
void EntityWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_EntityAttribute:
        if (Object_Factory::createChildObject(m_umlObject->asUMLClassifier(),
                                              UMLObject::ot_EntityAttribute))  {
            UMLApp::app()->document()->setModified();
        }
        break;

    case ListPopupMenu::mt_PrimaryKeyConstraint:
    case ListPopupMenu::mt_UniqueConstraint:
        if (UMLObject* obj = Object_Factory::createChildObject(m_umlObject->asUMLEntity(),
                                               UMLObject::ot_UniqueConstraint)) {
            UMLApp::app()->document()->setModified();

            if (sel == ListPopupMenu::mt_PrimaryKeyConstraint) {
                UMLUniqueConstraint* uc = obj->asUMLUniqueConstraint();
                m_umlObject->asUMLEntity()->setAsPrimaryKey(uc);
            }
        }
        break;

    case ListPopupMenu::mt_ForeignKeyConstraint:
         if (Object_Factory::createChildObject(m_umlObject->asUMLEntity(),
                                               UMLObject::ot_ForeignKeyConstraint)) {
             UMLApp::app()->document()->setModified();

        }
        break;

    case ListPopupMenu::mt_CheckConstraint:
         if (Object_Factory::createChildObject(m_umlObject->asUMLEntity(),
                                               UMLObject::ot_CheckConstraint)) {
             UMLApp::app()->document()->setModified();

        }
        break;

    case ListPopupMenu::mt_Show_Attribute_Signature:
        setShowAttributeSignature(!showAttributeSignature());
        break;

    case ListPopupMenu::mt_Show_Stereotypes:
        setShowStereotype(showStereotype() == Uml::ShowStereoType::None ?
                                              Uml::ShowStereoType::Tags : Uml::ShowStereoType::None);
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF EntityWidget::minimumSize() const
{
    if (!m_umlObject) {
        return UMLWidget::minimumSize();
    }

    return calculateSize();
}

/**
 * Set the status of whether to show attributes.
 *
 * @param flag   True if attributes shall be shown.
 */
void EntityWidget::setShowAttributeSignature(bool flag)
{
    m_showAttributeSignatures = flag;
    updateGeometry();
    update();
}

/**
 * Returns the status of whether to show attributes.
 *
 * @return  True if attributes are shown.
 */
bool EntityWidget::showAttributeSignature() const
{
    return m_showAttributeSignatures;
}
