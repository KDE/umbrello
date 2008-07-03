/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "textitem.h"

#include <QtGui/QTextOption>
#include <QtGui/QTextDocument>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>

TextItem::TextItem(const QString& text, QGraphicsItem *parent) :
    QGraphicsTextItem(text, parent)
{
    setAcceptHoverEvents(false);
}

TextItem::~TextItem()
{
}

Qt::Alignment TextItem::alignment() const
{
    return document()->defaultTextOption().alignment();
}

void TextItem::setAlignment(Qt::Alignment align)
{
    QTextOption opt = document()->defaultTextOption();
    opt.setAlignment(align);
    document()->setDefaultTextOption(opt);
}

void TextItem::setBold(bool b)
{
    QFont f = font();
    if(b != f.bold()) {
        f.setBold(b);
        QGraphicsTextItem::setFont(f);
    }
}

void TextItem::setItalic(bool b)
{
    QFont f = font();
    if(b != f.italic()) {
        f.setItalic(b);
        QGraphicsTextItem::setFont(f);
    }
}

void TextItem::setFont(QFont f)
{
    const QFont old = font();

    f.setBold(old.bold());
    f.setItalic(old.italic());

    QGraphicsTextItem::setFont(f);
}

void TextItem::setHoverBrush(const QBrush& brush)
{
    m_hoverBrush = brush;
    update();
}

void TextItem::setBackgroundBrush(const QBrush& brush)
{
    m_backgroundBrush = brush;
    update();
}

void TextItem::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    const QRectF rect = o->exposedRect;

    if(o->state & QStyle::State_MouseOver) {
        p->setBrush(m_hoverBrush);
        p->setPen(Qt::NoPen);

        p->drawRect(rect);
    }
    else if(m_backgroundBrush.style() != Qt::NoBrush) {
        p->setBrush(m_backgroundBrush);
        p->setPen(Qt::NoPen);

        p->drawRect(rect);
    }

    QGraphicsTextItem::paint(p, o, w);
}

void TextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    update();
}

void TextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    update();
}

#include "textitem.moc"
