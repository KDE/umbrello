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
#include "datatypewidget.h"

// app includes
#include "classifier.h"
#include "textitem.h"
#include "textitemgroup.h"

// qt includes
#include <QPainter>

/// Constructs a dataTypeWidget representing UMLClassifier \a d
DatatypeWidget::DatatypeWidget(UMLClassifier *d) 
  : UMLWidget(WidgetBase::wt_Datatype, d)
{
    createTextItemGroup();
}

/**
 * Destructor.
 */
DatatypeWidget::~DatatypeWidget()
{
}

/**
 * Reimplemented from WidgetBase::saveToXMI to save DataTypeWidget
 * data into XMI element.
 */
void DatatypeWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    QDomElement conceptElement = qDoc.createElement("datatypewidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

/**
 * Reimplemented from UMLWidget::paint. Draws a rectangle.
 */
void DatatypeWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(brush());
    painter->setPen(QPen(lineColor(), lineWidth()));

    painter->drawRect(rect());
}

/**
 * Reimplemented from UMLWidget::updateGeometry to calculate
 * and set the minimum size for this widget.
 */
void DatatypeWidget::updateGeometry()
{
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    QSizeF minSize = grp->minimumSize();
    setMinimumSize(minSize);

    UMLWidget::updateGeometry();
}

/**
 * Reimplemented from UMLWidget::updateTextItemGroups to update
 * the text of TextItemGroups.
 */
void DatatypeWidget::updateTextItemGroups()
{
    if(umlObject()) {
        TextItemGroup *grp = textItemGroupAt(DatatypeWidget::GroupIndex);
        grp->setTextItemCount(DatatypeWidget::TextItemCount);

        TextItem *stereo = grp->textItemAt(DatatypeWidget::StereoTypeItemIndex);
        stereo->setText(umlObject()->stereotype(true));
        stereo->setBold(true);

        TextItem *nameItem = grp->textItemAt(DatatypeWidget::NameItemIndex);
        nameItem->setText(name());
        nameItem->setItalic(umlObject()->isAbstract());
    }

    UMLWidget::updateTextItemGroups();
}

/**
 * Reimplemented from UMLWidget::attributeChange to handle
 * change of size to align TextItemGroup
 */
QVariant DatatypeWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if(change == SizeHasChanged) {
        TextItemGroup *grp = textItemGroupAt(DatatypeWidget::GroupIndex);
        grp->setGroupGeometry(rect());
    }
    return UMLWidget::attributeChange(change, oldValue);
}
