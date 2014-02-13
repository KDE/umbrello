/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// onw header
#include "messagewidget.h"

//app includes
#include "classifier.h"
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "listpopupmenu.h"
#include "objectwidget.h"
#include "operation.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "uniqueid.h"

//qt includes
#include <QMoveEvent>
#include <QPainter>
#include <QPolygon>
#include <QResizeEvent>

//kde includes
#include <klocale.h>

DEBUG_REGISTER_DISABLED(MessageWidget)

static const int circleWidth = 10;

/**
 * Constructs a MessageWidget.
 *
 * This method is used for creation, synchronous and synchronous message types.
 *
 * @param scene   The parent to this class.
 * @param a       The role A widget for this message.
 * @param b       The role B widget for this message.
 * @param y       The vertical position to display this message.
 * @param sequenceMessageType Whether synchronous or asynchronous
 * @param id      A unique id used for deleting this object cleanly.
 *                The default (-1) will prompt generation of a new ID.
 */
MessageWidget::MessageWidget(UMLScene * scene, ObjectWidget* a, ObjectWidget* b,
                             int y, Uml::SequenceMessage::Enum sequenceMessageType,
                             Uml::ID::Type id /* = Uml::id_None */)
  : UMLWidget(scene, WidgetBase::wt_Message, id)
{
    init();
    m_pOw[Uml::RoleType::A] = a;
    m_pOw[Uml::RoleType::B] = b;
    m_sequenceMessageType = sequenceMessageType;
    if (m_sequenceMessageType == Uml::SequenceMessage::Creation) {
        y -= m_pOw[Uml::RoleType::B]->height() / 2;
        m_pOw[Uml::RoleType::B]->setY(y);
    }
    updateResizability();
    calculateWidget();
    y = y < getMinY() ? getMinY() : y;
    y = y > getMaxY() ? getMaxY() : y;
    setY(y);

    this->activate();
}

/**
 * Constructs a MessageWidget.
 *
 * @param scene              The parent to this class.
 * @param sequenceMessageType The Uml::SequenceMessage::Enum of this message widget
 * @param id                The ID to assign (-1 will prompt a new ID.)
 */
MessageWidget::MessageWidget(UMLScene * scene, Uml::SequenceMessage::Enum seqMsgType,
                             Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_Message, id)
{
    init();
    m_sequenceMessageType = seqMsgType;
}

/**
 * Constructs a Lost or Found MessageWidget.
 *
 * @param scene  The parent to this class.
 * @param a      The role A widget for this message.
 * @param xclick The horizontal position clicked by the user
 * @param yclick The vertical position clicked by the user
 * @param sequenceMessageType Whether lost or found
 * @param id     The ID to assign (-1 will prompt a new ID.)
 */
MessageWidget::MessageWidget(UMLScene * scene, ObjectWidget* a, int xclick, int yclick,
                             Uml::SequenceMessage::Enum sequenceMessageType,
                             Uml::ID::Type id /*= Uml::id_None*/)
  : UMLWidget(scene, WidgetBase::wt_Message, id)
{
    init();
    m_pOw[Uml::RoleType::A] = a;
    m_pOw[Uml::RoleType::B] = a;

    m_sequenceMessageType = sequenceMessageType;

    xclicked = xclick;
    yclicked = yclick;

    updateResizability();
    calculateWidget();
    yclick = yclick < getMinY() ? getMinY() : yclick;
    yclick = yclick > getMaxY() ? getMaxY() : yclick;
    setY(yclick);
    yclicked = yclick;

    this->activate();
}

/**
 * Initializes key variables of the class.
 */
void MessageWidget::init()
{
    m_ignoreSnapToGrid = true;
    m_ignoreSnapComponentSizeToGrid = true;
    m_pOw[Uml::RoleType::A] = m_pOw[Uml::RoleType::B] = NULL;
    m_pFText = NULL;
}

/**
 * Standard destructor.
 */
MessageWidget::~MessageWidget()
{
}

/**
 * Sets the y-coordinate.
 * Reimplemented from UMLWidget.
 *
 * @param y The y-coordinate to be set.
 */
void MessageWidget::setY(qreal y)
{
    if (y < getMinY()) {
        DEBUG(DBG_SRC) << "got out of bounds y position, check the reason" << this->y() << getMinY();
        return;
    }

    UMLWidget::setY(y);
    if (m_sequenceMessageType == Uml::SequenceMessage::Creation) {
        const qreal objWidgetHalfHeight = m_pOw[Uml::RoleType::B]->height() / 2;
        m_pOw[Uml::RoleType::B]->setY(y - objWidgetHalfHeight);
    }
    moveEvent(0);
}

/**
 * Update the UMLWidget::m_resizable flag according to the
 * charactersitics of this message.
 */
void MessageWidget::updateResizability()
{
    if (m_sequenceMessageType == Uml::SequenceMessage::Synchronous)
        UMLWidget::m_resizable = true;
    else
        UMLWidget::m_resizable = false;
}

/**
 * Overridden from UMLWidget.
 * Returns the cursor to be shown when resizing the widget.
 * The cursor shown is KCursor::sizeVerCursor().
 *
 * @return The cursor to be shown when resizing the widget.
 */
QCursor MessageWidget::resizeCursor() const
{
    return Qt::SizeVerCursor;
}

/**
 * Overridden from UMLWidget.
 * Resizes the height of the message widget and emits the message moved signal.
 * Message widgets can only be resized vertically, so width isn't modified.
 *
 * @param newW   The new width for the widget (isn't used).
 * @param newH   The new height for the widget.
 */
