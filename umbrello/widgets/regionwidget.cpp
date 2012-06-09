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
#include "regionwidget.h"

/// Minimum size of region widget.
const QSizeF RegionWidget::MinimumSize(90, 45);

/**
 * Creates a Region widget.
 *
 * @param id   The ID to assign (-1 will prompt a new ID).
 */
RegionWidget::RegionWidget(Uml::IDType id)
  : UMLWidget(WidgetBase::wt_Region, id)
{
    setMinimumSize(RegionWidget::MinimumSize);
}

/**
 * Destructor.
 */
RegionWidget::~RegionWidget()
{
}

/**
 * Draws a rounded rect with dash line property.
 */
void RegionWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen(lineColor(), lineWidth());
    pen.setStyle(Qt::DashLine);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    const QRectF r = rect();
    painter->drawRoundRect(r, (r.height() * 60) / r.width(), 60);
}

/**
 * Saves region widget to XMI element.
 */
void RegionWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement regionElement = qDoc.createElement("regionwidget");
    UMLWidget::saveToXMI(qDoc, regionElement);
    regionElement.setAttribute("regionname", name());
    regionElement.setAttribute("documentation", documentation());

    qElement.appendChild(regionElement);
}

/**
 * Loads region widget from XMI element.
 */
bool RegionWidget::loadFromXMI(QDomElement& qElement)
{
    if (!UMLWidget::loadFromXMI(qElement)) {
        return false;
    }
    setName(qElement.attribute("regionname", ""));
    setDocumentation(qElement.attribute("documentation", ""));
    return true;
}
