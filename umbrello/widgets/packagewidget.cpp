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
#include "packagewidget.h"

// app includes
#include "debug_utils.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"

// qt includes
#include <QPainter>

/**
 * Constructs a PackageWidget.
 *
 * @param scene   The parent of this PackageWidget.
 * @param o       The UMLObject this will be representing.
 */
PackageWidget::PackageWidget(UMLScene * scene, UMLPackage *o)
  : UMLWidget(scene, WidgetBase::wt_Package, o)
{
    setSize(100, 30);
    setZValue(1);  // above box but below UMLWidget because may embed widgets
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
void PackageWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    setPenFromSettings(painter);
    if ( UMLWidget::useFillColor() )
        painter->setBrush( UMLWidget::fillColor() );
    else
        painter->setBrush( m_scene->activeView()->viewport()->palette().color(QPalette::Background) );

    int w = width();
    int h = height();
    QFont font = UMLWidget::font();
    font.setBold(true);
    //FIXME italic is true when a package is first created until you click elsewhere, not sure why
    font.setItalic(false);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight  = fm.lineSpacing();

    painter->drawRect(0, 0, 50, fontHeight);
    if (m_umlObject->stereotype() == "subsystem") {
        const int fHalf = fontHeight / 2;
        const int symY = fHalf;
        const int symX = 38;
        painter->drawLine(symX, symY, symX, symY + fHalf - 2);          // left leg
        painter->drawLine(symX + 8, symY, symX + 8, symY + fHalf - 2);  // right leg
        painter->drawLine(symX, symY, symX + 8, symY);                  // waist
        painter->drawLine(symX + 4, symY, symX + 4, symY - fHalf + 2);  // head
    }
    painter->drawRect(0, fontHeight - 1, w, h - fontHeight);

    painter->setPen(textColor());
    painter->setFont(font);

    int lines = 1;
    if (m_umlObject != NULL) {
        QString stereotype = m_umlObject->stereotype();
        if (!stereotype.isEmpty()) {
            painter->drawText(0, fontHeight + PACKAGE_MARGIN,
                       w, fontHeight, Qt::AlignCenter, m_umlObject->stereotype(true));
            lines = 2;
        }
    }

    painter->drawText(0, (fontHeight*lines) + PACKAGE_MARGIN,
               w, fontHeight, Qt::AlignCenter, name() );

    if(m_selected) {
        drawSelected(painter);
    }
}

/**
 * Overrides method from UMLWidget
 */
UMLSceneSize PackageWidget::minimumSize()
{
    if ( !m_umlObject ) {
        return UMLWidget::minimumSize();
    }

    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight = fm.lineSpacing();

    int lines = 1;

    int width = fm.width( m_umlObject->name() );

    int tempWidth = 0;
    if (!m_umlObject->stereotype().isEmpty()) {
        tempWidth = fm.width(m_umlObject->stereotype(true));
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
