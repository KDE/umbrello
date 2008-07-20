/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "artifactwidget.h"

// app includes
#include "artifact.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "widget_utils.h"

// qt includes
#include <QtGui/QPainter>

const qreal ArtifactWidget::Margin = 0;
const QSizeF ArtifactWidget::MinimumIconSize(50, 50);

/**
 * Constructs a ArtifactWidget.
 *
 * @param a The Artifact this widget will be representing.
 */
ArtifactWidget::ArtifactWidget(UMLArtifact *a) : NewUMLRectWidget(a)
{
	m_baseType = Uml::wt_Artifact;
	m_textItemGroup = new TextItemGroup(this);
	m_cachedTextHeight = 0; // Initialize on first call of sizeHasChanged.
}

/// Destructor
ArtifactWidget::~ArtifactWidget()
{
	delete m_textItemGroup;
}

/// Reimplemented from NewUMLRectWidget::sizeHint suiting this widget.
QSizeF ArtifactWidget::sizeHint(Qt::SizeHint which)
{
	if(which == Qt::MinimumSize) {
		return m_minimumSize;
	}
	return NewUMLRectWidget::sizeHint(which);
}

/**
 * Reimplemented to paint the articraft widget. Some part of specific
 * drawing is delegeted to private method like drawAsFile..
 */
void ArtifactWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
	painter->setPen(QPen(lineColor(), lineWidth()));
	painter->setBrush(brush());

	if(umlObject()) {
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
		uWarning() << "Cannot draw as there is no UMLArtifact for this widget";
	}
}

/**
 * Reimplemented from NewUMLWidget::saveToXMI to save the widget to
 * the "artifactwidget" XMI element.
 */
void ArtifactWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("artifactwidget");
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

void ArtifactWidget::updateGeometry()
{
	if(umlObject()) {
		UMLArtifact *articraft = static_cast<UMLArtifact*>(umlObject());
		m_textItemGroup->ensureTextItemCount(ArtifactWidget::TextItemCount);

		// Create a dummy item, to store the properties so that it can
        // easily be used to copy the properties to other text items.
        TextItem dummy("");
        dummy.setDefaultTextColor(fontColor());
        dummy.setFont(font());
        dummy.setAlignment(Qt::AlignCenter);
        dummy.setBackgroundBrush(Qt::NoBrush);

		TextItem *stereoItem = m_textItemGroup->textItemAt(ArtifactWidget::StereotypeItemIndex);
		stereoItem->setText(articraft->getStereotype(true));
		dummy.copyAttributesTo(stereoItem);
		bool hideStereo = articraft->getStereotype(false).isEmpty();
		stereoItem->setVisible(!hideStereo);

		TextItem *nameItem = m_textItemGroup->textItemAt(ArtifactWidget::NameItemIndex);
		nameItem->setText(name());
		dummy.copyAttributesTo(nameItem);

		m_minimumSize = m_textItemGroup->calculateMinimumSize() +
			QSizeF(2 * ArtifactWidget::Margin, 2 * ArtifactWidget::Margin);
		if(articraft->getDrawAsType() != UMLArtifact::defaultDraw) {
			m_minimumSize.rheight() += ArtifactWidget::MinimumIconSize.height();
			m_minimumSize.rwidth() = qMax(m_minimumSize.width(),
										  ArtifactWidget::MinimumIconSize.width());
		}
	}
	NewUMLRectWidget::updateGeometry();
}

void ArtifactWidget::sizeHasChanged(const QSizeF& oldSize)
{
	if (!umlObject()) {
		uWarning() << "No UMLArtifact for this widget.";
		NewUMLRectWidget::sizeHasChanged(oldSize);
		return;
	}

	const QRectF geometry = rect();
	UMLArtifact *articraft = static_cast<UMLArtifact*>(umlObject());
	QPointF offset(ArtifactWidget::Margin, ArtifactWidget::Margin);
	QSizeF groupSize(geometry.size());
	if (articraft->getDrawAsType() != UMLArtifact::defaultDraw) {
		qreal groupHeight = m_textItemGroup->calculateMinimumSize().height();
		groupSize.setHeight(groupHeight);
		offset.setY(geometry.height() - ArtifactWidget::Margin - groupHeight);
	}

	m_textItemGroup->setPos(offset);
	m_textItemGroup->alignVertically(groupSize);
	m_cachedTextHeight = groupSize.height(); // cache it to speed up.

	NewUMLRectWidget::sizeHasChanged(oldSize);
}

/**
 * draw as a file icon
 * @see Widget_Utils::drawTriangledRect
 */
void ArtifactWidget::drawAsFile(QPainter *painter)
{
	QRectF iconRect = rect();
	iconRect.setHeight(iconRect.height() - m_cachedTextHeight);
	QSizeF topRightTriSize(10, 10);

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
	QSizeF topRightTriSize(10, 10);

	Widget_Utils::drawTriangledRect(painter, iconRect, topRightTriSize);
	//FIXME this should have gears on it
}

/// draw as a database table icon
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
	painter->setPen(QPen(lineColor(), lineWidth() + 1));
	painter->drawLine(vLines[1]);
	painter->drawLine(hLines[1]);
}

/// draw as a box
void ArtifactWidget::drawAsNormal(QPainter *painter)
{
	painter->drawRect(rect());
}
