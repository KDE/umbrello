/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "componentwidget.h"

// app includes
#include "component.h"
#include "debug_utils.h"
#include "umlscene.h"
#include "umlview.h"

/**
 * Constructs a ComponentWidget.
 *
 * @param scene      The parent of this ComponentWidget.
 * @param c The UMLComponent this will be representing.
 */
ComponentWidget::ComponentWidget(UMLScene * scene, UMLComponent *c)
  : UMLWidget(scene, WidgetBase::wt_Component, c)
{
    setSize(100, 30);
    //set defaults from m_scene
    if (m_scene) {
        //check to see if correct
        const Settings::OptionState& ops = m_scene->optionState();
        m_showStereotype = ops.classState.showStereoType;
    }
}

/**
 * Destructor.
 */
ComponentWidget::~ComponentWidget()
{
}

/**
 * Reimplemented from UMLWidget::paint to paint component
 * widget.
 */
void ComponentWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    UMLComponent *umlcomp = static_cast<UMLComponent*>(m_umlObject);
    if (umlcomp == NULL)
        return;
    setPenFromSettings(painter);
    if (umlcomp->getExecutable()) {
        QPen thickerPen = painter->pen();
        thickerPen.setWidth(2);
        painter->setPen(thickerPen);
    }
    if (UMLWidget::useFillColor()) {
        painter->setBrush(UMLWidget::fillColor());
    } else {
        painter->setBrush(m_scene->activeView()->viewport()->palette().color(QPalette::Background));
    }

    const int w = width();
    const int h = height();
    QFont font = UMLWidget::font();
    font.setBold(true);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight = fm.lineSpacing();
    QString nameStr = name();
    const QString stereotype = m_umlObject->stereotype();

    painter->drawRect(2*COMPONENT_MARGIN, 0, w - 2*COMPONENT_MARGIN, h);
    painter->drawRect(0, h/2 - fontHeight/2 - fontHeight, COMPONENT_MARGIN*4, fontHeight);
    painter->drawRect(0, h/2 + fontHeight/2, COMPONENT_MARGIN*4, fontHeight);

    painter->setPen(textColor());
    painter->setFont(font);

    int lines = 1;

    if (!stereotype.isEmpty()) {
        painter->drawText((COMPONENT_MARGIN*4), (h/2) - fontHeight,
                   w - (COMPONENT_MARGIN*4), fontHeight, Qt::AlignCenter,
                   m_umlObject->stereotype(true));
        lines = 2;
    }

    if (UMLWidget::isInstance()) {
        font.setUnderline(true);
        painter->setFont(font);
        nameStr = UMLWidget::instanceName() + " : " + nameStr;
    }

    if (lines == 1) {
        painter->drawText((COMPONENT_MARGIN*4), (h/2) - (fontHeight/2),
                   w - (COMPONENT_MARGIN*4), fontHeight, Qt::AlignCenter, nameStr);
    } else {
        painter->drawText((COMPONENT_MARGIN*4), (h/2),
                   w - (COMPONENT_MARGIN*4), fontHeight, Qt::AlignCenter, nameStr);
    }

    UMLWidget::paint(painter, option, widget);
}

/**
 * Saves to the "componentwidget" XMI element.
 */
void ComponentWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("componentwidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF ComponentWidget::minimumSize()
{
    if (!m_umlObject) {
        return QSizeF(70, 70);
    }
    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight = fm.lineSpacing();

    QString name = m_umlObject->name();
    if (UMLWidget::isInstance()) {
        name = UMLWidget::instanceName() + " : " + name;
    }

    int width = fm.width(name);

    int stereoWidth = 0;
    if (!m_umlObject->stereotype().isEmpty()) {
        stereoWidth = fm.width(m_umlObject->stereotype(true));
    }
    if (stereoWidth > width)
        width = stereoWidth;
    width += COMPONENT_MARGIN * 6;
    width = 70>width ? 70 : width; //minumin width of 70

    int height = (2*fontHeight) + (COMPONENT_MARGIN * 3);

    UMLComponent *umlcomp = static_cast<UMLComponent*>(m_umlObject);
    if (umlcomp && umlcomp->getExecutable()) {
        width  += 2;
        height += 2;
    }

    return QSizeF(width, height);
}