void MessageWidget::resizeWidget(qreal newW, qreal newH)
{
    if (sequenceMessageType() == Uml::SequenceMessage::Creation)
        setSize(width(), newH);
    else {
        qreal x1 = m_pOw[Uml::RoleType::A]->x();
        qreal x2 = getxclicked();
        qreal diffX = 0;
        if (x1 < x2) {
            diffX = x2 + (newW - width());
        }
        else {
            diffX = x2 - (newW - width());
        }
        if (diffX <= 0 )
            diffX = 10;
        setxclicked (diffX);
        setSize(newW, newH);
        calculateWidget();

    }
    emit sigMessageMoved();
}

/**
 * Constrains the vertical position of the message widget so it doesn't go
 * upper than the bottom side of the lower object.
 * The height of the floating text widget in the message is taken in account
 * if there is any and isn't empty.
 *
 * @param diffY The difference between current Y position and new Y position.
 * @return The new Y position, constrained.
 */
qreal MessageWidget::constrainPositionY(qreal diffY)
{
    qreal newY = y() + diffY;

    qreal minY = getMinY();
    if (m_pFText && !m_pFText->displayText().isEmpty()) {
        minY += m_pFText->height();
    }

    if (newY < minY) {
        newY = minY;
    }

    return newY;
}

/**
 * Overridden from UMLWidget.
 * Moves the widget to a new position using the difference between the
 * current position and the new position. X position is ignored, and widget
 * is only moved along Y axis. If message goes upper than the object, it's
 * kept at this position until it should be lowered again (the unconstrained
 * Y position is saved to know when it's the time to lower it again).
 * If the message is a creation message, the object created is also moved to
 * the new vertical position.
 * @see constrainPositionY
 *
 * @param diffX The difference between current X position and new X position
 *                          (isn't used).
 * @param diffY The difference between current Y position and new Y position.
 */
void MessageWidget::moveWidgetBy(qreal diffX, qreal diffY)
{
    Q_UNUSED(diffX);
    qreal newY = constrainPositionY(diffY);
    setY(newY);
}

/**
 * Overridden from UMLWidget.
 * Modifies the value of the diffX and diffY variables used to move the widgets.
 * All the widgets are constrained to be moved only in Y axis (diffX is set to 0).
 * @see constrainPositionY
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 */
void MessageWidget::constrainMovementForAllWidgets(qreal &diffX, qreal &diffY)
{
    diffX = 0;
    diffY = constrainPositionY(diffY) - y();
}

/**
 * Reimplemented from UMLWidget and calls other paint...() methods
 * depending on the message type.
 */
void MessageWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if(!m_pOw[Uml::RoleType::A] || !m_pOw[Uml::RoleType::B]) {
        return;
    }
    setPenFromSettings(painter);
    if (m_sequenceMessageType == Uml::SequenceMessage::Synchronous) {
        paintSynchronous(painter);
    } else if (m_sequenceMessageType == Uml::SequenceMessage::Asynchronous) {
        paintAsynchronous(painter);
    } else if (m_sequenceMessageType == Uml::SequenceMessage::Creation) {
        paintCreation(painter);
    } else if (m_sequenceMessageType == Uml::SequenceMessage::Lost) {
        paintLost(painter);
    } else if (m_sequenceMessageType == Uml::SequenceMessage::Found) {
        paintFound(painter);
    } else {
        uWarning() << "Unknown message type";
    }
}

/**
 * Draw a solid (triangular) arrowhead pointing in the given direction.
 * The direction can be either Qt::LeftArrow or Qt::RightArrow.
 */
void MessageWidget::paintSolidArrowhead(QPainter *p, int x, int y, Qt::ArrowType direction)
{
    int arrowheadExtentX = 4;
    if (direction == Qt::RightArrow) {
        arrowheadExtentX = -arrowheadExtentX;
    }
    QPolygon points;
    points.putPoints(0, 3, x, y, x + arrowheadExtentX, y - 3, x + arrowheadExtentX, y + 3);
    p->setBrush(QBrush(p->pen().color()));
    p->drawPolygon(points);
}

/**
 * Draw an arrow pointing in the given direction.
 * The arrow head is not solid, i.e. it is made up of two lines
 * like so:  --->
 * The direction can be either Qt::LeftArrow or Qt::RightArrow.
 */
void MessageWidget::paintArrow(QPainter *p, int x, int y, int w,
                              Qt::ArrowType direction, bool useDottedLine /* = false */)
{
    if (w > 3) {
        int arrowheadStartX = x;
        int arrowheadExtentX = 4;
        if (direction == Qt::RightArrow) {
            arrowheadStartX += w;
            arrowheadExtentX = -arrowheadExtentX;
        }
        // draw upper half of arrowhead
        p->drawLine(arrowheadStartX, y, arrowheadStartX + arrowheadExtentX, y - 3);
        // draw lower half of arrowhead
        p->drawLine(arrowheadStartX, y, arrowheadStartX + arrowheadExtentX, y + 3);
    }
    // draw arrow line
    if (useDottedLine) {
        QPen pen = p->pen();
        pen.setStyle(Qt::DotLine);
        p->setPen(pen);
    }
    p->drawLine(x, y, x + w, y);
}

/**
 * Draws the calling arrow with filled in arrowhead, the
 * timeline box and the returning arrow with a dashed line and
 * stick arrowhead.
 */
