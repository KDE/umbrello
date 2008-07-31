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
#include "nodewidget.h"

// app includes
#include "node.h"
#include "textitem.h"
#include "textitemgroup.h"

// qt/kde includes
#include <QtGui/QPolygonF>

const qreal NodeWidget::DEPTH = 30;  ///< pixels on Z axis

/**
 * Constructs a NodeWidget.
 *
 * @param n The UMLNode this will be representing.
 */
NodeWidget::NodeWidget(UMLNode *n )
	: NewUMLRectWidget(n)
{
    m_baseType = Uml::wt_Node;
	createTextItemGroup();
	// above box but below NewUMLRectWidget because may embed widgets
    setZValue(1);
}

/// Destructor
NodeWidget::~NodeWidget()
{
}

/**
 * Reimplemented from NewUMLRectWidget::paint to draw node widget
 * drawing stored in the painter path.
 */
void NodeWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	painter->setPen(QPen(lineColor(), lineWidth()));
	painter->setBrush(brush());
	painter->drawPath(m_nodeWidgetPath);
}

/**
 * Reimplemented form NewUMLRectWidget::saveToXMI to save this widget
 * info into 'nodewidget' xmi element.
 */
void NodeWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("nodewidget");
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

/**
 * Reimplemented from NewUMLRectWidget::updateGeometry to calculate
 * minimum size for this widget.
 */
void NodeWidget::updateGeometry()
{
	TextItemGroup *grp = textItemGroupAt(GroupIndex);
	QSizeF minSize = grp->minimumSize();

	const qreal m = 2 * margin(); // This will be added by setMinimumSize call
	minSize += QSizeF(m, m) + QSizeF(NodeWidget::DEPTH, NodeWidget::DEPTH);

	setMinimumSize(minSize);

	NewUMLRectWidget::updateGeometry();
}

/**
 * Reimplemented from NewUMLRectWidget::updateTextItemGroups() to
 * calculate the texts and also show/hide the texts based on current
 * state.
 */
void NodeWidget::updateTextItemGroups()
{
	TextItemGroup *grp = textItemGroupAt(GroupIndex);
	grp->setTextItemCount(NodeWidget::TextItemCount);

	if(umlObject()) {
		UMLNode *node = static_cast<UMLNode*>(umlObject());

		TextItem *stereo = grp->textItemAt(NodeWidget::StereoItemIndex);
		stereo->setText(node->getStereotype(true));
		stereo->setBold(true);
		stereo->setVisible(!node->getStereotype(false).isEmpty());

		TextItem *nameItem = grp->textItemAt(NodeWidget::NameItemIndex);
		QString nameText = name();
		bool underline = false;
		if(isInstance()) {
			nameText.prepend(':');
			nameText.prepend(instanceName());
			underline = true;
		}
		nameItem->setBold(true);
		nameItem->setUnderline(underline);
		nameItem->setText(nameText);
	}

	NewUMLRectWidget::updateTextItemGroups();
}

/**
 * Reimplemented from NewUMLRectWidget::attributeChange to handle @ref
 * SizeHasChanged to position the texts as well as build the painter
 * path corresponding to current size.
 */
QVariant NodeWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
	if(change == SizeHasChanged) {
		TextItemGroup *grp = textItemGroupAt(GroupIndex);
		m_nodeWidgetPath = QPainterPath(); // reset path

		const qreal m = margin();
		const qreal w = size().width();
		const qreal h = size().height();
		const qreal wDepth = qMin(w/3, NodeWidget::DEPTH);
		const qreal hDepth = qMin(h/3, NodeWidget::DEPTH);
		const qreal bodyOffsetY = hDepth;
		const qreal bodyWidth = w - wDepth;
		const qreal bodyHeight = h - hDepth;

		QPolygonF poly;
		poly << QPointF(0, bodyOffsetY)
			 << QPointF(wDepth, 0)
			 << QPointF(w - 1, 0)
			 << QPointF(w - 1, bodyHeight)
			 << QPointF(bodyWidth, h - 1)
			 << QPointF(bodyWidth, bodyOffsetY)
			 << QPointF(0, bodyOffsetY);
		m_nodeWidgetPath.addPolygon(poly);

		QRectF bodyRect(0, bodyOffsetY, bodyWidth, bodyHeight);
		m_nodeWidgetPath.addRect(bodyRect);

		QLineF line(w - 1, 0, bodyWidth - 2, bodyOffsetY + 1);
		m_nodeWidgetPath.moveTo(line.p1());
		m_nodeWidgetPath.lineTo(line.p2());

		bodyRect.adjust(+m, +m, -m, -m);
		grp->setGroupGeometry(bodyRect);
	}

	return NewUMLRectWidget::attributeChange(change, oldValue);
}

