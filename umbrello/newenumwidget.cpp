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
#include "newenumwidget.h"

// qt/kde includes
#include <QtGui/QPainter>
#include <kdebug.h>

// app includes
#include "enum.h"
#include "enumliteral.h"
#include "classifier.h"
#include "umlclassifierlistitemlist.h"
#include "classifierlistitem.h"

NewEnumWidget::NewEnumWidget(UMLObject* o) : NewUMLRectWidget(o)
{
    m_baseType = Uml::wt_Enum;
}

void NewEnumWidget::init()
{
    updateGeometry();
}

QSizeF NewEnumWidget::sizeHint(Qt::SizeHint which)
{
    switch(which)
    {
    case Qt::MinimumSize:
        return m_minimumSize;

    default:
        return QSizeF(1000, 1000);
    }
}

void NewEnumWidget::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    QFont fnt = NewUMLRectWidget::font();
    const QSizeF sz = size();
    QFontMetricsF fm(fnt);
    const qreal fontHeight  = fm.lineSpacing();

    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawRect(QRectF(QPointF(0, 0), sz));

    QRectF rect(ENUM_MARGIN, 0, sz.width() - ENUM_MARGIN * 2, fontHeight);


    fnt.setBold(true);
    painter->setFont(fnt);
    painter->setPen(QPen(Qt::black));
    painter->drawText(rect, Qt::AlignCenter, umlObject()->getStereotype(true));

    fnt.setItalic(umlObject()->getAbstract());
    painter->setFont(fnt);
    rect.moveTop(rect.top() + fontHeight);
    painter->drawText(rect, Qt::AlignCenter, name());

    qreal y = fontHeight * 2;
    fnt.setBold(false);
    fnt.setItalic(false);
    painter->setFont(fnt);
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawLine(QLineF(0, y, sz.width() - 1, y));

    fm = QFontMetricsF(fnt);
    UMLClassifier *classifier = static_cast<UMLClassifier*>(umlObject());
    UMLClassifierListItem* enumLiteral = 0;
    UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EnumLiteral);

    painter->setPen(Qt::black);
    foreach (enumLiteral , list ) {
        rect.moveTop(y);
        painter->drawText(rect, Qt::AlignVCenter | Qt::AlignHCenter, enumLiteral->getName());
        y += fontHeight;
    }
}

bool NewEnumWidget::loadFromXMI( QDomElement & qElement ) {
    if ( !NewUMLRectWidget::loadFromXMI(qElement) ) {
        return false;
    }
    return true;
}

void NewEnumWidget::saveToXMI( QDomDocument& qDoc, QDomElement& qElement ) {
    QDomElement conceptElement = qDoc.createElement("enumwidget");
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);

    qElement.appendChild(conceptElement);
}

void NewEnumWidget::updateGeometry()
{
    calculateMinimumSize();
    NewUMLRectWidget::updateGeometry();
}

void NewEnumWidget::calculateMinimumSize()
{
    if (!umlObject()) {
        m_minimumSize.setWidth(100);
        m_minimumSize.setHeight(100);
        return;
    }

    qreal width, height;
    QFont font = this->font();
    font.setItalic(false);
    font.setUnderline(false);
    font.setBold(false);
    const QFontMetricsF fm(font);

    const qreal fontHeight = fm.lineSpacing();

    int lines = 1;//always have one line - for name
    lines++; //for the stereotype

    const qreal numberOfEnumLiterals = static_cast<UMLEnum*>(umlObject())->enumLiterals();
    lines += numberOfEnumLiterals;

    height = width = 0;
    //set the height of the enum
    if (numberOfEnumLiterals == 0) {
        height += 0.5 * fontHeight; //no enum literals, so just add a bit of space
    }

    height += lines * fontHeight;

    font.setBold(true);
    font.setItalic(true);
    width = QFontMetricsF(font).boundingRect(name()).width();
    font.setItalic(false);
    qreal w = QFontMetricsF(font).boundingRect(umlObject()->getStereotype(true)).width();


    width = qMax(w, width);

    UMLClassifier *classifier = static_cast<UMLClassifier*>(umlObject());
    UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EnumLiteral);
    UMLClassifierListItem* listItem = 0;

    foreach (listItem , list ) {
        qreal w = fm.width (listItem->getName());
        if(w > width)
            width = w;
    }

    //allow for width margin
    width += ENUM_MARGIN * 2;

    m_minimumSize.setWidth(width);
    m_minimumSize.setHeight(height);
}
