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
#include "componentwidget.h"

// app includes
#include "component.h"
#include "textitem.h"
#include "textitemgroup.h"

// qt/kde includes
#include <QtGui/QPainter>

/**
 * Constructs a ComponentWidget.
 *
 * @param c The UMLComponent this will be representing.
 */
ComponentWidget::ComponentWidget(UMLComponent *c)
	: UMLWidget(0, c)
{
	m_baseType = Uml::wt_Component;
    setMargin(10); // override default of 5 for other widgets.
	createTextItemGroup();
}

/// Destructor
ComponentWidget::~ComponentWidget()
{
}

/**
 * Reimplemented from UMLWidget::paint to paint component
 * widget.
 */
void ComponentWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	UMLComponent *umlcomp = static_cast<UMLComponent*>(umlObject());

	QPen pen(lineColor(), lineWidth());
	if (umlcomp && umlcomp->getExecutable()) {
		pen.setWidth(lineWidth() + 2);
	}
	painter->setPen(pen);
	painter->setBrush(brush());

	painter->drawRects(m_rects, 3);
}

void ComponentWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("componentwidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

void ComponentWidget::updateGeometry()
{
	TextItemGroup *grp = textItemGroupAt(GroupIndex);
	QSizeF minSize = grp->minimumSize();

	minSize.rwidth() += 4 * margin();
	if(minSize.width() < 70) {
		// atleast minwidth = 70
		minSize.setWidth(70);
	}

	qreal minHeight = 3 * QFontMetricsF(grp->font()).lineSpacing();
	if(minSize.height() < minHeight) {
		minSize.setHeight(minHeight);
	}

	setMinimumSize(minSize, UMLWidget::AddMargin);

	UMLWidget::updateGeometry();
}

void ComponentWidget::updateTextItemGroups()
{
	TextItemGroup *grp = textItemGroupAt(GroupIndex);
	grp->setTextItemCount(TextItemCount);

	if(umlObject()) {
		TextItem *stereo = grp->textItemAt(StereoItemIndex);
		stereo->setText(umlObject()->getStereotype(true));
		stereo->setBold(true);
		stereo->setVisible(umlObject()->getStereotype(false).isEmpty() == false);

		TextItem *nameItem = grp->textItemAt(NameItemIndex);
		nameItem->setBold(true);
		QString nameText = name();
		bool underline = false;
		if(this->isInstance()) {
			nameText.prepend(':');
			nameText.prepend(instanceName());
			underline = true;
		}
		nameItem->setText(nameText);
		nameItem->setUnderline(underline);
	}

	UMLWidget::updateTextItemGroups();
}

QVariant ComponentWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
	if (change == SizeHasChanged) {
		TextItemGroup *grp = textItemGroupAt(GroupIndex);
		const qreal m = margin();
		const qreal fontHeight = QFontMetricsF(grp->font()).lineSpacing();
		qreal w = size().width();
		qreal h = size().height();

		m_rects[0] = QRectF(2 * m, 0, w - 2 * m, h);
		m_rects[1] = QRectF(0, h/2 - fontHeight/2 - fontHeight, m * 4, fontHeight);
		m_rects[2] = QRectF(0, h/2 + fontHeight/2, m * 4, fontHeight);

		QRectF grpRect(m*4, m, w - 4 * m, h - 2 * m);
		grp->setGroupGeometry(grpRect);
    }

	return UMLWidget::attributeChange(change, oldValue);
}
