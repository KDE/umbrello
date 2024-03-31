/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLSCENE_H
#define UMLSCENE_H

// local includes
#include "associationwidgetlist.h"
#include "basictypes.h"
#include "classifierwidget.h"
#include "messagewidgetlist.h"
#include "optionstate.h"
#include "umlobject.h"
#include "umlobjectlist.h"
#include "umlwidgetlist.h"
#include "worktoolbar.h"
#include "widgetbase.h"

// Qt includes
#include <QDomDocument>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPolygonItem>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QXmlStreamWriter>

// forward declarations
class ClassOptionsPage;
class IDChangeLog;
class LayoutGrid;
class FloatingTextWidget;
class ObjectWidget;
class ToolBarState;
class ToolBarStateFactory;
class UMLFolder;
class UMLDoc;
class UMLAttribute;
class UMLCanvasObject;
class UMLClassifier;
class UMLViewImageExporter;
class UMLForeignKeyConstraint;
class UMLEntity;
class UMLView;

class QHideEvent;
class QMouseEvent;
class QPrinter;
class QShowEvent;
class UMLScenePrivate;

// migration wrapper for QGraphicsScene items
typedef QList<QGraphicsItem*> UMLSceneItemList;

/**
 * UMLScene instances represent diagrams.
 * The UMLScene class inherits from QGraphicsScene and it owns the
 * objects displayed (see m_WidgetList.)
 */
class UMLScene : public QGraphicsScene
{
    Q_OBJECT
public:
    friend class UMLViewImageExporterModel;

    explicit UMLScene(UMLFolder *parentFolder, UMLView *view = 0);
    virtual ~UMLScene();

    UMLView* activeView() const;

    // Accessors and other methods dealing with loaded/saved data

    UMLFolder* folder() const;
    void setFolder(UMLFolder *folder);

    QString documentation() const;
    void setDocumentation(const QString &doc);

    bool autoIncrementSequence() const;
    void setAutoIncrementSequence(bool state);
    QString autoIncrementSequenceValue();

    QString name() const;
    void setName(const QString &name);

    Uml::DiagramType::Enum type() const;
    void setType(Uml::DiagramType::Enum type);

    Uml::ID::Type ID() const;
    void setID(Uml::ID::Type id);

    QPointF pos() const;
    void setPos(const QPointF &pos);

    const QColor& fillColor() const;
    void setFillColor(const QColor &color);

    const QColor& lineColor() const;
    void setLineColor(const QColor &color);

    uint lineWidth() const;
    void setLineWidth(uint width);

    const QColor& textColor() const;
    void setTextColor(const QColor& color);

    const QColor& gridDotColor() const;
    void setGridDotColor(const QColor& color);

    const QColor& backgroundColor() const;

    bool snapToGrid() const;
    void setSnapToGrid(bool bSnap);

    bool snapComponentSizeToGrid() const;
    void setSnapComponentSizeToGrid(bool bSnap);

    int snapX() const;
    int snapY() const;
    void setSnapSpacing(int x, int y);

    qreal snappedX(qreal x);
    qreal snappedY(qreal y);

    bool isSnapGridVisible() const;
    void setSnapGridVisible(bool bShow);

    bool isShowDocumentationIndicator() const;
    void setShowDocumentationIndicator(bool bShow);

    bool useFillColor() const;
    void setUseFillColor(bool ufc);

    QFont font() const;
    void setFont(QFont font, bool changeAllWidgets = false);

    bool showOpSig() const;
    void setShowOpSig(bool bShowOpSig);

    Settings::OptionState& optionState();
    void setOptionState(const Settings::OptionState& options);

    AssociationWidgetList associationList() const;
    MessageWidgetList messageList() const;
    UMLWidgetList widgetList() const;
    void addWidgetCmd(UMLWidget* widget);
    void addWidgetCmd(AssociationWidget *widget);

    bool isOpen() const;
    void setIsOpen(bool isOpen);

    // End of accessors and methods that only deal with loaded/saved data
    ////////////////////////////////////////////////////////////////////////

    void print(QPrinter *pPrinter, QPainter & pPainter);

