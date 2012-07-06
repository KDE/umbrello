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
#include "packagewidget.h"

// app includes
#include "package.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"

// qt includes
#include <QPainter>

/**
 * Constructs a PackageWidget for a given UMLPackage object.
 */
PackageWidget::PackageWidget(UMLPackage *o) 
  : UMLWidget(WidgetBase::wt_Package, o)
{
    createTextItemGroup();
}

/**
 * Destructor.
 */
PackageWidget::~PackageWidget()
{
}

/**
 * Reimplemented from UMLWidget::paint to draw PackageWidget.
 */
void PackageWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(brush());
    painter->setPen(QPen(lineColor(), lineWidth()));

    painter->drawRect(m_topTextRect);
    painter->drawRect(m_packageRect);
    if (umlObject()->stereotype() == "subsystem") {
        const qreal fHalf = m_topTextRect.height() / 2;
        const qreal symY = fHalf;
        const qreal symX = 38;
        painter->drawLine(symX, symY, symX, symY + fHalf - 2);          // left leg
        painter->drawLine(symX + 8, symY, symX + 8, symY + fHalf - 2);  // right leg
        painter->drawLine(symX, symY, symX + 8, symY);                  // waist
        painter->drawLine(symX + 4, symY, symX + 4, symY - fHalf + 2);  // head
    }
}

/**
 * Reimplemented from UMLWidget::updateGeometry to calculate
 * minimum size.
 */
void PackageWidget::updateGeometry()
{
    TextItemGroup *grp = textItemGroupAt(GroupIndex);

    QSizeF minSize = grp->minimumSize();
    // add 10.0 to show more than the topTextRect
    minSize.setWidth(minSize.width() + 10.0);

    if(minSize.width() < 70) {
        minSize.setWidth(70);
    }
    minSize.rheight() += QFontMetricsF(grp->font()).lineSpacing();
    setMinimumSize(minSize);

    UMLWidget::updateGeometry();
}

/**
 * Reimplemented from UMLWidget::updateTextItemGroups to update
 * texts and their properties.
 */
void PackageWidget::updateTextItemGroups()
{
    if(umlObject()) {
        TextItemGroup *grp = textItemGroupAt(GroupIndex);
        grp->setTextItemCount(PackageWidget::TextItemCount);

        TextItem *stereo = grp->textItemAt(PackageWidget::StereoTypeItemIndex);
        stereo->setText(umlObject()->stereotype(true));
        stereo->setBold(true);
        stereo->setExplicitVisibility(umlObject()->stereotype().isEmpty() == false);

        TextItem *nameItem = grp->textItemAt(PackageWidget::NameItemIndex);
        nameItem->setText(name());
    }
    UMLWidget::updateTextItemGroups();
}

/**
 * Reimplemented from UMLWidget::attributeChange to handle
 * SizeHasChanged in which the text position is updated.
 */
QVariant PackageWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if(change == SizeHasChanged) {
        TextItemGroup *grp = textItemGroupAt(GroupIndex);
        QSizeF minSize = grp->minimumSize();
        m_topTextRect.setRect(0, 0, minSize.width(), minSize.height());

        m_packageRect.setTopLeft(QPointF(0, m_topTextRect.bottom()));
        m_packageRect.setBottomRight(rect().bottomRight());

        grp->setGroupGeometry(m_topTextRect);
    }

    return UMLWidget::attributeChange(change, oldValue);
}

/**
 * Reimplemented from UMLWidget::saveToXMI to save
 * PackageWidget info into XMI.
 */
void PackageWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("packagewidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}
