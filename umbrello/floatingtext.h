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
 * This is a multipurpose class.  In its simplest form it will display a line of text.
 * It can also be setup to be the text for an operation with regard to the @ref MessageWidget
 * on the sequence diagram.  It is also used for the text required for an association.
 *
 * The differences between all these different uses will be the popup menu that is
 * associated with it.
 *
 *	@short	Displays a line of text or an operation.
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class FloatingText : public UMLWidget {
public:

	/**
	 * Constructs a FloatingText instance.
	 *
	 * @param	view	The parent of this FloatingText.
	 * @param	role	The role this FloatingText will take up.
	 * @param	text	The main text to display.
	 */
	FloatingText(UMLView * view, Text_Role role, QString text = "");

	/**
	 * Constructs a FloatingText instance.
	 *
	 * @param	view	The parent of this FloatingText.
	 */
	FloatingText(UMLView * view);

	/**
	 * Standard deconstructor.
	 */
	~FloatingText();

	/**
	 * Called to set the position of the FloatingText.
	 *
	 * @param	x	Vertical co-ordinate of the point.
	 * @param	y	Horizontal co-ordinate of the point.
	 */
	void setLinePos(int x, int y);

	/**
	 * Called to set the position of the FloatingText by the difference between the two points
	 */
	void setLinePositionRelatively(int newX, int newY, int oldX, int oldY);

	/**
	 * Set the main body of text to display.
	 *
	 * @param	t	The text to display.
	 */
	void setText(QString t);

	/**
	 * Set some text to be prepended to the main body of text.
	 * @param	t	The text to prepend to main body which is displayed.
	 */
	void setPreText(QString t);

	/**
	 * Set some text to be appended to the main body of text.
	 * @param	t	The text to append to main body which is displayed.
	 */
	void setPostText(QString t);

	/**
	 * Set the sequence number to display.
	 *
	 * @param	sn	The sequence number to display.
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
	 * @param	op	The operation to display.
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
	 * Use to get the _main body_ of text (e.g. prepended and appended text is omitted)
	 * as currently displayed by the widget.
	 * @return	The main text currently being displayed by the widget.
	 */
	QString getText() const;

	/**
	 * Use to get the pre-text which is prepended to the main body of text
	 * to be displayed
	 */
	QString getPreText() const;

	/**
	 * Use to get the post-text which is appended to the main body of text
	 * to be displayed
	 */
	QString getPostText() const;

	/**
	 * Use to get the total text (prepended + main body + appended) currently displayed by the widget.
	 * @return	The text currently being displayed by the widget.
	 */
	QString getDisplayText() const;

	/**
	 * If this object is associated with a @ref Association object then
	 * this method will be called by that association when it thinks this
	 * object has been right clicked on.
	 *
	 * @param	p	The point at which the right button was clicked.
	 */
	void startMenu(AssociationWidget * a, QPoint p);

	/**
	 * Initializes key variables of the class.
	 */
	void init();

	/**
	 * Displays a dialog box to change the text.
	 */
	void changeTextDlg();

	/**
	 * Sets the message to the @ref MessageWidget that this class may represent.
	 *
	 * @param	m	The MessageWidget that may be represented.
	 */
	void setMessage(MessageWidget * m);

	/**
	 * Sets the position relative to the sequence message
	 */
	void setPositionFromMessage();

	/**
	 * Returns the @ref MessageWidget this floating text is related to.
	 *
	 * @return	The @ref MessageWidget this floating text is related to.
	 */
	MessageWidget * getMessage() const {
		return m_pMessage;
	}

	/**
	 * Overrides a method.  Used to pickup double clicks.
	 */
	void mouseDoubleClickEvent(QMouseEvent * /*me*/);

	/**
	 * Sets the assoc to the @ref Association to represent.
	 *
	 * @param	a	The Association to represent.
	 */
	void setAssoc(AssociationWidget * a) {
		m_pAssoc = a;
	}

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
	 * Returns the Association this object is related to.
	 *
	 * @return	Returns the Association this object is related to.
	 */
	AssociationWidget * getAssoc() const {
		return m_pAssoc;
	}

	/**
	 * Activate the FloatingText after the saved data has been loaded
	 */
	virtual bool activate( IDChangeLog* ChangeLog = 0 );

	/**
	 * Sets the role type of this FloatingText
	 */
	void setRole(Text_Role role);

	/**
	 * Return the role of the text widget
	 */
	Uml::Text_Role getRole() const;

	/**
	 * For a text to be valid it must be non-empty, i.e. have a length
	 * larger that zero, and have at least one non whitespace character.
	 */
	static bool isTextValid(QString text);

	/**
	 * Returns true if we are dealing with an association related role.
	 */
	bool playsAssocRole() const;

	/**
	 * Returns true if we are dealing with a message related role.
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

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );

protected:
	// Data loaded/saved:

	/**
	 *  These strings describe the text that the widget will display.
	 */
	QString m_PreText; // prepended text (such as for scope of association Role or method)
	QString m_Text; // main body
	QString m_PostText; // ending text (such as bracket on changability notation for association Role )

	/**
	 *  The role the text widget will enact.
	 */
	Uml::Text_Role m_Role;

private:
	/**
	 *  The association it may be linked to.
	 */
	AssociationWidget * m_pAssoc;

	/**
	 *  The message widget it may be linked to.
	 */
	MessageWidget * m_pMessage;

	/**
	 *  Override default method
	 */
	void moveEvent(QMoveEvent* /*m*/);

	/**
	 *  Override default method
	 */
	void resizeEvent(QResizeEvent* /*re*/);

	/**
	 *  Shows an operation dialog box.
	 */
	void showOpDlg();

	/**
	 *  Override default method and keeps position in sync with
	 *  a sequence diagram message if appropriate
	 */
	void mouseMoveEvent(QMouseEvent* me);

	/**
	 *  Handle the ListPopupMenu::mt_Rename case of the slotMenuSelection.
	 *  Given an own method because it requires rather lengthy code.
	 */
	void handleRename();

public slots:
	/**
	 *  Called when a menu selection has been made.
	 *  This method is public due to called by @ref MessageWidget
	 *  when this is text for a @ref MessageWidget.
	 *
	 *  @param sel		The selection that has been made.
	 */
	void slotMenuSelection(int sel);
};

#endif
