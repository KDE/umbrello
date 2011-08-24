/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "soc-umbrello-2011/entitywidget.h"
#include "umlview.h"
#include "umlobject.h"

#define ENTITY_MARGIN 5

namespace QGV {

EntityWidget::EntityWidget(UMLView* view, UMLObject* o): UMLWidget(view, o),
m_view(view), 
m_obj(o)
{

    UMLWidget::setBaseType(Uml::wt_Entity);
    setRect(qreal(cursor().pos().x()), qreal(cursor().pos().y()), 100, 30);
}



EntityWidget::~EntityWidget(){}

void EntityWidget::draw(QPainter& p, qreal offsetX, qreal offsetY)
{
    p.setPen(QPen(Qt::black));
    QFont font = UMLWidget::font();
    font.setBold(true);
    p.setFont(font);    
    
    qreal w = rect().width();
    qreal h = rect().height();   
    
    const QFontMetrics &fm = fontMetrics(FT_NORMAL);
    int fontHeight  = fm.lineSpacing();
    const QString name = this->name();    
    
    p.drawRect(offsetX, offsetY, w, h);  
    
    int y = 0;
    if ( !m_obj->stereotype().isEmpty() ) {
        p.drawText(offsetX + ENTITY_MARGIN, offsetY,
                   w - ENTITY_MARGIN * 2,fontHeight,
                   Qt::AlignCenter, m_obj->stereotype(true));
        font.setItalic( m_obj->isAbstract() );
        p.setFont(font);
        p.drawText(offsetX + ENTITY_MARGIN, offsetY + fontHeight,
                   w - ENTITY_MARGIN * 2, fontHeight, Qt::AlignCenter, name);
        font.setBold(false);
        font.setItalic(false);
        p.setFont(font);
        y = fontHeight * 2;
    } else {
        font.setItalic( m_obj->isAbstract() );
        p.setFont(font);
        p.drawText(offsetX + ENTITY_MARGIN, offsetY,
                   w - ENTITY_MARGIN * 2, fontHeight, Qt::AlignCenter, name);
        font.setBold(false);
        font.setItalic(false);
        p.setFont(font);

        y = fontHeight;
    }
    
    p.drawLine(offsetX, offsetY + 3, offsetX + w - 1, offsetY + 3);    
}



}

