/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OBJECTWIDGET_H
#define OBJECTWIDGET_H

#define O_MARGIN 5
#define O_WIDTH 40
#define A_WIDTH 20
#define A_HEIGHT 40
#define A_MARGIN 5

#include "umlwidget.h"
#include "objectwidgetdata.h"

class SeqLineWidget;

/**
 *	Displays an instance UMLObject of a concept.
 *
 *	@short	Displays an instance of a Concept.
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@version	1.0
 *	@see	UMLWidget
 */
class ObjectWidget : public UMLWidget {
	Q_OBJECT
public:
	/**
	 *	Creates an ObjectWidget.
	 *
	 *	@param	view		The parent to this object.
	 *	@param	o				The object it will be representing.
	 *	@param	lid			The local id for the object.
	 */
	ObjectWidget(UMLView * view, UMLObject *o, int lid);

	/**
	 *	Creates an ObjectWidget.
	 *
	 *	@param	view		The parent to this object.
	 *	@param	o				The object it will be representing.
	 *  @param pData		The UMLWidgetData to represent.
	 */
	ObjectWidget(UMLView * view,  UMLObject *o, UMLWidgetData *pData);

	/**
	 *	Creates an ObjectWidget.
	 *
	 *	@param	view		The parent to this object.
	 */
	ObjectWidget(UMLView * view);

	/**
	 *	Initializes the key attributes of the class.
	 */
	void init();

	/**
	 *	Standard deconstructor.
	 */
	~ObjectWidget();

	/**
	 *  Returns the local ID for this object.  This ID is used so that
	 * many objects of the same @ref UMLObject instance can be on the
	 * same diagram.
	 *
	 *	@return Returns the local ID.
	 */
	int getLocalID() {
		return ((ObjectWidgetData*)m_pData)->m_nLocalID;
	}

	/**
	 *	Returns the instance name.
	 *
	 *	@return Returns the instance name.
	 */
	QString getInstanceName() {
		return ((ObjectWidgetData*)m_pData)->m_InstanceName;
	}

	/**
	 *	Sets the instance name.
	 *
	 *	@param	name	The name to set the instance name to.
	 */
	void setInstanceName(QString name) {
		((ObjectWidgetData*)m_pData)->m_InstanceName = name;
	}

	/**
	 *	Returns the documentation for the object.
	 *
	 *	@return	Returns the documentation for the object.
	 */
	QString getDoc();

	/**
	 *	Sets the documentation.
	 *
	 *	@param	_doc	The documentation to set to.
	 */
	void setDoc(QString _doc) {
		m_Doc = _doc;
	}

	/**
	 *	Returns whether object is representing a multi-object.
	 *
	 *	@return	Returns whether object is representing a multi-object.
	 */
	bool getMultipleInstance() {
		return ((ObjectWidgetData*)m_pData)->m_bMultipleInstance;
	}

	/**
	 *	Sets whether representing a multi-instance object.
	 *
	 *	@param	multiple	Object state. true- multi, false - single.
	 */
	void setMultipleInstance(bool m_bMultiple);

	/**
	 *	Sets the local id of the object.
	 *
	 * 	@param id the local id of the object.
	 */
	void setLocalID(int id) {
		((ObjectWidgetData*)m_pData)->m_nLocalID = id;
	}

	/**
	 *	Returns the local id of the object.
	 *
	 *	@param Returns the local id of the object.
	 */
	int getOldID() {
		return m_nOldID;
	}

	/**
	 * Activate the object after serializing it from a QDataStream
	 */
	virtual bool activate(IDChangeLog* ChangeLog = 0);

	/**
	 * Synchronize the Widget's m_pData member with its display properties, for example:
	 * the X and Y positions of the widget, etc
	 */
	virtual void synchronizeData();

	/**
	 * Override default method
	 */
	void draw(QPainter & p, int offsetX, int offsetY);

	/**
	 *	Overrides the standard operation.
	 */
	virtual void moveEvent(QMoveEvent */*m*/);

	/**
	 *   Used to cleanup any other widget it may need to delete.
	 */
	void cleanup();

	/**
	 * 		Returns whether to draw as an object or not.
	 */
	bool getDrawAsActor() {
		return ( ( ObjectWidgetData *)m_pData ) -> m_bDrawAsActor;
	}

	/**
	 * 		Sets whether to draw as an Actor.
	 */
	void setDrawAsActor( bool drawAsActor ) {
		( ( ObjectWidgetData *)m_pData ) -> m_bDrawAsActor = drawAsActor;
		calculateSize();
	}

	/**
	 *  Overrides the standard operation keeping it at the correct height
	 */
	virtual void mouseMoveEvent(QMouseEvent* me);

	/**
	 *		Sets whether to show deconstruction on sequence line
	 */
	void setShowDeconstruction( bool bShow );

	/**
	 *		Returns whether to show deconstruction on sequence line
	 */
	bool getShowDeconstruction() {
		return ( ( ObjectWidgetData *)m_pData ) -> getShowDeconstruction();
	}

	/**
	 *		Returns the end Y co-ord of the seq. line.
	 */
	int getEndLineY();
protected:
	QString m_Doc;
	int m_nOldID;
	SeqLineWidget * m_pLine;

	/**
	 *   Calculates the size of the widget.
	 */
	void calculateSize();

	/**
	 * 		Draw the object as an actor.
	 */
	void drawActor(QPainter & p, int offsetX, int offsetY);

	/**
	 * 		Draw the object as an object (default).
	 */
	void drawObject(QPainter & p, int offsetX, int offsetY);

	/**
	 *		Move the object up on a sequence diagram.
	 */
	void tabUp();

	/**
	 *		Move the object down on a sequence diagram.
	 */
	void tabDown();

public slots:
	/**
	 *     Handles a popup menu selection.
	 */
	void slotMenuSelection(int sel);

	/**
	 *   Handles a color change signal.
	 */
	virtual void slotColorChanged(int viewID);

	/**
	 *   When a message is added to the view, all lines must extend themselves.  The view will change
	 *   the length and signal all objects.
	 *
	 *   FUTURE:- signal here when you add manual line length changes by the user.
	 */
	void slotLineLengthChanged();

};

#endif
