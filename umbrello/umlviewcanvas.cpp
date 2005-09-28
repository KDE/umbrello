/*
 *  copyright (C) 2002-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpainter.h>

#include "umlview.h"
#include "umlviewcanvas.h"

UMLViewCanvas::UMLViewCanvas( UMLView * pView ) : Q3Canvas( pView ) {
    m_pView = pView;
}

UMLViewCanvas::~UMLViewCanvas() {}

void UMLViewCanvas::drawBackground( QPainter & painter, const QRect & clip ) {
    Q3Canvas::drawBackground( painter, clip );
    if( m_pView -> getShowSnapGrid() ) {
        painter.setPen( Qt::gray );
        int gridX = m_pView -> getSnapX();
        int gridY = m_pView -> getSnapY();
        int numX = width() / gridX;
        int numY = height() / gridY;
        for( int x = 0; x <= numX; x++ )
            for( int y = 0; y < numY; y++ )
                painter.drawPoint( x * gridX, y * gridY );
    }
}

