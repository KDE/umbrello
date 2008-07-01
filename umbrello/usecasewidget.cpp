/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
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


UseCaseWidget::UseCaseWidget(UMLScene * view, UMLUseCase *o) : NewUMLRectWidget(view, o) {
    NewUMLRectWidget::setBaseType(Uml::wt_UseCase);
    //updateComponentSize();  Doing this during loadFromXMI() gives futile updates.
    //                  Instead, it is done afterwards by NewUMLRectWidget::activate()
}

UseCaseWidget::~UseCaseWidget() {}

void UseCaseWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
	QPainter &p = *painter;
	qreal offsetX = 0, offsetY = 0;

    setPenFromSettings(p);
    if ( NewUMLRectWidget::getUseFillColour() )
        p.setBrush( NewUMLRectWidget::getFillColour() );
    QFont font = NewUMLRectWidget::getFont();
    font.setUnderline(false);
    font.setBold(false);
    font.setItalic( umlObject()->getAbstract() );
    p.setFont( font );
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int w = getWidth();
    const int h = getHeight();
    //int middleX = w / 2;
    const int textStartY = (h / 2) - (fontHeight / 2);

    p.drawEllipse(offsetX, offsetY, w, h);
    p.setPen(Qt::black);
    p.drawText(offsetX + UC_MARGIN, offsetY + textStartY, w - UC_MARGIN * 2, fontHeight, Qt::AlignCenter, getName());
    setPenFromSettings(p);
    if(isSelected())
        drawSelected(&p, offsetX, offsetY);
}

QSizeF UseCaseWidget::calculateSize() {
    const NewUMLRectWidget::FontType ft = ( umlObject()->getAbstract() ? FT_BOLD_ITALIC : FT_BOLD );
    const QFontMetrics &fm = NewUMLRectWidget::getFontMetrics(ft);
    const int fontHeight = fm.lineSpacing();
    const int textWidth = fm.width(getName());
    int width = textWidth > UC_WIDTH?textWidth:UC_WIDTH;
    int height = UC_HEIGHT + fontHeight + UC_MARGIN;

    width += UC_MARGIN * 2;

    return QSizeF(width, height);
}

void UseCaseWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement usecaseElement = qDoc.createElement( "usecasewidget" );
    NewUMLRectWidget::saveToXMI( qDoc, usecaseElement );
    qElement.appendChild( usecaseElement );
}

