/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLVIEW_H
#define UMLVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "umlobjectlist.h"
#include "umlwidgetlist.h"
#include "messagewidgetlist.h"
#include "associationwidgetlist.h"

#include "umlnamespace.h"
#include "worktoolbar.h"
#include "floatingtext.h"
#include "dialogs/settingsdlg.h"

// QT includes
#include <qcanvas.h>

// KDE includes
#include <kurl.h>

// #define OFFSET		50

class ClassOptionsPage;
class IDChangeLog;
class ListPopupMenu;
class SeqLineWidget;
class ObjectWidget;

class UMLListView;
class UMLApp;
class UMLDoc;

class KPrinter;

using namespace Uml;

/** The UMLView class provides the view widget for the UMLApp
 * instance.  The View instance inherits QWidget as a base class and
 * represents the view object of a KTMainWindow. As UMLView is part of
 * the document-view model, it needs a reference to the document
 * object connected with it by the UMLApp class to manipulate and
 * display the document structure provided by the UMLDoc class.
 *
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLView : public QCanvasView {
	Q_OBJECT
public:
	/**
	 * Constructor for the main view
	 */
	UMLView(QWidget* parent, UMLDoc* doc);

	/**
	 * Destructor for the main view
	 */
	~UMLView();

        // Accessors and other methods dealing with loaded/saved data

	/**
	 * Return the documentation of the diagram.
	 */
	QString getDoc() const {
		return m_Documentation;
	}

	/**
	 * Set the documentation of the diagram.
	 */
	void setDoc( QString doc ) {
		m_Documentation = doc;
	}

	/**
	 * Return the name of the diagram.
	 */
	QString getName() const {
		return m_Name;
	}

	/**
	 * Set the name of the diagram.
	 */
	void setName( QString name ) {
		m_Name = name;
	}

	/**
	 * Returns the type of the diagram.
	 */
	Uml::Diagram_Type getType() const {
		return m_Type;
	}

	/**
	 * Set the type of diagram.
	 */
	void setType( Uml::Diagram_Type type ) {
		m_Type = type;
	}

	/**
	 * Returns the fill color to use.
	 */
	QColor getFillColor() const {
		return m_Options.uiState.fillColor;
	}

	/**
	 * Set the background color.
	 *
	 * @param color		The color to use.
	 */
	void setFillColor( QColor color );

	/**
	 * Returns the line color to use.
	 */
	QColor getLineColor() const {
		return m_Options.uiState.lineColor;
	}

	/**
	 * Sets the line color.
	 *
	 * @param color		The color to use.
	 */
	void setLineColor( QColor color );

	/**
	 * Returns the ID of the diagram.
	 */
	int getID() const {
		return m_nID;
	}

	/**
	 * Sets the ID of the diagram.
	 */
	void setID( int id ) {
		m_nID = id;
	}

	/**
	 * Returns the zoom of the diagram.
	 */
	int getZoom() const {
		return m_nZoom;
	}

	/**
	 * Sets the zoom of the diagram.
	 */
	void setZoom(int zoom);

	/**
	 * Returns the height of the diagram.
	 */
	int getCanvasHeight() const {
		return m_nCanvasHeight;
	}

	/**
	 * Sets the height of the diagram.
	 */
	void setCanvasHeight(int height) {
		m_nCanvasHeight = height;
	}

	/**
	 * Returns the width of the diagram.
	 */
	int getCanvasWidth() const {
		return m_nCanvasWidth;
	}

	/**
	 * Sets the height of the diagram.
	 */
	void setCanvasWidth(int width) {
		m_nCanvasWidth = width;
	}

	/**
	 * Return whether to use snap to grid.
	 */
	bool getSnapToGrid() const {
		return m_bUseSnapToGrid;
	}

	/**
	 * Return whether to use snap to grid for component size.
	 */
	bool getSnapComponentSizeToGrid() const {
		return m_bUseSnapComponentSizeToGrid;
	}

	/**
	 * Returns the x grid size.
	 */
	int getSnapX() const {
		return m_nSnapX;
	}

	/**
	 * Returns the y grid size.
	 */
	int getSnapY() const {
		return m_nSnapY;
	}

	/**
	 *  Returns whether to show snap grid or not.
	 */
	bool getShowSnapGrid() const {
		return m_bShowSnapGrid;
	}

	/**
	 * Sets whether to show snap grid.
	 */
	void setShowSnapGrid( bool bShow );

	/**
	 *  Sets whether to snap to grid.
	 */
	void setSnapToGrid( bool bSnap );

	/**
	 * Sets whether to snap to grid for component size.
	 */
	void setSnapComponentSizeToGrid( bool bSnap );

	/**
	 * Sets whether to use the fill/background color
	 */

	void setUseFillColor( bool ufc ) {
		m_Options.uiState.useFillColor = ufc;
	}

	/**
	 * Returns whether to use the fill/background color
	 */
	bool getUseFillColor() const {
		return m_Options.uiState.useFillColor;
	}

	/**
	 * Sets the font for the view and all the widgets on the view.
	 */
	void setFont( QFont font );

	/**
	 * Returns the font to use
	 */
	QFont getFont() const {
		return m_Options.uiState.font;
	}

	/**
	 * Returns the options being used.
	 */
	const SettingsDlg::OptionState& getOptionState() const {
		return m_Options;
	}

	/**
	 * Sets the options to be used.
	 */
	void setOptionState( const SettingsDlg::OptionState& options) {
		m_Options = options;
	}

	/**
	 * Returns a reference to the association list.
	 */
	AssociationWidgetList& getAssociationList() {
		return m_AssociationList;
	}

	/**
	 * Returns a reference to the widget list.
	 */
	UMLWidgetList& getWidgetList() {
		return m_WidgetList;
	}

	/**
	 *   Returns a reference to the message list.
	 */
	MessageWidgetList& getMessageList() {
		return m_MessageList;
	}

        // End of accessors and methods that only deal with loaded/saved data
	////////////////////////////////////////////////////////////////////////

	/**
	* return the current zoom factor
	*/
	int currentZoom();

	/**
	 * returns a pointer to the document connected to the view
	 * instance. Mind that this method requires a UMLApp instance
	 * as a parent widget to get to the window document pointer by
	 * calling the UMLApp::getDocument() method.
	 *
	 * @see UMLApp#getDocument
	 */
	UMLDoc* getDocument() const;


	/**
	 * contains the implementation for printing functionality
	 */
	void print(KPrinter *pPrinter, QPainter & pPainter);

	/**
	 * Print the entire diagram in this view to the file 'filename'
	 * @param isEPS The file is an eps file and needs adjusting
	 *   of the eps bounding box values
	 */
	void printToFile(QString filename, bool isEPS);

	/**
	 * Fix the file 'filename' to be a valid EPS containing the
	 * specified area (rect) of the diagram.
	 * Corrects the bounding box.
	 */
	void fixEPS(QString filename, QRect rect);

	/**
	 * Overrides the standard operation.
	 */
	void hideEvent(QHideEvent *he);

	/**
	 * Overrides the standard operation.
	 */
	void showEvent(QShowEvent *se);

	/**
	 * Returns a reference to the @ref UMLView class.
	 *
	 * @return Returns a reference to the @ref UMLView class.
	 */
	UMLListView * getListView();

	/**
	 * Sets an association to include the given widget.  If this is the second
	 * widget set for the association it creates the association.
	 *
	 * @param	w	The widget to set for the association.
	 */
	bool setAssoc(UMLWidget *w);

	/**
	 * Sees if a message is relevant to the given widget.  If it does delete it.
	 * @param	w	The widget to check messages against.
	 */
	void checkMessages(ObjectWidget * w);

	/**
	 * Finds a widget with the given ID.
	 *
	 * @param id			The ID of the widget to find.
	 * @param allowClassForObject	True if ok to return the class of an ObjectWidget.
	 *
	 * @return	Returns the widget found, returns 0 if no widget found.
	 */
	UMLWidget * findWidget(int id, bool allowClassForObject = false);

	/**
	 * Finds an association widget with the given ID.
	 *
	 * @param	id	The ID of the widget to find.
	 *
	 * @return	Returns the widget found, returns 0 if no widget found.
	 */
	AssociationWidget * findAssocWidget(int id);

	/**
	 * Remove a widget from view.
	 *
	 * @param	o	The widget to remove.
	 */
	void removeWidget(UMLWidget * o);

	/**
	 *	Sets a widget to a selected state and adds it to a list of selected widgets.
	 *
	 * @param	w	The widget to set to selected.
	 * @param me	The mouse event containing the information about the selection.
	 */
	void setSelected(UMLWidget * w, QMouseEvent * me);

	/**
	 *  Clear the selected widgets list.
	 */
	void clearSelected();

	/**
	 * Move all the selected widgets.
	 *
	 * @param	w	The widget in the selected list to move in reference to.
	 * @param	x	The distance to move horizontally.
	 * @param y	The distance to move vertically.
	 */
	void moveSelected(UMLWidget * w, int x, int y);

	/**
	 * Return the amount of widgets selected.
	 *
	 * @return Return the amount of widgets selected.
	 */
	int getSelectCount() const {
		return m_SelectedList.count();
	}

	/**
	 * Set the useFillColor variable to all selected widgets
	 *
	 * @param	useFC	The state to set the widget to.
	 */
	void selectionUseFillColor(bool useFC);

	/**
	 * Set the font for all the currently selected items.
	 */
	void selectionSetFont( QFont font );

	/**
	 * Set the line color for all the currently selected items.
	 */
	void selectionSetLineColor( QColor color );

	/**
	 * Set the fill color for all the currently selected items.
	 */
	void selectionSetFillColor( QColor color );

	/**
	 * Toggles the show setting sel of all selected items.
	 */
	void selectionToggleShow(int sel);

	/**
	 * Delete the selected widgets list and the widgets in it.
	 */
	void deleteSelection();

	/**
	 * Selects all widgets
	 */
	void selectAll();

	/**
	 * Return a unique ID for the diagram.  Used by the @ref ObjectWidget class.
	 *
	 * @return Return a unique ID for the diagram.
	 */
	int getLocalID() {
		return --m_nLocalID;
	}

	/**
	 * Returns whether a widget is already on the diagram.
	 *
	 * @param id	The id of the widget to check for.
	 *
	 * @return	Returns true if the widget is already on the diagram, false if not.
	 */
	bool widgetOnDiagram(int id);


	/**
	 * Set the pos variable.  Used internally to keep track of the cursor.
	 *
	 * @param _pos	The position to set to.
	 */
	void setPos(QPoint _pos) {
		m_Pos = _pos;
	}

	/**
	 * Sets the popup menu to use when clicking on a diagram background
	 * (rather than a widget or listView).
	 */
	void setMenu();

	/**
	 * Reset the toolbar.
	 */
	void resetToolbar() {
		emit sigResetToolBar();
		m_CurrentCursor = WorkToolBar::tbb_Arrow;
	}

	/**
	 * Returns the status on whether in a paste state.
	 *
	 * @return Returns the status on whether in a paste state.
	 */
	bool getPaste() const {
		return m_bPaste;
	}

	/**
	 *   FIXME Gustavo ???
	 */
	void updateNoteWidgets();

	/**
	 * Returns a List of all the UMLObjects(Use Cases, Concepts and Actors) in the View
	 */
	UMLObjectList* getUMLObjects();

	/**
	 * Activate all the objects and associations after a load from the clipboard
	 */
	bool activate();

	/**
	 * Fills the List with all the selected associations from the diagram
	 */
	bool getSelectedAssocs(AssociationWidgetList & assocWidgetList);

	/**
	 * Fills the List with all the selected widgets from the diagram
	 */
	bool getSelectedWidgets(UMLWidgetList& WidgetList);

	/**
	 * Activate the view after a load a new file
	 *
	 * @return Return true if the activate was succesful
	 */
	bool activateAfterLoad( bool bUseLog = false );

	/**
	 *  FIXME Gustavo ???
	 */
	IDChangeLog* getLocalIDChangeLog();

	/**
	* FIXME Gustavo ???
	*/
	void endPartialWidgetPaste();
	/**
	 * FIXME Gustavo ???
	 */
	void beginPartialWidgetPaste();

	/**
	 * Removes a AssociationWidget from a diagram
	 */
	void removeAssoc(AssociationWidget* pAssoc);

	/**
	 * Removes all the associations related to Widget
	 */
	void removeAssociations(UMLWidget* Widget);

	/**
	 * Sets each association as selected if the widgets it associates are selected
	 */
	void selectAssociations(bool bSelect);

	/**
	 * Fills Associations with all the associations that includes a widget related to object
	 */
	void getWidgetAssocs(UMLObject* Obj, AssociationWidgetList & Associations);

	/**
	 * Removes All the associations of the diagram
	 */
	void removeAllAssociations();

	/**
	 * Removes All the widgets of the diagram
	 */
	void removeAllWidgets();

	/**
	 *  Calls the same method in the DocWindow.
	 */
	void showDocumentation( UMLObject * object, bool overwrite );

	/**
	 *  Calls the same method in the DocWindow.
	 */
	void showDocumentation( UMLWidget * widget, bool overwrite );

	/**
	*  Calls the same method in the DocWindow.
	*/
	void showDocumentation( AssociationWidget * widget, bool overwrite );

	/**
	*  Calls the same method in the DocWindow.
	*/
	void updateDocumentation( bool clear );

	/**
	 * Returns the PNG picture of the paste operation.
	 *
	 * @param rect the area of the diagram to copy
	 * @param diagram the class to store PNG picture of the paste operation.
	 */
	void getDiagram(const QRect &rect, QPixmap & diagram);

	/**
	 * Paint diagram to the paint device
	 */
	void  getDiagram(const QRect &area, QPainter & painter);

	/**
	 * Returns the PNG picture of the paste operation.
	 */
	void copyAsImage(QPixmap*& pix);

	/**
	 * Saves a png file to the given url.
	 */
	void exportImage();

	/**
	 * Adds an association to the view from the given data.
	 * Use this method when pasting.
	 */
	bool addAssociation( AssociationWidget* pAssoc );

	/**
	 * Adds a widget to the view from the given data.
	 * Use this method when pasting.
	 */
	bool addWidget( UMLWidget * pWidget );

	/**
	 * Returns the offset point at which to place the paste from clipboard.
	 * Just add the amount to your co-ords.
	 * Only call this straight after the event, the value won't stay valid.
	 * Should only be called by Assoc widgets at the moment. no one else needs it.
	 */
	QPoint getPastePoint();

	/**
	 * Called by the view or any of its children when they start a cut
	 * operation.
	 */
	void setStartedCut() {
		m_bStartedCut = true;
	}

	/**
	 * Creates automatically any Associations that the given @ref UMLWidget
	 * may have on any diagram.  This method is used when you just add the UMLWidget
	 * to a diagram.
	 */
	void createAutoAssociations( UMLWidget * widget );

	/**
	 * Sets the x grid size.
	 */
	void setSnapX( int x) {
		m_nSnapX = x;
		canvas() -> setAllChanged();
	}

	/**
	 * Sets the y grid size.
	 */
	void setSnapY( int y) {
		m_nSnapY = y;
		canvas() -> setAllChanged();
	}

	/**
	 * Shows the properties dialog for the view.
	 */
	bool showPropDialog();

	/**
	 * Sets some options for all the @ref ClassWidget on the view.
	 */
	void setClassWidgetOptions( ClassOptionsPage * page );

	/**
	* Call before copying/cutting selected widgets.  This will make sure
	* any associations/message selected will make sure both the widgets
	* widgets they are connected to are selected.
	*/
	void checkSelections();

	/**
	 * This function checks if the currently selected items have all the same
	 * type (class, interface, ...). If true, the selection is unique and true
	 * will be returned.
	 * If there are no items selected, the function will return always true.
	 */
	bool checkUniqueSelection();

	/**
	* Adds a sequence line to the list.
	*/
	void addSeqLine( SeqLineWidget * pLine ) {
		m_SeqLineList.append( pLine );
	}

	/**
	* Removes a sequence line from the list.
	*/
	void removeSeqLine(SeqLineWidget* pLine) {
		m_SeqLineList.remove(pLine);
	}

	/**
	 * Asks for confirmation and clears everything on the diagram.
	 * Called from menues.
	 */
	void clearDiagram();

	/**
	 * Changes snap to grid boolean.
	 * Called from menues.
	 */
	void toggleSnapToGrid();

	/**
	 * Changes snap to grid for component size boolean.
	 * Called from menues.
	 */
	void toggleSnapComponentSizeToGrid();

	/**
	 *  Changes show grid boolean.
	 * Called from menues.
	 */
	void toggleShowGrid();

	/**
	 * Changes the zoom to the currently set level (now loaded from file)
	 * Called from UMLApp::slotUpdateViews()
	 */
	void fileLoaded();

	/**
	 * Sets the diagram width and height in pixels
	 */
	void setCanvasSize(int width, int height);

	/**
	 * Sets the size of the canvas to just fit on all the items
	 */
	void resizeCanvasToItems();

	/**
	 * the width and height of a diagram canvas in pixels
	 */
	static const int defaultCanvasSize;

	// Load/Save interface:

	/**
	 * creates the <diagram> tag, and fills it with the contents of the diagram
	 */
	virtual bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the <diagram> tag
	 */
	virtual bool loadFromXMI( QDomElement & qElement );

	/**
	 * Loads a <widget> element such as <UML:Class>, used by loadFromXMI() and the clipboard
	 */
	UMLWidget* loadWidgetFromXMI(QDomElement& widgetElement);

