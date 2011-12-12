/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef TOOLBARSTATEARROW_H
#define TOOLBARSTATEARROW_H

#include "toolbarstate.h"
#include "umlscene.h"

#include <QtCore/QList>

/**
 * Arrow tool for select, move and resize widgets and associations.
 * Arrow tool delegates the event handling in the widgets and associations. When
 * no widget nor association is being used,the arrow tool acts as a selecting
 * tool that selects all the elements in the rectangle created when dragging the
 * mouse.
 *
 * This is the default tool.
 */
class ToolBarStateArrow : public ToolBarState
{
    Q_OBJECT
public:

    ToolBarStateArrow(UMLScene *umlScene);
    virtual ~ToolBarStateArrow();

    virtual void init();

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
    QList<UMLSceneLine *> m_selectionRect;

    /**
     * The start position of the selection rectangle.
     */
    QPoint m_startPosition;

};

#endif //TOOLBARSTATEARROW_H
