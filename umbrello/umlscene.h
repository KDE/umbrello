/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLSCENE_H
#define UMLSCENE_H

// local includes
#include "umlnamespace.h"
#include "optionstate.h"
#include "umlobjectlist.h"
#include "umlwidgetlist.h"
#include "associationwidgetlist.h"
#include "messagewidgetlist.h"
#include "worktoolbar.h"

// Qt includes
#include <QtGui/QGraphicsScene>
#include <QtXml/QDomDocument>

// forward declarations
class QPrinter;
class ClassOptionsPage;
class IDChangeLog;
class ListPopupMenu;
class FloatingTextWidget;
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
class UMLView;

/**
 * UMLScene instances represent diagrams.
 * The UMLScene class inherits from QGraphicsScene.
 */
class UMLScene : public QGraphicsScene
{
    Q_OBJECT
public:
    friend class UMLViewImageExporterModel;

    UMLScene(UMLFolder *parentFolder);

    virtual ~UMLScene();

    // Accessors and other methods dealing with loaded/saved data

    /**
     * Return the UMLFolder in which this diagram lives.
     */
     UMLFolder *getFolder() {
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
     QString getDoc() const {
         return m_Documentation;
     }
 
    /**
     * Set the documentation of the diagram.
     */
     void setDoc(const QString &doc) {
         m_Documentation = doc;
     }
 
     QString getName() const;
     void setName(const QString &name);
 
    /**
     * Returns the type of the diagram.
     */
     Uml::Diagram_Type getType() const {
         return m_Type;
     }
 
    /**
     * Set the type of diagram.
     */
    void setType(Uml::Diagram_Type type) {
        m_Type = type;
        test();
    }

    /**
     * Returns the fill color to use.
     */
     QColor getFillColor() const;
 
    /**
     * Returns the default brush for diagram widgets.
     */
    QBrush brush() const {
        // TODO: Remove getFillColor()
        return getFillColor();
    }

    /**
     * Set the background color.
     *
     * @param color  The color to use.
     */
     void setFillColor(const QColor &color);

    /**
     * Returns the line color to use.
     */
     QColor getLineColor() const;

    /**
     * Sets the line color.
     *
     * @param color  The color to use.
     */
     void setLineColor(const QColor &color);

    /**
     * Returns the line width to use.
     */
     uint getLineWidth() const;

    /**
     * Sets the line width.
     *
     * @param width  The width to use.
     */
     void setLineWidth(uint width);

     QColor getTextColor() const;
     void setTextColor(const QColor& color);

    /**
     * Returns the ID of the diagram.
     */
     Uml::IDType getID() const {
         return m_nID;
     }
 
    /**
     * Sets the ID of the diagram.
     */
     void setID(Uml::IDType id) {
         m_nID = id;
     }
 
    /**
     * Returns height of the diagram canvas.
     */
     qreal canvasHeight() const {
         return sceneRect().height();
     }
 
    /**
     * Returns width of the diagram canvas.
     */
    qreal canvasWidth() const {
        return sceneRect().width();
    }

    /**
     * Return whether to use snap to grid.
     */
    bool getSnapToGrid() const {
        return m_bUseSnapToGrid;
    }

    void setSnapToGrid(bool bSnap);

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
     * Sets the x grid size.
     */
    void setSnapX(int x) {
        m_nSnapX = x;
        update();
    }

    /**
     * Returns the y grid size.
     */
    int getSnapY() const {
        return m_nSnapY;
    }

    /**
     * Sets the y grid size.
     */
    void setSnapY(int y) {
        m_nSnapY = y;
        update();
    }

    qreal snappedX(qreal x);
    qreal snappedY(qreal y);

    bool getShowSnapGrid() const;
    void setShowSnapGrid(bool bShow);

    void setSnapComponentSizeToGrid(bool bSnap);

    bool getUseFillColor() const;
    void setUseFillColor(bool ufc);

    QFont getFont() const;
    void setFont(QFont font, bool changeAllWidgets = false);

    bool getShowOpSig() const;
    void setShowOpSig(bool bShowOpSig);

    /**
     * Returns the options being used.
     */
    const Settings::OptionState& getOptionState() const {
        return m_Options;
    }

    /**
     * Sets the options to be used.
     */
    void setOptionState(const Settings::OptionState& options) {
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

    // End of accessors and methods that only deal with loaded/saved data
    ////////////////////////////////////////////////////////////////////////

    void print(QPrinter *pPrinter, QPainter & pPainter);

    void checkMessages(ObjectWidget * w);

    UMLWidget* findWidget(Uml::IDType id);

    AssociationWidget* findAssocWidget(Uml::IDType id);

    AssociationWidget* findAssocWidget(Uml::Association_Type at,
                                       UMLWidget *pWidgetA, UMLWidget *pWidgetB);

    AssociationWidget* findAssocWidget(UMLWidget *pWidgetA,
                                       UMLWidget *pWidgetB, const QString& roleNameB);

    void removeWidget(UMLWidget *o);

    void setSelected(UMLWidget *w, QGraphicsSceneMouseEvent *me);

    UMLWidgetList selectedWidgets() const;

    void clearSelected();

    void moveSelectedBy(qreal dX, qreal dY);

    int getSelectCount(bool filterText = false) const;

    void selectionUseFillColor(bool useFC);
    void selectionSetFont(const QFont &font);
    void selectionSetLineColor(const QColor &color);
    void selectionSetLineWidth(uint width);
    void selectionSetFillColor(const QColor &color);
    void selectionToggleShow(int sel);

    void deleteSelection();

    void selectAll();

    Uml::IDType getLocalID();

    bool widgetOnDiagram(Uml::IDType id);

    bool isSavedInSeparateFile();

    QPointF & getPos() {
        return m_Pos;
    }

    void setPos(const QPointF &_pos) {
        m_Pos = _pos;
    }

    UMLView* activeView() const;

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

    UMLObjectList getUMLObjects();

    void activate();

    AssociationWidgetList getSelectedAssocs();

    bool getSelectedWidgets(UMLWidgetList& WidgetList, bool filterText = true);

    void activateAfterLoad(bool bUseLog = false);

    void endPartialWidgetPaste();
    void beginPartialWidgetPaste();

    void removeAssoc(AssociationWidget* pAssoc);

    void removeAssociations(UMLWidget* pWidget);

    void selectAssociations(bool bSelect);

    void getWidgetAssocs(UMLObject* Obj, AssociationWidgetList & Associations);

    void removeAllAssociations();

    void removeAllWidgets();

    void showDocumentation(UMLObject * object, bool overwrite);
    void showDocumentation(UMLWidget * widget, bool overwrite);
    void showDocumentation(AssociationWidget * widget, bool overwrite);

    void updateDocumentation(bool clear);

    void getDiagram(const QRectF &rect, QPixmap & diagram);
    void getDiagram(const QRectF &area, QPainter & painter);

    void copyAsImage(QPixmap*& pix);

    UMLViewImageExporter* getImageExporter();

    bool addAssociation(AssociationWidget* pAssoc , bool isPasteOperation = false);

    void removeAssocInViewAndDoc(AssociationWidget* assoc);

    bool addWidget(UMLWidget * pWidget , bool isPasteOperation = false);

    QPointF getPastePoint();

    void resetPastePoint();

    /**
     * Called by the view or any of its children when they start a cut
     * operation.
     */
    void setStartedCut() {
        m_bStartedCut = true;
    }

    void createAutoAssociations(UMLWidget * widget);
    void createAutoAttributeAssociations(UMLWidget *widget);
    void createAutoConstraintAssociations(UMLWidget* widget);

    void updateContainment(UMLCanvasObject *self);

    bool showPropDialog();

    void setClassWidgetOptions(ClassOptionsPage * page);

    void checkSelections();

    bool checkUniqueSelection();

    void clearDiagram();

    void toggleSnapToGrid();
    void toggleSnapComponentSizeToGrid();
    void toggleShowGrid();

    void fileLoaded();

    void resizeCanvasToItems();

    /**
     * The width and height of a diagram canvas in pixels.
     */
    static const qreal defaultCanvasSize;

    // Load/Save interface:

    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);
    virtual bool loadFromXMI(QDomElement & qElement);

    bool loadUISDiagram(QDomElement & qElement);

    UMLWidget* loadWidgetFromXMI(QDomElement& widgetElement);

    void addObject(UMLObject *object);

    void selectWidgets(qreal px, qreal py, qreal qx, qreal qy);

    ObjectWidget * onWidgetLine(const QPointF &point) const;
    ObjectWidget * onWidgetDestructionBox(const QPointF &point) const;

    UMLWidget* getFirstMultiSelectedWidget() const;

    UMLWidget *getWidgetAt(const QPointF& p);

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

    int generateCollaborationId();

    /**
     * Return the UMLDoc pointer
     */
    UMLDoc* getUMLDoc() {
        return m_pDoc;
    }

    void callBaseMouseMethod(QGraphicsSceneMouseEvent *event);

    bool isMouseMovingItems() const;
    void setIsMouseMovingItems(bool b);

protected:

    void drawBackground(QPainter *p, const QRectF& rect);

    // Methods and members related to loading/saving

    bool loadWidgetsFromXMI(QDomElement & qElement);
    bool loadMessagesFromXMI(QDomElement & qElement);
    bool loadAssociationsFromXMI(QDomElement & qElement);

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
    Uml::Diagram_Type m_Type;

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

    // End of methods and members related to loading/saving
    ////////////////////////////////////////////////////////////////////////

    void dragEnterEvent(QGraphicsSceneDragDropEvent *enterEvent);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *moveEvent);
    void dropEvent(QGraphicsSceneDragDropEvent *dropEvent);

