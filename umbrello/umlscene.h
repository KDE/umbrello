/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2010                                               *
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

// Qt includes
#include <QtGui/QGraphicsScene>
#include <QtXml/QDomDocument>

// forward declarations
class QAction;
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

    UMLFolder* folder() const; 
    void setFolder(UMLFolder *folder);
 
    QString documentation() const;
    void setDocumentation(const QString &doc);
 
    QString name() const;
    void setName(const QString &name);
 
    Uml::Diagram_Type type() const;
    void setType(Uml::Diagram_Type type);

    Uml::IDType getID() const;
    void setID(Uml::IDType id);

    QPointF pos() const;
    void setPos(const QPointF &pos);

    QColor fillColor() const;
    void setFillColor(const QColor &color);
 
    /**
     * Returns the default brush for diagram widgets.
     */
    QBrush brush() const {
        // TODO: Remove fillColor()
        return fillColor();
    }

    QColor lineColor() const;
    void setLineColor(const QColor &color);

    uint lineWidth() const;
    void setLineWidth(uint width);

    QColor textColor() const;
    void setTextColor(const QColor& color);
 
    bool getSnapToGrid() const;
    void setSnapToGrid(bool bSnap);

    bool getSnapComponentSizeToGrid() const;
    void setSnapComponentSizeToGrid(bool bSnap);

    int getSnapX() const;
    void setSnapX(int x);
    int getSnapY() const;
    void setSnapY(int y);

    qreal snappedX(qreal x);
    qreal snappedY(qreal y);

    bool getShowSnapGrid() const;
    void setShowSnapGrid(bool bShow);

    bool getUseFillColor() const;
    void setUseFillColor(bool ufc);

    QFont getFont() const;
    void setFont(QFont font, bool changeAllWidgets = false);

    bool getShowOpSig() const;
    void setShowOpSig(bool bShowOpSig);

    const Settings::OptionState& optionState() const;
    void setOptionState(const Settings::OptionState& options);

    AssociationWidgetList& associationList();
    UMLWidgetList& widgetList();
    MessageWidgetList& messageList();

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

    Uml::IDType localID();

    bool widgetOnDiagram(Uml::IDType id);

    bool isSavedInSeparateFile();

    UMLView* activeView() const;

    void setMenu();

    void resetToolbar();

    bool getPaste() const;
    void setPaste(bool paste);

    UMLObjectList umlObjects();

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

    void setStartedCut();

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

    UMLWidget* widgetAt(const QPointF& p);

    void setupNewWidget(UMLWidget *w);

    bool getCreateObject() const;
    void setCreateObject(bool bCreate);

    /**
     * Emit the sigRemovePopupMenu Qt signal.
     */
    void emitRemovePopupMenu() {
        emit sigRemovePopupMenu();
    }

    int generateCollaborationId();

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

    Uml::IDType m_nID;                ///< The ID of the view. Allocated by @ref UMLDoc.
    Uml::Diagram_Type m_Type;         ///< The type of diagram to represent.
    QString m_Name;                   ///< The name of the diagram.
    QString m_Documentation;          ///< The documentation of the diagram.
    Settings::OptionState m_Options;  ///< Options used by view.

    MessageWidgetList m_MessageList;  ///< All the message widgets on the diagram.
    UMLWidgetList m_WidgetList;       ///< All the UMLWidgets on the diagram.
    AssociationWidgetList m_AssociationList;  ///< All the AssociationWidgets on the diagram.

    int m_nSnapX;  ///< The snap to grid x size.
    int m_nSnapY;  ///< The snap to grid y size.

    bool m_bUseSnapToGrid;  ///< Flag to use snap to grid. The default is off.
    bool m_bUseSnapComponentSizeToGrid;  ///< Flag to use snap to grid for component size. The default is off.
    bool m_bShowSnapGrid;  ///< Flag to show the snap grid. The default will be on if the grid is on.

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

    int m_nCollaborationId;  ///< Used for creating unique name of collaboration messages.
    QPointF m_Pos;
    bool m_bCreateObject;
    bool m_bDrawSelectedOnly;
    bool m_bPaste;
    ListPopupMenu * m_pMenu;
    bool m_bStartedCut;  ///< Flag if view/children started cut operation.

private:

    static const qreal DEFAULT_CANVAS_SIZE;  ///< The default size of a diagram in pixels.

    UMLFolder *m_pFolder;  ///< The folder in which this UMLScene is contained.

    /**
     * set to true when a child has used the showDocumentation method,
     * thus when one clicks on a child widget.
     * Reset to false when clicking in an empty region of the view.
     */
    bool m_bChildDisplayedDoc;

    ToolBarStateFactory* m_pToolBarStateFactory;
    ToolBarState* m_pToolBarState;
    IDChangeLog * m_pIDChangesLog;  ///< LocalID Changes Log for paste actions
    bool m_isActivated;             ///< True if the view was activated after the serialization(load).
    bool m_bPopupShowing;           ///< Status of a popupmenu on view. True - a popup is on view.
    QPointF m_PastePoint;           ///< The offset at which to paste the clipboard.
    UMLDoc* m_pDoc;                 ///< Pointer to the UMLDoc.
    UMLViewImageExporter* m_pImageExporter;  ///< Used to export the view.

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
