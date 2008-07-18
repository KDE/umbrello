/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "actorwidget.h"

// local includes
#include "actor.h"
#include "textitemgroup.h"
#include "textitem.h"

// qt includes
#include <QtGui/QPainter>

const QSizeF ActorWidget::MinimumActorSize = QSizeF(20, 40);
const qreal ActorWidget::Margin = 5;

/**
 * Constructs an ActorWidget.
 *
 * @param o The Actor class this ActorWidget will display.
 */
ActorWidget::ActorWidget(UMLActor *a) : NewUMLRectWidget(a)
{
	m_baseType = Uml::wt_Actor;
	m_textItemGroup = new TextItemGroup(this);
}

/// Destructor
ActorWidget::~ActorWidget()
{
	delete m_textItemGroup;
}

/// Reimplemented to return size hint corresponding to ActorWidget.
QSizeF ActorWidget::sizeHint(Qt::SizeHint which)
{
    if(which == Qt::MinimumSize) {
		return m_minimumSize;
	}
	return NewUMLRectWidget::sizeHint(which);
}

/**
 * Overrides the standard paint event.
 */
void ActorWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
	painter->setPen(QPen(lineColor(), lineWidth()));
	painter->setBrush(brush());

	painter->drawPath(m_actorPath);
}

/// Saves the widget to the "actorwidget" XMI element.
void ActorWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement actorElement = qDoc.createElement( "actorwidget" );
    NewUMLRectWidget::saveToXMI( qDoc, actorElement );
    qElement.appendChild( actorElement );
}

/**
 * Reimplemented to calculate minimum size based on the name text and
 * ActorWidget::MinimumActorSize
 */
void ActorWidget::updateGeometry()
{
	if(umlObject()) {
		const int totalItemCount = 1; // Only name text
		const int nameItemIndex = 0;
		m_textItemGroup->ensureTextItemCount(totalItemCount);

		TextItem *nameItem = m_textItemGroup->textItemAt(nameItemIndex);
		nameItem->setDefaultTextColor(fontColor());
		nameItem->setFont(font());
		nameItem->setAlignment(Qt::AlignCenter);
		nameItem->setBackgroundBrush(Qt::NoBrush);
		nameItem->setText(name());

		// Calculate minimum size.
		m_minimumSize = m_textItemGroup->calculateMinimumSize();
		m_minimumSize.rheight() += ActorWidget::MinimumActorSize.height();
		m_minimumSize.rheight() += 2 * ActorWidget::Margin;

		m_minimumSize.rwidth() = qMax(m_minimumSize.width(),
									  ActorWidget::MinimumActorSize.width());
		m_minimumSize.rwidth() += 2 * ActorWidget::Margin;
	}
	NewUMLRectWidget::updateGeometry();
}

/**
 * Reimplemented to calculated new shape based on current size and
 * also align the actor text.
 */
void ActorWidget::sizeHasChanged(const QSizeF& oldSize)
{
	const QSizeF sz = size();

	// First adjust the position of text and align it.
	const int NameIndex = 0;
	qreal fontHeight = m_textItemGroup->textItemAt(NameIndex)->height();
	QPointF offset(ActorWidget::Margin,
				   sz.height() - fontHeight - ActorWidget::Margin);
	m_textItemGroup->setPos(offset);
	QSizeF groupCurSize = QSizeF(sz.width() - 2 * ActorWidget::Margin,
								 fontHeight);
	m_textItemGroup->alignVertically(groupCurSize);

	// Calculate actor path again.
	m_actorPath = QPainterPath();
	qreal actorHeight = sz.height() - fontHeight -
		2 * ActorWidget::Margin;
	qreal actorWidth = sz.width() - 2 * ActorWidget::Margin;

	// Make sure width of actor isn't too much, it looks ugly otherwise.
	if(actorWidth > .5 * actorHeight) {
		actorWidth = .5 * actorHeight;
	}

	//TODO: Probably use above similar approach to limit height.
	QRectF headEllipse;
	headEllipse.setTopLeft(QPointF(.5 * (sz.width() - actorWidth), ActorWidget::Margin));
	headEllipse.setSize(QSizeF(actorWidth, actorHeight / 3));
	m_actorPath.addEllipse(headEllipse);

	QLineF bodyLine(.5 * sz.width(), headEllipse.bottom(),
						.5 * sz.width(), (2. / 3.) * actorHeight);
	m_actorPath.moveTo(bodyLine.p1());
	m_actorPath.lineTo(bodyLine.p2());

	QLineF leftLeg(.5 * sz.width(), bodyLine.p2().y(),
				   headEllipse.left(), actorHeight);
	m_actorPath.moveTo(leftLeg.p1());
	m_actorPath.lineTo(leftLeg.p2());

	QLineF rightLeg(.5 * sz.width(), bodyLine.p2().y(),
				   headEllipse.right(), actorHeight);
	m_actorPath.moveTo(rightLeg.p1());
	m_actorPath.lineTo(rightLeg.p2());

	QLineF arms(headEllipse.left(), .5 * actorHeight,
				headEllipse.right(), .5 * actorHeight);
	m_actorPath.moveTo(arms.p1());
	m_actorPath.lineTo(arms.p2());

	NewUMLRectWidget::sizeHasChanged(oldSize);
}
