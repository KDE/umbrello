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
#include "datatypewidget.h"

// qt/kde includes
#include <QtGui/QPainter>

// app includes
#include "classifier.h"
#include "classifierlistitem.h"
#include "debug_utils.h"
#include "operation.h"
#include "umldoc.h"
#include "umlview.h"

#define CIRCLE_SIZE 30


/**
 * Constructs an DatatypeWidget.
 *
 * @param scene              The parent of this DatatypeWidget.
 * @param d         The UMLClassifier this will be representing.
 */
DatatypeWidget::DatatypeWidget(UMLScene *scene, UMLClassifier *d) 
  : UMLWidget(scene, WidgetBase::wt_Datatype, d)
{
    setSize(100, 30);
}

/**
 * Standard deconstructor.
 */
DatatypeWidget::~DatatypeWidget()
{
}

/**
 * Overrides standard method.
 */
void DatatypeWidget::paint(QPainter& p, int offsetX, int offsetY)
{
    setPenFromSettings(p);
    if (UMLWidget::useFillColor())  {
        p.setBrush(UMLWidget::fillColor());
    } else {
        p.setBrush( m_scene->viewport()->palette().color(QPalette::Background) );
    }

    int w = width();
    int h = height();

    QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    int fontHeight  = fm.lineSpacing();

    p.drawRect(offsetX, offsetY, w, h);
    p.setPen(textColor());

    QFont font = UMLWidget::font();
    font.setBold(true);
    p.setFont(font);
    p.drawText(offsetX + DATATYPE_MARGIN, offsetY,
               w - DATATYPE_MARGIN* 2,fontHeight,
               Qt::AlignCenter, m_pObject->stereotype(true));

    font.setItalic( m_pObject->isAbstract() );
    p.setFont(font);
    p.drawText(offsetX + DATATYPE_MARGIN, offsetY + fontHeight,
               w - DATATYPE_MARGIN * 2, fontHeight, Qt::AlignCenter, name());

    if (m_selected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

/**
 * Loads from a "datatypewidget" XMI element.
 */
bool DatatypeWidget::loadFromXMI( QDomElement & qElement )
{
    return UMLWidget::loadFromXMI(qElement);
}

/**
 * Saves to the "datatypewidget" XMI element.
 */
void DatatypeWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement conceptElement = qDoc.createElement("datatypewidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

/**
 * Overrides method from UMLWidget.
 */
UMLSceneSize DatatypeWidget::minimumSize()
{
    if (!m_pObject)  {
        return UMLWidget::minimumSize();
    }
    int width, height;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight = fm.lineSpacing();

    int lines = 1;//always have one line - for name
    lines++; //for the stereotype

    height = width = 0;
    height += lines * fontHeight;

    //now set the width of the concept
    //set width to name to start with
    //set width to name to start with
    width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(m_pObject->fullyQualifiedName()).width();
    int w = getFontMetrics(FT_BOLD).boundingRect(m_pObject->stereotype(true)).width();

    width = w > width?w:width;

    //allow for width margin
    width += DATATYPE_MARGIN * 2;

    return UMLSceneSize(width, height);
}
