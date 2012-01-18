/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "usecasewidget.h"

// app includes
#include "usecase.h"
#include "umlview.h"

/**
 *  Creates a UseCase widget.
 *  @param  view   The parent of the widget.
 *  @param  o      The UMLObject to represent.
 */
UseCaseWidget::UseCaseWidget(UMLScene * scene, UMLUseCase *o)
  : UMLWidget(scene, WidgetBase::wt_UseCase, o)
{
}

/**
 * Destructor.
 */
UseCaseWidget::~UseCaseWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void UseCaseWidget::paint(QPainter & p, int offsetX, int offsetY)
{
    setPenFromSettings(p);
    if ( UMLWidget::useFillColor() )
        p.setBrush( UMLWidget::fillColor() );
    QFont font = UMLWidget::font();
    font.setUnderline(false);
    font.setBold(false);
    font.setItalic( m_pObject->isAbstract() );
    p.setFont( font );
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int w = width();
    const int h = height();
    //int middleX = w / 2;
    const int textStartY = (h / 2) - (fontHeight / 2);

    p.drawEllipse(offsetX, offsetY, w, h);
    p.setPen(textColor());
    p.drawText(offsetX + UC_MARGIN, offsetY + textStartY, w - UC_MARGIN * 2, fontHeight, Qt::AlignCenter, name());
    setPenFromSettings(p);
    if(m_selected)
        drawSelected(&p, offsetX, offsetY);
}

/**
 * Saves this UseCase to file.
 */
void UseCaseWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement usecaseElement = qDoc.createElement( "usecasewidget" );
    UMLWidget::saveToXMI( qDoc, usecaseElement );
    qElement.appendChild( usecaseElement );
}

/**
 * Overrides method from UMLWidget
 */
UMLSceneSize UseCaseWidget::minimumSize()
{
    const UMLWidget::FontType ft = ( m_pObject->isAbstract() ? FT_BOLD_ITALIC : FT_BOLD );
    const QFontMetrics &fm = UMLWidget::getFontMetrics(ft);
    const int fontHeight = fm.lineSpacing();
    const int textWidth = fm.width(name());
    int width = textWidth > UC_WIDTH?textWidth:UC_WIDTH;
    int height = UC_HEIGHT + fontHeight + UC_MARGIN;

    width += UC_MARGIN * 2;

    return UMLSceneSize(width, height);
}
