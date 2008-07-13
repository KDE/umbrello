/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "newenumwidget.h"

// app includes
#include "classifier.h"
#include "classifierlistitem.h"
#include "enum.h"
#include "enumliteral.h"
#include "listpopupmenu.h"
#include "object_factory.h"
#include "optionstate.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"
#include "umlclassifierlistitemlist.h"
#include "umldoc.h"
#include "umlscene.h"
#include "widget_utils.h"

// qt/kde includes
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>

const qreal NewEnumWidget::Margin = 5.;

NewEnumWidget::NewEnumWidget(UMLObject* o) :
    NewUMLRectWidget(o),
    m_showPackage(false)
{
    m_textItemGroup = new TextItemGroup(this);
    m_baseType = Uml::wt_Enum;
    // init();
}

NewEnumWidget::~NewEnumWidget()
{
    delete m_textItemGroup;
}

// void NewEnumWidget::init()
// {
//     if(umlScene()) {
//         const Settings::OptionState& ops = umlScene()->getOptionState();
//         m_showPackage = ops.classState.showPackage;
//     }

//     updateGeometry();
// }

QSizeF NewEnumWidget::sizeHint(Qt::SizeHint which)
{
    if(which == Qt::MinimumSize) {
        return m_minimumSize;
    }
    return NewUMLRectWidget::sizeHint(which);
}

/**
 * Set whether to show package or not.
 *
 * @param b True to show package, false to hide.
 */
void NewEnumWidget::setShowPackage(bool b)
{
    m_showPackage = b;
    updateGeometry();
}

void NewEnumWidget::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    // First draw the outer rectangle with the pen and brush of this widget.
    painter->drawRect(rect());

    // Now get the position to draw the line.
    const TextItem *item = m_textItemGroup->textItemAt(NameItemIndex);
    const QPointF bottomLeft = item->mapToParent(item->boundingRect().bottomLeft());
    const qreal y = bottomLeft.y();
    painter->drawLine(QLineF(0, y, size().width() - 1, y));
}

/// Loads from an "enumwidget" XMI element.
bool NewEnumWidget::loadFromXMI( QDomElement & qElement )
{
    if( !NewUMLRectWidget::loadFromXMI(qElement) ) {
        return false;
    }
    bool show = bool(qElement.attribute("showpackage", "0").toInt());
    setShowPackage(show);
    return true;
}

/// Saves to the "enumwidget" XMI element.
void NewEnumWidget::saveToXMI( QDomDocument& qDoc, QDomElement& qElement )
{
    QDomElement conceptElement = qDoc.createElement("enumwidget");
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);

    conceptElement.setAttribute("showpackage", m_showPackage);
    qElement.appendChild(conceptElement);
}

void NewEnumWidget::slotMenuSelection(QAction *action)
{
    ListPopupMenu *menu = qobject_cast<ListPopupMenu*>(action->parent());
    if (!menu) {
        uError() << "ListPopupMenu's pointer should be the parent of the action parameter of this slot";
        return;
    }

    ListPopupMenu::Menu_Type sel = menu->getMenuType(action);

    if (sel == ListPopupMenu::mt_EnumLiteral) {
        if (!umlObject()) {
            uWarning() << "There is no UMLObject for this widget to create the literal!";
        }
        else if (Object_Factory::createChildObject(static_cast<UMLClassifier*>(umlObject()),
                                              Uml::ot_EnumLiteral) )  {
            UMLApp::app()->getDocument()->setModified();
        }
        return;
    }

    NewUMLRectWidget::slotMenuSelection(action);
}

/**
 * Overidden to update the literals, enum name and stereotype text
 * display on modifying the same.
 */
void NewEnumWidget::updateGeometry()
{
    if(umlObject()) {
        UMLClassifier *classifier = static_cast<UMLClassifier*>(umlObject());
        UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EnumLiteral);
        int totalTextItems = list.size() + 2; // +2 because stereo text + name text.

        // Create a dummy item with the common properties set on
        // it. We can then use TextItem::copyAttributesTo method to
        // copy these common attributes, just to avoid some code
        // duplication.
        TextItem dummy("");
        dummy.setDefaultTextColor(fontColor());
        dummy.setFont(font());
        dummy.setAlignment(Qt::AlignCenter);
        dummy.setBackgroundBrush(Qt::NoBrush);

        // Ensure there are appropriate number of textitems.
        m_textItemGroup->ensureTextItemCount(totalTextItems);

        TextItem *stereo = m_textItemGroup->textItemAt(NewEnumWidget::StereoTypeItemIndex);
        stereo->setText(classifier->getStereotype(true));
        dummy.copyAttributesTo(stereo);
        stereo->setBold(true);

        TextItem *nameItem = m_textItemGroup->textItemAt(NewEnumWidget::NameItemIndex);
        nameItem->setText(m_showPackage ?
                          classifier->getFullyQualifiedName() :
                          name());
        dummy.copyAttributesTo(nameItem);
        nameItem->setBold(true);
        nameItem->setItalic(classifier->getAbstract());

        int index = NewEnumWidget::EnumLiteralStartIndex;
        foreach(UMLClassifierListItem* enumLiteral, list) {
            TextItem *literal = m_textItemGroup->textItemAt(index);
            literal->setText(enumLiteral->getName());
            dummy.copyAttributesTo(literal);
            ++index;
        }

        m_minimumSize = m_textItemGroup->calculateMinimumSize();
        m_minimumSize += QSizeF(NewEnumWidget::Margin * 2, NewEnumWidget::Margin * 2);
    }
    NewUMLRectWidget::updateGeometry();
}

void NewEnumWidget::sizeHasChanged(const QSizeF& oldSize)
{
    QSizeF groupSize = size();
    groupSize -= QSizeF(NewEnumWidget::Margin * 2, NewEnumWidget::Margin * 2);
    QPointF offset(NewEnumWidget::Margin, NewEnumWidget::Margin);

    m_textItemGroup->alignVertically(groupSize);
    m_textItemGroup->setPos(offset);

    NewUMLRectWidget::sizeHasChanged(oldSize);
}
