/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef TOOLBARSTATEONEWIDGET_H
#define TOOLBARSTATEONEWIDGET_H

#include "toolbarstatepool.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <qpoint.h>

class ObjectWidget;
class NewUMLRectWidget;

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

    /**
     * Creates a new ToolBarStateOneWidget.
     *
     * @param umlView The UMLView to use.
     */
    ToolBarStateOneWidget(UMLScene *umlScene);

    /**
     * Destroys this ToolBarStateOneWidget.
     */
    virtual ~ToolBarStateOneWidget();

    virtual void init();

    virtual void cleanBeforeChange();

    virtual void mouseMove(QGraphicsSceneMouseEvent* ome);

public slots:

    virtual void slotWidgetRemoved(NewUMLRectWidget* widget);

protected:

    virtual void setCurrentElement();

    virtual void mouseReleaseWidget();

    virtual void mouseReleaseEmpty();

protected:

    void setWidget(NewUMLRectWidget* firstObject);


    Uml::Widget_Type getWidgetType();


    /**
     * The first object in the message.
     */
    NewUMLRectWidget* m_firstObject;

    UMLScene * m_umlScene;

    /**
     * If there is a current widget, it is true if the press event happened on
     * the line of an object, or false if it happened on a normal NewUMLRectWidget.
     */
    bool m_isObjectWidgetLine;

};

#endif //TOOLBARSTATEONEWIDGET_H
