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
#include "associationwidgetdatalist.h"

#include "umlnamespace.h"
#include "worktoolbar.h"
#include "dialogs/settingsdlg.h"

#include "umldoc.h"
#include "umlviewdata.h"

// QT includes
#include <qcanvas.h>

// KDE includes
#include <kurl.h>

// #define OFFSET		50

class ClassOptionsPage;
class IDChangeLog;
class ListPopupMenu;
class MessageWidget;
class SeqLineWidget;

class UMLListView;
class UMLApp;

class KPrinter;

using namespace Uml;

/** The UMLView class provides the view widget for the UMLApp
 * instance.  The View instance inherits QWidget as a base class and
 * represents the view object of a KTMainWindow. As UMLView is part of
 * the docuement-view model, it needs a reference to the document
 * object connected with it by the UMLApp class to manipulate and
 * display the document structure provided by the UMLDoc class.
 *
 * @author Paul Hensgen <phensgen@techie.com>
 * @version 1.0
 */
class UMLView : public QCanvasView {
	Q_OBJECT
public:
	/**
	 * Constructor for the main view
	 */
	UMLView(QWidget* parent, UMLViewData* pData, UMLDoc* doc);

	/**
	 * Destructor for the main view
	 */
	~UMLView();

	/**
	* sets the zoom factor of the view
	*/
	void setZoom(int zoom);

	/**
	* return the current zoom factor
	*/
	int currentZoom();

	/**
	*	Synchronize the views data. Call before saving or a full cip of the view.
	*/
	void synchronizeData();

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
	 * Print the specified area (rect) of the diagram to the file 'filename'
	 */
	void printToFile(QString filename, QRect rect);

	/** 
	 * Fix the file 'filename' to be a valid EPS containing the 
	 * specified area (rect) of the diagram. 
	 * Corrects the bounding box.
	 */
	void fixEPS(QString filename, QRect rect);

	/**
	*	Returns the type of diagram this is.
	*
	*	@return	Returns the type of diagram this is.
	*/

	Diagram_Type getType() {
		return m_pData -> m_Type;
	}

	/**
	*	Returns the ID of thios diagram.
	*
	*	@return	Returns the ID of thios diagram.
	*/
	int getID() {
		return m_pData -> m_nID;
	}

	/**
	*	Sets the ID of this diagram.
	*
	*	@return	Sets the ID of this diagram.
	*/
	void setID(int NewID) {
		m_pData -> m_nID = NewID;
	}

	/**
	 * Overrides the standard operation.
	 */
	void hideEvent(QHideEvent *he);

	/**
	 * Overrides the standard operation.
	 */
	void showEvent(QShowEvent *se);

	/**
	 *	Returns a reference to the @ref UMLView class.
	 *
	 *	@return Returns a reference to the @ref UMLView class.
	 */
	UMLListView * getListView();

	/**
	 *	Sets an association to include the given widget.  If this is the second
	 *	widget set for the association it creates the association.
	 *
	 *	@param	w	The widget to set for the association.
	 */
	bool setAssoc(UMLWidget *w);

	/**
	 *	Sees if a message is relevant to the given widget.  If it does delete it.
	 *	@param	w	The widget to check messages against.
	 */
	void checkMessages(UMLWidget * w);

	/**
	 *	Finds a widget with the given ID.
	 *
	 *	@param	id	The ID of the widget to find.
	 *
	 *	@return	Returns the widget found, returns 0 if no widget found.
	 */
	UMLWidget * findWidget(int id);

	/**
	 *	Remove a widget from view.
	 *
	 *	@param	o	The widget to remove.
	 */
	void removeWidget(UMLWidget * o);

	/**
	 *	Set the background color.
	 *
	 *	@param	color	The color to use.
	 */
	void setFillColour(QColor colour);

	/**
	 *	Sets the line color.
	 *
	 *	@param	color	The color to use.
	 */
	void setLineColor(QColor color);

	/**
	 *	Sets the font for the view and all the widgets on the view.
	 */
	void setFont( QFont font );

