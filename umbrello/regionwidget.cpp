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
#include "regionwidget.h"

// qt includes
#include <qevent.h>
#include <QPolygon>

// kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>

// app includes
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "umlwidget.h"
#include "umlview.h"
#include "floatingtextwidget.h"

RegionWidget::RegionWidget(UMLView * view, Uml::IDType id)
        : UMLWidget(view, id) {
     UMLWidget::setBaseType( Uml::wt_Region );
    updateComponentSize();
}

RegionWidget::~RegionWidget() {}

void RegionWidget::draw(QPainter & p, int offsetX, int offsetY)
{
    UMLWidget::setPen(p);
    const int w = width();
    const int h = height();
    QPen pen = p.pen();
    {
        UMLWidget::setPen(p);
        pen.setColor ( Qt::red );
        pen.setStyle ( Qt::DashLine );
        p.setPen( pen );
        p.drawRoundRect(offsetX, offsetY, w, h, (h * 60) / w, 60);

    }
    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);
}

QSize RegionWidget::calculateSize() {

    int width = 10, height = 10;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    int textWidth = fm.width(getName());

    height  = fontHeight;
    width   = textWidth > REGION_WIDTH?textWidth:REGION_WIDTH;
    height  = height > REGION_HEIGHT ? height : REGION_HEIGHT;
    width  += REGION_MARGIN * 2;
    height += REGION_MARGIN * 2;

    return QSize(width, height);
}

void RegionWidget::setName(const QString &strName) {
    m_Text = strName;
    updateComponentSize();
    adjustAssocs( getX(), getY() );
}

QString RegionWidget::getName() const {
    return m_Text;
}


void RegionWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement regionElement = qDoc.createElement( "regionwidget" );
    UMLWidget::saveToXMI( qDoc, regionElement );
    regionElement.setAttribute( "regionname", m_Text );
    regionElement.setAttribute( "documentation", m_Doc );

    qElement.appendChild( regionElement );
}

bool RegionWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "regionname", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    return true;
}


#include "regionwidget.moc"

