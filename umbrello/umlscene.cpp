/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlscene.h"

// application specific includes
#include "activitywidget.h"
#include "actorwidget.h"
#include "artifactwidget.h"
#include "association.h"
#include "associationwidget.h"
#include "assocrules.h"
#include "attribute.h"
#include "boxwidget.h"
#include "classifier.h"
#include "classifierwidget.h"
#include "classoptionspage.h"
#include "component.h"
#include "cmds.h"
#include "componentwidget.h"
#include "datatype.h"
#include "diagram_utils.h"
#include "pinportbase.h"
#include "datatypewidget.h"
#define DBG_SRC QStringLiteral("UMLScene")
#include "debug_utils.h"   // we cannot use the predefined DBG_SRC due to class UMLScenePrivate
#include "dialog_utils.h"
#include "docwindow.h"
#include "entity.h"
#include "entitywidget.h"
#include "enumwidget.h"
#include "floatingtextwidget.h"
#include "folder.h"
#include "foreignkeyconstraint.h"
#include "forkjoinwidget.h"
#include "idchangelog.h"
#include "interfacewidget.h"
#include "import_utils.h"
#include "layoutgenerator.h"
#include "layoutgrid.h"
#include "messagewidget.h"
#include "model_utils.h"
#include "notewidget.h"
#include "object_factory.h"
#include "objectnodewidget.h"
#include "objectwidget.h"
#include "package.h"
#include "packagewidget.h"
#include "pinwidget.h"
#include "portwidget.h"
#include "seqlinewidget.h"
#include "signalwidget.h"
#include "statewidget.h"
#include "toolbarstate.h"
#include "toolbarstatefactory.h"
#include "uml.h"
#include "umldoc.h"
#include "umldragdata.h"
#include "umlfiledialog.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "umlobject.h"
#include "umlobjectlist.h"
#include "umlrole.h"
#include "umlscenepopupmenu.h"
#include "umlview.h"
#include "umlviewimageexporter.h"
#include "umlwidget.h"
#include "uniqueid.h"
#include "widget_factory.h"
#include "widget_utils.h"
#include "widgetlist_utils.h"

//kde include files
#include <KMessageBox>
#include <kcursor.h>
#include <KLocalizedString>

// include files for Qt
#include <QColor>
#include <QLineF>
#include <QPainter>
#include <QPixmap>
#include <QPrinter>
#include <QString>
#include <QStringList>
#include <QXmlStreamWriter>

// system includes
#include <cmath>  // for ceil

// static members
const qreal UMLScene::s_defaultCanvasWidth  = 1100;
const qreal UMLScene::s_defaultCanvasHeight =  800;
const qreal UMLScene::s_maxCanvasSize = 100000.0;
const qreal UMLScene::s_sceneBorder = 5.0;
bool UMLScene::s_showDocumentationIndicator = false;


using namespace Uml;

DEBUG_REGISTER_DISABLED(UMLScene)

/**
 * The class UMLScenePrivate is intended to hold private
 * members/classes to reduce the size of the public class
 * and to speed up recompiling.
 * The migration to this class is not complete yet.
 */
class UMLScenePrivate {
public:
    UMLScenePrivate(UMLScene *parent)
      : p(parent)
      , toolBarState(nullptr)
      , inMouseMoveEvent(false)
    {
        toolBarStateFactory = new ToolBarStateFactory;
    }

    ~UMLScenePrivate()
    {
        delete toolBarState;
        delete toolBarStateFactory;
    }
    /**
     * Check if there is a corresponding port widget
     * for all UMLPort instances and add if not.
     */
    void addMissingPorts()
    {
        UMLWidgetList ports;
        UMLWidgetList components;

        for(UMLWidget  *w : p->widgetList()) {
            if (w->isPortWidget())
                ports.append(w);
            else if (w->isComponentWidget())
                components.append(w);
        }

        for(UMLWidget  *cw : components) {
            const UMLComponent *c = cw->umlObject()->asUMLComponent();
            if (!c)
                continue;
            // iterate through related ports for this component widget
            for(UMLObject  *o : c->containedObjects()) {
                UMLPort *up = o->asUMLPort();
                if (!up)
                    continue;
                Uml::ID::Type id = o->id();
                bool found = false;
                for(UMLWidget  *p : ports) {
                    if (p->id() == id) {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    new PortWidget(p, up, cw);
            }
        }
    }

    /**
     * Check if port are located equally on the border of a component
     * and fix position if not.
     */
    void fixPortPositions()
    {
        for(UMLWidget  *w : p->widgetList()) {
            if (w->isPortWidget()) {
                QGraphicsItem *g = w->parentItem();
                ComponentWidget *c = dynamic_cast<ComponentWidget*>(g);
                Q_ASSERT(c);
                qreal w2 = w->width()/2;
                qreal h2 = w->height()/2;
                if (w->x() <= -w2 || w->y() <= -h2
                        || w->x() >= c->width() - w2
                        || w->y() >= c->height() - h2)
                    continue;
                if (w->x() >= c->width() - 3 * w2) { // right
                    w->setX(c->width() - w2);
                } else if (w->y() >= c->height() - 3 * h2) { // bottom
                    w->setY(c->height() - h2);
                } else if (w->x() < 3 * w2) { // left
                    w->setX(-w2);
                } else if (w->y() < 3 * h2) { // top
                    w->setY(-h2);
                } else
                    logWarn1("UMLScenePrivate::fixPortPositions unhandled widget %1 position", w->name());
            }
        }
    }

    /**
     * Check if duplicated floating text labels are in the scene and remove them
     */
    void removeDuplicatedFloatingTextInstances()
    {
        UMLWidgetList labelsWithoutParents;
        UMLWidgetList labelsWithParent;
        const QString pName(p->name());
        logDebug2("UMLScenePrivate::removeDuplicatedFloatingTextInstances checking diagram %1 id %2",
                  pName, Uml::ID::toString(p->ID()));

        for(UMLWidget  *w : p->widgetList()) {
            if (!w->isTextWidget())
                continue;
            if (w->parentItem())
                labelsWithParent.append(w);
            else
                labelsWithoutParents.append(w);
        }
        for(UMLWidget  *w : labelsWithoutParents) {
            for(UMLWidget  *wp : labelsWithParent) {
                if (w->id() == wp->id() &&
                        w->localID() == wp->localID() &&
                        w->name() == wp->name()) {
                    p->removeWidgetCmd(w);
                    logDebug2("UMLScenePrivate::removeDuplicatedFloatingTextInstances removed "
                              "duplicated text label %1 id %2", w->name(), Uml::ID::toString(w->id()));
                    break;
                }
            }
        }
    }

    void setToolBarChanged(WorkToolBar::ToolBar_Buttons button)
    {
        if (toolBarState)
            toolBarState->cleanBeforeChange();
        toolBarState = toolBarStateFactory->getState(button, p);
        toolBarState->init();
        p->setPaste(false);
    }

    void triggerToolBarButton(WorkToolBar::ToolBar_Buttons button)
    {
        UMLApp::app()->workToolBar()->buttonChanged(button);
        setToolBarChanged(button);
        QGraphicsSceneMouseEvent event;
        event.setScenePos(p->pos());
        event.setButton(Qt::LeftButton);
        toolBarState->mousePress(&event);
        toolBarState->mouseRelease(&event);
        p->connect(toolBarState, SIGNAL(finished()), UMLApp::app()->workToolBar(), SLOT(slotResetToolBar()));
    }

    UMLScene *p;
    ToolBarStateFactory *toolBarStateFactory;
    ToolBarState *toolBarState;
    QPointer<WidgetBase> widgetLink;
    bool inMouseMoveEvent;
};

/**
 * Constructor.
 */
UMLScene::UMLScene(UMLFolder *parentFolder, UMLView *view)
  : QGraphicsScene(0, 0, s_defaultCanvasWidth, s_defaultCanvasHeight),
    m_nLocalID(Uml::ID::None),
    m_nID(Uml::ID::None),
    m_Type(Uml::DiagramType::Undefined),
    m_Name(QString()),
    m_Documentation(QString()),
    m_Options(Settings::optionState()),
    m_bUseSnapToGrid(false),
    m_bUseSnapComponentSizeToGrid(false),
    m_isOpen(true),
    m_nCollaborationId(0),
    m_bCreateObject(false),
    m_bDrawSelectedOnly(false),
    m_bPaste(false),
    m_bStartedCut(false),
    m_d(new UMLScenePrivate(this)),
    m_view(view),
    m_pFolder(parentFolder),
    m_pIDChangesLog(0),
    m_isActivated(false),
    m_bPopupShowing(false),
    m_autoIncrementSequence(false),
    m_minX(s_maxCanvasSize), m_minY(s_maxCanvasSize),
    m_maxX(0.0), m_maxY(0.0),
    m_fixX(0.0), m_fixY(0.0)
{
    m_PastePoint = QPointF(0, 0);

    m_pImageExporter = new UMLViewImageExporter(this);

    // setup signals
    connect(UMLApp::app(), SIGNAL(sigCutSuccessful()),
            this, SLOT(slotCutSuccessful()));
    m_d->setToolBarChanged(WorkToolBar::tbb_Arrow);

    m_doc = UMLApp::app()->document();

//    // settings for background
//    setBackgroundBrush(QColor(195, 195, 195));
    m_layoutGrid = new LayoutGrid(this);

    // fix crash caused by Qt stale item issue see https://bugs.kde.org/show_bug.cgi?id=383592
    setItemIndexMethod(NoIndex);
}

/**
 * Destructor.
 */
UMLScene::~UMLScene()
{
    delete m_pImageExporter;
    m_pImageExporter = 0;
    delete m_pIDChangesLog;
    m_pIDChangesLog = 0;

    // before we can delete the QCanvas, all widgets must be explicitly
    // removed
    // otherwise the implicit remove of the contained widgets will cause
    // events which would demand a valid connected QCanvas
    // ==> this causes umbrello to crash for some - larger?? - projects
    // first avoid all events, which would cause some update actions
    // on deletion of each removed widget
    blockSignals(true);
    removeAllWidgets();

    delete m_layoutGrid;
    delete m_d;
}


/**
 * The size returned is large enough to account for possible bogus widget
 * coordinate offsets as described in
 *     https://bugs.kde.org/show_bug.cgi?id=449622
 * Coordinate values below -maxCanvasSize() or above maxCanvasSize() are
 * deemed unrecoverable.
 *
 * @return the maximum possible canvas size
 */
qreal UMLScene::maxCanvasSize() {
    return s_maxCanvasSize;
}

/**
 * Return the UMLFolder in which this diagram lives.
 */
UMLFolder* UMLScene::folder() const
{
    return m_pFolder;
}

/**
 * Set the UMLFolder in which this diagram lives.
 */
void UMLScene::setFolder(UMLFolder *folder)
{
    m_pFolder = folder;
}

/**
 * Returns the active view associated with this scene.
 */
UMLView* UMLScene::activeView() const
{
    return m_view;
}

/**
 * Return the documentation of the diagram.
 */
QString UMLScene::documentation() const
{
    return m_Documentation;
}

/**
 * Set the documentation of the diagram.
 */
void UMLScene::setDocumentation(const QString &doc)
{
    m_Documentation = doc;
}

/**
 * Return the state of the auto increment sequence
 */
bool UMLScene::autoIncrementSequence() const
{
    return m_autoIncrementSequence;
}

void UMLScene::setAutoIncrementSequence(bool state)
{
    m_autoIncrementSequence = state;
}

/**
 * Return the next auto increment sequence value
 */
QString UMLScene::autoIncrementSequenceValue()
{
    int sequenceNumber = 0;
    if (isSequenceDiagram()) {
        for(MessageWidget *message : messageList()) {
            bool ok;
            int value = message->sequenceNumber().toInt(&ok);
            if (ok && value > sequenceNumber)
               sequenceNumber = value;
        }
    }
    else if (isCollaborationDiagram()) {
        for(AssociationWidget *assoc : associationList()) {
            bool ok;
            int value = assoc->sequenceNumber().toInt(&ok);
            if (ok && value > sequenceNumber)
               sequenceNumber = value;
        }
    }
    return QString::number(sequenceNumber + 1);
}

/**
 * Return the name of the diagram.
 */
QString UMLScene::name() const
{
    return m_Name;
}

/**
 * Set the name of the diagram.
 */
void UMLScene::setName(const QString &name)
{
    m_Name = name;
}

/**
 * Returns the type of the diagram.
 */
DiagramType::Enum UMLScene::type() const
{
    return m_Type;
}

/**
 * Set the type of diagram.
 */
void UMLScene::setType(DiagramType::Enum type)
{
    m_Type = type;
}

/**
 * Returns the ID of the diagram.
 */
Uml::ID::Type UMLScene::ID() const
{
    return m_nID;
}

/**
 * Sets the ID of the diagram.
 */
void UMLScene::setID(Uml::ID::Type id)
{
    m_nID = id;
}

/**
 * Returns the position of the diagram.
 */
QPointF UMLScene::pos() const
{
    return m_pos;
}

/**
 * Sets the position of the diagram.
 */
void UMLScene::setPos(const QPointF &pos)
{
    m_pos = pos;
}

/**
 * Returns the fill color to use.
 */
const QColor& UMLScene::fillColor() const
{
    return m_Options.uiState.fillColor;
}

/**
 * Set the background color.
 *
 * @param color  The color to use.
 */
void UMLScene::setFillColor(const QColor &color)
{
    m_Options.uiState.fillColor = color;
    Q_EMIT sigFillColorChanged(ID());
}

/**
 * Returns the line color to use.
 */
const QColor& UMLScene::lineColor() const
{
    return m_Options.uiState.lineColor;
}

/**
 * Sets the line color.
 *
 * @param color  The color to use.
 */
void UMLScene::setLineColor(const QColor &color)
{
    m_Options.uiState.lineColor = color;
    Q_EMIT sigLineColorChanged(ID());
}

/**
 * Returns the line width to use.
 */
uint UMLScene::lineWidth() const
{
    return m_Options.uiState.lineWidth;
}

/**
 * Sets the line width.
 *
 * @param width  The width to use.
 */
void UMLScene::setLineWidth(uint width)
{
    m_Options.uiState.lineWidth = width;
    Q_EMIT sigLineWidthChanged(ID());
}

/**
 * Returns the text color to use.
 */
const QColor& UMLScene::textColor() const
{
    return m_Options.uiState.textColor;
}

/**
 * Sets the text color.
 *
 * @param color  The color to use.
 */
void UMLScene::setTextColor(const QColor& color)
{
    m_Options.uiState.textColor = color;
    Q_EMIT sigTextColorChanged(ID());
}

/**
 * return grid dot color
 *
 * @return Color
 */
const QColor& UMLScene::gridDotColor() const
{
    return m_layoutGrid->gridDotColor();
}

/**
 * set grid dot color
 *
 * @param color grid dot color
 */
void UMLScene::setGridDotColor(const QColor& color)
{
    m_Options.uiState.gridDotColor = color;
    m_layoutGrid->setGridDotColor(color);
}

/**
 * Returns the options being used.
 */
Settings::OptionState& UMLScene::optionState()
{
    return m_Options;
}

/**
 * Sets the options to be used.
 */
void UMLScene::setOptionState(const Settings::OptionState& options)
{
    m_Options = options;
    setBackgroundBrush(options.uiState.backgroundColor);
    setGridDotColor(options.uiState.gridDotColor);
}

/**
 * Returns the association list.
 */
AssociationWidgetList UMLScene::associationList() const
{
    AssociationWidgetList result;
    for(QGraphicsItem  *item : items()) {
        AssociationWidget *w = dynamic_cast<AssociationWidget*>(item);
        if (w)
            result.append(w);
    }
    return result;
}

/**
 * Returns the widget list.
 */
UMLWidgetList UMLScene::widgetList() const
{
    UMLWidgetList result;
    for(QGraphicsItem  *item : items()) {
        UMLWidget *w = dynamic_cast<UMLWidget*>(item);
        if (w && !w->isMessageWidget() && !w->isAssociationWidget())
            result.append(w);
    }
    return result;
}

void UMLScene::addWidgetCmd(UMLWidget* widget)
{
    Q_ASSERT(0 != widget);
    logDebug5("UMLScene::addWidgetCmd(%1) : x=%2, y=%3, w=%4, h=%5",
              widget->name(), widget->x(), widget->y(), widget->width(), widget->height());
    addItem(widget);
}

void UMLScene::addWidgetCmd(AssociationWidget* widget)
{
    Q_ASSERT(0 != widget);
    addItem(widget);
}

/**
 * Returns the message list.
 */
MessageWidgetList UMLScene::messageList() const
{
    MessageWidgetList result;
    for(QGraphicsItem  *item : items()) {
        MessageWidget *w = dynamic_cast<MessageWidget*>(item);
        if (w)
            result.append(w);
    }
    return result;
}

/**
 * Used for creating unique name of collaboration messages.
 */
int UMLScene::generateCollaborationId()
{
    return ++m_nCollaborationId;
}

/**
 * Returns the open state.
 * @return   when true diagram is shown to the user
 */
bool UMLScene::isOpen() const
{
    return m_isOpen;
}

/**
 * Sets the flag 'isOpen'.
 * @param isOpen   flag indicating that the diagram is shown to the user
 */
void UMLScene::setIsOpen(bool isOpen)
{
    m_isOpen = isOpen;
}

/**
 * Contains the implementation for printing functionality.
 */
void UMLScene::print(QPrinter *pPrinter, QPainter & pPainter)
{
    bool isFooter = optionState().generalState.footerPrinting;

    // The printer will probably use a different font with different font metrics,
    // force the widgets to update accordingly on paint
    forceUpdateWidgetFontMetrics(&pPainter);

    QRectF source = diagramRect();
    QRect paper = pPrinter->paperRect();
    QRect page = pPrinter->pageRect();

    // use the painter font metrics, not the screen fm!
    QFontMetrics fm = pPainter.fontMetrics();
    int fontHeight  = fm.lineSpacing();

    if (paper == page) {
        QSize margin = page.size() * 0.025;
        page.adjust(margin.width(), margin.height(), -margin.width(), -margin.height());
    }

    if (isFooter) {
        int margin = 3 + 3 * fontHeight;
        page.adjust(0, 0, 0, -margin);
    }

    getDiagram(pPainter, QRectF(source), QRectF(page));

    //draw foot note
    if (isFooter) {
        page.adjust(0, 0, 0, fontHeight);
        QString string = i18n("Diagram: %2 Page %1", 1, name());
        QColor textColor(50, 50, 50);
        pPainter.setPen(textColor);
        pPainter.drawLine(page.left(), page.bottom()    , page.right(), page.bottom());
        pPainter.drawText(page.left(), page.bottom() + 3, page.right(), 2*fontHeight, Qt::AlignLeft, string);
    }

    // next painting will most probably be to a different device (i.e. the screen)
    forceUpdateWidgetFontMetrics(0);
}

/**
 * Initialize and announce a newly created widget.
 * Auxiliary to contentsMouseReleaseEvent().
 */
void UMLScene::setupNewWidget(UMLWidget *w, bool setPosition)
{
    if (setPosition &&
            !w->isPinWidget() &&
            !w->isPortWidget() &&
            !w->isObjectWidget()) {
        // ObjectWidget's position is handled by the widget
        w->setX(m_pos.x());
        w->setY(m_pos.y());
    }
    w->setVisible(true);
    w->activate();
    w->setFontCmd(font());
    w->slotFillColorChanged(ID());
    w->slotTextColorChanged(ID());
    w->slotLineWidthChanged(ID());
    m_doc->setModified();

    if (m_doc->loading()) {  // do not emit signals while loading
        addWidgetCmd(w);
        // w->activate();  // will be done by UMLDoc::activateAllViews() after loading
    } else {
        UMLApp::app()->executeCommand(new CmdCreateWidget(w));
    }
}

/**
 * Return whether we are currently creating an object.
 */
bool UMLScene::getCreateObject() const
{
    return m_bCreateObject;
}

/**
 * Set whether we are currently creating an object.
 */
void UMLScene::setCreateObject(bool bCreate)
{
    m_bCreateObject = bCreate;
}

/**
 * Overrides the standard operation.
 */
void UMLScene::showEvent(QShowEvent* /*se*/)
{
    connect(m_doc, SIGNAL(sigObjectCreated(UMLObject*)),
            this, SLOT(slotObjectCreated(UMLObject*)));
    connect(this, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
            UMLApp::app()->docWindow(), SLOT(slotAssociationRemoved(AssociationWidget*)));
    connect(this, SIGNAL(sigWidgetRemoved(UMLWidget*)),
            UMLApp::app()->docWindow(), SLOT(slotWidgetRemoved(UMLWidget*)));
}

/**
 * Overrides the standard operation.
 */
void UMLScene::hideEvent(QHideEvent* /*he*/)
{
    disconnect(m_doc, SIGNAL(sigObjectCreated(UMLObject*)), this, SLOT(slotObjectCreated(UMLObject*)));
    disconnect(this, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
               UMLApp::app()->docWindow(), SLOT(slotAssociationRemoved(AssociationWidget*)));
    disconnect(this, SIGNAL(sigWidgetRemoved(UMLWidget*)),
               UMLApp::app()->docWindow(), SLOT(slotWidgetRemoved(UMLWidget*)));
}

/**
 * Changes the current tool to the selected tool.
 * The current tool is cleaned and the selected tool initialized.
 */
void UMLScene::slotToolBarChanged(int c)
{
    m_d->setToolBarChanged((WorkToolBar::ToolBar_Buttons)c);
}

/**
 * Slot called when an object is created.
 * @param o   created UML object
 */
void UMLScene::slotObjectCreated(UMLObject* o)
{
    logDebug2("UMLScene::slotObjectCreated: scene=%1 / object=%2", name(), o->name());
    m_bPaste = false;
    //check to see if we want the message
    //may be wanted by someone else e.g. list view

    if (!m_bCreateObject) {
        return;
    }

    UMLWidget* newWidget = Widget_Factory::createWidget(this, o);

    if (!newWidget) {
        return;
    }

    setupNewWidget(newWidget);

    m_bCreateObject = false;

    if (Model_Utils::hasAssociations(o->baseType()))
    {
        createAutoAssociations(newWidget);
        // We need to invoke createAutoAttributeAssociations()
        // on all other widgets again because the newly created
        // widget might saturate some latent attribute assocs.
        createAutoAttributeAssociations2(newWidget);
    }

    UMLView* cv = activeView();
    if (cv) {
        // this should activate the bird view:
        UMLApp::app()->setCurrentView(cv, false);
    }
}

/**
 * Slot called when an object is removed.
 * @param o   removed UML object
 */
void UMLScene::slotObjectRemoved(UMLObject * o)
{
    m_bPaste = false;
    Uml::ID::Type id = o->id();

    for(UMLWidget *obj : widgetList()) {
        if (obj->id() != id)
            continue;
        removeWidget(obj);
        break;
    }
}

/**
 * Override standard method.
 */
void UMLScene::dragEnterEvent(QGraphicsSceneDragDropEvent *e)
{
    UMLDragData::LvTypeAndID_List tidList;
    if (!UMLDragData::getClip3TypeAndID(e->mimeData(), tidList)) {
        logDebug0("UMLScene::dragEnterEvent: UMLDragData::getClip3TypeAndID returned false");
        return;
    }
    const DiagramType::Enum diagramType = type();

    bool bAccept = true;
    for(UMLDragData::LvTypeAndID_List::const_iterator it = tidList.begin(); it != tidList.end(); it++) {
        UMLListViewItem::ListViewType lvtype = (*it)->type;
        Uml::ID::Type id = (*it)->id;

        UMLObject *temp = nullptr;
        //if dragging diagram - might be a drag-to-note
        if (Model_Utils::typeIsDiagram(lvtype)) {
            break;
        }
        //can't drag anything onto state/activity diagrams
        if (diagramType == DiagramType::State || diagramType == DiagramType::Activity) {
            bAccept = false;
            break;
        }
        //make sure can find UMLObject
        if (!(temp = m_doc->findObjectById(id))) {
            logDebug1("UMLScene::dragEnterEvent: object %1 not found", Uml::ID::toString(id));
            bAccept = false;
            break;
        }
        if (!Model_Utils::typeIsAllowedInDiagram(temp, this)) {
            logDebug2("UMLScene::dragEnterEvent: %1 is not allowed in diagram type %2", temp->name(), diagramType);
            bAccept = false;
            break;
        }
    }
    if (bAccept) {
        e->accept();
    } else {
        e->ignore();
    }
}

/**
 * Override standard method.
 */
void UMLScene::dragMoveEvent(QGraphicsSceneDragDropEvent* e)
{
    e->accept();
}

/**
 * Override standard method.
 */
void UMLScene::dropEvent(QGraphicsSceneDragDropEvent *e)
{
    UMLDragData::LvTypeAndID_List tidList;
    if (!UMLDragData::getClip3TypeAndID(e->mimeData(), tidList)) {
        logDebug0("UMLScene::dropEvent: UMLDragData::getClip3TypeAndID returned error");
        return;
    }
    m_pos = e->scenePos();

    for(UMLDragData::LvTypeAndID_List::const_iterator it = tidList.begin(); it != tidList.end(); it++) {
        UMLListViewItem::ListViewType lvtype = (*it)->type;
        Uml::ID::Type id = (*it)->id;

        if (Model_Utils::typeIsDiagram(lvtype)) {
            bool breakFlag = false;
            UMLWidget *w = nullptr;
            for(w: widgetList()) {
                if (w->isNoteWidget() && w->onWidget(e->scenePos())) {
                    breakFlag = true;
                    break;
                }
            }
            if (breakFlag) {
                NoteWidget *note = static_cast<NoteWidget*>(w);
                note->setDiagramLink(id);
            }
            continue;
        }
        UMLObject* o = m_doc->findObjectById(id);
        if (!o) {
            logDebug1("UMLScene::dropEvent: object id=%1 not found", Uml::ID::toString(id));
            continue;
        }

        UMLWidget* newWidget = Widget_Factory::createWidget(this, o);
        if (!newWidget) {
            logWarn1("UMLScene::dropEvent could not create widget for uml object %1", o->name());
            continue;
        }

        setupNewWidget(newWidget, true);
        m_pos += QPointF(UMLWidget::DefaultMinimumSize.width(), UMLWidget::DefaultMinimumSize.height());
        createAutoAssociations(newWidget);
        createAutoAttributeAssociations2(newWidget);
    }
}

/**
 * Overrides the standard operation.
 * Calls the same method in the current tool bar state.
 */
void UMLScene::mouseMoveEvent(QGraphicsSceneMouseEvent* ome)
{
    if (m_d->inMouseMoveEvent)
        return;
    m_d->inMouseMoveEvent = true;
    m_d->toolBarState->mouseMove(ome);
    m_d->inMouseMoveEvent = false;
}

/**
 * Override standard method.
 * Calls the same method in the current tool bar state.
 */
void UMLScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    m_d->toolBarState->mousePress(event);

    // setup document
    if (selectedItems().count() == 0)
        UMLApp::app()->docWindow()->showDocumentation(this);
    event->accept();
}

/**
 * Override standard method.
 * Calls the same method in the current tool bar state.
 */
void UMLScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_doc->loading())
        m_d->toolBarState->mouseDoubleClick(event);
    if (!event->isAccepted()) {
        // show properties dialog of the scene
        if (m_view->showPropertiesDialog() == true) {
            m_doc->setModified();
        }
        event->accept();
    }
}

