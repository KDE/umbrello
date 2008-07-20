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
#include "categorywidget.h"

// local includes
#include "category.h"
#include "listpopupmenu.h"

// qt includes
#include <QtGui/QPainter>

const qreal CategoryWidget::Margin = 5.;

/**
 *  Creates a Category widget.
 *
 *  @param  o The UMLCategory to represent.
 */
CategoryWidget::CategoryWidget(UMLCategory *o) : NewUMLRectWidget(o)
{
	m_baseType = Uml::wt_Category;
}

/// Destructor
CategoryWidget::~CategoryWidget()
{
}

/// Reimplemented from NewUMLRectWidget::sizeHint
QSizeF CategoryWidget::sizeHint(Qt::SizeHint which)
{
	if(which == Qt::MinimumSize) {
		return m_minimumSize;
	}
	return NewUMLRectWidget::sizeHint(which);
}

/// Reimplemented from NewUMLRectWidget::paint to draw this widget.
void CategoryWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
	if(!umlObject()) {
		uWarning() << "No UMLCategory object for this widget to paint";
		return;
	}

	QFont fnt = font();
	fnt.setItalic( umlObject()->getAbstract() );

	painter->setPen(QPen(lineColor(), lineWidth()));
	painter->setBrush(brush());
	painter->setFont(fnt);

	const QSizeF sz = size();
	const qreal radius = qMin(sz.width(), sz.height());

    // draw a circle
	QRectF circle(0, 0, radius, radius);
	// adjust circle to be at center (0 + width / 2, 0 + height / 2)
	circle.moveCenter(QPointF(sz.width(), sz.height()) * .5);

    painter->drawEllipse(circle);

	// Now draw the letter inside circle.
	painter->setPen(fontColor());

    QString letterType('D');
    switch( static_cast<UMLCategory*>( umlObject() )->getType() ) {
       case UMLCategory::ct_Disjoint_Specialisation:
           letterType = 'D';
           break;
       case UMLCategory::ct_Overlapping_Specialisation:
           letterType = 'O';
           break;
       case UMLCategory::ct_Union:
           letterType = 'U';
           break;
       default:
           break;
    }

    painter->drawText(circle, Qt::AlignCenter, letterType );
}

/// Reimplemented from NewUMLRectWidget::saveToXMI to save CategoyWidget
void CategoryWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement categoryElement = qDoc.createElement( "categorywidget" );
    NewUMLRectWidget::saveToXMI( qDoc, categoryElement );
    qElement.appendChild( categoryElement );
}

void CategoryWidget::updateGeometry()
{
	QFontMetrics fm(font());
	qreal minRadius = fm.lineSpacing() + CategoryWidget::Margin;
	m_minimumSize = QSizeF(minRadius, minRadius);

	NewUMLRectWidget::updateGeometry();
}

void CategoryWidget::sizeHasChanged(const QSizeF& oldSize)
{
	NewUMLRectWidget::sizeHasChanged(oldSize);
}

/**
 * Will be called when a menu selection has been made from the
 * popup menu.
 *
 * @param action    The action that has been selected.
 */
void CategoryWidget::slotMenuSelection(QAction* action)
{
    UMLCategory* catObj = static_cast<UMLCategory*>(umlObject());
	if(!catObj) {
		uWarning() << "No UMLCategory for this widget.";
		return;
	}

	// menu is passed in as parent .
    ListPopupMenu *menu = qobject_cast<ListPopupMenu*>(action->parent());
    ListPopupMenu::Menu_Type sel = menu->getMenuType(action);

    switch(sel) {
      case ListPopupMenu::mt_DisjointSpecialisation:
          catObj->setType(UMLCategory::ct_Disjoint_Specialisation);
          break;

      case ListPopupMenu::mt_OverlappingSpecialisation:
          catObj->setType(UMLCategory::ct_Overlapping_Specialisation);
          break;

      case ListPopupMenu::mt_Union:
          catObj->setType(UMLCategory::ct_Union);
          break;

      default:
          NewUMLRectWidget::slotMenuSelection(action);
    }
}

#include "categorywidget.moc"
