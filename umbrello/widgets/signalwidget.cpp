/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "signalwidget.h"

// app includes
#include "debug_utils.h"
#include "listpopupmenu.h"
#include "textitem.h"
#include "textitemgroup.h"

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

// qt includes
#include <QtGui/QPolygonF>

/// Minimum size for signal widget.
const QSizeF SignalWidget::MinimumSize(50, 50);

/**
 * Creates a Signal widget.
 *
 * @param signalType        The type of Signal.
 * @param id                The ID to assign (-1 will prompt a new ID.)
 */
SignalWidget::SignalWidget(SignalType signalType, Uml::IDType id)
  : UMLWidget(WidgetBase::wt_Signal, id)
{
    m_signalType = signalType;
    createTextItemGroup();
}

/**
 * destructor
 */
SignalWidget::~SignalWidget()
{
}

/**
 * Reimplemented from UMLWidget::paint to draw signal widget.
 */
void SignalWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());
    painter->drawPath(m_signalPath);
}

/**
 * Returns the type of Signal.
 */
SignalWidget::SignalType SignalWidget::signalType() const
{
    return m_signalType;
}

/**
 * Returns the type string of Signal.
 */
QString SignalWidget::signalTypeStr() const
{
    return QLatin1String(ENUM_NAME(SignalWidget, SignalType, m_signalType));
}

/**
 * Sets the signaltype of widget to \a signalType.
 */
void SignalWidget::setSignalType( SignalType signalType )
{
    m_signalType = signalType;
    updateTextItemGroups();
}

/**
 * Reimplemented from UMLWidget::showPropertiesDialog to not to
 * show anything.
 */
void SignalWidget::showPropertiesDialog()
{
}

/**
 * Reimplemented from UMLWidget::loadFromXMI to load
 * SignalWidget from XMI.
 */
bool SignalWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    setName(qElement.attribute( "signalname", "" ));

    setDocumentation(qElement.attribute( "documentation", "" ));

    QString type = qElement.attribute( "signaltype", "" );
    setSignalType((SignalType)type.toInt());

   return true;
}

/**
 * Reimplemented from UMLWidget::saveToXMI to save SignalWidget
 * to XMI.
 */
void SignalWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement signalElement = qDoc.createElement( "signalwidget" );
    UMLWidget::saveToXMI( qDoc, signalElement );
    signalElement.setAttribute( "signalname", name() );
    signalElement.setAttribute( "documentation", documentation() );
    signalElement.setAttribute( "signaltype", m_signalType );
    qElement.appendChild( signalElement );
}

/**
 * Reimplemented from UMLWidget::slotMenuSelection to handle
 * rename action.
 */
void SignalWidget::slotMenuSelection(QAction* action)
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
        text = KInputDialog::getText( i18n("Enter signal name"),
                                      i18n("Enter the signal name :"),
                                      name(), &ok );
        if (ok && !text.isEmpty()) {
            setName(text);
        }
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Reimplemented from UMLWidget::updateGeometry to calculate
 * minimum size for signal widget based on the current signal type.
 */
void SignalWidget::updateGeometry()
{
    QSizeF minSize = textItemGroupAt(GroupIndex)->minimumSize();
    if (minSize.width() < SignalWidget::MinimumSize.width()) {
        minSize.setWidth(SignalWidget::MinimumSize.width());
    }

    if (m_signalType == SignalWidget::Accept) {
        // We need 1/3rd part for the '>' shape.(hollow or convex)
        minSize.rwidth() += minSize.width() / 3 + margin();
    }
    else if (m_signalType == SignalWidget::Send) {
        // Add one third for the '>' shape.
        minSize.rwidth() += minSize.width() / 3 + margin();
    }
    else if (m_signalType == SignalWidget::Time) {
        // In case of SignalWidget::Time add minimum height to
        // calculated as the text appears below drawing.
        minSize.rheight() += SignalWidget::MinimumSize.height();
    }

    setMinimumSize(minSize);

    UMLWidget::updateGeometry();
}

/**
 * Reimplemented from UMLWidget::updateTextItemGroups to update
 * the texts displayed.
 */
void SignalWidget::updateTextItemGroups()
{
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    grp->setTextItemCount(1); // Only name item

    TextItem *nameItem = grp->textItemAt(0);
    nameItem->setText(name());

    UMLWidget::updateTextItemGroups();
}

/**
 * Reimplemented from UMLWidget::attributeChange to handle
 * SizeHasChanged to position the texts and calculate the path to be
 * drawn based on current signal type.
 */
QVariant SignalWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if (change == SizeHasChanged) {
        QSizeF sz = size();
        TextItemGroup *grp = textItemGroupAt(GroupIndex);
        const QSizeF grpMinSize = grp->minimumSize();
        m_signalPath = QPainterPath(); // reset path

        if (m_signalType == SignalWidget::Send) {
            QPolygonF poly;

            qreal vShapeWidth = sz.width() - grpMinSize.width();
            // Ensure atmost one third of space is available at the
            // right end for '>' shape.
            vShapeWidth = qMin(vShapeWidth, sz.width() / 3.0);
            qreal textWidth = sz.width() - vShapeWidth;

            poly << QPointF(0,0)
                 << QPointF(textWidth, 0)
                 << QPointF(sz.width(), 0.5 * sz.height())
                 << QPointF(textWidth, sz.height())
                 << QPointF(0, sz.height())
                 << QPointF(0, 0);

            m_signalPath.addPolygon(poly);

            QRectF grpRect = rect();
            grpRect.setRight(textWidth);
            grp->setGroupGeometry(grpRect);
        }
        else if (m_signalType == SignalWidget::Accept) {
            QPolygonF poly;

            // This represents the sharp point of VShape.
            qreal vShapeX = sz.width() - grpMinSize.width();
            // VShape is atmost one third of widget width so that it
            // doesn't look ugly.
            vShapeX = qMin(vShapeX, sz.width() / 3.);

            poly << QPointF(0, 0)
                 << QPointF(vShapeX, .5 * sz.height())
                 << QPointF(0 , sz.height())
                 << QPointF(sz.width(), sz.height())
                 << QPointF(sz.width(), 0)
                 << QPointF(0, 0);

            m_signalPath.addPolygon(poly);
            QRectF grpRect(vShapeX, 0,
                           sz.width() - vShapeX, sz.height());
            grp->setGroupGeometry(grpRect);
        }
        else if (m_signalType == SignalWidget::Time) {
            QRectF polyRect(0, 0, sz.width(), sz.height() - grpMinSize.height());
            QPolygonF poly;
            poly << polyRect.topLeft()
                 << polyRect.bottomRight()
                 << polyRect.bottomLeft()
                 << polyRect.topRight()
                 << polyRect.topLeft();

            m_signalPath.addPolygon(poly);

            QRectF grpRect(0, polyRect.bottom(), polyRect.width(), grpMinSize.height());
            grp->setGroupGeometry(grpRect);
        }
    }

    return UMLWidget::attributeChange(change, oldValue);
}

#include "signalwidget.moc"