/**
 * Overrides the standard operation.
 * Calls the same method in the current tool bar state.
 */
void UMLScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* ome)
{
    m_d->toolBarState->mouseRelease(ome);
}

/**
 * Determine whether on a sequence diagram we have clicked on a line
 * of an Object.
 *
 * @return The widget owning the line which was clicked.
 *  Returns 0 if no line was clicked on.
 */
ObjectWidget * UMLScene::onWidgetLine(const QPointF &point) const
{
    for(UMLWidget *obj : widgetList()) {
        ObjectWidget *ow = obj->asObjectWidget();
        if (ow == 0)
            continue;
        SeqLineWidget *pLine = ow->sequentialLine();
        if (pLine == 0) {
            logError2("UMLScene::onWidgetLine: SeqLineWidget of %1 (id %2) is null",
                      ow->name(), Uml::ID::toString(ow->localID()));
            continue;
        }
        if (pLine->onWidget(point))
            return ow;
    }
    return 0;
}

/**
 * Determine whether on a sequence diagram we have clicked on
 * the destruction box of an Object.
 *
 * @return The widget owning the destruction box which was clicked.
 *  Returns 0 if no destruction box was clicked on.
 */
ObjectWidget * UMLScene::onWidgetDestructionBox(const QPointF &point) const
{
    for(UMLWidget *obj : widgetList()) {
        ObjectWidget *ow = obj->asObjectWidget();
        if (ow == 0)
            continue;
        SeqLineWidget *pLine = ow->sequentialLine();
        if (pLine == 0) {
            logError2("UMLScene::onWidgetDestructionBox: SeqLineWidget of %1 (id %2) is null",
                      ow->name(), Uml::ID::toString(ow->localID()));
            continue;
        }
        if (pLine->onDestructionBox(point))
            return ow;
    }
    return 0;
}

/**
 * Return pointer to the first selected widget (for multi-selection)
 */
UMLWidget* UMLScene::getFirstMultiSelectedWidget() const
{
    if (selectedWidgets().size() == 0)
        return 0;
    return selectedWidgets().first();
}

/**
 * Checks the specified point against all widgets and returns the widget
 * for which the point is within its bounding box.
 * @param p Point in scene coordinates to search for
 * @return Returns the first widget of type UMLWidget returned by QGraphicsScene::items() for multiple matches
 * @return Returns NULL if the point is not inside any widget.
 */
UMLWidget* UMLScene::widgetAt(const QPointF& p)
{
    for(QGraphicsItem  *item : items(p)) {
        UMLWidget *w = dynamic_cast<UMLWidget*>(item);
        if (w)
            return w;
    }
    return nullptr;
}

/**
 * Tests the given point against all associations and returns the
 * association widget for which the point is on the line.
 * Returns NULL if the point is not inside any association.
 * CHECK: This is the same method as in ToolBarState.
 */
AssociationWidget* UMLScene::associationAt(const QPointF& p)
{
    for(AssociationWidget *association : associationList()) {
        if (association->onAssociation(p)) {
            return association;
        }
    }
    return 0;
}

/**
 * Tests the given point against all associations and returns the
 * association widget for which the point is on the line.
 * Returns NULL if the point is not inside any association.
 */
MessageWidget* UMLScene::messageAt(const QPointF& p)
{
    for(MessageWidget  *message : messageList()) {
        if (message->onWidget(p)) {
            return message;
        }
    }
    return 0;
}

/**
 * Sees if a message is relevant to the given widget.  If it does delete it.
 * @param w The widget to check messages against.
 */
void UMLScene::checkMessages(ObjectWidget * w)
{
    if (type() != DiagramType::Sequence) {
        return;
    }

    for(MessageWidget  *obj : messageList()) {
        if (obj->hasObjectWidget(w)) {
            removeWidgetCmd(obj);
        }
    }
}

/**
 * Returns whether a widget is already on the diagram.
 *
 * @param id The id of the widget to check for.
 *
 * @return Returns pointer to the widget if it is on the diagram, NULL if not.
 */
UMLWidget* UMLScene::widgetOnDiagram(Uml::ID::Type id)
{
    for(UMLWidget  *obj : widgetList()) {
        if (!obj)
            continue;
        UMLWidget* w = obj->widgetWithID(id);
        if (w)
            return w;
    }

    for(UMLWidget  *obj : messageList()) {
        // CHECK: Should MessageWidget reimplement widgetWithID() ?
        //       If yes then we should use obj->widgetWithID(id) here too.
        if (id == obj->id())
            return obj;
    }

    return 0;
}

/**
 * Returns whether a widget is already on the diagram.
 *
 * @param type The type of the widget to check for.
 *
 * @return Returns pointer to the widget if it is on the diagram, NULL if not.
 */
UMLWidget* UMLScene::widgetOnDiagram(WidgetBase::WidgetType type)
{
    for(UMLWidget  *widget : widgetList()) {
        if (!widget)
            continue;
        if (widget->baseType() == type)
            return widget;
    }
    return nullptr;
}

/**
 * Finds a widget with the given ID.
 * Search both our UMLWidget AND MessageWidget lists.
 * @param id The ID of the widget to find.
 *
 * @return Returns the widget found, returns 0 if no widget found.
 */
UMLWidget * UMLScene::findWidget(Uml::ID::Type id)
{
    for(UMLWidget *obj : widgetList()) {
        if (!obj)
            continue;
        UMLWidget* w = obj->widgetWithID(id);
        if (w) {
            return w;
        }
    }

    for(UMLWidget *obj : messageList()) {
        // CHECK: Should MessageWidget reimplement widgetWithID() ?
        //       If yes then we should use obj->widgetWithID(id) here too.
        if (obj->localID() == id ||
            obj->id() == id)
            return obj;
    }

    return 0;
}

/**
 * Finds an association widget with the given ID.
 *
 * @param id The ID of the widget to find.
 *
 * @return Returns the widget found, returns 0 if no widget found.
 */
AssociationWidget * UMLScene::findAssocWidget(Uml::ID::Type id)
{
    for(AssociationWidget *obj : associationList()) {
        UMLAssociation* umlassoc = obj->association();
        if (umlassoc && umlassoc->id() == id) {
            return obj;
        }
    }
    return 0;
}

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
AssociationWidget * UMLScene::findAssocWidget(UMLWidget *pWidgetA,
                                              UMLWidget *pWidgetB, const QString& roleNameB)
{
    for(AssociationWidget *assoc : associationList()) {
        const Uml::AssociationType::Enum testType = assoc->associationType();
        if (testType != Uml::AssociationType::Association &&
                testType != Uml::AssociationType::UniAssociation &&
                testType != Uml::AssociationType::Composition &&
                testType != Uml::AssociationType::Aggregation &&
                testType != Uml::AssociationType::Relationship) {
            continue;
        }

        if (pWidgetA->id() == assoc->widgetIDForRole(Uml::RoleType::A) &&
                pWidgetB->id() == assoc->widgetIDForRole(Uml::RoleType::B) &&
                assoc->roleName(Uml::RoleType::B) == roleNameB) {
            return assoc;
        }
    }
    return 0;
}

/**
 * Finds an association widget with the given type and widgets.
 *
 * @param at  The AssociationType of the widget to find.
 * @param pWidgetA Pointer to the UMLWidget of role A.
 * @param pWidgetB Pointer to the UMLWidget of role B.
 *
 * @return Returns the widget found, returns 0 if no widget found.
 */
AssociationWidget * UMLScene::findAssocWidget(AssociationType::Enum at,
                                              UMLWidget *pWidgetA, UMLWidget *pWidgetB)
{
    for(AssociationWidget *assoc : associationList()) {
        Uml::AssociationType::Enum testType = assoc->associationType();
        if (testType != at) {
            continue;
        }

        if (pWidgetA->id() == assoc->widgetIDForRole(Uml::RoleType::A) &&
                pWidgetB->id() == assoc->widgetIDForRole(Uml::RoleType::B)) {
            return assoc;
        }
    }
    return 0;
}

/**
 * Remove a widget from view (undo command)
 *
 * @param o  The widget to remove.
 */
void UMLScene::removeWidget(UMLWidget * o)
{
    UMLApp::app()->executeCommand(new CmdRemoveWidget(o));
}

/**
 * Remove an associationwidget from view (undo command)
 *
 * @param w  The associationwidget to remove.
 */
void UMLScene::removeWidget(AssociationWidget* w)
{
    UMLApp::app()->executeCommand(new CmdRemoveWidget(w));
}

/**
 * Remove a widget from view.
 *
 * @param o  The widget to remove.
 */
