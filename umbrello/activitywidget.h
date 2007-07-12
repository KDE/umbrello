/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ACTIVITYWIDGET_H
#define ACTIVITYWIDGET_H

#include "umlwidget.h"
#include "worktoolbar.h"

#define ACTIVITY_MARGIN 5
#define ACTIVITY_WIDTH 30
#define ACTIVITY_HEIGHT 10

/**
 * This class is the graphical version of a UML Activity.  A ActivityWidget is created
 * by a @ref UMLView.  An ActivityWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The ActivityWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML Activity.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ActivityWidget : public UMLWidget {
    Q_OBJECT

public:
    enum ActivityType
    {
        Initial = 0,
        Normal,
        End,
        Branch
    };

    /**
     * Creates a Activity widget.
     *
     * @param view              The parent of the widget.
     * @param activityType      The type of activity.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    explicit ActivityWidget( UMLView * view, ActivityType activityType = Normal, Uml::IDType id = Uml::id_None );


    /**
     *  destructor
     */
    virtual ~ActivityWidget();

    /**
     * Overrides the standard paint event.
     */
    void draw(QPainter & p, int offsetX, int offsetY);

    /**
     * Overrides Method from UMLWidget.
     */
    void constrain(int& width, int& height);

    /**
     * Returns the type of activity.
     */
    ActivityType getActivityType() const;

    /**
     * Sets the type of activity.
     */
    void setActivityType( ActivityType activityType );

    /**
     * Show a properties dialog for an ActivityWidget.
     */
    void showProperties();

    /**
     * Determines whether a toolbar button represents an Activity.
     * CHECK: currently unused - can this be removed?
     *
     * @param tbb               The toolbar button enum input value.
     * @param resultType        The ActivityType corresponding to tbb.
     *                  This is only set if tbb is an Activity.
     * @return  True if tbb represents an Activity.
     */
    static bool isActivity( WorkToolBar::ToolBar_Buttons tbb,
                            ActivityType& resultType );

    /**
     * Saves the widget to the "activitywidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the widget from the "activitywidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

protected:
    /**
     * Overrides method from UMLWidget
     */
    QSize calculateSize();

    /**
     * Type of activity.
     */
    ActivityType m_ActivityType;

public slots:

    /**
     * Captures any popup menu signals for menus it created.
     */
    void slotMenuSelection(int sel);
};

#endif
