/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
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
class ActivityWidget : public UMLWidget
{
    Q_OBJECT

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

    explicit ActivityWidget( UMLScene * scene, ActivityType activityType = Normal, Uml::IDType id = Uml::id_None );
    virtual ~ActivityWidget();

    ActivityType activityType() const;
    void setActivityType( ActivityType activityType );

    static bool isActivity( WorkToolBar::ToolBar_Buttons tbb,
                            ActivityType& resultType );

    QString preconditionText();
    void setPreconditionText(const QString&);

    QString postconditionText();
    void setPostconditionText(const QString&);

    virtual void showPropertiesDialog();

    virtual void paint(QPainter & p, int offsetX, int offsetY);

    virtual bool loadFromXMI( QDomElement & qElement );
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    void constrain(int& width, int& height);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    virtual UMLSceneSize minimumSize();
    virtual UMLSceneSize maximumSize();

    ActivityType m_activityType; ///< Type of activity.

    /**
     * Type of normal activity (Invok or not).
     * This function is call by the dialog box properties
     */
    bool m_NormalActivityType;

    QString m_preconditionText;
    QString m_postconditionText;
};

#endif