void UMLScene::removeWidgetCmd(UMLWidget * o)
{
    if (!o)
        return;

    Q_EMIT sigWidgetRemoved(o);

    removeAssociations(o);

    removeOwnedWidgets(o);

    WidgetBase::WidgetType t = o->baseType();
    if (type() == DiagramType::Sequence && t == WidgetBase::wt_Object) {
        checkMessages(static_cast<ObjectWidget*>(o));
    }

    o->cleanup();
    if (!UMLApp::app()->shuttingDown()) {
        /* Manipulating the Selected flag during shutdown may crash:
           UMLWidget::setSelectedFlag ->
             WidgetBase::setSelected ->
                QGraphicsObjectWrapper::setSelected ->
                  QGraphicsItem::setSelected ->
                    QGraphicsObjectWrapper::itemChange ->
                      UMLWidget::setSelected ->
                        UMLApp::slotCopyChanged ->
                          UMLListView::selectedItemsCount ->
                            UMLListView::selectedItems ->
                              Crash somewhere in qobject_cast
                              (listview already destructed?)
         */
        o->setSelectedFlag(false);
    }
    disconnect(this, SIGNAL(sigFillColorChanged(Uml::ID::Type)), o, SLOT(slotFillColorChanged(Uml::ID::Type)));
    disconnect(this, SIGNAL(sigLineColorChanged(Uml::ID::Type)), o, SLOT(slotLineColorChanged(Uml::ID::Type)));
    disconnect(this, SIGNAL(sigTextColorChanged(Uml::ID::Type)), o, SLOT(slotTextColorChanged(Uml::ID::Type)));
    removeItem(o);
    m_doc->setModified(true);
    update();
}

/**
 * Remove all widgets that have given widget as owner.
 *
 * @param o The owner widget that will be removed.
 */
void UMLScene::removeOwnedWidgets(UMLWidget* o)
{
    for(QGraphicsItem *item : o->childItems()) {
        UMLWidget* widget = dynamic_cast<UMLWidget*>(item);
        if ((widget != 0) &&
            (widget->isPinWidget() ||
             widget->isPortWidget())) {
            removeWidgetCmd(widget);
        }
    }
}

/**
 * Returns background color
 */
const QColor& UMLScene::backgroundColor() const
{
    return backgroundBrush().color();
}

/**
 * Returns whether to use the fill/background color
 */
bool UMLScene::useFillColor() const
{
    return m_Options.uiState.useFillColor;
}

/**
 * Sets whether to use the fill/background color
 */
void UMLScene::setUseFillColor(bool ufc)
{
    m_Options.uiState.useFillColor = ufc;
}

/**
 * Gets the smallest area to print.
 *
 * @return Returns the smallest area to print.
 */
QRectF UMLScene::diagramRect()
{
    return itemsBoundingRect();
}

/**
 * Returns a list of selected widgets
 * @return list of selected widgets based on class UMLWidget
 * @note This method returns widgets including message widgets, but no association widgets
 */
UMLWidgetList UMLScene::selectedWidgets() const
{
    QList<QGraphicsItem *> items = selectedItems();

    UMLWidgetList widgets;
    for(QGraphicsItem  *item : items) {
        UMLWidget *w = dynamic_cast<UMLWidget*>(item);
        if (w)
            widgets.append(w);
    }
    return widgets;
}

/**
 * Returns a list of selected association widgets
 * @return list of selected widgets based on class AssociationWidget
 */
AssociationWidgetList UMLScene::selectedAssociationWidgets() const
{
    QList<QGraphicsItem *> items = selectedItems();

    AssociationWidgetList widgets;
    for(QGraphicsItem  *item : items) {
        AssociationWidget *w = dynamic_cast<AssociationWidget*>(item);
        if (w)
            widgets.append(w);
    }
    return widgets;
}

/**
 * Returns a list of selected message widgets
 * @return list of selected widgets based on class MessageWidget
 */
UMLWidgetList UMLScene::selectedMessageWidgets() const
{
    QList<QGraphicsItem *> items = selectedItems();

    UMLWidgetList widgets;
    for(QGraphicsItem  *item : items) {
        MessageWidget *w = dynamic_cast<MessageWidget*>(item);
        if (w) {
            widgets.append(w);
        } else {
            WidgetBase *wb = dynamic_cast<WidgetBase*>(item);
            QString name = (wb ? wb->name() : QStringLiteral("(null)"));
            logDebug1("UMLScene::selectedMessageWidgets: %1 is not a MessageWidget", name);
        }
    }
    return widgets;
}

/**
 *  Clear the selected widgets list.
 */
void UMLScene::clearSelected()
{
    QList<QGraphicsItem *> items = selectedItems();
    for(QGraphicsItem  *item : items) {
        WidgetBase *wb = dynamic_cast<WidgetBase*>(item);
        if (wb) {
            wb->setSelected(false);
        }
    }
    clearSelection();
    //m_doc->enableCutCopy(false);
}

/**
 * Move all the selected widgets by a relative X and Y offset.
 * TODO: Only used in UMLApp::handleCursorKeyReleaseEvent
 *
 * @param dX The distance to move horizontally.
 * @param dY The distance to move vertically.
 */
void UMLScene::moveSelectedBy(qreal dX, qreal dY)
{
    // logDebug1("UMLScene::moveSelectedBy: m_selectedList count=%1", m_selectedList.count());
    for(UMLWidget  *w : selectedWidgets()) {
        w->moveByLocal(dX, dY);
    }
}

/**
 * Set the useFillColor variable to all selected widgets
 *
 * @param useFC The state to set the widget to.
 */
void UMLScene::selectionUseFillColor(bool useFC)
{
    if (useFC) {
        UMLApp::app()->beginMacro(i18n("Use fill color"));
    } else {
        UMLApp::app()->beginMacro(i18n("No fill color"));
    }

    for(UMLWidget *widget : selectedWidgets()) {
        widget->setUseFillColor(useFC);
    }

    UMLApp::app()->endMacro();
}

/**
 * Set the font for all the currently selected items.
 */
void UMLScene::selectionSetFont(const QFont &font)
{
    UMLApp::app()->beginMacro(i18n("Change font"));

    for(UMLWidget *temp : selectedWidgets()) {
        temp->setFont(font);
    }

    UMLApp::app()->endMacro();
}

/**
 * Set the line color for all the currently selected items.
 */
void UMLScene::selectionSetLineColor(const QColor &color)
{
    UMLApp::app()->beginMacro(i18n("Change line color"));

    for(UMLWidget  *temp : selectedWidgets()) {
        temp->setLineColor(color);
    }
    AssociationWidgetList assoclist = selectedAssocs();
    for(AssociationWidget  *aw : assoclist) {
        aw->setLineColor(color);
    }

    UMLApp::app()->endMacro();
}

/**
 * Set the line width for all the currently selected items.
 */
void UMLScene::selectionSetLineWidth(uint width)
{
    UMLApp::app()->beginMacro(i18n("Change line width"));

    for(UMLWidget *temp : selectedWidgets()) {
        temp->setLineWidth(width);
        temp->setUsesDiagramLineWidth(false);
    }
    AssociationWidgetList assoclist = selectedAssocs();
    for(AssociationWidget  *aw : assoclist) {
        aw->setLineWidth(width);
        aw->setUsesDiagramLineWidth(false);
    }

    UMLApp::app()->endMacro();
}

/**
 * Set the fill color for all the currently selected items.
 */
void UMLScene::selectionSetFillColor(const QColor &color)
{
    UMLApp::app()->beginMacro(i18n("Change fill color"));

    for(UMLWidget *widget : selectedWidgets()) {
        widget->setFillColor(color);
        widget->setUsesDiagramFillColor(false);
    }

    UMLApp::app()->endMacro();
}

/**
 * Set or unset the visual property (show ..) setting of all selected items.
 */
void UMLScene::selectionSetVisualProperty(ClassifierWidget::VisualProperty property, bool value)
{
    UMLApp::app()->beginMacro(i18n("Change visual property"));

    for(UMLWidget  *temp : selectedWidgets()) {
        ClassifierWidget *cw = temp->asClassifierWidget();
        cw->setVisualProperty(property, value);
    }

    UMLApp::app()->endMacro();
}

/**
 * Unselect child widgets when their owner is already selected.
 */
void UMLScene::unselectChildrenOfSelectedWidgets()
{
    for(UMLWidget *widget : selectedWidgets()) {
        if (widget->isPinWidget() ||
            widget->isPortWidget()) {
            for(UMLWidget *potentialParentWidget : selectedWidgets()) {
                if (widget->parentItem() == potentialParentWidget) {
                    widget->setSelectedFlag(false);
                }
            }
        }
    }
}

/**
 * Delete the selected widgets list and the widgets in it.
 */
void UMLScene::deleteSelection()
{
    AssociationWidgetList selectedAssociations = selectedAssociationWidgets();
    int selectionCount = selectedWidgets().count() + selectedAssociations.count();

    if (selectionCount == 0)
        return;

    // check related associations
    bool hasAssociations = false;
    for(UMLWidget *widget : selectedWidgets()) {
        if (widget->isTextWidget() && widget->asFloatingTextWidget()->textRole() != Uml::TextRole::Floating) {
            continue;
        }
        if (widget->isMessageWidget() || widget->associationWidgetList().size() > 0)
            hasAssociations = true;
    }

    if (hasAssociations && !Dialog_Utils::askDeleteAssociation())
        return;

    UMLApp::app()->beginMacro(i18n("Delete widgets"));

    unselectChildrenOfSelectedWidgets();

    for(UMLWidget *widget : selectedWidgets()) {
        //  Don't delete text widget that are connect to associations as these will
        //  be cleaned up by the associations.
        if (widget->isTextWidget() &&
                widget->asFloatingTextWidget()->textRole() != Uml::TextRole::Floating) {
            widget->setSelectedFlag(false);
            widget->hide();
        } else if (widget->isPortWidget()) {
            UMLObject *o = widget->umlObject();
            removeWidget(widget);
            if (o)
                UMLApp::app()->executeCommand(new CmdRemoveUMLObject(o));
            // message widgets are handled later
        } else if (!widget->isMessageWidget()){
            removeWidget(widget);
        }
    }

    // Delete any selected associations.
    for(AssociationWidget *assocwidget : selectedAssociations) {
        removeWidget(assocwidget);
    }

    // we also have to remove selected messages from sequence diagrams
    for(UMLWidget *cur_msgWgt : selectedMessageWidgets()) {
        removeWidget(cur_msgWgt);
    }

    //make sure list empty - it should be anyway, just a check.
    clearSelected();

    UMLApp::app()->endMacro();
}

/**
 * resize selected widgets
 */
void UMLScene::resizeSelection()
{
    int selectionCount = selectedWidgets().count();

    if (selectionCount > 1) {
        UMLApp::app()->beginMacro(i18n("Resize widgets"));
    }

    if (selectedCount() == 0)
        return;
    for(UMLWidget  *w : selectedWidgets()) {
        w->resize();
    }
    m_doc->setModified();

    if (selectionCount > 1) {
        UMLApp::app()->endMacro();
    }
}

/**
 * Selects all widgets
 */
void UMLScene::selectAll()
{
    selectWidgets(sceneRect().left(), sceneRect().top(), sceneRect().right(), sceneRect().bottom());
}

/**
 * Returns true if this diagram resides in an externalized folder.
 * CHECK: It is probably cleaner to move this to the UMLListViewItem.
 */
bool UMLScene::isSavedInSeparateFile()
{
    if (optionState().generalState.tabdiagrams) {
        // Umbrello currently does not support external folders
        // when tabbed diagrams are enabled.
        return false;
    }
    const QString msgPrefix(QStringLiteral("UMLScene::isSavedInSeparateFile(") + name() + QStringLiteral("): "));
    UMLListView *listView = UMLApp::app()->listView();
    UMLListViewItem *lvItem = listView->findItem(m_nID);
    if (lvItem == 0) {
        logError2("UMLScene::isSavedInSeparateFile(%1) : listView->findItem(%2) returns false",
                  name(), Uml::ID::toString(m_nID));
        return false;
    }
    UMLListViewItem *parentItem = dynamic_cast<UMLListViewItem*>(lvItem->parent());
    if (parentItem == 0) {
        logError1("UMLScene::isSavedInSeparateFile(%1) : parent item in listview is not a UMLListViewItem (?)",
                  name());
        return false;
    }
    const UMLListViewItem::ListViewType lvt = parentItem->type();
    if (! Model_Utils::typeIsFolder(lvt))
        return false;
    const UMLFolder *modelFolder = parentItem->umlObject()->asUMLFolder();
    if (modelFolder == 0) {
        logError1("UMLScene::isSavedInSeparateFile(%1) : parent model object is not a UMLFolder (?)",
                  name());
        return false;
    }
    QString folderFile = modelFolder->folderFile();
    return !folderFile.isEmpty();
}

UMLSceneItemList UMLScene::collisions(const QPointF &p, int delta)
{
    QPointF a = p-QPointF(delta, delta);
    QPointF b = p+QPointF(delta, delta);
    QList<QGraphicsItem *> list = items(QRectF(a, b));
    return list;
}

/**
 * Calls setSelected on the given UMLWidget and enters
 * it into the m_selectedList while making sure it is
 * there only once.
 */
void UMLScene::makeSelected(UMLWidget* uw)
{
    if (uw) {
        uw->setSelected(true);
    }
}

/**
 * Selects all the widgets of the given association widget.
 */
void UMLScene::selectWidgetsOfAssoc(AssociationWidget * a)
{
    if (a) {
        a->setSelected(true);
        //select the two widgets
        makeSelected(a->widgetForRole(Uml::RoleType::A));
        makeSelected(a->widgetForRole(Uml::RoleType::B));
        //select all the text
        makeSelected(a->multiplicityWidget(Uml::RoleType::A));
        makeSelected(a->multiplicityWidget(Uml::RoleType::B));
        makeSelected(a->roleWidget(Uml::RoleType::A));
        makeSelected(a->roleWidget(Uml::RoleType::B));
        makeSelected(a->changeabilityWidget(Uml::RoleType::A));
        makeSelected(a->changeabilityWidget(Uml::RoleType::B));
    }
}

/**
 * Selects all the widgets within an internally kept rectangle.
 */
void UMLScene::selectWidgets(qreal px, qreal py, qreal qx, qreal qy)
{
    clearSelected();

    QRectF  rect;
    if (px <= qx) {
        rect.setLeft(px);
        rect.setRight(qx);
    } else {
        rect.setLeft(qx);
        rect.setRight(px);
    }
    if (py <= qy) {
        rect.setTop(py);
        rect.setBottom(qy);
    } else {
        rect.setTop(qy);
        rect.setBottom(py);
    }

    // Select UMLWidgets that fall within the selection rectangle
    for(UMLWidget *temp : widgetList()) {
        uIgnoreZeroPointer(temp);
        selectWidget(temp, &rect);
    }

    // Select messages that fall within the selection rectangle
    for(MessageWidget *temp : messageList()) {
        selectWidget(temp->asUMLWidget(), &rect);
    }

    // Select associations of selected widgets
    selectAssociations(true);

    // Automatically select all messages if two object widgets are selected
    for(MessageWidget  *w : messageList()) {
        if (w->objectWidget(Uml::RoleType::A) &&
            w->objectWidget(Uml::RoleType::B) &&
            w->objectWidget(Uml::RoleType::A)->isSelected() &&
            w->objectWidget(Uml::RoleType::B)->isSelected()) {
            makeSelected(w);
        }
    }
}

/**
 * Select a single widget
 *
 * If QRectF* rect is provided, the selection is only made if the widget is
 * visible within the rectangle.
 */
void UMLScene::selectWidget(UMLWidget* widget, QRectF* rect)
{
    if (rect == 0) {
        makeSelected(widget);
        return;
    }

    int x = widget->x();
    int y = widget->y();
    int w = widget->width();
    int h = widget->height();
    QRectF  rect2(x, y, w, h);

    //see if any part of widget is in the rectangle
    if (!rect->intersects(rect2)) {
        return;
    }

    //if it is text that is part of an association then select the association
    //and the objects that are connected to it.
    if (widget->isTextWidget()) {
        FloatingTextWidget *ft = widget->asFloatingTextWidget();
        Uml::TextRole::Enum t = ft->textRole();
        LinkWidget *lw = ft->link();
        MessageWidget * mw = dynamic_cast<MessageWidget*>(lw);
        if (mw) {
            makeSelected(mw);
        } else if (t != Uml::TextRole::Floating) {
            AssociationWidget * a = dynamic_cast<AssociationWidget*>(lw);
            if (a)
                selectWidgetsOfAssoc(a);
        }
    } else if (widget->isMessageWidget()) {
        MessageWidget *mw = widget->asMessageWidget();
        makeSelected(mw);
    }
    if (widget->isVisible()) {
        makeSelected(widget);
    }
}

/**
 * Selects all the widgets from a list.
 */
void UMLScene::selectWidgets(UMLWidgetList &widgets)
{
    for(UMLWidget *widget : widgets)
        makeSelected(widget);
}

/**
 * Returns the PNG picture of the paste operation.
 * @param diagram the class to store PNG picture of the paste operation.
 * @param rect the area of the diagram to copy
 */
void  UMLScene::getDiagram(QPixmap &diagram, const QRectF &rect)
{
    logDebug4("UMLScene::getDiagram pixmap(w=%1 h=%2) / copyArea(w=%3 h=%4)",
               diagram.rect().width(), diagram.rect().height(),
               rect.width(), rect.height());
    QPainter painter(&diagram);
    painter.fillRect(0, 0, rect.width(), rect.height(), Qt::white);
    getDiagram(painter, rect);
}

/**
 * Paint diagram to the paint device
 * @param painter the QPainter to which the diagram is painted
 * @param source the area of the diagram to copy
 * @param target the rect where to paint into
 */
