/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLWIDGET_H
#define UMLWIDGET_H
#include <qobject.h>
#include <qstring.h>
#include <qcanvas.h>

#include "umlnamespace.h"
#include "umlwidgetdata.h"

class AssociationWidget;
class UMLObject;
class UMLView;
class ListPopupMenu;
class IDChangeLog;

class QPainter;

using namespace Uml;

/**
 * This is the base class for nearly all graphical widgets.
 *
 * @short	The base class for graphical UML objects.
 * @author 	Paul Hensgen <phensgen@techie.com>
 * @version 	1.0
 */
class UMLWidget :  public QObject, public QCanvasRectangle {
	Q_OBJECT

	friend class UMLView;

public:
	/**
	 * Creates a UMLWidget object.
	 *
	 * @param view	The view to be displayed on.
	 * @param o	The UMLObject to represent.
	 * @param pData	The UMLWidgetData for this UMLWidget, it must not be null
	 */
	UMLWidget( UMLView * view, UMLObject * o, UMLWidgetData * pData );

	/**
	 * Creates a UMLWidget object.
	 *
	 * @param view	The view to be displayed on.
	 * @param id	The id of the widget.
	 * @param pData	The UMLWidgetData for this UMLWidget, it must not be null
	 */
	UMLWidget( UMLView * view, int id,  UMLWidgetData * pData);

	/**
	 * Creates a UMLWidget object.
	 *
	 * @param view	The view to be displayed on.
	 * @param pData	The UMLWidgetData for this UMLWidget, it must not be null
	 */
	UMLWidget( UMLView * view,  UMLWidgetData * pData );

	/**
	 * Standard deconstructor
	 */
	~UMLWidget();

	/**
	 * Returns the type of object this represents.
	 *
	 * @return	Returns the type of object this represents.
	 */
	virtual Uml::UMLWidget_Type getBaseType();

	/**
	 * Returns the ID of this object.
	 *
	 * @return	Returns the ID this object.
	 */
	virtual int getID();

	/**
	 * Sets the id og the UMLWidget.
	 *
	 * @param id The id to set the widget to.
	 */
	virtual void setID(int Id) {
		m_pData->m_nId = Id;
	}

	/**
	 * Adds one to the amount of association this object is connected to.
	 *
	 * @param pAssoc The association to be added.
	 */
	void addAssoc(AssociationWidget* pAssoc) {
		m_pData->addAssoc(pAssoc);
	}

	/**
	 * Removes one from the count of the amount of associations this object
	 * is connected to.
	 *
	 * @param pAssoc The association to be deleted.
	 */
	void removeAssoc(AssociationWidget* pAssoc) {
		m_pData->removeAssoc(pAssoc);
	}

	/**
	 * Returns the @ref UMLObject set to represent.
	 * 
	 * @return the UMLObject to represent.
	 */
	UMLObject * getUMLObject() {
		return m_pObject;
	}

	/**
	 * Sets the @ref UMLObject to represent.
	 *
	 * @param o	The object to represent.
	 */
	virtual void setUMLObject(UMLObject * o) {
		m_pObject = o;
	}

	/**
	 * Overrides the standard operation.
	 *
	 * @param me The mouse event.
	 */
	virtual void mouseReleaseEvent(QMouseEvent * me);

	/**
	 * Overrides the standard operation.
	 *
	 * @param me The mouse event.
	 */
	virtual void mouseDoubleClickEvent(QMouseEvent *me);

	/**
	 * Return the status of using fill color.
	 *
	 * @return	Return the status of using fill color.
	 */
	bool getUseFillColor() {
		return m_pData->m_bUseFillColor;
	}

	/**
	 * Return the background color.
	 *
	 * @return	Return the background color.
	 */
	QColor getFillColour();

	/**
	 * Return the line color.
	 *
	 * @return Return the line color.
	 */
	QColor getLineColor();

	/**
	 * Set the status of using fill color.
	 *
	 * @param	fc the status of using fill color.
	 */
	void setUseFillColor(bool fc);

	/**
	 * Sets the line colour
	 *
	 * @param colour the new line colour
	 */
	void setLineColour(QColor colour);

	/**
	 * Sets the background fill colour
	 *
	 * @param colour the new fill colour
	 */
	void setFillColour(QColor colour);

	/**
	 * Overrides the standard operation.
	 * Any code specific to this type of widget is here after it
	 * has called doMouseMove()
	 *
	 * @param me
	 */
	virtual void mouseMoveEvent(QMouseEvent* me);