    void hideEvent(QHideEvent *he);
    void showEvent(QShowEvent *se);

    void checkMessages(ObjectWidget * w);

    UMLWidget* findWidget(Uml::ID::Type id);

    AssociationWidget* findAssocWidget(Uml::ID::Type id);
    AssociationWidget* findAssocWidget(Uml::AssociationType::Enum at,
                                       UMLWidget *pWidgetA, UMLWidget *pWidgetB);
    AssociationWidget* findAssocWidget(UMLWidget *pWidgetA,
                                       UMLWidget *pWidgetB, const QString& roleNameB);

    void removeWidget(UMLWidget *o);
    void removeWidget(AssociationWidget *w);
    void removeWidgetCmd(UMLWidget *o);
private:
    void removeOwnedWidgets(UMLWidget* o);
public:

    UMLWidgetList selectedWidgets() const;
    AssociationWidgetList selectedAssociationWidgets() const;
    UMLWidgetList selectedMessageWidgets() const;
    void clearSelected();

    void moveSelectedBy(qreal dX, qreal dY);

    int selectedCount(bool filterText = false) const;

    void selectionUseFillColor(bool useFC);
    void selectionSetFont(const QFont &font);
    void selectionSetLineColor(const QColor &color);
    void selectionSetLineWidth(uint width);
    void selectionSetFillColor(const QColor &color);
    void selectionSetVisualProperty(ClassifierWidget::VisualProperty property, bool value);

private:
    void unselectChildrenOfSelectedWidgets();
public:
    void deleteSelection();
    void resizeSelection();

    void selectAll();

    UMLWidget* widgetOnDiagram(Uml::ID::Type id);
    UMLWidget *widgetOnDiagram(WidgetBase::WidgetType type);

    bool isSavedInSeparateFile();

    void setMenu(const QPoint& pos);

    void resetToolbar();
    void triggerToolbarButton(WorkToolBar::ToolBar_Buttons button);

    bool getPaste() const;
    void setPaste(bool paste);

    void activate();

    AssociationWidgetList selectedAssocs();
    UMLWidgetList selectedWidgetsExt(bool filterText = true);

    void activateAfterLoad(bool bUseLog = false);

    void endPartialWidgetPaste();
    void beginPartialWidgetPaste();

    void removeWidgetCmd(AssociationWidget* pAssoc);
    void removeAssociations(UMLWidget* pWidget);
    void selectAssociations(bool bSelect);

    void getWidgetAssocs(UMLObject* Obj, AssociationWidgetList & Associations);

    void removeAllAssociations();

    void removeAllWidgets();

    void getDiagram(QPixmap & diagram, const QRectF &rect);
    void getDiagram(QPainter &painter, const QRectF &source, const QRectF &target = QRectF());

    void copyAsImage(QPixmap*& pix);

    UMLViewImageExporter* getImageExporter();

    bool addAssociation(AssociationWidget* pAssoc, bool isPasteOperation = false);

    void removeAssocInViewAndDoc(AssociationWidget* assoc);

    void addFloatingTextWidget(FloatingTextWidget* pWidget);

    QPointF getPastePoint();
    void resetPastePoint();

    void setStartedCut();

    void createAutoAssociations(UMLWidget * widget);
    void createAutoAttributeAssociations(UMLWidget *widget);
    void createAutoConstraintAssociations(UMLWidget* widget);
    void createAutoAttributeAssociations2(UMLWidget *widget);

    void updateContainment(UMLCanvasObject *self);

    void setClassWidgetOptions(ClassOptionsPage * page);

    WidgetBase::WidgetType getUniqueSelectionType();

    void clearDiagram();

    void applyLayout(const QString &actionText);

    void toggleSnapToGrid();
    void toggleSnapComponentSizeToGrid();
    void toggleShowGrid();

    void fileLoaded();

    // Load/Save interface:

    virtual void saveToXMI(QXmlStreamWriter& writer);
    virtual bool loadFromXMI(QDomElement & qElement);

    bool loadUISDiagram(QDomElement & qElement);
    UMLWidget* loadWidgetFromXMI(QDomElement& widgetElement);