	/**
	 *	Returns the background color.
	 *
	 *	@return	Returns the background color.
	 */
	QColor getFillColour() {
		return m_pData->getFillColor();
	}

	/**
	 *	Returns the line color.
	 *
	 *	@return	Returns the line color.
	 */
	QColor getLineColor() {
		return m_pData -> getLineColor();
	};

	/**
	 *	Sets a widget to a selected state and adds it to a list of selected widgets.
	 *
	 *	@param	w	The widget to set to selected.
	 *	@param me	The mouse event containing the information about the selection.
	 */
	void setSelected(UMLWidget * w, QMouseEvent * me);

	/**
	 * 	Clear the selected widgets list.
	 */
	void clearSelected();

	/**
	 *	Move all the selected widgets.
	 *
	 *	@param	w	The widget in the selected list to move in reference to.
	 *	@param	x	The distance to move horizontally.
	 *	@param y	The distance to move vertically.
	 */
	void moveSelected(UMLWidget * w, int x, int y);

	/**
	 * Return the amount of widgets selected.
	 *
	 *	@return Return the amount of widgets selected.
	 */
	int getSelectCount() {
		return m_SelectedList.count();
	};

	/**
	 *	Set the useFillColor variable to all selected widgets
	 *
	 *	@param	useFC	The state to set the widget to.
	 */
	void selectionUseFillColor(bool useFC);

	/**
	 * 	Set the font for all the currently selected items.
	 */
	void selectionSetFont( QFont font );

	/**
	 *	Delete the selected widgets list and the widgets in it.
	 */
	void deleteSelection();

 	/**
	 * Selects all widgets
	 */
	void selectAll();

	/**
	 *	Return a unique ID for the diagram.  Used by the @ref ObjectWidget class.
	 *
	 *	@return Return a unique ID for the diagram.
	 */
	int getLocalID() {
		return m_pData -> getUniqueID();
	}

	/**
	 *	Returns whether a widget is already on the diagram.
	 *
	 *	@param id	The id of the widget to check for.
	 *
	 *	@return	Returns true if the widget is already on the diagram, false if not.
	 */
	bool widgetOnDiagram(int id);


	/**
	 *	Set the pos variable.  Used internally to keep track of the cursor.
	 *
	 *	@param _pos	The position to set to.
	 */
	void setPos(QPoint _pos) {
		m_Pos = _pos;
	};

	/**
	 *	Sets the popup menu to use when clicking on a diagram background
	 *	(rather than a widget or listView).
	 */
	void setMenu();

	/**
	 *	Fills the list parameter with the Associations contained in this UMLView.
	 *
	 *	@return Returns true if the list was filled without problems
	 */
	bool getAssocWidgets(AssociationWidgetList & Associations);

	/**
	 *	Reset the toolbar.
	 */
	void resetToolbar() {
		emit sigResetToolBar();
		m_CurrentCursor = WorkToolBar::tbb_Arrow;
	}

	/**
	 *	Returns the status on whether in a paste state.
	 *
	 *	@return Returns the status on whether in a paste state.
	 */
	bool getPaste() {
		return m_bPaste;
	};

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
	bool getSelectedAssocDatas(AssociationWidgetDataList & AssociationWidgetDataList);

	/**
	 * Fills the List with all the selected widgets from the diagram
	 */
	bool getSelectedWidgets(UMLWidgetList& WidgetList);

	/**
	 * Fills the List with all the selected widget Datas from the diagram
	 */
	bool getSelectedWidgetDatas(UMLWidgetDataList& WidgetDataList);

	/**
	 * Creates an UMLWidget from the UMLWidgetData
	 */
	bool createWidget(UMLWidgetData* WidgetData);

	/**
	 * Creates an association from a AssociationWidgetData from the clipboard,
	 *Activate must be called to make this assoc visible
	 */
	bool createAssoc(AssociationWidgetData* AssocData);

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
	 * Sets whether to use the fill/background color
	 */
	void setUseFillColor( bool ufc ) {
		m_pData -> setUseFillColor( ufc );
	}

