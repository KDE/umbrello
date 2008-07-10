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

#include "umlnamespace.h"

#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QTextDocument>
#include <QtGui/QTextOption>

// Inline and other documentation BEGIN

/**
 * @class TextItem
 *
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

/**
 * @fn TextItem::bold
 *
 * @return Returns the font's weight status.
 */

/**
 * @fn TextItem::italic
 *
 * @return Returns the font's italic status.
 */

/**
 * @fn TextItem::hoverBrush
 *
 * @return The brush used to paint the background of text on mouse
 *         hover.
 *
 * @note The hover events should be enabled for this to work.
 * @see QGraphicsTextItem::setAcceptHoverEvent(bool)
 */

/**
 * @fn TextItem::backgroundBrush
 *
 * @return The brush to paing background of text (all time except
 *         hover)
 */


// Inline and other documentation ENDS

/**
 * Constructs a TextItem object with its text content set to \a text
 * and its parent item being \a parent.
 *
 * The default color is black, hover and background brush is
 * Qt::NoBrush.  Hover events are disabled by default.
 *
 * @param text The text content this item should represent (plain
 *             text)
 *
 * @param parent The parent of this item.
 */
TextItem::TextItem(const QString& text, QGraphicsItem *parent) :
    QGraphicsTextItem(text, parent)
{
    setAcceptHoverEvents(false);
}

/**
 * Destructor
 */
TextItem::~TextItem()
{
}

/**
 * @return The alignment for the item's text.
 * @see setAlignment
 */
Qt::Alignment TextItem::alignment() const
{
    return document()->defaultTextOption().alignment();
}

QString TextItem::text() const
{
    return toPlainText();
}

void TextItem::setText(const QString& text)
{
    setPlainText(text);
}

/**
 * Set the alignment for the item's text.
 *
 * @param align An OR combination specifying the alignment.
 *
 * @note The text width of this item should be set by setTextWidth for
 *       center alignment.
 */
void TextItem::setAlignment(Qt::Alignment align)
{
    QTextOption opt = document()->defaultTextOption();
    opt.setAlignment(align);
    document()->setDefaultTextOption(opt);
}

/**
 * Sets the font weight to \a b
 *
 * @param b True to enable bold, false otherwise.
 */
void TextItem::setBold(bool b)
{
    QFont f = font();
    if(b != f.bold()) {
        f.setBold(b);
        QGraphicsTextItem::setFont(f);
    }
}

/**
 * Sets the italic property of font to \a b.
 */
void TextItem::setItalic(bool b)
{
    QFont f = font();
    if(b != f.italic()) {
        f.setItalic(b);
        QGraphicsTextItem::setFont(f);
    }
}

/**
 * Reimplemented to retain the italic/bold status on font change.
 *
 * @note This overrides (hides) the base's non virtual method.
 */
void TextItem::setFont(QFont f)
{
    const QFont old = font();

    f.setBold(old.bold());
    f.setItalic(old.italic());

    QGraphicsTextItem::setFont(f);
}

/**
 * Sets the brush to paint background of text on mouse hover.  Default
 * brush = Qt::NoBrush
 *
 * @note The hover events should be enabled for this to work.

 * @see QGraphicsTextItem::setAcceptHoverEvent(bool)
 */
void TextItem::setHoverBrush(const QBrush& brush)
{
    m_hoverBrush = brush;
    update();
}

/**
 * Sets the background brush to \a brush which is used to paint as
 * background of text.
 * Default brush = Qt::NoBrush
 */
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

void TextItem::copyAttributesTo(TextItem *other) const
{
    other->setDefaultTextColor(defaultTextColor());
    other->setHoverBrush(hoverBrush());
    other->setAcceptHoverEvents(acceptHoverEvents());
    other->setAlignment(alignment());
    other->setFont(font());
    other->setBackgroundBrush(backgroundBrush());
}

void TextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    update();
}

void TextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    update();
}

void TextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->ignore();
}

#include "textitem.moc"
