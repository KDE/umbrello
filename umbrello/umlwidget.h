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
#include <qdom.h>
#include <qdatetime.h>

#include "umlnamespace.h"
#include "associationwidgetlist.h"
#include "dialogs/settingsdlg.h"

class UMLObject;
class UMLView;
class ListPopupMenu;
class IDChangeLog;

class QPainter;
class QFont;
class QFontMetrics;

using namespace Uml;

/**
 * This is the base class for nearly all graphical widgets.
 *
 * @short	The base class for graphical UML objects.
 * @author 	Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLWidget : public QObject, public QCanvasRectangle {
	Q_OBJECT
public:
	/**
	 * Creates a UMLWidget object.
	 *
	 * @param view	The view to be displayed on.
	 * @param o	The UMLObject to represent.
	 */
	UMLWidget( UMLView * view, UMLObject * o );

	/**
	 * Constructor
	 *
	 * @param view		The view to be displayed on.
	 * @param o		The UMLObject to represent.
	 * @param optionState	The optionState of the diagram.  Used to set the initial colours.
	 */
	UMLWidget( UMLView * view, UMLObject * o, SettingsDlg::OptionState optionState );

	/**
	 * Creates a UMLWidget object.
	 *
	 * @param view	The view to be displayed on.
	 * @param id	The id of the widget.
	 */
	UMLWidget( UMLView * view, int id = -1 );

	/**
	 * Constructor
	 *
	 * @param view		The view to be displayed on.
	 * @param optionState	The optionState of the diagram.  Used to set the initial colours.
	 */
	UMLWidget( UMLView * view, SettingsDlg::OptionState optionState );

	/**
	 * Standard deconstructor
	 */
	virtual ~UMLWidget();

	/**
	 * Copy constructor
	 */
	UMLWidget(const UMLWidget& other);

	/**
	 * Assignment operator
	 */
	virtual UMLWidget& operator=(const UMLWidget& other);

	/**
	 * Overload '==' operator
	 */
	virtual bool operator==(const UMLWidget& other);

	/**
	* Write property of m_Type.
	*/
	void setBaseType( UMLWidget_Type type ) {
		m_Type = type;
	}

	/**
	* Read property of m_Type.
	*/
	UMLWidget_Type getBaseType() const {
		return m_Type;
	}

	/**
	* Write property of m_nId.
	*/
	void setID( int id );

	/**
	* Read property of m_nId.
	*/
	int getID() const;

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
	 * Set the status of using fill color.
	 *
	 * @param	fc the status of using fill color.
	 */
	void setUseFillColour(bool fc);

	/**
	 * Read property of bool m_bUseFillColour.
	 */
	bool getUseFillColour() const {
		return m_bUseFillColour;
	}

	/**
	 * Sets the line colour
	 *
	 * @param colour the new line colour
	 */
	void setLineColour(QColor colour);

	/**
	 * Read property of QColor m_LineColour.
	 */
	QColor getLineColour() const {
		return m_LineColour;
	}

	/**
	 * Sets the background fill colour
	 *
	 * @param colour the new fill colour
	 */
	void setFillColour(QColor colour);

	/**
	 * Read property of QColor m_FillColour.
	 */
	QColor getFillColour() const {
		return m_FillColour;
	}

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
	 * Returns whether this is a line of text.
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
	bool getSelected() const {
		return m_bSelected;
	}

	void setSelectedFlag(bool _select) {
		m_bSelected = _select;
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
	virtual int getOldID() const {
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
	virtual void setFont( QFont font );

	/**
	 *  Returns the font the widget is to use.
	 */
	virtual QFont getFont() const {
		return m_Font;
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
	 * Gets the x-coordinate.
	 */
	int getX() const {
		return (int)QCanvasItem::x();
	}

	/**
	 * Gets the y-coordinate.
	 */
	int getY() const {
		return (int)QCanvasItem::y();
	}

	/**
	 * Returns the height of widget.
	 */
	int getHeight() const {
		return QCanvasRectangle::height();
	}

	/**
	 * Returns the width of the widget.
	 */
	int getWidth() const {
		return QCanvasRectangle::width();
	}

	/**
	 * Sets the size.
	 * If m_pView->getSnapComponentSizeToGrid() is true, then
	 * set the next larger size that snaps to the grid.
	 */
	void setSize(int width,int height);

	/**
	 * Used by some child classes to get documentation.
	 *
	 * @return	The documentation from the UMLObject (if m_pObject is set.)
	 */
	virtual QString getDoc() const;

	/**
	 * Used by some child classes to set documentation.
	 *
	 * @param doc The documentation to be set in the UMLObject (if m_pObject is set.)
	 */
	virtual void setDoc( QString doc );

	/**
	 * Removes an already created association from the list of
	 * associations that include this UMLWidget
	 */
	void removeAssoc(AssociationWidget* pAssoc);

	/**
	 * Adds an already created association to the list of
	 * associations that include this UMLWidget
	 */
	void addAssoc(AssociationWidget* pAssoc);

	/**
	 *  Returns the list of associations connected to this widget.
	 */
	AssociationWidgetList & getAssocList() {
		return m_Assocs;
	}

	/**
	 * Returns m_bUsesDiagramFillColour
	 */
	bool getUsesDiagramFillColour() const {
		return m_bUsesDiagramFillColour;
	}

	/**
	 * Returns m_bUsesDiagramLineColour
	 */
	bool getUsesDiagramLineColour() const {
		return m_bUsesDiagramLineColour;
	}

	/**
	 * Returns m_bUsesDiagramUseFillColour
	 */
	bool getUsesDiagramUseFillColour() const {
		return m_bUsesDiagramUseFillColour;
	}

	/**
	 * Sets m_bUsesDiagramFillColour
	 */
	void setUsesDiagramFillColour(bool usesDiagramFillColour) {
		m_bUsesDiagramFillColour = usesDiagramFillColour;
	}

	/**
	 * Sets m_bUsesDiagramLineColour
	 */
	void setUsesDiagramLineColour(bool usesDiagramLineColour) {
		m_bUsesDiagramLineColour = usesDiagramLineColour;
	}

	/**
	 * Sets m_bUsesDiagramUseFillColour
	 */
	void setUsesDiagramUseFillColour(bool usesDiagramUseFillColour) {
		m_bUsesDiagramUseFillColour = usesDiagramUseFillColour;
	}

	/**
	 * Write property of bool m_bIsInstance
	 */
	void setIsInstance(bool isInstance) {
		m_bIsInstance = isInstance;
	}

	/**
	 * Read property of bool m_bIsInstance
	 */
	bool getIsInstance() const {
		return m_bIsInstance;
	}

	/**
	 * Write property of m_instanceName
	 */
	void setInstanceName(QString instanceName) {
		m_instanceName = instanceName;
	}

	/**
	 * Read property of m_instanceName
	 */
	QString getInstanceName() const {
		return m_instanceName;
	}

	virtual bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	virtual bool loadFromXMI( QDomElement & qElement );

	/**
	 * Returns true if the Activate method has been called for this instance
	 *
	 * @return The activate status.
	 */
	bool isActivated();

	/**
	 * Sets the name in the corresponding UMLObject.
	 * No-op if m_pObject is NULL.
	 *
	 * @param strName The name to be set.
	 */
	virtual void setName(QString strName);

	/**
	 * Gets the name from the corresponding UMLObject.
	 * Returns an empty string if m_pObject is NULL.
	 *
	 * @return The currently set name.
	 */
	virtual QString getName() const;

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
	 * Adjusts all unselected associations with the given co-ordinates
	 *
	 * @param x The x-coordinate.
	 * @param y The y-coordinate.
	 */
	void adjustUnselectedAssocs(int x, int y); 

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

	/**
	 * @note For performance Reasons, only FontMetrics for already used
	 *  font types are updated. Not yet used font types will not get a font metric
	 *  and will get the same font metric as if painter was zero.
	 *  This behaviour is acceptable, because diagrams will always be showed on Display
	 *  first before a special painter like a printer device is used.
	 */
	void forceUpdateFontMetrics(QPainter *painter);

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

protected:
	/**
	 * Draws that the widget is selected.
	 *
	 * @param p Device on which is the selection is to be drawn.
	 * @param offsetX The x-coordinate for drawing.
	 * @param offsetY The y-coordinate for drawing.
	 */
	virtual void drawSelected(QPainter * p, int offsetX, int offsetY, bool resizeable = false);

	/**
	 * Overrides default method.
	 *
	 * @param p Device on which the shape has to be drawn.^
	 */
	virtual void drawShape(QPainter &p );

	/**
	 * Calculates the size of the widget.
	 */
	virtual void calculateSize() {}

	typedef enum {
		FT_NORMAL = 0,
		FT_BOLD  = 1,
		FT_ITALIC = 2,
		FT_UNDERLINE = 3,
		FT_BOLD_ITALIC = 4,
		FT_BOLD_UNDERLINE = 5,
		FT_ITALIC_UNDERLINE = 6,
		FT_BOLD_ITALIC_UNDERLINE = 7,
		FT_INVALID = 8
	} FontType;

	/** Template Method, override this to set the default
	 *  font metric.
	 */
	virtual void setDefaultFontMetrics(UMLWidget::FontType fontType);
	virtual void setDefaultFontMetrics(UMLWidget::FontType fontType, QPainter &painter);

	/** Returns the font metric used by this object for Text which uses bold/italic fonts*/
	QFontMetrics &getFontMetrics(UMLWidget::FontType fontType);
	/** set the font metric to use */
	void setFontMetrics(UMLWidget::FontType fontType, QFontMetrics &fm);
	void setFontMetrics(UMLWidget::FontType fontType, QFontMetrics fm);
	void setupFontType(QFont &font, UMLWidget::FontType fontType);
	/**
	 * Initializes key attributes of the class.
	 */
	void init();

	///////////////// Data Loaded/Saved /////////////////////////////////

	/**
	 * Type of widget.
	 */
	UMLWidget_Type m_Type;

	/**
	 * This ID is only used when the UMLWidget does not have a
	 * corresponding UMLObject (i.e. the m_pObject pointer is NULL.)
	 * For UML objects, the ID from the UMLObject is used.
	 */
	int m_nId;

	/**
	 * This flag indicates if the UMLWidget uses the Diagram FillColour
	 */
	bool m_bUseFillColour;

	/**
	 *  true by default, false if the colours have
	 *  been explicity set for this widget
	 */
	bool m_bUsesDiagramFillColour, m_bUsesDiagramLineColour, m_bUsesDiagramUseFillColour;

	/**
	 * Color of the lines of the widget
	 */
	QColor m_LineColour;

	/**
	 * Color of the background of the widget
	 */
	QColor m_FillColour;

	/**
	 * A list of AssociationWidgets between the UMLWidget and other UMLWidgets in the diagram
	 */
	AssociationWidgetList m_Assocs;

	/**
	 * 	The font the widget will use.
	 */
	QFont m_Font;

	/**
	 * Holds whether this widget is a component instance (i.e. on a deployment diagram)
	 */
	bool m_bIsInstance;

	/**
	 * The instance name (used if on a deployment diagram)
	 */
	QString m_instanceName;

	///////////////// End of Data Loaded/Saved //////////////////////////

	bool 		m_bMouseDown,
			m_bMouseOver,
			m_bSelected,
			m_bStartMove;

	/**
	 * True if the object was moved during mouseMoveEvent
	 */
	bool m_bMoved;

	/**
	 * True if the shift key was pressed during mousePressEvent
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
	int 		m_nPressOffsetX,
			m_nPressOffsetY;
	int 		m_nOldH,
			m_nOldW;
	QFontMetrics	*m_pFontMetrics[FT_INVALID];

	/**
	 * It is true if the Activate Function has been called for this
	 * class instance
	 */
	bool m_bActivated;

	/**
	 * Change Widget Behaviour
	 */
	bool m_bIgnoreSnapToGrid;
	bool m_bIgnoreSnapComponentSizeToGrid;

	/**
	 * Timer that prevents excessive updates (be easy on the CPU)
	 */
	QTime lastUpdate;

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
