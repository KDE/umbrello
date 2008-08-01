/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
// own header
#include "floatingdashlinewidget.h"

//app includes
#include "listpopupmenu.h"
#include "umlscene.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "widgethandle.h"

//kde includes
#include <kinputdialog.h>
#include <klocale.h>

// qt includes
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QPainterPathStroker>

const qreal FloatingDashLineWidget::Margin = 5.;

/**
 * Constructs a horizontal FloatingDashLineWidget which is used in
 * CombinedFragmentWidget.
 */
FloatingDashLineWidget::FloatingDashLineWidget(QGraphicsItem *parent, Uml::IDType id)
    : NewUMLRectWidget(0, id),
      m_yMin(0),
      m_yMax(1000)
{
    m_baseType = Uml::wt_FloatingDashLine;
    setResizable(false);
    setParentItem(parent);
    createTextItemGroup();
}

/// Destructor
FloatingDashLineWidget::~FloatingDashLineWidget()
{
}

/**
 * Reimplement from NewUMLWidget::paint to draw a dash line and text
 * associated with this widget.
 */
void FloatingDashLineWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                   QWidget *)
{
    QLineF line(0, 0, size().width(), 0);

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

/// Sets minimum y. @ref yMin
void FloatingDashLineWidget::setYMin(qreal y)
{
    m_yMin = y;
    ensureConstraintRequirement();
}

/// Sets maximum y. @ref yMax
void FloatingDashLineWidget::setYMax(qreal y)
{
    m_yMax = y;
    ensureConstraintRequirement();
}

/**
 * Reimplemented from NewUMLWidget::loadFromXMI to load
 * FloatingDashLineWidget data from XMI element.
 */
bool FloatingDashLineWidget::loadFromXMI( QDomElement & qElement )
{
    if( !NewUMLWidget::loadFromXMI( qElement ) ) {
        return false;
    }

    m_yMax = qElement.attribute( "maxY", "" ).toDouble();
    m_yMin = qElement.attribute( "minY", "" ).toDouble();
    setName(qElement.attribute( "text", "" ));
    ensureConstraintRequirement();
    return true;
}

/**
 * Reimplemented from NewUMLWidget::saveToXMI to save widget data into
 * XMI element - 'floatingdashlinewidget'
 */
void FloatingDashLineWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement textElement = qDoc.createElement( "floatingdashlinewidget" );
    NewUMLWidget::saveToXMI( qDoc, textElement );
    textElement.setAttribute( "text", name() );
    textElement.setAttribute( "minY", m_yMin );
    textElement.setAttribute( "maxY", m_yMax );

    qElement.appendChild( textElement );
}

/**
 * Reimplemented from NewUMLWidget::slotMenuSelection to handle rename
 * action.
 */
void FloatingDashLineWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString text;

    // The menu is passed in as action's parent
    ListPopupMenu *menu = qobject_cast<ListPopupMenu*>(action->parent());
    ListPopupMenu::Menu_Type sel = menu->getMenuType(action);
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
        NewUMLWidget::slotMenuSelection(action);
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
    return NewUMLWidget::itemChange(change, value);
}

/**
 * Reimplemented from NewUMLWidget::updateGeometry to calculate the
 * minimum size.
 */
void FloatingDashLineWidget::updateGeometry()
{
    TextItemGroup *grp = textItemGroupAt(0);
    QSizeF minSize = grp->minimumSize();
    setMinimumSize(minSize);

    NewUMLRectWidget::updateGeometry();
}

/**
 * Reimplemented from NewUMLRectWidget to update the text.
 */
void FloatingDashLineWidget::updateTextItemGroups()
{
    TextItemGroup *grp = textItemGroupAt(0);
    grp->setTextItemCount(1);

    QString text = name();
    text.prepend('[');
    text.append(']');
    grp->textItemAt(0)->setText(text);

    NewUMLRectWidget::updateTextItemGroups();
}

/**
 * Reimplemented from NewUMLRectWidget::attributeChange to handle @ref
 * SizeHasChanged and to calculate the new bounding rect and shape.
 */
QVariant FloatingDashLineWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if (change == SizeHasChanged) {
        TextItemGroup *grp = textItemGroupAt(0);
        const QRectF grpRect(QPointF(Margin, Margin), grp->minimumSize());
        grp->setGroupGeometry(grpRect);

        QRectF boundRect = boundingRect();
        qreal adj = .5 * WidgetHandle::HandleSize;
        boundRect.adjust(-adj, -adj, +adj, 0);

        setBoundingRect(boundRect);

        QPainterPath shape;
        shape.moveTo(QPointF(0, 0));
        shape.lineTo(QPointF(size().width(), 0));

        QPainterPathStroker stroker;
        stroker.setWidth(lineWidth() + 5);
        QPainterPath stroke = stroker.createStroke(shape);

        stroke.addRect(grpRect);
        setShape(stroke);
        return QVariant();
    }

    return NewUMLRectWidget::attributeChange(change, oldValue);
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
