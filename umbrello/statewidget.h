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
#include "umlwidget.h"

#define STATE_MARGIN 5
#define STATE_WIDTH 30
#define STATE_HEIGHT 10

/**
 * This class is the graphical version of a UML State.  A StateWidget is created
 * by a @ref UMLView.  An StateWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The StateWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML State.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class StateWidget : public UMLWidget {
	Q_OBJECT

public:

	enum StateType
	{
	    Initial = 0,
	    Normal,
	    End
	};
	/**
	 *	Creates a State widget.
	 *
	 *	@param	view		The parent of the widget.
	 *	@param	pData		The UMLWidgetData to represent.
	 */
	StateWidget( UMLView * view, UMLWidgetData* pData );

	/**
	 *	Creates a State widget.
	 *
	 *	@param	view		The parent of the widget.
	 *	@param	stateType  The type of state.
	 */
	StateWidget( UMLView * view, StateType stateType );

	/**
	 *	Creates a State widget.
	 *
	 *	@param	view		The parent of the widget.
	 */
	StateWidget(UMLView * view);

	/**
	 *	Standard deconstructor
	 */
	~StateWidget();

	/**
	 * Synchronize the Widget's m_pData member with its display properties, for example:
	 * the X and Y positions of the widget, etc
	 */
	virtual void synchronizeData();

	/**
	 *	Overrides the standard paint event.
	 */
	void draw(QPainter & p, int offsetX, int offsetY);

	/**
	 * Sets the name of the State.
	 */
	virtual void setName(QString strName);

	/**
	 * Returns the name of the State.
	 */
	virtual QString getName();

	/**
	 *   Returns the documentation of the state.
	 */
	QString getDoc();

	/**
	 *   Sets the documenation of the state.
	 */
	void setDoc( QString doc );

	/**
	 *   Returns the type of state.
	 */
	StateType getStateType();

	/**
	 *     Sets the type of state.
	 */
	void setStateType( StateType stateType );

	/**
	 *   Overrides a method.  Used to pickup double clicks.
	 */
	void mouseDoubleClickEvent(QMouseEvent * /*me*/);

	/**
	 *		Adds the given activity to the state.
	 */
	bool addActivity( QString activity );

	/**
	 *		Removes the given activity from the state.
	 */
	bool removeActivity( QString activity );

	/**
	 *		Renames the given activity.
	 */
	bool renameActivity( QString activity, QString newName );

	/**
	 *		Sets the states activities to the ones given.
	 */
	void setActivities( QStringList & list );

	/**
	 *		Returns the list of activities.
	 */
	QStringList & getActivityList();

protected:
	/**
	 *	Calculates the size of the widget.
	 */
	void calculateSize();

public slots:

	/**
	 *   Captures any popup menu signals for menus it created.
	 */
	void slotMenuSelection(int sel);
};

#endif
