/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "artifactwidget.h"

// app includes
#include "artifact.h"
#include "debug_utils.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "widget_utils.h"

const QSizeF ArtifactWidget::MinimumIconSize(50, 50);

/**
 * Constructs a ArtifactWidget.
 *
 * @param a The Artifact this widget will be representing.
 */
ArtifactWidget::ArtifactWidget(UMLArtifact *a)
  : UMLWidget(WidgetBase::wt_Artifact, a)
{
    m_cachedTextHeight = 0; // Initialize on first call of sizeHasChanged.

    createTextItemGroup();
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
void ArtifactWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    if (umlObject()) {
        UMLArtifact *umlart = static_cast<UMLArtifact*>(umlObject());
        UMLArtifact::Draw_Type drawType = umlart->getDrawAsType();
        switch (drawType) {
        case UMLArtifact::defaultDraw:
            drawAsNormal(painter);
            break;
        case UMLArtifact::file:
            drawAsFile(painter);
            break;
        case UMLArtifact::library:
            drawAsLibrary(painter);
            break;
        case UMLArtifact::table:
            drawAsTable(painter);
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
void ArtifactWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("artifactwidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

/**
 * Reimplemented from UMLWidget::updateGeometry to calculate
 * minimum size appropriately.
 */
void ArtifactWidget::updateGeometry()
{
    if(umlObject()) {
        UMLArtifact *artifact = static_cast<UMLArtifact*>(umlObject());
        QSizeF minSize = textItemGroupAt(ArtifactWidget::GroupIndex)->minimumSize();

        if(artifact->getDrawAsType() != UMLArtifact::defaultDraw) {
            minSize.rheight() += ArtifactWidget::MinimumIconSize.height();
            if(minSize.width() < ArtifactWidget::MinimumIconSize.width()) {
                minSize.setWidth(ArtifactWidget::MinimumIconSize.width());
            }
        }

        setMinimumSize(minSize);
    }
    UMLWidget::updateGeometry();
}

QVariant ArtifactWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if (!umlObject()) {
        uWarning() << "No UMLArtifact for this widget.";
        return UMLWidget::attributeChange(change, oldValue);
    }

    if(change == SizeHasChanged) {
        UMLArtifact *artifact = static_cast<UMLArtifact*>(umlObject());
        TextItemGroup *grp = textItemGroupAt(GroupIndex);
        const qreal groupHeight = grp->minimumSize().height();

        QRectF groupGeometry = rect();

        if (artifact->getDrawAsType() != UMLArtifact::defaultDraw) {
            groupGeometry.setTop(groupGeometry.bottom() - groupHeight);
        }

        grp->setGroupGeometry(groupGeometry);
        m_cachedTextHeight = groupHeight;
    }

    return UMLWidget::attributeChange(change, oldValue);
}

void ArtifactWidget::updateTextItemGroups()
{
    if(umlObject()) {
        UMLArtifact *artifact = static_cast<UMLArtifact*>(umlObject());
        TextItemGroup *grp = textItemGroupAt(GroupIndex);
        grp->setTextItemCount(ArtifactWidget::TextItemCount);

        TextItem *stereoItem = grp->textItemAt(ArtifactWidget::StereotypeItemIndex);
        stereoItem->setText(artifact->stereotype(true));
        bool hideStereo = artifact->stereotype(false).isEmpty()
            || artifact->getDrawAsType() != UMLArtifact::defaultDraw ;
        stereoItem->setExplicitVisibility(!hideStereo);

        TextItem *nameItem = grp->textItemAt(ArtifactWidget::NameItemIndex);
        nameItem->setText(name());
    }
    UMLWidget::updateTextItemGroups();
}

/**
 * draw as a file icon
 * @see Widget_Utils::drawTriangledRect
 */
void ArtifactWidget::drawAsFile(QPainter *painter)
{
    QRectF iconRect = rect();
    iconRect.setHeight(iconRect.height() - m_cachedTextHeight);
    qreal factor = .2 * qMin(iconRect.width(), iconRect.height());
    QSizeF topRightTriSize(factor, factor);

    Widget_Utils::drawTriangledRect(painter, iconRect, topRightTriSize);
}

/**
 * draw as a library file icon
 * @see Widget_Utils::drawTriangledRect
 * @todo Drawing should have gears on it.
 */
void ArtifactWidget::drawAsLibrary(QPainter *painter)
{
    QRectF iconRect = rect();
    iconRect.setHeight(iconRect.height() - m_cachedTextHeight);
    qreal factor = .2 * qMin(iconRect.width(), iconRect.height());
    QSizeF topRightTriSize(factor, factor);

    Widget_Utils::drawTriangledRect(painter, iconRect, topRightTriSize);
    //FIXME this should have gears on it
}

/**
 * draw as a database table icon
 */
void ArtifactWidget::drawAsTable(QPainter *painter)
{
    QRectF iconRect = rect();
    iconRect.setHeight(iconRect.height() - m_cachedTextHeight);

    painter->fillRect(iconRect, painter->brush());

    QVector<QLineF> vLines(6);
    qreal hSpacing = iconRect.width() / 5.0;
    qreal x = iconRect.left();
    for(int i = 0; i < vLines.size(); ++i) {
        vLines[i].setLine(x, iconRect.top(), x, iconRect.bottom());
        x += hSpacing;
    }

    QVector<QLineF> hLines(5);
    qreal vSpacing = iconRect.height() / 4.0;
    qreal y = iconRect.top();
    for(int i = 0; i < hLines.size(); ++i) {
        hLines[i].setLine(iconRect.left(), y, iconRect.right(), y);
        y += vSpacing;
    }

    painter->drawLines(vLines);
    painter->drawLines(hLines);

    // Draw thick lines.
    painter->setPen(QPen(lineColor(), lineWidth() + 2));
    painter->drawLine(vLines[1]);
    painter->drawLine(hLines[1]);
}

/**
 * draw as a box
 */
void ArtifactWidget::drawAsNormal(QPainter *painter)
{
    painter->drawRect(rect());
}

