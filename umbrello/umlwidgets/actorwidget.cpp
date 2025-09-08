/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header file
#include "actorwidget.h"

// local includes
#include "umlactor.h"
#include "debug_utils.h"
#include "umlview.h"

// qt includes
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(ActorWidget)

/**
 * Constructs an ActorWidget.
 *
 * @param scene   The parent of this ActorWidget.
 * @param a      The Actor class this ActorWidget will display.
 */
ActorWidget::ActorWidget(UMLScene * scene, UMLActor *a)
  : UMLWidget(scene, WidgetBase::wt_Actor, a)
{
    setFixedAspectRatio(true);
}

/**
 * Destructor.
 */
ActorWidget::~ActorWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void ActorWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    UMLWidget::setPenFromSettings(painter);
    if(UMLWidget::useFillColor())
        painter->setBrush(UMLWidget::fillColor());
    const int w = width();
    const int h = height();
    painter->setFont(UMLWidget::font());
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight = fm.lineSpacing();
    bool drawStereotype = umlObject() && !umlObject()->stereotype().isEmpty();
    const int a_height = h - (drawStereotype ? 2 * fontHeight : fontHeight) - A_MARGIN;
    const int h2 = a_height / 2;
    const int a_width = (h2);
    const int middleX = w / 2;
    const int thirdY = a_height / 3;

    //draw actor
    painter->drawEllipse(middleX - a_width / 2, 0, a_width, thirdY); //head
    painter->drawLine(middleX, thirdY,
               middleX, thirdY * 2); //body
    painter->drawLine(middleX, 2 * thirdY,
               middleX - a_width / 2, a_height); //left leg
    painter->drawLine(middleX,  2 * thirdY,
               middleX + a_width / 2, a_height); //right leg
    painter->drawLine(middleX - a_width / 2, thirdY + thirdY / 2,
               middleX + a_width / 2, thirdY + thirdY / 2); //arms
    //draw text
    painter->setPen(textColor());
    if (drawStereotype)
        painter->drawText(A_MARGIN, h - 2 * fontHeight, w - A_MARGIN * 2, fontHeight, Qt::AlignCenter, umlObject()->stereotype(true));
    painter->drawText(A_MARGIN, h - fontHeight,
               w - A_MARGIN * 2, fontHeight, Qt::AlignCenter, name());
    UMLWidget::paint(painter, option, widget);
}

/**
 * Saves the widget to the "actorwidget" XMI element.
 * Note: For loading from XMI, the inherited parent method is used.
 */
void ActorWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("actorwidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeEndElement();
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF ActorWidget::minimumSize() const
{
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int textWidth = fm.horizontalAdvance(name());
    bool drawStereotype = umlObject() && !umlObject()->stereotype().isEmpty();
    int width = textWidth > A_WIDTH ? textWidth : A_WIDTH;
    int height = A_HEIGHT + (drawStereotype ? 2 * fontHeight : fontHeight) + A_MARGIN;
    width += A_MARGIN * 2;
    return QSizeF(width, height);
}


