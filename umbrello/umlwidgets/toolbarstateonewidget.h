/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
    explicit ToolBarStateOneWidget(UMLScene *umlScene);
    virtual ~ToolBarStateOneWidget();

    virtual void cleanBeforeChange();

    // FIXME: obsolete
    virtual void mouseMove(QGraphicsSceneMouseEvent* ome);

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