void  UMLScene::getDiagram(QPainter &painter, const QRectF &source, const QRectF &target)
{
    DEBUG() << "UMLScene::getDiagram painter=" << painter.window() << ", source=" << source << ", target=" << target; //@todo logDebug
    //TODO unselecting and selecting later doesn't work now as the selection is
    //cleared in UMLSceneImageExporter. Check if the anything else than the
    //following is needed and, if it works, remove the clearSelected in
    //UMLSceneImageExporter and UMLSceneImageExporterModel

    UMLWidgetList selected = selectedWidgets();
    for(UMLWidget *widget : selected) {
        widget->setSelected(false);
    }
    AssociationWidgetList selectedAssociationsList = selectedAssocs();

    for(AssociationWidget *association : selectedAssociationsList) {
        association->setSelected(false);
    }

    // we don't want to get the grid
    bool showSnapGrid = isSnapGridVisible();
    setSnapGridVisible(false);

    const int sourceMargin = 1;
    QRectF alignedSource(source);
    alignedSource.adjust(-sourceMargin, -sourceMargin, sourceMargin, sourceMargin);

    logDebug0("UMLScene::getDiagram TODO: Check if this render method is identical to canvas()->drawArea()");
    // [PORT]
    render(&painter, target, alignedSource, Qt::KeepAspectRatio);

    setSnapGridVisible(showSnapGrid);

    //select again
    for(UMLWidget *widget : selected) {
        widget->setSelected(true);
    }
    for(AssociationWidget *association : selectedAssociationsList) {
        association->setSelected(true);
    }
}

/**
 * Returns the imageExporter used to export the view.
 *
 * @return The imageExporter used to export the view.
 */
UMLViewImageExporter* UMLScene::getImageExporter()
{
    return m_pImageExporter;
}

/**
 * makes this view the active view by asking the document to show us
 */
void UMLScene::slotActivate()
{
    m_doc->changeCurrentView(ID());
}

/**
 * Activate all the objects and associations after a load from the clipboard
 */
void UMLScene::activate()
{
    //Activate Regular widgets then activate  messages
    for(UMLWidget *obj : widgetList()) {
        uIgnoreZeroPointer(obj);
        //If this UMLWidget is already activated or is a MessageWidget then skip it
        if (obj->isActivated() || obj->isMessageWidget()) {
            continue;
        }

       if (obj->activate()) {
           obj->setVisible(true);
       } else {
           removeItem(obj);
           delete obj;
       }
    }//end Q_FOREACH

    //Activate Message widgets
    for(UMLWidget *obj : messageList()) {
        //If this MessageWidget is already activated then skip it
        if (obj->isActivated())
            continue;

        obj->activate(m_doc->changeLog());
        obj->setVisible(true);

    }//end Q_FOREACH

    // Activate all association widgets

    for(AssociationWidget *aw : associationList()) {
        if (aw->activate()) {
            if (m_PastePoint.x() != 0) {
                int x = m_PastePoint.x() - m_pos.x();
                int y = m_PastePoint.y() - m_pos.y();
                aw->moveEntireAssoc(x, y);
            }
        } else {
            removeWidgetCmd(aw);
            delete aw;
        }
    }
}

/**
 * Return the amount of widgets selected.
 *
 * @param filterText  When true, do NOT count floating text widgets that
 *                    belong to other widgets (i.e. only count TextRole::Floating.)
 *                    Default: Count all widgets.
 * @return  Number of widgets selected.
 */
int UMLScene::selectedCount(bool filterText) const
{
    if (!filterText)
        return selectedWidgets().count();
    int counter = 0;
    for(UMLWidget *temp : selectedWidgets()) {
        if (temp->isTextWidget()) {
            const FloatingTextWidget *ft = static_cast<const FloatingTextWidget*>(temp);
            if (ft->textRole() == TextRole::Floating)
                counter++;
        } else {
            counter++;
        }
    }
    return counter;
}

/**
 * Fills the List with all the selected widgets from the diagram
 * The list can be filled with all the selected widgets, or be filtered to prevent
 * text widgets other than tr_Floating to be append.
 *
 * @param filterText Don't append the text unless their role is TextRole::Floating
 * @return           The UMLWidgetList to fill.
 */
UMLWidgetList UMLScene::selectedWidgetsExt(bool filterText /*= true*/)
{
    UMLWidgetList widgetList;

    for(UMLWidget *widgt : selectedWidgets()) {
        if (filterText && widgt->isTextWidget()) {
            FloatingTextWidget *ft = widgt->asFloatingTextWidget();
            if (ft->textRole() == Uml::TextRole::Floating)
                widgetList.append(widgt);
        } else {
            widgetList.append(widgt);
        }
    }
    return widgetList;
}

/**
 * Returns a list with all the selected associations from the diagram
 */
AssociationWidgetList UMLScene::selectedAssocs()
{
    AssociationWidgetList assocWidgetList;

    for(AssociationWidget *assocwidget : associationList()) {
        if (assocwidget->isSelected())
            assocWidgetList.append(assocwidget);
    }
    return assocWidgetList;
}

/**
 * Adds a floating text widget to the view
 */
void UMLScene::addFloatingTextWidget(FloatingTextWidget* pWidget)
{
    /* We cannot do the range check like this:
       pWidget's x() and y() can legitimately be negative.
       The scene's origin point (0,0) is somewhere in the middle of the diagram
       area. QGraphicsItems located left or up from the scene's origin have
       negative coordinates.
       sceneRect() returns non negative coordinates such as (0,0,5000,5000).
       That does not fit with the negative widget coordinates. */
#if 0
    int wX = pWidget->x();
    int wY = pWidget->y();
    bool xIsOutOfRange = (wX < sceneRect().left() || wX > sceneRect().right());
    bool yIsOutOfRange = (wY < sceneRect().top() || wY > sceneRect().bottom());
    if (xIsOutOfRange || yIsOutOfRange) {
        QString name = pWidget->name();
        if (name.isEmpty()) {
            FloatingTextWidget *ft = pWidget->asFloatingTextWidget();
            if (ft)
                name = ft->displayText();
        }
        logDebug4("UMLScene::addFloatingTextWidget(%1) type=%2 : position (%3,%4) is out of range",
                  name, pWidget->baseTypeStr(), wX, wY);
        if (xIsOutOfRange) {
            pWidget->setX(0);
            wX = 0;
        }
        if (yIsOutOfRange) {
            pWidget->setY(0);
            wY = 0;
        }
    }
#endif
    addWidgetCmd(pWidget);
}

/**
 * Adds an association to the view from the given data.
 * Use this method when pasting.
 */
bool UMLScene::addAssociation(AssociationWidget* pAssoc, bool isPasteOperation)
{
    if (!pAssoc) {
        return false;
    }
    const Uml::AssociationType::Enum assocType = pAssoc->associationType();

    if (isPasteOperation) {
        IDChangeLog * log = m_doc->changeLog();

        if (!log) {
            return false;
        }

        Uml::ID::Type ida = Uml::ID::None, idb = Uml::ID::None;
        if (type() == DiagramType::Collaboration || type() == DiagramType::Sequence) {
            //check local log first
            ida = m_pIDChangesLog->findNewID(pAssoc->widgetIDForRole(Uml::RoleType::A));
            idb = m_pIDChangesLog->findNewID(pAssoc->widgetIDForRole(Uml::RoleType::B));
            //if either is still not found and assoc type is anchor
            //we are probably linking to a notewidet - else an error
            if (ida == Uml::ID::None && assocType == Uml::AssociationType::Anchor)
                ida = log->findNewID(pAssoc->widgetIDForRole(Uml::RoleType::A));
            if (idb == Uml::ID::None && assocType == Uml::AssociationType::Anchor)
                idb = log->findNewID(pAssoc->widgetIDForRole(Uml::RoleType::B));
        } else {
            Uml::ID::Type oldIdA = pAssoc->widgetIDForRole(Uml::RoleType::A);
            Uml::ID::Type oldIdB = pAssoc->widgetIDForRole(Uml::RoleType::B);
            ida = log->findNewID(oldIdA);
            if (ida == Uml::ID::None) {  // happens after a cut
                if (oldIdA == Uml::ID::None) {
                    return false;
                }
                ida = oldIdA;
            }
            idb = log->findNewID(oldIdB);
            if (idb == Uml::ID::None) {  // happens after a cut
                if (oldIdB == Uml::ID::None) {
                    return false;
                }
                idb = oldIdB;
            }
        }
        if (ida == Uml::ID::None || idb == Uml::ID::None) {
            return false;
        }
        // cant do this anymore.. may cause problem for pasting
        //      pAssoc->setWidgetID(ida, A);
        //      pAssoc->setWidgetID(idb, B);
        pAssoc->setWidgetForRole(findWidget(ida), Uml::RoleType::A);
        pAssoc->setWidgetForRole(findWidget(idb), Uml::RoleType::B);
    }

    UMLWidget * pWidgetA = findWidget(pAssoc->widgetIDForRole(Uml::RoleType::A));
    UMLWidget * pWidgetB = findWidget(pAssoc->widgetIDForRole(Uml::RoleType::B));
    //make sure valid widget ids
    if (!pWidgetA || !pWidgetB) {
        return false;
    }

    //make sure there isn't already the same assoc

    for(AssociationWidget *assocwidget : associationList()) {
        if (*pAssoc == *assocwidget)
            // this is nuts. Paste operation wants to know if 'true'
            // for duplicate, but loadFromXMI needs 'false' value
            return (isPasteOperation ? true : false);
    }

    addWidgetCmd(pAssoc);

    FloatingTextWidget *ft[5] = { pAssoc->nameWidget(),
                                  pAssoc->roleWidget(Uml::RoleType::A),
                                  pAssoc->roleWidget(Uml::RoleType::B),
                                  pAssoc->multiplicityWidget(Uml::RoleType::A),
                                  pAssoc->multiplicityWidget(Uml::RoleType::B)
    };
    for (int i = 0; i < 5; i++) {
        FloatingTextWidget *flotxt = ft[i];
        if (flotxt) {
            flotxt->updateGeometry();
            addFloatingTextWidget(flotxt);
        }
    }

    return true;
}

/**
 * Activate the view after a load a new file
 */
void UMLScene::activateAfterLoad(bool bUseLog)
{
    if (m_isActivated) {
        return;
    }
    if (bUseLog) {
        beginPartialWidgetPaste();
    }

    //now activate them all
    activate();

    if (bUseLog) {
        endPartialWidgetPaste();
    }
    m_view->centerOn(0, 0);
    m_isActivated = true;
}

void UMLScene::beginPartialWidgetPaste()
{
    delete m_pIDChangesLog;
    m_pIDChangesLog = 0;

    m_pIDChangesLog = new IDChangeLog();
    m_bPaste = true;
}

void UMLScene::endPartialWidgetPaste()
{
    delete m_pIDChangesLog;
    m_pIDChangesLog = 0;

    m_bPaste = false;
}

/**
 * Removes an AssociationWidget from a diagram
 * Physically deletes the AssociationWidget passed in.
 *
 * @param pAssoc  Pointer to the AssociationWidget.
 */
void UMLScene::removeWidgetCmd(AssociationWidget* pAssoc)
{
    if (!pAssoc)
        return;

    Q_EMIT sigAssociationRemoved(pAssoc);

    pAssoc->cleanup();
    removeItem(pAssoc);
    pAssoc->deleteLater();
    m_doc->setModified();
}

/**
 * Removes an AssociationWidget from the association list
 * and removes the corresponding UMLAssociation from the current UMLDoc.
 */
void UMLScene::removeAssocInViewAndDoc(AssociationWidget* a)
{
    // For umbrello 1.2, UMLAssociations can only be removed in two ways:
    // 1. Right click on the assocwidget in the view and select Delete
    // 2. Go to the Class Properties page, select Associations, right click
    //    on the association and select Delete
    if (!a)
        return;
    if (a->associationType() == Uml::AssociationType::Containment) {
        UMLObject *objToBeMoved = a->widgetForRole(Uml::RoleType::B)->umlObject();
        if (objToBeMoved != 0) {
            UMLListView *lv = UMLApp::app()->listView();
            lv->moveObject(objToBeMoved->id(),
                           Model_Utils::convert_OT_LVT(objToBeMoved),
                           lv->theLogicalView());
            // UMLListView::moveObject() will delete the containment
            // AssociationWidget via UMLScene::updateContainment().
        } else {
            logDebug0("UMLScene::removeAssocInViewAndDoc(containment): objB is NULL");
        }
    } else {
        // Remove assoc in doc.
        m_doc->removeAssociation(a->association());
        // Remove assoc in view.
        removeWidgetCmd(a);
    }
}

/**
 * Removes all the associations related to Widget.
 *
 * @param widget  Pointer to the widget to remove.
 */
void UMLScene::removeAssociations(UMLWidget* widget)
{
    for(AssociationWidget *assocwidget : associationList()) {
        if (assocwidget->containsAsEndpoint(widget)) {
            removeWidgetCmd(assocwidget);
        }
    }
}

/**
 * Sets each association as selected if the widgets it associates are selected
 *
 * @param bSelect  True to select, false for unselect
 */
void UMLScene::selectAssociations(bool bSelect)
{
    for(AssociationWidget *assocwidget : associationList()) {
        UMLWidget *widA = assocwidget->widgetForRole(Uml::RoleType::A);
        UMLWidget *widB = assocwidget->widgetForRole(Uml::RoleType::B);
        if (bSelect &&
                widA && widA->isSelected() &&
                widB && widB->isSelected()) {
            assocwidget->setSelected(true);
        } else {
            assocwidget->setSelected(false);
        }
    }
}

/**
 * Fills Associations with all the associations that includes a widget related to object
 */
void UMLScene::getWidgetAssocs(UMLObject* Obj, AssociationWidgetList & Associations)
{
    if (! Obj)
        return;

    for(AssociationWidget *assocwidget : associationList()) {
        if (assocwidget->widgetForRole(Uml::RoleType::A)->umlObject() == Obj ||
            assocwidget->widgetForRole(Uml::RoleType::B)->umlObject() == Obj)
            Associations.append(assocwidget);
    }

}

/**
 * Removes All the associations of the diagram
 */
void UMLScene::removeAllAssociations()
{
    //Remove All association widgets
    for(AssociationWidget *assocwidget : associationList()) {
        removeWidgetCmd(assocwidget);
    }
}

/**
 * Removes All the widgets of the diagram
 */
void UMLScene::removeAllWidgets()
{
    // Remove widgets.
    for(UMLWidget *temp : widgetList()) {
        uIgnoreZeroPointer(temp);
        // I had to take this condition back in, else umbrello
        // crashes on exit. Still to be analyzed.  --okellogg
        if (!(temp->isTextWidget() &&
              temp->asFloatingTextWidget()->textRole() != TextRole::Floating)) {
            removeWidgetCmd(temp);
        }
    }
}

/**
 * Refreshes containment association, i.e. removes possible old
 * containment and adds new containment association if applicable.
 *
 * @param self  Pointer to the contained object for which
 *   the association to the containing object is
 *   recomputed.
 */
void UMLScene::updateContainment(UMLCanvasObject *self)
{
    if (self == 0)
        return;
    // See if the object has a widget representation in this view.
    // While we're at it, also see if the new parent has a widget here.
    UMLWidget   *selfWidget = nullptr, *newParentWidget = nullptr;
    UMLPackage *newParent = self->umlPackage();
    for(UMLWidget *w : widgetList()) {
        UMLObject *o = w->umlObject();
        if (o == self)
            selfWidget = w;
        else if (newParent != 0 && o == newParent)
            newParentWidget = w;
    }
    if (selfWidget == 0)
        return;
    // Remove possibly obsoleted containment association.
    for(AssociationWidget *a : associationList()) {
        if (a->associationType() != Uml::AssociationType::Containment)
            continue;
        // Container is at role A, containee at B.
        // We only look at association for which we are B.
        UMLWidget *wB = a->widgetForRole(Uml::RoleType::B);
        UMLObject *roleBObj = wB->umlObject();
        if (roleBObj != self)
            continue;
        UMLWidget *wA = a->widgetForRole(Uml::RoleType::A);
        UMLObject *roleAObj = wA->umlObject();
        if (roleAObj == newParent) {
            // Wow, all done. Great!
            return;
        }
        removeWidgetCmd(a);
        // It's okay to break out because there can only be a single
        // containing object.
        break;
    }
    if (newParentWidget == 0)
        return;
    // Create the new containment association.
    AssociationWidget *a = AssociationWidget::create
                             (this, newParentWidget,
                              Uml::AssociationType::Containment, selfWidget);
    addWidgetCmd(a);
}

/**
 * Creates automatically any Associations that the given @ref UMLWidget
 * may have on any diagram.  This method is used when you just add the UMLWidget
 * to a diagram.
 */
