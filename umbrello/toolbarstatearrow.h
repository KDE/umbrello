/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef TOOLBARSTATEARROW_H
#define TOOLBARSTATEARROW_H

#include "toolbarstate.h"

#include <QList>

class QGraphicsLineItem;
class UMLScene;

/**
 * Arrow tool for select, move and resize widgets and associations.
 * Arrow tool delegates the event handling in the widgets and associations. When
 * no widget nor association is being used, the arrow tool acts as a selecting
 * tool that selects all the elements in the rectangle created when dragging the
 * mouse.
 *
 * This is the default tool.
 */
class ToolBarStateArrow : public ToolBarState
{
    Q_OBJECT
public:
    explicit ToolBarStateArrow(UMLScene *umlScene);
    virtual ~ToolBarStateArrow();

    virtual void init();
    void cleanup();

protected:

    virtual void mousePressAssociation();
    virtual void mousePressWidget();
    virtual void mousePressEmpty();
    virtual void mouseReleaseAssociation();
    virtual void mouseReleaseWidget();
    virtual void mouseReleaseEmpty();
    virtual void mouseDoubleClickAssociation();
    virtual void mouseDoubleClickWidget();
    virtual void mouseMoveAssociation();
    virtual void mouseMoveWidget();
    virtual void mouseMoveEmpty();

    virtual void setCurrentWidget(UMLWidget* currentWidget);

    virtual void changeTool();

    /**
     * The selection rectangle that contains the four lines of its borders.
     */
    QList<QGraphicsLineItem*> m_selectionRect;

    /**
     * The start position of the selection rectangle.
     */
    QPointF m_startPosition;

};

#endif //TOOLBARSTATEARROW_H