void MessageWidget::paintSynchronous(QPainter *painter)
{
    int x1 = m_pOw[Uml::RoleType::A]->x();
    int x2 = m_pOw[Uml::RoleType::B]->x();
    int w = width() - 1;
    int h = height();
    int offsetX = 0;
    int offsetY = 0;

    bool messageOverlaps = m_pOw[Uml::RoleType::A]->messageOverlap(y(), this);
    const int boxWidth = 17;
    const int wr = w < boxWidth ? w : boxWidth;
    const int arrowWidth = 4;

    if(isSelf()) {
        painter->fillRect(offsetX, offsetY, wr, h,  QBrush(Qt::white));              //box
        painter->drawRect(offsetX, offsetY, wr, h);                                    //box
        offsetX += wr;
        w -= wr;
        offsetY += 3;
        const int lowerLineY = offsetY + h - 6;
        // draw upper line segment (leaving the life line)
        painter->drawLine(offsetX, offsetY, offsetX + w, offsetY);
        // draw line segment parallel to (and at the right of) the life line
        painter->drawLine(offsetX + w, offsetY, offsetX + w, lowerLineY);
        // draw lower line segment (back to the life line)
        paintArrow(painter, offsetX, lowerLineY, w, Qt::LeftArrow);
        offsetX -= wr;
        offsetY -= 3;
    } else if(x1 < x2) {
        if (messageOverlaps)  {
            offsetX += 8;
            w -= 8;
        }
        QPen pen = painter->pen();
        int startX = offsetX + w - wr + 1;
        painter->fillRect(startX, offsetY, wr, h,  QBrush(Qt::white));         //box
        painter->drawRect(startX, offsetY, wr, h);                             //box
        painter->drawLine(offsetX, offsetY + arrowWidth, startX, offsetY + arrowWidth);          //arrow line
        if (w > boxWidth + arrowWidth)
            paintSolidArrowhead(painter, startX - 1, offsetY + arrowWidth, Qt::RightArrow);
        paintArrow(painter, offsetX, offsetY + h - arrowWidth + 1, w - wr + 1, Qt::LeftArrow, true); // return arrow
        if (messageOverlaps)  {
            offsetX -= 8; //reset for drawSelected()
        }
    } else      {
        if (messageOverlaps)  {
            w -=8;
        }
        QPen pen = painter->pen();
        painter->fillRect(offsetX, offsetY, wr, h,  QBrush(Qt::white));              //box
        painter->drawRect(offsetX, offsetY, wr, h);                                    //box
        painter->drawLine(offsetX + wr + 1, offsetY + arrowWidth, offsetX + w, offsetY + arrowWidth);    //arrow line
        if (w > boxWidth + arrowWidth)
            paintSolidArrowhead(painter, offsetX + wr, offsetY + arrowWidth, Qt::LeftArrow);
        paintArrow(painter, offsetX + wr + 1, offsetY + h - arrowWidth + 1, w - wr - 1, Qt::RightArrow, true); // return arrow
    }

    UMLWidget::paint(painter, 0);
}

/**
 * Draws a solid arrow line and a stick arrow head.
 */
void MessageWidget::paintAsynchronous(QPainter *painter)
{
    int x1 = m_pOw[Uml::RoleType::A]->x();
    int x2 = m_pOw[Uml::RoleType::B]->x();
    int w = width() - 1;
    int h = height() - 1;
    int offsetX = 0;
    int offsetY = 0;
    bool messageOverlapsA = m_pOw[Uml::RoleType::A]->messageOverlap(y(), this);
    //bool messageOverlapsB = m_pOw[Uml::RoleType::B]->messageOverlap(y(), this);

    if(isSelf()) {
        if (messageOverlapsA)  {
            offsetX += 7;
            w -= 7;
        }
        const int lowerLineY = offsetY + h - 3;
        // draw upper line segment (leaving the life line)
        painter->drawLine(offsetX, offsetY, offsetX + w, offsetY);
        // draw line segment parallel to (and at the right of) the life line
        painter->drawLine(offsetX + w, offsetY, offsetX + w, lowerLineY);
        // draw lower line segment (back to the life line)
        paintArrow(painter, offsetX, lowerLineY, w, Qt::LeftArrow);
        if (messageOverlapsA)  {
            offsetX -= 7; //reset for drawSelected()
        }
    } else if(x1 < x2) {
        if (messageOverlapsA) {
            offsetX += 7;
            w -= 7;
        }
        paintArrow(painter, offsetX, offsetY + 4, w, Qt::RightArrow);
        if (messageOverlapsA) {
            offsetX -= 7;
        }
    } else      {
        if (messageOverlapsA) {
            w -= 7;
        }
        paintArrow(painter, offsetX, offsetY + 4, w, Qt::LeftArrow);
    }

    UMLWidget::paint(painter, 0);
}

/**
 * Draws a solid arrow line and a stick arrow head to the
 * edge of the target object widget instead of to the
 * sequence line.
 */
void MessageWidget::paintCreation(QPainter *painter)
{
    int x1 = m_pOw[Uml::RoleType::A]->x();
    int x2 = m_pOw[Uml::RoleType::B]->x();
    int w = width();
    //int h = height() - 1;
    int offsetX = 0;
    int offsetY = 0;
    bool messageOverlapsA = m_pOw[Uml::RoleType::A]->messageOverlap(y(), this);
    //bool messageOverlapsB = m_pOw[Uml::RoleType::B]->messageOverlap(y(), this);

    const int lineY = offsetY + 4;
    if (x1 < x2) {
        if (messageOverlapsA) {
            offsetX += 7;
            w -= 7;
        }
        paintArrow(painter, offsetX, lineY, w, Qt::RightArrow);
        if (messageOverlapsA) {
            offsetX -= 7;
        }
    } else      {
        if (messageOverlapsA) {
            w -= 7;
        }
        paintArrow(painter, offsetX, lineY, w, Qt::LeftArrow);
    }

    UMLWidget::paint(painter, 0);
}


/**
 * Draws a solid arrow line and a stick arrow head
 * and a circle
 */
