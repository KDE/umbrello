/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef TOOLBARSTATEONEWIDGET_H
#define TOOLBARSTATEONEWIDGET_H

#include "toolbarstatepool.h"
#include "widgetbase.h"

/**
 * Sequence tool to create components linked with one object in sequence diagram
 * like precondition.
 * With sequence tool, one objects is selected clicking with left button on
 * it
 */
class ToolBarStateOneWidget : public ToolBarStatePool
{
    Q_OBJECT
public:
    ToolBarStateOneWidget(UMLScene *umlScene);
    virtual ~ToolBarStateOneWidget();

    virtual void cleanBeforeChange();

	// FIXME: obsolate
    virtual void mouseMove(UMLSceneMouseEvent* ome);

public Q_SLOTS:
    virtual void slotWidgetRemoved(UMLWidget* widget);

protected:
    virtual void setCurrentElement();

    virtual void mouseReleaseWidget();
    virtual void mouseReleaseEmpty();

    void setWidget(UMLWidget* firstObject);
    WidgetBase::WidgetType widgetType();

    UMLWidget* m_firstObject;  ///< The first object in the message.

    /**
     * If there is a current widget, it is true if the press event happened on
     * the line of an object, or false if it happened on a normal UMLWidget.
     */
    bool m_isObjectWidgetLine;

private:
    virtual void init();
};

#endif //TOOLBARSTATEONEWIDGET_H
