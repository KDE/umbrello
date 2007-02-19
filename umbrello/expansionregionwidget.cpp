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
#include "expansionregionwidget.h"

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

#include "listpopupmenu.h"

ExpansionRegionWidget::ExpansionRegionWidget(UMLView * view, Uml::IDType id)
        : UMLWidget(view, id) {
    updateComponentSize();
}

ExpansionRegionWidget::~ExpansionRegionWidget() {}

void ExpansionRegionWidget::draw(QPainter & p, int offsetX, int offsetY) 
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

QSize ExpansionRegionWidget::calculateSize() {

    int width = 10, height = 10;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    int textWidth = fm.width(getName());

    height  = fontHeight;
    width   = textWidth > EXPANSION_REGION_WIDTH?textWidth:EXPANSION_REGION_WIDTH;
    height  = height > EXPANSION_REGION_HEIGHT ? height : EXPANSION_REGION_HEIGHT;
    width  += EXPANSION_REGION_MARGIN * 2;
    height += EXPANSION_REGION_MARGIN * 2;

    return QSize(width, height);
}

void ExpansionRegionWidget::setName(const QString &strName) {
    m_Text = strName;
    updateComponentSize();
    adjustAssocs( getX(), getY() );
}

QString ExpansionRegionWidget::getName() const {
    return m_Text;
}

bool ExpansionRegionWidget::showProperties() 
{
    return true;
}

void ExpansionRegionWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement regionElement = qDoc.createElement( "expansionregionwidget" );
    UMLWidget::saveToXMI( qDoc, regionElement );
    regionElement.setAttribute( "regionname", m_Text );
    regionElement.setAttribute( "documentation", m_Doc );

    qElement.appendChild( regionElement );
}

bool ExpansionRegionWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "regionname", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    return true;
}


#include "expansionregionwidget.moc"

