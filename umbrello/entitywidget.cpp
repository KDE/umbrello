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
#include "entitywidget.h"

// app includes
#include "classifier.h"
#include "classifierlistitem.h"
#include "entity.h"
#include "entityattribute.h"
#include "foreignkeyconstraint.h"
#include "listpopupmenu.h"
#include "object_factory.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"
#include "umlclassifierlistitemlist.h"
#include "umldoc.h"
#include "uniqueconstraint.h"

// qt includes
#include <QtGui/QPainter>

const qreal EntityWidget::Margin = 5.0;

/**
 * Constructs an EntityWidget.
 *
 * @param o The UMLObject this will be representing.
 */
EntityWidget::EntityWidget(UMLObject* o) :
	NewUMLRectWidget(o)
{
    m_baseType = Uml::wt_Entity;
	m_textItemGroup = new TextItemGroup(this);
}

/// Destructor
EntityWidget::~EntityWidget()
{
	delete m_textItemGroup;
}

/// Reimplemented from NewUMLRectWidget::sizeHint
QSizeF EntityWidget::sizeHint(Qt::SizeHint which)
{
	if(which == Qt::MinimumSize) {
		return m_minimumSize;
	}
	return NewUMLRectWidget::sizeHint(which);
}

/**
 * Reimplemented from NewUMLRectWidget::paint
 *
 * Draws the entity as a rectangle with a box underneith with a list
 * of literals
 */
void EntityWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
	painter->setPen(QPen(lineColor(), lineWidth()));
	painter->setBrush(brush());

	painter->drawRect(rect());

	// Now get the position to draw the line.
    const TextItem *item = m_textItemGroup->textItemAt(NameItemIndex);
    const QPointF bottomLeft = item->mapToParent(item->boundingRect().bottomLeft());
    const qreal y = bottomLeft.y();
    painter->drawLine(QLineF(0, y, size().width() - 1, y));
}

/**
 * Reimplemented from NewUMLRectWidget::saveToXMI to save this widget
 * info to the "entitywidget" XMI element.
 */
void EntityWidget::saveToXMI( QDomDocument& qDoc, QDomElement& qElement )
{
    QDomElement conceptElement = qDoc.createElement("entitywidget");
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

/**
 * Reimplemented from NewUMLRectWidget::updateGeometry to apply the
 * properties and calculate minimum size for this widget.
 */
void EntityWidget::updateGeometry()
{
	if(umlObject()) {
		UMLClassifier *classifier = static_cast<UMLClassifier*>(umlObject());
        UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EntityAttribute);
        int totalTextItems = list.size() + 2; // +2 because stereo text + name text.

		// Ensure there are appropriate number of textitems.
        m_textItemGroup->ensureTextItemCount(totalTextItems);

        // Create a dummy item with the common properties set on
        // it. We can then use TextItem::copyAttributesTo method to
        // copy these common attributes, just to avoid some code
        // duplication.
        TextItem dummy("");
        dummy.setDefaultTextColor(fontColor());
        dummy.setFont(font());
        dummy.setAlignment(Qt::AlignCenter);
        dummy.setBackgroundBrush(Qt::NoBrush);

		TextItem *stereo = m_textItemGroup->textItemAt(EntityWidget::StereotypeItemIndex);
        stereo->setText(classifier->getStereotype(true));
        dummy.copyAttributesTo(stereo);
        stereo->setBold(true);
		bool hideStereo = classifier->getStereotype(false).isEmpty();
		stereo->setVisible(!hideStereo);

        TextItem *nameItem = m_textItemGroup->textItemAt(EntityWidget::NameItemIndex);
        nameItem->setText(name());
        dummy.copyAttributesTo(nameItem);
        nameItem->setBold(true);
        nameItem->setItalic(classifier->getAbstract());

        int index = EntityWidget::EntityItemStartIndex;
        foreach(UMLClassifierListItem* entityItem, list) {
            TextItem *literal = m_textItemGroup->textItemAt(index);
            literal->setText(entityItem->getName());
            dummy.copyAttributesTo(literal);
			UMLEntityAttribute* casted = dynamic_cast<UMLEntityAttribute*>(entityItem);
			if( casted && casted->getIndexType() == Uml::Primary ) {
				literal->setUnderline(true);
			}
            ++index;
        }

        m_minimumSize = m_textItemGroup->calculateMinimumSize();

		// FIXME: The width doesn't function as expected if it is Margin * 2
        m_minimumSize += QSizeF(EntityWidget::Margin * 3, EntityWidget::Margin * 2);
    }
    NewUMLRectWidget::updateGeometry();
}

/**
 * Reimplemented from NewUMLRectWidget::sizeHasChanged to align and
 * position the text items.
 */
void EntityWidget::sizeHasChanged(const QSizeF& oldSize)
{
	QSizeF groupSize = size();
    groupSize -= QSizeF(EntityWidget::Margin * 2, EntityWidget::Margin * 2);
    QPointF offset(EntityWidget::Margin, EntityWidget::Margin);

    m_textItemGroup->alignVertically(groupSize);
    m_textItemGroup->setPos(offset);

    NewUMLRectWidget::sizeHasChanged(oldSize);
}

/**
 * Will be called when a menu selection has been made from the popup
 * menu.
 *
 * @param action       The action that has been selected.
 */
void EntityWidget::slotMenuSelection(QAction* action)
{
	// The menu is passed in as parent of the action.
	ListPopupMenu *menu = qobject_cast<ListPopupMenu*>(action->parent());
	ListPopupMenu::Menu_Type sel = menu->getMenuType(action);

	switch(sel) {
    case ListPopupMenu::mt_EntityAttribute:
        if (Object_Factory::createChildObject(static_cast<UMLClassifier*>(umlObject()),
                                              Uml::ot_EntityAttribute) )  {
            UMLApp::app()->getDocument()->setModified();
        }
        break;

    case ListPopupMenu::mt_PrimaryKeyConstraint:
    case ListPopupMenu::mt_UniqueConstraint:
        if ( UMLObject* obj = Object_Factory::createChildObject(static_cast<UMLEntity*>(umlObject()),
                                               Uml::ot_UniqueConstraint) ) {
            UMLApp::app()->getDocument()->setModified();

            if ( sel == ListPopupMenu::mt_PrimaryKeyConstraint ) {
                UMLUniqueConstraint* uc = static_cast<UMLUniqueConstraint*>(obj);
                static_cast<UMLEntity*>(umlObject())->setAsPrimaryKey(uc);
            }
        }
        break;

    case ListPopupMenu::mt_ForeignKeyConstraint:
         if (Object_Factory::createChildObject(static_cast<UMLEntity*>(umlObject()),
                                               Uml::ot_ForeignKeyConstraint) ) {
             UMLApp::app()->getDocument()->setModified();

        }
        break;

    case ListPopupMenu::mt_CheckConstraint:
         if (Object_Factory::createChildObject(static_cast<UMLEntity*>(umlObject()),
                                               Uml::ot_CheckConstraint) ) {
             UMLApp::app()->getDocument()->setModified();

        }
        break;

    default:
        NewUMLRectWidget::slotMenuSelection(action);
    }

}

#include "entitywidget.moc"
