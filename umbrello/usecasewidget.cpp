/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "usecasewidget.h"
// system includes
#include <qpainter.h>
#include <kdebug.h>
// local includes
#include "usecase.h"
#include "umlview.h"


UseCaseWidget::UseCaseWidget(UMLView * view, UMLUseCase *o) : UMLWidget(view, o) {
    UMLWidget::setBaseType(Uml::wt_UseCase);
    //updateComponentSize();  Doing this during loadFromXMI() gives futile updates.
    //                  Instead, it is done afterwards by UMLWidget::activate()
}

UseCaseWidget::~UseCaseWidget() {}

void UseCaseWidget::draw(QPainter & p, int offsetX, int offsetY) {
    UMLWidget::setPen(p);
    if ( UMLWidget::getUseFillColour() )
        p.setBrush( UMLWidget::getFillColour() );
    QFont font = UMLWidget::getFont();
    font.setUnderline(false);
    font.setBold(false);
    font.setItalic( m_pObject->getAbstract() );
    p.setFont( font );
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int w = width();
    const int h = height();
    //int middleX = w / 2;
    const int textStartY = (h / 2) - (fontHeight / 2);

    p.drawEllipse(offsetX, offsetY, w, h);
    p.setPen(Qt::black);
    p.drawText(offsetX + UC_MARGIN, offsetY + textStartY, w - UC_MARGIN * 2, fontHeight, Qt::AlignCenter, getName());
    UMLWidget::setPen(p);
    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);
}

QSize UseCaseWidget::calculateSize() {
    const UMLWidget::FontType ft = ( m_pObject->getAbstract() ? FT_BOLD_ITALIC : FT_BOLD );
    const QFontMetrics &fm = UMLWidget::getFontMetrics(ft);
    const int fontHeight = fm.lineSpacing();
    const int textWidth = fm.width(getName());
    int width = textWidth > UC_WIDTH?textWidth:UC_WIDTH;
    int height = UC_HEIGHT + fontHeight + UC_MARGIN;

    width += UC_MARGIN * 2;

    return QSize(width, height);
}

void UseCaseWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement usecaseElement = qDoc.createElement( "usecasewidget" );
    UMLWidget::saveToXMI( qDoc, usecaseElement );
    qElement.appendChild( usecaseElement );
}

