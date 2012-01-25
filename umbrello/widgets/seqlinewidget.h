/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef SEQLINEWIDGET_H
#define SEQLINEWIDGET_H

#include "umlscene.h"

class ObjectWidget;

/**
 * @short Widget class for graphical representation of sequence lines
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class SeqLineWidget : public UMLSceneLine
{
public:
    SeqLineWidget(UMLView * pView, ObjectWidget * pObject);
    virtual ~SeqLineWidget();

    int onWidget(const QPoint & p);

    int onDestructionBox(const QPoint & p);

    void cleanup();

    void setupDestructionBox();

    void setStartPoint( int startX, int startY );

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
    UMLView*      m_scene;    ///< view displayed on

    struct DestructionBox {
        UMLSceneLine * line1;
        UMLSceneLine * line2;
        void setLine1Points(QRect rect) {
            line1->setPoints( rect.x(), rect.y(),
                              rect.x() + rect.width(), rect.y() + rect.height() );
        }
        void setLine2Points(QRect rect) {
            line2->setPoints( rect.x(), rect.y() + rect.height(),
                              rect.x() + rect.width(), rect.y() );
        }
    } m_DestructionBox;  ///< the destruction box

    int m_nLengthY;  ///< the length of the line

    static int const m_nMouseDownEpsilonX;   ///< margin used for mouse clicks
};

#endif
