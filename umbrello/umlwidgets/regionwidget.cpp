/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "regionwidget.h"

// app includes
#include "basictypes.h"
#include "debug_utils.h"

// kde includes
#include <KLocalizedString>

#define REGION_MARGIN 5
#define REGION_WIDTH 90
#define REGION_HEIGHT 45

/**
 * Creates a Region widget.
 *
 * @param scene   The parent of the widget.
 * @param id      The ID to assign (-1 will prompt a new ID.)
 */
RegionWidget::RegionWidget(UMLScene* scene, Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_Region, id)
{
}

/**
 * Destructor.
 */
RegionWidget::~RegionWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void RegionWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const int w = width();
    const int h = height();

    setPenFromSettings(painter);
    QPen pen = painter->pen();
    pen.setColor(Qt::red);
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);
    painter->drawRoundRect(0, 0, w, h, (h * 60) / w, 60);

    UMLWidget::paint(painter, option, widget);
}

/**
 * Overrides method from UMLWidget
 */
QSizeF RegionWidget::minimumSize() const
{
    int width = 10, height = 10;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    int textWidth = fm.width(name());

    height  = fontHeight;
    width   = textWidth > REGION_WIDTH?textWidth:REGION_WIDTH;
    height  = height > REGION_HEIGHT ? height : REGION_HEIGHT;
    width  += REGION_MARGIN * 2;
    height += REGION_MARGIN * 2;

    return QSizeF(width, height);
}

/**
 * Saves region widget to XMI element.
 */
void RegionWidget::SaveToXMI1(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement regionElement = qDoc.createElement(QLatin1String("regionwidget"));
    UMLWidget::SaveToXMI1(qDoc, regionElement);
    regionElement.setAttribute(QLatin1String("regionname"), name());
    regionElement.setAttribute(QLatin1String("documentation"), documentation());

    qElement.appendChild(regionElement);
}

/**
 * Loads region widget from XMI element.
 */
bool RegionWidget::loadFromXMI1(QDomElement& qElement)
{
    if (!UMLWidget::loadFromXMI1(qElement)) {
        return false;
    }
    setName(qElement.attribute(QLatin1String("regionname")));
    setDocumentation(qElement.attribute(QLatin1String("documentation")));
    return true;
}

