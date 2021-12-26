/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "artifactwidget.h"

// app includes
#include "artifact.h"
#include "debug_utils.h"
#include "umlscene.h"
#include "umlview.h"

// qt includes
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(ArtifactWidget)

/**
 * Constructs an ArtifactWidget.
 *
 * @param scene     The parent of this ArtifactWidget.
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
 * Reimplemented to paint the artifact widget. Some part of specific
 * drawing is delegated to private method like drawAsFile..
 */
void ArtifactWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    UMLWidget::setPenFromSettings(painter);
    if (UMLWidget::useFillColor()) {
        painter->setBrush(UMLWidget::fillColor());
    } else {
        painter->setBrush(m_scene->backgroundColor());
    }

    if (umlObject()) {
        const UMLArtifact *umlart = m_umlObject->asUMLArtifact();
        UMLArtifact::Draw_Type drawType = umlart->getDrawAsType();
        switch (drawType) {
        case UMLArtifact::defaultDraw:
            paintAsNormal(painter, option);
            break;
        case UMLArtifact::file:
            paintAsFile(painter, option);
            break;
        case UMLArtifact::library:
            paintAsLibrary(painter, option);
            break;
        case UMLArtifact::table:
            paintAsTable(painter, option);
            break;
        default:
            uWarning() << "Artifact drawn as unknown type";
            break;
        }
    }
    else {
        uWarning() << "Cannot draw as there is no UMLArtifact for this widget.";
    }
}

/**
 * Reimplemented from WidgetBase::saveToXMI to save the widget to
 * the "artifactwidget" XMI element.
 */
void ArtifactWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("artifactwidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeEndElement();
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF ArtifactWidget::minimumSize() const
{
    if (!m_umlObject) {
        return UMLWidget::minimumSize();
    }
    const UMLArtifact *umlart = m_umlObject->asUMLArtifact();
    if (umlart->getDrawAsType() == UMLArtifact::defaultDraw) {
        return calculateNormalSize();
    } else {
        return calculateIconSize();
    }
}

/**
 * calculates the size when drawing as an icon (it's the same size for all icons)
 */
QSize ArtifactWidget::calculateIconSize() const
{
    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight  = fm.lineSpacing();

    int width = fm.width(m_umlObject->name());

    width = width<50 ? 50 : width;

    int height = 50 + fontHeight;

    return QSize(width, height);
}

/**
 * calculates the size for drawing as a box
 */
QSize ArtifactWidget::calculateNormalSize() const
{
    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight  = fm.lineSpacing();

    int width = fm.width(m_umlObject->name());

    int tempWidth = 0;
    if(!m_umlObject->stereotype().isEmpty()) {
        tempWidth = fm.width(m_umlObject->stereotype(true));
    }
    width = tempWidth>width ? tempWidth : width;
    width += ARTIFACT_MARGIN * 2;

    int height = (2*fontHeight) + (ARTIFACT_MARGIN * 2);

    return QSize(width, height);
}

/**
 * draw as a file icon
 */
void ArtifactWidget::paintAsFile(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    const int w = width();
    const int h = height();
    QFont font = UMLWidget::font();
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();

    int startX = (w/2) - 25;
    int iconHeight = h - fontHeight;
    QPolygon pointArray(5);
    pointArray.setPoint(0, startX, 0);
    pointArray.setPoint(1, startX + 40, 0);
    pointArray.setPoint(2, startX + 50, 10);
    pointArray.setPoint(3, startX + 50, iconHeight);
    pointArray.setPoint(4, startX, iconHeight);
    painter->drawPolygon(pointArray);

    painter->drawLine(startX + 40, 0, startX + 40, 10);
    painter->drawLine(startX + 40, 10, startX + 50, 10);
    painter->drawLine(startX + 40, 0, startX + 50, 10);

    painter->setPen(textColor());
    painter->setFont(font);

    painter->drawText(0, h - fontHeight,
               w, fontHeight, Qt::AlignCenter, name());

    UMLWidget::paint(painter, option);
}

/**
 * draw as a library file icon
 */
void ArtifactWidget::paintAsLibrary(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    //FIXME this should have gears on it
    const int w = width();
    const int h = height();
    const QFont font = UMLWidget::font();
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();

    const int startX = (w/2) - 25;
    const int iconHeight = h - fontHeight;
    QPolygon pointArray(5);
    pointArray.setPoint(0, startX, 0);
    pointArray.setPoint(1, startX + 40, 0);
    pointArray.setPoint(2, startX + 50, 10);
    pointArray.setPoint(3, startX + 50, iconHeight);
    pointArray.setPoint(4, startX, iconHeight);
    painter->drawPolygon(pointArray);

    painter->drawLine(startX + 40, 0, startX + 40, 10);
    painter->drawLine(startX + 40, 10, startX + 50, 10);
    painter->drawLine(startX + 40, 0, startX + 50, 10);

    painter->setPen(textColor());
    painter->setFont(font);

    painter->drawText(0, h - fontHeight,
               w, fontHeight, Qt::AlignCenter, name());

    UMLWidget::paint(painter, option);
}

/**
 * draw as a database table icon
 */
void ArtifactWidget::paintAsTable(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    const int w = width();
    const int h = height();
    const QFont font = UMLWidget::font();
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();

    const int startX = (w/2) - 25;
    const int iconHeight = h - fontHeight;

    painter->drawRect(startX, 0, 50, h - fontHeight + 1);
    painter->drawLine(startX + 20, 0, startX + 20, iconHeight);
    painter->drawLine(startX + 30, 0, startX + 30, iconHeight);
    painter->drawLine(startX + 40, 0, startX + 40, iconHeight);
    painter->drawLine(startX, (iconHeight/2), startX + 49, (iconHeight/2));
    painter->drawLine(startX, (iconHeight/2) + (iconHeight/4),
               startX + 49, (iconHeight/2) + (iconHeight/4));

    QPen thickerPen = painter->pen();
    thickerPen.setWidth(2);
    painter->setPen(thickerPen);
    painter->drawLine(startX + 10, 0, startX + 10, iconHeight);
    painter->drawLine(startX, (iconHeight/4), startX + 50, (iconHeight/4));

    painter->setPen(textColor());
    painter->setFont(font);

    painter->drawText(0, h - fontHeight,
               w, fontHeight, Qt::AlignCenter, name());

    UMLWidget::paint(painter, option);
}

/**
 * draw as a box
 */
void ArtifactWidget::paintAsNormal(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    int w = width();
    int h = height();
    QFont font = UMLWidget::font();
    font.setBold(true);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight  = fm.lineSpacing();
    QString stereotype = m_umlObject->stereotype();

    painter->drawRect(0, 0, w, h);

    painter->setPen(textColor());
    painter->setFont(font);

    if (!stereotype.isEmpty()) {
        painter->drawText(ARTIFACT_MARGIN, (h/2) - fontHeight,
                   w, fontHeight, Qt::AlignCenter, m_umlObject->stereotype(true));
    }

    int lines;
    if (!stereotype.isEmpty()) {
        lines = 2;
    } else {
        lines = 1;
    }

    if (lines == 1) {
        painter->drawText(0, (h/2) - (fontHeight/2),
                   w, fontHeight, Qt::AlignCenter, name());
    } else {
        painter->drawText(0, (h/2),
                   w, fontHeight, Qt::AlignCenter, name());
    }

    UMLWidget::paint(painter, option);
}

