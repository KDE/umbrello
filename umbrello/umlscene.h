/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef UMLSCENE_H
#define UMLSCENE_H

// local includes
#include "associationwidgetlist.h"
#include "messagewidgetlist.h"
#include "optionstate.h"
#include "umlobjectlist.h"
#include "umlwidgetlist.h"
#include "worktoolbar.h"

// Qt includes
#include <QMouseEvent>
#include <QKeyEvent>
#include <QtGui/QPixmap>
#include <Q3Canvas>
#include <QtXml/QDomDocument>

// forward declarations
class ClassOptionsPage;
class IDChangeLog;
class ListPopupMenu;
class FloatingTextWidget;
class ObjectWidget;
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

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QHideEvent;
class QMouseEvent;
class QPrinter;
class QShowEvent;
class UMLScene;

/// uml related types - makes it easier to switch to QGraphicsScene types
// base types
typedef QPoint UMLScenePoint;
typedef QRect UMLSceneRect;
typedef QSize UMLSceneSize;
typedef QLine UMLSceneLine;
typedef int UMLSceneValue;

// event types
typedef QKeyEvent UMLSceneKeyEvent;
typedef QHoverEvent UMLSceneHoverEvent;
typedef QContextMenuEvent UMLSceneContextMenuEvent;
//typedef QDragDropEvent UMLSceneDragDropEvent;

typedef Q3CanvasItem UMLSceneItem;
typedef Q3CanvasItemList UMLSceneItemList;

/**
 * UMLScene instances represent diagrams.
 * The UMLScene class inherits from Q3CanvasView and
 * in the future from QGraphicsScene.
 */
class UMLScene : public Q3Canvas
{
    Q_OBJECT
public:
    friend QDebug operator<<(QDebug out, const UMLScene *item);
    friend class UMLViewImageExporterModel;

    UMLScene(UMLFolder *parentFolder, UMLView *view=0);
    virtual ~UMLScene();

    Q3Canvas *canvas()
    {
        return this;
    }

    UMLView *view();
    void setView(UMLView *view);

    // Accessors and other methods dealing with loaded/saved data

    UMLFolder* folder() const; 
    void setFolder(UMLFolder *folder);

    QString documentation() const;
    void setDocumentation(const QString &doc);

    QString name() const;
    void setName(const QString &name);

    Uml::DiagramType type() const;
    void setType(Uml::DiagramType type);

    Uml::IDType getID() const;
    void setID(Uml::IDType id);

    UMLScenePoint pos() const;
    void setPos(const UMLScenePoint &pos);

    const QColor& fillColor() const;
    void setFillColor(const QColor &color);

    const QColor& lineColor() const;
    void setLineColor(const QColor &color);

    uint lineWidth() const;
    void setLineWidth(uint width);

    const QColor& textColor() const;
    void setTextColor(const QColor& color);

    const QColor& gridDotColor() const;
    void setGridDotColor(const QColor &gridColor);

    void setSize(int width, int height);

    bool snapToGrid() const;
    void setSnapToGrid(bool bSnap);

    bool snapComponentSizeToGrid() const;
    void setSnapComponentSizeToGrid(bool bSnap);

    int snapX() const;
    int snapY() const;
    void setSnapX(int x);
    void setSnapY(int y);

    int snappedX(int x);
    int snappedY(int y);

    bool isSnapGridVisible() const;
    void setSnapGridVisible(bool bShow);

    bool useFillColor() const;
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

    bool isOpen() const;
    void setIsOpen(bool isOpen);

    // End of accessors and methods that only deal with loaded/saved data
    ////////////////////////////////////////////////////////////////////////

    void print(QPrinter *pPrinter, QPainter & pPainter);

    void hideEvent(QHideEvent *he);
    void showEvent(QShowEvent *se);

    void checkMessages(ObjectWidget * w);

    UMLWidget * findWidget(Uml::IDType id);

    AssociationWidget * findAssocWidget(Uml::IDType id);
    AssociationWidget * findAssocWidget(Uml::AssociationType at,
                                        UMLWidget *pWidgetA, UMLWidget *pWidgetB);
    AssociationWidget * findAssocWidget(UMLWidget *pWidgetA,
                                        UMLWidget *pWidgetB, const QString& roleNameB);

    void removeWidget(UMLWidget * o);