	/**
	 * Moves the item to it's new position (called from mouseMoveEvent)
	 *
	 * @param me
	 */
	QPoint doMouseMove(QMouseEvent* me);

	/**
	 * Returns wether this is a line of text.
	 * Used for transparency in printing.
	 *
	 * @return always false
	 */
	virtual bool isText() {
		return false;
	}

	/**
	 * Sets the state of whether the widget is selected.
	 *
	 * @param _select The state of whether the widget is selected.
	 */
	virtual void setSelected(bool _select);

	/**
	 * Returns the state of whether the widget is selected.
	 *
	 * @return Returns the state of whether the widget is selected.
	 */
	bool getSelected() {
		return m_bSelected;
	}

	/**
	 * Sets the view the widget is on.
	 *
	 * @param v The view the widget is on.
	 */
	void setView(UMLView * v);

	/**
	 * Returns the old id of the widget.  Used for copy/cut operations.
	 *
	 * @return Returns the old id of the widget.
	 */
	virtual int getOldID() {
		return m_nOldID;
	}

	/**
	 * Sets the old id of the widget.  Used for copy/cut operations.
	 *
	 * @param _id Sets the old id of the widget.
	 */
	virtual void setOldID(int _id) {
		m_nOldID = _id;
	}

	/**
	 * Activate the object after serializing it from a QDataStream
	 *
	 * @param ChangeLog
	 * 
	 * @return 
	 */
	virtual bool activate(IDChangeLog* ChangeLog = 0);

	/**
	 * Returns true if the given point is in the boundaries of the widget
	 *
	 * @param p Point to be checked.
	 * 
	 * @return
	 */
	virtual bool onWidget(const QPoint & p);

	/**
	 * Draws the UMLWidget on the given paint device
	 *
	 * @param p The painter for the drawing device
	 * @param offsetX x position to start the drawing.
	 * @param offsetY y position to start the drawing.
	 *
	 */
	virtual void draw( QPainter & p, int offsetX, int offsetY );

	/**
	 * Sets the font the widget is to use.
	 * 
	 * @param font Font to be set.
	 */

	virtual void setFont( QFont font ) {
		m_pData -> setFont( font );
		calculateSize();
		update();
	}

	/**
	 * Returns the font the widget is too use.
	 *
	 * @return The current font set.
	 */
	virtual QFont getFont() {
		return m_pData -> getFont();
	}

	/**
	 * Returns whether we triggered the update of position movement.
	 * If so, you probably don't want to move it.
	 *
	 * @return The moving state.
	 */
	bool getStartMove() {
		return m_bStartMove;
	}

	/**
	 * Sets the x-coordinate.
	 * 
	 * @param x The x-coordinate to be set.
	 */
	void setX( int x );

	/**
	 * Sets the y-coordinate.
	 *
	 * @param y The y-coordinate to be set.
	 */
	void setY( int y );

	/** 
	 * Sets the size.
	 * If m_pView->getSnapComponentSizeToGrid() is true, then 
	 * set the next larger size that snaps to the grid.
	 */
	void setSize(int width,int height);

	/**
	 * Used by some child classes to get documentation.
	 *
	 * @return Always an empty string.
	 */
	virtual QString getDoc() {
		return "";
	}

	/**
	 * Used by some child classes to set documentation.
	 *
	 * @param doc The documentation to be set. Will be ignored.
	 */
	virtual void setDoc( QString ) { }
 

	/**
	 * Returns true if the Activate method has been called for this instance
	 *
	 * @return The activate status.
	 */
	virtual const bool& isActivated();

	/**
	 * Return a UMLWidgetData Containing all information about this instance of UMLWidget
	 *
	 * @return The widget data represting the UML object.
	 */
	virtual UMLWidgetData* getData();

	/**
	 * Synchronize the Widget's m_pData member with its display properties, for example:
	 * the X and Y positions of the widget, etc.
	 */
	virtual void synchronizeData();

	/**
	 * Sets the m_Name property
	 *
	 * @param strName The name to be set.
	 */
	virtual void setName(QString strName);

	/**
	 * Returns a copy of m_Name
	 *
	 * @return The currently set name.
	 */
	virtual QString getName() {
		return m_Name;
	}

	/**
	 * Starts the popup menu.
	 *
	 * @param At The Point where the diagram is to be coming up.
	 */
	void startPopupMenu(QPoint At);

