/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include "umlwidget.h"

// forward declarations
class FloatingText;
class ObjectWidget;
/**
 * Used to display a message on a sequence diagram.  The message
 * could be between two objects or a message that calls itself on
 * an object.  This class will only display the line that is
 * required and the text will be setup by the @ref FloatingText
 * widget that is passed in the constructor.  A message can be
 * synchronous (calls a method and gains control back on return,
 * as happens in most programming languages) or asynchronous
 * (calls a method and gains back control immediately).
 *
 * @short Displays a message.
 * @author Paul Hensgen
 * @see	UMLWidget
 * @see	ObjectWidget
 * @see	FloatingText
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class MessageWidget : public UMLWidget {
	Q_OBJECT
public:

	/**
	 * Constructs a MessageWidget.
	 *
	 * @param view	The parent to this class.
	 * @param a	The role A widget for this message.
	 * @param b	The role B widget for this message.
	 * @param ft	The FloatingText widget that is needed to display text.
	 * @param y	The vertical position to display this message.
	 * @param sequenceMessageType Whether synchronous or asynchronous
	 * @param id	A unique id used for deleting this object cleanly.
	 *              The default (-1) will prompt generation of a new ID.
	 */
	MessageWidget(UMLView * view, ObjectWidget* a, ObjectWidget* b, FloatingText* ft, 
		      int y, Sequence_Message_Type sequenceMessageType, int id = -1);

	/**
	 * Constructs a MessageWidget.
	 *
	 * @param view		The parent to this class.
	 * @param id		The ID to assign (-1 will prompt a new ID.)
	 */
	MessageWidget(UMLView * view, Sequence_Message_Type sequenceMessageType, int id = -1);

	/**
	 * Initializes key variables of the class.
	 */
	void init();

	/**
	 * Standard deconstructor.
	 */
	virtual ~MessageWidget();

	/**
	 * Write property of QString m_SequenceNumber.
	 */
	void setSequenceNumber( QString sequenceNumber );

	/**
	 * Read property of QString m_SequenceNumber.
	 */
	QString getSequenceNumber() const;

	/**
	 * Write property of QString m_Operation.
	 */
	void setOperation( QString operation );

	/**
	 * Read property of QString m_Operation.
	 */
	QString getOperation() const;

	/**
	 * Returns whether the message is synchronous or asynchronous
	 */
	Sequence_Message_Type getSequenceMessageType() const {
		return m_sequenceMessageType;
	}

	/**
	 * Check to see if the given ObjectWidget is involved in the message.
	 *
	 * @param w	The ObjectWidget to check for.
	 * @return 	true - if is contained, false - not contained.
	 */
	bool contains(ObjectWidget * w);

	/**
	 * Returns the related widget on the given side.
	 *
	 * @return	The ObjectWidget we are related to.
	 */
	ObjectWidget* getWidget(Role_Type role);

	/**
	 * Sets the related widget on the given side.
	 *
	 * @param ow	The ObjectWidget we are related to.
	 */
	void setWidget(ObjectWidget * ow, Role_Type role) ;

	/**
	 * Returns the text widget it is related to.
	 *
	 * @return	The text widget we are related to.
	 */
	FloatingText * getFloatingText() {
		return m_pFText;
	}

	/**
	 * Sets the text widget it is related to.
	 *
	 * @param f	The text widget we are related to.
	 */
	void setFloatingText(FloatingText * f) {
		m_pFText = f;
	}

	/**
	 * Overrides operation from LinkWidget.
	 * Required by FloatingText.
	 */
	UMLClassifier *getOperationOwner(FloatingText *ft);

	/**
	 * Overrides operation from LinkWidget.
	 * Required by FloatingText.
	 */
	void setOperationText(FloatingText *ft, QString opText);

	/**
	 * Overrides operation from LinkWidget.
	 * Required by FloatingText.
	 *
	 * @param ft	The text widget which to update.
	 */
	void setMessageText(FloatingText *ft);

	/**
	 * Overrides operation from LinkWidget.
	 * Required by FloatingText.
	 *
	 * @param ft	The text widget which to update.
	 * @param newText	The new text to set.
	 */
	void setText(FloatingText *ft, QString newText);

	/**
	 * Overrides operation from LinkWidget.
	 * Required by FloatingText.
	 *
	 * @param seqNum	The new sequence number string to set.
	 * @param op		The new operation string to set.
	 */
	void setSeqNumAndOp(QString seqNum, QString op);

	/**
	 * Overrides operation from LinkWidget.
	 * Required by FloatingText.
	 *
	 * @param ft		The calling FloatingText.
	 * @param seqNum	Return this MessageWidget's sequence number string.
	 * @param op		Return this MessageWidget's operation string.
	 */
	UMLClassifier * getSeqNumAndOp(FloatingText *ft, QString& seqNum,
							 QString& op);

	/**
	 * Overrides operation from LinkWidget.
	 * Required by FloatingText.
	 */
	void setupAfterFTsetLink(FloatingText *ft);

	/**
	 * Overrides operation from LinkWidget.
	 * Required by FloatingText.
	 */
	void setFTselected(FloatingText *ft);

	/**
	 * Calculate the geometry of the widget.
	 */
	void calculateWidget();

	/**
	 * Activates a MessageWidget.  Connects its m_pOw[] pointers
	 * to UMLObjects and also send signals about its FloatingText.
	 */
	bool activate(IDChangeLog * Log = 0);

	/**
	 * Calculates the size of the widget by calling
	 * calculateDimenstionsSynchronous() or
	 * calculateDimenstionsAsynchronous()
	 */
	void calculateDimensions();

	/**
	 * Calculates and sets the size of the widget for a synchronous message
	 */
	void calculateDimensionsSynchronous();

	/**
	 * Calculates and sets the size of the widget for an asynchronous message
	 */
	void calculateDimensionsAsynchronous();

	/** 
	 * Calls drawSynchronous() or drawAsynchronous()
	 */
	void draw(QPainter& p, int offsetX, int offsetY);

	/**
	 * Draws the calling arrow with filled in arrowhead, the
	 * timeline box and the returning arrow with a dashed line and
	 * stick arrowhead.
	 */
	void drawSynchronous(QPainter& p, int offsetX, int offsetY);

	/**
	 * Draws a solid arrow line and a stick arrow head.
	 */
	void drawAsynchronous(QPainter& p, int offsetX, int offsetY);

	/**
	 * Sets the text position relative to the sequence message.
	 */
	void setTextPosition();

	/**
	 * Updates the message widget X and Y coordinates based on
	 * the given height of a floating text.
	 * Overrides operation from LinkWidget.
	 */
	void updateMessagePos(int textHeight, int& newX, int& newY);

	/**
	 * Used to cleanup any other widget it may need to delete.
	 */
	void cleanup();

	/**
	 * Overrides the standard operation.
	 */
	void mouseMoveEvent(QMouseEvent *me);

	/**
	 * Sets the state of whether the widget is selected.
	 *
	 * @param _select	True if the widget is selected.
	 */
	void setSelected(bool _select);

	/**
	 * Returns the minimum height this widget should be set at on
	 * a sequence diagrams.  Takes into account the widget positions
	 * it is related to.
	 */
	int getMinHeight();

	/**
	 * Returns the maximum height this widget should be set at on
	 * a sequence diagrams.  Takes into account the widget positions
	 * it is related to.
	 */
	int getMaxHeight();

	/**
	 * Overrides the standard operation.
	 */
	virtual void mouseReleaseEvent(QMouseEvent * me);

	/**
	 * Overrides the standard operation.
	 */
	virtual void mousePressEvent(QMouseEvent *me);

	/**
	 * Saves to the <UML:MessageWidget> XMI element.
	 */
	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads from the <UML:MessageWidget> XMI element.
	 */
	bool loadFromXMI( QDomElement & qElement );

protected:
	// Data loaded/saved
	QString m_SequenceNumber;
	QString m_Operation;
	/**
	 * Whether the message is synchronous or asynchronous
	 */
        Sequence_Message_Type m_sequenceMessageType;

private:
	void moveEvent(QMoveEvent */*m*/);
	void resizeEvent(QResizeEvent */*re*/);
	void mouseDoubleClickEvent(QMouseEvent */* me*/);

	ObjectWidget * m_pOw[2];
	FloatingText * m_pFText;
	int m_nY;
public slots:
	void slotWidgetMoved(int id);
	void slotMenuSelection(int sel);
signals:
	/**
	 * emitted when the message widget is moved up or down
	 * slots into ObjectWidget::slotMessageMoved()
	 */
	void sigMessageMoved();
};

#endif
