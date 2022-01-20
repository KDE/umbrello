/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>
    SPDX-FileCopyrightText: 2012 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layoutgrid.h"

#include "debug_utils.h"
#include "umlscene.h"

#include <QPainter>
#include <QVarLengthArray>

#define DBG_LOG() DEBUG_N(QLatin1String("LayoutGrid"))
DEBUG_REGISTER_DISABLED(LayoutGrid)

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
    DBG_LOG() << "x = " << sizeX << " / y = " << sizeY;
    m_gridSpacingX= sizeX;
    m_gridSpacingY= sizeY;
}

const QColor& LayoutGrid::gridDotColor() const
{
    return m_gridDotColor;
}

void LayoutGrid::setGridDotColor(const QColor& color)
{
    DBG_LOG() << "color = " << color;
    m_gridDotColor = color;
}

bool LayoutGrid::isVisible() const
{
    return m_isVisible;
}

void LayoutGrid::setVisible(bool visible)
{
    if (m_isVisible != visible) {
        DBG_LOG() << "visible = " << visible;
        m_isVisible = visible;
        m_scene->update();
    }
}
