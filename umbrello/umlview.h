/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLVIEW_H
#define UMLVIEW_H

// system includes
#include <kurl.h>
#include <qdom.h>
#include <umlviewcanvas.h>
#include <QtGui/QPixmap>

//local includes
#include "umlobjectlist.h"
#include "umlwidgetlist.h"
#include "associationwidgetlist.h"
#include "messagewidgetlist.h"
#include "optionstate.h"
#include "worktoolbar.h"

// forward declarations
class ClassOptionsPage;
class IDChangeLog;
class FloatingTextWidget;
class ListPopupMenu;
class ObjectWidget;
class ToolBarState;
class ToolBarStateFactory;
class UMLFolder;
class UMLApp;
class UMLDoc;
class UMLAttribute;
class UMLCanvasObject;
class UMLClassifier;
class UMLViewImageExporter;
class UMLForeignKeyConstraint;
class UMLEntity;

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QHideEvent;
class QMouseEvent;
class QPrinter;
class QShowEvent;
class UMLScene;

/**
 * UMLView instances represent diagrams.
 * The UMLApp instance manages a QWidgetStack of UMLView instances.
 * The visible diagram is at the top of stack.
 * The UMLView class inherits from QCanvasView and it owns the
 * objects displayed on its related QCanvas (see m_WidgetList.)
 *
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLView : public Q3CanvasView
{
    Q_OBJECT
public:
    friend class UMLViewImageExporterModel;

    UMLView(UMLFolder *parentFolder);
    virtual ~UMLView();

    /**
     * Hack for reducing the difference
     * between the new QGraphicsScreen port.
     * TODO: Check if we should pass an UMLScene instead of an UMLView
     *       at places where we see "view->umlScene()->doSomething()".
     */
    UMLScene* umlScene() {
        return m_scene;
    }

    // Accessors and other methods dealing with loaded/saved data

    /**
     * Return the UMLFolder in which this diagram lives.
     */
    UMLFolder *folder() {
        return m_pFolder;
    }

    /**
     * Set the UMLFolder in which this diagram lives.
     */
    void setFolder(UMLFolder *folder) {
        m_pFolder = folder;
    }

    /**
     * Return the documentation of the diagram.
     */
    QString documentation() const {
        return m_Documentation;
    }

    /**
     * Set the documentation of the diagram.
     */
    void setDocumentation(const QString &doc) {
        m_Documentation = doc;
    }

    QString name() const;
    void setName(const QString &name);

    /**
     * Returns the type of the diagram.
     */
    Uml::DiagramType type() const {
        return m_Type;
    }

    /**
     * Set the type of diagram.
     */
    void setType( Uml::DiagramType type ) {
        m_Type = type;
    }

    /**
     * Returns the fill color to use.
     */
    QColor fillColor() const;

    /**
     * Set the background color.
     *
     * @param color  The color to use.
     */
    void setFillColor( const QColor &color );

    /**
     * Returns the line color to use.
     */
    QColor lineColor() const;

    /**
     * Sets the line color.
     *
     * @param color  The color to use.
     */
    void setLineColor( const QColor &color );

    /**
     * Returns the line width to use.
     */
    uint lineWidth() const;

    /**
     * Sets the line width.
     *
     * @param width  The width to use.
     */
    void setLineWidth( uint width );

    /**
     * Returns the text color to use.
     */
    QColor textColor() const;

    /**
     * Sets the text color.
     *
     * @param color  The color to use.
     */
    void setTextColor( const QColor &color );

    /**
     * Returns the ID of the diagram.
     */
    Uml::IDType getID() const {
        return m_nID;
    }

    /**
     * Sets the ID of the diagram.
     */
    void setID( Uml::IDType id ) {
        m_nID = id;
    }

    /**
     * Returns the zoom of the diagram.
     */
    int zoom() const {
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
     *  Sets whether to snap to grid.
     */
    void setSnapToGrid( bool bSnap );

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
     * Returns the input coordinate with possible grid-snap applied.
     */
    int snappedX(int x);

    /**
     * Returns the input coordinate with possible grid-snap applied.
     */
    int snappedY(int y);

    /**
     *  Returns whether to show snap grid or not.
     */
    bool getShowSnapGrid() const;

    /**
     * Sets whether to show snap grid.
     */
    void setShowSnapGrid( bool bShow );

    /**
     * Sets whether to snap to grid for component size.
     */
    void setSnapComponentSizeToGrid( bool bSnap );

    /**
     * Returns whether to use the fill/background color
     */
    bool useFillColor() const;

    /**
     * Sets whether to use the fill/background color
     */
    void setUseFillColor(bool ufc);

    /**
     * Returns the font to use
     */
    QFont getFont() const;

    /**
     * Sets the font for the view and optionally all the widgets on the view.
     */
    void setFont(QFont font, bool changeAllWidgets = false);

    /**
     * Returns whether to show operation signatures.
     */
    bool getShowOpSig() const;

    /**
     * Sets whether to show operation signatures.
     */
    void setShowOpSig(bool bShowOpSig);

    /**
     * Returns the options being used.
     */
    const Settings::OptionState& optionState() const {
        return m_Options;
    }

    /**
     * Sets the options to be used.
     */
    void setOptionState( const Settings::OptionState& options) {
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
     * Returns a reference to the message list.
     */
    MessageWidgetList& getMessageList() {
        return m_MessageList;
    }

    bool isOpen() const;
    void setIsOpen(bool isOpen);

    // End of accessors and methods that only deal with loaded/saved data
    ////////////////////////////////////////////////////////////////////////

    /**
     * return the current zoom factor
     */
    int currentZoom();

    /**
     * contains the implementation for printing functionality
     */
    void print(QPrinter *pPrinter, QPainter & pPainter);

    /**
     * Overrides the standard operation.
     */
    void hideEvent(QHideEvent *he);

    /**
     * Overrides the standard operation.
     */
    void showEvent(QShowEvent *se);

    /**
     * Sees if a message is relevant to the given widget.  If it does delete it.
     * @param w The widget to check messages against.
     */
    void checkMessages(ObjectWidget * w);

    /**
     * Finds a widget with the given ID.
     *
     * @param id The ID of the widget to find.
     *
     * @return Returns the widget found, returns 0 if no widget found.
     */
    UMLWidget * findWidget(Uml::IDType id);

    /**
     * Finds an association widget with the given ID.
     *
     * @param id The ID of the widget to find.
     *
     * @return Returns the widget found, returns 0 if no widget found.
     */
    AssociationWidget * findAssocWidget(Uml::IDType id);

    /**
     * Finds an association widget with the given type and widgets.
     *
     * @param at  The AssociationType of the widget to find.
     * @param pWidgetA Pointer to the UMLWidget of role A.
     * @param pWidgetB Pointer to the UMLWidget of role B.
     *
     * @return Returns the widget found, returns 0 if no widget found.
     */
    AssociationWidget * findAssocWidget(Uml::AssociationType at,
                                        UMLWidget *pWidgetA, UMLWidget *pWidgetB);

    /**
     * Finds an association widget with the given widgets and the given role B name.
     * Considers the following association types:
     *  at_Association, at_UniAssociation, at_Composition, at_Aggregation
     * This is used for seeking an attribute association.
     *
     * @param pWidgetA  Pointer to the UMLWidget of role A.
     * @param pWidgetB  Pointer to the UMLWidget of role B.
     * @param roleNameB Name at the B side of the association (the attribute name)
     *
     * @return Returns the widget found, returns 0 if no widget found.
     */
    AssociationWidget * findAssocWidget(UMLWidget *pWidgetA,
                                        UMLWidget *pWidgetB, const QString& roleNameB);

    /**
     * Remove a widget from view.
     *
     * @param o  The widget to remove.
     */
    void removeWidget(UMLWidget * o);

    /**
     * Sets a widget to a selected state and adds it to a list of selected widgets.
     * This method also sets the state of the cut and copy menu entries.
     *
     * @param w The widget to set to selected.
     * @param me The mouse event containing the information about the selection.
     */
    void setSelected(UMLWidget * w, QMouseEvent * me);

    /**
     *  Clear the selected widgets list.
     */
    void clearSelected();

    /**
     * Move all the selected widgets by a relative X and Y offset.
     *
     * @param dX The distance to move horizontally.
     * @param dY The distance to move vertically.
     */
    void moveSelectedBy(int dX, int dY);

    /**
     * Return the amount of widgets selected.
     *
     * @param filterText  When true, do NOT count floating text widgets that
     *                    belong to other widgets (i.e. only count tr_Floating.)
     *                    Default: Count all widgets.
     * @return  Number of widgets selected.
     */
    int getSelectCount(bool filterText = false) const;

    /**
     * Set the useFillColor variable to all selected widgets
     *
     * @param useFC The state to set the widget to.
     */
    void selectionUseFillColor(bool useFC);

    /**
     * Set the font for all the currently selected items.
     */
    void selectionSetFont( const QFont &font );

    /**
     * Set the line color for all the currently selected items.
     */
    void selectionSetLineColor( const QColor &color );

    /**
     * Set the line width for all the currently selected items.
     */
    void selectionSetLineWidth( uint width );

    /**
     * Set the fill color for all the currently selected items.
     */
    void selectionSetFillColor( const QColor &color );

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
    Uml::IDType getLocalID();

    /**
     * Returns whether a widget is already on the diagram.
     *
     * @param id The id of the widget to check for.
     *
     * @return Returns true if the widget is already on the diagram, false if not.
     */
    bool widgetOnDiagram(Uml::IDType id);

    /**
     * Returns true if this diagram resides in an externalized folder.
     * CHECK: It is probably cleaner to move this to the UMLListViewItem.
     */
    bool isSavedInSeparateFile();

    /**
     * Get the pos variable.  Used internally to keep track of the cursor.
     */
    QPoint & getPos() {
        return m_Pos;
    }

    /**
     * Set the pos variable.  Used internally to keep track of the cursor.
     *
     * @param _pos The position to set to.
     */
    void setPos(const QPoint &_pos) {
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
     * Sets the status on whether in a paste state.
     */
    void setPaste(bool paste) {
        m_bPaste = paste;
    }

    /**
     * Returns a List of all the UMLObjects(Use Cases, Concepts and Actors) in the View
     */
    UMLObjectList umlObjects();

    /**
     * Activate all the objects and associations after a load from the clipboard
     */
    void activate();

    /**
     * Returns a list with all the selected associations from the diagram
     */
    AssociationWidgetList getSelectedAssocs();

    /**
     * Fills the List with all the selected widgets from the diagram
     * The list can be filled with all the selected widgets, or be filtered to prevent
     * text widgets other than tr_Floating to be append.
     *
     * @param WidgetList The UMLWidgetList to fill.
     * @param filterText Don't append the text, unless their role is tr_Floating
     */
    bool getSelectedWidgets(UMLWidgetList& WidgetList, bool filterText = true);

    /**
     * Activate the view after a load a new file
     */
    void activateAfterLoad( bool bUseLog = false );

    void endPartialWidgetPaste();
    void beginPartialWidgetPaste();

    /**
     * Removes a AssociationWidget from a diagram
     * Physically deletes the AssociationWidget passed in.
     *
     * @param pAssoc  Pointer to the AssociationWidget.
     */
    void removeAssoc(AssociationWidget* pAssoc);

    /**
     * Removes all the associations related to Widget
     *
     * @param pWidget  Pointer to the widget to remove.
     */
    void removeAssociations(UMLWidget* pWidget);

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
     * Returns the imageExporter used to export the view.
     *
     * @return The imageExporter used to export the view.
     */
    UMLViewImageExporter* getImageExporter();

    /**
     * Adds an association to the view from the given data.
     * Use this method when pasting.
     */
    bool addAssociation( AssociationWidget* pAssoc , bool isPasteOperation = false);

    /**
     * Removes an AssociationWidget from the association list
     * and removes the corresponding UMLAssociation from the current UMLDoc.
     */
    void removeAssocInViewAndDoc(AssociationWidget* assoc);

    /**
     * Adds a widget to the view from the given data.
     * Use this method when pasting.
     */
    bool addWidget( UMLWidget * pWidget , bool isPasteOperation = false);

    /**
     * Returns the offset point at which to place the paste from clipboard.
     * Just add the amount to your co-ords.
     * Only call this straight after the event, the value won't stay valid.
     * Should only be called by Assoc widgets at the moment. no one else needs it.
     */
    QPoint getPastePoint();

    /**
     * Reset the paste point.
     */
    void resetPastePoint();

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
     * If the m_Type of the given widget is Uml::wt_Class then
     * iterate through the class' attributes and create an
     * association to each attribute type widget that is present
     * on the current diagram.
     */
    void createAutoAttributeAssociations(UMLWidget *widget);

    void createAutoConstraintAssociations(UMLWidget* widget);


    /**
     * Refreshes containment association, i.e. removes possible old
     * containment and adds new containment association if applicable.
     *
     * @param self  Pointer to the contained object for which
     *   the association to the containing object is
     *   recomputed.
     */
    void updateContainment(UMLCanvasObject *self);

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
     * Sets some options for all the @ref ClassifierWidget on the view.
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
     * Asks for confirmation and clears everything on the diagram.
     * Called from menus.
     */
    void clearDiagram();

    /**
     * Changes snap to grid boolean.
     * Called from menus.
     */
    void toggleSnapToGrid();

    /**
     * Changes snap to grid for component size boolean.
     * Called from menus.
     */
    void toggleSnapComponentSizeToGrid();

    /**
     *  Changes show grid boolean.
     * Called from menus.
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
     * The width and height of a diagram canvas in pixels.
     */
    static const int defaultCanvasSize;

    // Load/Save interface:

    /**
     * Creates the "diagram" tag and fills it with the contents of the diagram.
     */
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the "diagram" tag.
     */
    virtual bool loadFromXMI( QDomElement & qElement );

    /**
     * Loads the "UISDiagram" tag of Unisys.IntegratePlus.2 generated files.
     */
    bool loadUISDiagram(QDomElement & qElement);

    /**
     * Loads a "widget" element from XMI, used by loadFromXMI() and the clipboard.
     */
    UMLWidget* loadWidgetFromXMI(QDomElement& widgetElement);

    /**
     * Add an object to the application, and update the view.
     */
    void addObject(UMLObject *object);

    /**
     * Selects all the widgets within an internally kept rectangle.
     */
    void selectWidgets(int px, int py, int qx, int qy);

    /**
     * Selects all the widgets of the given association widget.
     */
    void selectWidgetsOfAssoc (AssociationWidget * a);

    /**
     * Selects all the widgets from a list.
     */
    void selectWidgets(UMLWidgetList &widgets);

    /**
     * Determine whether on a sequence diagram we have clicked on a line
     * of an Object.
     *
     * @return The widget thats line was clicked on.
     *  Returns 0 if no line was clicked on.
     */
    ObjectWidget * onWidgetLine( const QPoint &point );

    /**
     * Determine whether on a sequence diagram we have clicked on
     * the destruction box of an Object.
     *
     * @return The widget thats destruction box was clicked on.
     *  Returns 0 if no destruction box was clicked on.
     */
    ObjectWidget * onWidgetDestructionBox(const QPoint &point );

    /**
     * Return pointer to the first selected widget (for multi-selection)
     */
    UMLWidget* getFirstMultiSelectedWidget() {
        return m_SelectedList.first();
    }

    /**
     * Tests the given point against all widgets and returns the
     * widget for which the point is within its bounding rectangle.
     * In case of multiple matches, returns the smallest widget.
     * Returns NULL if the point is not inside any widget.
     * Does not use or modify the m_pOnWidget member.
     */
    UMLWidget *widgetAt(const QPoint& p);

    /**
     * Initialize and announce a newly created widget.
     * Auxiliary to contentsMouseReleaseEvent().
     */
    void setupNewWidget(UMLWidget *w);

    /**
     * Return whether we are currently creating an object.
     */
    bool getCreateObject() const {
        return m_bCreateObject;
    }

    /**
     * Set whether we are currently creating an object.
     */
    void setCreateObject(bool bCreate) {
        m_bCreateObject = bCreate;
    }

    /**
     * Emit the sigRemovePopupMenu Qt signal.
     */
    void emitRemovePopupMenu() {
        emit sigRemovePopupMenu();
    }

    /**
     * Used for creating unique name of collaboration messages.
     */
    int generateCollaborationId();

    /**
     * Return the UMLDoc pointer
     */
    UMLDoc* getUMLDoc() {
        return m_doc;
    }

protected:

    // Methods and members related to loading/saving

    bool loadWidgetsFromXMI( QDomElement & qElement );
    bool loadMessagesFromXMI( QDomElement & qElement );
    bool loadAssociationsFromXMI( QDomElement & qElement );
    bool loadUisDiagramPresentation(QDomElement & qElement);

    /**
     * Contains the unique ID to allocate to a widget that needs an
     * ID for the view.  @ref ObjectWidget is an example of this.
     */
    Uml::IDType m_nLocalID;

    /**
     * The ID of the view.  Allocated by @ref UMLDoc
     */
    Uml::IDType m_nID;

    /**
     * The type of diagram to represent.
     */
    Uml::DiagramType m_Type;

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
    Settings::OptionState m_Options;

    /**
     * Contains all the message widgets on the diagram.
     */
    MessageWidgetList m_MessageList;

    /**
     * Contains all the UMLWidgets on the diagram.
     */
    UMLWidgetList m_WidgetList;

    /**
     * Contains all the AssociationWidgets on the diagram.
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
     * Determines whether the view is shown to the user, i.e. opened in a tab.
     */
    bool m_isOpen;

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

    void closeEvent(QCloseEvent * e);
    void dragEnterEvent(QDragEnterEvent* enterEvent);
    void dragMoveEvent(QDragMoveEvent* moveEvent);
    void dropEvent(QDropEvent* dropEvent);

    QRect diagramRect();

    /**
     * Overrides the standard operation.
     * Calls the same method in the current tool bar state.
     */
    void contentsMouseReleaseEvent(QMouseEvent* mouseEvent);

    /**
     * Overrides the standard operation.
     * Calls the same method in the current tool bar state.
     */
    void contentsMouseMoveEvent(QMouseEvent* mouseEvent);

    /**
     * Override standard method.
     * Calls the same method in the current tool bar state.
     */
    void contentsMouseDoubleClickEvent(QMouseEvent* mouseEvent);

    /**
     * Override standard method.
     * Calls the same method in the current tool bar state.
     */
    void contentsMousePressEvent(QMouseEvent* mouseEvent);

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
     * Find the maximum bounding rectangle of FloatingTextWidget widgets.
     * Auxiliary to copyAsImage().
     *
     * @param ft Pointer to the FloatingTextWidget widget to consider.
     * @param px  X coordinate of lower left corner. This value will be
     *            updated if the X coordinate of the lower left corner
     *            of ft is smaller than the px value passed in.
     * @param py  Y coordinate of lower left corner. This value will be
     *            updated if the Y coordinate of the lower left corner
     *            of ft is smaller than the py value passed in.
     * @param qx  X coordinate of upper right corner. This value will be
     *            updated if the X coordinate of the upper right corner
     *            of ft is larger than the qx value passed in.
     * @param qy  Y coordinate of upper right corner. This value will be
     *            updated if the Y coordinate of the upper right corner
     *            of ft is larger than the qy value passed in.
     */
    void findMaxBoundingRectangle(const FloatingTextWidget* ft,
                                  int& px, int& py, int& qx, int& qy);

    void forceUpdateWidgetFontMetrics(QPainter *painter);

    /**
     * Used for creating unique name of collaboration messages.
     */
    int m_nCollaborationId;

    QPoint m_Pos;
    bool m_bCreateObject, m_bDrawSelectedOnly, m_bPaste;
    ListPopupMenu * m_pMenu;
    UMLWidgetList m_SelectedList;

    /**
     *  Flag if view/children started cut operation.
     */
    bool m_bStartedCut;

private:
    /**
     * The folder in which this UMLView is contained
     */
    UMLScene *m_scene;
    /**
     * The folder in which this UMLView is contained
     */
    UMLFolder *m_pFolder;

    /**
     * set to true when a child has used the showDocumentation method,
     * thus when one clicks on a child widget.
     * Reset to false when clicking in an empty region of the view.
     */
    bool m_bChildDisplayedDoc;

    ToolBarStateFactory* m_pToolBarStateFactory;
    ToolBarState* m_pToolBarState;

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
     * Status of a popupmenu on view.
     * true - a popup is on view
     */
    bool m_bPopupShowing;

    /**
     * The offset at which to paste the clipboard.
     */
    QPoint m_PastePoint;

    /**
     * Pointer to the UMLDoc
     */
    UMLDoc* m_doc;

    /**
     * The UMLViewImageExporter used to export the view.
     */
    UMLViewImageExporter* m_pImageExporter;

    /**
     * Create an association with the attribute attr associated with the UMLWidget
     * widget if the UMLClassifier type is present on the current diagram.
     */
    void createAutoAttributeAssociation(UMLClassifier *type,
                                        UMLAttribute *attr,
                                        UMLWidget *widget);

    void createAutoConstraintAssociation(UMLEntity* refEntity,
                                         UMLForeignKeyConstraint* fkConstraint,
                                         UMLWidget* widget);

    /**
     * Returns true if the first widget's X is smaller than second's.
     * Used for sorting the UMLWidgetList.
     *
     * @param widget1 The widget to compare.
     * @param widget2 The widget to compare with.
     */
    static bool hasWidgetSmallerX(const UMLWidget* widget1, const UMLWidget* widget2);

    /**
     * Returns true if the first widget's Y is smaller than second's.
     * Used for sorting the UMLWidgetList.
     *
     * @param widget1 The widget to compare.
     * @param widget2 The widget to compare with.
     */
    static bool hasWidgetSmallerY(const UMLWidget* widget1, const UMLWidget* widget2);

    /**
     * Looks for the smallest x-value of the given UMLWidgets.
     *
     * @param widgetList A list with UMLWidgets.
     */
    int getSmallestX(const UMLWidgetList &widgetList);

    /**
     * Looks for the smallest y-value of the given UMLWidgets.
     *
     * @param widgetList A list with UMLWidgets.
     */
    int getSmallestY(const UMLWidgetList &widgetList);

    /**
     * Looks for the biggest x-value of the given UMLWidgets.
     *
     * @param widgetList A list with UMLWidgets.
     */
    int getBiggestX(const UMLWidgetList &widgetList);

    /**
     * Looks for the biggest y-value of the given UMLWidgets.
     *
     * @param widgetList A list with UMLWidgets.
     */
    int getBiggestY(const UMLWidgetList &widgetList);

    /**
     * Returns the sum of the heights of the given UMLWidgets
     *
     * @param widgetList A list with UMLWidgets.
     */
    int getHeightsSum(const UMLWidgetList &widgetList);

    /**
     * Returns the sum of the widths of the given UMLWidgets.
     *
     * @param widgetList A list with UMLWidgets.
     */
    int getWidthsSum(const UMLWidgetList &widgetList);

    /**
     * Sorts the given UMLWidgetList based on the Compare function.
     * The list is cleared and all the widgets are added again in order.
     *
     * The comp function gets two const UMLWidget* params and returns
     * a boolean telling if the first widget was smaller than the second,
     * whatever the "smaller" concept is depending on the sorting to do.
     *
     * @param widgetList The list with the widgets to order.
     * @param comp The comp function to compare the widgets.
     */
    template<typename Compare>
    void sortWidgetList(UMLWidgetList &widgetList, Compare comp);

public slots:
    void zoomIn();
    void zoomOut();
    void show();

    /**
     * Changes the current tool to the selected tool.
     * The current tool is cleaned and the selected tool initialized.
     */
    void slotToolBarChanged(int c);
    void slotObjectCreated(UMLObject * o);
    void slotObjectRemoved(UMLObject * o);

    /**
     * When a menu selection has been made on the menu
     * that this view created, this method gets called.
     */
    void slotMenuSelection(QAction* action);

    /**
     * This slot is entered when an event has occurred on the views display,
     * most likely a mouse event.  Before it sends out that mouse event everyone
     * that displays a menu on the views surface (widgets and this) should remove any
     * menu.  This stops more than one menu being displayed.
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
     * Left Alignment
     */
    void alignLeft();

    /**
     * Right Alignment
     */
    void alignRight();

    /**
     * Top Alignment
     */
    void alignTop();

    /**
     * Bottom Alignment
     */
    void alignBottom();

    /**
     * Vertical Middle Alignment
     */
    void alignVerticalMiddle();

    /**
     * Horizontal Middle Alignment
     */
    void alignHorizontalMiddle();

    /**
     * Vertical Distribute Alignment
     */
    void alignVerticalDistribute();

    /**
     * Horizontal Distribute Alignment
     */
    void alignHorizontalDistribute();

signals:
    void sigResetToolBar();
    void sigColorChanged( Uml::IDType );
    void sigRemovePopupMenu();
    void sigClearAllSelected();
    void sigLineWidthChanged( Uml::IDType );
    void sigSnapToGridToggled(bool);
    void sigSnapComponentSizeToGridToggled(bool);
    void sigShowGridToggled(bool);

    /**
     * Emitted when an association is removed.
     */
    void sigAssociationRemoved(AssociationWidget*);

    /**
     * Emitted when a widget is removed.
     */
    void sigWidgetRemoved(UMLWidget*);
};

#endif // UMLVIEW_H
