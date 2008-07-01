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
 * The ActivityWidget class inherits from the @ref NewUMLRectWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML Activity.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ActivityWidget : public NewUMLRectWidget {
    Q_OBJECT

    QString preText;
    QString postText;
public:
    enum ActivityType
    {
        Initial = 0,
        Normal,
        End,
        Final,
        Branch,
        Invok,
        Param
    };

    /**
     * Creates a Activity widget.
     *
     * @param view              The parent of the widget.
     * @param activityType      The type of activity.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    explicit ActivityWidget( UMLScene * scene, ActivityType activityType = Normal, Uml::IDType id = Uml::id_None );


    /**
     *  destructor
     */
    virtual ~ActivityWidget();

    /**
     * Overrides the standard paint event.
     */
    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *);

    /**
     * Overrides Method from NewUMLRectWidget.
     */
    void constrain(qreal& width, qreal& height);

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


    /**
     * This method set the name of the preText attribute
     */
     void setPreText(const QString&);

    /**
     * This method get the name of the preText attribute
     */
     QString getPreText();

     /**
     * This method set the name of the postText attribute
     */
     void setPostText(const QString&);

   /**
     * This method get the name of the postText attribute
     */
     QString getPostText();

protected:
    /**
     * Overrides method from NewUMLRectWidget
     */
    QSizeF calculateSize();

    /**
     * Type of activity.
     */
    ActivityType m_ActivityType;


    /**
     * Type of normal activity (Invok or not).
     * This function is call by the dialog box properties
     */
    bool m_NormalActivityType;

public slots:

    /**
     * Captures any popup menu signals for menus it created.
     */
    void slotMenuSelection(QAction* action);
};

#endif