    void setSelected(UMLWidget * w, QMouseEvent * me);
    void clearSelected();

    void moveSelectedBy(UMLSceneValue dX, UMLSceneValue dY);

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

    void setMenu();

    bool getPaste() const;
    void setPaste(bool paste);

    UMLObjectList umlObjects();

    void activate();

    AssociationWidgetList selectedAssocs();

    bool selectedWidgets(UMLWidgetList& WidgetList, bool filterText = true);

    void activateAfterLoad( bool bUseLog = false );

    void endPartialWidgetPaste();
    void beginPartialWidgetPaste();

    void removeAssoc(AssociationWidget* pAssoc);
    void removeAssociations(UMLWidget* pWidget);
    void selectAssociations(bool bSelect);

    void getWidgetAssocs(UMLObject* Obj, AssociationWidgetList & Associations);

    void removeAllAssociations();

    void removeAllWidgets();

    void getDiagram(const UMLSceneRect &rect, QPixmap & diagram);
    void getDiagram(const UMLSceneRect &area, QPainter & painter);

    void copyAsImage(QPixmap*& pix);

    UMLViewImageExporter* getImageExporter();

    bool addAssociation(AssociationWidget* pAssoc, bool isPasteOperation = false);

    void removeAssocInViewAndDoc(AssociationWidget* assoc);

    bool addWidget(UMLWidget * pWidget, bool isPasteOperation = false);

    UMLScenePoint getPastePoint();
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

    void applyLayout(const QString &actionText);

    void toggleSnapToGrid();
    void toggleSnapComponentSizeToGrid();
    void toggleShowGrid();

    void fileLoaded();

    void resizeCanvasToItems();

    /**
     * The width and height of a diagram canvas in pixels.
     */
    static const int defaultCanvasSize;

    // Load/Save interface:

    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);
    virtual bool loadFromXMI(QDomElement & qElement);

    bool loadUISDiagram(QDomElement & qElement);
    UMLWidget* loadWidgetFromXMI(QDomElement& widgetElement);

    void addObject(UMLObject *object);

    void selectWidgets(UMLSceneValue px, UMLSceneValue py, UMLSceneValue qx, UMLSceneValue qy);
    void selectWidgetsOfAssoc(AssociationWidget * a);
    void selectWidgets(UMLWidgetList &widgets);

    ObjectWidget * onWidgetLine(const UMLScenePoint &point) const;
    ObjectWidget * onWidgetDestructionBox(const UMLScenePoint &point) const;

    UMLWidget* getFirstMultiSelectedWidget() const;

    UMLWidget* widgetAt(const UMLScenePoint& p);

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

protected:
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
    Uml::DiagramType m_Type;          ///< The type of diagram to represent.
    QString m_Name;                   ///< The name of the diagram.
    QString m_Documentation;          ///< The documentation of the diagram.
    Settings::OptionState m_Options;  ///< Options used by view.

    MessageWidgetList m_MessageList;  ///< All the message widgets on the diagram.
    UMLWidgetList m_WidgetList;       ///< All the UMLWidgets on the diagram.
    AssociationWidgetList m_AssociationList;  ///< All the AssociationWidgets on the diagram.

    bool m_bUseSnapToGrid;  ///< Flag to use snap to grid. The default is off.
    bool m_bUseSnapComponentSizeToGrid;  ///< Flag to use snap to grid for component size. The default is off.
    bool m_isOpen;  ///< Flag is set to true when diagram is open, i.e. shown to the user.

    int m_nSnapX;  ///< The snap to grid x size.
    int m_nSnapY;  ///< The snap to grid y size.
    bool m_bShowSnapGrid;  ///< Determines whether to show the snap grid. The default will be on if the grid is on.
    int m_nCanvasWidth;    ///< Width of canvas in pixels.
    int m_nCanvasHeight;   ///< Height of canvas in pixels.

    // End of methods and members related to loading/saving
    ////////////////////////////////////////////////////////////////////////

    void closeEvent(QCloseEvent * e);
    void dragEnterEvent(QDragEnterEvent* enterEvent);
    void dragMoveEvent(QDragMoveEvent* moveEvent);
    void dropEvent(QDropEvent* dropEvent);

    QRect diagramRect();

    void makeSelected(UMLWidget * uw);

    void updateComponentSizes();

    void findMaxBoundingRectangle(const FloatingTextWidget* ft,
                                  int& px, int& py, int& qx, int& qy);
    void forceUpdateWidgetFontMetrics(QPainter *painter);

    virtual void drawBackground(QPainter & painter, const QRect & clip);

    int m_nCollaborationId;  ///< Used for creating unique name of collaboration messages.
    UMLScenePoint m_Pos;
    bool m_bCreateObject;
    bool m_bDrawSelectedOnly;
    bool m_bPaste;
    ListPopupMenu * m_pMenu;
    bool m_bStartedCut;  ///< Flag if view/children started cut operation.
    UMLWidgetList m_SelectedList;

