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
#include "packagewidget.h"

// qt/kde includes
#include <qpainter.h>
#include <kdebug.h>

// app includes
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"
#include "umlscene.h"


PackageWidget::PackageWidget(UMLScene * view, UMLPackage *o)
  : NewUMLRectWidget(view, o) {
    init();
}

void PackageWidget::init() {
    NewUMLRectWidget::setBaseType(Uml::wt_Package);
    setSize(100, 30);
    setZ(m_origZ = 1);  // above box but below NewUMLRectWidget because may embed widgets
    m_pMenu = 0;
    //set defaults from umlScene()
    if (umlScene()) {
        //check to see if correct
        const Settings::OptionState& ops = umlScene()->getOptionState();
        setShowStereotype(ops.classState.showStereoType);
    }
    //maybe loading and this may not be set.
    if (umlObject() && !UMLApp::app()->getDocument()->loading())
        updateComponentSize();
}

PackageWidget::~PackageWidget() {}

void PackageWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
	QPainter &p = *painter;
	qreal offsetX = 0, offsetY = 0;

    setPenFromSettings(p);
    if ( NewUMLRectWidget::getUseFillColour() )
        p.setBrush( NewUMLRectWidget::getFillColour() );
    else {
        // [PORT]
        // p.setBrush( umlScene()->viewport()->palette().color(QPalette::Background) );
    }

    qreal w = getWidth();
    qreal h = getHeight();
    QFont font = NewUMLRectWidget::getFont();
    font.setBold(true);
    //FIXME italic is true when a package is first created until you click elsewhere, not sure why
    font.setItalic(false);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const qreal fontHeight  = fm.lineSpacing();
    QString name = getName();

    p.drawRect(offsetX, offsetY, 50, fontHeight);
    if (umlObject()->getStereotype() == "subsystem") {
        const qreal fHalf = fontHeight / 2;
        const qreal symY = offsetY + fHalf;
        const qreal symX = offsetX + 38;
        p.drawLine(symX, symY, symX, symY + fHalf - 2);          // left leg
        p.drawLine(symX + 8, symY, symX + 8, symY + fHalf - 2);  // right leg
        p.drawLine(symX, symY, symX + 8, symY);                  // waist
        p.drawLine(symX + 4, symY, symX + 4, symY - fHalf + 2);  // head
    }
    p.drawRect(offsetX, offsetY + fontHeight - 1, w, h - fontHeight);

    p.setPen( QPen(Qt::black) );
    p.setFont(font);

    qreal lines = 1;
    if (umlObject() != NULL) {
        QString stereotype = umlObject()->getStereotype();
        if (!stereotype.isEmpty()) {
            p.drawText(offsetX, offsetY + fontHeight + PACKAGE_MARGIN,
                       w, fontHeight, Qt::AlignCenter, umlObject()->getStereotype(true));
            lines = 2;
        }
    }

    p.drawText(offsetX, offsetY + (fontHeight*lines) + PACKAGE_MARGIN,
               w, fontHeight, Qt::AlignCenter, name );

    if(isSelected()) {
        drawSelected(&p, offsetX, offsetY);
    }
}

QSizeF PackageWidget::calculateSize() {
    if ( !umlObject() ) {
        return NewUMLRectWidget::calculateSize();
    }

    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const qreal fontHeight = fm.lineSpacing();

    qreal lines = 1;

    qreal width = fm.width( umlObject()->getName() );

    qreal tempWidth = 0;
    if (!umlObject()->getStereotype().isEmpty()) {
        tempWidth = fm.width(umlObject()->getStereotype(true));
        lines = 2;
    }
    if (tempWidth > width)
        width = tempWidth;
    width += PACKAGE_MARGIN * 2;
    if (width < 70)
        width = 70;  // minumin width of 70

    qreal height = (lines*fontHeight) + fontHeight + (PACKAGE_MARGIN * 2);

    return QSizeF(width, height);
}

void PackageWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement conceptElement = qDoc.createElement("packagewidget");
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

