/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FLOATINGTEXT_H
#define FLOATINGTEXT_H

#include "umlwidget.h"

class AssociationWidget;
class MessageWidget;
class UMLView;

/**
 * This is a multipurpose class.  In its simplest form it will display a
 * line of text.
 * It can also be setup to be the text for an operation with regard to the
 * @ref MessageWidget on the sequence diagram.
 * It is also used for the text required for an association.
 *
 * The differences between all these different uses will be the popup menu
 * that is associated with it.
 *
 * @short Displays a line of text or an operation.
 * @author Paul Hensgen	<phensgen@techie.com>
 * @see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class FloatingText : public UMLWidget {
public:
	/** sometimes the x/y values get numbers of <0 and >10000 - which is
	    probably due to a bug somewhere in calculating the position.
			-> workaround till problem is found: define min and max limits
			=> if x or y is outside of interval, the position is reset
		   ( e.g. by AssociationWidget::resetTextPositions() )
	 */
	static const int restrictPositionMin = 0;
	static const int restrictPositionMax = 3000;


	/**
	 * Constructs a FloatingText instance.
	 *
	 * @param view	The parent of this FloatingText.
	 * @param role	The role this FloatingText will take up.
	 * @param text	The main text to display.
	 * @param id	The ID to assign (-1 will prompt a new ID.)
	 */
	FloatingText(UMLView * view, Text_Role role = Uml::tr_Floating,
		     QString text = "", int id = -1);

	/**
	 * destructor
	 */
	virtual ~FloatingText();

	/**
	 * Called to set the position of the FloatingText.
	 *
	 * @param x	Vertical co-ordinate of the point.
	 * @param y	Horizontal co-ordinate of the point.
	 */
	void setLinePos(int x, int y);

	/**
	 * Called to set the position of the FloatingText by the difference
	 * between the two points.
	 *
	 * @param newX	The new X coordinate.
	 * @param newY	The new Y coordinate.
	 * @param oldX	The old X coordinate.
	 * @param oldY	The old Y coordinate.
	 */
	void setLinePositionRelatively(int newX, int newY, int oldX, int oldY);

	/**
	 * Set the main body of text to display.
	 *
	 * @param t	The text to display.
	 */
	void setText(QString t);

	/**
	 * Set some text to be prepended to the main body of text.
	 * @param t	The text to prepend to main body which is displayed.
	 */
	void setPreText(QString t);

	/**
	 * Set some text to be appended to the main body of text.
	 * @param t	The text to append to main body which is displayed.
	 */
	void setPostText(QString t);

	/**
	 * Set the sequence number to display.
	 *
	 * @param sn	The sequence number to display.
	 *
	void setSeqNum(QString sn);

	 **
	 * Return the sequence number.
	 *
	 * @return	The sequence number.
	 *
	QString getSeqNum() const;

	 **
	 * Set the operation to display.
	 *
	 * @param op	The operation to display.
	 *
	void setOperation(QString op);

	 **
	 * Return the operation that is displayed.
	 *
	 * @return	The operation that is displayed.
	 *
	QString getOperation() const;
	 */

	/**
	 * Use to get the _main body_ of text (e.g. prepended and appended
	 * text is omitted) as currently displayed by the widget.
	 *
	 * @return	The main text currently being displayed by the widget.
	 */
	QString getText() const;

	/**
	 * Use to get the pre-text which is prepended to the main body of
	 * text to be displayed.
	 *
	 * @return	The pre-text currently displayed by the widget.
	 */
	QString getPreText() const;

	/**
	 * Use to get the post-text which is appended to the main body of
	 * text to be displayed.
	 *
	 * @return	The post-text currently displayed by the widget.
	 */
	QString getPostText() const;

	/**
	 * Use to get the total text (prepended + main body + appended)
	 * currently displayed by the widget.
	 *
	 * @return	The text currently being displayed by the widget.
	 */
	QString getDisplayText() const;

	/**
	 * If this object is associated with a @ref AssociationWidget
	 * then this method will be called by that association when it
	 * thinks this object has been right clicked on.
	 *
	 * @param a	The AssociationWidget linked to this FloatingText.
	 * @param p	The point at which the right button was clicked.
	 */
	void startMenu(AssociationWidget * a, QPoint p);

	/**
	 * Displays a dialog box to change the text.
	 */
	void changeTextDlg();

	/**
	 * Sets the message to the @ref MessageWidget that this class may
	 * represent.
	 *
	 * @param m	The MessageWidget that may be represented.
	 */
	void setMessage(MessageWidget * m);

	/**
	 * Sets the position relative to the sequence message.
	 */
	void setPositionFromMessage();

	/**
	 * Returns the @ref MessageWidget this floating text is related to.
	 *
	 * @return	The @ref MessageWidget this floating text is
	 *		related to.
	 */
	MessageWidget * getMessage() const {
		return m_pMessage;
	}

	/**
	 * Overrides a method.  Used to pickup double clicks.
	 */
	void mouseDoubleClickEvent(QMouseEvent * /*me*/);

	/**
	 * Sets the @ref AssociationWidget to represent.
	 *
	 * @param a	The Association to represent.
	 */
	void setAssoc(AssociationWidget * a);

	/**
	 * Returns whether this is a line of text.
	 * Used for transparency in printing.
	 *
	 * @return	Returns whether this is a line of text.
	 */
	bool isText() {
		return true;
	}

	/**
	 * Returns the @ref AssociationWidget this object is related to.
	 *
	 * @return	The AssociationWidget this object is related to.
	 */
	AssociationWidget * getAssoc() const {
		return m_pAssoc;
	}

	/**
	 * Activate the FloatingText after the saved data has been loaded
	 *
	 * @param ChangeLog	Pointer to the IDChangeLog.
	 * @return	True if activation was successful.
	 */
	virtual bool activate( IDChangeLog* ChangeLog = 0 );

	/**
	 * Sets the role type of this FloatingText.
	 *
	 * @param role		The Text_Role of this FloatingText.
	 */
	void setRole(Text_Role role);

	/**
	 * Return the role of the text widget
	 *
	 * @return	The Text_Role of this FloatingText.
	 */
	Uml::Text_Role getRole() const;

	/**
	 * For a text to be valid it must be non-empty, i.e. have a length
	 * larger that zero, and have at least one non whitespace character.
	 *
	 * @param text	The string to analyze.
	 * @return	True if the given text is valid.
	 */
	static bool isTextValid(QString text);

	/**
	 * Returns true if we are dealing with an association related role.
	 *
	 * @return	True if this FloatingText plays an association role.
	 */
	bool playsAssocRole() const;

	/**
	 * Returns true if we are dealing with a message related role.
	 *
	 * @return	True if this FloatingText plays a message role.
	 */
	bool playsMessageRole() const;

	/**
	 * Calculates the size of the widget.
	 */
	void calculateSize();

	/**
	 * Overrides default method
	 */
	void draw(QPainter & p, int offsetX, int offsetY);

	/**
	 * Sets the state of whether the widget is selected.
	 *
	 * @param _select	The state of whether the widget is selected.
	 */
	void setSelected(bool _select);

	/**
	 * Sets the text for this label if it is acting as a sequence
	 * diagram message or a collaboration diagram message.
	 */
	void setMessageText();

	/**
	 * Creates the <UML:FloatingTextWidget> XMI element.
	 */
	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the <UML:FloatingTextWidget> XMI element.
	 */
	bool loadFromXMI( QDomElement & qElement );
