/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
***************************************************************************/

#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include <QtGui/QPainter>
#include <QtCore/QStringList>

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
 * The StateWidget class inherits from the @ref UMLWidget class
 * which adds most of the functionality to this class.
 *
 * @short  A graphical version of a UML State.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class StateWidget : public UMLWidget
{
    Q_OBJECT
    Q_ENUMS(StateType)
public:
    /// Enumeration that codes the different types of state.
    enum StateType
    {
        Initial = 0,     // Pseudostate
        Normal,
        End,
        Fork,            // Pseudostate
        Join,            // Pseudostate
        Junction,        // Pseudostate
        DeepHistory,     // Pseudostate
        ShallowHistory,  // Pseudostate
        Choice           // Pseudostate
        //Terminate        // Pseudostate
        //EntryPoint       // Pseudostate
        //ExitPoint        // Pseudostate
    };

    explicit StateWidget(UMLScene * scene, StateType stateType = Normal, Uml::IDType id = Uml::id_None);
    virtual ~StateWidget();

    void paint(QPainter & p, int offsetX, int offsetY);

    virtual QString name() const;
    virtual void setName(const QString &strName);

    StateType stateType() const;
    void setStateType(StateType stateType);

    bool addActivity(const QString &activity);
    bool removeActivity(const QString &activity);
    bool renameActivity(const QString &activity, const QString &newName);

    QStringList activities() const;
    void setActivities(const QStringList &list);

    virtual void showPropertiesDialog();

    static bool isState(WorkToolBar::ToolBar_Buttons tbb,
                        StateType& resultType);

    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

protected:
    UMLSceneSize minimumSize();

public slots:
    virtual void slotMenuSelection(QAction* action);

private:
    StateType   m_stateType;   ///< Type of state.
    QStringList m_Activities;  ///< List of activities for the state.

};

#endif
