/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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
	 * @param view		The parent of the widget.
	 * @param stateType	The type of state.
	 */
	StateWidget( UMLView * view, StateType stateType = StateWidget::Normal );

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
	virtual void setName(QString strName);

	/**
	 * Returns the name of the State.
	 */
	virtual QString getName() const;

	/**
	 * Returns the documentation of the state.
	 */
	QString getDoc() const;

	/**
	 * Sets the documenation of the state.
	 */
	void setDoc( QString doc );

	/**
	 * Returns the type of state.
	 */
	StateType getStateType() const;

	/**
	 * Sets the type of state.
	 */
	void setStateType( StateType stateType );

	/**
	 * Overrides a method.  Used to pickup double clicks.
	 */
	void mouseDoubleClickEvent(QMouseEvent * /*me*/);

	/**
	 * Adds the given activity to the state.
	 */
	bool addActivity( QString activity );

	/**
	 * Removes the given activity from the state.
	 */
	bool removeActivity( QString activity );

	/**
	 * Renames the given activity.
	 */
	bool renameActivity( QString activity, QString newName );

	/**
	 * Sets the states activities to the ones given.
	 */
	void setActivities( QStringList & list );

	/**
	 * Returns the list of activities.
	 */
	QStringList & getActivityList();

	/**
	 * Returns true if the given toolbar button represents a State.
	 *
	 * @param tbb		Input value of type WorkToolBar::ToolBar_Buttons.
	 * @param resultType	Output value, the StateType that corresponds to tbb.
	 *			Only set if the method returns true.
	 */
	static bool isState( WorkToolBar::ToolBar_Buttons tbb,
			     StateType& resultType );

	/**
	 * Creates the <UML:StateWidget> XMI element.
	 */
	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads a <UML:StateWidget> XMI element.
	 */
	bool loadFromXMI( QDomElement & qElement );

protected:
	/**
	 * Calculates the size of the widget.
	 */
	void calculateSize();

	/**
	 * Type of state.
	 */
	StateWidget::StateType m_StateType;

	/**
	 * Name of the state.
	 */
	QString m_Name;

	/**
	 * Documentation for the state
	 */
	QString m_Doc;

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