void UMLScene::createAutoAssociations(UMLWidget * widget)
{
    if (widget == 0 ||
        (m_Type != Uml::DiagramType::Class &&
         m_Type != Uml::DiagramType::Object &&
         m_Type != Uml::DiagramType::Component &&
         m_Type != Uml::DiagramType::Deployment
         && m_Type != Uml::DiagramType::EntityRelationship))
        return;
    // Recipe:
    // If this widget has an underlying UMLCanvasObject then
    //   for each of the UMLCanvasObject's UMLAssociations
    //     if umlassoc's "other" role has a widget representation on this view then
    //       if the AssocWidget does not already exist then
    //         if the assoc type is permitted in the current diagram type then
    //           create the AssocWidget
    //         end if
    //       end if
    //     end if
    //   end loop
    //   Do createAutoAttributeAssociations()
    //   if this object is capable of containing nested objects then
    //     for each of the object's containedObjects
    //       if the containedObject has a widget representation on this view then
    //         if the containedWidget is not physically located inside this widget
    //           create the containment AssocWidget
    //         end if
    //       end if
    //     end loop
    //   end if
    //   if the UMLCanvasObject has a parentPackage then
    //     if the parentPackage has a widget representation on this view then
    //       create the containment AssocWidget
    //     end if
    //   end if
    // end if
    UMLObject *tmpUmlObj = widget->umlObject();
    if (tmpUmlObj == 0)
        return;
    const UMLCanvasObject *umlObj = tmpUmlObj->asUMLCanvasObject();
    if (umlObj == 0)
        return;
    const UMLAssociationList& umlAssocs = umlObj->getAssociations();

    Uml::ID::Type myID = umlObj->id();
    for(UMLAssociation *assoc : umlAssocs) {
        UMLCanvasObject  *other = nullptr;
        UMLObject *roleAObj = assoc->getObject(Uml::RoleType::A);
        if (roleAObj == 0) {
            logDebug1("UMLScene::createAutoAssociations: roleA object is NULL at UMLAssoc %1",
                              Uml::ID::toString(assoc->id()));
            continue;
        }
        UMLObject *roleBObj = assoc->getObject(Uml::RoleType::B);
        if (roleBObj == 0) {
            logDebug1("UMLScene::createAutoAssociations: roleB object is NULL at UMLAssoc %1",
                              Uml::ID::toString(assoc->id()));
            continue;
        }
        if (roleAObj->id() == myID) {
            other = roleBObj->asUMLCanvasObject();
        } else if (roleBObj->id() == myID) {
            other = roleAObj->asUMLCanvasObject();
        } else {
            logDebug2("UMLScene::createAutoAssociations: Cannot find own object %1 in UMLAssoc %2",
                              Uml::ID::toString(myID), Uml::ID::toString(assoc->id()));
            continue;
        }
        // Now that we have determined the "other" UMLObject, seek it in
        // this view's UMLWidgets.
        if (!other) {
            continue;
        }
        Uml::ID::Type otherID = other->id();

        bool breakFlag = false;
        UMLWidget *pOtherWidget = nullptr;
        for(pOtherWidget: widgetList()) {
            if (pOtherWidget->id() == otherID) {
                breakFlag = true;
                break;
            }
        }
        if (!breakFlag)
            continue;
        // Both objects are represented in this view:
        // Assign widget roles as indicated by the UMLAssociation.
        UMLWidget *widgetA, *widgetB;
        if (myID == roleAObj->id()) {
            widgetA = widget;
            widgetB = pOtherWidget;
        } else {
            widgetA = pOtherWidget;
            widgetB = widget;
        }
        // Check that the assocwidget does not already exist.
        Uml::AssociationType::Enum assocType = assoc->getAssocType();
        AssociationWidget * assocwidget = findAssocWidget(assocType, widgetA, widgetB);
        if (assocwidget) {
            assocwidget->calculateEndingPoints();  // recompute assoc lines
            continue;
        }
        // Check that the assoc is allowed.
        if (!AssocRules::allowAssociation(assocType, widgetA, widgetB)) {
            logDebug1("UMLScene::createAutoAssociations: not transferring assoc of type %1",
                      assocType);
            continue;
        }

        // Create the AssociationWidget.
        assocwidget = AssociationWidget::create(this);
        assocwidget->setWidgetForRole(widgetA, Uml::RoleType::A);
        assocwidget->setWidgetForRole(widgetB, Uml::RoleType::B);
        assocwidget->setAssociationType(assocType);
        assocwidget->setUMLObject(assoc);
        // Call calculateEndingPoints() before setting the FloatingTexts
        // because their positions are computed according to the
        // assocwidget line positions.
        assocwidget->calculateEndingPoints();
        assocwidget->syncToModel();
        assocwidget->setActivated(true);
        if (! addAssociation(assocwidget))
            delete assocwidget;
    }

    createAutoAttributeAssociations(widget);

    if (m_Type == Uml::DiagramType::EntityRelationship) {
        createAutoConstraintAssociations(widget);
    }

    // if this object is capable of containing nested objects then
    UMLObject::ObjectType t = umlObj->baseType();
    if (t == UMLObject::ot_Package || t == UMLObject::ot_Class ||
        t == UMLObject::ot_Interface || t == UMLObject::ot_Component) {
        // for each of the object's containedObjects
        const UMLPackage *umlPkg = umlObj->asUMLPackage();
        UMLObjectList lst = umlPkg->containedObjects();
        for(UMLObject *obj :  lst) {
            uIgnoreZeroPointer(obj);
            // if the containedObject has a widget representation on this view then
            Uml::ID::Type id = obj->id();
            for(UMLWidget  *w : widgetList()) {
                uIgnoreZeroPointer(w);
                if (w->id() != id)
                    continue;
                // if the containedWidget is not physically located inside this widget
                if (w->isLocatedIn(widget))
                    continue;
                // create the containment AssocWidget
                AssociationWidget *a = AssociationWidget::create(this, widget,
                        Uml::AssociationType::Containment, w);
                a->calculateEndingPoints();
                a->setActivated(true);
                if (! addAssociation(a))
                    delete a;
            }
        }
    }
    // if the UMLCanvasObject has a parentPackage then
    UMLPackage *parent = umlObj->umlPackage();
    if (parent == 0)
        return;
    // if the parentPackage has a widget representation on this view then
    Uml::ID::Type pkgID = parent->id();

    bool breakFlag = false;
    UMLWidget *pWidget = nullptr;
    for(pWidget: widgetList()) {
        uIgnoreZeroPointer(pWidget);
        if (pWidget->id() == pkgID) {
            breakFlag = true;
            break;
        }
    }
    if (!breakFlag || widget->isLocatedIn(pWidget))
        return;
    // create the containment AssocWidget
    AssociationWidget *a = AssociationWidget::create(this, pWidget, Uml::AssociationType::Containment, widget);
    if (! addAssociation(a))
        delete a;
}

/**
 * If the m_Type of the given widget is WidgetBase::wt_Class then
 * iterate through the class' attributes and create an
 * association to each attribute type widget that is present
 * on the current diagram.
 */
void UMLScene::createAutoAttributeAssociations(UMLWidget *widget)
{
    if (widget == 0 || m_Type != Uml::DiagramType::Class || !m_Options.classState.showAttribAssocs)
        return;

    // Pseudocode:
    //   if the underlying model object is really a UMLClassifier then
    //     for each of the UMLClassifier's UMLAttributes
    //       if the attribute type has a widget representation on this view then
    //         if the AssocWidget does not already exist then
    //           if the current diagram type permits compositions then
    //             create a composition AssocWidget
    //           end if
    //         end if
    //       end if
    //       if the attribute type is a Datatype then
    //         if the Datatype is a reference (pointer) type then
    //           if the referenced type has a widget representation on this view then
    //             if the AssocWidget does not already exist then
    //               if the current diagram type permits aggregations then
    //                 create an aggregation AssocWidget from the ClassifierWidget to the
    //                                                 widget of the referenced type
    //               end if
    //             end if
    //           end if
    //         end if
    //       end if
    //     end loop
    //   end if
    //
    // Implementation:
    UMLObject *tmpUmlObj = widget->umlObject();
    if (tmpUmlObj == 0)
        return;
    // if the underlying model object is really a UMLClassifier then
    if (tmpUmlObj->isUMLDatatype()) {
        const UMLDatatype *dt = tmpUmlObj->asUMLDatatype();
        while (dt && dt->originType() != 0) {
            tmpUmlObj = dt->originType();
            if (!tmpUmlObj->isUMLDatatype())
                break;
            dt = tmpUmlObj->asUMLDatatype();
        }
    }
    if (tmpUmlObj->baseType() != UMLObject::ot_Class)
        return;
    const UMLClassifier * klass = tmpUmlObj->asUMLClassifier();
    // for each of the UMLClassifier's UMLAttributes
    UMLAttributeList attrList = klass->getAttributeList();
    for(UMLAttribute *attr : attrList) {
        createAutoAttributeAssociation(attr->getType(), attr, widget);
        /*
         * The following code from attachment 19935 of https://bugs.kde.org/140669
         * creates Aggregation/Composition to the template parameters.
         * The current solution uses Dependency instead, see handling of template
         * instantiation at Import_Utils::createUMLObject().
        UMLClassifierList templateList = attr->getTemplateParams();
        for (UMLClassifierListIt it(templateList); it.current(); ++it) {
            createAutoAttributeAssociation(it, attr, widget);
        }
         */
    }
}

/**
 * Create an association with the attribute attr associated with the UMLWidget
 * widget if the UMLClassifier type is present on the current diagram.
 */
void UMLScene::createAutoAttributeAssociation(UMLClassifier *type, UMLAttribute *attr,
                                              UMLWidget *widget /*, UMLClassifier * klass*/)
{
    if (type == 0) {
        // logDebug2("UMLScene::createAutoAttributeAssociation(%1): type is NULL for attribute %2",
        //               klass->getName(), attr->getName());
        return;
    }
    Uml::AssociationType::Enum assocType = Uml::AssociationType::Composition;
    UMLWidget *w = findWidget(type->id());
    // if the attribute type has a widget representation on this view
    if (w) {
        AssociationWidget *a = findAssocWidget(widget, w, attr->name());
        if (a) {
            a->setAssociationType(assocType);
        } else if (AssocRules::allowAssociation(assocType, widget, w)) {
            // Create a composition AssocWidget, or, if the attribute type is
            // stereotyped <<CORBAInterface>>, create a UniAssociation widget.
            if (type->stereotype() == QStringLiteral("CORBAInterface"))
                assocType = Uml::AssociationType::UniAssociation;
            a = AssociationWidget::create(this, widget, assocType, w, attr);
            a->setVisibility(attr->visibility(), Uml::RoleType::B);
            /*
            if (assocType == Uml::AssociationType::Aggregation || assocType == Uml::AssociationType::UniAssociation)
            a->setMulti("0..1", Uml::RoleType::B);
            */
            a->setRoleName(attr->name(), Uml::RoleType::B);
            a->setActivated(true);
            if (! addAssociation(a))
                delete a;
        }
    }
    // if the attribute type is a Datatype then
    if (type->isUMLDatatype()) {
        const UMLDatatype *dt = type->asUMLDatatype();
        // if the Datatype is a reference (pointer) type
        if (dt && dt->isReference()) {
            UMLClassifier *c = dt->originType();
            UMLWidget *w = c ? findWidget(c->id()) : 0;
            // if the referenced type has a widget representation on this view
            if (w) {
                Uml::AssociationType::Enum assocType = Uml::AssociationType::Aggregation;
                AssociationWidget *a = findAssocWidget(widget, w, attr->name());
                if (a) {
                    a->setAssociationType(assocType);
                } else if (AssocRules::allowAssociation(assocType, widget, w)) {
                    // create an aggregation AssocWidget from the ClassifierWidget
                    // to the widget of the referenced type
                    a = AssociationWidget::create (this, widget, assocType, w, attr);
                    a->setVisibility(attr->visibility(), Uml::RoleType::B);
                    //a->setChangeability(true, Uml::RoleType::B);
                    a->setMultiplicity(QStringLiteral("0..1"), Uml::RoleType::B);
                    a->setRoleName(attr->name(), Uml::RoleType::B);
                    a->setActivated(true);
                    if (! addAssociation(a))
                        delete a;
                }
            }
        }
    }
}

void UMLScene::createAutoConstraintAssociations(UMLWidget *widget)
{
    if (widget == 0 || m_Type != Uml::DiagramType::EntityRelationship)
        return;

    // Pseudocode:
    //   if the underlying model object is really a UMLEntity then
    //     for each of the UMLEntity's UMLForeignKeyConstraint's
    //       if the attribute type has a widget representation on this view then
    //         if the AssocWidget does not already exist then
    //           if the current diagram type permits relationships then
    //             create a relationship AssocWidget
    //           end if
    //         end if
    //       end if

    UMLObject *tmpUmlObj = widget->umlObject();
    if (tmpUmlObj == 0)
        return;
    // check if the underlying model object is really a UMLEntity
    const UMLCanvasObject *umlObj = tmpUmlObj->asUMLCanvasObject();
    if (umlObj == 0)
        return;
    // finished checking whether this widget has a UMLCanvas Object

    if (tmpUmlObj->baseType() != UMLObject::ot_Entity)
        return;
    const UMLEntity *entity = tmpUmlObj->asUMLEntity();

    // for each of the UMLEntity's UMLForeignKeyConstraints
    UMLClassifierListItemList constrList = entity->getFilteredList(UMLObject::ot_ForeignKeyConstraint);

    for(UMLClassifierListItem *cli : constrList) {
        UMLEntityConstraint *eConstr = cli->asUMLEntityConstraint();

        UMLForeignKeyConstraint* fkc = eConstr->asUMLForeignKeyConstraint();
        if (fkc == 0) {
            return;
        }

        UMLEntity* refEntity = fkc->getReferencedEntity();
        if (refEntity == 0) {
            return;
        }

        createAutoConstraintAssociation(refEntity, fkc, widget);
    }
}

void UMLScene::createAutoConstraintAssociation(UMLEntity* refEntity, UMLForeignKeyConstraint* fkConstraint, UMLWidget* widget)
{
    if (refEntity == 0) {
        return;
    }

    Uml::AssociationType::Enum assocType = Uml::AssociationType::Relationship;
    UMLWidget *w = findWidget(refEntity->id());
    AssociationWidget *aw = nullptr;

    if (w) {
        aw = findAssocWidget(assocType, w, widget);
        if (aw) {
            if (aw->roleWidget(Uml::RoleType::B))
                aw->roleWidget(Uml::RoleType::B)->setName(fkConstraint->name());
            else
                logError0("UMLScene::createAutoConstraintAssociation could not find role widget B -> cannot rename constraint");
        // if the current diagram type permits relationships
        } else if (AssocRules::allowAssociation(assocType, w, widget)) {
            // for foreign key constraint, we need to create the association type Uml::AssociationType::Relationship.
            // The referenced entity is the "1" part (Role A) and the entity holding the relationship is the "many" part. (Role B)
            AssociationWidget *a = AssociationWidget::create(this, w, assocType, widget);
            a->setUMLObject(fkConstraint);
            //a->setVisibility(attr->getVisibility(), Uml::RoleType::B);
            a->setRoleName(fkConstraint->name(), Uml::RoleType::B);
            a->setActivated(true);
            if (! addAssociation(a))
                delete a;
        }
    }
}

void UMLScene::createAutoAttributeAssociations2(UMLWidget *widget)
{
    for(UMLWidget *w : widgetList()) {
        uIgnoreZeroPointer(w);
        if (w != widget) {
            createAutoAttributeAssociations(w);

            if (widget->umlObject() && widget->umlObject()->baseType() == UMLObject::ot_Entity)
                createAutoConstraintAssociations(w);
        }
    }
}

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
void UMLScene::findMaxBoundingRectangle(const FloatingTextWidget* ft, qreal& px, qreal& py, qreal& qx, qreal& qy)
{
    if (ft == 0 || !ft->isVisible())
        return;

    qreal x = ft->x();
    qreal y = ft->y();
    qreal x1 = x + ft->width() - 1;
    qreal y1 = y + ft->height() - 1;

    if (px == -1 || x < px)
        px = x;
    if (py == -1 || y < py)
        py = y;
    if (qx == -1 || x1 > qx)
        qx = x1;
    if (qy == -1 || y1 > qy)
        qy = y1;
}

/**
 * Returns the PNG picture of the paste operation.
 */
void UMLScene::copyAsImage(QPixmap*& pix)
{
    //get the smallest rect holding the diagram
    QRectF rect = diagramRect();
    QPixmap diagram(rect.width(), rect.height());

    //only draw what is selected
    m_bDrawSelectedOnly = true;
    selectAssociations(true);
    getDiagram(diagram, rect);

    //now get the selection cut
    qreal px = -1, py = -1, qx = -1, qy = -1;

    //first get the smallest rect holding the widgets
    for(UMLWidget *temp : selectedWidgets()) {
        qreal x = temp->x();
        qreal y = temp->y();
        qreal x1 = x + temp->width() - 1;
        qreal y1 = y + temp->height() - 1;
        if (px == -1 || x < px) {
            px = x;
        }
        if (py == -1 || y < py) {
            py = y;
        }
        if (qx == -1 || x1 > qx) {
            qx = x1;
        }
        if (qy == -1 || y1 > qy) {
            qy = y1;
        }
    }

    //also take into account any text lines in assocs or messages

    //get each type of associations
    //This needs to be reimplemented to increase the rectangle
    //if a part of any association is not included
    for(AssociationWidget  *a : associationList()) {
        if (! a->isSelected())
            continue;
        const FloatingTextWidget* multiA = a->multiplicityWidget(Uml::RoleType::A);
        const FloatingTextWidget* multiB = a->multiplicityWidget(Uml::RoleType::B);
        const FloatingTextWidget* roleA = a->roleWidget(Uml::RoleType::A);
        const FloatingTextWidget* roleB = a->roleWidget(Uml::RoleType::B);
        const FloatingTextWidget* changeA = a->changeabilityWidget(Uml::RoleType::A);
        const FloatingTextWidget* changeB = a->changeabilityWidget(Uml::RoleType::B);
        findMaxBoundingRectangle(multiA, px, py, qx, qy);
        findMaxBoundingRectangle(multiB, px, py, qx, qy);
        findMaxBoundingRectangle(roleA, px, py, qx, qy);
        findMaxBoundingRectangle(roleB, px, py, qx, qy);
        findMaxBoundingRectangle(changeA, px, py, qx, qy);
        findMaxBoundingRectangle(changeB, px, py, qx, qy);
    }//end Q_FOREACH

    QRectF imageRect;  //area with respect to diagramRect()
    //i.e. all widgets on the scene.  Was previously with
    //respect to whole scene

    imageRect.setLeft(px - rect.left());
    imageRect.setTop(py - rect.top());
    imageRect.setRight(qx - rect.left());
    imageRect.setBottom(qy - rect.top());

    pix = new QPixmap(imageRect.width(), imageRect.height());
    QPainter output(pix);
    output.drawPixmap(QPoint(0, 0), diagram, imageRect);
    m_bDrawSelectedOnly = false;
}

/**
 * Reset the toolbar.
 */
void UMLScene::resetToolbar()
{
    Q_EMIT sigResetToolBar();
}

void UMLScene::triggerToolbarButton(WorkToolBar::ToolBar_Buttons button)
{
    m_d->triggerToolBarButton(button);
}

