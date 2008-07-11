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
#include "boxwidget.h"

/**
 * @class BoxWidget
 *
 * Displays a rectangular box.  These widgets are diagram specific.
 * They will still need a unique id from the @ref UMLDoc class for
 * deletion and other purposes.
 *
 * @short Displays a box.
 * @author Jonathan Riddell
 * @see NewUMLRectWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */


/**
 * Constructs a BoxWidget.
 * @param id The ID to assign (-1 will prompt a new ID.)
 */
BoxWidget::BoxWidget(Uml::IDType id)
    : NewUMLRectWidget(0, id),
      m_minimumSize(100, 80)
{
    m_baseType = Uml::wt_Box;
    setLineColor(Qt::black);
}

/**
 * destructor
 */
BoxWidget::~BoxWidget()
{
}

/**
 * Draws a rectangle.
 */
void BoxWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(Qt::NoBrush);

    painter->drawRect(rect());
}

/**
 * Saves the widget to the "boxwidget" XMI element.
 * @note For loading from XMI, the inherited parent method is used.
 */
void BoxWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement boxElement = qDoc.createElement("boxwidget");
    NewUMLRectWidget::saveToXMI(qDoc, boxElement);
    qElement.appendChild(boxElement);
}