protected:

	// Methods and members related to loading/saving

	bool loadWidgetsFromXMI( QDomElement & qElement );

	bool loadMessagesFromXMI( QDomElement & qElement );

	bool loadAssociationsFromXMI( QDomElement & qElement );

	/**
	 * Contains the unique IDs to allocate to and widget that that needs an
	 * ID for the view.  @ref ObjectWidgets are an example of this.
	 */
	int m_nLocalID;

	/**
	 * The ID of the view.  Allocated by @ref UMLDoc
	 */
	int m_nID;

	/**
	 * The type of diagram to represent.
	 */
	Diagram_Type m_Type;

	/**
	 * The name of the diagram.
	 */
	QString m_Name;

	/**
	 * The documentation of the diagram.
	 */
	QString m_Documentation;

	/**
	 * Options used by view
	 */
	SettingsDlg::OptionState	m_Options;

	/**
	 * Contains all the data items for @ref MessageWidgets on the diagram.
	 */
	MessageWidgetList m_MessageList;

	/**
	 * Contains all the data for @ref UMLWidgets on the diagram.
	 */
	UMLWidgetList m_WidgetList;

	/**
	 * Contains all the data for @ref AssociationWidgets on the diagram.
	 */
	AssociationWidgetList m_AssociationList;

	/**
	 * The snap to grid x size.
	 */
	int m_nSnapX;

	/**
	 * The snap to grid y size.
	 */
	int m_nSnapY;

	/**
	 * Determines whether to use snap to grid.  The default is off.
	 */
	bool m_bUseSnapToGrid;

	/**
	 * Determines whether to use snap to grid for component
	 * size.  The default is off.
	 */
	bool m_bUseSnapComponentSizeToGrid;

	/**
	 * Determines whether to show the snap grid.  The default will be on if the grid is on.
	 */
	bool m_bShowSnapGrid;

	/**
	 * The zoom level in percent, default 100
	 */
	int m_nZoom;

	/**
	 * Width of canvas in pixels
	 */
	int m_nCanvasWidth;

	/**
	 * Height of canvas in pixels
	 */
	int m_nCanvasHeight;

	// End of methods and members related to loading/saving
	////////////////////////////////////////////////////////////////////////

	/**
	 * Override standard method.
	 */
	void closeEvent ( QCloseEvent * e );

	/**
	 * Override standard method.
	 */
	void contentsDragEnterEvent(QDragEnterEvent* mouseEvent);

	/**
	 * Override standard method.
	 */
	void contentsDropEvent(QDropEvent* mouseEvent);

	/**
	 * Determine whether on a sequence diagram we have clicked on a line of an Object.
	 *
	 * @returns	Returns the widget thats line was clicked on.  Returns 0 if no line was clicked on.
	 */
	ObjectWidget * onWidgetLine( QPoint point );

	/**
	 * Adds an AssociationWidget to the association list
	 * and creates the corresponding UMLAssociation in the current UMLDoc.
	 * FIXME: Since the addition of the UMLAssociation class this method is
	 * not strictly required. Its raison d'etre is to maintain compatibility
	 * with XMI files generated by umbrello versions smaller than or equal to 1.1-rc1.
	 * This method can be removed when support for the old XMI files is dropped.
	 */
        void addAssocInViewAndDoc(AssociationWidget* assoc);

	/**
	 * Removes an AssociationWidget from the association list
	 * and removes the corresponding UMLAssociation from the current UMLDoc.
	 * FIXME: See comment at addAssocInViewAndDoc().
	 */
	void removeAssocInViewAndDoc(AssociationWidget* assoc, bool deleteLater = false);

	/**
	 * Initializes key variables.
	 */
	void init();

	/**
	 * Initialize and announce a newly created widget.
	 * Auxiliary to contentsMouseReleaseEvent().
	 */
	void setupNewWidget(UMLWidget *w, bool setNewID=true);

	/**
	 * Overrides the standard operation.
	 */
	void contentsMouseReleaseEvent(QMouseEvent* mouseEvent);

	/**
	 * Overrides the standard operation.
	 */
	void contentsMouseMoveEvent(QMouseEvent* mouseEvent);

	/**
	 * Override standard method.
	 */
	void contentsMouseDoubleClickEvent(QMouseEvent* mouseEvent);

	/**
	 * Override standard method.
	 */
	void contentsMousePressEvent(QMouseEvent* mouseEvent);

	/**
	 * Gets the smallest area to print.
	 *
	 * @return	Returns the smallest area to print.
	 */
	QRect getDiagramRect();

	/**
	 * Selects all the widgets within an internally kept rectangle.
	 */
	void selectWidgets();

	/**
	 * Selects all the widgets of the given association widget.
	 */
	void selectWidgetsOfAssoc (AssociationWidget * a);

	/**
	 * Calls setSelected on the given UMLWidget and enters
         * it into the m_SelectedList while making sure it is
         * there only once.
	 */
        void makeSelected (UMLWidget * uw);

	/**
	 * Updates the size of all components in this view.
	 */
	void updateComponentSizes();

	/**
	 * Find the maximum bounding rectangle of FloatingText widgets.
	 * Auxiliary to copyAsImage().
	 *
	 * @param ft	Pointer to the FloatingText widget to consider.
	 * @param px	X coordinate of lower left corner. This value will be
	 *		updated if the X coordinate of the lower left corner
	 *		of ft is smaller than the px value passed in.
	 * @param py	Y coordinate of lower left corner. This value will be
	 *		updated if the Y coordinate of the lower left corner
	 *		of ft is smaller than the py value passed in.
	 * @param qx	X coordinate of upper right corner. This value will be
	 *		updated if the X coordinate of the upper right corner
	 *		of ft is larger than the qx value passed in.
	 * @param qy	Y coordinate of upper right corner. This value will be
	 *		updated if the Y coordinate of the upper right corner
	 *		of ft is larger than the qy value passed in.
	 */
	void findMaxBoundingRectangle(const FloatingText* ft,
				      int& px, int& py, int& qx, int& qy);

	void forceUpdateWidgetFontMetrics(QPainter *painter);

	QPoint m_Pos, m_LineToPos;
	bool m_bCreateObject, m_bDrawRect, m_bDrawSelectedOnly, m_bPaste;
	ListPopupMenu * m_pMenu;
	UMLWidgetList m_SelectedList;
	AssociationWidget * m_pMoveAssoc;

	/**
	 *  Flag if view/children started cut operation.
	 */
	bool m_bStartedCut;

