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

class UMLView;
class ObjectWidget;

/**
 * @short Widget class for graphical representation of sequence lines
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class SeqLineWidget : public QCanvasLine {
public:
	/**
	 * Constructor.
	 */
	SeqLineWidget( UMLView * pView, ObjectWidget * pObject );

	/**
	 * Destructor.
	 */
	~SeqLineWidget();

	/**
	 * Return whether on seq. line.
	 * Takes into account destruction box if shown.
	 *
	 * @param p	The point to investigate.
	 * @return	True if point is on this sequence line.
	 */
	bool onWidget( const QPoint & p );

	/**
	 * Clean up anything before deletion.
	 */
	void cleanup();

	/**
	 * Set up destruction box.
	 */
	void setupDestructionBox();

	/**
	 * Set the start point of the line.
	 *
	 * @param startX	X coordinate of the start point.
	 * @param startY	Y coordinate of the start point.
	 */
	void setStartPoint( int startX, int startY );

	/**
	 * Sets the length of the line.
	 *
	 * @param nLength	New length of the line.
	 */
	void setLineLength( int nLength );

	/**
	 * Gets the length of the line.
	 *
	 * @return	Length of the line.
	 */
	int getLineLength() {
		return m_nLengthY;
	}

	/**
	 * Returns the @ref ObjectWidget associated with this sequence line.
	 *
	 * @return	Pointer to the associated ObjectWidget.
	 */
	ObjectWidget * getObjectWidget() {
		return m_pObject;
	}

	/**
	 * Sets the y position of the bottom of the vertical line.
	 *
	 * @param yPosition	The y coordinate for the bottom of the line.
	 */
	void setEndOfLine(int yPosition);

protected:
	/**
	 * Clean up destruction box.
	 */
	void cleanupDestructionBox();

	/**
	 * Move destruction box.
	 */
	void moveDestructionBox();

	/**
	 * ObjectWidget associated with this sequence line.
	 */
	ObjectWidget * m_pObject;

	/**
	 * View displayed on.
	 */
	UMLView * m_pView;

	/// The destruction box.
	struct {
		QCanvasRectangle * rect;
		QCanvasLine * line1;
		QCanvasLine * line2;
	} m_pDestructionBox;

	/**
	 * The length of the line.
	 */
	int m_nLengthY;

	int m_nOffsetY, m_nOldY, m_nMinY;

	/**
	 * Margin used for mouse clicks.
	 */
	static int const m_nMouseDownEpsilonX;
};

#endif