	/**
	 * Returns whether to use the fill/background color
	 */
	bool getUseFillColor() {
		return m_pData -> getUseFillColor();
	}

	/**
	 *   Returns the options being used.
	 */
	SettingsDlg::OptionState getOptionState() {
		return m_pData -> getOptionState();
	}

	/**
	 *		Sets the options to be used.
	 */
	void setOptionState( SettingsDlg::OptionState options) {
		m_pData -> setOptionState( options );
	}
	/**
	 *  Returns a copy of m_Name
	 */
	QString getName();


	/**
	 *  Set the UMLView's m_Name property
	 */
	void setName(QString& strName);

	/**
	 * Returns the list contianing all the association widgets on this view.
	 */
	AssociationWidgetList * getAssociationWidgetList() {
		return & m_Associations;
	}

	/**
	 * 	Calls the same method in the DocWindow.
	 */
	void showDocumentation( UMLObject * object, bool overwrite );

	/**
	 * 	Calls the same method in the DocWindow.
	 */
	void showDocumentation( UMLWidget * widget, bool overwrite );

	/**
	* 	Calls the same method in the DocWindow.
	*/
	void showDocumentation( AssociationWidget * widget, bool overwrite );

	/**
	* 	Calls the same method in the DocWindow.
	*/
	void updateDocumentation( bool clear );

	/**
	 * 	Returns the documentation for the view.
	 */
	QString getDoc() {
		return m_pData -> m_Documentation;
	}

	/**
	 * 	Sets the documentation for the view.
	 */
	void setDoc( QString doc ) {
		m_pData -> m_Documentation = doc;
	}

	/**
	 *	Returns the PNG picture of the paste operation.
	 *
	 *	@param rect the area of the diagram to copy
	 *	@param diagram the class to store PNG picture of the paste operation.
	 */
	void getDiagram(const QRect &rect, QPixmap & diagram);

	/**
	 *      Paint diagram to the paint device
	 */
	void  getDiagram(const QRect &area, QPainter & painter);

	/**
	 *	Returns the PNG picture of the paste operation.
	 */
	void copyAsImage(QPixmap*& pix);

	/**
	 * 		Saves a png file to the given url.
	 */
	void exportImage();

	/**
	 * 		Adds an association to the view from the given data.
	 *		Use this method when pasting.
	 */
	bool addAssociation( AssociationWidgetData* AssocData );

	/**
	 * 		Adds a widget to the view from the given data.
	 *		Use this method when pasting.
	 */
	bool addWidget( UMLWidgetData * pWidgetData );

	/**
	 * 		Returns the views data.
	 */
	UMLViewData * getData() {
		return m_pData;
	}

	/**
	 * 		Returns the offset point at which to place the paste from clipboard.
	 *		Just add the amount to your co-ords.
	 *		Only call this straight after the event, the value won't stay valid.
	 *		Should only be called by Assoc widgets at the moment. no one else needs it.
	 */
	QPoint getPastePoint();

	/**
	 * 		Called by the view or any of its children when they start a cut
	 *		operation.
	 */
	void setStartedCut() {
		m_bStartedCut = true;
	}

	/**
	 *		Creates automatically any Associations that the given @ref UMLWidget
	 *		may have on any diagram.  This method is used when you just add the UMLWidget
	 *		to a diagram.
	 */
	void createAutoAssociations( UMLWidget * widget );

	/**
	 * 		Return whether to use snap to grid.
	 */
	bool getSnapToGrid() {
		return m_pData -> getSnapToGrid();
	}

	/**
	 * 		Return whether to use snap to grid for component size.
	 */
	bool getSnapComponentSizeToGrid() {
		return m_pData -> getSnapComponentSizeToGrid();
	}

	/**
	 * 		Returns whether to show snap grid.
	 */
	bool getShowSnapGrid() {
		return m_pData -> getShowSnapGrid();
	}

	/**
	 *		Returns the x grid size.
	 */
	int getSnapX() {
		return m_pData -> getSnapX();
	}