/**
 * Event handler for context menu events.
 */
void UMLScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QGraphicsScene::contextMenuEvent(event);
    if (!event->isAccepted()) {
        setPos(event->scenePos());
        UMLScenePopupMenu popup(m_view, this);
        QAction *triggered = popup.exec(event->screenPos());
        slotMenuSelection(triggered);
        event->accept();
    }
}

/**
 * Returns the status on whether in a paste state.
 *
 * @return Returns the status on whether in a paste state.
 */
bool UMLScene::getPaste() const
{
    return m_bPaste;
}

/**
 * Sets the status on whether in a paste state.
 */
void UMLScene::setPaste(bool paste)
{
    m_bPaste = paste;
}

/**
 * When a menu selection has been made on the menu
 * that this view created, this method gets called.
 */
void UMLScene::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch (sel) {
    case ListPopupMenu::mt_Undo:
        UMLApp::app()->undo();
        break;

    case ListPopupMenu::mt_Redo:
        UMLApp::app()->redo();
        break;

    case ListPopupMenu::mt_Clear:
        clearDiagram();
        break;

    case ListPopupMenu::mt_Export_Image:
        m_pImageExporter->exportView();
        break;

    case ListPopupMenu::mt_Apply_Layout:
    case ListPopupMenu::mt_Apply_Layout1:
    case ListPopupMenu::mt_Apply_Layout2:
    case ListPopupMenu::mt_Apply_Layout3:
    case ListPopupMenu::mt_Apply_Layout4:
    case ListPopupMenu::mt_Apply_Layout5:
    case ListPopupMenu::mt_Apply_Layout6:
    case ListPopupMenu::mt_Apply_Layout7:
    case ListPopupMenu::mt_Apply_Layout8:
    case ListPopupMenu::mt_Apply_Layout9:
        {
            QVariant value = ListPopupMenu::dataFromAction(ListPopupMenu::dt_ApplyLayout, action);
            applyLayout(value.toString());
        }
        break;

    case ListPopupMenu::mt_FloatText:
        {
            FloatingTextWidget* ft = new FloatingTextWidget(this);
            ft->showChangeTextDialog();
            //if no text entered delete
            if (!FloatingTextWidget::isTextValid(ft->text())) {
                delete ft;
            } else {
                ft->setID(UniqueID::gen());
                setupNewWidget(ft);
            }
        }
        break;

    case ListPopupMenu::mt_UseCase:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_UseCase);
        break;

    case ListPopupMenu::mt_Actor:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Actor);
        break;

    case ListPopupMenu::mt_Class:
    case ListPopupMenu::mt_Object:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Class);
        break;

    case ListPopupMenu::mt_Package:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Package);
        break;

    case ListPopupMenu::mt_Subsystem:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_SubSystem);
       break;

    case ListPopupMenu::mt_Component:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Component);
        break;

    case ListPopupMenu::mt_Node:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Node);
        break;

    case ListPopupMenu::mt_Artifact:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Artifact);
        break;

    case ListPopupMenu::mt_Interface:
    case ListPopupMenu::mt_InterfaceComponent:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Interface);
        break;

    case ListPopupMenu::mt_Enum:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Enum);
        break;

    case ListPopupMenu::mt_Entity:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Entity);
        break;

    case ListPopupMenu::mt_Category:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Category);
        break;

    case ListPopupMenu::mt_Datatype:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Datatype);
        break;

    case ListPopupMenu::mt_Instance:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(UMLObject::ot_Instance);
        break;

    case ListPopupMenu::mt_Note: {
        m_bCreateObject = true;
        UMLWidget* widget = new NoteWidget(this);
        addItem(widget);
        widget->setPos(pos());
        widget->setSize(100, 40);
        widget->showPropertiesDialog();
        QSizeF size = widget->minimumSize();
        widget->setSize(size);
        break;
    }

    case ListPopupMenu::mt_Cut:
        //FIXME make this work for diagram's right click menu
        if (selectedWidgets().count() &&
            UMLApp::app()->editCutCopy(true)) {
            deleteSelection();
            m_doc->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Copy:
        //FIXME make this work for diagram's right click menu
        selectedWidgets().count() && UMLApp::app()->editCutCopy(true);
        break;

    case ListPopupMenu::mt_Paste:
        m_PastePoint = m_pos;
        m_pos.setX(2000);
        m_pos.setY(2000);
        UMLApp::app()->slotEditPaste();

        m_PastePoint.setX(0);
        m_PastePoint.setY(0);
        break;

    case ListPopupMenu::mt_Initial_State:
        {
            StateWidget* state = new StateWidget(this, StateWidget::Initial);
            setupNewWidget(state);
        }
        break;

    case ListPopupMenu::mt_End_State:
        {
            StateWidget* state = new StateWidget(this, StateWidget::End);
            setupNewWidget(state);
        }
        break;

    case ListPopupMenu::mt_Junction:
        {
            StateWidget* state = new StateWidget(this, StateWidget::Junction);
            setupNewWidget(state);
        }
        break;

    case ListPopupMenu::mt_DeepHistory:
        {
            StateWidget* state = new StateWidget(this, StateWidget::DeepHistory);
            setupNewWidget(state);
        }
        break;

    case ListPopupMenu::mt_ShallowHistory:
        {
            StateWidget* state = new StateWidget(this, StateWidget::ShallowHistory);
            setupNewWidget(state);
        }
        break;

    case ListPopupMenu::mt_Choice:
        {
            StateWidget* state = new StateWidget(this, StateWidget::Choice);
            setupNewWidget(state);
        }
        break;

    case ListPopupMenu::mt_StateFork:
        {
            StateWidget* state = new StateWidget(this, StateWidget::Fork);
            setupNewWidget(state);
        }
        break;

    case ListPopupMenu::mt_StateJoin:
        {
            StateWidget* state = new StateWidget(this, StateWidget::Join);
            setupNewWidget(state);
        }
        break;

    case ListPopupMenu::mt_State:
        {
            QString name = Widget_Utils::defaultWidgetName(WidgetBase::WidgetType::wt_State);
            bool ok = Dialog_Utils::askNewName(WidgetBase::WidgetType::wt_State, name);
            if (ok) {
                StateWidget* state = new StateWidget(this);
                state->setName(name);
                setupNewWidget(state);
            }
        }
        break;

    case ListPopupMenu::mt_CombinedState:
        {
            QString name = Widget_Utils::defaultWidgetName(WidgetBase::WidgetType::wt_State);
            bool ok;
            do {
                if (!Diagram_Utils::isUniqueDiagramName(Uml::DiagramType::State, name))
                    name.append(QStringLiteral("_1"));
                ok = Dialog_Utils::askNewName(WidgetBase::WidgetType::wt_State, name);
            } while(ok && !Diagram_Utils::isUniqueDiagramName(Uml::DiagramType::State, name));
            if (ok) {
                StateWidget* state = new StateWidget(this);
                state->setName(name);
                setupNewWidget(state);
                Uml::CmdCreateDiagram* d = new Uml::CmdCreateDiagram(m_doc, Uml::DiagramType::State, name);
                UMLApp::app()->executeCommand(d);
                state->setDiagramLink(d->view()->umlScene()->ID());
                d->view()->umlScene()->setWidgetLink(state);
                state->setStateType(StateWidget::Combined);
            }
        }
        break;

    case ListPopupMenu::mt_ReturnToClass:
    case ListPopupMenu::mt_ReturnToCombinedState:
        if (widgetLink()) {
            UMLApp::app()->document()->changeCurrentView(widgetLink()->umlScene()->ID());
            widgetLink()->update();
            widgetLink()->umlScene()->update();
            setWidgetLink(nullptr);
        }
        break;

    case ListPopupMenu::mt_Initial_Activity:
        {
            ActivityWidget* activity = new ActivityWidget(this, ActivityWidget::Initial);
            setupNewWidget(activity);
        }
        break;

    case ListPopupMenu::mt_End_Activity:
        {
            ActivityWidget* activity = new ActivityWidget(this, ActivityWidget::End);
            setupNewWidget(activity);
        }
        break;

    case ListPopupMenu::mt_Branch:
        {
            ActivityWidget* activity = new ActivityWidget(this, ActivityWidget::Branch);
            setupNewWidget(activity);
        }
        break;

    case ListPopupMenu::mt_Activity:
        {
            QString name;
            bool ok = Dialog_Utils::askDefaultNewName(WidgetBase::wt_Activity, name);
            if (ok) {
                ActivityWidget* activity = new ActivityWidget(this, ActivityWidget::Normal);
                activity->setName(name);
                setupNewWidget(activity);
            }
        }
        break;

    case ListPopupMenu::mt_SnapToGrid:
        toggleSnapToGrid();
        m_doc->setModified();
        break;

    case ListPopupMenu::mt_SnapComponentSizeToGrid:
        toggleSnapComponentSizeToGrid();
        m_doc->setModified();
        break;

    case ListPopupMenu::mt_ShowSnapGrid:
        toggleShowGrid();
        m_doc->setModified();
        break;

    case ListPopupMenu::mt_ShowDocumentationIndicator:
        setShowDocumentationIndicator(!isShowDocumentationIndicator());
        update();
        break;

    case ListPopupMenu::mt_Properties:
        if (m_view->showPropertiesDialog() == true)
            m_doc->setModified();
        break;

    case ListPopupMenu::mt_Delete:
        m_doc->removeDiagram(ID());
        break;

    case ListPopupMenu::mt_Rename:
        {
            QString newName = name();
            bool ok = Dialog_Utils::askName(i18n("Enter Diagram Name"),
                                            i18n("Enter the new name of the diagram:"),
                                            newName);
            if (ok) {
                setName(newName);
                m_doc->signalDiagramRenamed(activeView());
            }
        }
        break;

    case ListPopupMenu::mt_Import_from_File:
    {
        QPointer<UMLFileDialog> dialog = new UMLFileDialog(QUrl(), QString(), UMLApp::app());
        dialog->exec();
        QUrl url = dialog->selectedUrl();
        if (!url.isEmpty())
            if (!Diagram_Utils::importGraph(url.toLocalFile(), this))
                UMLApp::app()->slotStatusMsg(i18n("Failed to import from file."));
        break;
    }

    case ListPopupMenu::mt_MessageCreation:
        m_d->triggerToolBarButton(WorkToolBar::tbb_Seq_Message_Creation);
        break;

    case ListPopupMenu::mt_MessageDestroy:
        m_d->triggerToolBarButton(WorkToolBar::tbb_Seq_Message_Destroy);
        break;

    case ListPopupMenu::mt_MessageSynchronous:
        m_d->triggerToolBarButton(WorkToolBar::tbb_Seq_Message_Synchronous);
        break;

    case ListPopupMenu::mt_MessageAsynchronous:
        m_d->triggerToolBarButton(WorkToolBar::tbb_Seq_Message_Asynchronous);
        break;

    case ListPopupMenu::mt_MessageFound:
        m_d->triggerToolBarButton(WorkToolBar::tbb_Seq_Message_Found);
        break;

    case ListPopupMenu::mt_MessageLost:
        m_d->triggerToolBarButton(WorkToolBar::tbb_Seq_Message_Lost);
        break;

    default:
        logWarn1("UMLScene::slotMenuSelection: unknown ListPopupMenu::MenuType %1",
                 ListPopupMenu::toString(sel));
        break;
    }
}

/**
 * Connects to the signal that @ref UMLApp emits when a cut operation
 * is successful.
 * If the view or a child started the operation the flag m_bStartedCut will
 * be set and we can carry out any operation that is needed, like deleting the selected
 * widgets for the cut operation.
 */
void UMLScene::slotCutSuccessful()
{
    if (m_bStartedCut) {
        deleteSelection();
        m_bStartedCut = false;
    }
}

/**
 * Called by menu when to show the instance of the view.
 */
void UMLScene::slotShowView()
{
    m_doc->changeCurrentView(ID());
}

/**
 * Returns the offset point at which to place the paste from clipboard.
 * Just add the amount to your co-ords.
 * Only call this straight after the event, the value won't stay valid.
 * Should only be called by Assoc widgets at the moment. no one else needs it.
 */
QPointF UMLScene::getPastePoint()
{
    QPointF point = m_PastePoint;
    point.setX(point.x() - m_pos.x());
    point.setY(point.y() - m_pos.y());
    return point;
}

/**
 * Reset the paste point.
 */
void UMLScene::resetPastePoint()
{
    m_PastePoint = m_pos;
}

/**
 * Called by the view or any of its children when they start a cut
 * operation.
 */
void UMLScene::setStartedCut()
{
    m_bStartedCut = true;
}

/**
 * Returns the font to use
 */
QFont UMLScene::font() const
{
    return m_Options.uiState.font;
}

/**
 * Sets the font for the view and optionally all the widgets on the view.
 */
void UMLScene::setFont(QFont font, bool changeAllWidgets /* = false */)
{
    m_Options.uiState.font = font;
    if (!changeAllWidgets)
        return;
    for(UMLWidget *w : widgetList()) {
        uIgnoreZeroPointer(w);
        w->setFont(font);
    }
}

/**
 * Sets some options for all the @ref ClassifierWidget on the view.
 */
void UMLScene::setClassWidgetOptions(ClassOptionsPage * page)
{
    for(UMLWidget *pWidget : widgetList()) {
        uIgnoreZeroPointer(pWidget);
        WidgetBase::WidgetType wt = pWidget->baseType();
        if (wt == WidgetBase::wt_Class) {
            page->setWidget(pWidget->asClassifierWidget());
            page->apply();
        } else if (wt == WidgetBase::wt_Interface) {
            page->setWidget(pWidget->asInterfaceWidget());
            page->apply();
        }
    }
}

/**
 * Returns the type of the selected widget or widgets.
 *
 * If multiple widgets of different types are selected then WidgetType
 * wt_UMLWidget is returned.
 */
WidgetBase::WidgetType UMLScene::getUniqueSelectionType()
{
    if (selectedWidgets().isEmpty()) {
        return WidgetBase::wt_UMLWidget;
    }

    // Get the first item and its base type
    UMLWidget * pTemp = (UMLWidget *) selectedWidgets().first();
    WidgetBase::WidgetType tmpType = pTemp->baseType();

    // Check all selected items, if they have the same BaseType
    for(pTemp: selectedWidgets()) {
        if (pTemp->baseType() != tmpType) {
            return WidgetBase::wt_UMLWidget;
        }
    }

    return tmpType;
}

/**
 * Asks for confirmation and clears everything on the diagram.
 * Called from menus.
 */
void UMLScene::clearDiagram()
{
    if (Dialog_Utils::askDeleteDiagram()) {
        removeAllWidgets();
    }
}

/**
 * Apply an automatic layout.
 */
void UMLScene::applyLayout(const QString &variant)
{
    logDebug1("UMLScene::applyLayout: %1", variant);
    LayoutGenerator r;
    r.generate(this, variant);
    r.apply(this);
    UMLApp::app()->slotZoomFit();
}

/**
 * Changes snap to grid boolean.
 * Called from menus.
 */
void UMLScene::toggleSnapToGrid()
{
    setSnapToGrid(!snapToGrid());
}

/**
 * Changes snap to grid for component size boolean.
 * Called from menus.
 */
void UMLScene::toggleSnapComponentSizeToGrid()
{
    setSnapComponentSizeToGrid(!snapComponentSizeToGrid());
}

/**
 * Changes show grid boolean.
 * Called from menus.
 */
void UMLScene::toggleShowGrid()
{
    setSnapGridVisible(!isSnapGridVisible());
}

/**
 * Return whether to use snap to grid.
 */
bool UMLScene::snapToGrid() const
{
    return m_bUseSnapToGrid;
}

/**
 *  Sets whether to snap to grid.
 */
void UMLScene::setSnapToGrid(bool bSnap)
{
    m_bUseSnapToGrid = bSnap;
    Q_EMIT sigSnapToGridToggled(snapToGrid());
}

/**
 * Return whether to use snap to grid for component size.
 */
bool UMLScene::snapComponentSizeToGrid() const
{
    return m_bUseSnapComponentSizeToGrid;
}

/**
 * Sets whether to snap to grid for component size.
 */
void UMLScene::setSnapComponentSizeToGrid(bool bSnap)
{
    m_bUseSnapComponentSizeToGrid = bSnap;
    updateComponentSizes();
    Q_EMIT sigSnapComponentSizeToGridToggled(snapComponentSizeToGrid());
}

/**
 * Returns the x grid size.
 */
int UMLScene::snapX() const
{
    return m_layoutGrid->gridSpacingX();
}

/**
 * Returns the y grid size.
 */
int UMLScene::snapY() const
{
    return m_layoutGrid->gridSpacingY();
}

/**
 * Sets the grid size in x and y.
 */
void UMLScene::setSnapSpacing(int x, int y)
{
    m_layoutGrid->setGridSpacing(x, y);
}

/**
 * Returns the input coordinate with possible grid-snap applied.
 */
qreal UMLScene::snappedX(qreal _x)
{
    if (snapToGrid()) {
        int x = (int)_x;
        int gridX = snapX();
        int modX = x % gridX;
        x -= modX;
        if (modX >= gridX / 2)
            x += gridX;
        return x;
    }
    else
        return _x;
}

/**
 * Returns the input coordinate with possible grid-snap applied.
 */
qreal UMLScene::snappedY(qreal _y)
{
    if (snapToGrid()) {
        int y = (int)_y;
        int gridY = snapY();
        int modY = y % gridY;
        y -= modY;
        if (modY >= gridY / 2)
            y += gridY;
        return y;
    }
    else
        return _y;
}

/**
 *  Returns whether to show snap grid or not.
 */
bool UMLScene::isSnapGridVisible() const
{
    return m_layoutGrid->isVisible();
}

/**
 * Sets whether to show snap grid.
 */
void UMLScene::setSnapGridVisible(bool bShow)
{
    m_layoutGrid->setVisible(bShow);
    Q_EMIT sigShowGridToggled(bShow);
}

/**
 *  Returns whether to show documentation indicator.
 */