public:
	WorkToolBar::ToolBar_Buttons getCurrentCursor() const;

private:
	WorkToolBar::ToolBar_Buttons m_CurrentCursor;

	/**
	 * converts toolbar button enums to association type enums
	 */
	Uml::Association_Type convert_TBB_AT(WorkToolBar::ToolBar_Buttons tbb);

	/**
	 * convert toolbar button enums to umlobject type enums
	 */
	Uml::UMLObject_Type convert_TBB_OT(WorkToolBar::ToolBar_Buttons tbb);

	/**
	 * Sees if the MousePressEvent needs to be allocated to a UMLWidget
	 */
	bool allocateMousePressEvent(QMouseEvent * me);

	/**
	 * Sees if the MouseReleaseEvent needs to be allocated to a UMLWidget
	 */
	bool allocateMouseReleaseEvent(QMouseEvent * me);

	/**
	 * Sees if the MouseDoubleClickEvent needs to be allocated to a UMLWidget
	 */
	bool allocateMouseDoubleClickEvent(QMouseEvent * me);

	/**
	 * Sees if the MouseMoveEvent needs to be allocated to a UMLWidget
	 */
	bool allocateMouseMoveEvent(QMouseEvent * me);

	/**
	 * LocalID Changes Log for paste actions
	 */
	IDChangeLog * m_pIDChangesLog;

	/**
	 *
	 * True if the view was activated after the serialization(load)
	 */
	bool m_bActivated;

	/**
	 * It holds a pointer the the first selected widget when creating an Association
	 */
	UMLWidget* m_pFirstSelectedWidget;

	/**
	 * Status of a popupmenu on view.
	 * true - a popup is on view
	 */
	bool m_bPopupShowing;

	/**
	 * The current @ref UMLWidget we are on. A reference is
	 * kept so we can monitor for a leave event.
	 */
	UMLWidget * m_pOnWidget;

	/**
	 * Holds the Canvas lines for the selection rectangle.
	 */
	QPtrList<QCanvasLine> m_SelectionRect;

	/**
	 * The Line used to show a join between objects as an association is being made.
	 */
	QCanvasLine * m_pAssocLine;

	/**
	 * The offset at which to paste the clipboard.
	 */
	QPoint m_PastePoint;

	/**
	 * Holds a list of all the sequence lines on a sequence diagram.
	 */
	QPtrList<SeqLineWidget> m_SeqLineList;

	/**
	 * Pointer to the UMLDoc
	 */
	UMLDoc* m_pDoc;

	/**
	 * The url of the last saved image
	 */
	KURL m_ImageURL;

	/**
	 * Used by @ref contentsMouseMoveEvent() to know if a mouse button is pressed.
	 */
	bool m_bMouseButtonPressed;

