/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>
    SPDX-FileCopyrightText: 2012 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUTGRID_H
#define LAYOUTGRID_H

#include <QColor>
#include <QFont>

class UMLScene;
class QRectF;

/**
 * @author Andi Fischer
 * This class handles the layout grid, which is drawn in the background
 * of the scene. It is used only in UMLScene.
 */
class LayoutGrid
{
public:
    explicit LayoutGrid(UMLScene *scene);
    ~LayoutGrid();

    void paint(QPainter *painter, const QRectF &rect);
    
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

private:
    UMLScene           *m_scene;
    int                 m_gridSpacingX;
    int                 m_gridSpacingY;
    QColor              m_gridDotColor;
    bool                m_isVisible;
};

#endif  // LAYOUTGRID_H
