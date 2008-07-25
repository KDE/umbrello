/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "forkjoinwidget.h"

//app includes
#include "listpopupmenu.h"


/**
 * Constructs a ForkJoinWidget.
 * @param o The orientation of ForkJoinWidget.
 * @param id ID of the widget. (-1 for new id)
 */
ForkJoinWidget::ForkJoinWidget(Qt::Orientation o, Uml::IDType id)
    : BoxWidget(id),
	  m_orientation(o)
{
	m_baseType = Uml::wt_ForkJoin;
	setMargin(0);
	setBrush(QBrush(Qt::black));
}

/// Destructor
ForkJoinWidget::~ForkJoinWidget()
{
}

/// Sets the orientation of this widget to \a o
void ForkJoinWidget::setOrientation(Qt::Orientation o)
{
	m_orientation = o;
	updateGeometry();
}

/**
 * Reimplemented from NewUMLRectWidget::paint to draw the plate of
 * fork join.
 */
void ForkJoinWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->fillRect(rect(), brush());
}

/**
 * Reimplemented from NewUMLRectWidget::loadFromXMI to load widget
 * info from XMI element - 'forkjoin'.
 */
bool ForkJoinWidget::loadFromXMI(QDomElement& qElement)
{
    if ( !NewUMLRectWidget::loadFromXMI(qElement) ) {
        return false;
    }
    QString orientation = qElement.attribute("orientation",
											 QString::number(Qt::Horizontal));
    setOrientation( (Qt::Orientation)orientation.toInt() );
    return true;
}

/**
 * Reimplemented from NewUMLRectWidget::saveToXMI to save widget info
 * into XMI element - 'forkjoin'.
 */
void ForkJoinWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement fjElement = qDoc.createElement("forkjoin");
    NewUMLRectWidget::saveToXMI(qDoc, fjElement);
    fjElement.setAttribute("orientation", m_orientation);
    qElement.appendChild(fjElement);
}

/**
 * Reimplemented form NewUMLRectWidget::slotMenuSelection to handle
 * Flip action.
 */
void ForkJoinWidget::slotMenuSelection(QAction* action)
{
	// Menu is passed as parent of action
	ListPopupMenu *menu = qobject_cast<ListPopupMenu*>(action->parent());
	if (menu->getMenuType(action) == ListPopupMenu::mt_Flip) {
        setOrientation(m_orientation == Qt::Horizontal ?
					   Qt::Vertical : Qt::Horizontal);
	}
	else {
		BoxWidget::slotMenuSelection(action);
	}
}

/**
 * Reimplemented from NewUMLRectWidget::updateGeometry to update the
 * minimum and maximum size of the widget based on current
 * orientation.
 */
void ForkJoinWidget::updateGeometry()
{
	if(m_orientation == Qt::Horizontal) {
		setMinimumSize(QSizeF(40, 4));
		setMaximumSize(QSizeF(100, 10));
	}
	else {
		setMinimumSize(QSizeF(4, 40));
		setMaximumSize(QSizeF(10, 100));
	}

	BoxWidget::updateGeometry();
}

#include "forkjoinwidget.moc"
