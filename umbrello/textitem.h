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

#ifndef TEXTITEM_H
#define TEXTITEM_H

#include <QBrush>
#include <QFont>
#include <QGraphicsTextItem>

class QGraphicsItem;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneHoverEvent;
class QStyleOptionGraphicsItem;

class TextItem : public QGraphicsTextItem
{
    Q_OBJECT
public:
    TextItem(const QString& text, QGraphicsItem *parent = 0);
    virtual ~TextItem();

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment align);

    bool bold() const {
        return font().bold();
    }
    void setBold(bool b);

    bool italic() const {
        return font().italic();
    }
    void setItalic(bool i);

    bool underline() const {
        return font().underline();
    }
    void setUnderline(bool u);

    qreal width() const {
        return boundingRect().width();
    }
    qreal height() const {
        return boundingRect().height();
    }

    QString text() const;
    void setText(const QString& text);

    void setFont(QFont font);

    QBrush hoverBrush() const {
        return m_hoverBrush;
    }
    void setHoverBrush(const QBrush& brush);

    QBrush backgroundBrush() const {
        return m_backgroundBrush;
    }
    void setBackgroundBrush(const QBrush& brush);

    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w);

    void copyAttributesTo(TextItem *other) const;

    bool explicitVisiblity() const {
        return m_explicitVisiblity;
    }
    void setExplicitVisibility(bool b);
    void updateVisibility();

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    QBrush m_hoverBrush;
    QBrush m_backgroundBrush;
    bool   m_explicitVisiblity;
};

#endif //TEXTITEM_H
