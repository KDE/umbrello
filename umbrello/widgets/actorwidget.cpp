/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "actorwidget.h"

// local includes
#include "actor.h"
#include "textitemgroup.h"
#include "textitem.h"

const QSizeF ActorWidget::MinimumSize = QSizeF(20, 40);

/**
 * Constructs an ActorWidget.
 *
 * @param o The Actor class this ActorWidget will display.
 */
ActorWidget::ActorWidget(UMLActor *a) 
  : UMLWidget(WidgetBase::wt_Actor, a)
{
    createTextItemGroup();
    textItemGroupAt(0)->setMargin(0);
    setMargin(0);
}

/**
 * Destructor.
 */
ActorWidget::~ActorWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void ActorWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    painter->drawPath(m_actorPath);
}

/**
 * Saves the widget to the "actorwidget" XMI element.
 * Note: For loading from XMI, the inherited parent method is used.
 */
void ActorWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement actorElement = qDoc.createElement( "actorwidget" );
    UMLWidget::saveToXMI( qDoc, actorElement );
    qElement.appendChild( actorElement );
}

/**
 * Reimplemented to calculate minimum size based on the name text and
 * ActorWidget::MinimumActorSize
 */
void ActorWidget::updateGeometry()
{
    const int groupIndex = 0; // Only one group

    QSizeF minSize = textItemGroupAt(groupIndex)->minimumSize();
    if(minSize.width() < ActorWidget::MinimumSize.width()) {
        minSize.setWidth(ActorWidget::MinimumSize.width());
    }
    minSize.rheight() += ActorWidget::MinimumSize.height();
    setMinimumSize(minSize);

    UMLWidget::updateGeometry();
}

/**
 * Reimplemented from UMLWidget::attributeChange to handle
 * SizeHasChanged change.
 */
QVariant ActorWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldVal)
{
    if(change == SizeHasChanged) {
        // Calculate new path and position the text.
        const QSizeF sz = size();

        const int groupIndex = 0; // Only one group
        TextItemGroup *grp = textItemGroupAt(groupIndex);

        // First adjust the position of text and align it.
        qreal fontHeight = grp->minimumSize().height();

        QRectF r(0, rect().bottom() - fontHeight,
                 sz.width(), fontHeight);
        grp->setGroupGeometry(r);

        // Now calculate actorPath
        m_actorPath = QPainterPath();
        qreal actorHeight = r.top();
        qreal actorWidth = r.width();

        // Make sure width of actor isn't too much, it looks ugly otherwise.
        if(actorWidth > .5 * actorHeight) {
            actorWidth = .5 * actorHeight;
        }
        //TODO: Probably use above similar approach to limit height.

        QRectF headEllipse;
        headEllipse.setTopLeft(QPointF(.5 * (sz.width() - actorWidth), 0));
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
    }

    return UMLWidget::attributeChange(change, oldVal);
}

/**
 * Reimplemented from UMLWidget::updateTextItemGroups to update text content.
 */
void ActorWidget::updateTextItemGroups()
{
    const int groupIndex = 0; // Only one group
    TextItemGroup *grp = textItemGroupAt(groupIndex);
    grp->setTextItemCount(1);
    const int nameIndex = 0;
    grp->textItemAt(nameIndex)->setText(name());
    UMLWidget::updateTextItemGroups();
}
