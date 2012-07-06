/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
// own header
#include "floatingdashlinewidget.h"

//app includes
#include "debug_utils.h"
#include "listpopupmenu.h"
#include "umlscene.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "widgethandle.h"

//kde includes
#include <kinputdialog.h>
#include <klocale.h>

// qt includes
#include <QStyleOptionGraphicsItem>
#include <QPainterPathStroker>

/**
 * Constructs a horizontal FloatingDashLineWidget which is used in
 * CombinedFragmentWidget.
 */
FloatingDashLineWidget::FloatingDashLineWidget(QGraphicsItem *parent, Uml::IDType id)
  : UMLWidget(WidgetBase::wt_FloatingDashLine, id),
    m_yMin(0),
    m_yMax(1000)
{
    setResizable(false);
    setParentItem(parent);
    createTextItemGroup();
}

/**
 * Destructor.
 */
FloatingDashLineWidget::~FloatingDashLineWidget()
{
}

/**
 * Reimplement from UMLWidget::paint to draw a dash line and text
 * associated with this widget.
 */
void FloatingDashLineWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                   QWidget *)
{
    QLineF line(0, 0, width(), 0);

    QPen pen = QPen(lineColor(), lineWidth(), Qt::DashLine);
    painter->setPen(pen);
    painter->drawLine(line);

    // Draw handles if selected
    if (option->state & QStyle::State_Selected) {
        QRectF handleRect(0, 0, WidgetHandle::HandleSize, WidgetHandle::HandleSize);

        handleRect.moveCenter(line.p1());
        painter->fillRect(handleRect, Qt::blue);

        handleRect.moveCenter(line.p2());
        painter->fillRect(handleRect, Qt::blue);
    }
}

/**
 * Sets minimum y. @ref yMin.
 */
void FloatingDashLineWidget::setYMin(qreal y)
{
    m_yMin = y;
    ensureConstraintRequirement();
}

/**
 * Sets maximum y. @ref yMax.
 */
void FloatingDashLineWidget::setYMax(qreal y)
{
    m_yMax = y;
    ensureConstraintRequirement();
}

/**
 * Reimplemented from UMLWidget::loadFromXMI to load
 * FloatingDashLineWidget data from XMI element.
 */
bool FloatingDashLineWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) ) {
        return false;
    }

    m_yMax = qElement.attribute( "maxY", "" ).toDouble();
    m_yMax = mapToParent(mapFromScene(QPointF(0, m_yMax))).y();

    m_yMin = qElement.attribute( "minY", "" ).toDouble();
    m_yMin = mapToParent(mapFromScene(QPointF(0, m_yMin))).y();

    // y value is already loaded when pos is loaded by UMLWidget
    // so, no need to reload it even from older version saved file.
    setName(qElement.attribute( "text", "" ));
    ensureConstraintRequirement();
    return true;
}

/**
 * Reimplemented from UMLWidget::saveToXMI to save widget data into
 * XMI element - 'floatingdashlinewidget'
 */
void FloatingDashLineWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement textElement = qDoc.createElement( "floatingdashlinewidget" );
    UMLWidget::saveToXMI( qDoc, textElement );
    textElement.setAttribute( "text", name() );

    QGraphicsItem *parent = parentItem();
    Q_ASSERT(parent);
    qreal yMin = parent->mapToScene(QPointF(0, m_yMin)).y();
    qreal yMax = parent->mapToScene(QPointF(0, m_yMax)).y();
    textElement.setAttribute( "minY", yMin );
    textElement.setAttribute( "maxY", yMax );

    qElement.appendChild( textElement );
}

/**
 * Reimplemented from UMLWidget::slotMenuSelection to handle rename
 * action.
 */
void FloatingDashLineWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString text;

    ListPopupMenu *menu = ListPopupMenu::menuFromAction(action);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }
    ListPopupMenu::MenuType sel = menu->getMenuType(action);
    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        text = KInputDialog::getText( i18n("Enter alternative Name"),
                                      i18n("Enter the alternative :"),
                                      name(), &ok );
        if( ok && !text.isEmpty() ) {
            setName(text);
        }
        break;
    default:
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Reimplemented from QGraphicsItem::itemChange to handle @ref
 * ItemPositionChange notification.
 *
 * Care is taken to ensure "yMin < pos().y() < yMax".
 */
QVariant FloatingDashLineWidget::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange) {
        QPointF point = value.toPointF();

        qreal y = point.y();
        y = qMax(m_yMin, y);
        y = qMin(m_yMax, y);

        // Set x back to previous value if items are being moved.
        if (umlScene()->isMouseMovingItems()) {
            point.setX(pos().x());
        }

        point.setY(y);
        return point;
    }
    return UMLWidget::itemChange(change, value);
}

/**
 * Reimplemented from UMLWidget::updateGeometry to calculate the
 * minimum size.
 */
void FloatingDashLineWidget::updateGeometry()
{
    TextItemGroup *grp = textItemGroupAt(0);
    QSizeF minSize = grp->minimumSize();
    setMinimumSize(minSize);

    UMLWidget::updateGeometry();
}

/**
 * Reimplemented from UMLWidget to update the text.
 */
void FloatingDashLineWidget::updateTextItemGroups()
{
    TextItemGroup *grp = textItemGroupAt(0);
    grp->setTextItemCount(1);
    grp->setMargin(0);

    QString text = name();
    text.prepend('[');
    text.append(']');
    grp->textItemAt(0)->setText(text);

    UMLWidget::updateTextItemGroups();
}

/**
 * Reimplemented from UMLWidget::attributeChange to handle @ref
 * SizeHasChanged and to calculate the new bounding rect and shape.
 */
QVariant FloatingDashLineWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if (change == SizeHasChanged) {
        TextItemGroup *grp = textItemGroupAt(0);
        const QRectF grpRect(QPointF(0, 0), grp->minimumSize());
        grp->setGroupGeometry(grpRect);

        QRectF boundRect = boundingRect();
        qreal adj = .5 * WidgetHandle::HandleSize;
        boundRect.adjust(-adj, -adj, +adj, 0);

        setBoundingRect(boundRect);

        QPainterPath shape;
        shape.moveTo(QPointF(0, 0));
        shape.lineTo(QPointF(width(), 0));

        QPainterPathStroker stroker;
        stroker.setWidth(lineWidth() + 5);
        QPainterPath stroke = stroker.createStroke(shape);

        stroke.addRect(grpRect);
        setShape(stroke);
        return QVariant();
    }

    return UMLWidget::attributeChange(change, oldValue);
}

/**
 * Ensures that the line of this widget satisfies the requirement
 * yMin < pos().y() < yMax
 */
void FloatingDashLineWidget::ensureConstraintRequirement()
{
    QPointF newPos = itemChange(ItemPositionChange, pos()).toPointF();
    setPos(newPos);
}

#include "floatingdashlinewidget.moc"
