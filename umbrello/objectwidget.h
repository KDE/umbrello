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
#include "messagewidget.h"

#include <qptrlist.h>
#include <qfont.h>

class SeqLineWidget;

/**
 *	Displays an instance UMLObject of a concept.
 *
 *	@short	Displays an instance of a Concept.
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
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
		return ((ObjectWidgetData*)m_pData)->getLocalID();
	}

	/**
	 *	Returns the instance name.
	 *
	 *	@return Returns the instance name.
	 */
	QString getInstanceName() {
		return ((ObjectWidgetData*)m_pData)->getInstanceName();
	}

	/**
	 *	Sets the instance name.
	 *
	 *	@param	name	The name to set the instance name to.
	 */
	void setInstanceName(QString name) {
		((ObjectWidgetData*)m_pData)->setInstanceName( name );
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
		return ((ObjectWidgetData*)m_pData)->getMultipleInstance();
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
		((ObjectWidgetData*)m_pData)->setLocalID( id );
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
		return ( ( ObjectWidgetData *)m_pData ) -> getDrawAsActor();
	}

	/**
	 * 		Sets whether to draw as an Actor.
	 */
	void setDrawAsActor( bool drawAsActor ) {
		( ( ObjectWidgetData *)m_pData ) -> setDrawAsActor( drawAsActor );
		calculateSize();
	}

	/**
	 *  Overrides the standard operation keeping it at the correct height
	 */
	virtual void mouseMoveEvent(QMouseEvent* me);

	/**
	 *		Sets whether to show deconstruction on sequence line
	 */
	void setShowDestruction( bool bShow );

	/**
	 *		Returns whether to show deconstruction on sequence line
	 */
	bool getShowDestruction() {
		return ( ( ObjectWidgetData *)m_pData ) -> getShowDestruction();
	}

	/**
	 *		Returns the end Y co-ord of the seq. line.
	 */
	int getEndLineY();

	/**
	 * Add a message widget to the list
	 */
	void messageAdded(MessageWidget* message);

	/**
	 * Remove a message widget from the list
	 */
	void messageRemoved(MessageWidget* message);

	/**
	 * returns whether or not the widget can be moved vertically up
	 */
	bool canTabUp();

	/**
	 * Returns whether a message is overlapping with another message, 
	 * used by MessageWidget::draw() methods
	 * 
	 * @param y the top of your message
	 * @param messageWidget a pointer to your message so it doesn't check against itself
	 */
	bool messageOverlap(int y, MessageWidget* messageWidget);

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

private:
	/**
	 * A list of the message widgets with an end on this widget
	 */
	QPtrList<MessageWidget> messageWidgetList;

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
	 * Called when a message widget with an end on this object has moved up or down.
	 * Sets the bottom of the line to a nice position.
	 */
	void slotMessageMoved();
};

#endif
