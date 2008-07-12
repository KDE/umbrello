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

#ifndef TEXTITEMGROUP_H
#define TEXTITEMGROUP_H

#include <QtCore/QList>
#include <QtCore/QPointF>
#include <QtCore/QSizeF>

class QGraphicsItem;
class TextItem;
class QSizeF;

class TextItemGroup
{
public:
    static const qreal NoLineBreak;
    TextItemGroup( QGraphicsItem *parent = 0 );
    ~TextItemGroup();

    QGraphicsItem* parentItem() const {
        return m_parentItem;
    }
    void setParentItem(QGraphicsItem *item);

    void appendTextItem(TextItem *item);

    void deleteTextItem(TextItem *item);
    void deleteTextItemAt(int index);

    bool isIndexValid(int index) const;

    int indexOf(TextItem* item) const;
    TextItem* textItemAt(int index) const;

    const QList<TextItem*> &textItems() const;
    int size() const {
        return m_textItems.size();
    }

    QSizeF calculateMinimumSize();

    qreal lineBreakWidth() const {
        return m_lineBreakageWidth;
    }
    void setLineBreakWidth(qreal w);

    void alignVertically(qreal currentWidth, qreal currentHeight);
    void alignVertically(const QSizeF& size) {
        alignVertically(size.width(), size.height());
    }

    QPointF pos() const {
        return m_pos;
    }
    void setPos(const QPointF& pos);

    void unparent();
    void reparent();

    void ensureTextItemNumbers(int count);

private:
    QGraphicsItem *m_parentItem;
    QList<TextItem*> m_textItems;
    QPointF m_pos;
    qreal m_lineBreakageWidth;
};


#endif //TEXTITEMGROUP_H
