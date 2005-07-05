/*
 *  copyright (C) 2003-2004
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

#include "artifactwidget.h"
#include "artifact.h"
#include "umlview.h"
#include <kdebug.h>
#include <qpainter.h>
#include <qpointarray.h>

ArtifactWidget::ArtifactWidget(UMLView *view, UMLArtifact *a) : UMLWidget(view, a) {
    init();
    setSize(100, 30);
    calculateSize();
}


void ArtifactWidget::init() {
    UMLWidget::setBaseType( Uml::wt_Artifact );
    m_pMenu = 0;
    // 	//maybe loading and this may not be set.
    // 	if (m_pObject) {
    calculateSize();
    update();
    // 	}
}

ArtifactWidget::~ArtifactWidget() {}

void ArtifactWidget::drawAsNormal(QPainter& p, int offsetX, int offsetY) {
    int w = width();
    int h = height();
    QFont font = UMLWidget::getFont();
    font.setBold(true);
    QFontMetrics &fm = getFontMetrics(FT_BOLD);
    int fontHeight  = fm.lineSpacing();
    QString name = getName();
    QString stereotype = m_pObject->getStereotype();

    p.drawRect(offsetX, offsetY, w, h);

    p.setPen( QPen(black) );
    p.setFont(font);

    if (!stereotype.isEmpty()) {
        p.drawText(offsetX + ARTIFACT_MARGIN, offsetY + (h/2) - fontHeight,
                   w, fontHeight, AlignCenter, stereotype);
    }

    int lines;
    if (!stereotype.isEmpty()) {
        lines = 2;
    } else {
        lines = 1;
    }

    if (lines == 1) {
        p.drawText(offsetX, offsetY + (h/2) - (fontHeight/2),
                   w, fontHeight, AlignCenter, name);
    } else {
        p.drawText(offsetX, offsetY + (h/2),
                   w, fontHeight, AlignCenter, name);
    }

    if(m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

void ArtifactWidget::drawAsFile(QPainter& p, int offsetX, int offsetY) {
    int w = width();
    int h = height();
    QFont font = UMLWidget::getFont();
    QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    int fontHeight  = fm.lineSpacing();
    QString name = getName();

    int startX = offsetX + (w/2) - 25;
    int iconHeight = h - fontHeight;
    QPointArray pointArray(5);
    pointArray.setPoint(0, startX, offsetY);
    pointArray.setPoint(1, startX + 40, offsetY);
    pointArray.setPoint(2, startX + 50, offsetY + 10);
    pointArray.setPoint(3, startX + 50, offsetY + iconHeight);
    pointArray.setPoint(4, startX, offsetY + iconHeight);
    p.drawPolygon(pointArray);

    p.drawLine(startX + 40, offsetY, startX + 40, offsetY + 10);
    p.drawLine(startX + 40, offsetY + 10, startX + 50, offsetY + 10);
    p.drawLine(startX + 40, offsetY, startX + 50, offsetY + 10);

    p.setPen( QPen(black) );
    p.setFont(font);

    p.drawText(offsetX, offsetY + h - fontHeight,
               w, fontHeight, AlignCenter, name);

    if(m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

void ArtifactWidget::drawAsLibrary(QPainter& p, int offsetX, int offsetY) {
    //FIXME this should have gears on it
    int w = width();
    int h = height();
    QFont font = UMLWidget::getFont();
    QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    int fontHeight  = fm.lineSpacing();
    QString name = getName();

    int startX = offsetX + (w/2) - 25;
    int iconHeight = h - fontHeight;
    QPointArray pointArray(5);
    pointArray.setPoint(0, startX, offsetY);
    pointArray.setPoint(1, startX + 40, offsetY);
    pointArray.setPoint(2, startX + 50, offsetY + 10);
    pointArray.setPoint(3, startX + 50, offsetY + iconHeight);
    pointArray.setPoint(4, startX, offsetY + iconHeight);
    p.drawPolygon(pointArray);

    p.drawLine(startX + 40, offsetY, startX + 40, offsetY + 10);
    p.drawLine(startX + 40, offsetY + 10, startX + 50, offsetY + 10);
    p.drawLine(startX + 40, offsetY, startX + 50, offsetY + 10);

    p.setPen( QPen(black) );
    p.setFont(font);

    p.drawText(offsetX, offsetY + h - fontHeight,
               w, fontHeight, AlignCenter, name);

    if(m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

void ArtifactWidget::drawAsTable(QPainter& p, int offsetX, int offsetY) {
    int w = width();
    int h = height();
    QFont font = UMLWidget::getFont();
    QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    int fontHeight  = fm.lineSpacing();
    QString name = getName();

    int startX = offsetX + (w/2) - 25;
    int iconHeight = h - fontHeight;

    p.drawRect(startX, offsetY, 50, h - fontHeight + 1);
    p.drawLine(startX + 20, offsetY, startX + 20, offsetY + iconHeight);
    p.drawLine(startX + 30, offsetY, startX + 30, offsetY + iconHeight);
    p.drawLine(startX + 40, offsetY, startX + 40, offsetY + iconHeight);
    p.drawLine(startX, offsetY + (iconHeight/2), startX + 49, offsetY + (iconHeight/2));
    p.drawLine(startX, offsetY + (iconHeight/2) + (iconHeight/4),
               startX + 49, offsetY + (iconHeight/2) + (iconHeight/4));

    QPen thickerPen = p.pen();
    thickerPen.setWidth(2);
    p.setPen(thickerPen);
    p.drawLine(startX + 10, offsetY, startX + 10, offsetY + iconHeight);
    p.drawLine(startX, offsetY + (iconHeight/4), startX + 50, offsetY + (iconHeight/4));

    p.setPen( QPen(black) );
    p.setFont(font);

    p.drawText(offsetX, offsetY + h - fontHeight,
               w, fontHeight, AlignCenter, name);

    if(m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

void ArtifactWidget::draw(QPainter& p, int offsetX, int offsetY) {
    UMLWidget::setPen(p);
    if ( UMLWidget::getUseFillColour() ) {
        p.setBrush( UMLWidget::getFillColour() );
    } else {
        p.setBrush( m_pView->viewport()->backgroundColor() );
    }

    UMLArtifact *umlart = static_cast<UMLArtifact*>(m_pObject);
    UMLArtifact::Draw_Type drawType = umlart->getDrawAsType();
    switch (drawType) {
    case UMLArtifact::defaultDraw:
        return drawAsNormal(p, offsetX, offsetY);
        break;
    case UMLArtifact::file:
        return drawAsFile(p, offsetX, offsetY);
        break;
    case UMLArtifact::library:
        return drawAsLibrary(p, offsetX, offsetY);
        break;
    case UMLArtifact::table:
        return drawAsTable(p, offsetX, offsetY);
        break;
    default:
        kdWarning() << "Artifact drawn as unknown type" << endl;
        break;
    }
}

QSize ArtifactWidget::calculateIconSize() {
    int width, height;

    QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    int fontHeight  = fm.lineSpacing();

    width = fm.width( m_pObject->getName() );

    width = width<50 ? 50 : width;

    height = 50 + fontHeight;

    setSize(width, height);
    adjustAssocs( getX(), getY() );//adjust assoc lines
    return QSize(width, height);
}

QSize ArtifactWidget::calculateNormalSize() {
    int width, height;

    QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    int fontHeight  = fm.lineSpacing();

    width = fm.width( m_pObject->getName() );

    int tempWidth = 0;
    if(!m_pObject->getStereotype().isEmpty()) {
        tempWidth = fm.width("«" + m_pObject->getStereotype() + "»");
    }
    width = tempWidth>width ? tempWidth : width;
    width += ARTIFACT_MARGIN * 2;

    height = (2*fontHeight) + (ARTIFACT_MARGIN * 2);

    setSize(width, height);
    adjustAssocs( getX(), getY() );//adjust assoc lines
    return QSize(width, height);
}

void ArtifactWidget::calculateSize() {
    if ( !m_pObject) {
        return;
    }
    UMLArtifact *umlart = static_cast<UMLArtifact*>(m_pObject);
    if (umlart->getDrawAsType() == UMLArtifact::defaultDraw) {
        calculateNormalSize();
    } else {
        calculateIconSize();
    }
}

bool ArtifactWidget::activate(IDChangeLog* ChangeLog /* = 0 */) {
    bool status = UMLWidget::activate(ChangeLog);
    if(status) {
        calculateSize();
    }
    return status;
}

void ArtifactWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement conceptElement = qDoc.createElement("artifactwidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

