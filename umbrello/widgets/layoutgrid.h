/***************************************************************************
 * Copyright (C) 2011 by Andi Fischer <andi.fischer@hispeed.ch>            *
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

#ifndef LAYOUTGRID_H
#define LAYOUTGRID_H

#include <QColor>
#include <QFont>
#include <QGraphicsItem>

class QGraphicsScene;
class QStyleOptionGraphicsItem;

/**
 * @author Andi Fischer
 * This class handles the layout grid, which is drawn in the background
 * of the scene. It is used only in UMLScene.
 */
class LayoutGrid : public QGraphicsItem
{
public:
    LayoutGrid(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    ~LayoutGrid();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

    QRect gridRect() const;
    void setGridRect(const QRect& rect);

    int gridSpacingX() const;
    int gridSpacingY() const;
    void setGridSpacing(int sizeX, int sizeY);

    const QColor& gridDotColor() const;
    void setGridDotColor(const QColor& color);

    const QColor& gridCrossColor() const;
    void setGridCrossColor(const QColor& color);

    const QColor& textColor() const;
    void setTextColor(const QColor& color);

    QFont textFont() const;
    void setTextFont(const QFont& font);

    bool isVisible() const;
    void setVisible(bool visible);

    bool isTextVisible() const;
    void setTextVisible(bool visible);

protected:
     void mousePressEvent(QGraphicsSceneMouseEvent *event);
     void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    int                 m_gridSpacingX;
    int                 m_gridSpacingY;
    QRect               m_gridRect;
    QFont               m_textFont;
    QColor              m_gridDotColor;
    QColor              m_gridCrossColor;
    QColor              m_textColor;
    bool                m_isVisible;
    bool                m_isTextVisible;
    QGraphicsTextItem*  m_coordLabel;
};

#endif  // LAYOUTGRID_H
