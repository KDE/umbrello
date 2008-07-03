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
#include "textitem.h"

QBrush randHoverBrush()
{
    QLinearGradient grad(QPointF(0,0), QPointF(0, 1));
    int r, g, b, a;
    r = qrand() % 255;
    g = qrand() % 255;
    b = qrand() % 255;
    a = 10 + qrand() % 245; // set minimum to atleast 10
    grad.setColorAt(0, QColor(r, g, b, a));

    r = qrand() % 255;
    g = qrand() % 255;
    b = qrand() % 255;
    a = 10 + qrand() % 245; // set minimum to atleast 10
    grad.setColorAt(1, QColor(r, g, b, a));

    return QBrush(grad);
}

NewEnumWidget::NewEnumWidget(UMLObject* o) :
    NewUMLRectWidget(o),
    m_showPackage(false),
    m_stereoTypeItem(0),
    m_nameItem(0)
{
    m_baseType = Uml::wt_Enum;
}

NewEnumWidget::~NewEnumWidget()
{
    cleanup();
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
    const QSizeF sz = size();
    const QSizeF minSz = sizeHint(Qt::MinimumSize);

    // if(sz.height() > minSz.height()) {
    //     fontHeight += (sz.height() - minSz.height()) / qreal(list.size() + 2);
    // }

    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());
    // First draw the outer rectangle with the pen and brush of this widget.
    painter->drawRect(rect());

    if(m_nameItem) {
        QPointF botLeft = m_nameItem->boundingRect().bottomLeft();
        botLeft = m_nameItem->mapToParent(botLeft);
        qreal y = botLeft.y();
        painter->drawLine(QLineF(0, y, sz.width() - 1, y));
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
    cleanup();
    if(umlObject()) {
        qreal maxWidth = 0;
        UMLClassifier *classifier = static_cast<UMLClassifier*>(umlObject());

        QBrush hoverBrush = randHoverBrush();
        // Setup the steretype item first.

        m_stereoTypeItem = new TextItem(umlObject()->getStereotype(true));
        m_stereoTypeItem->setToolTip(m_stereoTypeItem->toPlainText());
        m_stereoTypeItem->setDefaultTextColor(fontColor());
        m_stereoTypeItem->setFont(font());
        m_stereoTypeItem->setBold(true);
        m_stereoTypeItem->setAcceptHoverEvents(true);
        m_stereoTypeItem->setHoverBrush(hoverBrush);
        m_stereoTypeItem->setAlignment(Qt::AlignCenter);
        m_stereoTypeItem->adjustSize();

        maxWidth = qMax(maxWidth, m_stereoTypeItem->boundingRect().width());

        // Now setup the name item.

        QString nameText = name();
        if(m_showPackage) {
            nameText = umlObject()->getFullyQualifiedName();
        }
        m_nameItem = new TextItem(nameText);
        m_nameItem->setToolTip(m_nameItem->toPlainText());
        m_nameItem->setDefaultTextColor(fontColor());
        m_nameItem->setFont(font());
        m_nameItem->setBold(true);
        m_nameItem->setItalic(classifier->getAbstract());
        m_nameItem->setAcceptHoverEvents(true);
        m_nameItem->setHoverBrush(hoverBrush);
        m_nameItem->setAlignment(Qt::AlignCenter);
        m_nameItem->adjustSize();

        maxWidth = qMax(maxWidth, m_nameItem->boundingRect().width());

        // Now setup all the literals

        foreach(UMLClassifierListItem* enumLiteral ,
                classifier->getFilteredList(Uml::ot_EnumLiteral)) {
            TextItem *literal = new TextItem(enumLiteral->getName());
            literal->setToolTip(literal->toPlainText());
            literal->setDefaultTextColor(fontColor());
            literal->setFont(font());
            literal->setAcceptHoverEvents(true);
            literal->setHoverBrush(hoverBrush);
            literal->setAlignment(Qt::AlignCenter);
            literal->adjustSize();

            m_enumLiteralItems.append(literal);
            maxWidth = qMax(maxWidth, literal->boundingRect().width());
        }

        // Now align all these text items.
        maxWidth = qMin(maxWidth, QFontMetricsF(font()).width('w') * 20);
        maxWidth += ENUM_MARGIN * 2;
        qreal y = 0;
        QRectF newBound;

        m_stereoTypeItem->setTextWidth(maxWidth);
        m_stereoTypeItem->setParentItem(this);
        m_stereoTypeItem->setPos(0, y);
        y += m_stereoTypeItem->boundingRect().height();
        newBound |= m_stereoTypeItem->sceneBoundingRect();

        m_nameItem->setTextWidth(maxWidth);
        if(umlScene()) {
            umlScene()->addItem(m_nameItem);
        }
        m_nameItem->setParentItem(this);
        m_nameItem->setPos(0, y);
        y += m_nameItem->boundingRect().height();
        newBound |= m_nameItem->sceneBoundingRect();

        foreach(TextItem *literal, m_enumLiteralItems) {
            literal->setTextWidth(maxWidth);
            if(umlScene()) {
                umlScene()->addItem(literal);
            }
            literal->setParentItem(this);
            literal->setPos(0, y);
            y += literal->boundingRect().height();
            newBound |= literal->sceneBoundingRect();
        }

        m_minimumSize = newBound.size();
    }
    NewUMLRectWidget::updateGeometry();
}

void NewEnumWidget::sizeHasChanged(const QSizeF& sz)
{
    const QSizeF minSz = sizeHint(Qt::MinimumSize);

    int visibleItems = 0;

    if(m_stereoTypeItem) {
        m_stereoTypeItem->setTextWidth(sz.width());
        ++visibleItems;
    }

    if(m_nameItem) {
        m_nameItem->setTextWidth(sz.width());
        ++visibleItems;
    }

    foreach(TextItem *literal, m_enumLiteralItems) {
        literal->setTextWidth(sz.width());
        ++visibleItems;
    }

    qreal spacing = 0;
    if(sz.height() - minSz.height() > 0 && visibleItems != 0) {
        spacing = (sz.height() - minSz.height()) / qreal(visibleItems);
    }

    if(spacing > 0) {
        qreal y = 0;
        if(m_stereoTypeItem) {
            m_stereoTypeItem->setPos(0, y);
            y += m_stereoTypeItem->boundingRect().height() + spacing;
        }

        if(m_nameItem) {
            m_nameItem->setPos(0, y);
            y += m_nameItem->boundingRect().height() + spacing;
        }

        foreach(TextItem *literal, m_enumLiteralItems) {
            literal->setPos(0, y);
            y += literal->boundingRect().height() + spacing;
        }
    }
    NewUMLRectWidget::sizeHasChanged(sz);
}

void NewEnumWidget::cleanup()
{
    delete m_stereoTypeItem;
    delete m_nameItem;
    qDeleteAll(m_enumLiteralItems);

    m_stereoTypeItem = m_nameItem = 0;
    m_enumLiteralItems.clear();
}