    QRectF getDiagramRect();

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);

    /**
     * This variable is set/reset by items which would call
     * @ref UMLScene::setIsMouseMovingItems in its mouse events.
     */
    bool m_isMouseMovingItems;
    bool isArrowMode();

    void selectWidgetsOfAssoc (AssociationWidget * a);

    void makeSelected (UMLWidget * uw);

    void updateComponentSizes();

    void findMaxBoundingRectangle(const FloatingTextWidget* ft,
                                  qreal& px, qreal& py, qreal& qx, qreal& qy);

    void forceUpdateWidgetFontMetrics(QPainter *painter);

    /**
     * Used for creating unique name of collaboration messages.
     */
    int m_nCollaborationId;

    QPointF m_Pos;
    bool m_bCreateObject, m_bDrawSelectedOnly, m_bPaste;
    ListPopupMenu * m_pMenu;

    /**
     *  Flag if view/children started cut operation.
     */
    bool m_bStartedCut;

private:

    /**
     * The folder in which this UMLScene is contained
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
    QPointF m_PastePoint;

    /**
     * Pointer to the UMLDoc
     */
    UMLDoc* m_pDoc;

    /**
     * The UMLViewImageExporter used to export the view.
     */
    UMLViewImageExporter* m_pImageExporter;

    void createAutoAttributeAssociation(UMLClassifier *type,
                                        UMLAttribute *attr,
                                        UMLWidget *widget);

    void createAutoConstraintAssociation(UMLEntity* refEntity,
                                         UMLForeignKeyConstraint* fkConstraint,
                                         UMLWidget* widget);

public slots:
    void slotToolBarChanged(int c);
    void slotObjectCreated(UMLObject * o);
    void slotObjectRemoved(UMLObject * o);
    void slotMenuSelection(QAction* action);
    void slotRemovePopupMenu();
    void slotActivate();
    void slotCutSuccessful();
    void slotShowView();

    void alignLeft();
    void alignRight();
    void alignTop();
    void alignBottom();
    void alignVerticalMiddle();
    void alignHorizontalMiddle();
    void alignVerticalDistribute();
    void alignHorizontalDistribute();

    void test();

signals:
    void sigResetToolBar();
    void sigColorChanged(Uml::IDType);
    void sigRemovePopupMenu();
    void sigClearAllSelected();
    void sigLineWidthChanged(Uml::IDType);
    void sigSnapToGridToggled(bool);
    void sigSnapComponentSizeToGridToggled(bool);
    void sigShowGridToggled(bool);
    void sigAssociationRemoved(AssociationWidget*);
    void sigWidgetRemoved(UMLWidget*);
};

#endif // UMLSCENE_H
