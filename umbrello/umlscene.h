/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
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

    const AssociationWidgetList associationList() const;
    const MessageWidgetList messageList() const;
    const UMLWidgetList widgetList() const;
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

    void resizeSceneToItems();

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

#if QT_VERSION >= 0x050000
#if !(QT_DEPRECATED_SINCE(5, 0))
    inline QGraphicsItem *itemAt(const QPointF &position) const {
        QList<QGraphicsItem *> itemsAtPoint = items(position);
        return itemsAtPoint.isEmpty() ? 0 : itemsAtPoint.first();
    }
#endif
#endif

    int m_nCollaborationId;  ///< Used for creating unique name of collaboration messages.
    QPointF m_pos;
    bool m_bCreateObject;
    bool m_bDrawSelectedOnly;
    bool m_bPaste;
    bool m_bStartedCut;  ///< Flag if view/children started cut operation.

private:
    UMLScenePrivate *m_d;
    static const qreal defaultCanvasSize;  ///< The default size of a diagram in pixels.
    static bool m_showDocumentationIndicator; ///< Status of documentation indicator

    UMLView *m_view;   ///< The view to which this scene is related.
    UMLFolder *m_pFolder;  ///< The folder in which this UMLView is contained.

    IDChangeLog * m_pIDChangesLog;  ///< LocalID Changes Log for paste actions
    bool m_isActivated;             ///< True if the view was activated after the serialization(load).
    bool m_bPopupShowing;           ///< Status of a popupmenu on view. True - a popup is on view.
    QPointF m_PastePoint;     ///< The offset at which to paste the clipboard.
    UMLDoc* m_doc;                  ///< Pointer to the UMLDoc.
    UMLViewImageExporter* m_pImageExporter;  ///< Used to export the view.
    LayoutGrid*  m_layoutGrid;      ///< layout grid in the background
    bool m_autoIncrementSequence; ///< state of auto increment sequence

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
    void sigResetToolBar();

    void sigFillColorChanged(Uml::ID::Type);
    void sigGridColorChanged(Uml::ID::Type);
    void sigLineColorChanged(Uml::ID::Type);
    void sigTextColorChanged(Uml::ID::Type);
    void sigLineWidthChanged(Uml::ID::Type);
    void sigSnapToGridToggled(bool);
    void sigSnapComponentSizeToGridToggled(bool);
    void sigShowGridToggled(bool);
    void sigAssociationRemoved(AssociationWidget*);
    void sigWidgetRemoved(UMLWidget*);

    friend class DiagramProxyWidget;
};

QDebug operator<<(QDebug dbg, UMLScene *item);

#endif // UMLSCENE_H
