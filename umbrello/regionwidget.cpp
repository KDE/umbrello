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

RegionWidget::RegionWidget(UMLScene * view, Uml::IDType id)
        : NewUMLRectWidget(view, id) {
     NewUMLRectWidget::setBaseType( Uml::wt_Region );
    updateComponentSize();
}

RegionWidget::~RegionWidget() {}

void RegionWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
	QPainter &p = *painter;
	qreal offsetX = 0, offsetY = 0;

    setPenFromSettings(p);
    const int w = getWidth();
    const int h = getHeight();
    QPen pen = p.pen();
    {
        setPenFromSettings(p);
        pen.setColor ( Qt::red );
        pen.setStyle ( Qt::DashLine );
        p.setPen( pen );
        p.drawRoundRect(offsetX, offsetY, w, h, (h * 60) / w, 60);

    }
    if(isSelected())
        drawSelected(&p, offsetX, offsetY);
}

QSizeF RegionWidget::calculateSize() {

    int width = 10, height = 10;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    int textWidth = fm.width(getName());

    height  = fontHeight;
    width   = textWidth > REGION_WIDTH?textWidth:REGION_WIDTH;
    height  = height > REGION_HEIGHT ? height : REGION_HEIGHT;
    width  += REGION_MARGIN * 2;
    height += REGION_MARGIN * 2;

    return QSizeF(width, height);
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
    NewUMLRectWidget::saveToXMI( qDoc, regionElement );
    regionElement.setAttribute( "regionname", m_Text );
    regionElement.setAttribute( "documentation", documentation() );

    qElement.appendChild( regionElement );
}

bool RegionWidget::loadFromXMI( QDomElement & qElement ) {
    if( !NewUMLRectWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "regionname", "" );
    setDocumentation(qElement.attribute( "documentation", "" ));
    return true;
}


#include "regionwidget.moc"

