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
#include "packagewidget.h"

// qt/kde includes
#include <qpainter.h>
#include <kdebug.h>

// app includes
#include "package.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"

/**
 * Constructs a PackageWidget for a given UMLPackage object.
 */
PackageWidget::PackageWidget(UMLPackage *o) :
    NewUMLRectWidget(o)
{
    m_baseType = Uml::wt_Package;
	createTextItemGroup();
}

/// Destructor
PackageWidget::~PackageWidget()
{
}

/**
 * Reimplemented from NewUMLRectWidget::saveToXMI to save
 * PackageWidget info into XMI.
 */
void PackageWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("packagewidget");
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

/**
 * Reimplemented from NewUMLRectWidget::paint to draw PackageWidget.
 */
void PackageWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(brush());
    painter->setPen(QPen(lineColor(), lineWidth()));

    painter->drawRect(m_topRect);
    painter->drawRect(m_packageTextRect);
    if (umlObject()->getStereotype() == "subsystem") {
        const qreal fHalf = m_topRect.height() / 2;
        const qreal symY = fHalf;
        const qreal symX = 38;
        painter->drawLine(symX, symY, symX, symY + fHalf - 2);          // left leg
        painter->drawLine(symX + 8, symY, symX + 8, symY + fHalf - 2);  // right leg
        painter->drawLine(symX, symY, symX + 8, symY);                  // waist
        painter->drawLine(symX + 4, symY, symX + 4, symY - fHalf + 2);  // head
    }
}

/**
 * Reimplemented from NewUMLRectWidget::updateGeometry to calculate
 * minimum size.
 */
void PackageWidget::updateGeometry()
{
	TextItemGroup *grp = textItemGroupAt(GroupIndex);

	QSizeF minSize = grp->minimumSize();
	if(minSize.width() < 70) {
		minSize.setWidth(70);
	}
	minSize.rheight() += QFontMetricsF(grp->font()).lineSpacing()
		+ 3 * margin(); // HACK

	setMinimumSize(minSize);

	NewUMLRectWidget::updateGeometry();
}

/**
 * Reimplemented from NewUMLRectWidget::updateTextItemGroups to update
 * texts and their properties.
 */
void PackageWidget::updateTextItemGroups()
{
	if(umlObject()) {
		TextItemGroup *grp = textItemGroupAt(GroupIndex);
		grp->setTextItemCount(TextItemCount);

        TextItem *stereo = grp->textItemAt(PackageWidget::StereoTypeItemIndex);
        stereo->setText(umlObject()->getStereotype(true));
        stereo->setBold(true);
        stereo->setVisible(umlObject()->getStereotype().isEmpty() == false);

        TextItem *nameItem = grp->textItemAt(PackageWidget::NameItemIndex);
        nameItem->setText(name());
    }
    NewUMLRectWidget::updateGeometry();
}

/**
 * Reimplemented from NewUMLRectWidget::attributeChange to handle
 * SizeHasChanged in which the text position is updated.
 */
QVariant PackageWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
	if(change == SizeHasChanged) {
		TextItemGroup *grp = textItemGroupAt(GroupIndex);

		m_topRect.setRect(0, 0, 50, QFontMetricsF(grp->font()).lineSpacing());

		m_packageTextRect.setTopLeft(QPointF(0, m_topRect.bottom()));
		m_packageTextRect.setBottomRight(rect().bottomRight());

		QRectF groupGeometry(margin(),
							 m_packageTextRect.top(),
							 m_packageTextRect.width() - 2 * margin(),
							 m_packageTextRect.height());

		grp->setGroupGeometry(groupGeometry);
	}

    return NewUMLRectWidget::attributeChange(change, oldValue);
}
