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
#include "enumwidget.h"

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

const qreal EnumWidget::Margin = 5;

EnumWidget::EnumWidget(UMLObject* o) :
    NewUMLRectWidget(o),
    m_showPackage(false)
{
    m_textItemGroup = new TextItemGroup(this);
    m_baseType = Uml::wt_Enum;
    // init();
}

EnumWidget::~EnumWidget()
{
    delete m_textItemGroup;
}

// void EnumWidget::init()
// {
//     if(umlScene()) {
//         const Settings::OptionState& ops = umlScene()->getOptionState();
//         m_showPackage = ops.classState.showPackage;
//     }

//     updateGeometry();
// }

QSizeF EnumWidget::sizeHint(Qt::SizeHint which)
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
void EnumWidget::setShowPackage(bool b)
{
    m_showPackage = b;
    updateGeometry();
}

void EnumWidget::paint(QPainter *painter,
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
bool EnumWidget::loadFromXMI( QDomElement & qElement )
{
    if( !NewUMLRectWidget::loadFromXMI(qElement) ) {
        return false;
    }
    bool show = bool(qElement.attribute("showpackage", "0").toInt());
    setShowPackage(show);
    return true;
}

/// Saves to the "enumwidget" XMI element.
void EnumWidget::saveToXMI( QDomDocument& qDoc, QDomElement& qElement )
{
    QDomElement conceptElement = qDoc.createElement("enumwidget");
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);

    conceptElement.setAttribute("showpackage", m_showPackage);
    qElement.appendChild(conceptElement);
}

void EnumWidget::slotMenuSelection(QAction *action)
{
	// The menu is passed in as parameter of action
    ListPopupMenu *menu = qobject_cast<ListPopupMenu*>(action->parent());
    ListPopupMenu::Menu_Type sel = menu->getMenuType(action);

    if (sel == ListPopupMenu::mt_EnumLiteral) {
		if (Object_Factory::createChildObject(static_cast<UMLClassifier*>(umlObject()),
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
void EnumWidget::updateGeometry()
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

        TextItem *stereo = m_textItemGroup->textItemAt(EnumWidget::StereoTypeItemIndex);
        stereo->setText(classifier->getStereotype(true));
        dummy.copyAttributesTo(stereo);
        stereo->setBold(true);

        TextItem *nameItem = m_textItemGroup->textItemAt(EnumWidget::NameItemIndex);
        nameItem->setText(m_showPackage ?
                          classifier->getFullyQualifiedName() :
                          name());
        dummy.copyAttributesTo(nameItem);
        nameItem->setBold(true);
        nameItem->setItalic(classifier->getAbstract());

        int index = EnumWidget::EnumLiteralStartIndex;
        foreach(UMLClassifierListItem* enumLiteral, list) {
            TextItem *literal = m_textItemGroup->textItemAt(index);
            literal->setText(enumLiteral->getName());
            dummy.copyAttributesTo(literal);
            ++index;
        }

        m_minimumSize = m_textItemGroup->calculateMinimumSize();

        // FIXME: Don't know why Margin * 2 for width doesn't fit the items.
        m_minimumSize += QSizeF(EnumWidget::Margin * 3, EnumWidget::Margin * 2);
    }
    NewUMLRectWidget::updateGeometry();
}

void EnumWidget::sizeHasChanged(const QSizeF& oldSize)
{
    QSizeF groupSize = size();
    groupSize -= QSizeF(EnumWidget::Margin * 2, EnumWidget::Margin * 2);
    QPointF offset(EnumWidget::Margin, EnumWidget::Margin);

    m_textItemGroup->alignVertically(groupSize);
    m_textItemGroup->setPos(offset);

    NewUMLRectWidget::sizeHasChanged(oldSize);
}

#include "enumwidget.moc"
