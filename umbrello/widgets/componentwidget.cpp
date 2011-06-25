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

// qt/kde includes
#include <QtGui/QPainter>

// app includes
#include "component.h"
#include "debug_utils.h"
#include "umlview.h"

ComponentWidget::ComponentWidget(UMLView * view, UMLComponent *c)
  : UMLWidget(view, c)
{
    init();
}

void ComponentWidget::init()
{
    UMLWidget::setBaseType(WidgetBase::wt_Component);
    setSize(100, 30);
    m_pMenu = 0;
    //set defaults from m_pView
    if (m_pView) {
        //check to see if correct
        const Settings::OptionState& ops = m_pView->optionState();
        m_bShowStereotype = ops.classState.showStereoType;
    }
    //maybe loading and this may not be set.
    if (m_pObject) {
        updateComponentSize();
        update();
    }
}

ComponentWidget::~ComponentWidget()
{
}

void ComponentWidget::draw(QPainter & p, int offsetX, int offsetY)
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
    if ( UMLWidget::getUseFillColour() ) {
        p.setBrush( UMLWidget::getFillColor() );
    } else {
        p.setBrush( m_pView->viewport()->palette().color(QPalette::Background) );
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

    p.setPen( QPen(Qt::black) );
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

    if(m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

QSize ComponentWidget::calculateSize()
{
    if ( !m_pObject) {
        return QSize(70, 70);
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

    return QSize(width, height);
}

void ComponentWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("componentwidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