	/**
	 *		Returns the y grid size.
	 */
	int getSnapY() {
		return m_pData -> getSnapY();
	}

	/**
	 *		Sets the x grid size.
	 */
	void setSnapX( int x ) {
		m_pData -> setSnapX( x );
		canvas() -> setAllChanged();
	}

	/**
	 *		Sets the y grid size.
	 */
	void setSnapY( int y ) {
		m_pData -> setSnapY( y );
		canvas() -> setAllChanged();
	}

	/**
	 *		Sets whether to use snap to grid.
	 */
	void setSnapToGrid( bool bSnap );

	/**
	 *		Sets whether to use snap to grid for component sizes.
	 */
	void setSnapComponentSizeToGrid( bool bSnap );

	/**
	 *		Sets whether to to show snap grid.
	 */
	void setShowSnapGrid( bool bShow );

	/**
	 * 		Saves the class to an XMI file.
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 *		Shows the properties dialog for the view.
	 */
	bool showPropDialog();

	/**
	 *		Sets some options for all the @ref ClassWidget on the view.
	 */
	void setClassWidgetOptions( ClassOptionsPage * page );

	/**
	*		Call before copying/cutting selected widgets.  This will make sure
	*		any associations/message selected will make sure both the widgets
	*		widgets they are connected to are selected.
	*/
	void checkSelections();

	/**
	*		Adds a sequence line to the list.
	*/
	void addSeqLine( SeqLineWidget * pLine ) {
		m_SeqLineList.append( pLine );
	}

	/**
	*		Removes a sequence line from the list.
	*/
	void removeSeqLine(SeqLineWidget* pLine) {
		m_SeqLineList.remove(pLine);
	}

	/**
	*		Returns a list of all the messagewidgets on the view.
	*/
	QPtrList<MessageWidget> getMessageWidgetList();

	/**
	 *	Asks for confirmation and clears everything on the diagram.
	 *	Called from menus.
	 */
	void clearDiagram();

	/**
	 *	Changes snap to grid boolean in m_pData.
	 *	Called from menus.
	 */
	void toggleSnapToGrid();

	/**
	 *	Changes snap to grid for component size boolean in m_pData.
	 *	Called from menus.
	 */
	void toggleSnapComponentSizeToGrid();

	/**
	 * 	Changed show grid boolean in m_pData
	 *	Called from menus.
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
	 *      the width and height of a diagram canvas in pixels
	 */
	static const int defaultCanvasSize;

protected:

	/**
	 *	Override standard method.
	 */
	void closeEvent ( QCloseEvent * e );

	/**
	 *	Override standard method.
	 */
	void contentsDragEnterEvent(QDragEnterEvent* mouseEvent);

	/**
	 *	Override standard method.
	 */
	void contentsDropEvent(QDropEvent* mouseEvent);

	/**
	 *	Determine whether on a sequence diagram we have clicked on a line of an Object.
	 *
	 *	@returns	Returns the widget thats line was clicked on.  Returns 0 if no line was clicked on.
	 */
	UMLWidget * onWidgetLine( QPoint point );

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
	 *	Initializes key variables.
	 */
	void init();

	/**
	 * Overrides the standard operation.
	 */
	void contentsMouseReleaseEvent(QMouseEvent* mouseEvent);

	/**
	 * Overrides the standard operation.
	 */
	void contentsMouseMoveEvent(QMouseEvent* mouseEvent);

	/**
	 *	Override standard method.
	 */
	void contentsMouseDoubleClickEvent(QMouseEvent* mouseEvent);

	/**
	 *	Override standard method.
	 */
	void contentsMousePressEvent(QMouseEvent* mouseEvent);

	/**
	 *  Gets the smallest area to print.
	 *
	 *	@return	Returns the smallest area to print.
	 */
	QRect getDiagramRect();

	/**
	 *	Selects all the widgets within an internally kept rectangle.
	 */
	void selectWidgets();

	/**
	 *	Updates the size of all components in this view.
	 */
	void updateComponentSizes();

