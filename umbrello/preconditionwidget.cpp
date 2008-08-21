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

// own header
#include "preconditionwidget.h"

// app includes
#include "listpopupmenu.h"
#include "objectwidget.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "umlscene.h"

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

/**
 * Creates a Precondition widget.
 *
 * @param a  The role A widget for this precondition.
 * @param id The ID to assign (-1 will prompt a new ID.)
 */
PreconditionWidget::PreconditionWidget( ObjectWidget* a, Uml::IDType id )
    : NewUMLRectWidget(0, id),
      m_objectWidget(a)
{
    m_baseType = Uml::wt_Precondition;
    setIgnoreSnapToGrid(true);
    setIgnoreSnapComponentSizeToGrid(true);

    // Make the ObjectWidget the parent, so that they move in unison.
    setParentItem(a);

    // Create a textitem group to display the precondition text.
    createTextItemGroup();
}

/// Destructor
PreconditionWidget::~PreconditionWidget()
{
}

/**
 * @return The 'y' position in terms of the parent item coordinates
 *         i.e - the ObjectWidget, that can at minimum be set to this
 *         widget.
 *
 * Uses ObjectWidget to calculate this minimum.
 */
qreal PreconditionWidget::minY() const
{
    if (!m_objectWidget) {
        return 0;
    }

    return m_objectWidget->rect().bottom() + margin();
}

/**
 * @return The 'y' position in terms of the parent item coordinates
 *         i.e - the ObjectWidget, that can at maximum be set to this
 *         widget.
 *
 * Uses ObjectWidget to calculate this maximum.
 */
qreal PreconditionWidget::maxY() const
{
    if (!m_objectWidget) {
        return 1000;
    }

    return m_objectWidget->lineEndYInParentCoords() - margin();
}

/**
 * Reimplemented from NewUMLRectWidget::paint to draw the rounded
 * rect. The text is drawn by the TextItem.
 */
void PreconditionWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const QRectF r = rect();
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    painter->drawRoundedRect(r, r.height() * 60, r.width());
}

/**
 * Reimplemented from NewUMLRectWidget::loadFromXMI to load
 * PreconditionWidget data from XMI into this widget.
 */
bool PreconditionWidget::loadFromXMI( QDomElement & qElement )
{
    if( !NewUMLRectWidget::loadFromXMI( qElement ) )
        return false;
    QString widgetaid = qElement.attribute( "widgetaid", "-1" );
    setName(qElement.attribute( "preconditionname", "" ));
    setDocumentation(qElement.attribute( "documentation", "" ));

    Uml::IDType aId = STR2ID(widgetaid);

    m_objectWidget = dynamic_cast<ObjectWidget*>(umlScene()->findWidget( aId ));
    if (!m_objectWidget) {
        uDebug() << "role A widget " << ID2STR(aId) << " is not an ObjectWidget";
        return false;
    }

    setParentItem(m_objectWidget);
    updateGeometry(); // ensure constraint satisfaction

    return true;
}

/**
 * Reimplemented form NewUMLRectWidget::saveToXMI to save this
 * PreconditionWidget data into 'preconditionwidget' XMI element.
 */
void PreconditionWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement preconditionElement = qDoc.createElement( "preconditionwidget" );
    NewUMLRectWidget::saveToXMI( qDoc, preconditionElement );

    preconditionElement.setAttribute( "widgetaid", ID2STR(m_objectWidget->localID()) );
    preconditionElement.setAttribute( "preconditionname", name() );
    preconditionElement.setAttribute( "documentation", documentation() );
    qElement.appendChild( preconditionElement );
}

/**
 * Reimplemented from NewUMLRectWidget::updateGeometry to calculate
 * minimum size.
 */
void PreconditionWidget::updateGeometry()
{
    setMinimumSize(textItemGroupAt(0)->minimumSize());
    NewUMLRectWidget::updateGeometry();
}

/**
 * Reimplemented from NewUMLRectWidget::updateTextItemGroups to update
 * the text content dispaly.
 */
void PreconditionWidget::updateTextItemGroups()
{
    TextItemGroup *grp = textItemGroupAt(0);
    grp->setTextItemCount(1);

    QString text = name();
    text.prepend("{ ");
    text.append(" }");

    grp->textItemAt(0)->setText(text);

    NewUMLRectWidget::updateTextItemGroups();
}

/**
 * Reimplemented from NewUMLRectWidget::attributeChange to handle @ref
 * SizeHasChanged to align the text in the widget area.
 */
QVariant PreconditionWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if (change == SizeHasChanged) {
        const qreal m = margin();
        QRectF r = rect();
        r.adjust(+m, +m, -m, -m);
        textItemGroupAt(0)->setGroupGeometry(r);

        // Center the precondtion widget horizontally on sequential line.
        if (m_objectWidget) {
            QRectF objectRect = m_objectWidget->rect();
            setX(objectRect.center().x() - .5 * size().width());
        }
    }
    return NewUMLRectWidget::attributeChange(change, oldValue);
}

/**
 * Reimplemented from NewUMLRectWidget::itemChange to ensure
 * X-centered and Y-limited constraints on this PreconditionWidget.
 */
QVariant PreconditionWidget::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange) {
        QPointF newPos = value.toPointF();

        if (m_objectWidget) {
            QRectF objectRect = m_objectWidget->rect();
            newPos.setX(objectRect.center().x() - .5 * size().width());
        }

        newPos.ry() = qMax(newPos.y(), minY());
        newPos.ry() = qMin(newPos.y(), maxY());

        return newPos;
    }

    return NewUMLRectWidget::itemChange(change, value);
}

/**
 * Reimplemented from NewUMLRectWidget::slotMenuSelection to handle
 * Rename action.
 */
void PreconditionWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString text = name();

    // The menu is passed in as action's parent
    ListPopupMenu *menu = qobject_cast<ListPopupMenu*>(action->parent());
    ListPopupMenu::Menu_Type sel = menu->getMenuType(action);
    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        text = KInputDialog::getText( i18n("Enter Precondition Name"),
                                      i18n("Enter the precondition :"),
                                      text, &ok );
        if( ok && !text.isEmpty() ) {
            setName(text);
        }
        break;

    default:
        NewUMLRectWidget::slotMenuSelection(action);
    }
}

#include "preconditionwidget.moc"
