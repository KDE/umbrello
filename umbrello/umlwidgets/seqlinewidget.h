/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef SEQLINEWIDGET_H
#define SEQLINEWIDGET_H

#include <QGraphicsLineItem>

#include <QPen>

class ObjectWidget;
class UMLScene;

/**
 * A sequence lifeline consists of the object widget at the top and
 * a vertical line starting at the bottom edge of the object widget
 * at half its width. The line grows downward when sequence messages
 * are added such that the line always extends far enough to act as
 * the background for all messages.
 * This class represents only the line part of the lifeline.
 *
 * @short Widget class for graphical representation of sequence lines
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class SeqLineWidget : public QGraphicsLineItem
{
public:
    SeqLineWidget(UMLScene *scene, ObjectWidget * pObject);
    virtual ~SeqLineWidget();

    bool onWidget(const QPointF& p);

    bool onDestructionBox(const QPointF& p);

    void cleanup();

    void setupDestructionBox();

    void setStartPoint(int startX, int startY);

    /**
     * Gets the length of the line.
     *
     * @return  Length of the line.
     */
    int getLineLength() const {
        return m_nLengthY;
    }

    /**
     * Returns the @ref ObjectWidget associated with this sequence line.
     *
     * @return  Pointer to the associated ObjectWidget.
     */
    ObjectWidget * getObjectWidget() {
        return m_pObject;
    }

    void setEndOfLine(int yPosition);
    void setLineColorCmd(const QColor &color);

protected:
    void cleanupDestructionBox();

    void moveDestructionBox();

    ObjectWidget* m_pObject;  ///< ObjectWidget associated with this sequence line
    UMLScene*     m_scene;    ///< scene displayed on

    struct DestructionBox {
        QGraphicsLineItem * line1{nullptr};
        QGraphicsLineItem * line2{nullptr};
        void setLineColorCmd(const QColor &color)
        {
            if (!line1)
                return;
            QPen pen = line1->pen();
            pen.setColor(color);
            line1->setPen(pen);
            line2->setPen(pen);
        }

        void setLine1Points(QRect rect) {
            line1->setLine(rect.x(), rect.y(),
                            rect.x() + rect.width(), rect.y() + rect.height());
        }
        void setLine2Points(QRect rect) {
            line2->setLine(rect.x(), rect.y() + rect.height(),
                            rect.x() + rect.width(), rect.y());
        }
    } m_DestructionBox;  ///< the destruction box

    int m_nLengthY;  ///< the length of the line

    static int const m_nMouseDownEpsilonX;   ///< margin used for mouse clicks
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
};

#endif
