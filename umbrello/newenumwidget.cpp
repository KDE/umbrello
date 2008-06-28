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
#include <QtGui/QStyleOptionGraphicsItem>
#include <kdebug.h>

// app includes
#include "enum.h"
#include "enumliteral.h"
#include "classifier.h"
#include "umlclassifierlistitemlist.h"
#include "classifierlistitem.h"
#include "widget_utils.h"
#include "optionstate.h"
#include "umlscene.h"

NewEnumWidget::NewEnumWidget(UMLObject* o) :
    NewUMLRectWidget(o),
    m_showPackage(false)
{
    m_baseType = Uml::wt_Enum;
}

void NewEnumWidget::init()
{
    if(umlScene()) {
        const Settings::OptionState& ops = umlScene()->getOptionState();
        m_showPackage = ops.classState.showPackage;
    }
    updateGeometry();
}

QSizeF NewEnumWidget::sizeHint(Qt::SizeHint which)
{
    switch(which)
    {
    case Qt::MinimumSize:
        return m_minimumSize;

    default:
        return NewUMLRectWidget::sizeHint(which);
    }
}

void NewEnumWidget::setShowPackage(bool b)
{
    m_showPackage = b;
    updateGeometry();
}

void NewEnumWidget::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    UMLClassifier *classifier = 0;
    UMLClassifierListItemList list;

    if(umlObject()) {
        classifier = static_cast<UMLClassifier*>(umlObject());
        list = classifier->getFilteredList(Uml::ot_EnumLiteral);
    }

    QFont fnt = NewUMLRectWidget::font();
    // Get font metrics for max font type
    fnt.setBold(true);
    fnt.setItalic(true);
    QFontMetricsF fm(fnt);
    fnt.setItalic(false); // Reset italic.

    qreal fontHeight  = fm.lineSpacing();
    const QSizeF sz = size();
    const QSizeF minSz = sizeHint(Qt::MinimumSize);

    if(sz.height() > minSz.height()) {
        fontHeight += (sz.height() - minSz.height()) / qreal(list.size() + 2);
    }

    QRectF fontRect(ENUM_MARGIN, 0, sz.width() - ENUM_MARGIN * 2, fontHeight);


    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());
    // First draw the outer rectangle with the pen and brush of this widget.
    painter->drawRect(rect());
    painter->drawLine(QLineF(0, fontHeight * 2, sz.width() - 1, fontHeight * 2));

    // Set the font pen and empty brush to draw the font.
    painter->setPen(fontColor());
    painter->setBrush(QBrush(Qt::NoBrush));
    painter->setFont(fnt);

    QString text = umlObject() ? umlObject()->getStereotype(true) : "<< >>";

    painter->drawText(fontRect, Qt::AlignCenter, text);

    bool italicNeeded = umlObject() ? umlObject()->getAbstract() : false;
    fnt.setItalic(italicNeeded);
    painter->setFont(fnt);
    fontRect.moveTop(fontRect.top() + fontHeight);

    QString nameText = name();
    if(m_showPackage && umlObject()) {
        nameText = umlObject()->getFullyQualifiedName();
    }
    painter->drawText(fontRect, Qt::AlignCenter, nameText);

    qreal y = fontHeight * 2;
    fnt.setBold(false);
    fnt.setItalic(false);
    painter->setFont(fnt);

    foreach(UMLClassifierListItem* enumLiteral , list ) {
        fontRect.moveTop(y);
        painter->drawText(fontRect, Qt::AlignVCenter, enumLiteral->getName());
        y += fontHeight;
    }
}

bool NewEnumWidget::loadFromXMI( QDomElement & qElement )
{
    if( !NewUMLRectWidget::loadFromXMI(qElement) ) {
        return false;
    }
    bool show = bool(qElement.attribute("showpackage", "0").toInt());
    setShowPackage(show);
    return true;
}

void NewEnumWidget::saveToXMI( QDomDocument& qDoc, QDomElement& qElement ) {
    QDomElement conceptElement = qDoc.createElement("enumwidget");
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);

    conceptElement.setAttribute("showpackage", m_showPackage);
    qElement.appendChild(conceptElement);
}

void NewEnumWidget::updateGeometry()
{
    calculateMinimumSize();
    NewUMLRectWidget::updateGeometry();
}

void NewEnumWidget::calculateMinimumSize()
{
    if(!umlObject()) {
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

    const int numberOfEnumLiterals = static_cast<UMLEnum*>(umlObject())->enumLiterals();
    lines += numberOfEnumLiterals;

    height = width = 0;
    //set the height of the enum
    if(numberOfEnumLiterals == 0) {
        height += 0.5 * fontHeight; //no enum literals, so just add a bit of space
    }

    height += lines * fontHeight;

    font.setBold(true);
    font.setItalic(true);
    QString nameText = name();
    if(m_showPackage && umlObject()) {
        nameText = umlObject()->getFullyQualifiedName();
    }
    width = QFontMetricsF(font).boundingRect(nameText).width();
    font.setItalic(false);

    if(umlObject()) {
        qreal w = QFontMetricsF(font).boundingRect(umlObject()->getStereotype(true)).width();
        width = qMax(w, width);

        UMLClassifier *classifier = static_cast<UMLClassifier*>(umlObject());
        UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EnumLiteral);
        UMLClassifierListItem* listItem = 0;

        foreach(listItem , list ) {
            qreal w = fm.width(listItem->getName());
            if(w > width)
                width = w;
        }
    }
    //allow for width margin
    width += ENUM_MARGIN * 2;

    m_minimumSize.setWidth(width);
    m_minimumSize.setHeight(height);
}