    void addObject(UMLObject *object);

    void selectWidgets(qreal px, qreal py, qreal qx, qreal qy);
    void selectWidgets(UMLWidgetList &widgets);
    void selectWidget(UMLWidget* widget, QRectF* rect = 0);
    void selectWidgetsOfAssoc(AssociationWidget *a);

    ObjectWidget * onWidgetLine(const QPointF &point) const;
    ObjectWidget * onWidgetDestructionBox(const QPointF &point) const;

    UMLWidget* getFirstMultiSelectedWidget() const;

    UMLWidget* widgetAt(const QPointF& p);
    AssociationWidget* associationAt(const QPointF& p);
    MessageWidget* messageAt(const QPointF& p);

    void setupNewWidget(UMLWidget *w, bool setPosition=true);

    bool getCreateObject() const;
    void setCreateObject(bool bCreate);

    int generateCollaborationId();

    UMLSceneItemList collisions(const QPointF &p, int delta = 3);

    bool isClassDiagram()              const { return type() == Uml::DiagramType::Class;  }
    bool isUseCaseDiagram()            const { return type() == Uml::DiagramType::UseCase; }
    bool isSequenceDiagram()           const { return type() == Uml::DiagramType::Sequence; }
    bool isCollaborationDiagram()      const { return type() == Uml::DiagramType::Collaboration; }
    bool isStateDiagram()              const { return type() == Uml::DiagramType::State; }
    bool isActivityDiagram()           const { return type() == Uml::DiagramType::Activity; }
    bool isComponentDiagram()          const { return type() == Uml::DiagramType::Component; }
    bool isDeploymentDiagram()         const { return type() == Uml::DiagramType::Deployment; }
    bool isEntityRelationshipDiagram() const { return type() == Uml::DiagramType::EntityRelationship; }
    bool isObjectDiagram()             const { return type() == Uml::DiagramType::Object; }

    void setWidgetLink(WidgetBase *w);
    WidgetBase *widgetLink();

    static qreal maxCanvasSize();

    void updateCanvasSizeEstimate(qreal x, qreal y, qreal w, qreal h);
    void updateSceneRect();

    qreal fixX() const;
    qreal fixY() const;

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
    Uml::ID::Type          m_nLocalID;

    Uml::ID::Type          m_nID;      ///< The ID of the view. Allocated by @ref UMLDoc.
    Uml::DiagramType::Enum m_Type;     ///< The type of diagram to represent.
    QString                m_Name;     ///< The name of the diagram.
    QString          m_Documentation;  ///< The documentation of the diagram.
    Settings::OptionState  m_Options;  ///< Options used by view.

    bool m_bUseSnapToGrid;  ///< Flag to use snap to grid. The default is off.
    bool m_bUseSnapComponentSizeToGrid;  ///< Flag to use snap to grid for component size. The default is off.
    bool m_isOpen;  ///< Flag is set to true when diagram is open, i.e. shown to the user.

    // End of methods and members related to loading/saving
    ////////////////////////////////////////////////////////////////////////

    void dragEnterEvent(QGraphicsSceneDragDropEvent* enterEvent);
    void dragMoveEvent(QGraphicsSceneDragDropEvent* moveEvent);
    void dropEvent(QGraphicsSceneDragDropEvent* dropEvent);

    void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

    QRectF diagramRect();

    void makeSelected(UMLWidget* uw);

    void updateComponentSizes();

    void findMaxBoundingRectangle(const FloatingTextWidget* ft,
                                  qreal& px, qreal& py, qreal& qx, qreal& qy);
    void forceUpdateWidgetFontMetrics(QPainter *painter);

    virtual void drawBackground(QPainter *painter, const QRectF &rect);

#if !(QT_DEPRECATED_SINCE(5, 0))
    inline QGraphicsItem *itemAt(const QPointF &position) const {
        QList<QGraphicsItem *> itemsAtPoint = items(position);
        return itemsAtPoint.isEmpty() ? 0 : itemsAtPoint.first();
    }
#endif