void MessageWidget::paintLost(QPainter *painter)
{
    int x1 = m_pOw[Uml::RoleType::A]->centerX();
    int x2 = xclicked;
    int w = width();
    int h = height();
    int offsetX = 0;
    int offsetY = 0;
    bool messageOverlapsA = m_pOw[Uml::RoleType::A]->messageOverlap(y(), this);
    //bool messageOverlapsB = m_pOw[Uml::RoleType::B]->messageOverlap(y(), this);

    if(x1 < x2) {
        if (messageOverlapsA)  {
            offsetX += 7;
            w -= 7;
        }

        setPenFromSettings(painter);
        painter->setBrush(WidgetBase::lineColor());
        painter->drawEllipse(offsetX + w - h, offsetY, h, h);
        paintArrow(painter, offsetX, offsetY + h/2, w - h, Qt::RightArrow);

        if (messageOverlapsA)  {
            offsetX -= 7;
        }
    } else      {
        setPenFromSettings(painter);
        painter->setBrush(WidgetBase::lineColor());
        painter->drawEllipse(offsetX, offsetY, h, h);
        paintArrow(painter, offsetX + h, offsetY + h/2, w - h, Qt::LeftArrow);
    }

    UMLWidget::paint(painter, 0);
}

/**
 * Draws a circle and a solid arrow line and a stick arrow head.
 */
void MessageWidget::paintFound(QPainter *painter)
{
    int x1 = m_pOw[Uml::RoleType::A]->centerX();
    int x2 = xclicked;
    int w = width();
    int h = height();
    int offsetX = 0;
    int offsetY = 0;
    bool messageOverlapsA = m_pOw[Uml::RoleType::A]->messageOverlap(y(), this);
    //bool messageOverlapsB = m_pOw[Uml::RoleType::B]->messageOverlap(y(), this);

    if(x1 < x2) {
        if (messageOverlapsA)  {
            offsetX += 7;
            w -= 7;
        }
        setPenFromSettings(painter);
        painter->setBrush(WidgetBase::lineColor());
        painter->drawEllipse(offsetX + w - h, offsetY, h, h);
        paintArrow(painter, offsetX, offsetY + h/2, w, Qt::LeftArrow);
        if (messageOverlapsA)  {
            offsetX -= 7;
        }
    } else {
        if (messageOverlapsA)  {
            w -= 7;
        }
        setPenFromSettings(painter);
        painter->setBrush(WidgetBase::lineColor());
        painter->drawEllipse(offsetX, offsetY, h, h);
        paintArrow(painter, offsetX, offsetY + h/2, w, Qt::RightArrow);
    }

    UMLWidget::paint(painter, 0);
}

/**
 * Overrides operation from UMLWidget.
 *
 * @param p Point to be checked.
 *
 * @return Non-zero if the point is on a part of the MessageWidget.
 *         NB In case of a synchronous message, the empty space
 *         between call line and return line does not count, i.e. if
 *         the point is located in that space the function returns 0.
 */
qreal MessageWidget::onWidget(const QPointF& p)
{
    if (m_sequenceMessageType != Uml::SequenceMessage::Synchronous) {
        return UMLWidget::onWidget(p);
    }
    // Synchronous message:
    // Consists of top arrow (call) and bottom arrow (return.)
    if (p.x() < x() || p.x() > x() + width())
        return 0;
    const int tolerance = 5;  // pixels
    const int pY = p.y();
    const int topArrowY = y() + 3;
    const int bottomArrowY = y() + height() - 3;
    if (pY < topArrowY - tolerance || pY > bottomArrowY + tolerance)
        return 0;
    if (height() <= 2 * tolerance)
        return 1;
    if (pY > topArrowY + tolerance && pY < bottomArrowY - tolerance)
        return 0;
    return 1;
}

/**
 * Sets the text position relative to the sequence message.
 */
void MessageWidget::setTextPosition()
{
    if (m_pFText == NULL) {
        DEBUG(DBG_SRC) << "m_pFText is NULL";
        return;
    }
    if (m_pFText->displayText().isEmpty()) {
        return;
    }
    m_pFText->updateGeometry();
    int ftX = constrainX(m_pFText->x(), m_pFText->width(), m_pFText->textRole());
    int ftY = y() - m_pFText->height();
    m_pFText->setX(ftX);
    m_pFText->setY(ftY);
}

/**
 * Returns the textX arg with constraints applied.
 * Auxiliary to setTextPosition() and constrainTextPos().
 */
int MessageWidget::constrainX(int textX, int textWidth, Uml::TextRole::Enum tr)
{
    int result = textX;
    const int minTextX = x() + 5;
    if (textX < minTextX || tr == Uml::TextRole::Seq_Message_Self) {
        result = minTextX;
    } else {
        ObjectWidget *objectAtRight = NULL;
        if (m_pOw[Uml::RoleType::B]->x() > m_pOw[Uml::RoleType::A]->x())
            objectAtRight = m_pOw[Uml::RoleType::B];
        else
            objectAtRight = m_pOw[Uml::RoleType::A];
        const int objRight_seqLineX = objectAtRight->centerX();
        const int maxTextX = objRight_seqLineX - textWidth - 5;
        if (maxTextX <= minTextX)
            result = minTextX;
        else if (textX > maxTextX)
            result = maxTextX;
    }
    return result;
}

/**
 * Constrains the FloatingTextWidget X and Y values supplied.
 * Overrides operation from LinkWidget.
 *
 * @param textX        candidate X value (may be modified by the constraint)
 * @param textY        candidate Y value (may be modified by the constraint)
 * @param textWidth    width of the text
 * @param textHeight   height of the text
 * @param tr           Uml::TextRole::Enum of the text
 */
