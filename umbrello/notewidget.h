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
#include "notewidgetdata.h"

// forward declarations

// Qt forward declarations
class QPainter;

/**
 *	Displays a note box to allow multiple lines of text to be displayed.
 *	These widgets are diagram specific.  They will still need a unique id
 *	from the @ref UMLDoc class for deletion and other purposes.
 *
 *	@short	Displays a note box.
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@version	1.0
 *	@see	UMLWidget
 */
class NoteWidget : public UMLWidget {
public:
	/**
	 *	Constructs a NoteWidget.
	 *
	 *	@param	view		The parent to this widget.
	 *	@param	pData		The NoteWidget to represent.
	 */
	NoteWidget(UMLView * view, UMLWidgetData* pData);

	/**
	 *	Constructs a NoteWidget.
	 *
	 *	@param	view		The parent to this widget.
	 *	@param	id				The unique id of the widget.
	 */
	NoteWidget(UMLView * view, int id);

	/**
	 *	Constructs a NoteWidget.
	 *
	 *	@param	view 		The parent to this widget.
	 */
	NoteWidget(UMLView * view);

	/**
	 *	Initializes key variables for the class.
	 */
	void init();

	/**
	 *	Standard deconstructor.
	 */
	~NoteWidget();

	/**
	 *	Overrides the standard function.
	 */
	void mousePressEvent(QMouseEvent *me);

	/**
	 *	Overrides the standard function.
	 */
	void mouseMoveEvent(QMouseEvent *me);

	/**
	 *	Overrides the standard operation.
	 */
	void mouseReleaseEvent(QMouseEvent * me);

	/**
	 *		Overrides the standard operation.
	 */
	void mouseDoubleClickEvent(QMouseEvent * /*me*/);

	/**
	 *	Returns the text in the box.
	 *
	 *	@return Returns the text in the box.
	 */
	QString getDoc() {
		return ((NoteWidgetData*)m_pData) -> getText();
	}

	/**
	 *   Sets the note documentation.
	 */
	void setDoc(QString newText) {
		((NoteWidgetData*)m_pData)->setText(newText);
	}

	bool getLinkState() {
		return ((NoteWidgetData*)m_pData)->getLinkDocumentation();
	}

	/**
	 *Activate the NoteWidget after the saved data has been loaded
	 */
	virtual bool activate( IDChangeLog* ChangeLog = 0 );

	/**
	 * Synchronize the Widget's m_pData member with its display properties, for example:
	 * the X and Y positions of the widget, etc
	 */
	virtual void synchronizeData();

	/**
	 * Override default method
	 */
	void draw(QPainter & p, int offsetX, int offsetY);

protected:
	/**
	 * Draws the text.  Called by draw and not called by anyone else.
	 */
	void drawText(QPainter & p, int offsetX, int offsetY);
public slots:
	void slotMenuSelection(int sel);
};

#endif
