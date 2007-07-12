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

#ifndef STATEWIDGET_H
#define STATEWIDGET_H
#include <qpainter.h>
#include <qstringlist.h>
#include "umlwidget.h"
#include "worktoolbar.h"

#define STATE_MARGIN 5
#define STATE_WIDTH 30
#define STATE_HEIGHT 10

/**
 * This class is the graphical version of a UML State.
 *
 * A StateWidget is created by a @ref UMLView.  A StateWidget belongs to
 * only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to is destroyed,
 * it will be automatically deleted.
 *
 * The StateWidget class inherits from the @ref UMLWidget class which adds
 * most of the functionality to this class.
 *
 * @short  A graphical version of a UML State.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class StateWidget : public UMLWidget {
    Q_OBJECT
public:

    /// Enumeration that codes the different types of state.
    enum StateType
    {
        Initial = 0,
        Normal,
        End
    };

    /**
     * Creates a State widget.
     *
     * @param view              The parent of the widget.
     * @param stateType The type of state.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    explicit StateWidget( UMLView * view, StateType stateType = Normal, Uml::IDType id = Uml::id_None );

    /**
     * destructor
     */
    virtual ~StateWidget();

    /**
     * Overrides the standard paint event.
     */
    void draw(QPainter & p, int offsetX, int offsetY);

    /**
     * Sets the name of the State.
     */
    virtual void setName(const QString &strName);

    /**
     * Returns the name of the State.
     */
    virtual QString getName() const;

    /**
     * Returns the type of state.
     */
    StateType getStateType() const;

    /**
     * Sets the type of state.
     */
    void setStateType( StateType stateType );

    /**
     * Adds the given activity to the state.
     */
    bool addActivity( const QString &activity );

    /**
     * Removes the given activity from the state.
     */
    bool removeActivity( const QString &activity );

    /**
     * Renames the given activity.
     */
    bool renameActivity( const QString &activity, const QString &newName );

    /**
     * Sets the states activities to the ones given.
     */
    void setActivities( QStringList & list );

    /**
     * Returns the list of activities.
     */
    QStringList & getActivityList();

    /**
     * Show a properties dialog for a StateWidget.
     */
    void showProperties();

    /**
     * Returns true if the given toolbar button represents a State.
     *
     * @param tbb               Input value of type WorkToolBar::ToolBar_Buttons.
     * @param resultType        Output value, the StateType that corresponds to tbb.
     *                  Only set if the method returns true.
     */
    static bool isState( WorkToolBar::ToolBar_Buttons tbb,
                         StateType& resultType );

    /**
     * Creates the "statewidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads a "statewidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

protected:
    /**
     * Overrides method from UMLWidget
     */
    QSize calculateSize();

    /**
     * Type of state.
     */
    StateType m_StateType;

    /**
     * List of activities for the state.
     */
    QStringList m_Activities;

public slots:

    /**
     * Captures any popup menu signals for menus it created.
     */
    void slotMenuSelection(int sel);
};

#endif
