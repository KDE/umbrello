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
#include <QtCore/QRectF>
#include <QtCore/QSizeF>
#include <QtGui/QBrush>
#include <QtGui/QColor>
#include <QtGui/QFont>

class QGraphicsItem;
class TextItem;

class TextItemGroup
{
public:
    static const qreal NoLineBreak;

	TextItemGroup(QGraphicsItem *parent = 0);
    ~TextItemGroup();

    QGraphicsItem* parentItem() const {
        return m_parentItem;
    }
    void setParentItem(QGraphicsItem *item);

	int indexOf(TextItem* item) const {
		return m_textItems.indexOf(item);
	}
    TextItem* textItemAt(int index);

	int textItemCount() const {
        return m_textItems.size();
    }
	void setTextItemCount(int count);

	const QList<TextItem*> &textItems() const {
		return m_textItems;
	}

    void appendTextItem(TextItem *item);
    void insertTextItemAt(int i, TextItem *item);
    void deleteTextItem(TextItem *item);
    void deleteTextItemAt(int index);

	qreal lineBreakWidth() const {
        return m_lineBreakageWidth;
    }
    void setLineBreakWidth(qreal w);

	QSizeF minimumSize() const;

	QRectF groupGeometry() const {
		return m_groupGeometry;
	}
    void setGroupGeometry(const QRectF& rect);

	Qt::Alignment alignment() const {
		return m_alignment;
	}
	void setAlignment(Qt::Alignment align);

	QColor fontColor() const {
		return m_fontColor;
	}
	void setFontColor(const QColor& color);

	QBrush hoverBrush() const {
		return m_hoverBrush;
	}
	void setHoverBrush(const QBrush& brush);

	QBrush backgroundBrush() const {
		return m_backgroundBrush;
	}
	void setBackgroundBrush(const QBrush& brush);

	QFont font() const {
		return m_font;
	}
	void setFont(const QFont& font);

private:
    QGraphicsItem *m_parentItem;
    QList<TextItem*> m_textItems;
	QRectF m_groupGeometry;
    qreal m_lineBreakageWidth;

	Qt::Alignment m_alignment;
	QColor m_fontColor;
	QBrush m_hoverBrush;
	QBrush m_backgroundBrush;
	QFont m_font;
};


#endif //TEXTITEMGROUP_H
