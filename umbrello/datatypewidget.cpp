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

// qt/kde includes
#include <QtGui/QPainter>

DatatypeWidget::DatatypeWidget(UMLClassifier *d) :
    NewUMLRectWidget(d),
    m_minimumSize(100, 30)
{
    m_textItemGroup = new TextItemGroup(this);
    m_baseType = Uml::wt_Datatype;
}

DatatypeWidget::~DatatypeWidget()
{
}

bool DatatypeWidget::loadFromXMI(QDomElement &qElement)
{
    return NewUMLRectWidget::loadFromXMI(qElement);
}

void DatatypeWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    QDomElement conceptElement = qDoc.createElement("datatypewidget");
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

QSizeF DatatypeWidget::sizeHint(Qt::SizeHint which)
{
    if(which == Qt::MinimumSize) {
        return m_minimumSize;
    }

    return NewUMLRectWidget::sizeHint(which);
}

void DatatypeWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
    painter->setBrush(brush());
    painter->setPen(QPen(lineColor(), lineWidth()));

    painter->drawRect(rect());
}

void DatatypeWidget::updateGeometry()
{
    if(umlObject()) {
        int totalItemCount = 2; // SteroType and name

        // Create a dummy item, to store the properties so that it can
        // easily be used to copy the properties to other text items.
        TextItem dummy("");
        dummy.setDefaultTextColor(fontColor());
        dummy.setFont(font());
        dummy.setAcceptHoverEvents(true);
        // dummy.setHoverBrush(hoverBrush);
        dummy.setAlignment(Qt::AlignCenter);
        dummy.setBackgroundBrush(Qt::NoBrush);

        if(m_textItemGroup->size() != totalItemCount) {
            while(m_textItemGroup->size() < totalItemCount) {
                m_textItemGroup->appendTextItem(new TextItem(""));
            }
        }

        TextItem *stereo = m_textItemGroup->textItemAt(DatatypeWidget::StereoTypeItemIndex);
        stereo->setText(umlObject()->getStereotype(true));
        dummy.copyAttributesTo(stereo); // apply the attributes
        stereo->setBold(true);

        TextItem *nameItem = m_textItemGroup->textItemAt(DatatypeWidget::NameItemIndex);
        nameItem->setText(name());
        dummy.copyAttributesTo(nameItem); // apply the attributes
        nameItem->setItalic(umlObject()->getAbstract());

        m_minimumSize = m_textItemGroup->calculateMinimumSize();
        m_minimumSize.rwidth() += DATATYPE_MARGIN * 2;
    }

    NewUMLRectWidget::updateGeometry();
}

void DatatypeWidget::sizeHasChanged(const QSizeF& oldSize)
{
    QPointF offset(DATATYPE_MARGIN, 0);
    QSizeF groupSize = size();
    groupSize.rwidth() -= 2 * DATATYPE_MARGIN;

    m_textItemGroup->alignVertically(groupSize);
    m_textItemGroup->setPos(offset);

    NewUMLRectWidget::sizeHasChanged(oldSize);
}
