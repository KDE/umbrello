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
#include "signalwidget.h"

// app includes
#include "listpopupmenu.h"
#include "textitem.h"
#include "textitemgroup.h"

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

// qt includes
#include <QtGui/QPolygonF>

/// Minimum size for signal widget.
const QSizeF SignalWidget::MinimumSize(45,15);

/**
 * Creates a Signal widget.
 *
 * @param signalType        The type of Signal.
 * @param id                The ID to assign (-1 will prompt a new ID.)
 */
SignalWidget::SignalWidget(SignalType signalType, Uml::IDType id)
	: NewUMLRectWidget(0, id)
{
    m_baseType = Uml::wt_Signal;
    m_signalType = signalType;
	createTextItemGroup();
}

/// Destructor
SignalWidget::~SignalWidget()
{
}

/**
 * Reimplemented from NewUMLRectWidget::paint to draw signal widget.
 */
void SignalWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	painter->setPen(QPen(lineColor(), lineWidth()));
	painter->setBrush(brush());
	painter->drawPath(m_signalPath);
}

/// Sets the signaltype of widget to \a signalType
void SignalWidget::setSignalType( SignalType signalType )
{
    m_signalType = signalType;
	updateTextItemGroups();
}

/**
 * Reimplemented from NewUMLRectWidget::showPropertiesDialog to not to
 * show anything.
 */
void SignalWidget::showPropertiesDialog()
{
}

/**
 * Reimplemented from NewUMLRectWidget::loadFromXMI to load
 * SignalWidget from XMI.
 */
bool SignalWidget::loadFromXMI( QDomElement & qElement )
{
    if( !NewUMLRectWidget::loadFromXMI( qElement ) )
        return false;
    setName(qElement.attribute( "signalname", "" ));

	setDocumentation(qElement.attribute( "documentation", "" ));

	QString type = qElement.attribute( "signaltype", "" );
    setSignalType((SignalType)type.toInt());

   return true;
}

/**
 * Reimplemented from NewUMLRectWidget::saveToXMI to save SignalWidget
 * to XMI.
 */
void SignalWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement signalElement = qDoc.createElement( "signalwidget" );
    NewUMLRectWidget::saveToXMI( qDoc, signalElement );
    signalElement.setAttribute( "signalname", name() );
    signalElement.setAttribute( "documentation", documentation() );
    signalElement.setAttribute( "signaltype", m_signalType );
    qElement.appendChild( signalElement );
}

/**
 * Reimplemented from NewUMLRectWidget::slotMenuSelection to handle
 * rename action.
 */
void SignalWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString text;

	// ListPopupMenu is passed as parent of action
	ListPopupMenu *menu = qobject_cast<ListPopupMenu*>(action->parent());
    ListPopupMenu::Menu_Type sel = menu->getMenuType(action);

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
        NewUMLRectWidget::slotMenuSelection(action);
    }
}

/**
 * Reimplemented from NewUMLRectWidget::updateGeometry to calculate
 * minimum size for signal widget based on the current signal type.
 */
void SignalWidget::updateGeometry()
{
	QSizeF minSize = textItemGroupAt(GroupIndex)->minimumSize();
	if(minSize.width() < SignalWidget::MinimumSize.width()) {
		minSize.setWidth(SignalWidget::MinimumSize.width());
	}
	// We need 1/3rd part for the rotated 'V' shape.
	if(m_signalType == SignalWidget::Accept) {
		minSize.rwidth() += minSize.width() / 3 + margin();
	}

	// In case of SignalWidget::Time add minimum height to calculated
	// as the text appears below drawing.
	if(m_signalType == SignalWidget::Time) {
		minSize.rheight() += SignalWidget::MinimumSize.height();
	}
	else {
		// Otherwise assure minimum height >= MinimumSize.height()
		if(minSize.height() < SignalWidget::MinimumSize.height()) {
			minSize.setHeight(SignalWidget::MinimumSize.height());
		}
	}
	setMinimumSize(minSize);

	NewUMLRectWidget::updateGeometry();
}

/**
 * Reimplemented from NewUMLRectWidget::updateTextItemGroups to update
 * the texts displayed.
 */
void SignalWidget::updateTextItemGroups()
{
	TextItemGroup *grp = textItemGroupAt(GroupIndex);
	grp->setTextItemCount(1); // Only name item

	TextItem *nameItem = grp->textItemAt(0);
	nameItem->setText(name());

	NewUMLRectWidget::updateTextItemGroups();
}

/**
 * Reimplemented from NewUMLRectWidget::attributeChange to handle
 * SizeHasChanged to position the texts and calculate the path to be
 * drawn based on current signal type.
 */
QVariant SignalWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
	if (change == SizeHasChanged) {
		QSizeF sz = size();
		TextItemGroup *grp = textItemGroupAt(GroupIndex);
		const qreal m = margin();
		m_signalPath = QPainterPath(); // reset path

		if (m_signalType == SignalWidget::Send) {
			QPolygonF poly;
			poly << QPointF(0,0)
				 << QPointF((sz.width() * 2) / 3, 0)
				 << QPointF(sz.width(), 0.5 * sz.height())
				 << QPointF(sz.width() * 2 / 3., sz.height())
				 << QPointF(0, sz.height())
				 << QPointF(0, 0);

			m_signalPath.addPolygon(poly);
			grp->setGroupGeometry(rect().adjusted(+m, +m, -m, -m));
		}
		else if (m_signalType == SignalWidget::Accept) {
			QPolygonF poly;
			poly << QPointF(0, 0)
				 << QPointF(sz.width() / 3, .5 * sz.height())
				 << QPointF(0 , sz.height())
				 << QPointF(sz.width(), sz.height())
				 << QPointF(sz.width(), 0)
				 << QPointF(0, 0);

			m_signalPath.addPolygon(poly);
			QRectF grpRect(sz.width() / 3, m,
						   2 * sz.width() / 3, sz.height() - 2 * m);
			grp->setGroupGeometry(grpRect);
		}
		else {
			qreal grpMinHeight = grp->minimumSize().height();
			QSizeF polySize(sz.width(),
							sz.height() - grpMinHeight - 2 * m);
			QPolygonF poly;
			poly << QPointF(0, 0)
				 << QPointF(polySize.width(), polySize.height())
				 << QPointF(0,                polySize.height())
				 << QPointF(polySize.width(), 0)
				 << QPointF(0, 0);

			m_signalPath.addPolygon(poly);

			QRectF grpRect(m, polySize.height() + m,
						   polySize.width() - 2 * m,
						   grpMinHeight);
			grp->setGroupGeometry(grpRect);
        }
	}

	return NewUMLRectWidget::attributeChange(change, oldValue);
}

#include "signalwidget.moc"
