/***************************************************************************
 * Copyright (C) 2011 by Andi Fischer <andi.fischer@hispeed.ch>            *
 * Copyright (C) 2012 by Ralf Habacker <ralf.habacker@freenet.de>          *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "layoutgrid.h"

#include "debug_utils.h"
#include "umlscene.h"

#include <QPainter>
#include <QVarLengthArray>

/**
 * Constructor.
 */
LayoutGrid::LayoutGrid(UMLScene *scene)
  : m_scene(scene),
    m_gridSpacingX(25),
    m_gridSpacingY(25),
    m_gridDotColor(Qt::gray),
    m_isVisible(false)
{
    DEBUG_REGISTER_DISABLED("LayoutGrid");
}

/**
 * Destructor.
 */
LayoutGrid::~LayoutGrid()
{
}

void LayoutGrid::paint(QPainter *painter, const QRectF &rect)
{
    if (!isVisible())
        return;

    int gridSizeX = gridSpacingX();
    int gridSizeY = gridSpacingY();

    qreal left = int(rect.left()) - (int(rect.left()) % gridSizeX);
    qreal top = int(rect.top()) - (int(rect.top()) % gridSizeY);

    QVarLengthArray<QLineF, 200> lines;

    for (qreal x = left; x < rect.right(); x += gridSizeX)
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (qreal y = top; y < rect.bottom(); y += gridSizeY)
        lines.append(QLineF(rect.left(), y, rect.right(), y));

    painter->setPen(m_gridDotColor);
    painter->drawLines(lines.data(), lines.size());
}

int LayoutGrid::gridSpacingX() const
{
    return m_gridSpacingX;
}

int LayoutGrid::gridSpacingY() const
{
    return m_gridSpacingY;
}

void LayoutGrid::setGridSpacing(int sizeX, int sizeY)
{
    DEBUG("LayoutGrid") << "x = " << sizeX << " / y = " << sizeY;
    m_gridSpacingX= sizeX;
    m_gridSpacingY= sizeY;
}

const QColor& LayoutGrid::gridDotColor() const
{
    return m_gridDotColor;
}

void LayoutGrid::setGridDotColor(const QColor& color)
{
    DEBUG("LayoutGrid") << "color = " << color;
    m_gridDotColor = color;
}

bool LayoutGrid::isVisible() const
{
    return m_isVisible;
}

void LayoutGrid::setVisible(bool visible)
{
    if (m_isVisible != visible) {
        DEBUG("LayoutGrid") << "visible = " << visible;
        m_isVisible = visible;
        m_scene->update();
    }
}