	/**
	 * Adjusts associations with the given co-ordinates
	 *
	 * @param x The x-coordinate.
	 * @param y The y-coordinate.
	 */
	void adjustAssocs(int x, int y);

	/**
	 * Set the m_bActivated flag of a widget but does not perform the Activate method
	 *
	 * @param Active Status of activation is to be set.
	 */
	void setActivated(bool Active = true);

	/**
	 * Used to cleanup any other widget it may need to delete.
	 * Used by child classes.  This should be called before deleting a widget of a diagram.
	 */
	virtual void cleanup();

	/**
	 * Returns whether the widget type has an associated UMLObject
	 */
	static bool widgetHasUMLObject(Uml::UMLWidget_Type type);

	/**
	 * Update the size of this widget.
	 */
	void updateComponentSize();

protected:
	/**
	 * Draws that the widget is selected.
	 *
	 * @param p Device on which is the selection is to be drawn.
	 * @param offsetX The x-coordinate for drawing.
	 * @param offsetY The y-coordinate for drawing.
	 */
	virtual void drawSelected(QPainter * p, int offsetX, int offsetY);

	/**
	 * Overrides default method.
	 * 
	 * @param p Device on which the shape has to be drawn.^
	 */
	virtual void drawShape(QPainter &p );

	/**
	 * Overrides the standard operation.
	 * 
	 * @param me The mouse event.
	 */
	virtual void mousePressEvent(QMouseEvent *me);

	/**
	 * Overrides the standard operation.
	 * 
	 * @param me The move event.
	 */
	virtual void moveEvent(QMoveEvent *) { } 

	/**
	 * Calculates the size of the widget.
	 */
	virtual void calculateSize() {}

	/**
	 * Initializes key attributes of the class.
	 */
	void init();

	bool 		m_bMouseDown, 
			m_bMouseOver, 
			m_bSelected, 
			m_bStartMove;
	
	/**
	 * It is true, if the object was moved during mouseMoveEvent
	 */
	bool m_bMoved;

	/**
	 * It is true, if the shift key was pressed during mousePressEvent
	 */
	bool m_bShiftPressed;

	int  		m_nOldX, 
			m_nOldY, 
			m_nPosX, 
			m_nOldID;
	UMLObject 	*m_pObject;
	UMLView 	*m_pView;
	ListPopupMenu 	*m_pMenu;
	bool 		m_bResizing;
	UMLWidgetData	*m_pData;
	int 		m_nPressOffsetX, 
			m_nPressOffsetY;
	int 		m_nOldH, 
			m_nOldW;

	/**
	 * It is true if the Activate Function has been called for this
	 * class instance
	 */
	bool m_bActivated;

	/**
	 * UMLWidget's name property
	 */
	QString m_Name;

	/**
	 * Change Widget Behaviour
	 */
	bool m_bIgnoreSnapToGrid;
	bool m_bIgnoreSnapComponentSizeToGrid;

public slots:

	/**
	 * This slot is entered when an event has occurred on the views display,
	 * most likely a mouse event.  Before it sends out that mouse event all
	 * children should make sure that they don't have a menu active or there
	 * could be more than one popup menu displayed.
	 */
	virtual void slotRemovePopupMenu();

	/**
	 * When a widget changes this slot captures that signal.
	 *
	 * @param o The changed UMLobject
	 */
	virtual void updateWidget();
	

	/**
	 * Captures any popup menu signals for menus it created.
	 *
	 * @param sel The command which has to be executed.
	 */
	virtual void slotMenuSelection(int sel);

	/**
	 * Captures when another widget moves if it is link to it that signal.
	 *
	 * @param id The id of object behind the widget.
	 */
	virtual void slotWidgetMoved(int id);

	/**
	 * Captures a color change signal.
	 *
	 * @param viewID The id of the object behind the widget.
	 */
	virtual void slotColorChanged(int viewID);

	/**
	 *   Captures a sigClearAllSelected signal sent by @ref UMLView
	 */
	void slotClearAllSelected();

	/**
	 * Tells the widget to snap to grid.
	 * Will use the grid settings of the @ref UMLView it belongs to.
	 */
	void slotSnapToGrid();

signals:
	/**
	 * Emit when the widget moves its' position.
	 *
	 * @param id The id of the object behind the widget.
	 */
	void sigWidgetMoved(int id);
};

#endif
