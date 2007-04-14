/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
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


PackageWidget::PackageWidget(UMLView * view, UMLPackage *o)
  : UMLWidget(view, o) {
    init();
}

void PackageWidget::init() {
    UMLWidget::setBaseType(Uml::wt_Package);
    setSize(100, 30);
    setZ(m_origZ = 1);  // above box but below UMLWidget because may embed widgets
    m_pMenu = 0;
    //set defaults from m_pView
    if (m_pView) {
        //check to see if correct
        const Settings::OptionState& ops = m_pView->getOptionState();
        m_bShowStereotype = ops.classState.showStereoType;
    }
    //maybe loading and this may not be set.
    if (m_pObject && !UMLApp::app()->getDocument()->loading())
        updateComponentSize();
}

PackageWidget::~PackageWidget() {}

void PackageWidget::draw(QPainter & p, int offsetX, int offsetY) {
    UMLWidget::setPen(p);
    if ( UMLWidget::getUseFillColour() )
        p.setBrush( UMLWidget::getFillColour() );
    else
        p.setBrush(m_pView -> viewport() -> backgroundColor());

    int w = width();
    int h = height();
    QFont font = UMLWidget::getFont();
    font.setBold(true);
    //FIXME italic is true when a package is first created until you click elsewhere, not sure why
    font.setItalic(false);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight  = fm.lineSpacing();
    QString name = getName();

    p.drawRect(offsetX, offsetY, 50, fontHeight);
    if (m_pObject->getStereotype() == "subsystem") {
        const int fHalf = fontHeight / 2;
        const int symY = offsetY + fHalf;
        const int symX = offsetX + 38;
        p.drawLine(symX, symY, symX, symY + fHalf - 2);          // left leg
        p.drawLine(symX + 8, symY, symX + 8, symY + fHalf - 2);  // right leg
        p.drawLine(symX, symY, symX + 8, symY);                  // waist
        p.drawLine(symX + 4, symY, symX + 4, symY - fHalf + 2);  // head
    }
    p.drawRect(offsetX, offsetY + fontHeight - 1, w, h - fontHeight);

    p.setPen( QPen(Qt::black) );
    p.setFont(font);

    int lines = 1;
    if (m_pObject != NULL) {
        QString stereotype = m_pObject->getStereotype();
        if (!stereotype.isEmpty()) {
            p.drawText(offsetX, offsetY + fontHeight + PACKAGE_MARGIN,
                       w, fontHeight, Qt::AlignCenter, m_pObject->getStereotype(true));
            lines = 2;
        }
    }

    p.drawText(offsetX, offsetY + (fontHeight*lines) + PACKAGE_MARGIN,
               w, fontHeight, Qt::AlignCenter, name );

    if(m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

QSize PackageWidget::calculateSize() {
    if ( !m_pObject ) {
        return UMLWidget::calculateSize();
    }

    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight = fm.lineSpacing();

    int lines = 1;

    int width = fm.width( m_pObject->getName() );

    int tempWidth = 0;
    if (!m_pObject->getStereotype().isEmpty()) {
        tempWidth = fm.width(m_pObject->getStereotype(true));
        lines = 2;
    }
    if (tempWidth > width)
        width = tempWidth;
    width += PACKAGE_MARGIN * 2;
    if (width < 70)
        width = 70;  // minumin width of 70

    int height = (lines*fontHeight) + fontHeight + (PACKAGE_MARGIN * 2);

    return QSize(width, height);
}

void PackageWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement conceptElement = qDoc.createElement("packagewidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

