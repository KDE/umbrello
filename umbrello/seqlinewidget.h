/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef SEQLINEWIDGET_H
#define SEQLINEWIDGET_H

#include <QGraphicsScene>
#include <QGraphicsLineItem>

class UMLScene;
class ObjectWidget;

/**
 * @short Widget class for graphical representation of sequence lines
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class SeqLineWidget : public QGraphicsLineItem
{
public:
    /**
     * Constructor.
     */
    SeqLineWidget( UMLScene * scene, ObjectWidget * pObject );

    /**
     * Destructor.
     */
    ~SeqLineWidget();

    /**
     * Return whether on seq. line.
     * Takes into account destruction box if shown.
     *
     * @param p The point to investigate.
     * @return  Non-zero if point is on this sequence line.
     */
    qreal onWidget(const QPointF & p);

    /**
     * Return whether on the destruction box.
     *
     * @param p The point to investigate.
     * @return  Non-zero if point is on the destruction box of this sequence line.
     */
    qreal onDestructionBox ( const QPointF & p );


    /**
     * Clean up anything before deletion.
     */
    void cleanup();

    /**
     * Set up destruction box.
     */
    void setupDestructionBox();

    /**
     * Set the start point of the line.
     *
     * @param startX    X coordinate of the start point.
     * @param startY    Y coordinate of the start point.
     */
    void setStartPoint( qreal startX, qreal startY );

    /**
     * Gets the length of the line.
     *
     * @return  Length of the line.
     */
    qreal getLineLength() {
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

    /**
     * Sets the y position of the bottom of the vertical line.
     *
     * @param yPosition The y coordinate for the bottom of the line.
     */
    void setEndOfLine(qreal yPosition);

protected:
    /**
     * Clean up destruction box.
     */
    void cleanupDestructionBox();

    /**
     * Move destruction box.
     */
    void moveDestructionBox();

    /**
     * ObjectWidget associated with this sequence line.
     */
    ObjectWidget * m_pObject;

    /**
     * View displayed on.
     */
    UMLScene * m_pScene;

    /// The destruction box.
    struct DestructionBox {
        QGraphicsLineItem * line1;
        QGraphicsLineItem * line2;
        void setLine1Points(QRectF rect) {
            line1->setLine( rect.x(), rect.y(),
                            rect.x() + rect.width(), rect.y() + rect.height() );
        }
        void setLine2Points(QRectF rect) {
            line2->setLine( rect.x(), rect.y() + rect.height(),
                            rect.x() + rect.width(), rect.y() );
        }
    } m_DestructionBox;

    /**
     * The length of the line.
     */
    qreal m_nLengthY;

    /**
     * Margin used for mouse clicks.
     */
    static qreal const m_nMouseDownEpsilonX;
};

#endif
