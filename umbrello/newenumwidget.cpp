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
#include "classifier.h"
#include "classifierlistitem.h"
#include "enum.h"
#include "enumliteral.h"
#include "optionstate.h"
#include "textitemgroup.h"
#include "textitem.h"
#include "umlclassifierlistitemlist.h"
#include "umlscene.h"
#include "widget_utils.h"

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
    m_showPackage(false)
{
    m_textItemGroup = new TextItemGroup(this);
    m_baseType = Uml::wt_Enum;
    init();
}

NewEnumWidget::~NewEnumWidget()
{
    delete m_textItemGroup;
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
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    // First draw the outer rectangle with the pen and brush of this widget.
    painter->drawRect(rect());

    const TextItem *item = m_textItemGroup->textItemAt(NameItemIndex);
    const QPointF bottomLeft = item->mapToParent(item->boundingRect().bottomLeft());
    const qreal y = bottomLeft.y();
    painter->drawLine(QLineF(0, y, size().width() - 1, y));
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
    if(umlObject()) {
        UMLClassifier *classifier = static_cast<UMLClassifier*>(umlObject());
        UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EnumLiteral);
        int totalTextItems = list.size() + 2; // +2 because stereo text + name text.
        bool shouldAlign = false;

        QBrush hoverBrush = randHoverBrush();

        m_textItemGroup->unparent();

        TextItem dummy("");
        dummy.setDefaultTextColor(fontColor());
        dummy.setFont(font());
        dummy.setAcceptHoverEvents(true);
        dummy.setHoverBrush(hoverBrush);
        dummy.setAlignment(Qt::AlignCenter);
        dummy.setBackgroundBrush(Qt::NoBrush);

        if(m_textItemGroup->size() != totalTextItems) {
            while(m_textItemGroup->size() < totalTextItems) {
                m_textItemGroup->appendTextItem(new TextItem(""));
            }

            while(m_textItemGroup->size() > totalTextItems) {
                int lastIndex = m_textItemGroup->textItems().size() - 1;
                m_textItemGroup->deleteTextItemAt(lastIndex);
            }
            shouldAlign = true;
        }

        TextItem *stereo = m_textItemGroup->textItemAt(StereoTypeItemIndex);
        stereo->setText(classifier->getStereotype(true));
        dummy.copyAttributesTo(stereo);
        stereo->setBold(true);
        stereo->setToolTip(stereo->text());

        TextItem *nameItem = m_textItemGroup->textItemAt(NameItemIndex);
        nameItem->setText(m_showPackage ?
                          classifier->getFullyQualifiedName() :
                          name());
        dummy.copyAttributesTo(nameItem);
        nameItem->setBold(true);
        nameItem->setItalic(classifier->getAbstract());
        nameItem->setToolTip(nameItem->text());

        int index = NameItemIndex + 1;
        foreach(UMLClassifierListItem* enumLiteral, list) {
            TextItem *literal = m_textItemGroup->textItemAt(index);
            literal->setText(enumLiteral->getName());
            dummy.copyAttributesTo(literal);
            literal->setToolTip(literal->text());
            ++index;
        }

        m_minimumSize = m_textItemGroup->calculateMinimumSize();
        m_minimumSize.rwidth() += ENUM_MARGIN * 2;

        m_textItemGroup->reparent();
    }
    NewUMLRectWidget::updateGeometry();
}

void NewEnumWidget::sizeHasChanged(const QSizeF& sz)
{
    m_textItemGroup->alignVertically(size().width() - 2*ENUM_MARGIN, size().height());
    m_textItemGroup->setPos(QPointF(ENUM_MARGIN, 0));

    NewUMLRectWidget::sizeHasChanged(sz);
}