bool UMLScene::isShowDocumentationIndicator() const
{
    return s_showDocumentationIndicator;
}

/**
 *  sets whether to show documentation indicator.
 */
void UMLScene::setShowDocumentationIndicator(bool bShow)
{
    s_showDocumentationIndicator = bShow;
}

/**
 * Returns whether to show operation signatures.
 */
bool UMLScene::showOpSig() const
{
    return m_Options.classState.showOpSig;
}

/**
 * Sets whether to show operation signatures.
 */
void UMLScene::setShowOpSig(bool bShowOpSig)
{
    m_Options.classState.showOpSig = bShowOpSig;
}

/**
 * Changes the zoom to the currently set level (now loaded from file)
 * Called from UMLApp::slotUpdateViews()
 */
void UMLScene::fileLoaded()
{
    m_view->setZoom(m_view->zoom());
}

/**
 * Updates the size of all components in this view.
 */
void UMLScene::updateComponentSizes()
{
    // update sizes of all components
    for(UMLWidget  *obj : widgetList()) {
        uIgnoreZeroPointer(obj);
        obj->updateGeometry();
    }
}

/**
 * Force the widget font metrics to be updated next time
 * the widgets are drawn.
 * This is necessary because the widget size might depend on the
 * font metrics and the font metrics might change for different
 * QPainter, i.e. font metrics for Display font and Printer font are
 * usually different.
 * Call this when you change the QPainter.
 */
void UMLScene::forceUpdateWidgetFontMetrics(QPainter * painter)
{
    for(UMLWidget  *obj : widgetList()) {
        uIgnoreZeroPointer(obj);
        obj->forceUpdateFontMetrics(painter);
    }
}

/**
 * Overrides standard method from QGraphicsScene drawing the background.
 */
void UMLScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    /* For some reason the incoming rect may contain invalid data.
       Seen on loading the XMI file attached to bug 449393 :
       (gdb) p rect
       $2 = (const QRectF &) @0x7fffffffa3d0: {xp = -4160651296.6437497, yp = -18968990857.816666,
                                               w = 4026436738.6874995, h = 33643115861.033333}
    */
    const bool validX = (rect.x() >= -s_maxCanvasSize && rect.x() <= s_maxCanvasSize);
    const bool validY = (rect.y() >= -s_maxCanvasSize && rect.y() <= s_maxCanvasSize);
    const bool validW = (rect.width()  >= 0.0 && rect.width()  <= s_maxCanvasSize);
    const bool validH = (rect.height() >= 0.0 && rect.height() <= s_maxCanvasSize);
    if (!validX || !validY || !validW || !validH) {
        logError4("UMLScene::drawBackground rejecting event due to invalid data: "
                  "validX=%1, validY=%2, validW=%3, validH=%4", validX, validY, validW, validH);
        return;
    }
    QGraphicsScene::drawBackground(painter, rect);
    m_layoutGrid->paint(painter, rect);
    // debug info
    if (Tracer::instance()->isEnabled(QLatin1String(metaObject()->className()))) {
        painter->setPen(Qt::green);
        painter->drawRect(sceneRect());
        QVector<QLineF> origin;
        origin << QLineF(QPointF(-5,0), QPointF(5,0))
              << QLineF(QPointF(0,-5), QPointF(0,5));
        painter->drawLines(origin);
        painter->setPen(Qt::blue);
        painter->drawRect(itemsBoundingRect());
        QVector<QLineF> lines;
        lines << QLineF(m_pos + QPointF(-5,0), m_pos + QPointF(5,0))
              << QLineF(m_pos + QPointF(0,-5), m_pos + QPointF(0,5));
        painter->setPen(Qt::gray);
        painter->drawLines(lines);
    }
}

/**
 * Creates the "diagram" tag and fills it with the contents of the diagram.
 */
void UMLScene::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("diagram"));
    writer.writeAttribute(QStringLiteral("xmi.id"), Uml::ID::toString(m_nID));
    writer.writeAttribute(QStringLiteral("name"), name());
    writer.writeAttribute(QStringLiteral("type"), QString::number(m_Type));
    writer.writeAttribute(QStringLiteral("documentation"), m_Documentation);
    //option state
    m_Options.saveToXMI(writer);
    //misc
    writer.writeAttribute(QStringLiteral("localid"), Uml::ID::toString(m_nLocalID));
    writer.writeAttribute(QStringLiteral("showgrid"), QString::number(m_layoutGrid->isVisible()));
    writer.writeAttribute(QStringLiteral("snapgrid"), QString::number(m_bUseSnapToGrid));
    writer.writeAttribute(QStringLiteral("snapcsgrid"), QString::number(m_bUseSnapComponentSizeToGrid));
    writer.writeAttribute(QStringLiteral("snapx"), QString::number(m_layoutGrid->gridSpacingX()));
    writer.writeAttribute(QStringLiteral("snapy"), QString::number(m_layoutGrid->gridSpacingY()));
    // FIXME: move to UMLView
    writer.writeAttribute(QStringLiteral("zoom"), QString::number(activeView()->zoom()));
    writer.writeAttribute(QStringLiteral("canvasheight"), QString::number(height()));
    writer.writeAttribute(QStringLiteral("canvaswidth"), QString::number(width()));
    writer.writeAttribute(QStringLiteral("isopen"), QString::number(isOpen()));
    if (isSequenceDiagram() || isCollaborationDiagram())
        writer.writeAttribute(QStringLiteral("autoincrementsequence"), QString::number(autoIncrementSequence()));

    //now save all the widgets
    writer.writeStartElement(QStringLiteral("widgets"));
    for(UMLWidget  *widget : widgetList()) {
        uIgnoreZeroPointer(widget);
        // do not save floating text widgets having a parent widget; they are saved as part of the parent
        if (widget->isTextWidget() && widget->parentItem())
            continue;
        // Having an exception is bad I know, but gotta work with
        // system we are given.
        // We DON'T want to record any text widgets which are belonging
        // to associations as they are recorded later in the "associations"
        // section when each owning association is dumped. -b.t.
        if ((!widget->isTextWidget() &&
             !widget->isFloatingDashLineWidget()) ||
             (widget->asFloatingTextWidget() && widget->asFloatingTextWidget()->link() == 0))
            widget->saveToXMI(writer);
    }
    writer.writeEndElement();            // widgets
    //now save the message widgets
    writer.writeStartElement(QStringLiteral("messages"));
    for(UMLWidget *widget : messageList()) {
        widget->saveToXMI(writer);
    }
    writer.writeEndElement();            // messages
    //now save the associations
    writer.writeStartElement(QStringLiteral("associations"));
    if (associationList().count()) {
        // We guard against (associationList().count() == 0) because
        // this code could be reached as follows:
        //  ^  UMLScene::saveToXMI()
        //  ^  UMLDoc::saveToXMI()
        //  ^  UMLDoc::addToUndoStack()
        //  ^  UMLDoc::setModified()
        //  ^  UMLDoc::createDiagram()
        //  ^  UMLDoc::newDocument()
        //  ^  UMLApp::newDocument()
        //  ^  main()
        //
        AssociationWidget  *assoc = nullptr;
        for(assoc: associationList()) {
            assoc->saveToXMI(writer);
        }
    }
    writer.writeEndElement();            // associations
    writer.writeEndElement();  // diagram
}

/**
 * Loads the "diagram" tag.
 */
bool UMLScene::loadFromXMI(QDomElement & qElement)
{
    QString id = qElement.attribute(QStringLiteral("xmi.id"), QStringLiteral("-1"));
    m_nID = Uml::ID::fromString(id);
    if (m_nID == Uml::ID::None)
        return false;
    setName(qElement.attribute(QStringLiteral("name")));
    QString type = qElement.attribute(QStringLiteral("type"), QStringLiteral("0"));
    m_Documentation = qElement.attribute(QStringLiteral("documentation"));
    QString localid = qElement.attribute(QStringLiteral("localid"), QStringLiteral("0"));
    // option state
    m_Options.loadFromXMI(qElement);
    setBackgroundBrush(m_Options.uiState.backgroundColor);
    setGridDotColor(m_Options.uiState.gridDotColor);
    //misc
    QString showgrid = qElement.attribute(QStringLiteral("showgrid"), QStringLiteral("0"));
    m_layoutGrid->setVisible((bool)showgrid.toInt());

    QString snapgrid = qElement.attribute(QStringLiteral("snapgrid"), QStringLiteral("0"));
    m_bUseSnapToGrid = (bool)snapgrid.toInt();

    QString snapcsgrid = qElement.attribute(QStringLiteral("snapcsgrid"), QStringLiteral("0"));
    m_bUseSnapComponentSizeToGrid = (bool)snapcsgrid.toInt();

    QString snapx = qElement.attribute(QStringLiteral("snapx"), QStringLiteral("10"));
    QString snapy = qElement.attribute(QStringLiteral("snapy"), QStringLiteral("10"));
    m_layoutGrid->setGridSpacing(snapx.toInt(), snapy.toInt());

    QString canvheight = qElement.attribute(QStringLiteral("canvasheight"), QString());
    QString canvwidth  = qElement.attribute(QStringLiteral("canvaswidth"), QString());
    qreal canvasWidth  = 0.0;
    qreal canvasHeight = 0.0;
    if (!canvwidth.isEmpty()) {
        canvasWidth = toDoubleFromAnyLocale(canvwidth);
        if (canvasWidth <= 0.0 || canvasWidth > s_maxCanvasSize) {
            canvasWidth = 0.0;
        }
    }
    if (!canvheight.isEmpty()) {
        canvasHeight = toDoubleFromAnyLocale(canvheight);
        if (canvasHeight <= 0.0 || canvasHeight > s_maxCanvasSize) {
            canvasHeight = 0.0;
        }
    }
    if (!qFuzzyIsNull(canvasWidth) && !qFuzzyIsNull(canvasHeight)) {
        setSceneRect(0, 0, canvasWidth, canvasHeight);
    }

    QString zoom = qElement.attribute(QStringLiteral("zoom"), QStringLiteral("100"));
    activeView()->setZoom(zoom.toInt());

    QString isOpen = qElement.attribute(QStringLiteral("isopen"), QStringLiteral("1"));
    m_isOpen = (bool)isOpen.toInt();

    int nType = type.toInt();
    if (nType == -1 || nType >= 400) {
        // Pre 1.5.5 numeric values
        // Values of "type" were changed in 1.5.5 to merge with Settings::Diagram
        switch (nType) {
        case 400:
            m_Type = Uml::DiagramType::UseCase;
            break;
        case 401:
            m_Type = Uml::DiagramType::Collaboration;
            break;
        case 402:
            m_Type = Uml::DiagramType::Class;
            break;
        case 403:
            m_Type = Uml::DiagramType::Sequence;
            break;
        case 404:
            m_Type = Uml::DiagramType::State;
            break;
        case 405:
            m_Type = Uml::DiagramType::Activity;
            break;
        case 406:
            m_Type = Uml::DiagramType::Component;
            break;
        case 407:
            m_Type = Uml::DiagramType::Deployment;
            break;
        case 408:
            m_Type = Uml::DiagramType::EntityRelationship;
            break;
        case 409:
            m_Type = Uml::DiagramType::Object;
            break;
        default:
            m_Type = Uml::DiagramType::Undefined;
            break;
        }
    } else {
        m_Type = Uml::DiagramType::fromInt(nType);
    }
    m_nLocalID = Uml::ID::fromString(localid);

    if (m_Type == Uml::DiagramType::Sequence ||
        m_Type == Uml::DiagramType::Collaboration) {
        QString autoIncrementSequence = qElement.attribute(QStringLiteral("autoincrementsequence"),
                                                           QStringLiteral("0"));
        m_autoIncrementSequence = (bool)autoIncrementSequence.toInt();
    }

    QDomNode node = qElement.firstChild();
    /*
      https://bugs.kde.org/show_bug.cgi?id=449622
      In order to compensate for QGraphicsScene offsets we make an extra loop in which
      negative or positive X / Y offsets are determined.
      The problem stems from the time when the QGraphicsScene coordinates were derived
      from the coordinates of its widgets (function resizeSceneToItems prior to v2.34).
     */
    m_fixX = m_fixY = 0.0;
    qreal xNegOffset = 0.0;
    qreal yNegOffset = 0.0;
    qreal xPosOffset = 1.0e6;
    qreal yPosOffset = 1.0e6;

    // If an offset fix is applied then add a margin on the fix so that the leftmost
    // or topmost widgets have some separation from the scene border.
    const qreal marginIfFixIsApplied = 50.0;

    // If all widgets have a positive X or Y offset exceeding this value
    // then the fix will be applied.
    const qreal xTriggerValueForPositiveFix = s_defaultCanvasWidth / 2.0;
    const qreal yTriggerValueForPositiveFix = s_defaultCanvasHeight / 2.0;

    while (!node.isNull()) {
        QDomElement element = node.toElement();
        if (element.isNull()) {
            node = node.nextSibling();
            continue;
        }
        if (element.tagName() == QStringLiteral("widgets")) {
            QDomNode wNode = element.firstChild();
            QDomElement widgetElement = wNode.toElement();
            while (!widgetElement.isNull()) {
                QString tag  = widgetElement.tagName();
                if ((tag.endsWith(QStringLiteral("widget")) && tag != QStringLiteral("pinwidget")
                                                           && tag != QStringLiteral("portwidget"))
                                                         || tag == QStringLiteral("floatingtext")) {
                    QString xStr = widgetElement.attribute(QStringLiteral("x"), QStringLiteral("0"));
                    qreal x = toDoubleFromAnyLocale(xStr);
                    if (x < -s_maxCanvasSize || x > s_maxCanvasSize) {
                        QString wName = widgetElement.attribute(QStringLiteral("name"), QString());
                        logWarn3("UMLScene::loadFromXMI(%1) ignoring widget %2 due to invalid X value %3",
                                 name(), wName, xStr);
                        wNode = widgetElement.nextSibling();
                        widgetElement = wNode.toElement();
                        continue;
                    }
                    QString yStr = widgetElement.attribute(QStringLiteral("y"), QStringLiteral("0"));
                    qreal y = toDoubleFromAnyLocale(yStr);
                    if (y < -s_maxCanvasSize || y > s_maxCanvasSize) {
                        QString wName = widgetElement.attribute(QStringLiteral("name"), QString());
                        logWarn3("UMLScene::loadFromXMI(%1) ignoring widget %2 due to invalid Y value %3",
                                 name(), wName, yStr);
                        wNode = widgetElement.nextSibling();
                        widgetElement = wNode.toElement();
                        continue;
                    }
                    if (x < 0.0) {
                        if (x < xNegOffset)
                            xNegOffset = x;
                    } else if (x < xPosOffset) {
                        xPosOffset = x;
                    }
                    if (y < 0.0) {
                        if (y < yNegOffset)
                            yNegOffset = y;
                    } else if (y < yPosOffset) {
                        yPosOffset = y;
                    }
                    // QString hStr = widgetElement.attribute(QStringLiteral("height"), QStringLiteral("0"));
                    // QString wStr = widgetElement.attribute(QStringLiteral("width"), QStringLiteral("0"));
                }
                wNode = widgetElement.nextSibling();
                widgetElement = wNode.toElement();
            }
        }
        node = node.nextSibling();
    }
    if (xNegOffset < 0.0)
        m_fixX = -xNegOffset + marginIfFixIsApplied;
    else if (xPosOffset > xTriggerValueForPositiveFix)
        m_fixX = -xPosOffset + marginIfFixIsApplied;
    if (yNegOffset < 0.0)
        m_fixY = -yNegOffset + marginIfFixIsApplied;
    else if (yPosOffset > yTriggerValueForPositiveFix)
        m_fixY = -yPosOffset + marginIfFixIsApplied;
    if (!qFuzzyIsNull(m_fixX) || !qFuzzyIsNull(m_fixY)) {
        logDebug3("UMLScene::loadFromXMI(%1) : fixX = %2, fixY = %3", name(), m_fixX, m_fixY);
    }

    node = qElement.firstChild();
    bool widgetsLoaded = false, messagesLoaded = false, associationsLoaded = false;
    while (!node.isNull()) {
        QDomElement element = node.toElement();
        if (!element.isNull()) {
            if (element.tagName() == QStringLiteral("widgets"))
                widgetsLoaded = loadWidgetsFromXMI(element);
            else if (element.tagName() == QStringLiteral("messages"))
                messagesLoaded = loadMessagesFromXMI(element);
            else if (element.tagName() == QStringLiteral("associations"))
                associationsLoaded = loadAssociationsFromXMI(element);
        }
        node = node.nextSibling();
    }

    if (!widgetsLoaded) {
        logWarn0("UMLScene::loadFromXMI failed on widgets");
        return false;
    }
    if (!messagesLoaded) {
        logWarn0("UMLScene::loadFromXMI failed on messages");
        return false;
    }
    if (!associationsLoaded) {
        logWarn0("UMLScene::loadFromXMI failed on associations");
        return false;
    }

    if (this->isComponentDiagram()) {
        m_d->addMissingPorts();
        m_d->fixPortPositions();
    }
    m_d->removeDuplicatedFloatingTextInstances();

    /*
      During loadWidgetsFromXMI(), the required QGraphicsScene size was calculated
      by finding the minimum (x, y) and maximum (x+width, y+height) values of the
      widgets loaded.
      These are stored in members m_minX, m_minY and m_maxX, m_maxY respectively.
      This extra step in necessary because the XMI attributes "canvaswidth" and
      "canvasheight" may contain invalid values.  See updateCanvasSizeEstimate().
     */
    if (m_maxX > canvasWidth || m_maxY > canvasHeight) {
        if (m_minX < 0.0 || m_minY < 0.0) {
            logWarn3("UMLScene::loadFromXMI(%1): Setting canvas size with strange values x=%2, y=%3",
                     name(), m_minX, m_minY);
            setSceneRect(m_minX, m_minY, m_maxX, m_maxY);
        } else {
            logDebug5("UMLScene::loadFromXMI(%1) : Setting canvas size with w=%2, h=%3 (minX=%4, minY=%5)",
                      name(), m_maxX, m_maxY, m_minX, m_minY);
            setSceneRect(0.0, 0.0, m_maxX, m_maxY);
        }
    }
    return true;
}

