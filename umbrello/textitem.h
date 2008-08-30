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

#include <QtGui/QGraphicsTextItem>
#include <QtGui/QBrush>
#include <QtGui/QFont>

class QGraphicsSceneContextMenuEvent;

class TextItem : public QGraphicsTextItem
{
    Q_OBJECT
public:

    TextItem(const QString& text, QGraphicsItem *parent = 0);
    ~TextItem();

    /**
     * @return The alignment for the item's text.
     * @see setAlignment
     */
    Qt::Alignment alignment() const;

    /**
     * Set the alignment for the item's text.
     *
     * @param align An OR combination specifying the alignment.
     *
     * @note The text width of this item should be set by setTextWidth for
     *       center alignment.
     */
    void setAlignment(Qt::Alignment align);

    bool bold() const {
        return font().bold();
    }

    /**
     * Sets the font weight to \a b
     *
     * @param b True to enable bold, false otherwise.
     */
    void setBold(bool b);

    bool italic() const {
        return font().italic();
    }

    /**
     * Sets the italic property of font to \a b.
     */
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

    /**
     * Reimplemented to retain the italic/bold status on font change.
     *
     * It is important to note that, this method retains the old bold,
     * italic and underline property of the font and hence all these
     * properties should be set explicitly.
     *
     * @note This overrides (hides) the base's non virtual method.
     */
    void setFont(QFont font);


    QBrush hoverBrush() const {
        return m_hoverBrush;
    }

    /**
     * Sets the brush to paint background of text on mouse hover.  Default
     * brush = Qt::NoBrush
     *
     * @note The hover events should be enabled for this to work.
          * @see QGraphicsTextItem::setAcceptHoverEvent(bool)
     */
    void setHoverBrush(const QBrush& brush);


    QBrush backgroundBrush() const {
        return m_backgroundBrush;
    }

    /**
     * Sets the background brush to \a brush which is used to paint as
     * background of text.
     * Default brush = Qt::NoBrush
     */
    void setBackgroundBrush(const QBrush& brush);

    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w);

    /**
     * This is a utility method to copy all the attributes of this
     * TextItem to \a other TextItem.
     */
    void copyAttributesTo(TextItem *other) const;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    QBrush m_hoverBrush;
    QBrush m_backgroundBrush;

};

#endif //TEXTITEM_H
