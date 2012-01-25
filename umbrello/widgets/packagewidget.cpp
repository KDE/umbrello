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
#include "packagewidget.h"

// qt/kde includes
#include <QtGui/QPainter>

// app includes
#include "debug_utils.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"

/**
 * Constructs a PackageWidget.
 *
 * @param scene              The parent of this PackageWidget.
 * @param o         The UMLObject this will be representing.
 */
PackageWidget::PackageWidget(UMLScene * scene, UMLPackage *o)
  : UMLWidget(scene, WidgetBase::wt_Package, o)
{
    setSize(100, 30);
    setZ(m_origZ = 1);  // above box but below UMLWidget because may embed widgets
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
PackageWidget::~PackageWidget()
{
}

/**
 * Overrides standard method.
 */
void PackageWidget::paint(QPainter & p, int offsetX, int offsetY)
{
    setPenFromSettings(p);
    if ( UMLWidget::useFillColor() )
        p.setBrush( UMLWidget::fillColor() );
    else
        p.setBrush( m_scene->viewport()->palette().color(QPalette::Background) );

    int w = width();
    int h = height();
    QFont font = UMLWidget::font();
    font.setBold(true);
    //FIXME italic is true when a package is first created until you click elsewhere, not sure why
    font.setItalic(false);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight  = fm.lineSpacing();

    p.drawRect(offsetX, offsetY, 50, fontHeight);
    if (m_pObject->stereotype() == "subsystem") {
        const int fHalf = fontHeight / 2;
        const int symY = offsetY + fHalf;
        const int symX = offsetX + 38;
        p.drawLine(symX, symY, symX, symY + fHalf - 2);          // left leg
        p.drawLine(symX + 8, symY, symX + 8, symY + fHalf - 2);  // right leg
        p.drawLine(symX, symY, symX + 8, symY);                  // waist
        p.drawLine(symX + 4, symY, symX + 4, symY - fHalf + 2);  // head
    }
    p.drawRect(offsetX, offsetY + fontHeight - 1, w, h - fontHeight);

    p.setPen(textColor());
    p.setFont(font);

    int lines = 1;
    if (m_pObject != NULL) {
        QString stereotype = m_pObject->stereotype();
        if (!stereotype.isEmpty()) {
            p.drawText(offsetX, offsetY + fontHeight + PACKAGE_MARGIN,
                       w, fontHeight, Qt::AlignCenter, m_pObject->stereotype(true));
            lines = 2;
        }
    }

    p.drawText(offsetX, offsetY + (fontHeight*lines) + PACKAGE_MARGIN,
               w, fontHeight, Qt::AlignCenter, name() );

    if(m_selected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

/**
 * Overrides method from UMLWidget
 */
UMLSceneSize PackageWidget::minimumSize()
{
    if ( !m_pObject ) {
        return UMLWidget::minimumSize();
    }

    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight = fm.lineSpacing();

    int lines = 1;

    int width = fm.width( m_pObject->name() );

    int tempWidth = 0;
    if (!m_pObject->stereotype().isEmpty()) {
        tempWidth = fm.width(m_pObject->stereotype(true));
        lines = 2;
    }
    if (tempWidth > width)
        width = tempWidth;
    width += PACKAGE_MARGIN * 2;
    if (width < 70)
        width = 70;  // minumin width of 70

    int height = (lines*fontHeight) + fontHeight + (PACKAGE_MARGIN * 2);

    return UMLSceneSize(width, height);
}

/**
 * Saves to the "packagewidget" XMI element.
 */
void PackageWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("packagewidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}
