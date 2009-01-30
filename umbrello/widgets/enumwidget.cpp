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

/**
 * Constructs an instance of EnumWidget.
 * @param o The NewUMLObject this will be representing.
 */
EnumWidget::EnumWidget(UMLObject* o) :
    UMLWidget(o),
    m_showPackage(false)
{
    m_baseType = Uml::wt_Enum;
    createTextItemGroup();
}

/// Destructor
EnumWidget::~EnumWidget()
{
}

/**
 * Set whether to show package or not.
 *
 * @param b True to show package, false to hide.
 */
void EnumWidget::setShowPackage(bool b)
{
    m_showPackage = b;
    updateTextItemGroups();
}

/**
 * Reimplemented from UMLWidget::paint to draw enum widget.
 */
void EnumWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    // First draw the outer rectangle with the pen and brush of this widget.
    painter->drawRect(rect());
    painter->drawLine(m_nameLine);
}

/// Loads from an "enumwidget" XMI element.
bool EnumWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI(qElement) ) {
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
    UMLWidget::saveToXMI(qDoc, conceptElement);

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

    UMLWidget::slotMenuSelection(action);
}

/**
 * Reimplemented from UMLWidget::updateGeometry to calculate
 * minimum size.
 */
void EnumWidget::updateGeometry()
{
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    setMinimumSize(grp->minimumSize());
    UMLWidget::updateGeometry();
}

/**
 * Reimplemented from UMLWidget::updateTextItemGroups to update
 * the texts and its properties.
 */
void EnumWidget::updateTextItemGroups()
{
    if(umlObject()) {
        UMLClassifier *classifier = static_cast<UMLClassifier*>(umlObject());
        UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EnumLiteral);
        int totalTextItems = list.size() + 2; // +2 because stereo text + name text.

        TextItemGroup *grp = textItemGroupAt(GroupIndex);
        grp->setTextItemCount(totalTextItems);

        TextItem *stereo = grp->textItemAt(EnumWidget::StereoTypeItemIndex);
        stereo->setText(classifier->getStereotype(true));
        stereo->setBold(true);

        TextItem *nameItem = grp->textItemAt(EnumWidget::NameItemIndex);
        nameItem->setText(m_showPackage ?
                          classifier->getFullyQualifiedName() :
                          name());
        nameItem->setBold(true);
        nameItem->setItalic(classifier->getAbstract());

        int index = EnumWidget::EnumLiteralStartIndex;
        foreach(UMLClassifierListItem* enumLiteral, list) {
            TextItem *literal = grp->textItemAt(index);
            literal->setText(enumLiteral->getName());
            ++index;
        }
    }
    UMLWidget::updateTextItemGroups();
}

/**
 * Reimplemented from UMLWidget::attributeChange to handle
 * SizeHasChanged to adjust position of texts.
 */
QVariant EnumWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if(change == SizeHasChanged) {
        QRectF groupGeometry(rect());

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

#include "enumwidget.moc"
