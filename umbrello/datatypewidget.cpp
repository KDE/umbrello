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
#include "datatypewidget.h"

// app includes
#include "classifier.h"
#include "textitem.h"
#include "textitemgroup.h"

// qt includes
#include <QtGui/QPainter>

/// Constructs a dataTypeWidget representing UMLClassifier \a d
DatatypeWidget::DatatypeWidget(UMLClassifier *d) :
    UMLRectWidget(d)
{
    m_baseType = Uml::wt_Datatype;
	createTextItemGroup();
}

/// Destructor
DatatypeWidget::~DatatypeWidget()
{
}

/**
 * Reimplemented from UMLWidget::saveToXMI to save DataTypeWidget
 * data into XMI element.
 */
void DatatypeWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    QDomElement conceptElement = qDoc.createElement("datatypewidget");
    UMLRectWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

/**
 * Reimplemented from UMLRectWidget::paint. Draws a rectangle.
 */
void DatatypeWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(brush());
    painter->setPen(QPen(lineColor(), lineWidth()));

    painter->drawRect(rect());
}

/**
 * Reimplemented from UMLRectWidget::updateGeometry to calculate
 * and set the minimum size for this widget.
 */
void DatatypeWidget::updateGeometry()
{
	TextItemGroup *grp = textItemGroupAt(GroupIndex);
	QSizeF minSize = grp->minimumSize();
	setMinimumSize(minSize);

	UMLRectWidget::updateGeometry();
}

/**
 * Reimplemented from UMLRectWidget::updateTextItemGroups to update
 * the text of TextItemGroups.
 */
void DatatypeWidget::updateTextItemGroups()
{
	if(umlObject()) {
		TextItemGroup *grp = textItemGroupAt(DatatypeWidget::GroupIndex);
		grp->setTextItemCount(DatatypeWidget::TextItemCount);

        TextItem *stereo = grp->textItemAt(DatatypeWidget::StereoTypeItemIndex);
        stereo->setText(umlObject()->getStereotype(true));
        stereo->setBold(true);

        TextItem *nameItem = grp->textItemAt(DatatypeWidget::NameItemIndex);
        nameItem->setText(name());
        nameItem->setItalic(umlObject()->getAbstract());
    }

    UMLRectWidget::updateTextItemGroups();
}

/**
 * Reimplemented from UMLRectWidget::attributeChange to handle
 * change of size to align TextItemGroup
 */
QVariant DatatypeWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if(change == SizeHasChanged) {
		TextItemGroup *grp = textItemGroupAt(DatatypeWidget::GroupIndex);
		grp->setGroupGeometry(rect());
	}
    return UMLRectWidget::attributeChange(change, oldValue);
}