	QPoint m_Pos, m_LineToPos;
	bool m_bCreateObject, m_bDrawRect, m_bDrawSelectedOnly, m_bPaste;
	ListPopupMenu * m_pMenu;
	QPtrList<UMLWidget> m_SelectedList;
	AssociationWidget * m_pMoveAssoc;

	/**
	 * 		Flag if view/children started cut operation.
	 */
	bool m_bStartedCut;

public:
	//TODO:  make these protected with relevant get/set methods.
	WorkToolBar::ToolBar_Buttons m_CurrentCursor;

private:
	/**
	 * converts toolbar button enums to association type enums
	 */
	Uml::Association_Type convert_TBB_AT(WorkToolBar::ToolBar_Buttons tbb);

	/**
	 * convert toolbar button enums to umlobject type enums
	 */
	Uml::UMLObject_Type convert_TBB_OT(WorkToolBar::ToolBar_Buttons tbb);

	/**
	 *  Sees if the MousePressEvent needs to be allocated to a UMLWidget
	 */
	bool allocateMousePressEvent(QMouseEvent * me);

	/**
	 *  Sees if the MouseReleaseEvent needs to be allocated to a UMLWidget
	 */
	bool allocateMouseReleaseEvent(QMouseEvent * me);

	/**
	 *  Sees if the MouseDoubleClickEvent needs to be allocated to a UMLWidget
	 */
	bool allocateMouseDoubleClickEvent(QMouseEvent * me);

	/**
	 *  Sees if the MouseMoveEvent needs to be allocated to a UMLWidget
	 */
	bool allocateMouseMoveEvent(QMouseEvent * me);

	/**
	 *  Used to store the Widgets not activated yet
	 */
	QPtrList<UMLWidget> widgetsNotActivatedList;

	/**
	 *  Used to store the Messages not activated yet
	 */
	QPtrList<MessageWidget> messagesNotActivatedList;

	/**
	 * LocalID Changes Log for paste actions
	 */
	IDChangeLog * m_pIDChangesLog;

	/**
	 *  List of AssociationWidgets inside this View
	 */
	AssociationWidgetList m_Associations;

	/**
	 *
	 * True if the view was activated after the serialization(load)
	 */
	bool m_bActivated;

	/**
	 *  It holds a pointer the the first selected widget when creating an Association
	 */
	UMLWidget* m_pFirstSelectedWidget;

	/**
	 * Status of a popupmenu on view.
	 * true - a popup is on view
	 */
	bool m_bPopupShowing;

	/**
	 *   The current @ref UMLWidget we are on. A reference is
	 *   kept so we can monitor for a leave event.
	 */
	UMLWidget * m_pOnWidget;

	/**
	 *   Holds the Canvas lines for the selection rectangle.
	 */
	QPtrList<QCanvasLine> m_SelectionRect;

	/**
	 *   The Line used to show a join between objects as an association is being made.
	 */
	QCanvasLine * m_pAssocLine;

	/**
	 * 		The data object for the view class.
	 */
	UMLViewData * m_pData;

	/**
	 * 		The offset at which to paste the clipboard.
	 */
	QPoint m_PastePoint;

	/**
	 *		Holds a list of all the sequence lines on a sequence diagram.
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
	 * 	When a menu selection has been made on the menu
	 *	that this view created, this method gets called.
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
	 * 	makes this view the active view by asking the document to show us
	 */
	void slotActivate();

	/**
	 * 	Connects to the signal that @ref UMLApp emits when a cut operation
	 *	is successful.
	 *	If the view or a child started the operation the flag m_bStartedCut will
	 *	be set and we can carry out any operation that is needed, like deleting the selected
	 *	widgets for the cut operation.
	 */
	void slotCutSuccessful();

	/**
	 *	Called by menu when to show the instance of the view.
	 */
	void slotShowView() {
		getDocument() -> changeCurrentView( m_pData -> getID() );
	}

	/**
	 *  Overrides standard method from QWidget to resize canvas when
	 *   it's shown.
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
