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
#include "messagewidgetlist.h"
#include "messagewidget.h"

#include <qptrlist.h>
#include <qfont.h>

class SeqLineWidget;

/**
 * Displays an instance UMLObject of a concept.
 *
 * @short Displays an instance of a Concept.
 * @author Paul Hensgen	<phensgen@techie.com>
 * @see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ObjectWidget : public UMLWidget {
	Q_OBJECT
public:
	/**
	 * Creates an ObjectWidget.
	 *
	 * @param view		The parent to this object.
	 * @param o		The object it will be representing.
	 * @param lid		The local id for the object.
	 */
	ObjectWidget(UMLView * view, UMLObject *o, int lid = -1 );

	/**
	 * destructor 
	 */
	virtual ~ObjectWidget();

	/**
	 * Returns the local ID for this object.  This ID is used so that
	 * many objects of the same @ref UMLObject instance can be on the
	 * same diagram.
	 *
	 * @return	The local ID.
	 */
	int getLocalID() const {
		return m_nLocalID;
	}

	/**
	 * Returns the instance name.
	 *
	 * @return	The instance name.
	 */
	QString getInstanceName() const {
		return m_InstanceName;
	}

	/**
	 * Sets the instance name.
	 *
	 * @param name		The name to set the instance name to.
	 */
	void setInstanceName(QString name) {
		m_InstanceName = name;
	}

	/**
	 * Returns the documentation for the object.
	 *
	 * @return	The documentation for the object.
	 */
	QString getDoc() const;

	/**
	 * Sets the documentation.
	 *
	 * @param _doc		The documentation to set to.
	 */
	void setDoc(QString _doc) {
		m_Doc = _doc;
	}

	/**
	 * Returns whether object is representing a multi-object.
	 *
	 * @return	True if object is representing a multi-object.
	 */
	bool getMultipleInstance() const {
		return m_bMultipleInstance;
	}

	/**
	 * Sets whether representing a multi-instance object.
	 *
	 * @param multiple	Object state. true- multi, false - single.
	 */
	void setMultipleInstance(bool multiple);

	/**
	 * Sets the local id of the object.
	 *
	 * @param id		The local id of the object.
	 */
	void setLocalID(int id) {
		m_nLocalID = id;
	}

	/**
	 * Activate the object after serializing it from a QDataStream
	 */
	virtual bool activate(IDChangeLog* ChangeLog = 0);

	/**
	 * Override default method.
	 */
	void draw(QPainter & p, int offsetX, int offsetY);

	/**
	 * Overrides the standard operation.
	 */
	virtual void moveEvent(QMoveEvent */*m*/);

	/**
	 * Used to cleanup any other widget it may need to delete.
	 */
	void cleanup();

	/**
	 * Returns whether to draw as an Actor or not.
	 *
	 * @return	True if widget is drawn as an actor.
	 */
	bool getDrawAsActor() const {
		return m_bDrawAsActor;
	}

	/**
	 * Sets whether to draw as an Actor.
	 *
	 * @param drawAsActor	True if widget shall be drawn as an actor.
	 */
	void setDrawAsActor( bool drawAsActor ) {
		m_bDrawAsActor = drawAsActor;
	}

	/**
	 * Overrides the standard operation keeping it at the correct height
	 */
	virtual void mouseMoveEvent(QMouseEvent* me);

	/**
	 * Sets whether to show deconstruction on sequence line.
	 *
	 * @param bShow		True if destruction on line shall be shown.
	 */
	void setShowDestruction( bool bShow );

	/**
	 * Returns whether to show deconstruction on sequence line.
	 *
	 * @return	True if destruction on sequence line is shown.
	 */
	bool getShowDestruction() const {
		return m_bShowDestruction;
	}

	/**
	 * Returns the end Y co-ord of the seq. line.
	 *
	 * @return	Y coordinate of the endpoint of the sequence line.
	 */
	int getEndLineY();

	/**
	 * Add a message widget to the list.
	 *
	 * @param message	Pointer to the MessageWidget to add.
	 */
	void messageAdded(MessageWidget* message);

	/**
	 * Remove a message widget from the list.
	 *
	 * @param message	Pointer to the MessageWidget to remove.
	 */
	void messageRemoved(MessageWidget* message);

	/**
	 * Returns whether or not the widget can be moved vertically up.
	 *
	 * @return	True if widget can be moved upwards vertically.
	 */
	bool canTabUp();

	/**
	 * Returns whether a message is overlapping with another message.
	 * Used by MessageWidget::draw() methods.
	 * 
	 * @param y		The top of your message.
	 * @param messageWidget	A pointer to your message so it doesn't
	 *			check against itself.
	 */
	bool messageOverlap(int y, MessageWidget* messageWidget);

	/**
	 * Saves to the <UML:ObjectWidget> XMI element.
	 */
	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads from a <UML:ObjectWidget> XMI element.
	 */
	bool loadFromXMI( QDomElement & qElement );
	
public slots:
	/**
	 * Handles a popup menu selection.
	 */
	void slotMenuSelection(int sel);

	/**
	 * Handles a color change signal.
	 */
	virtual void slotColorChanged(int viewID);

	/**
	 * Called when a message widget with an end on this object has
	 * moved up or down.
	 * Sets the bottom of the line to a nice position.
	 */
	void slotMessageMoved();	

protected:
	QString m_Doc;
	SeqLineWidget * m_pLine;

	/**
	 * Calculates the size of the widget.
	 */
	void calculateSize();

	/**
	 * Draw the object as an actor.
	 */
	void drawActor(QPainter & p, int offsetX, int offsetY);

	/**
	 * Draw the object as an object (default).
	 */
	void drawObject(QPainter & p, int offsetX, int offsetY);

	/**
	 * Move the object up on a sequence diagram.
	 */
	void tabUp();

	/**
	 * Move the object down on a sequence diagram.
	 */
	void tabDown();

	// Data loaded/saved:

	/**
	 * Instance name of object.
	 */
	QString m_InstanceName;

	/**
	 * Local ID used on views.  Needed as a it can represent a class
	 * that has many objects representing it.
	 */
	int m_nLocalID;

	/**
	 * Determines whether to draw an object as a multiple object
	 * instance.
	 */
	bool m_bMultipleInstance;

	/**
	 * Determines whether the object should be drawn as an Actor or 
	 * an Object.
	 */
	bool m_bDrawAsActor;

	/**
	 * Determines whether to show object destruction on sequence
	 * diagram line.
	 */
	bool m_bShowDestruction;

private:
	/**
	 * Initializes the key attributes of the class.
	 */
	void init();
	
	/**
	 * A list of the message widgets with an end on this widget.
	 */
	MessageWidgetList messageWidgetList;
};

#endif
