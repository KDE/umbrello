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

/**
 * @short A flexible item used to display text and is tunable for
 *        specific needs.
 *
 * This item supports hovering effect, editing, fonts, text color,
 * tooltip, alignment and much more. All these are configurable.
 *
 * To make the item movable, use @ref
 * QGraphicsItem::setFlag(QGraphicsItem::ItemIsMovable, true)
 *
 * To set some properties inherited from QGraphicsTextItem use the
 * following
 *
 * Text color    @ref QGraphicsTextItem::setDefaultTextColor(QColor)
 *
 * ToolTip       @ref QGraphicsTextItem::setToolTip(QString)
 *
 * Hovering      @ref QGraphicsTextItem::setAcceptHoverEvent(bool)
 *
 * Editing text  @ref QGraphicsTextItem::setTextInteractionFlags(Qt::TextInteractionFlags)
 *               @ref Qt::TextInteractionFlags
 *               Set to Qt::TextEditable to enable editing.
 */
class TextItem : public QGraphicsTextItem
{
Q_OBJECT
public:

    /**
     * Constructs a TextItem object with its text content set to \a
     * text and its parent item being \a parent.
     *
     * The default color is black, hover and background brush is Qt::NoBrush.
     * Hover events are disabled by default.
     *
     * @param text The text content this item should represent (plain
     *             text)
     *
     * @param parent The parent of this item.
     */
    TextItem(const QString& text, QGraphicsItem *parent = 0);

    /**
     * Destructor
     */
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
     * @note The text width of this item should be set by setTextWidth
     *       for center alignment.
     */
    void setAlignment(Qt::Alignment align);

    /**
     * @return Returns the font's weight status.
     */
    bool bold() const {
        return font().bold();
    }
    /**
     * Sets the font weight to \a b
     *
     * @param b True to enable bold, false otherwise.
     */
    void setBold(bool b);

    /**
     * @return True if the font's style is QFont::StyleItalic
     */
    bool italic() const {
        return font().italic();
    }
    /**
     * Sets the font's style to italic or normal based on i.
     *
     * @param i True to enable italic, false otherwise.
     */
    void setItalic(bool i);

    /**
     * Reimplemented to retain the italic/bold status on font change.
     *
     * @note This overrides (hides) the base's non virtual method.
     */
    void setFont(QFont font);

    /**
     * @return The brush used to paint the background of text on mouse
     *         hover.
     *
     * @note The hover events should be enabled for this to work.
     * @see QGraphicsTextItem::setAcceptHoverEvent(bool)
     */
    QBrush hoverBrush() const {
        return m_hoverBrush;
    }
    /**
     * Sets the brush to paint background of text on mouse hover.
     * Default brush = Qt::NoBrush
     *
     * @note The hover events should be enabled for this to work.
     * @see QGraphicsTextItem::setAcceptHoverEvent(bool)
     */
    void setHoverBrush(const QBrush& brush);

    /**
     * @return The brush to paing background of text (all time except
     *         hover)
     */
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


protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    QBrush m_hoverBrush;
    QBrush m_backgroundBrush;

};

#endif //TEXTITEM_H
