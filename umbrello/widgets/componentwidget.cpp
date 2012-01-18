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
#include "componentwidget.h"

// app includes
#include "component.h"
#include "debug_utils.h"
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
void ComponentWidget::paint(QPainter & p, int offsetX, int offsetY)
{
    UMLComponent *umlcomp = static_cast<UMLComponent*>(m_pObject);
    if (umlcomp == NULL)
        return;
    setPenFromSettings(p);
    if ( umlcomp->getExecutable() ) {
        QPen thickerPen = p.pen();
        thickerPen.setWidth(2);
        p.setPen(thickerPen);
    }
    if ( UMLWidget::useFillColor() ) {
        p.setBrush( UMLWidget::fillColor() );
    } else {
        p.setBrush( m_scene->viewport()->palette().color(QPalette::Background) );
    }

    const int w = width();
    const int h = height();
    QFont font = UMLWidget::font();
    font.setBold(true);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight = fm.lineSpacing();
    QString nameStr = name();
    const QString stereotype = m_pObject->stereotype();

    p.drawRect(offsetX + 2*COMPONENT_MARGIN, offsetY, w - 2*COMPONENT_MARGIN, h);
    p.drawRect(offsetX, offsetY + h/2 - fontHeight/2 - fontHeight, COMPONENT_MARGIN*4, fontHeight);
    p.drawRect(offsetX, offsetY + h/2 + fontHeight/2, COMPONENT_MARGIN*4, fontHeight);

    p.setPen(textColor());
    p.setFont(font);

    int lines = 1;

    if (!stereotype.isEmpty()) {
        p.drawText(offsetX + (COMPONENT_MARGIN*4), offsetY + (h/2) - fontHeight,
                   w - (COMPONENT_MARGIN*4), fontHeight, Qt::AlignCenter,
                   m_pObject->stereotype(true));
        lines = 2;
    }

    if ( UMLWidget::isInstance() ) {
        font.setUnderline(true);
        p.setFont(font);
        nameStr = UMLWidget::instanceName() + " : " + nameStr;
    }

    if (lines == 1) {
        p.drawText(offsetX + (COMPONENT_MARGIN*4), offsetY + (h/2) - (fontHeight/2),
                   w - (COMPONENT_MARGIN*4), fontHeight, Qt::AlignCenter, nameStr );
    } else {
        p.drawText(offsetX + (COMPONENT_MARGIN*4), offsetY + (h/2),
                   w - (COMPONENT_MARGIN*4), fontHeight, Qt::AlignCenter, nameStr );
    }

    if(m_selected) {
        drawSelected(&p, offsetX, offsetY);
    }
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
UMLSceneSize ComponentWidget::minimumSize()
{
    if ( !m_pObject) {
        return UMLSceneSize(70, 70);
    }
    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight = fm.lineSpacing();

    QString name = m_pObject->name();
    if ( UMLWidget::isInstance() ) {
        name = UMLWidget::instanceName() + " : " + name;
    }

    int width = fm.width(name);

    int stereoWidth = 0;
    if (!m_pObject->stereotype().isEmpty()) {
        stereoWidth = fm.width(m_pObject->stereotype(true));
    }
    if (stereoWidth > width)
        width = stereoWidth;
    width += COMPONENT_MARGIN * 6;
    width = 70>width ? 70 : width; //minumin width of 70

    int height = (2*fontHeight) + (COMPONENT_MARGIN * 3);

    return UMLSceneSize(width, height);
}
