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

#include "debug_utils.h"
#include "textitem.h"

const qreal TextItemGroup::NoLineBreak = -1;

TextItemGroup::TextItemGroup(QGraphicsItem *parent)
  : m_parentItem(parent),
    m_lineBreakageWidth(TextItemGroup::NoLineBreak),
    m_margin(0),
    m_alignment(Qt::AlignCenter),
    m_textColor(Qt::black),
    m_hoverBrush(Qt::NoBrush),
    m_backgroundBrush(Qt::NoBrush)
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

TextItem* TextItemGroup::textItemAt(int index)
{
    if(index < 0 || index >= m_textItems.size()) {
        uError() << "TextItemGroup::textItemAt: Invalid index %d" << index;
        return 0;
    }

    return m_textItems[index];
}

void TextItemGroup::setTextItemCount(int count)
{
    while(textItemCount() < count) {
        appendTextItem(new TextItem(""));
    }

    while(textItemCount() >  count) {
        deleteTextItemAt(0);
    }
}

void TextItemGroup::appendTextItem(TextItem *textItem)
{
    if(m_textItems.contains(textItem)) {
        uDebug() << "TextItem(" << (void*)textItem << ") "
                 << textItem->text() << " is already in the group";
        return;
    }

    textItem->setAlignment(m_alignment);
    textItem->setDefaultTextColor(m_textColor);
    textItem->setHoverBrush(m_hoverBrush);
    textItem->setBackgroundBrush(m_backgroundBrush);
    textItem->setFont(m_font);

    textItem->setParentItem(m_parentItem);
    m_textItems.append(textItem);
}

void TextItemGroup::insertTextItemAt(int i, TextItem *textItem)
{
    if(m_textItems.contains(textItem)) {
        uDebug() << "TextItem(" << (void*)textItem << ") "
                 << textItem->text() << " is already in the group";
        return;
    }

    textItem->setAlignment(m_alignment);
    textItem->setDefaultTextColor(m_textColor);
    textItem->setHoverBrush(m_hoverBrush);
    textItem->setBackgroundBrush(m_backgroundBrush);
    textItem->setFont(m_font);

    textItem->setParentItem(m_parentItem);
    m_textItems.insert(i, textItem);
}

void TextItemGroup::deleteTextItem(TextItem *textItem)
{
    deleteTextItemAt(indexOf(textItem));
}

void TextItemGroup::deleteTextItemAt(int index)
{
    delete m_textItems.takeAt(index);
}

void TextItemGroup::setLineBreakWidth(qreal width)
{
    m_lineBreakageWidth = width;
}

QSizeF TextItemGroup::minimumSize() const
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

    width += 2 * m_margin;
    height += 2 * m_margin;

    delete calcItem;

    return QSizeF(width, height);
}

void TextItemGroup::setGroupGeometry(const QRectF& rect)
{
    QSizeF minSize = minimumSize();

    if(rect.width() < minSize.width() || rect.height() < minSize.height()) {
        uDebug() << "Size of group less then minimum size"  << endl
                 << "Min size" << minSize << endl
                 << "Cur size" << rect.size();

        return;
    }

    QList<TextItem*> visibleItems;
    foreach(TextItem *item, m_textItems) {
        if(item->isVisible()) {
            visibleItems << item;
        }
    }

    qreal spacing = (rect.height() - minSize.height()) / (visibleItems.count() + 1);
    qreal x = rect.left() + m_margin;
    qreal y = (rect.top() + m_margin) + spacing;


    qreal textWidth = rect.width() - 2 * margin();
    foreach(TextItem *item, visibleItems) {
        item->setTextWidth(textWidth);
        item->setPos(x, y);
        y += item->height() + spacing;
    }

    m_groupGeometry = rect;
}

void TextItemGroup::updateVisibility()
{
    foreach (TextItem *item, m_textItems) {
        item->updateVisibility();
    }
}

void TextItemGroup::setAlignment(Qt::Alignment align)
{
    m_alignment = align;
    foreach(TextItem *item, m_textItems) {
        item->setAlignment(align);
    }
}

void TextItemGroup::setTextColor(const QColor& color)
{
    m_textColor = color;
    foreach(TextItem *item, m_textItems) {
        item->setDefaultTextColor(color);
    }
}

void TextItemGroup::setHoverBrush(const QBrush& brush)
{
    m_hoverBrush = brush;
    foreach(TextItem *item, m_textItems) {
        item->setHoverBrush(brush);
    }
}

void TextItemGroup::setBackgroundBrush(const QBrush& brush)
{
    m_backgroundBrush = brush;
    foreach(TextItem *item, m_textItems) {
        item->setBackgroundBrush(brush);
    }
}

void TextItemGroup::setFont(const QFont& font)
{
    m_font = font;
    foreach(TextItem *item, m_textItems) {
        QFont newFont = m_font;
        newFont.setBold(item->bold());
        newFont.setItalic(item->italic());
        newFont.setUnderline(item->underline());

        item->setFont(newFont);
    }
}