void MessageWidget::constrainTextPos(qreal &textX, qreal &textY, qreal textWidth, qreal textHeight,
                                     Uml::TextRole::Enum tr)
{
    textX = constrainX(textX, textWidth, tr);
    // Constrain Y.
    const qreal minTextY = getMinY();
    const qreal maxTextY = getMaxY() - textHeight - 5;
    if (textY < minTextY)
        textY = minTextY;
    else if (textY > maxTextY)
        textY = maxTextY;
//     setY(textY + textHeight);   // NB: side effect
}

/**
 * Shortcut for calling m_pFText->setLink() followed by
 * this->setTextPosition().
 */
void MessageWidget::setLinkAndTextPos()
{
    if (m_pFText) {
        m_pFText->setLink(this);
        setTextPosition();
    }
}

void MessageWidget::moveEvent(QGraphicsSceneMouseEvent* /*m*/)
{
    //DEBUG(DBG_SRC) << "m_pFText is " << m_pFText;
    if (!m_pFText) {
        return;
    }
    //TODO why this condition?
/*    if (m_scene->selectedCount() > 2) {
        return;
    }*/

    setTextPosition();

    emit sigMessageMoved();
}

void MessageWidget::resizeEvent(QResizeEvent* /*re*/)
{
}

/**
 * Calculate the geometry of the widget.
 */
void MessageWidget::calculateWidget()
{
    setMessageText(m_pFText);
    calculateDimensions();
    setVisible(true);
}

void MessageWidget::slotWidgetMoved(Uml::ID::Type id)
{
    const Uml::ID::Type idA = m_pOw[Uml::RoleType::A]->localID();
    const Uml::ID::Type idB = m_pOw[Uml::RoleType::B]->localID();
    if (idA != id && idB != id) {
        DEBUG(DBG_SRC) << "id=" << Uml::ID::toString(id) << ": ignoring for idA=" << Uml::ID::toString(idA)
            << ", idB=" << Uml::ID::toString(idB);
        return;
    }
    qreal y = this->y();
    if (y < getMinY())
        y = getMinY();
    if (y > getMaxY())
        y = getMaxY();
    setY(y);
    calculateWidget();
    if(!m_pFText)
        return;
    if (m_scene->selectedCount(true) > 1)
        return;
    setTextPosition();
}

/**
 * Check to see if the given ObjectWidget is involved in the message.
 *
 * @param w The ObjectWidget to check for.
 * @return  true - if is contained, false - not contained.
 */
bool MessageWidget::hasObjectWidget(ObjectWidget * w)
{
    if(m_pOw[Uml::RoleType::A] == w || m_pOw[Uml::RoleType::B] == w)
        return true;
    else
        return false;
}

/**
 * This method determines whether the message is for "Self" for
 * an ObjectWidget.
 *
 * @retval True If both ObjectWidgets for this widget exists and
 *              are same.
 */
bool MessageWidget::isSelf() const
{
    return (m_pOw[Uml::RoleType::A] && m_pOw[Uml::RoleType::B] &&
            m_pOw[Uml::RoleType::A] == m_pOw[Uml::RoleType::B]);
}

void MessageWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    if (sel == ListPopupMenu::mt_Delete) {
        // This will clean up this widget and the text widget:
        m_scene->removeWidget(this);
    } else {

        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Activates a MessageWidget.  Connects its m_pOw[] pointers
 * to UMLObjects and also send signals about its FloatingTextWidget.
 */
bool MessageWidget::activate(IDChangeLog * /*Log = 0*/)
{
    m_scene->resetPastePoint();
    // UMLWidget::activate(Log);   CHECK: I don't think we need this ?
    if (m_pOw[Uml::RoleType::A] == NULL) {
        UMLWidget *pWA = m_scene->findWidget(m_widgetAId);
        if (pWA == NULL) {
            DEBUG(DBG_SRC) << "role A object " << Uml::ID::toString(m_widgetAId) << " not found";
            return false;
        }
        m_pOw[Uml::RoleType::A] = dynamic_cast<ObjectWidget*>(pWA);
        if (m_pOw[Uml::RoleType::A] == NULL) {
            DEBUG(DBG_SRC) << "role A widget " << Uml::ID::toString(m_widgetAId)
                << " is not an ObjectWidget";
            return false;
        }
    }
    if (m_pOw[Uml::RoleType::B] == NULL) {
        UMLWidget *pWB = m_scene->findWidget(m_widgetBId);
        if (pWB == NULL) {
            DEBUG(DBG_SRC) << "role B object " << Uml::ID::toString(m_widgetBId) << " not found";
            return false;
        }
        m_pOw[Uml::RoleType::B] = dynamic_cast<ObjectWidget*>(pWB);
        if (m_pOw[Uml::RoleType::B] == NULL) {
            DEBUG(DBG_SRC) << "role B widget " << Uml::ID::toString(m_widgetBId)
                << " is not an ObjectWidget";
            return false;
        }
    }
    updateResizability();

    UMLClassifier *c = dynamic_cast<UMLClassifier*>(m_pOw[Uml::RoleType::B]->umlObject());
    UMLOperation *op = NULL;
    if (c && !m_CustomOp.isEmpty()) {
        Uml::ID::Type opId = Uml::ID::fromString(m_CustomOp);
        op = dynamic_cast<UMLOperation*>(c->findChildObjectById(opId, true));
        if (op) {
            // If the UMLOperation is set, m_CustomOp isn't used anyway.
            // Just setting it empty for the sake of sanity.
            m_CustomOp.clear();
        }
    }

    if(!m_pFText) {
        Uml::TextRole::Enum tr = Uml::TextRole::Seq_Message;
        if (isSelf())
            tr = Uml::TextRole::Seq_Message_Self;
        m_pFText = new FloatingTextWidget(m_scene, tr, "");
        m_pFText->setFont(UMLWidget::font());
    }
    if (op)
        setOperation(op);  // This requires a valid m_pFText.
    setLinkAndTextPos();
    m_pFText->setText("");
    m_pFText->setActivated();
    QString messageText = m_pFText->text();
    m_pFText->setVisible(messageText.length() > 1);

    connect(m_pOw[Uml::RoleType::A], SIGNAL(sigWidgetMoved(Uml::ID::Type)), this, SLOT(slotWidgetMoved(Uml::ID::Type)));
    connect(m_pOw[Uml::RoleType::B], SIGNAL(sigWidgetMoved(Uml::ID::Type)), this, SLOT(slotWidgetMoved(Uml::ID::Type)));

    connect(this, SIGNAL(sigMessageMoved()), m_pOw[Uml::RoleType::A], SLOT(slotMessageMoved()));
    connect(this, SIGNAL(sigMessageMoved()), m_pOw[Uml::RoleType::B], SLOT(slotMessageMoved()));
    m_pOw[Uml::RoleType::A]->messageAdded(this);
    if (!isSelf())
        m_pOw[Uml::RoleType::B]->messageAdded(this);
    calculateDimensions();

    emit sigMessageMoved();
    return true;
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param ft   The text widget which to update.
 */
void MessageWidget::setMessageText(FloatingTextWidget *ft)
{
    if (ft == NULL)
        return;
    QString displayText = m_SequenceNumber + ": " + operationText(m_scene);
    ft->setText(displayText);
    setTextPosition();
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param ft        The text widget which to update.
 * @param newText   The new text to set.
 */
void MessageWidget::setText(FloatingTextWidget *ft, const QString &newText)
{
    ft->setText(newText);
    UMLApp::app()->document()->setModified(true);
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param seqNum    The new sequence number string to set.
 * @param op        The new operation string to set.
 */
void MessageWidget::setSeqNumAndOp(const QString &seqNum, const QString &op)
{
    setSequenceNumber(seqNum);
    m_CustomOp = op;   ///FIXME m_pOperation
}

/**
 * Write property of QString m_SequenceNumber.
 */
void MessageWidget::setSequenceNumber(const QString &sequenceNumber)
{
    m_SequenceNumber = sequenceNumber;
}

/**
 * Read property of QString m_SequenceNumber.
 */
QString MessageWidget::sequenceNumber() const
{
    return m_SequenceNumber;
}

/**
 * Implements operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
void MessageWidget::lwSetFont (QFont font)
{
    UMLWidget::setFont(font);
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 * @todo Move to LinkWidget.
 */
UMLClassifier *MessageWidget::operationOwner()
{
    UMLObject *pObject = m_pOw[Uml::RoleType::B]->umlObject();
    if (pObject == NULL)
        return NULL;
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(pObject);
    return c;
}

/**
 * Implements operation from LinkWidget.
 * Motivated by FloatingTextWidget.
 */
UMLOperation *MessageWidget::operation()
{
    return static_cast<UMLOperation*>(m_umlObject);
}

/**
 * Implements operation from LinkWidget.
 * Motivated by FloatingTextWidget.
 */
void MessageWidget::setOperation(UMLOperation *op)
{
    if (m_umlObject && m_pFText)
        disconnect(m_umlObject, SIGNAL(modified()), m_pFText, SLOT(setMessageText()));
    m_umlObject = op;
    if (m_umlObject && m_pFText) {
        connect(m_umlObject, SIGNAL(modified()), m_pFText, SLOT(setMessageText()));
        m_pFText->setMessageText();
    }
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
QString MessageWidget::customOpText()
{
    return m_CustomOp;
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
void MessageWidget::setCustomOpText(const QString &opText)
{
    m_CustomOp = opText;
    m_pFText->setMessageText();
}

UMLClassifier * MessageWidget::seqNumAndOp(QString& seqNum, QString& op)
{
    seqNum = m_SequenceNumber;
    UMLOperation *pOperation = operation();
    if (pOperation != NULL) {
        op = pOperation->toString(Uml::SignatureType::SigNoVis);
    } else {
        op = m_CustomOp;
    }
    UMLObject *o = m_pOw[Uml::RoleType::B]->umlObject();
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(o);
    return c;
}

/**
 * Calculates the size of the widget by calling
 * calculateDimensionsSynchronous(),
 * calculateDimensionsAsynchronous(), or
 * calculateDimensionsCreation()
 */
void MessageWidget::calculateDimensions()
{
    if (m_sequenceMessageType == Uml::SequenceMessage::Synchronous) {
        calculateDimensionsSynchronous();
    } else if (m_sequenceMessageType == Uml::SequenceMessage::Asynchronous) {
        calculateDimensionsAsynchronous();
    } else if (m_sequenceMessageType == Uml::SequenceMessage::Creation) {
        calculateDimensionsCreation();
    } else if (m_sequenceMessageType == Uml::SequenceMessage::Lost) {
        calculateDimensionsLost();
    } else if (m_sequenceMessageType == Uml::SequenceMessage::Found) {
        calculateDimensionsFound();
    } else {
        uWarning() << "Unknown message type";
    }
    if (! UMLApp::app()->document()->loading()) {
        adjustAssocs(x(), y());  // adjust assoc lines
    }
}

/**
 * Calculates and sets the size of the widget for a synchronous message.
 */
void MessageWidget::calculateDimensionsSynchronous()
{
    int x = 0;

    int x1 = m_pOw[Uml::RoleType::A]->centerX();
    int x2 = m_pOw[Uml::RoleType::B]->centerX();

    int widgetWidth = 0;
    int widgetHeight = 0;
    if(isSelf()) {
        widgetWidth = 50;
        x = x1 - 2;
    } else if(x1 < x2) {
        x = x1;
        widgetWidth = x2 - x1 + 8;
    } else {
        x = x2 - 8;
        widgetWidth = x1 - x2 + 8;
    }

    if (height() < 20) {
        widgetHeight = 20;
    } else {
        widgetHeight = height();
    }

    setX(x);
    setSize(widgetWidth, widgetHeight);
}

/**
 * Calculates and sets the size of the widget for an asynchronous message.
 */
void MessageWidget::calculateDimensionsAsynchronous()
{
    int x = 0;

    int x1 = m_pOw[Uml::RoleType::A]->centerX();
    int x2 = m_pOw[Uml::RoleType::B]->centerX();

    int widgetWidth = 0;
    int widgetHeight = 8;
    if(isSelf()) {
        widgetWidth = 50;
        x = x1;
        if(height() < 20) {
            widgetHeight = 20;
        } else {
            widgetHeight = height();
        }
    } else if(x1 < x2) {
        x = x1;
        widgetWidth = x2 - x1;
    } else {
        x = x2;
        widgetWidth = x1 - x2;
    }
    x += 1;
    widgetWidth -= 2;
    setX(x);
    setSize(widgetWidth, widgetHeight);
}

/**
 * Calculates and sets the size of the widget for a creation message.
 */
void MessageWidget::calculateDimensionsCreation()
{
    int x = 0;

    int x1 = m_pOw[Uml::RoleType::A]->centerX();
    int x2 = m_pOw[Uml::RoleType::B]->x();
    int w2 = m_pOw[Uml::RoleType::B]->width();

    if (x1 > x2)
        x2 += w2;

    int widgetWidth = 0;
    int widgetHeight = 8;
    if (x1 < x2) {
        x = x1;
        widgetWidth = x2 - x1;
    } else {
        x = x2;
        widgetWidth = x1 - x2;
    }
    x += 1;
    widgetWidth -= 2;
    setX(x);
    setY(m_pOw[Uml::RoleType::B]->y() + m_pOw[Uml::RoleType::B]->height() / 2);
    setSize(widgetWidth, widgetHeight);
}

/**
 * Calculates and sets the size of the widget for a lost message.
 */
void MessageWidget::calculateDimensionsLost()
{
    int x = 0;

    int x1 = m_pOw[Uml::RoleType::A]->centerX();
    int x2 = xclicked;

    int widgetWidth = 0;
    int widgetHeight = 10;
    if(x1 < x2) {
        x = x1;
        widgetWidth = x2 - x1 + circleWidth/2;
    } else {
        x = x2 - circleWidth/2;
        widgetWidth = x1 - x2 + circleWidth/2;
    }
    setX(x);
    setSize(widgetWidth, widgetHeight);
}

/**
 * Calculates and sets the size of the widget for a found message.
 */
void MessageWidget::calculateDimensionsFound()
{
    int x = 0;

    int x1 = m_pOw[Uml::RoleType::A]->centerX();
    int x2 = xclicked;

    int widgetWidth = 0;
    int widgetHeight = 10;
    if(x1 < x2) {
        x = x1;
        widgetWidth = x2 - x1 + circleWidth/2;
    } else {
        x = x2 - circleWidth/2;
        widgetWidth = x1 - x2 + circleWidth/2;
    }

    setX(x);
    setSize(widgetWidth, widgetHeight);
}

/**
 * Used to cleanup any other widget it may need to delete.
 */
void MessageWidget::cleanup()
{
    if (m_pOw[Uml::RoleType::A]) {
        disconnect(this, SIGNAL(sigMessageMoved()), m_pOw[Uml::RoleType::A], SLOT(slotMessageMoved()));
        m_pOw[Uml::RoleType::A]->messageRemoved(this);
    }
    if (m_pOw[Uml::RoleType::B]) {
        disconnect(this, SIGNAL(sigMessageMoved()), m_pOw[Uml::RoleType::B], SLOT(slotMessageMoved()));
        m_pOw[Uml::RoleType::B]->messageRemoved(this);
    }

    UMLWidget::cleanup();
    if (m_pFText) {
        m_scene->removeWidget(m_pFText);
        m_pFText = NULL;
    }
}

/**
 * Sets the state of whether the widget is selected.
 *
 * @param _select   True if the widget is selected.
 */
void MessageWidget::setSelected(bool _select)
{
    UMLWidget::setSelected(_select);
    if(!m_pFText || m_pFText->displayText().isEmpty())
        return;
    if(m_selected && m_pFText->isSelected())
        return;
    if(!m_selected && !m_pFText->isSelected())
        return;

    m_scene->setSelected(m_pFText, 0);
    m_pFText->setSelected(m_selected);
}

/**
 * Returns the minimum height this widget should be set at on
 * a sequence diagrams.  Takes into account the widget positions
 * it is related to.
 */
int MessageWidget::getMinY()
{
    if (!m_pOw[Uml::RoleType::A] || !m_pOw[Uml::RoleType::B]) {
        return 0;
    }
    if (m_sequenceMessageType == Uml::SequenceMessage::Creation) {
        return m_pOw[Uml::RoleType::A]->y() + m_pOw[Uml::RoleType::A]->height();
    }
    int heightA = m_pOw[Uml::RoleType::A]->y() + m_pOw[Uml::RoleType::A]->height();
    int heightB = m_pOw[Uml::RoleType::B]->y() + m_pOw[Uml::RoleType::B]->height();
    int height = heightA;
    if(heightA < heightB) {
        height = heightB;
    }
    return height;
}

/**
 * Returns the maximum height this widget should be set at on
 * a sequence diagrams.  Takes into account the widget positions
 * it is related to.
 */
int MessageWidget::getMaxY()
{
    if(!m_pOw[Uml::RoleType::A] || !m_pOw[Uml::RoleType::B]) {
        return 0;
    }
    int heightA = (int)((ObjectWidget*)m_pOw[Uml::RoleType::A])->getEndLineY();
    int heightB = (int)((ObjectWidget*)m_pOw[Uml::RoleType::B])->getEndLineY();
    int height = heightA;
    if(heightA > heightB) {
        height = heightB;
    }
    return (height - this->height());
}

/**
 * Sets the related widget on the given side.
 *
 * @param ow     The ObjectWidget we are related to.
 * @param role   The Uml::RoleType::Enum to be set for the ObjectWidget
 */
void MessageWidget::setObjectWidget(ObjectWidget * ow, Uml::RoleType::Enum role)
{
    m_pOw[role] = ow;
    updateResizability();
}

/**
 * Returns the related widget on the given side.
 *
 * @return  The ObjectWidget we are related to.
 */
ObjectWidget* MessageWidget::objectWidget(Uml::RoleType::Enum role)
{
    return m_pOw[role];
}

/**
 * Set the xclicked
 */
void MessageWidget::setxclicked(int xclick)
{
    xclicked = xclick;
}

/**
 * Set the yclicked
 */
void MessageWidget::setyclicked(int yclick)
{
    yclicked = yclick;
}

/**
 * Event handler for mouse double click events.
 * @param me QGraphicsSceneMouseEvent which triggered the double click event
 */
void MessageWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    DEBUG(DBG_SRC) << "NOT IMPLEMENTED YET!";
    if (m_pFText != NULL) {
        DEBUG(DBG_SRC) << "NOT IMPLEMENTED YET - on floating text widget!";
//        QAction* action = m_pMenu->getAction(ListPopupMenu::mt_Select_Operation);
//        m_pFText->slotMenuSelection(action);
    }
}

/**
 * Saves to the "messagewidget" XMI element.
 */
void MessageWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement messageElement = qDoc.createElement("messagewidget");
    UMLWidget::saveToXMI(qDoc, messageElement);
    messageElement.setAttribute("widgetaid", Uml::ID::toString(m_pOw[Uml::RoleType::A]->localID()));
    messageElement.setAttribute("widgetbid", Uml::ID::toString(m_pOw[Uml::RoleType::B]->localID()));
    UMLOperation *pOperation = operation();
    if (pOperation)
        messageElement.setAttribute("operation", Uml::ID::toString(pOperation->id()));
    else
        messageElement.setAttribute("operation", m_CustomOp);
    messageElement.setAttribute("seqnum", m_SequenceNumber);
    messageElement.setAttribute("sequencemessagetype", m_sequenceMessageType);
    if (m_sequenceMessageType == Uml::SequenceMessage::Lost || m_sequenceMessageType == Uml::SequenceMessage::Found) {
        messageElement.setAttribute("xclicked", xclicked);
        messageElement.setAttribute("yclicked", yclicked);
    }

    // save the corresponding message text
    if (m_pFText && !m_pFText->text().isEmpty()) {
        messageElement.setAttribute("textid", Uml::ID::toString(m_pFText->id()));
        m_pFText->saveToXMI(qDoc, messageElement);
    }

    qElement.appendChild(messageElement);
}

/**
 * Loads from the "messagewidget" XMI element.
 */
bool MessageWidget::loadFromXMI(QDomElement& qElement)
{
    if (!UMLWidget::loadFromXMI(qElement)) {
        return false;
    }
    QString textid = qElement.attribute("textid", "-1");
    QString widgetaid = qElement.attribute("widgetaid", "-1");
    QString widgetbid = qElement.attribute("widgetbid", "-1");
    m_CustomOp = qElement.attribute("operation", "");
    m_SequenceNumber = qElement.attribute("seqnum", "");
    QString sequenceMessageType = qElement.attribute("sequencemessagetype", "1001");
    m_sequenceMessageType = Uml::SequenceMessage::fromInt(sequenceMessageType.toInt());
    if (m_sequenceMessageType == Uml::SequenceMessage::Lost || m_sequenceMessageType == Uml::SequenceMessage::Found) {
        xclicked = qElement.attribute("xclicked", "-1").toFloat();
        yclicked = qElement.attribute("yclicked", "-1").toFloat();
    }

    m_widgetAId = Uml::ID::fromString(widgetaid);
    m_widgetBId = Uml::ID::fromString(widgetbid);
    m_textId = Uml::ID::fromString(textid);

    Uml::TextRole::Enum tr = Uml::TextRole::Seq_Message;
    if (m_widgetAId == m_widgetBId)
        tr = Uml::TextRole::Seq_Message_Self;

    //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == "floatingtext") {
            m_pFText = new FloatingTextWidget(m_scene, tr, operationText(m_scene), m_textId);
            if(! m_pFText->loadFromXMI(element)) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete m_pFText;
                m_pFText = NULL;
            }
        } else {
            uError() << "unknown tag " << tag;
        }
    }
    return true;
}

/**
 * :TODO: REMOVE IT!
 */
//ListPopupMenu* MessageWidget::setupPopupMenu(ListPopupMenu *menu)
//{
//    UMLWidget::setupPopupMenu(menu); // will setup the menu in m_pMenu
//    ListPopupMenu* floatingtextSubMenu = m_pFText->setupPopupMenu();
//    floatingtextSubMenu->setTitle(i18n("Operation"));

//    m_pMenu->addMenu(floatingtextSubMenu);

//    return m_pMenu;
//}

#include "messagewidget.moc"
