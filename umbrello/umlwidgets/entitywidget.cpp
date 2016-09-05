/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "entitywidget.h"

// app includes
#include "classifier.h"
#include "classifierlistitem.h"
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
    if (!m_umlObject->stereotype().isEmpty()) {
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
    UMLClassifier *classifier = m_umlObject->asUMLClassifier();
    UMLClassifierListItem* entityattribute = 0;
    UMLClassifierListItemList list = classifier->getFilteredList(UMLObject::ot_EntityAttribute);
    foreach (entityattribute, list) {
        QString text = entityattribute->name();
        painter->setPen(textColor());
        UMLEntityAttribute* casted = entityattribute->asUMLEntityAttribute();
        if(casted && casted->indexType() == UMLEntityAttribute::Primary)
        {
            font.setUnderline(true);
            painter->setFont(font);
            font.setUnderline(false);
        }
        painter->drawText(ENTITY_MARGIN, y,
                   fontMetrics.width(text), fontHeight, Qt::AlignVCenter, text);
        painter->setFont(font);
        y+=fontHeight;
    }

    UMLWidget::paint(painter, option, widget);
}

/**
 * Saves to the "entitywidget" XMI element.
 */
void EntityWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement(QLatin1String("entitywidget"));
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
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

    int width, height;
    QFont font = UMLWidget::font();
    font.setItalic(false);
    font.setUnderline(false);
    font.setBold(false);
    const QFontMetrics fm(font);

    const int fontHeight = fm.lineSpacing();

    int lines = 1;//always have one line - for name
    if (!m_umlObject->stereotype().isEmpty()) {
        lines++;
    }

    const int numberOfEntityAttributes = m_umlObject->asUMLEntity()->entityAttributes();

    height = width = 0;
    //set the height of the entity

    lines += numberOfEntityAttributes;
    if (numberOfEntityAttributes == 0) {
        height += fontHeight / 2; //no entity literals, so just add a bit of space
    }

    height += lines * fontHeight;

    //now set the width of the concept
    //set width to name to start with
    // FIXME spaces to get round beastie with font width,
    // investigate UMLWidget::getFontMetrics()
    width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(QLatin1Char(' ') + name() +
                                                        QLatin1Char(' ')).width();
    const int w = getFontMetrics(FT_BOLD).boundingRect(m_umlObject->stereotype(true)).width();

    width = w > width ? w : width;

    UMLClassifier* classifier = m_umlObject->asUMLClassifier();
    UMLClassifierListItemList list = classifier->getFilteredList(UMLObject::ot_EntityAttribute);
    UMLClassifierListItem* listItem = 0;
    foreach (listItem, list) {
        int w = fm.width(listItem->name());
        width = w > width?w:width;
    }

    //allow for width margin
    width += ENTITY_MARGIN * 2;

    return QSizeF(width, height);
}
