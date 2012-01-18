/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "artifactwidget.h"

// app includes
#include "artifact.h"
#include "debug_utils.h"
#include "umlview.h"

/**
 * Constructs a ArtifactWidget.
 *
 * @param scene              The parent of this ArtifactWidget.
 * @param a         The Artifact this widget will be representing.
 */
ArtifactWidget::ArtifactWidget(UMLScene *scene, UMLArtifact *a)
  : UMLWidget(scene, WidgetBase::wt_Artifact, a)
{
    setSize(100, 30);
}

/**
 * Destructor.
 */
ArtifactWidget::~ArtifactWidget()
{
}

/**
 * Reimplemented to paint the articraft widget. Some part of specific
 * drawing is delegeted to private method like drawAsFile..
 */
void ArtifactWidget::paint(QPainter& p, int offsetX, int offsetY)
{
    UMLWidget::setPenFromSettings(p);
    if ( UMLWidget::useFillColor() ) {
        p.setBrush( UMLWidget::fillColor() );
    } else {
        p.setBrush( m_scene->viewport()->palette().color(QPalette::Background) );
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
        uWarning() << "Artifact drawn as unknown type";
        break;
    }
}

/**
 * Reimplemented from WidgetBase::saveToXMI to save the widget to
 * the "artifactwidget" XMI element.
 */
void ArtifactWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("artifactwidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

/**
 * Overrides method from UMLWidget.
 */
UMLSceneSize ArtifactWidget::minimumSize()
{
    if ( !m_pObject) {
        return UMLWidget::minimumSize();
    }
    UMLArtifact *umlart = static_cast<UMLArtifact*>(m_pObject);
    if (umlart->getDrawAsType() == UMLArtifact::defaultDraw) {
        return calculateNormalSize();
    } else {
        return calculateIconSize();
    }
}

/**
 * calculates the size when drawing as an icon (it's the same size for all icons)
 */
QSize ArtifactWidget::calculateIconSize()
{
    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight  = fm.lineSpacing();

    int width = fm.width( m_pObject->name() );

    width = width<50 ? 50 : width;

    int height = 50 + fontHeight;

    return QSize(width, height);
}

/**
 * calculates the size for drawing as a box
 */
QSize ArtifactWidget::calculateNormalSize()
{
    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight  = fm.lineSpacing();

    int width = fm.width( m_pObject->name() );

    int tempWidth = 0;
    if(!m_pObject->stereotype().isEmpty()) {
        tempWidth = fm.width( m_pObject->stereotype(true) );
    }
    width = tempWidth>width ? tempWidth : width;
    width += ARTIFACT_MARGIN * 2;

    int height = (2*fontHeight) + (ARTIFACT_MARGIN * 2);

    return QSize(width, height);
}

/**
 * draw as a file icon
 */
void ArtifactWidget::drawAsFile(QPainter& p, int offsetX, int offsetY)
{
    const int w = width();
    const int h = height();
    QFont font = UMLWidget::font();
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();

    int startX = offsetX + (w/2) - 25;
    int iconHeight = h - fontHeight;
    QPolygon pointArray(5);
    pointArray.setPoint(0, startX, offsetY);
    pointArray.setPoint(1, startX + 40, offsetY);
    pointArray.setPoint(2, startX + 50, offsetY + 10);
    pointArray.setPoint(3, startX + 50, offsetY + iconHeight);
    pointArray.setPoint(4, startX, offsetY + iconHeight);
    p.drawPolygon(pointArray);

    p.drawLine(startX + 40, offsetY, startX + 40, offsetY + 10);
    p.drawLine(startX + 40, offsetY + 10, startX + 50, offsetY + 10);
    p.drawLine(startX + 40, offsetY, startX + 50, offsetY + 10);

    p.setPen(textColor());
    p.setFont(font);

    p.drawText(offsetX, offsetY + h - fontHeight,
               w, fontHeight, Qt::AlignCenter, name());

    if(m_selected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

/**
 * draw as a library file icon
 */
void ArtifactWidget::drawAsLibrary(QPainter& p, int offsetX, int offsetY)
{
    //FIXME this should have gears on it
    const int w = width();
    const int h = height();
    const QFont font = UMLWidget::font();
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();

    const int startX = offsetX + (w/2) - 25;
    const int iconHeight = h - fontHeight;
    QPolygon pointArray(5);
    pointArray.setPoint(0, startX, offsetY);
    pointArray.setPoint(1, startX + 40, offsetY);
    pointArray.setPoint(2, startX + 50, offsetY + 10);
    pointArray.setPoint(3, startX + 50, offsetY + iconHeight);
    pointArray.setPoint(4, startX, offsetY + iconHeight);
    p.drawPolygon(pointArray);

    p.drawLine(startX + 40, offsetY, startX + 40, offsetY + 10);
    p.drawLine(startX + 40, offsetY + 10, startX + 50, offsetY + 10);
    p.drawLine(startX + 40, offsetY, startX + 50, offsetY + 10);

    p.setPen(textColor());
    p.setFont(font);

    p.drawText(offsetX, offsetY + h - fontHeight,
               w, fontHeight, Qt::AlignCenter, name());

    if(m_selected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

/**
 * draw as a database table icon
 */
void ArtifactWidget::drawAsTable(QPainter& p, int offsetX, int offsetY)
{
    const int w = width();
    const int h = height();
    const QFont font = UMLWidget::font();
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();

    const int startX = offsetX + (w/2) - 25;
    const int iconHeight = h - fontHeight;

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

    p.setPen(textColor());
    p.setFont(font);

    p.drawText(offsetX, offsetY + h - fontHeight,
               w, fontHeight, Qt::AlignCenter, name());

    if(m_selected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

/**
 * draw as a box
 */
void ArtifactWidget::drawAsNormal(QPainter& p, int offsetX, int offsetY)
{
    int w = width();
    int h = height();
    QFont font = UMLWidget::font();
    font.setBold(true);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight  = fm.lineSpacing();
    QString stereotype = m_pObject->stereotype();

    p.drawRect(offsetX, offsetY, w, h);

    p.setPen(textColor());
    p.setFont(font);

    if (!stereotype.isEmpty()) {
        p.drawText(offsetX + ARTIFACT_MARGIN, offsetY + (h/2) - fontHeight,
                   w, fontHeight, Qt::AlignCenter, m_pObject->stereotype(true));
    }

    int lines;
    if (!stereotype.isEmpty()) {
        lines = 2;
    } else {
        lines = 1;
    }

    if (lines == 1) {
        p.drawText(offsetX, offsetY + (h/2) - (fontHeight/2),
                   w, fontHeight, Qt::AlignCenter, name());
    } else {
        p.drawText(offsetX, offsetY + (h/2),
                   w, fontHeight, Qt::AlignCenter, name());
    }

    if(m_selected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