protected:
    UMLView *m_view;   ///< The view to which this scene is related.
    UMLFolder *m_pFolder;  ///< The folder in which this UMLView is contained.

    IDChangeLog * m_pIDChangesLog;  ///< LocalID Changes Log for paste actions
    bool m_isActivated;             ///< True if the view was activated after the serialization(load).
    bool m_bPopupShowing;           ///< Status of a popupmenu on view. True - a popup is on view.
    UMLScenePoint m_PastePoint;     ///< The offset at which to paste the clipboard.
    UMLDoc* m_doc;                  ///< Pointer to the UMLDoc.
    UMLViewImageExporter* m_pImageExporter;  ///< Used to export the view.

    void createAutoAttributeAssociation(UMLClassifier *type,
                                        UMLAttribute *attr,
                                        UMLWidget *widget);
    void createAutoConstraintAssociation(UMLEntity* refEntity,
                                         UMLForeignKeyConstraint* fkConstraint,
                                         UMLWidget* widget);

    static bool hasWidgetSmallerX(const UMLWidget* widget1, const UMLWidget* widget2);
    static bool hasWidgetSmallerY(const UMLWidget* widget1, const UMLWidget* widget2);

    int getSmallestX(const UMLWidgetList &widgetList);
    int getSmallestY(const UMLWidgetList &widgetList);

    int getBiggestX(const UMLWidgetList &widgetList);
    int getBiggestY(const UMLWidgetList &widgetList);

    int getHeightsSum(const UMLWidgetList &widgetList);
    int getWidthsSum(const UMLWidgetList &widgetList);

    template<typename Compare>
    void sortWidgetList(UMLWidgetList &widgetList, Compare comp);

public slots:
    void show();

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

signals:
    void sigFillColorChanged(Uml::IDType);
    void sigGridColorChanged(Uml::IDType);
    void sigLineColorChanged(Uml::IDType);
    void sigTextColorChanged(Uml::IDType);
    void sigLineWidthChanged(Uml::IDType);
    void sigRemovePopupMenu();
    void sigClearAllSelected();
    void sigSnapToGridToggled(bool);
    void sigSnapComponentSizeToGridToggled(bool);
    void sigShowGridToggled(bool);
    void sigAssociationRemoved(AssociationWidget*);
    void sigWidgetRemoved(UMLWidget*);
};

QDebug operator<<(QDebug debug, UMLScene *item);

/**
  qt3 migration wrapper for QMouseEvent
*/ 
class  UMLSceneMouseEvent : public QMouseEvent 
{
public:
    UMLSceneMouseEvent(Type type, const QPoint & position, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers) 
     : QMouseEvent(type, position, button, buttons, modifiers)
    {
    }

    UMLScenePoint scenePos() { return pos(); }
};

class UMLSceneLineItem : public Q3CanvasLine
{
public:
    UMLSceneLineItem()
      : Q3CanvasLine(0)
    {
    }
};

class UMLSceneRectItem : public Q3CanvasRectangle
{
public:
    UMLSceneRectItem()
      : Q3CanvasRectangle(0)
    {
    }

    UMLSceneRectItem(int x, int y, int w, int h)
      : Q3CanvasRectangle(x, y, w, h, 0)
    {
    }
};

class UMLScenePolygonItem : public Q3CanvasPolygon
{
public:
    UMLScenePolygonItem()
      : Q3CanvasPolygon(0)
    {
    }
};

class UMLSceneEllipseItem : public Q3CanvasEllipse
{
public:
    UMLSceneEllipseItem()
    : Q3CanvasEllipse(0)
    {
    }

    UMLSceneEllipseItem(int width, int height)
    : Q3CanvasEllipse(width, height, 0)
    {
    }
};

#endif
