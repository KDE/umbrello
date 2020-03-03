/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef SEQLINEWIDGET_H
#define SEQLINEWIDGET_H

#include <QGraphicsLineItem>

class ObjectWidget;
class UMLScene;

/**
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
    int getLineLength() {
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

protected:
    void cleanupDestructionBox();

    void moveDestructionBox();

    ObjectWidget* m_pObject;  ///< ObjectWidget associated with this sequence line
    UMLScene*     m_scene;    ///< scene displayed on

    struct DestructionBox {
        QGraphicsLineItem * line1;
        QGraphicsLineItem * line2;
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
};

#endif
