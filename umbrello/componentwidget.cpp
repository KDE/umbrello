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
#include "componentwidget.h"

// qt/kde includes
#include <qpainter.h>

// app includes
#include <kdebug.h>
#include "component.h"
#include "umlview.h"
#include "umlscene.h"


ComponentWidget::ComponentWidget(UMLScene * scene, UMLComponent *c)
  : NewUMLRectWidget(scene, c) {
    init();
}

void ComponentWidget::init() {
    NewUMLRectWidget::setBaseType(Uml::wt_Component);
    setSize(100, 30);
    m_pMenu = 0;
    //set defaults from umlScene()
    if (umlScene()) {
        //check to see if correct
        const Settings::OptionState& ops = umlScene()->getOptionState();
        setShowStereotype(ops.classState.showStereoType);
    }
    //maybe loading and this may not be set.
    if (umlObject()) {
        updateComponentSize();
        update();
    }
}

ComponentWidget::~ComponentWidget() {}

void ComponentWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
	QPainter &p = *painter;
	qreal offsetX = 0, offsetY = 0;

    UMLComponent *umlcomp = static_cast<UMLComponent*>(umlObject());
    if (umlcomp == NULL)
        return;
    setPenFromSettings(p);
    if ( umlcomp->getExecutable() ) {
        QPen thickerPen = p.pen();
        thickerPen.setWidth(2);
        p.setPen(thickerPen);
    }
    if ( NewUMLRectWidget::getUseFillColour() ) {
        p.setBrush( NewUMLRectWidget::getFillColour() );
    } else {
        // [PORT] Replace with styleoption based code.
        //p.setBrush( umlScene()->viewport()->palette().color(QPalette::Background) );
    }

    const qreal w = getWidth();
    const qreal h = getHeight();
    QFont font = NewUMLRectWidget::getFont();
    font.setBold(true);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const qreal fontHeight = fm.lineSpacing();
    QString name = getName();
    const QString stereotype = umlObject()->getStereotype();

    p.drawRect(offsetX + 2*COMPONENT_MARGIN, offsetY, w - 2*COMPONENT_MARGIN, h);
    p.drawRect(offsetX, offsetY + h/2 - fontHeight/2 - fontHeight, COMPONENT_MARGIN*4, fontHeight);
    p.drawRect(offsetX, offsetY + h/2 + fontHeight/2, COMPONENT_MARGIN*4, fontHeight);

    p.setPen( QPen(Qt::black) );
    p.setFont(font);

    int lines = 1;

    if (!stereotype.isEmpty()) {
        p.drawText(offsetX + (COMPONENT_MARGIN*4), offsetY + (h/2) - fontHeight,
                   w - (COMPONENT_MARGIN*4), fontHeight, Qt::AlignCenter,
                   umlObject()->getStereotype(true));
        lines = 2;
    }

    if ( NewUMLRectWidget::getIsInstance() ) {
        font.setUnderline(true);
        p.setFont(font);
        name = NewUMLRectWidget::getInstanceName() + " : " + name;
    }

    if (lines == 1) {
        p.drawText(offsetX + (COMPONENT_MARGIN*4), offsetY + (h/2) - (fontHeight/2),
                   w - (COMPONENT_MARGIN*4), fontHeight, Qt::AlignCenter, name );
    } else {
        p.drawText(offsetX + (COMPONENT_MARGIN*4), offsetY + (h/2),
                   w - (COMPONENT_MARGIN*4), fontHeight, Qt::AlignCenter, name );
    }

    if(isSelected()) {
        drawSelected(&p, offsetX, offsetY);
    }
}

QSizeF ComponentWidget::calculateSize()
{
    if ( !umlObject()) {
        return QSizeF(70, 70);
    }
    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const qreal fontHeight = fm.lineSpacing();

    QString name = umlObject()->getName();
    if ( NewUMLRectWidget::getIsInstance() ) {
        name = NewUMLRectWidget::getInstanceName() + " : " + name;
    }

    qreal width = fm.width(name);

    qreal stereoWidth = 0;
    if (!umlObject()->getStereotype().isEmpty()) {
        stereoWidth = fm.width(umlObject()->getStereotype(true));
    }
    if (stereoWidth > width)
        width = stereoWidth;
    width += COMPONENT_MARGIN * 6;
    width = 70>width ? 70 : width; //minumin width of 70

    qreal height = (2*fontHeight) + (COMPONENT_MARGIN * 3);

    return QSizeF(width, height);
}

void ComponentWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("componentwidget");
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

