 /*
  *  copyright (C) 2002-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NOTEWIDGET_H
#define NOTEWIDGET_H

//app includes
#include "umlwidget.h"

// forward declarations

// Qt forward declarations
class QPainter;
class QTextEdit;

/**
 * Displays a note box to allow multiple lines of text to be displayed.
 * These widgets are diagram specific.  They will still need a unique id
 * from the @ref UMLDoc class for deletion and other purposes.
 *
 * @short Displays a note box.
 * @author Paul Hensgen	<phensgen@techie.com>
 * @see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class NoteWidget : public UMLWidget {
public:

	/**
	 * Constructs a NoteWidget.
	 *
	 * @param view		The parent to this widget.
	 * @param id		The unique id of the widget.
	 *			The default (-1) will prompt a new ID.
	 */
	NoteWidget(UMLView * view, Uml::IDType id = Uml::id_None );

	/**
	 * destructor
	 */
	virtual ~NoteWidget();

	/**
	 * Overrides the standard function.
	 */
	void mousePressEvent(QMouseEvent *me);

	/**
	 * Overrides the standard function.
	 */
	void mouseMoveEvent(QMouseEvent *me);

	/**
	 * Overrides the standard operation.
	 */
	void mouseReleaseEvent(QMouseEvent * me);

	/**
	 * Overrides the standard operation.
	 */
	void mouseDoubleClickEvent(QMouseEvent * /*me*/);

	/**
	 * Returns the text in the box.
	 *
	 * @return	The text in the box.
	 */
	QString getDoc() const;

	/**
	 * Sets the note documentation.
	 *
	 * @param newText	The text to set the documentation to.
	 */
	void setDoc(const QString &newText);

	/**
	 * Read property of bool m_bLinkDocumentation.
	 */
	bool getLinkState() const {
		return m_bLinkDocumentation;
	}

	/**
	 * Write property of bool m_bLinkDocumentation.
	 */
	void setLinkState( bool linkDocumentation ) {
		m_bLinkDocumentation = linkDocumentation;
	}

	/**
	 * Activate the NoteWidget after the saved data has been loaded.
	 */
	virtual bool activate( IDChangeLog* ChangeLog = 0 );

	/**
	 * Override default method.
	 */
	void draw(QPainter & p, int offsetX, int offsetY);

	/**
	 * Override method from UMLWidget.
	 */
	void setX(int x);

	/**
	 * Override method from UMLWidget.
	 */
	void setY(int y);

	/**
	 * Saves to the <notewidget> XMI element.
	 */
	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads a <notewidget> XMI element.
	 */
	bool loadFromXMI( QDomElement & qElement );
public slots:
	void slotMenuSelection(int sel);	

protected:
	// Data loaded/saved
	bool m_bLinkDocumentation;

	/**
	 * Draws the text.  Called by draw and not called by anyone else.
	 */
	void drawText(QPainter & p, int offsetX, int offsetY);
private:
	/**
	 * Initializes key variables for the class.
	 */
	void init();

	void setEditorGeometry();
	QTextEdit *m_pEditor;
};

#endif
