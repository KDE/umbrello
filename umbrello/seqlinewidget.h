/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef SEQLINEWIDGET_H
#define SEQLINEWIDGET_H
//app includes
#include <qcanvas.h>

class UMLView;
class ObjectWidget;

/**
 *@author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class SeqLineWidget : public QCanvasLine {
public:
	/**
	*		Constructor
	*/
	SeqLineWidget( UMLView * pView, ObjectWidget * pObject );

	/**
	*		Destructor
	*/
	~SeqLineWidget();

	/**
	*		Return whether on seq. line.
	*		Takes into account destruction box if shown.
	*/
	bool onWidget( const QPoint & p );

	/**
	*		Cleanup anything before deletion
	*/
	void cleanup();

	/**
	*		Setup destruction box.
	*/
	void setupDestructionBox();

	/**
	*		Set the start point of the line
	*/
	void setStartPoint( int startX, int startY );

	/**
	*		Sets the length of the line
	*/
	void setLineLength( int nLength );

	/**
	*		Sets the length of the line
	*/
	int getLineLength() {
		return m_nLengthY;
	}

	/**
	*		Returns the @ref ObjectWidget associated with
	*/
	ObjectWidget * getObjectWidget() {
		return m_pObject;
	}

	/**
	 * Sets the y position of the bottom of the vertical line
	 *
	 * @param yPosition the y coordinate for the bottom of the line
	 */
	void setEndOfLine(int yPosition);

protected:
	/**
	*		Cleanup destruction. box.
	*/
	void cleanupDestructionBox();

	/**
	*		Move destruction box.
	*/
	void moveDestructionBox();

	/**
	*		ObjectWidget associated with.
	*/
	ObjectWidget * m_pObject;

	/**
	*		View displayed on.
	*/
	UMLView * m_pView;

	struct {
		QCanvasRectangle * rect;
		QCanvasLine * line1;
		QCanvasLine * line2;
	} m_pDestructionBox;

	/**
	 * The length of the line
	 */
	int m_nLengthY;

	int m_nOffsetY, m_nOldY, m_nMinY;

	/**
	 * Margin used for mouse clicks
	 */
	static int const m_nMouseDownEpsilonX;
};

#endif
