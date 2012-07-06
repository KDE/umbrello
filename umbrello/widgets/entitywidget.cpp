/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
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
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"
#include "umlclassifierlistitemlist.h"
#include "umldoc.h"
#include "uniqueconstraint.h"

/**
 * Constructs an EntityWidget.
 *
 * @param o The UMLObject this will be representing.
 */
EntityWidget::EntityWidget(UMLObject* o) 
  : UMLWidget(WidgetBase::wt_Entity, o)
{
    createTextItemGroup();
}

/**
 * Destructor.
 */
EntityWidget::~EntityWidget()
{
}

/**
 * Reimplemented from UMLWidget::paint
 *
 * Draws the entity as a rectangle with a box underneith with a list
 * of literals
 */
void EntityWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    painter->drawRect(rect());
    painter->drawLine(m_nameLine);
}

/**
 * Reimplemented from UMLWidget::saveToXMI to save this widget
 * info to the "entitywidget" XMI element.
 */
void EntityWidget::saveToXMI( QDomDocument& qDoc, QDomElement& qElement )
{
    QDomElement conceptElement = qDoc.createElement("entitywidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

/**
 * Reimplemented from UMLWidget::updateGeometry to apply the
 * properties and calculate minimum size for this widget.
 */
void EntityWidget::updateGeometry()
{
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    setMinimumSize(grp->minimumSize());
    UMLWidget::updateGeometry();
}

/**
 * Reimplemented from UMLWidget::updateTextItemGroups to set
 * text properties.
 */
void EntityWidget::updateTextItemGroups()
{
    if(umlObject()) {
        UMLClassifier *classifier = static_cast<UMLClassifier*>(umlObject());
        UMLClassifierListItemList list = classifier->getFilteredList(UMLObject::ot_EntityAttribute);
        int totalTextItems = list.size() + 2; // +2 because stereo text + name text.

        TextItemGroup *grp = textItemGroupAt(GroupIndex);
        grp->setTextItemCount(totalTextItems);

        TextItem *stereo = grp->textItemAt(EntityWidget::StereotypeItemIndex);
        stereo->setText(classifier->stereotype(true));
        stereo->setBold(true);
        bool hideStereo = classifier->stereotype(false).isEmpty();
        stereo->setExplicitVisibility(!hideStereo);

        TextItem *nameItem = grp->textItemAt(EntityWidget::NameItemIndex);
        nameItem->setText(name());
        nameItem->setBold(true);
        nameItem->setItalic(classifier->isAbstract());

        int index = EntityWidget::EntityItemStartIndex;
        foreach(UMLClassifierListItem* entityItem, list) {
            TextItem *literal = grp->textItemAt(index);
            literal->setText(entityItem->name());
            UMLEntityAttribute* casted = dynamic_cast<UMLEntityAttribute*>(entityItem);
            if( casted && casted->indexType() == UMLEntityAttribute::Primary ) {
                literal->setUnderline(true);
            }
            ++index;
        }
    }
    UMLWidget::updateTextItemGroups();
}

/**
 * Reimplemented from UMLWidget::attributeChange to handle
 * SizeHasChanged to align and position the text items.
 */
QVariant EntityWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if(change == SizeHasChanged) {
        QRectF groupGeometry = rect();

        TextItemGroup *grp = textItemGroupAt(GroupIndex);
        grp->setGroupGeometry(groupGeometry);

        // Check as textItems are uninitialized in during very first updates.
        if(grp->textItemCount() > NameItemIndex) {
            // Now get the position to draw the line.
            const TextItem *item = grp->textItemAt(NameItemIndex);
            const QPointF bottomLeft = item->mapToParent(item->boundingRect().bottomLeft());
            const qreal y = bottomLeft.y();
            m_nameLine.setLine(0, y, size().width() - 1, y);
        }
    }

    return UMLWidget::attributeChange(change, oldValue);
}

/**
 * Will be called when a menu selection has been made from the popup
 * menu.
 *
 * @param action       The action that has been selected.
 */
void EntityWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu *menu = ListPopupMenu::menuFromAction(action);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }
    ListPopupMenu::MenuType sel = menu->getMenuType(action);

    switch(sel) {
    case ListPopupMenu::mt_EntityAttribute:
        if (Object_Factory::createChildObject(static_cast<UMLClassifier*>(umlObject()),
                                              UMLObject::ot_EntityAttribute) )  {
            UMLApp::app()->document()->setModified();
        }
        break;

    case ListPopupMenu::mt_PrimaryKeyConstraint:
    case ListPopupMenu::mt_UniqueConstraint:
        if ( UMLObject* obj = Object_Factory::createChildObject(static_cast<UMLEntity*>(umlObject()),
                                               UMLObject::ot_UniqueConstraint) ) {
            UMLApp::app()->document()->setModified();

            if ( sel == ListPopupMenu::mt_PrimaryKeyConstraint ) {
                UMLUniqueConstraint* uc = static_cast<UMLUniqueConstraint*>(obj);
                static_cast<UMLEntity*>(umlObject())->setAsPrimaryKey(uc);
            }
        }
        break;

    case ListPopupMenu::mt_ForeignKeyConstraint:
         if (Object_Factory::createChildObject(static_cast<UMLEntity*>(umlObject()),
                                               UMLObject::ot_ForeignKeyConstraint) ) {
             UMLApp::app()->document()->setModified();

        }
        break;

    case ListPopupMenu::mt_CheckConstraint:
         if (Object_Factory::createChildObject(static_cast<UMLEntity*>(umlObject()),
                                               UMLObject::ot_CheckConstraint) ) {
             UMLApp::app()->document()->setModified();

        }
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }

}

#include "entitywidget.moc"
