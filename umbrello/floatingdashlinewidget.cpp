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

// own header
#include "floatingdashlinewidget.h"
#include "umlview.h"

// qt includes
#include <qpainter.h>

FloatingDashLineWidget::FloatingDashLineWidget(UMLView * view, Uml::IDType id)
: UMLWidget(view, id)
{
    UMLWidget::setBaseType(Uml::wt_FloatingDashLine);
    m_bResizable = false;
    m_text = "";
    updateComponentSize();
}

FloatingDashLineWidget::~FloatingDashLineWidget() {}

void FloatingDashLineWidget::draw(QPainter & p, int offsetX, int offsetY)
{
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    p.setPen(Qt::black);
    p.setFont(UMLWidget::getFont());
    p.drawText(getX() + FLOATING_DASH_LINE_TEXT_MARGIN, getY(),
                        getWidth() - FLOATING_DASH_LINE_TEXT_MARGIN * 2, fontHeight, Qt::AlignLeft, m_text);
    p.setPen(*(new QPen(UMLWidget::getLineColor(), 0, Qt::DashLine)));
    p.drawLine(getX(), getY(), getX() + getWidth(), getY());
    if(m_bSelected)
        drawSelected(&p, getX(), getY());
}

void FloatingDashLineWidget::setText(QString text)
{
    m_text = text;
}

void FloatingDashLineWidget::setY(int y)
{
    if(y >= m_yMin + FLOATING_DASH_LINE_MARGIN && y <= m_yMax - FLOATING_DASH_LINE_MARGIN)
        UMLWidget::setY(y);
}

void FloatingDashLineWidget::setYMin(int yMin)
{
    m_yMin = yMin;
}

void FloatingDashLineWidget::setYMax(int yMax)
{
    m_yMax = yMax;
}

int FloatingDashLineWidget::getYMin()
{
    return m_yMin;
}

int FloatingDashLineWidget::getDiffY()
{
    return (getY() - m_yMin);
}

void FloatingDashLineWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement textElement = qDoc.createElement( "floatingdashlinewidget" );
    UMLWidget::saveToXMI( qDoc, textElement );
    textElement.setAttribute( "text", m_Text );
    textElement.setAttribute( "y", m_y );
    textElement.setAttribute( "minY", m_yMin );
    textElement.setAttribute( "maxY", m_yMax );

    qElement.appendChild( textElement );
}

bool FloatingDashLineWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;

    m_yMax = qElement.attribute( "maxY", "" ).toInt();
    m_yMin = qElement.attribute( "minY", "" ).toInt();
    m_y = qElement.attribute( "y", "" ).toInt();
    m_Text = qElement.attribute( "text", "" );

}

#include "floatingdashlinewidget.moc"