public slots:
	/**
	 * Called when a menu selection has been made.
	 * This method is public due to called by @ref MessageWidget
	 * when this is text for a @ref MessageWidget.
	 *
	 * @param sel		The selection that has been made.
	 */
	void slotMenuSelection(int sel);	

protected:
	// Data loaded/saved:

	// These strings describe the text that the widget will display.

	/// Prepended text (such as for scope of association Role or method)
	QString m_PreText;
	/// Main text body.
	QString m_Text;
	/** Ending text (such as bracket on changability notation for
	 * association Role)
	 */
	QString m_PostText;

	/**
	 * The role the text widget will enact.
	 */
	Uml::Text_Role m_Role;

private:
	/**
	 * Initializes key variables of the class.
	 */
	void init();
	
	/**
	 * The association it may be linked to.
	 */
	AssociationWidget * m_pAssoc;

	/**
	 * The message widget it may be linked to.
	 */
	MessageWidget * m_pMessage;

	/**
	 * Override default method
	 */
	void moveEvent(QMoveEvent* /*m*/);

	/**
	 * Override default method
	 */
	void resizeEvent(QResizeEvent* /*re*/);

	/**
	 * Shows an operation dialog box.
	 */
	void showOpDlg();

	/**
	 * Override default method and keeps position in sync with
	 * a sequence diagram message if appropriate
	 */
	void mouseMoveEvent(QMouseEvent* me);

	/**
	 * Handle the ListPopupMenu::mt_Rename case of the slotMenuSelection.
	 * Given an own method because it requires rather lengthy code.
	 */
	void handleRename();

};

#endif
