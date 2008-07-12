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

class TextItemGroup;

class ActivityWidget : public NewUMLRectWidget
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

    ActivityType activityType() const {
        return m_activityType;
    }
    /**
     * Sets the type of activity.
     */
    void setActivityType( ActivityType activityType );

    /**
     * This method get the name of the preText attribute
     */
    QString preconditionText() const {
        return m_preconditionText;
    }
    /**
     * This method set the name of the preText attribute
     */
    void setPreconditionText(const QString&);

    /**
     * This method get the name of the postText attribute
     */
    QString postconditionText() const {
        return m_postconditionText;
    }
    /**
     * This method set the name of the postText attribute
     */
    void setPostconditionText(const QString&);

    QSizeF sizeHint(Qt::SizeHint which);

    /**
     * Show a properties dialog for an ActivityWidget.
     */
    void showPropertiesDialog();

    /**
     * Loads the widget from the "activitywidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );
    /**
     * Saves the widget to the "activitywidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *);

protected:
    /**
     * Overrides method from NewUMLRectWidget
     */
    void updateGeometry();
    void sizeHasChanged(const QSizeF& oldSize);

    /**
     * Type of activity.
     */
    ActivityType m_activityType;


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

private:
    enum {
        NameItemIndex = 0,
        PrecondtionItemIndex = 1,
        PostconditionItemIndex = 2,
    };

    QString m_preconditionText;
    QString m_postconditionText;
    QSizeF m_minimumSize;
    TextItemGroup *m_textItemGroup;
};

#endif