    int m_nCollaborationId;  ///< Used for creating unique name of collaboration messages.
    QPointF m_pos;
    bool m_bCreateObject;
    bool m_bDrawSelectedOnly;
    bool m_bPaste;
    bool m_bStartedCut;  ///< Flag if view/children started cut operation.

private:
    UMLScenePrivate *m_d;
    static const qreal s_defaultCanvasWidth;    ///< The default width of a diagram in pixels.
    static const qreal s_defaultCanvasHeight;   ///< The default height of a diagram in pixels.
    static const qreal s_maxCanvasSize;         ///< The maximum supported canvas size.
    static const qreal s_sceneBorder;           ///< A border around the bounding rectangle of the elements in the scene.
    static bool s_showDocumentationIndicator; ///< Status of documentation indicator

    UMLView *m_view;   ///< The view to which this scene is related.
    UMLFolder *m_pFolder;  ///< The folder in which this UMLView is contained.

    IDChangeLog * m_pIDChangesLog;  ///< LocalID Changes Log for paste actions
    bool m_isActivated;             ///< True if the view was activated after the deserialization(load).
    bool m_bPopupShowing;           ///< Status of a popupmenu on view. True - a popup is on view.
    QPointF m_PastePoint;     ///< The offset at which to paste the clipboard.
    UMLDoc* m_doc;                  ///< Pointer to the UMLDoc.
    UMLViewImageExporter* m_pImageExporter;  ///< Used to export the view.
    LayoutGrid*  m_layoutGrid;      ///< layout grid in the background
    bool m_autoIncrementSequence; ///< state of auto increment sequence
    qreal m_minX, m_minY;     ///< Gather data for estimating required canvas size (used during loadFromXMI)
    qreal m_maxX, m_maxY;     ///< Gather data for estimating required canvas size (used during loadFromXMI)
    qreal m_fixX;             ///< Compensate for QGraphicsScene offsets, https://bugs.kde.org/show_bug.cgi?id=449622
    qreal m_fixY;             ///< Compensate for QGraphicsScene offsets, https://bugs.kde.org/show_bug.cgi?id=449622

    void createAutoAttributeAssociation(UMLClassifier *type,
                                        UMLAttribute *attr,
                                        UMLWidget *widget);
    void createAutoConstraintAssociation(UMLEntity* refEntity,
                                         UMLForeignKeyConstraint* fkConstraint,
                                         UMLWidget* widget);

public:
    Q_SLOT void slotToolBarChanged(int c);
    Q_SLOT void slotObjectCreated(UMLObject * o);
    Q_SLOT void slotObjectRemoved(UMLObject * o);
    Q_SLOT void slotMenuSelection(QAction* action);
    Q_SLOT void slotActivate();
    Q_SLOT void slotCutSuccessful();
    Q_SLOT void slotShowView();

    Q_SLOT void alignLeft();
    Q_SLOT void alignRight();
    Q_SLOT void alignTop();
    Q_SLOT void alignBottom();
    Q_SLOT void alignVerticalMiddle();
    Q_SLOT void alignHorizontalMiddle();
    Q_SLOT void alignVerticalDistribute();
    Q_SLOT void alignHorizontalDistribute();

protected:
    Q_SIGNAL void sigResetToolBar();

    Q_SIGNAL void sigFillColorChanged(Uml::ID::Type);
    Q_SIGNAL void sigGridColorChanged(Uml::ID::Type);
    Q_SIGNAL void sigLineColorChanged(Uml::ID::Type);
    Q_SIGNAL void sigTextColorChanged(Uml::ID::Type);
    Q_SIGNAL void sigLineWidthChanged(Uml::ID::Type);
    Q_SIGNAL void sigSnapToGridToggled(bool);
    Q_SIGNAL void sigSnapComponentSizeToGridToggled(bool);
    Q_SIGNAL void sigShowGridToggled(bool);
    Q_SIGNAL void sigAssociationRemoved(AssociationWidget*);
    Q_SIGNAL void sigWidgetRemoved(UMLWidget*);

    friend class DiagramProxyWidget;
};

QDebug operator<<(QDebug dbg, UMLScene *item);

#endif // UMLSCENE_H