bool UMLScene::loadWidgetsFromXMI(QDomElement & qElement)
{
    UMLWidget *widget = nullptr;
    QDomNode node = qElement.firstChild();
    QDomElement widgetElement = node.toElement();
    while (!widgetElement.isNull()) {
        widget = loadWidgetFromXMI(widgetElement);
        if (widget) {
            addWidgetCmd(widget);
            widget->clipSize();
            // In the interest of best-effort loading, in case of a
            // (widget == 0) we still go on.
            // The individual widget's loadFromXMI method should
            // already have generated an error message to tell the
            // user that something went wrong.
        }
        node = widgetElement.nextSibling();
        widgetElement = node.toElement();
    }

    return true;
}

/**
 * Loads a "widget" element from XMI, used by loadFromXMI() and the clipboard.
 */
UMLWidget* UMLScene::loadWidgetFromXMI(QDomElement& widgetElement)
{
    if (!m_doc) {
        logWarn0("UMLScene::loadWidgetFromXMI: m_doc is NULL");
        return 0L;
    }

    QString tag  = widgetElement.tagName();
    QString idstr  = widgetElement.attribute(QStringLiteral("xmi.id"), QStringLiteral("-1"));
    UMLWidget* widget = Widget_Factory::makeWidgetFromXMI(tag, idstr, this);

    if (widget == 0)
        return 0;
    if (!widget->loadFromXMI(widgetElement)) {
        widget->cleanup();
        delete widget;
        return 0;
    }
    return widget;
}

bool UMLScene::loadMessagesFromXMI(QDomElement & qElement)
{
    MessageWidget  *message = nullptr;
    QDomNode node = qElement.firstChild();
    QDomElement messageElement = node.toElement();
    while (!messageElement.isNull()) {
        QString tag = messageElement.tagName();
        logDebug1("UMLScene::loadMessagesFromXMI: tag %1", tag);
        if (tag == QStringLiteral("messagewidget") ||
            tag == QStringLiteral("UML:MessageWidget")) {   // for bkwd compatibility
            message = new MessageWidget(this, SequenceMessage::Asynchronous,
                                        Uml::ID::Reserved);
            if (!message->loadFromXMI(messageElement)) {
                delete message;
                return false;
            }
            addWidgetCmd(message);
            FloatingTextWidget *ft = message->floatingTextWidget();
            if (!ft && message->sequenceMessageType() != SequenceMessage::Creation)
                logDebug1("UMLScene::loadMessagesFromXMI: floating text is NULL for message %1",
                          Uml::ID::toString(message->id()));
        }
        node = messageElement.nextSibling();
        messageElement = node.toElement();
    }
    return true;
}

bool UMLScene::loadAssociationsFromXMI(QDomElement & qElement)
{
    QDomNode node = qElement.firstChild();
    QDomElement assocElement = node.toElement();
    int countr = 0;
    while (!assocElement.isNull()) {
        QString tag = assocElement.tagName();
        if (tag == QStringLiteral("assocwidget") ||
            tag == QStringLiteral("UML:AssocWidget")) {  // for bkwd compatibility
            countr++;
            AssociationWidget *assoc = AssociationWidget::create(this);
            if (!assoc->loadFromXMI(assocElement)) {
                logError1("UMLScene::loadAssociationsFromXMI could not load association widget %1",
                          Uml::ID::toString(assoc->id()));
                delete assoc;
                /* return false;
                   Returning false here is a little harsh when the
                   rest of the diagram might load okay.
                 */
            } else {
                assoc->clipSize();
                if (!addAssociation(assoc, false)) {
                    logError1("UMLScene::loadAssociationsFromXMI could not addAssociation(%1) to scene",
                              Uml::ID::toString(assoc->id()));
                    delete assoc;
                    //return false; // soften error.. may not be that bad
                }
            }
        }
        node = assocElement.nextSibling();
        assocElement = node.toElement();
    }
    return true;
}

/**
 * Add an object to the application, and update the view.
 */
void UMLScene::addObject(UMLObject *object)
{
    m_bCreateObject = true;
    if (m_doc->addUMLObject(object))
        m_doc->signalUMLObjectCreated(object);  // m_bCreateObject is reset by slotObjectCreated()
    else
        m_bCreateObject = false;
}

bool UMLScene::loadUisDiagramPresentation(QDomElement & qElement)
{
    for (QDomNode node = qElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement elem = node.toElement();
        QString tag = elem.tagName();
        if (! UMLDoc::tagEq(tag, QStringLiteral("Presentation"))) {
            logError1("ignoring unknown UisDiagramPresentation tag %1", tag);
            continue;
        }
        QDomNode n = elem.firstChild();
        QDomElement e = n.toElement();
        QString idStr;
        int x = 0, y = 0, w = 0, h = 0;
        while (!e.isNull()) {
            tag = e.tagName();
            logDebug1("UMLScene::loadUisDiagramPresentation: tag %1", tag);
            if (UMLDoc::tagEq(tag, QStringLiteral("Presentation.geometry"))) {
                QDomNode gnode = e.firstChild();
                QDomElement gelem = gnode.toElement();
                QString csv = gelem.text();
                QStringList dim = csv.split(QLatin1Char(','));
                x = dim[0].toInt();
                y = dim[1].toInt();
                w = dim[2].toInt();
                h = dim[3].toInt();
            } else if (UMLDoc::tagEq(tag, QStringLiteral("Presentation.style"))) {
                // TBD
            } else if (UMLDoc::tagEq(tag, QStringLiteral("Presentation.model"))) {
                QDomNode mnode = e.firstChild();
                QDomElement melem = mnode.toElement();
                idStr = melem.attribute(QStringLiteral("xmi.idref"));
            } else {
                logDebug1("UMLScene::loadUisDiagramPresentation: ignoring tag %1", tag);
            }
            n = n.nextSibling();
            e = n.toElement();
        }
        Uml::ID::Type id = Uml::ID::fromString(idStr);
        UMLObject *o = m_doc->findObjectById(id);
        if (o == 0) {
            logError1("loadUisDiagramPresentation: Cannot find object for id %1", idStr);
        } else {
            UMLObject::ObjectType ot = o->baseType();
            logDebug1("UMLScene::loadUisDiagramPresentation: Create widget for model object of type %1",
                      UMLObject::toString(ot));
            UMLWidget  *widget = nullptr;
            switch (ot) {
            case UMLObject::ot_Class:
                widget = new ClassifierWidget(this, o->asUMLClassifier());
                break;
            case UMLObject::ot_Association: {
                UMLAssociation *umla = o->asUMLAssociation();
                Uml::AssociationType::Enum at = umla->getAssocType();
                UMLObject* objA = umla->getObject(Uml::RoleType::A);
                UMLObject* objB = umla->getObject(Uml::RoleType::B);
                if (objA == 0 || objB == 0) {
                    logError0("loadUisDiagramPresentation(association) intern err 1");
                    return false;
                }
                UMLWidget *wA = findWidget(objA->id());
                UMLWidget *wB = findWidget(objB->id());
                if (wA != 0 && wB != 0) {
                    AssociationWidget *aw =
                        AssociationWidget::create(this, wA, at, wB, umla);
                    aw->syncToModel();
                    addWidgetCmd(aw);
                } else {
                    const QString nullRole = (!wA && !wB ? QStringLiteral("both roles") :
                                              !wA ? QStringLiteral("role A") : QStringLiteral("role B"));
                    logError1("loadUisDiagramPresentation cannot create assocwidget due to null widget at %1",
                              nullRole);
                }
                break;
            }
            case UMLObject::ot_Role: {
                //UMLRole *robj = o->asUMLRole();
                //UMLAssociation *umla = robj->getParentAssociation();
                // @todo properly display role names.
                //       For now, in order to get the role names displayed
                //       simply delete the participating diagram objects
                //       and drag them from the list view to the diagram.
                break;
            }
            default:
                logError1("loadUisDiagramPresentation cannot create widget of type %1", ot);
            }
            if (widget) {
                logDebug4("UMLScene::loadUisDiagramPresentation Widget: x=%1, y=%2, w=%3, h=%4", x, y, w, h);
                widget->setX(x);
                widget->setY(y);
                widget->setSize(w, h);
                addWidgetCmd(widget);
            }
        }
    }
    return true;
}

/**
 * Loads the "UISDiagram" tag of Unisys.IntegratePlus.2 generated files.
 */
bool UMLScene::loadUISDiagram(QDomElement & qElement)
{
    QString idStr = qElement.attribute(QStringLiteral("xmi.id"));
    if (idStr.isEmpty())
        return false;
    m_nID = Uml::ID::fromString(idStr);
    UMLListViewItem  *ulvi = nullptr;
    for (QDomNode node = qElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement elem = node.toElement();
        QString tag = elem.tagName();
        if (tag == QStringLiteral("uisDiagramName")) {
            setName(elem.text());
            if (ulvi)
                ulvi->setText(name());
        } else if (tag == QStringLiteral("uisDiagramStyle")) {
            QString diagramStyle = elem.text();
            if (diagramStyle != QStringLiteral("ClassDiagram")) {
                logError1("loadUISDiagram: style %1 is not yet implemented", diagramStyle);
                continue;
            }
            m_doc->setMainViewID(m_nID);
            m_Type = Uml::DiagramType::Class;
            UMLListView *lv = UMLApp::app()->listView();
            ulvi = new UMLListViewItem(lv->theLogicalView(), name(),
                                       UMLListViewItem::lvt_Class_Diagram, m_nID);
        } else if (tag == QStringLiteral("uisDiagramPresentation")) {
            loadUisDiagramPresentation(elem);
        } else if (tag != QStringLiteral("uisToolName")) {
            logDebug1("UMLScene::loadUISDiagram ignoring tag %1", tag);
        }
    }
    return true;
}

/**
 * Left Alignment
 */
void UMLScene::alignLeft()
{
    UMLWidgetList widgetList = selectedWidgetsExt();
    if (widgetList.isEmpty())
        return;

    qreal smallestX = WidgetList_Utils::getSmallestX(widgetList);

    for(UMLWidget  *widget : widgetList) {
        widget->setX(smallestX);
        widget->adjustAssocs(widget->x(), widget->y());
    }
    //TODO: Push stored cmds to stack.
}

/**
 * Right Alignment
 */
void UMLScene::alignRight()
{
    UMLWidgetList widgetList = selectedWidgetsExt();
    if (widgetList.isEmpty())
        return;
    qreal biggestX = WidgetList_Utils::getBiggestX(widgetList);

    for(UMLWidget  *widget : widgetList) {
        widget->setX(biggestX - widget->width());
        widget->adjustAssocs(widget->x(), widget->y());
    }
    //TODO: Push stored cmds to stack.
}

/**
 * Top Alignment
 */
void UMLScene::alignTop()
{
    UMLWidgetList widgetList = selectedWidgetsExt();
    if (widgetList.isEmpty())
        return;

    qreal smallestY = WidgetList_Utils::getSmallestY(widgetList);

    for(UMLWidget  *widget : widgetList) {
        widget->setY(smallestY);
        widget->adjustAssocs(widget->x(), widget->y());
    }
    //TODO: Push stored cmds to stack.
}

/**
 * Bottom Alignment
 */
void UMLScene::alignBottom()
{
    UMLWidgetList widgetList = selectedWidgetsExt();
    if (widgetList.isEmpty())
        return;
    qreal biggestY = WidgetList_Utils::getBiggestY(widgetList);

    for(UMLWidget  *widget : widgetList) {
        widget->setY(biggestY - widget->height());
        widget->adjustAssocs(widget->x(), widget->y());
    }
    //TODO: Push stored cmds to stack.
}

/**
 * Vertical Middle Alignment
 */
void UMLScene::alignVerticalMiddle()
{
    UMLWidgetList widgetList = selectedWidgetsExt();
    if (widgetList.isEmpty())
        return;

    qreal smallestY = WidgetList_Utils::getSmallestY(widgetList);
    qreal biggestY = WidgetList_Utils::getBiggestY(widgetList);
    qreal middle = int((biggestY - smallestY) / 2) + smallestY;

    for(UMLWidget  *widget : widgetList) {
        widget->setY(middle - widget->height() / 2);
        widget->adjustAssocs(widget->x(), widget->y());
    }

    AssociationWidgetList assocList = selectedAssocs();
    if (!assocList.isEmpty()) {
        for(AssociationWidget  *widget : assocList) {
            widget->setYEntireAssoc(middle);
        }
    }

    //TODO: Push stored cmds to stack.
}

/**
 * Horizontal Middle Alignment
 */
void UMLScene::alignHorizontalMiddle()
{
    UMLWidgetList widgetList = selectedWidgetsExt();
    if (widgetList.isEmpty())
        return;

    qreal smallestX = WidgetList_Utils::getSmallestX(widgetList);
    qreal biggestX = WidgetList_Utils::getBiggestX(widgetList);
    qreal middle = int((biggestX - smallestX) / 2) + smallestX;

    for(UMLWidget  *widget : widgetList) {
        widget->setX(middle - widget->width() / 2);
        widget->adjustAssocs(widget->x(), widget->y());
    }

    AssociationWidgetList assocList = selectedAssocs();
    if (!assocList.isEmpty()) {
        for(AssociationWidget  *widget : assocList) {
            widget->setXEntireAssoc(middle);
        }
    }

    //TODO: Push stored cmds to stack.
}

/**
 * Vertical Distribute Alignment
 */
void UMLScene::alignVerticalDistribute()
{
    UMLWidgetList widgetList = selectedWidgetsExt();
    if (widgetList.isEmpty())
        return;

    qreal smallestY = WidgetList_Utils::getSmallestY(widgetList);
    qreal biggestY = WidgetList_Utils::getBiggestY(widgetList);
    qreal heightsSum = WidgetList_Utils::getHeightsSum(widgetList);
    qreal distance = int(((biggestY - smallestY) - heightsSum) / (widgetList.count() - 1.0) + 0.5);

    std::sort(widgetList.begin(), widgetList.end(), Widget_Utils::hasSmallerY);

    int i = 1;
    UMLWidget *widgetPrev = nullptr;
    for(UMLWidget  *widget : widgetList) {
        if (i == 1) {
            widgetPrev = widget;
        } else {
            widget->setY(widgetPrev->y() + widgetPrev->height() + distance);
            widget->adjustAssocs(widget->x(), widget->y());
            widgetPrev = widget;
        }
        i++;
    }
    //TODO: Push stored cmds to stack.
}

/**
 * Horizontal Distribute Alignment
 */
void UMLScene::alignHorizontalDistribute()
{
    UMLWidgetList widgetList = selectedWidgetsExt();
    if (widgetList.isEmpty())
        return;

    qreal smallestX = WidgetList_Utils::getSmallestX(widgetList);
    qreal biggestX = WidgetList_Utils::getBiggestX(widgetList);
    qreal widthsSum = WidgetList_Utils::getWidthsSum(widgetList);
    qreal distance = int(((biggestX - smallestX) - widthsSum) / (widgetList.count() - 1.0) + 0.5);

    std::sort(widgetList.begin(), widgetList.end(), Widget_Utils::hasSmallerX);

    int i = 1;
    UMLWidget *widgetPrev = nullptr;
    for(UMLWidget  *widget :  widgetList) {
        if (i == 1) {
            widgetPrev = widget;
        } else {
            widget->setX(widgetPrev->x() + widgetPrev->width() + distance);
            widget->adjustAssocs(widget->x(), widget->y());
            widgetPrev = widget;
        }
        i++;
    }
    //TODO: Push stored cmds to stack.
}

/**
 * Overloading operator for debugging output.
 */
QDebug operator<<(QDebug dbg, UMLScene *item)
{
    dbg.nospace() << "UMLScene: " << item->name()
                  << " / type=" << DiagramType::toString(item->type())
                  << " / id=" << Uml::ID::toString(item->ID())
                  << " / isOpen=" << item->isOpen();
    return dbg.space();
}

void UMLScene::setWidgetLink(WidgetBase *w)
{
    m_d->widgetLink = w;
}

WidgetBase *UMLScene::widgetLink()
{
    return m_d->widgetLink;
}

/**
 * Unfortunately the XMI attributes "canvaswidth" and "canvasheight" cannot be
 * relied on, in versions before 2.34 they sometimes contain bogus values.
 * We work around this problem by gathering the minimum and maximum values
 * of the widgets x, x+width, y, y+height into the variables m_minX, m_maxX,
 * m_minY, m_maxY.
 * These values are gathered, and the new scene rectangle is set if required,
 * during loadFromXMI().
 *
 * @param x  If value is less than m_minX then m_minX is set to this value.
 * @param y  If value is less than m_minY then m_minY is set to this value.
 * @param w  If @p x plus this value is greater than m_maxX then m_maxX is set to their sum.
 * @param h  If @p y plus this value is greater than m_maxY then m_maxY is set to their sum.
 */
void UMLScene::updateCanvasSizeEstimate(qreal x, qreal y, qreal w, qreal h)
{
    if (x < m_minX)
        m_minX = x;
    else if (x + w > m_maxX)
        m_maxX = x + w;
    if (y < m_minY)
        m_minY = y;
    else if (y + h > m_maxY)
        m_maxY = y + h;
}

/**
 * Adjust scene rect to cover all contained items
 */
void UMLScene::updateSceneRect()
{
    double b = s_sceneBorder;
    setSceneRect(itemsBoundingRect().adjusted(-b, -b, b, b));
}

/**
 * Compensate for QGraphicsScene offsets, https://bugs.kde.org/show_bug.cgi?id=449622
 */
qreal UMLScene::fixX() const
{
    return m_fixX;
}

/**
 * Compensate for QGraphicsScene offsets, https://bugs.kde.org/show_bug.cgi?id=449622
 */
qreal UMLScene::fixY() const
{
    return m_fixY;
}


