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

#include "textitemgroup.h"
#include "textitem.h"
#include "umlnamespace.h" // for uDebug

#include <QtGui/QGraphicsScene>

const qreal TextItemGroup::NoLineBreak = -1;

TextItemGroup::TextItemGroup(QGraphicsItem *parent) :
    m_parentItem(parent),
    m_lineBreakageWidth(TextItemGroup::NoLineBreak)
{
}

TextItemGroup::~TextItemGroup()
{
    if(!m_parentItem) {
        qDeleteAll(m_textItems);
    }
}

void TextItemGroup::setParentItem(QGraphicsItem *item)
{
    foreach(TextItem *textItem, m_textItems) {
        textItem->setParentItem(item);
    }
}

void TextItemGroup::appendTextItem(TextItem *textItem)
{
    if(m_textItems.contains(textItem)) {
        uDebug() << "TextItem(" << (void*)textItem << ") "
                 << textItem->text() << " is already in the group";
        return;
    }

    textItem->setParentItem(m_parentItem);
    m_textItems.append(textItem);
}

void TextItemGroup::deleteTextItem(TextItem *textItem)
{
    deleteTextItemAt(indexOf(textItem));
}

void TextItemGroup::deleteTextItemAt(int index)
{
    if(!isIndexValid(index)) {
        uDebug() << "Invalid index " << index;
        return;
    }

    delete m_textItems.takeAt(index);
}

bool TextItemGroup::isIndexValid(int index) const
{
    return index >= 0 && index < m_textItems.size();
}

int TextItemGroup::indexOf(TextItem *item) const
{
    return m_textItems.indexOf(item);
}

TextItem* TextItemGroup::textItemAt(int index) const
{
    if(!isIndexValid(index)) {
        uDebug() << "Wrong index : " << index;
        return 0;
    }
    return m_textItems[index];
}

const QList<TextItem*>& TextItemGroup::textItems() const
{
    return m_textItems;
}

QSizeF TextItemGroup::calculateMinimumSize()
{
    qreal width = 0;
    qreal height = 0;

    TextItem *calcItem = new TextItem("");
    calcItem->setTextWidth(-1);

    foreach(TextItem *item, m_textItems) {
        if(!item->isVisible()) {
            continue;
        }

        item->copyAttributesTo(calcItem);
        calcItem->setText(item->text());

        if(m_lineBreakageWidth != NoLineBreak) {
            if(calcItem->width() > m_lineBreakageWidth) {
                calcItem->setTextWidth(m_lineBreakageWidth);
            }
        }
        else {
            calcItem->setTextWidth(-1);
        }

        width = qMax(calcItem->width(), width);
        height += calcItem->height();
    }

    delete calcItem;

    return QSizeF(width, height);
}

void TextItemGroup::setLineBreakWidth(qreal width)
{
    m_lineBreakageWidth = width;
}

void TextItemGroup::alignVertically(qreal currentWidth, qreal currentHeight)
{
	// Update current size
	m_currentSize.setWidth(currentWidth);
	m_currentSize.setHeight(currentHeight);

    QSizeF minimumSize = calculateMinimumSize();
    if(currentWidth < minimumSize.width() || currentHeight < minimumSize.height()) {
        uDebug() << "Size of group less then minimum size\n"
                 << "Min size" << minimumSize << endl
                 << "Cur size" << QSizeF(currentWidth, currentWidth);

        return;
    }

    QList<TextItem*> visibleItems;
    foreach(TextItem *item, m_textItems) {
        if(item->isVisible()) {
            visibleItems << item;
        }
    }

    qreal spacing = (currentHeight - minimumSize.height()) / visibleItems.count();
    qreal x = m_pos.x();
    qreal y = m_pos.y();

    foreach(TextItem *item, visibleItems) {
        item->setTextWidth(currentWidth);
        item->setPos(x, y);
        y += item->height() + spacing;
    }
}

void TextItemGroup::setPos(const QPointF& pos)
{
    qreal dx = pos.x() - m_pos.x();
    qreal dy = pos.y() - m_pos.y();

    foreach(TextItem *item, m_textItems) {
        item->moveBy(dx, dy);
    }

    m_pos = pos;
}

void TextItemGroup::unparent()
{
    foreach(TextItem *item, m_textItems) {
        item->setParentItem(0);
        if(item->scene()) {
            item->scene()->removeItem(item);
        }
    }
}

void TextItemGroup::reparent()
{
    foreach(TextItem *item, m_textItems) {
        item->setParentItem(m_parentItem);
    }
}

void TextItemGroup::ensureTextItemCount(int newSize)
{
    while(size() < newSize) {
        appendTextItem(new TextItem(""));
    }

    while(size() >  newSize) {
        deleteTextItemAt(0);
    }
}
