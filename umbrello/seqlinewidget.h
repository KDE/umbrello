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
 */
class SeqLineWidget : public QCanvasLine {
	friend class ObjectWidget;
public:
	/**
	*		Constructor
	*/
	SeqLineWidget( UMLView * pView, ObjectWidget * pObject );

	/**
	*		Deconstructor
	*/
	~SeqLineWidget();

	/**
	*		Return whether on seq. line.
	*		Takes into account destruction box if shown.
	*/
	bool onWidget( const QPoint & p );

	/**
	*		Mouse moved when pressing on widget
	*/
	void mouseMoveEvent( QMouseEvent *me );

	/**
	*		Mouse release on widget
	*/
	void mouseReleaseEvent( QMouseEvent * me );

	/**
	*		Mouse press on widget
	*/
	void mousePressEvent( QMouseEvent * me );

	/**
	*		Cleanup anything before deletion
	*/
	void cleanup();

	/**
	*		Setup decon box.
	*/
	void setupDeconBox();

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
	*		Return whether mouse down, meaning a move event is allowed.
	*/
	bool getMouseDown() {
		return m_bMouseDown;
	}
protected:
	/**
	*		Cleanup decon. box.
	*/
	void cleanupDeconBox();

	/**
	*		Move decon box.
	*/
	void moveDeconBox();

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
	}    m_pDeconBox;

	int m_nLengthY;

	bool m_bMouseDown;

	int m_nOffsetY, m_nOldY, m_nMinY;
};

#endif