public slots:

	void zoomIn();
	void zoomOut();

	void slotToolBarChanged(int c);
	void slotObjectCreated(UMLObject * o);
	void slotObjectRemoved(UMLObject * o);

	/**
	 * When a menu selection has been made on the menu
	 * that this view created, this method gets called.
	 */
	void slotMenuSelection(int sel);

	/**
	 * This slot is entered when an event has occurred on the views display,
	 * most likely a mouse event.  Before it sends out that mouse event everyone
	 * that displays a menu on the views surface (widgets and this ) thould remove any
	 * menu.  This stops more then one menu bieing diplayed.
	 */
	void slotRemovePopupMenu();

	/**
	 * makes this view the active view by asking the document to show us
	 */
	void slotActivate();

	/**
	 * Connects to the signal that @ref UMLApp emits when a cut operation
	 * is successful.
	 * If the view or a child started the operation the flag m_bStartedCut will
	 * be set and we can carry out any operation that is needed, like deleting the selected
	 * widgets for the cut operation.
	 */
	void slotCutSuccessful();

	/**
	 * Called by menu when to show the instance of the view.
	 */
	void slotShowView();

	/**
	 * Overrides standard method from QWidget to resize canvas when
	 * it's shown.
	 */
	void show();

signals:
	void sigResetToolBar();
	void sigColorChanged( int );
	void sigRemovePopupMenu();
	void sigClearAllSelected();
	void sigLineColorChanged( QColor );
	void sigSnapToGridToggled(bool);
	void sigSnapComponentSizeToGridToggled(bool);
	void sigShowGridToggled(bool);
};

#endif // UMLVIEW_H
