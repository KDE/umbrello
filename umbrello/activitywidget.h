/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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
	    Branch,
	    Fork
	};

	/**
	 *	Creates a Activity widget.
	 *
	 *	@param	view		The parent of the widget.
	 *	@param	activityType	The type of activity.
	 */
	ActivityWidget( UMLView * view, ActivityType activityType );

	/**
	 *	Creates a Activity widget.
	 *
	 *	@param	view		The parent of the widget.
	 */
	ActivityWidget(UMLView * view);

	/**
	 *	Standard deconstructor
	 */
	~ActivityWidget();

	/**
	 *	Overrides the standard paint event.
	 */
	void draw(QPainter & p, int offsetX, int offsetY);

	/**
	 * Sets the name of the Activity.
	 */
	virtual void setName(QString strName);

	/**
	 * Returns the name of the Activity.
	 */
	virtual QString getName() const;

	/**
	 *   Returns the documentation of the activity.
	 */
	QString getDoc() const;

	/**
	 *   Sets the documenation of the activity.
	 */
	void setDoc( QString doc );

	/**
	 *   Returns the type of activity.
	 */
	ActivityType getActivityType() const;

	/**
	 *     Sets the type of activity.
	 */
	void setActivityType( ActivityType activityType );

	/**
	 *   Overrides a method.  Used to pickup double clicks.
	 */
	void mouseDoubleClickEvent(QMouseEvent * /*me*/);

	static bool isActivity( WorkToolBar::ToolBar_Buttons tbb,
				ActivityType& resultType );

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );

protected:
	/**
	 *	Calculates the size of the widget.
	 */
	void calculateSize();

	/*
	*   Data loaded/saved
	*/

	/**
	*   Type of activity.
	*/
	ActivityWidget::ActivityType m_ActivityType;

	/**
	*   Name of the activity.
	*/
	QString m_Name;

	/**
	*   Documentation for the activity;
	*/
	QString m_Doc;

public slots:

	/**
	 *   Captures any popup menu signals for menus it created.
	 */
	void slotMenuSelection(int sel);
};

#endif
