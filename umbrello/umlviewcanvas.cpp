/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlviewcanvas.h"

// app includes
#include "umlview.h"

// qt/kde includes
#include <QtGui/QPainter>
#include <QtGui/QPalette>

/**
 * Constructor
 */
UMLViewCanvas::UMLViewCanvas(UMLView * pView,const Settings::OptionState& option) : Q3Canvas(pView)
{
    m_pView = pView;

    QPalette palette;
    setColors(palette.base().color(), option.uiState.gridDotColor);
    //setColors(option.uiState.backgroundColor, option.uiState.gridDotColor); //TODO Somehow, it does not work, it should
}

/**
 * Destructor.
 */
UMLViewCanvas::~UMLViewCanvas()
{
}

/**
 * Overrides default method.
 */
void UMLViewCanvas::setColors(const QColor& backColor, const QColor& gridColor)
{
    setBackgroundColor(backColor);
    m_gridColor = gridColor;
}

QColor UMLViewCanvas::gridDotColor() const
{
    return m_gridColor;
}

/**
 * Sets the color of the background and the grid dots.
 */
void UMLViewCanvas::drawBackground(QPainter & painter, const QRect & clip)
{
    Q_UNUSED(clip);
//?    UMLViewCanvas::drawBackground( painter, clip );
    if( m_pView->getShowSnapGrid() ) {
        painter.setPen( m_gridColor );
        int gridX = m_pView->getSnapX();
        int gridY = m_pView->getSnapY();
        int numX = width() / gridX;
        int numY = height() / gridY;
        for( int x = 0; x <= numX; x++ )
            for( int y = 0; y < numY; y++ )
                painter.drawPoint( x * gridX, y * gridY );
    }
}
