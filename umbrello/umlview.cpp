/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlview.h"

// system includes
#include <climits>
#include <math.h>

// include files for Qt
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/QPrinter>
#include <QtGui/QColor>
#include <QtGui/QMatrix>
#include <QtGui/QHideEvent>
#include <QtGui/QCloseEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QShowEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QMouseEvent>

//kde include files
#include <ktemporaryfile.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <klocale.h>

// application specific includes
#include "assocrules.h"
#include "debug_utils.h"
#include "umlviewimageexporter.h"
#include "listpopupmenu.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "docwindow.h"
#include "umlrole.h"
#include "umlviewcanvas.h"
#include "classoptionspage.h"
#include "umlviewdialog.h"
#include "idchangelog.h"
#include "umldragdata.h"
#include "widget_factory.h"
#include "floatingtextwidget.h"
#include "classifierwidget.h"
#include "classifier.h"
#include "packagewidget.h"
#include "package.h"
#include "folder.h"
#include "componentwidget.h"
#include "nodewidget.h"
#include "artifactwidget.h"
#include "datatypewidget.h"
#include "enumwidget.h"
#include "entitywidget.h"
#include "actorwidget.h"
#include "usecasewidget.h"
#include "notewidget.h"
#include "boxwidget.h"
#include "associationwidget.h"
#include "objectwidget.h"
#include "messagewidget.h"
#include "statewidget.h"
#include "signalwidget.h"
#include "forkjoinwidget.h"
#include "activitywidget.h"
#include "objectnodewidget.h"
#include "pinwidget.h"
#include "seqlinewidget.h"
#include "uniqueid.h"
#include "umllistviewitemlist.h"
#include "umllistviewitem.h"
#include "umllistview.h"
#include "umlobjectlist.h"
#include "association.h"
#include "attribute.h"
#include "model_utils.h"
#include "object_factory.h"
#include "umlwidget.h"
#include "toolbarstatefactory.h"
#include "cmds.h"
#include "entity.h"
#include "foreignkeyconstraint.h"

// control the manual DoubleBuffering of QCanvas
// with a define, so that this memory X11 effect can
// be tested more easily
#define MANUAL_CONTROL_DOUBLE_BUFFERING

// static members
const int UMLView::defaultCanvasSize = 1300;

using namespace Uml;

/**
 * Constructor
 */
UMLView::UMLView(UMLFolder *parentFolder)
  : Q3CanvasView(UMLApp::app()->mainViewWidget()),
    m_nLocalID(Uml::id_None),
    m_nID(Uml::id_None),
    m_Type(Uml::DiagramType::Undefined),
    m_Name(QString()),
    m_Documentation(QString())
{
    // Initialize loaded/saved data
    m_bUseSnapToGrid = false;
    m_bUseSnapComponentSizeToGrid = false;
    m_bShowSnapGrid = false;
    m_isOpen = true;
    m_nSnapX = 10;
    m_nSnapY = 10;
    m_nZoom = 100;
    m_nCanvasWidth = UMLView::defaultCanvasSize;
    m_nCanvasHeight = UMLView::defaultCanvasSize;
    m_nCollaborationId = 0;
    m_scene = reinterpret_cast<UMLScene*>(this);

    // Initialize other data
    //m_AssociationList.setAutoDelete( true );
    //m_WidgetList.setAutoDelete( true );
    //m_MessageList.setAutoDelete( true );

    //Setup up booleans
    m_bChildDisplayedDoc = false;
    m_bPaste = false;
    m_bActivated = false;
    m_bCreateObject = false;
    m_bDrawSelectedOnly = false;
    m_bPopupShowing = false;
    m_bStartedCut = false;
    //clear pointers
    m_PastePoint = QPoint(0, 0);
    m_pIDChangesLog = 0;
    m_pMenu = 0;

    m_pImageExporter = new UMLViewImageExporter(m_scene);

    //setup graphical items
    setCanvas(new UMLViewCanvas(this,m_Options));
    // don't set the quite frequent update rate for each
    // diagram, as that causes also an update of invisible
    // diagrams, which can cost high CPU load for many
    // diagrams.
    // Instead: set the updatePeriod to 20 on Show event,
    //          and switch update back off on Hide event
    canvas()->setUpdatePeriod(-1);
    resizeContents(defaultCanvasSize, defaultCanvasSize);
    canvas()->resize(defaultCanvasSize, defaultCanvasSize);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDragAutoScroll(false);

    viewport()->setMouseTracking(false);

    //setup signals
    connect(this, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu()));
    connect(UMLApp::app(), SIGNAL(sigCutSuccessful()),
            this, SLOT(slotCutSuccessful()));

    // Create the ToolBarState factory. This class is not a singleton, because it
    // needs a pointer to this object.
    m_pToolBarStateFactory = new ToolBarStateFactory();
    m_pToolBarState = m_pToolBarStateFactory->getState(WorkToolBar::tbb_Arrow, m_scene);
    m_doc = UMLApp::app()->document();
    m_pFolder = parentFolder;

    DEBUG_REGISTER(DBG_SRC);
}

/**
 * Destructor
 */
UMLView::~UMLView()
{
    delete m_pImageExporter;

    if (m_pIDChangesLog) {
        delete    m_pIDChangesLog;
        m_pIDChangesLog = 0;
    }

    // before we can delete the QCanvas, all widgets must be explicitly
    // removed
    // otherwise the implicit remove of the contained widgets will cause
    // events which would demand a valid connected QCanvas
    // ==> this causes umbrello to crash for some - larger?? - projects
    // first avoid all events, which would cause some update actions
    // on deletion of each removed widget
    blockSignals(true);
    removeAllWidgets();

    delete m_pToolBarStateFactory;
    m_pToolBarStateFactory = NULL;

    // Qt Doc for QCanvasView::~QCanvasView () states:
    // "Destroys the canvas view. The associated canvas is not deleted."
    // we should do it now
    delete canvas();
}

/**
 * Return the name of the diagram.
 */
QString UMLView::name() const
{
    return m_Name;
}

/**
 * Set the name of the diagram.
 */
void UMLView::setName(const QString &name)
{
    m_Name = name;
}

int UMLView::generateCollaborationId()
{
    return ++m_nCollaborationId;
}

void UMLView::print(QPrinter *pPrinter, QPainter & pPainter)
{
    int height, width;
    //get the size of the page
    pPrinter->setFullPage(true);
    QFontMetrics fm = pPainter.fontMetrics(); // use the painter font metrics, not the screen fm!
    int fontHeight  = fm.lineSpacing();
    // fetch printer margins individual for all four page sides, as at least top and bottom are not the same
    int left, right, top, bottom;
    QRect paper = pPrinter->paperRect();
    QRect page  = pPrinter->pageRect();
    top = page.top() - paper.top();
    left = page.left() - paper.left();
    bottom = paper.bottom() - page.bottom();
    right = paper.right() - page.right();
    DEBUG(DBG_SRC) << "margin: top=" << QString().setNum(top) << " left=" << QString().setNum(left)
                   << " bottom=" << QString().setNum(bottom) << " right=" << QString().setNum(right);
    // give a little extra space at each side
    left += 2;
    right += 2;
    top += 2;
    bottom += 2;

    if (pPrinter->orientation() == QPrinter::Landscape) {
        // we are printing in LANDSCAPE --> swap marginX and marginY
        uint right_old = right;
        // the DiagramRight side is printed at PrintersTop
        right = top;
        // the DiagramTop side is printed at PrintersLeft
        top = left;
        // the DiagramLeft side is printed at PrintersBottom
        left = bottom;
        // the DiagramBottom side is printed at PrintersRight
        bottom = right_old;
    }

    // The printer will probably use a different font with different font metrics,
    // force the widgets to update accordingly on paint
    forceUpdateWidgetFontMetrics(&pPainter);

    width = pPrinter->width() - left - right;
    height = pPrinter->height() - top - bottom;

    //get the smallest rect holding the diagram
    QRect rect = diagramRect();
    //now draw to printer

#if 0
    int offsetX = 0, offsetY = 0, widthX = 0, heightY = 0;
    // respect the margin
    pPainter.translate(marginX, marginY);

    // clip away everything outside of the margin
    pPainter.setClipRect(marginX, marginY,
                         width, metrics.height() - marginY * 2);

    //loop until all of the picture is printed
    int numPagesX = (int)ceil((double)rect.width() / (double)width);
    int numPagesY = (int)ceil((double)rect.height() / (double)height);
    int page = 0;

    // print the canvas to multiple pages
    for (int pageY = 0; pageY < numPagesY; ++pageY) {
        // tile vertically
        offsetY = pageY * height + rect.y();
        heightY = (pageY + 1) * height > rect.height()
                  ? rect.height() - pageY * height
                  : height;
        for (int pageX = 0; pageX < numPagesX; ++pageX) {
            // tile horizontally
            offsetX = pageX * width + rect.x();
            widthX = (pageX + 1) * width > rect.width()
                     ? rect.width() - pageX * width
                     : width;

            // make sure the part of the diagram is painted at the correct
            // place in the printout
            pPainter.translate(-offsetX, -offsetY);
            getDiagram(QRect(offsetX, offsetY, widthX, heightY),
                       pPainter);
            // undo the translation so the coordinates for the painter
            // correspond to the page again
            pPainter.translate(offsetX, offsetY);

            //draw foot note
            QString string = i18n("Diagram: %2 Page %1", page + 1, name());
            QColor textColor(50, 50, 50);
            pPainter.setPen(textColor);
            pPainter.drawLine(0, height + 2, width, height + 2);
            pPainter.drawText(0, height + 4, width, fontHeight, Qt::AlignLeft, string);

            if (pageX + 1 < numPagesX || pageY + 1 < numPagesY) {
                pPrinter->newPage();
                page++;
            }
        }
    }
#else
    // be gentle - as described in Qt-Doc "The Coordinate System"
    pPainter.save();

    int diagramHeight = rect.height();
    // + 4+fontHeight between diagram and footline as space-buffer
    // + 2            between line and foot-text
    // + 1            for foot-line
    // + fontHeight   for foot-text
    // ==============
    // (2*fontHeight) + 7
    int footHeight;
    int footTop;
    int drawHeight;
    bool isFooter = optionState().generalState.footerPrinting;
    if (isFooter) {
        footHeight = (2 * fontHeight) + 7;
        footTop    = rect.y() + diagramHeight  + 4 + fontHeight;
        drawHeight = diagramHeight  + footHeight;
    } else {
        footHeight = 0;
        footTop    = rect.y() + diagramHeight;
        drawHeight = diagramHeight;
    }

    // set window of painter to dimensions of diagram
    // set window to viewport relation so that x:y isn't changed
    double dScaleX = (double)rect.width() / (double)width;
    double dScaleY = (double)drawHeight / (double)height;
    // select the scaling factor so that the larger dimension
    // fits on the printer page -> use the larger scaling factor
    // -> the virtual diagram window has some additional space at the
    // shorter dimension
    double dScaleUse = (dScaleX > dScaleY) ? dScaleX : dScaleY;

    int windowWidth  = (int)ceil(dScaleUse * width);
    int windowHeight = (int)ceil(dScaleUse * height);
#ifdef DEBUG_PRINTING
    DEBUG(DBG_SRC) << "drawHeight: " << drawHeight << ", width: " << rect.width()
                   << "\nPageHeight: " << height << ", PageWidht: " << width
                   << "\nScaleY: " << dScaleY << ", ScaleX: " << dScaleX
                   << "\ndScaleUse: " << dScaleUse
                   << "\nVirtualSize: Width: " << windowWidth << ", Height: " << windowHeight
                   << "\nFoot Top: " << footTop;
#endif
    // set virtual drawing area window - where diagram fits 100% in
    pPainter.setWindow(rect.x(), rect.y(), windowWidth, windowHeight);

    // set viewport - the physical mapping
    // --> Qt's QPainter will map all drawed elements from diagram area ( window )
    //     to printer area ( viewport )
    pPainter.setViewport(left, top, width, height);

    // get Diagram
    getDiagram(QRect(rect.x(), rect.y(), windowWidth, diagramHeight), pPainter);

    if (isFooter) {
        //draw foot note
        QString string = i18n("Diagram: %2 Page %1", 1, name());
        QColor textColor(50, 50, 50);
        pPainter.setPen(textColor);
        pPainter.drawLine(rect.x(), footTop    , windowWidth, footTop);
        pPainter.drawText(rect.x(), footTop + 3, windowWidth, fontHeight, Qt::AlignLeft, string);
    }
    // now restore scaling
    pPainter.restore();

#endif
    // next painting will most probably be to a different device (i.e. the screen)
    forceUpdateWidgetFontMetrics(0);
}

void UMLView::setupNewWidget(UMLWidget *w)
{
    w->setX(m_Pos.x());
    w->setY(m_Pos.y());
    w->setVisible(true);
    w->setActivated();
    w->setFont(getFont());
    w->slotColorChanged(getID());
    w->slotLineWidthChanged(getID());
    resizeCanvasToItems();
    m_WidgetList.append(w);
    m_doc->setModified();

    UMLApp::app()->executeCommand(new CmdCreateWidget(umlScene(), w));
}

void UMLView::contentsMouseReleaseEvent(QMouseEvent* ome)
{
    m_pToolBarState->mouseRelease(static_cast<UMLSceneMouseEvent*>(ome));
}

void UMLView::slotToolBarChanged(int c)
{
    m_pToolBarState->cleanBeforeChange();
    m_pToolBarState = m_pToolBarStateFactory->getState((WorkToolBar::ToolBar_Buttons)c, m_scene);
    m_pToolBarState->init();

    m_bPaste = false;
}

void UMLView::showEvent(QShowEvent* /*se*/)
{

# ifdef MANUAL_CONTROL_DOUBLE_BUFFERING
    //uWarning() << "Show Event for " << name();
    canvas()->setDoubleBuffering(true);
    // as the diagram gets now visible again,
    // the update of the diagram elements shall be
    // at the normal value of 20
    canvas()-> setUpdatePeriod(20);
# endif

    UMLApp* theApp = UMLApp::app();
    WorkToolBar* tb = theApp->workToolBar();
    connect(tb, SIGNAL(sigButtonChanged(int)), this, SLOT(slotToolBarChanged(int)));
    connect(this, SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));
    connect(m_doc, SIGNAL(sigObjectCreated(UMLObject*)),
            this, SLOT(slotObjectCreated(UMLObject*)));
    connect(this, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
            UMLApp::app()->docWindow(), SLOT(slotAssociationRemoved(AssociationWidget*)));
    connect(this, SIGNAL(sigWidgetRemoved(UMLWidget*)),
            UMLApp::app()->docWindow(), SLOT(slotWidgetRemoved(UMLWidget*)));
    resetToolbar();

}

void UMLView::hideEvent(QHideEvent* /*he*/)
{
    UMLApp* theApp = UMLApp::app();
    WorkToolBar* tb = theApp->workToolBar();
    disconnect(tb, SIGNAL(sigButtonChanged(int)), this, SLOT(slotToolBarChanged(int)));
    disconnect(this, SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));
    disconnect(m_doc, SIGNAL(sigObjectCreated(UMLObject*)), this, SLOT(slotObjectCreated(UMLObject*)));
    disconnect(this, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
               UMLApp::app()->docWindow(), SLOT(slotAssociationRemoved(AssociationWidget*)));
    disconnect(this, SIGNAL(sigWidgetRemoved(UMLWidget*)),
               UMLApp::app()->docWindow(), SLOT(slotWidgetRemoved(UMLWidget*)));

# ifdef MANUAL_CONTROL_DOUBLE_BUFFERING
    //uWarning() << "Hide Event for " << name();
    canvas()->setDoubleBuffering(false);
    // a periodic update of all - also invisible - diagrams
    // can cause a very high CPU load if more than 100diagrams
    // are inside a project - and this without any need
    // => switch the update off for hidden diagrams
    canvas()->setUpdatePeriod(-1);
# endif
}

void UMLView::slotObjectCreated(UMLObject* o)
{
    m_bPaste = false;
    //check to see if we want the message
    //may be wanted by someone else e.g. list view

    if (!m_bCreateObject)  {
        return;
    }

    UMLWidget* newWidget = Widget_Factory::createWidget(umlScene(), o);

    if (newWidget == NULL)
        return;

    newWidget->setVisible(true);
    newWidget->setActivated();
    newWidget->setFont(getFont());
    newWidget->slotColorChanged(getID());
    newWidget->slotLineWidthChanged(getID());
    newWidget->updateComponentSize();

    if (m_Type == Uml::DiagramType::Sequence) {
        // Set proper position on the sequence line widget which is
        // attached to the object widget.
        ObjectWidget *ow = dynamic_cast<ObjectWidget*>(newWidget);
        if (ow)
            ow->moveEvent(NULL);
    }
    m_bCreateObject = false;
    m_WidgetList.append(newWidget);

    switch (o->baseType()) {
        case UMLObject::ot_Actor:
        case UMLObject::ot_UseCase:
        case UMLObject::ot_Class:
        case UMLObject::ot_Package:
        case UMLObject::ot_Component:
        case UMLObject::ot_Node:
        case UMLObject::ot_Artifact:
        case UMLObject::ot_Interface:
        case UMLObject::ot_Enum:
        case UMLObject::ot_Entity:
        case UMLObject::ot_Datatype:
        case UMLObject::ot_Category:
            createAutoAssociations(newWidget);
            // We need to invoke createAutoAttributeAssociations()
            // on all other widgets again because the newly created
            // widget might saturate some latent attribute assocs.
            foreach(UMLWidget* w,  m_WidgetList) {
                if (w != newWidget) {
                    createAutoAttributeAssociations(w);

                    if (o->baseType() == UMLObject::ot_Entity)
                        createAutoConstraintAssociations(w);
                }
            }
            break;
        default:
            break;
    }
    resizeCanvasToItems();
}

void UMLView::slotObjectRemoved(UMLObject * o)
{
    m_bPaste = false;
    Uml::IDType id = o->id();

    foreach(UMLWidget* obj, m_WidgetList) {
        if (obj->id() != id)
            continue;
        removeWidget(obj);
        break;
    }
}

/**
 * Override standard method.
 */
void UMLView::dragEnterEvent(QDragEnterEvent *e)
{
    UMLDragData::LvTypeAndID_List tidList;
    if (!UMLDragData::getClip3TypeAndID(e->mimeData(), tidList)) {
        return;
    }
    UMLDragData::LvTypeAndID_It tidIt(tidList);
    UMLDragData::LvTypeAndID * tid;
    if (!tidIt.hasNext()) {
        DEBUG(DBG_SRC) << "UMLDragData::getClip3TypeAndID returned empty list";
        return;
    }
    tid = tidIt.next();
    UMLListViewItem::ListViewType lvtype = tid->type;
    Uml::IDType id = tid->id;

    Uml::DiagramType diagramType = type();

    UMLObject* temp = 0;
    //if dragging diagram - might be a drag-to-note
    if (Model_Utils::typeIsDiagram(lvtype)) {
        e->accept();
        return;
    }
    //can't drag anything onto state/activity diagrams
    if (diagramType == Uml::DiagramType::State || diagramType == Uml::DiagramType::Activity) {
        e->ignore();
        return;
    }
    //make sure can find UMLObject
    if (!(temp = m_doc->findObjectById(id))) {
        DEBUG(DBG_SRC) << "object " << ID2STR(id) << " not found";
        e->ignore();
        return;
    }
    //make sure dragging item onto correct diagram
    // concept - class,seq,coll diagram
    // actor,usecase - usecase diagram
    UMLObject::ObjectType ot = temp->baseType();
    bool bAccept = true;
    switch (diagramType) {
    case Uml::DiagramType::UseCase:
        if ((widgetOnDiagram(id) && ot == UMLObject::ot_Actor) ||
                (ot != UMLObject::ot_Actor && ot != UMLObject::ot_UseCase))
            bAccept = false;
        break;
    case Uml::DiagramType::Class:
        if (widgetOnDiagram(id) ||
                (ot != UMLObject::ot_Class &&
                 ot != UMLObject::ot_Package &&
                 ot != UMLObject::ot_Interface &&
                 ot != UMLObject::ot_Enum &&
                 ot != UMLObject::ot_Datatype)) {
            bAccept = false;
        }
        break;
    case Uml::DiagramType::Sequence:
    case Uml::DiagramType::Collaboration:
        if (ot != UMLObject::ot_Class &&
                ot != UMLObject::ot_Interface &&
                ot != UMLObject::ot_Actor)
            bAccept = false;
        break;
    case Uml::DiagramType::Deployment:
        if (widgetOnDiagram(id))
            bAccept = false;
        else if (ot != UMLObject::ot_Interface &&
                 ot != UMLObject::ot_Package &&
                 ot != UMLObject::ot_Component &&
                 ot != UMLObject::ot_Class &&
                 ot != UMLObject::ot_Node)
            bAccept = false;
        else if (ot == UMLObject::ot_Package &&
                 temp->stereotype() != "subsystem")
            bAccept = false;
        break;
    case Uml::DiagramType::Component:
        if (widgetOnDiagram(id) ||
                (ot != UMLObject::ot_Interface &&
                 ot != UMLObject::ot_Package &&
                 ot != UMLObject::ot_Component &&
                 ot != UMLObject::ot_Artifact &&
                 ot != UMLObject::ot_Class))
            bAccept = false;
        if (ot == UMLObject::ot_Class && !temp->isAbstract())
            bAccept = false;
        break;
    case Uml::DiagramType::EntityRelationship:
        if (ot != UMLObject::ot_Entity && ot != UMLObject::ot_Category)
            bAccept = false;
        break;
    default:
        break;
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
void UMLView::dragMoveEvent(QDragMoveEvent* e)
{
    e->accept();
}

/**
 * Override standard method.
 */
void UMLView::dropEvent(QDropEvent *e)
{
    UMLDragData::LvTypeAndID_List tidList;
    if (!UMLDragData::getClip3TypeAndID(e->mimeData(), tidList)) {
        return;
    }
    UMLDragData::LvTypeAndID_It tidIt(tidList);
    UMLDragData::LvTypeAndID * tid;
    if (!tidIt.hasNext()) {
        DEBUG(DBG_SRC) << "UMLDragData::getClip3TypeAndID returned empty list";
        return;
    }
    tid = tidIt.next();
    UMLListViewItem::ListViewType lvtype = tid->type;
    Uml::IDType id = tid->id;

    if (Model_Utils::typeIsDiagram(lvtype)) {
        bool breakFlag = false;
        UMLWidget* w = 0;
        foreach(w ,  m_WidgetList) {
            if (w->baseType() == WidgetBase::wt_Note && w->onWidget(e->pos())) {
                breakFlag = true;
                break;
            }
        }
        if (breakFlag) {
            NoteWidget *note = static_cast<NoteWidget*>(w);
            note->setDiagramLink(id);
        }
        return;
    }
    UMLObject* o = m_doc->findObjectById(id);
    if (!o) {
        DEBUG(DBG_SRC) << "object id=" << ID2STR(id) << " not found";
        return;
    }
    m_bCreateObject = true;
    m_Pos = (e->pos() * 100) / m_nZoom;

    slotObjectCreated(o);

    m_doc->setModified(true);
}

ObjectWidget * UMLView::onWidgetLine(const QPoint &point)
{
    foreach(UMLWidget* obj, m_WidgetList) {
        ObjectWidget *ow = dynamic_cast<ObjectWidget*>(obj);
        if (ow == NULL)
            continue;
        SeqLineWidget *pLine = ow->getSeqLine();
        if (pLine == NULL) {
            uError() << "SeqLineWidget of " << ow->name()
            << " (id=" << ID2STR(ow->localID()) << ") is NULL";
            continue;
        }
        if (pLine->onWidget(point))
            return ow;
    }
    return 0;
}

ObjectWidget * UMLView::onWidgetDestructionBox(const QPoint &point)
{
    foreach(UMLWidget* obj,  m_WidgetList) {
        ObjectWidget *ow = dynamic_cast<ObjectWidget*>(obj);
        if (ow == NULL)
            continue;
        SeqLineWidget *pLine = ow->getSeqLine();
        if (pLine == NULL) {
            uError() << "SeqLineWidget of " << ow->name()
            << " (id=" << ID2STR(ow->localID()) << ") is NULL";
            continue;
        }
        if (pLine->onDestructionBox(point))
            return ow;
    }
    return 0;
}

UMLWidget *UMLView::widgetAt(const QPoint& p)
{
    int relativeSize = 10000;  // start with an arbitrary large number
    UMLWidget  *retObj = NULL;
    UMLWidgetListIt it(m_WidgetList);
    foreach(UMLWidget* obj,  m_WidgetList) {
        const int s = obj->onWidget(p);
        if (!s)
            continue;
        if (s < relativeSize) {
            relativeSize = s;
            retObj = obj;
        }
    }
    return retObj;
}

void UMLView::checkMessages(ObjectWidget * w)
{
    if (type() != Uml::DiagramType::Sequence)
        return;

    MessageWidgetListIt it(m_MessageList);
    foreach(MessageWidget *obj , m_MessageList) {
        if (! obj->contains(w))
            continue;
        //make sure message doesn't have any associations
        removeAssociations(obj);
        obj->cleanup();
        //make sure not in selected list
        m_SelectedList.removeAll(obj);
        m_MessageList.removeAll(obj);
        obj->deleteLater();
    }
}

bool UMLView::widgetOnDiagram(Uml::IDType id)
{

    foreach(UMLWidget *obj, m_WidgetList) {
        if (id == obj->id())
            return true;
    }

    foreach(UMLWidget *obj , m_MessageList) {
        if (id == obj->id())
            return true;
    }

    return false;
}

void UMLView::contentsMouseMoveEvent(QMouseEvent* ome)
{
    m_pToolBarState->mouseMove(static_cast<UMLSceneMouseEvent*>(ome));
}

// search both our UMLWidget AND MessageWidget lists
UMLWidget * UMLView::findWidget(Uml::IDType id)
{
    foreach(UMLWidget* obj, m_WidgetList) {
        // object widgets are special..the widget id is held by 'localId' attribute (crappy!)
        if (obj->baseType() == WidgetBase::wt_Object) {
            if (static_cast<ObjectWidget *>(obj)->localID() == id)
                return obj;
        } else if (obj->id() == id) {
            return obj;
        }
    }

    foreach(UMLWidget* obj, m_MessageList) {
        if (obj->id() == id)
            return obj;
    }

    return 0;
}

AssociationWidget * UMLView::findAssocWidget(Uml::IDType id)
{
    foreach(AssociationWidget* obj , m_AssociationList) {
        UMLAssociation* umlassoc = obj->getAssociation();
        if (umlassoc && umlassoc->id() == id) {
            return obj;
        }
    }
    return 0;
}

AssociationWidget * UMLView::findAssocWidget(UMLWidget *pWidgetA,
        UMLWidget *pWidgetB, const QString& roleNameB)
{
    foreach(AssociationWidget* assoc, m_AssociationList) {
        const Uml::AssociationType testType = assoc->associationType();
        if (testType != Uml::AssociationType::Association &&
                testType != Uml::AssociationType::UniAssociation &&
                testType != Uml::AssociationType::Composition &&
                testType != Uml::AssociationType::Aggregation &&
                testType != Uml::AssociationType::Relationship)
            continue;
        if (pWidgetA->id() == assoc->getWidgetID(A) &&
                pWidgetB->id() == assoc->getWidgetID(B) &&
                assoc->roleName(Uml::B) == roleNameB)
            return assoc;
    }
    return 0;
}

AssociationWidget * UMLView::findAssocWidget(Uml::AssociationType at,
        UMLWidget *pWidgetA, UMLWidget *pWidgetB)
{
    foreach(AssociationWidget* assoc, m_AssociationList) {
        Uml::AssociationType testType = assoc->associationType();
        if (testType != at)
            continue;
        if (pWidgetA->id() == assoc->getWidgetID(A) &&
                pWidgetB->id() == assoc->getWidgetID(B))
            return assoc;
    }
    return 0;
}

void UMLView::removeWidget(UMLWidget * o)
{
    if (!o)
        return;

    emit sigWidgetRemoved(o);

    removeAssociations(o);

    WidgetBase::WidgetType t = o->baseType();
    if (type() == Uml::DiagramType::Sequence && t == WidgetBase::wt_Object)
        checkMessages(static_cast<ObjectWidget*>(o));

    o->cleanup();
    m_SelectedList.removeAll(o);
    disconnect(this, SIGNAL(sigRemovePopupMenu()), o, SLOT(slotRemovePopupMenu()));
    disconnect(this, SIGNAL(sigClearAllSelected()), o, SLOT(slotClearAllSelected()));
    disconnect(this, SIGNAL(sigColorChanged(Uml::IDType)), o, SLOT(slotColorChanged(Uml::IDType)));
    if (t == WidgetBase::wt_Message) {
        m_MessageList.removeAll(static_cast<MessageWidget*>(o));
    } else
        m_WidgetList.removeAll(o);
    o->deleteLater();
    m_doc->setModified();
}

bool UMLView::useFillColor() const
{
    return m_Options.uiState.useFillColor;
}

void UMLView::setUseFillColor(bool ufc)
{
    m_Options.uiState.useFillColor = ufc;
}

QColor UMLView::fillColor() const
{
    return m_Options.uiState.fillColor;
}

void UMLView::setFillColor(const QColor &color)
{
    m_Options.uiState.fillColor = color;
    emit sigColorChanged(getID());
    canvas()->setAllChanged();
}

QColor UMLView::lineColor() const
{
    return m_Options.uiState.lineColor;
}

void UMLView::setLineColor(const QColor &color)
{
    m_Options.uiState.lineColor = color;
    emit sigColorChanged(getID());
    canvas()->setAllChanged();
}

uint UMLView::lineWidth() const
{
    return m_Options.uiState.lineWidth;
}

void UMLView::setLineWidth(uint width)
{
    m_Options.uiState.lineWidth = width;
    emit sigLineWidthChanged(getID());
    canvas()->setAllChanged();
}

QColor UMLView::textColor() const
{
    return m_Options.uiState.textColor;
}

void UMLView::setTextColor(const QColor &color)
{
    m_Options.uiState.textColor = color;
    emit sigColorChanged(getID());
    canvas()->setAllChanged();
}

void UMLView::contentsMouseDoubleClickEvent(QMouseEvent* ome)
{
    m_pToolBarState->mouseDoubleClick(static_cast<UMLSceneMouseEvent*>(ome));
}

/**
 * Gets the smallest area to print.
 *
 * @return Returns the smallest area to print.
 */
QRect UMLView::diagramRect()
{
    int startx, starty, endx, endy;
    startx = starty = INT_MAX;
    endx = endy = 0;

    foreach(UMLWidget* obj, m_WidgetList) {
        if (! obj->isVisible())
            continue;
        int objEndX = (int)obj->getX() + obj->getWidth();   //:TODO: double
        int objEndY = (int)obj->getY() + obj->getHeight();  //:TODO: double
        int objStartX = (int)obj->getX();  //:TODO: double
        int objStartY = (int)obj->getY();  //:TODO: double
        if (startx >= objStartX)
            startx = objStartX;
        if (starty >= objStartY)
            starty = objStartY;
        if (endx <= objEndX)
            endx = objEndX;
        if (endy <= objEndY)
            endy = objEndY;
    }
    //if seq. diagram, make sure print all of the lines
    if (type() == Uml::DiagramType::Sequence) {
        foreach(UMLWidget* obj, m_WidgetList) {
            ObjectWidget *ow = dynamic_cast<ObjectWidget*>(obj);
            if (ow == NULL)
                continue;
            int y = ow->getEndLineY();
            if (endy < y)
                endy = y;
        }
    }

    // now we need another look at the associations,
    // because they are no UMLWidgets

    QRect rect;

    foreach(AssociationWidget* assoc_obj, m_AssociationList) {
        // get the rectangle around all segments of the assoc
        rect = assoc_obj->getAssocLineRectangle();

        if (startx >= rect.x())
            startx = rect.x();
        if (starty >= rect.y())
            starty = rect.y();
        if (endx <= rect.x() + rect.width())
            endx = rect.x() + rect.width();
        if (endy <= rect.y() + rect.height())
            endy = rect.y() + rect.height();
    }

    /* Margin causes problems of black border around the edge
       // Margin:
       startx -= 24;
       starty -= 20;
       endx += 24;
       endy += 20;
    */

    return QRect(startx, starty,  endx - startx, endy - starty);
}

void UMLView::setSelected(UMLWidget * w, QMouseEvent * me)
{
    Q_UNUSED(me);
    //only add if wasn't in list
    if (!m_SelectedList.removeAll(w))
        m_SelectedList.append(w);
    int count = m_SelectedList.count();
    //only call once - if we select more, no need to keep clearing  window

    // if count == 1, widget will update the doc window with their data when selected
    if (count == 2)
        updateDocumentation(true);  //clear doc window

    /* selection changed, we have to make sure the copy and paste items
     * are correctly enabled/disabled */
    UMLApp::app()->slotCopyChanged();
}

void UMLView::clearSelected()
{
    m_SelectedList.clear();
    emit sigClearAllSelected();
    //m_doc->enableCutCopy(false);
}

//TODO Only used in UMLApp::handleCursorKeyReleaseEvent
void UMLView::moveSelectedBy(int dX, int dY)
{
    // DEBUG(DBG_SRC) << "********** m_SelectedList count=" << m_SelectedList.count();
    foreach(UMLWidget *w, m_SelectedList) {
        w->moveByLocal(dX, dY);
    }
}

void UMLView::selectionUseFillColor(bool useFC)
{
    foreach(UMLWidget* temp, m_SelectedList) {
        temp->setUseFillColor(useFC);
    }
}

void UMLView::selectionSetFont(const QFont &font)
{
    foreach(UMLWidget* temp, m_SelectedList) {
        temp->setFont(font);
    }
}

void UMLView::selectionSetLineColor(const QColor &color)
{
    UMLApp::app()->beginMacro(i18n("Change Line Color"));
    UMLWidget * temp = 0;
    foreach(temp ,  m_SelectedList) {
        temp->setLineColor(color);
        temp->setUsesDiagramLineColor(false);
    }
    AssociationWidgetList assoclist = getSelectedAssocs();
    foreach(AssociationWidget *aw , assoclist) {
        aw->setLineColor(color);
        aw->setUsesDiagramLineColor(false);
    }
    UMLApp::app()->endMacro();
}

void UMLView::selectionSetLineWidth(uint width)
{
    foreach(UMLWidget* temp , m_SelectedList) {
        temp->setLineWidth(width);
        temp->setUsesDiagramLineWidth(false);
    }
    AssociationWidgetList assoclist = getSelectedAssocs();
    foreach(AssociationWidget *aw , assoclist) {
        aw->setLineWidth(width);
        aw->setUsesDiagramLineWidth(false);
    }
}

void UMLView::selectionSetFillColor(const QColor &color)
{
    UMLApp::app()->beginMacro(i18n("Change Fill Color"));

    foreach(UMLWidget* temp ,  m_SelectedList) {
        temp->setFillColor(color);
        temp->setUsesDiagramFillColor(false);
    }
    UMLApp::app()->endMacro();
}

void UMLView::selectionToggleShow(int sel)
{
    // loop through all selected items
    foreach(UMLWidget *temp , m_SelectedList) {
        WidgetBase::WidgetType type = temp->baseType();
        ClassifierWidget *cw = dynamic_cast<ClassifierWidget*>(temp);

        // toggle the show setting sel
        switch (sel) {
            // some setting are only available for class, some for interface and some
            // for both
        case ListPopupMenu::mt_Show_Attributes_Selection:
            if (type == WidgetBase::wt_Class)
                cw->toggleShowAtts();
            break;
        case ListPopupMenu::mt_Show_Operations_Selection:
            if (cw)
                cw->toggleShowOps();
            break;
        case ListPopupMenu::mt_Visibility_Selection:
            if (cw)
                cw->toggleShowVisibility();
            break;
        case ListPopupMenu::mt_DrawAsCircle_Selection:
            if (type == WidgetBase::wt_Interface)
                cw->toggleDrawAsCircle();
            break;
        case ListPopupMenu::mt_Show_Operation_Signature_Selection:
            if (cw)
                cw->toggleShowOpSigs();
            break;
        case ListPopupMenu::mt_Show_Attribute_Signature_Selection:
            if (type == WidgetBase::wt_Class)
                cw->toggleShowAttSigs();
            break;
        case ListPopupMenu::mt_Show_Packages_Selection:
            if (cw)
                cw->toggleShowPackage();
            break;
        case ListPopupMenu::mt_Show_Stereotypes_Selection:
            if (type == WidgetBase::wt_Class)
                cw->toggleShowStereotype();
            break;
        case ListPopupMenu::mt_Show_Public_Only_Selection:
            if (cw)
                cw->toggleShowPublicOnly();
            break;
        default:
            break;
        } // switch (sel)
    }
}

void UMLView::deleteSelection()
{
    /*
       Don't delete text widget that are connect to associations as these will
       be cleaned up by the associations.
    */

    foreach(UMLWidget* temp,  m_SelectedList) {
        if (temp->baseType() == WidgetBase::wt_Text &&
                ((FloatingTextWidget *)temp)->textRole() != Uml::TextRole::Floating) {
            // Porting from Q3PtrList to QList
            //m_SelectedList.remove(); // remove advances the iterator to the next position,
            //m_SelectedList.prev();      // let's allow for statement do the advancing
            m_SelectedList.removeAt(m_SelectedList.indexOf(temp));
            temp->hide();

        } else {
            removeWidget(temp);
        }

    }

    // Delete any selected associations.

    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        if (assocwidget-> getSelected())
            removeAssoc(assocwidget);
        // MARK
    }

    /* we also have to remove selected messages from sequence diagrams */

    /* loop through all messages and check the selection state */
    foreach(MessageWidget* cur_msgWgt, m_MessageList) {
        if (cur_msgWgt->getSelected() == true) {
            removeWidget(cur_msgWgt);  // Remove message - it is selected.
        }
    }

    // sometimes we miss one widget, so call this function again to remove it as
    // well
    if (m_SelectedList.count() != 0)
        deleteSelection();

    //make sure list empty - it should be anyway, just a check.
    m_SelectedList.clear();
}

void UMLView::selectAll()
{
    selectWidgets(0, 0, canvas()->width(), canvas()->height());
}

Uml::IDType UMLView::getLocalID()
{
    m_nLocalID = UniqueID::gen();
    return m_nLocalID;
}

bool UMLView::isSavedInSeparateFile()
{
    if (optionState().generalState.tabdiagrams) {
        // Umbrello currently does not support external folders
        // when tabbed diagrams are enabled.
        return false;
    }
    const QString msgPrefix("UMLView::isSavedInSeparateFile(" + name() + "): ");
    UMLListView *listView = UMLApp::app()->listView();
    UMLListViewItem *lvItem = listView->findItem(m_nID);
    if (lvItem == NULL) {
        uError() << msgPrefix << "listView->findUMLObject(this) returns false";
        return false;
    }
    UMLListViewItem *parentItem = dynamic_cast<UMLListViewItem*>(lvItem->parent());
    if (parentItem == NULL) {
        uError() << msgPrefix << "parent item in listview is not a UMLListViewItem (?)";
        return false;
    }
    const UMLListViewItem::ListViewType lvt = parentItem->type();
    if (! Model_Utils::typeIsFolder(lvt))
        return false;
    UMLFolder *modelFolder = dynamic_cast<UMLFolder*>(parentItem->umlObject());
    if (modelFolder == NULL) {
        uError() << msgPrefix << "parent model object is not a UMLFolder (?)";
        return false;
    }
    QString folderFile = modelFolder->folderFile();
    return !folderFile.isEmpty();
}

void UMLView::contentsMousePressEvent(QMouseEvent* ome)
{
    m_pToolBarState->mousePress(static_cast<UMLSceneMouseEvent*>(ome));
    //TODO should be managed by widgets when are selected. Right now also has some
    //problems, such as clicking on a widget, and clicking to move that widget shows
    //documentation of the diagram instead of keeping the widget documentation.
    //When should diagram documentation be shown? When clicking on an empty
    //space in the diagram with arrow tool?
    if (!m_bChildDisplayedDoc) {
        UMLApp::app()->docWindow()->showDocumentation(this, true);
    }
    m_bChildDisplayedDoc = false;
}

void UMLView::makeSelected(UMLWidget * uw)
{
    if (uw == NULL)
        return;
    uw->setSelected(true);
    m_SelectedList.removeAll(uw);  // make sure not in there
    m_SelectedList.append(uw);
}

void UMLView::selectWidgetsOfAssoc(AssociationWidget * a)
{
    if (!a)
        return;
    a->setSelected(true);
    //select the two widgets
    makeSelected(a->widgetForRole(A));
    makeSelected(a->widgetForRole(B));
    //select all the text
    makeSelected(a->getMultiWidget(A));
    makeSelected(a->getMultiWidget(B));
    makeSelected(a->getRoleWidget(A));
    makeSelected(a->getRoleWidget(B));
    makeSelected(a->getChangeWidget(A));
    makeSelected(a->getChangeWidget(B));
}

void UMLView::selectWidgets(int px, int py, int qx, int qy)
{
    clearSelected();

    QRect rect;
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

    foreach(UMLWidget* temp , m_WidgetList) {
        int x = temp->getX();
        int y = temp->getY();
        int w = temp->getWidth();
        int h = temp->getHeight();
        QRect rect2(x, y, w, h);

        //see if any part of widget is in the rectangle
        if (!rect.intersects(rect2))
            continue;
        //if it is text that is part of an association then select the association
        //and the objects that are connected to it.
        if (temp->baseType() == WidgetBase::wt_Text) {
            FloatingTextWidget *ft = static_cast<FloatingTextWidget*>(temp);
            Uml::TextRole t = ft->textRole();
            LinkWidget *lw = ft->link();
            MessageWidget * mw = dynamic_cast<MessageWidget*>(lw);
            if (mw) {
                makeSelected(mw);
                makeSelected(mw->objectWidget(A));
                makeSelected(mw->objectWidget(B));
            } else if (t != Uml::TextRole::Floating) {
                AssociationWidget * a = dynamic_cast<AssociationWidget*>(lw);
                if (a)
                    selectWidgetsOfAssoc(a);
            }
        } else if (temp->baseType() == WidgetBase::wt_Message) {
            MessageWidget *mw = static_cast<MessageWidget*>(temp);
            makeSelected(mw->objectWidget(A));
            makeSelected(mw->objectWidget(B));
        }
        if (temp->isVisible()) {
            makeSelected(temp);
        }
    }
    selectAssociations(true);

    //now do the same for the messagewidgets

    foreach(MessageWidget *w, m_MessageList) {
        if (w->objectWidget(A)->getSelected() &&
                w->objectWidget(B)->getSelected()) {
            makeSelected(w);
        }//end if
    }//end foreach
}

void UMLView::selectWidgets(UMLWidgetList &widgets)
{
    foreach ( UMLWidget* widget, widgets )
        makeSelected(widget);
}

void  UMLView::getDiagram(const QRect &rect, QPixmap &diagram)
{
    DEBUG(DBG_SRC) << "rect=" << rect << ", pixmap=" << diagram.rect();
    const int width  = rect.x() + rect.width();
    const int height = rect.y() + rect.height();
    QPixmap pixmap(width, height);
    QPainter painter(&pixmap);
    painter.fillRect(0, 0, width, height, Qt::white);
    getDiagram(canvas()->rect(), painter);
    QPainter output(&diagram);
    output.drawPixmap(QPoint(0, 0), pixmap, rect);
}

void  UMLView::getDiagram(const QRect &area, QPainter &painter)
{
    DEBUG(DBG_SRC) << "area=" << area << ", painter=" << painter.window();
    //TODO unselecting and selecting later doesn't work now as the selection is
    //cleared in UMLViewImageExporter. Check if the anything else than the
    //following is needed and, if it works, remove the clearSelected in
    //UMLViewImageExporter and UMLViewImageExporterModel

    foreach(UMLWidget* widget, m_SelectedList) {
        widget->setSelected(false);
    }
    AssociationWidgetList selectedAssociationsList = getSelectedAssocs();

    foreach(AssociationWidget* association, selectedAssociationsList) {
        association->setSelected(false);
    }

    // we don't want to get the grid
    bool showSnapGrid = getShowSnapGrid();
    setShowSnapGrid(false);

    canvas()->drawArea(area, &painter);

    setShowSnapGrid(showSnapGrid);

    canvas()->setAllChanged();
    //select again
    foreach(UMLWidget* widget, m_SelectedList) {
        widget->setSelected(true);
    }
    foreach(AssociationWidget* association, selectedAssociationsList) {
        association->setSelected(true);
    }
}

UMLViewImageExporter* UMLView::getImageExporter()
{
    return m_pImageExporter;
}

void UMLView::slotActivate()
{
    m_doc->changeCurrentView(getID());
}

UMLObjectList UMLView::umlObjects()
{
    UMLObjectList list;
    foreach(UMLWidget* w,  m_WidgetList) {

        switch (w->baseType()) { //use switch for easy future expansion
        case WidgetBase::wt_Actor:
        case WidgetBase::wt_Class:
        case WidgetBase::wt_Interface:
        case WidgetBase::wt_Package:
        case WidgetBase::wt_Component:
        case WidgetBase::wt_Node:
        case WidgetBase::wt_Artifact:
        case WidgetBase::wt_UseCase:
        case WidgetBase::wt_Object:
            list.append(w->umlObject());
            break;
        default:
            break;
        }
    }
    return list;
}

void UMLView::activate()
{
    //Activate Regular widgets then activate  messages
    foreach(UMLWidget* obj, m_WidgetList) {
        //If this UMLWidget is already activated or is a MessageWidget then skip it
        if (obj->isActivated() || obj->baseType() == WidgetBase::wt_Message)
            continue;

        if (obj->activate()) {
            obj->setVisible(true);
        } else {
            m_WidgetList.removeAll(obj);
            delete obj;
        }
    }//end foreach

    //Activate Message widgets
    foreach(UMLWidget* obj, m_MessageList) {
        //If this MessageWidget is already activated then skip it
        if (obj->isActivated())
            continue;

        obj->activate(m_doc->changeLog());
        obj->setVisible(true);

    }//end foreach

    // Activate all association widgets

    foreach(AssociationWidget* aw, m_AssociationList) {
        if (aw->activate()) {
            if (m_PastePoint.x() != 0) {
                int x = m_PastePoint.x() - m_Pos.x();
                int y = m_PastePoint.y() - m_Pos.y();
                aw->moveEntireAssoc(x, y);
            }
        } else {
            m_AssociationList.removeAll(aw);
            delete  aw;
        }
    }
}

int UMLView::getSelectCount(bool filterText) const
{
    if (!filterText)
        return m_SelectedList.count();
    int counter = 0;
    const UMLWidget * temp = 0;
    foreach(temp, m_SelectedList) {
        if (temp->baseType() == WidgetBase::wt_Text) {
            const FloatingTextWidget *ft = static_cast<const FloatingTextWidget*>(temp);
            if (ft->textRole() == Uml::TextRole::Floating)
                counter++;
        } else {
            counter++;
        }
    }
    return counter;
}


bool UMLView::getSelectedWidgets(UMLWidgetList &WidgetList, bool filterText /*= true*/)
{
    foreach(UMLWidget* temp, m_SelectedList) {
        if (filterText && temp->baseType() == WidgetBase::wt_Text) {
            const FloatingTextWidget *ft = static_cast<const FloatingTextWidget*>(temp);
            if (ft->textRole() == Uml::TextRole::Floating)
                WidgetList.append(temp);
        } else {
            WidgetList.append(temp);
        }
    }//end for
    return true;
}

AssociationWidgetList UMLView::getSelectedAssocs()
{
    AssociationWidgetList assocWidgetList;

    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        if (assocwidget->getSelected())
            assocWidgetList.append(assocwidget);
    }
    return assocWidgetList;
}

bool UMLView::addWidget(UMLWidget * pWidget, bool isPasteOperation)
{
    if (!pWidget) {
        return false;
    }
    WidgetBase::WidgetType type = pWidget->baseType();
    if (isPasteOperation) {
        if (type == WidgetBase::wt_Message)
            m_MessageList.append(static_cast<MessageWidget*>(pWidget));
        else
            m_WidgetList.append(pWidget);
        return true;
    }
    if (!isPasteOperation && findWidget(pWidget->id())) {
        uError() << "Not adding (id=" << ID2STR(pWidget->id())
            << "/type=" << type << "/name=" << pWidget->name()
            << ") because it is already there";
        return false;
    }
    IDChangeLog * log = m_doc->changeLog();
    if (isPasteOperation && (!log || !m_pIDChangesLog)) {
        uError() << " Cant addWidget to view in paste op because a log is not open";
        return false;
    }
    int wX = pWidget->getX();
    int wY = pWidget->getY();
    bool xIsOutOfRange = (wX <= 0 || wX >= FloatingTextWidget::restrictPositionMax);
    bool yIsOutOfRange = (wY <= 0 || wY >= FloatingTextWidget::restrictPositionMax);
    if (xIsOutOfRange || yIsOutOfRange) {
        QString name = pWidget->name();
        if (name.isEmpty()) {
            FloatingTextWidget *ft = dynamic_cast<FloatingTextWidget*>(pWidget);
            if (ft)
                name = ft->displayText();
        }
        DEBUG(DBG_SRC) << name << " type=" << pWidget->baseType() << "): position ("
                       << wX << "," << wY << ") is out of range";
        if (xIsOutOfRange) {
            pWidget->setX(0);
            wX = 0;
        }
        if (yIsOutOfRange) {
            pWidget->setY(0);
            wY = 0;
        }
    }
    if (wX < m_Pos.x())
        m_Pos.setX(wX);
    if (wY < m_Pos.y())
        m_Pos.setY(wY);

    //see if we need a new id to match object
    switch (type) {

    case WidgetBase::wt_Class:
    case WidgetBase::wt_Package:
    case WidgetBase::wt_Component:
    case WidgetBase::wt_Node:
    case WidgetBase::wt_Artifact:
    case WidgetBase::wt_Interface:
    case WidgetBase::wt_Enum:
    case WidgetBase::wt_Entity:
    case WidgetBase::wt_Datatype:
    case WidgetBase::wt_Actor:
    case WidgetBase::wt_UseCase:
    case WidgetBase::wt_Category: {
        Uml::IDType id = pWidget->id();
        Uml::IDType newID = log->findNewID(id);
        if (newID == Uml::id_None) {   // happens after a cut
            if (id == Uml::id_None)
                return false;
            newID = id; //don't stop paste
        } else
            pWidget->setID(newID);
        UMLObject * pObject = m_doc->findObjectById(newID);
        if (!pObject) {
            DEBUG(DBG_SRC) << "addWidget: Can not find UMLObject for id " << ID2STR(newID);
            return false;
        }
        pWidget->setUMLObject(pObject);
        //make sure it doesn't already exist.
        if (findWidget(newID)) {
            DEBUG(DBG_SRC) << "Not adding (id=" << ID2STR(pWidget->id())
                           << "/type=" << pWidget->baseType()
                           << "/name=" << pWidget->name()
                           << ") because it is already there";
            delete pWidget; // Not nice but if _we_ don't do it nobody else will
            return true;//don't stop paste just because widget found.
        }
        m_WidgetList.append(pWidget);
    }
    break;

    case WidgetBase::wt_Message:
    case WidgetBase::wt_Note:
    case WidgetBase::wt_Box:
    case WidgetBase::wt_Text:
    case WidgetBase::wt_State:
    case WidgetBase::wt_Activity:
    case WidgetBase::wt_ObjectNode: {
        Uml::IDType newID = m_doc->assignNewID(pWidget->id());
        pWidget->setID(newID);
        if (type != WidgetBase::wt_Message) {
            m_WidgetList.append(pWidget);
            return true;
        }
        // CHECK
        // Handling of WidgetBase::wt_Message:
        MessageWidget *pMessage = static_cast<MessageWidget *>(pWidget);
        if (pMessage == NULL) {
            DEBUG(DBG_SRC) << "pMessage is NULL";
            return false;
        }
        ObjectWidget *objWidgetA = pMessage->objectWidget(A);
        ObjectWidget *objWidgetB = pMessage->objectWidget(B);
        Uml::IDType waID = objWidgetA->localID();
        Uml::IDType wbID = objWidgetB->localID();
        Uml::IDType newWAID = m_pIDChangesLog->findNewID(waID);
        Uml::IDType newWBID = m_pIDChangesLog->findNewID(wbID);
        if (newWAID == Uml::id_None || newWBID == Uml::id_None) {
            DEBUG(DBG_SRC) << "Error with ids : " << ID2STR(newWAID)
                           << " " << ID2STR(newWBID);
            return false;
        }
        // Assumption here is that the A/B objectwidgets and the textwidget
        // are pristine in the sense that we may freely change their local IDs.
        objWidgetA->setLocalID(newWAID);
        objWidgetB->setLocalID(newWBID);
        FloatingTextWidget *ft = pMessage->floatingTextWidget();
        if (ft == NULL) {
            DEBUG(DBG_SRC) << "FloatingTextWidget of Message is NULL";
        }
        else if (ft->id() == Uml::id_None) {
            ft->setID(UniqueID::gen());
        }
        else {
            Uml::IDType newTextID = m_doc->assignNewID(ft->id());
            ft->setID(newTextID);
        }
        m_MessageList.append(pMessage);
    }
    break;

    case WidgetBase::wt_Object: {
        ObjectWidget* pObjectWidget = static_cast<ObjectWidget*>(pWidget);
        if (pObjectWidget == NULL) {
            DEBUG(DBG_SRC) << "pObjectWidget is NULL";
            return false;
        }
        Uml::IDType nNewLocalID = getLocalID();
        Uml::IDType nOldLocalID = pObjectWidget->localID();
        m_pIDChangesLog->addIDChange(nOldLocalID, nNewLocalID);
        pObjectWidget->setLocalID(nNewLocalID);
        UMLObject *pObject = m_doc->findObjectById(pWidget->id());
        if (!pObject) {
            DEBUG(DBG_SRC) << "Cannot find UMLObject";
            return false;
        }
        pWidget->setUMLObject(pObject);
        m_WidgetList.append(pWidget);
    }
    break;

    case WidgetBase::wt_Precondition: {
        ObjectWidget* pObjectWidget = static_cast<ObjectWidget*>(pWidget);
        if (pObjectWidget == NULL) {
            DEBUG(DBG_SRC) << "pObjectWidget is NULL";
            return false;
        }
        Uml::IDType newID = log->findNewID(pWidget->id());
        if (newID == Uml::id_None) {
            return false;
        }
        pObjectWidget->setID(newID);
        Uml::IDType nNewLocalID = getLocalID();
        Uml::IDType nOldLocalID = pObjectWidget->localID();
        m_pIDChangesLog->addIDChange(nOldLocalID, nNewLocalID);
        pObjectWidget->setLocalID(nNewLocalID);
        UMLObject *pObject = m_doc->findObjectById(newID);
        if (!pObject) {
            DEBUG(DBG_SRC) << "Cannot find UMLObject";
            return false;
        }
        pWidget->setUMLObject(pObject);
        m_WidgetList.append(pWidget);
    }
    break;

    case WidgetBase::wt_Pin:
    case WidgetBase::wt_CombinedFragment:
    case WidgetBase::wt_Signal: {
        ObjectWidget* pObjectWidget = static_cast<ObjectWidget*>(pWidget);
        if (pObjectWidget == NULL) {
            DEBUG(DBG_SRC) << "pObjectWidget is NULL";
            return false;
        }
        Uml::IDType newID = log->findNewID(pWidget->id());
        if (newID == Uml::id_None) {
            return false;
        }
        pObjectWidget->setID(newID);
        Uml::IDType nNewLocalID = getLocalID();
        Uml::IDType nOldLocalID = pObjectWidget->localID();
        m_pIDChangesLog->addIDChange(nOldLocalID, nNewLocalID);
        pObjectWidget->setLocalID(nNewLocalID);
        UMLObject *pObject = m_doc->findObjectById(newID);
        if (!pObject) {
            DEBUG(DBG_SRC) << "Cannot find UMLObject";
            return false;
        }
        pWidget->setUMLObject(pObject);
        m_WidgetList.append(pWidget);
    }
    break;

    default:
        DEBUG(DBG_SRC) << "Trying to add an invalid widget type";
        return false;
        break;
    }

    return true;
}

// Add the association, and its child widgets to this view
bool UMLView::addAssociation(AssociationWidget* pAssoc, bool isPasteOperation)
{

    if (!pAssoc) {
        return false;
    }
    const Uml::AssociationType assocType = pAssoc->associationType();

    if (isPasteOperation) {
        IDChangeLog * log = m_doc->changeLog();

        if (!log)
            return false;

        Uml::IDType ida = Uml::id_None, idb = Uml::id_None;
        if (type() == Uml::DiagramType::Collaboration || type() == Uml::DiagramType::Sequence) {
            //check local log first
            ida = m_pIDChangesLog->findNewID(pAssoc->getWidgetID(A));
            idb = m_pIDChangesLog->findNewID(pAssoc->getWidgetID(B));
            //if either is still not found and assoc type is anchor
            //we are probably linking to a notewidet - else an error
            if (ida == Uml::id_None && assocType == Uml::AssociationType::Anchor)
                ida = log->findNewID(pAssoc->getWidgetID(A));
            if (idb == Uml::id_None && assocType == Uml::AssociationType::Anchor)
                idb = log->findNewID(pAssoc->getWidgetID(B));
        } else {
            Uml::IDType oldIdA = pAssoc->getWidgetID(A);
            Uml::IDType oldIdB = pAssoc->getWidgetID(B);
            ida = log->findNewID(oldIdA);
            if (ida == Uml::id_None) {  // happens after a cut
                if (oldIdA == Uml::id_None)
                    return false;
                ida = oldIdA;
            }
            idb = log->findNewID(oldIdB);
            if (idb == Uml::id_None) {  // happens after a cut
                if (oldIdB == Uml::id_None)
                    return false;
                idb = oldIdB;
            }
        }
        if (ida == Uml::id_None || idb == Uml::id_None) {
            return false;
        }
        // cant do this anymore.. may cause problem for pasting
        //      pAssoc->setWidgetID(ida, A);
        //      pAssoc->setWidgetID(idb, B);
        pAssoc->setWidget(findWidget(ida), A);
        pAssoc->setWidget(findWidget(idb), B);
    }

    UMLWidget * pWidgetA = findWidget(pAssoc->getWidgetID(A));
    UMLWidget * pWidgetB = findWidget(pAssoc->getWidgetID(B));
    //make sure valid widget ids
    if (!pWidgetA || !pWidgetB) {
        return false;
    }

    //make sure valid
    if (!isPasteOperation && !m_doc->loading() &&
            !AssocRules::allowAssociation(assocType, pWidgetA, pWidgetB, false)) {
        uWarning() << "allowAssociation returns false " << "for AssocType " << assocType;
        return false;
    }

    //make sure there isn't already the same assoc

    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        if (*pAssoc == *assocwidget)
            // this is nuts. Paste operation wants to know if 'true'
            // for duplicate, but loadFromXMI needs 'false' value
            return (isPasteOperation ? true : false);
    }

    m_AssociationList.append(pAssoc);

    FloatingTextWidget *ft[5] = { pAssoc->getNameWidget(),
                                  pAssoc->getRoleWidget(A),
                                  pAssoc->getRoleWidget(B),
                                  pAssoc->getMultiWidget(A),
                                  pAssoc->getMultiWidget(B)
                                };
    for (int i = 0; i < 5; i++) {
        FloatingTextWidget *flotxt = ft[i];
        if (flotxt) {
            flotxt->updateComponentSize();
            addWidget(flotxt);
        }
    }

    return true;
}

void UMLView::activateAfterLoad(bool bUseLog)
{
    if (m_bActivated)
        return;
    if (bUseLog) {
        beginPartialWidgetPaste();
    }

    //now activate them all
    activate();

    if (bUseLog) {
        endPartialWidgetPaste();
    }
    resizeCanvasToItems();
    setZoom(zoom());
    m_bActivated = true;
}

void UMLView::beginPartialWidgetPaste()
{
    delete m_pIDChangesLog;
    m_pIDChangesLog = 0;

    m_pIDChangesLog = new IDChangeLog();
    m_bPaste = true;
}

void UMLView::endPartialWidgetPaste()
{
    delete    m_pIDChangesLog;
    m_pIDChangesLog = 0;

    m_bPaste = false;
}

void UMLView::removeAssoc(AssociationWidget* pAssoc)
{
    if (!pAssoc)
        return;

    emit sigAssociationRemoved(pAssoc);

    pAssoc->cleanup();
    m_AssociationList.removeAll(pAssoc);
    pAssoc->deleteLater();
    m_doc->setModified();
}

void UMLView::removeAssocInViewAndDoc(AssociationWidget* a)
{
    // For umbrello 1.2, UMLAssociations can only be removed in two ways:
    // 1. Right click on the assocwidget in the view and select Delete
    // 2. Go to the Class Properties page, select Associations, right click
    //    on the association and select Delete
    if (!a)
        return;
    if (a->associationType() == Uml::AssociationType::Containment) {
        UMLObject *objToBeMoved = a->widgetForRole(B)->umlObject();
        if (objToBeMoved != NULL) {
            UMLListView *lv = UMLApp::app()->listView();
            lv->moveObject(objToBeMoved->id(),
                           Model_Utils::convert_OT_LVT(objToBeMoved),
                           lv->theLogicalView());
            // UMLListView::moveObject() will delete the containment
            // AssociationWidget via UMLView::updateContainment().
        } else {
            DEBUG(DBG_SRC) << "removeAssocInViewAndDoc(containment): objB is NULL";
        }
    } else {
        // Remove assoc in doc.
        m_doc->removeAssociation(a->getAssociation());
        // Remove assoc in view.
        removeAssoc(a);
    }
}

/**
 * Removes all the associations related to Widget.
 */
void UMLView::removeAssociations(UMLWidget* Widget)
{
    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        if (assocwidget->contains(Widget)) {
            removeAssoc(assocwidget);
        }
    }
}

void UMLView::selectAssociations(bool bSelect)
{
    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        if (bSelect &&
                assocwidget->widgetForRole(A) && assocwidget->widgetForRole(A)->getSelected() &&
                assocwidget->widgetForRole(B) && assocwidget->widgetForRole(B)->getSelected()) {
            assocwidget->setSelected(true);
        } else {
            assocwidget->setSelected(false);
        }
    }//end foreach
}

void UMLView::getWidgetAssocs(UMLObject* Obj, AssociationWidgetList & Associations)
{
    if (! Obj)
        return;

    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        if (assocwidget->widgetForRole(A)->umlObject() == Obj ||
                assocwidget->widgetForRole(B)->umlObject() == Obj)
            Associations.append(assocwidget);
    }//end foreach

}

/**
 * Override standard method.
 */
void UMLView::closeEvent(QCloseEvent * e)
{
    QWidget::closeEvent(e);
}

void UMLView::removeAllAssociations()
{
    //Remove All association widgets

    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        removeAssoc(assocwidget);
    }

    // Porting to QList from QPtrList which doesn't support autodelete
    //m_AssociationList.clear();
    while (!m_AssociationList.empty()) {
        delete m_AssociationList.takeFirst();
    }
}

void UMLView::removeAllWidgets()
{
    // Remove widgets.

    foreach(UMLWidget* temp , m_WidgetList) {
        // I had to take this condition back in, else umbrello
        // crashes on exit. Still to be analyzed.  --okellogg
        if (!(temp->baseType() == WidgetBase::wt_Text &&
                ((FloatingTextWidget *)temp)->textRole() != Uml::TextRole::Floating)) {
            removeWidget(temp);
        }
    }
    // Porting to QList from QPtrList which doesn't support autodelete
    //m_WidgetList.clear();
    while (!m_WidgetList.empty()) {
        delete m_WidgetList.takeFirst();
    }

}

void UMLView::showDocumentation(UMLObject * object, bool overwrite)
{
    UMLApp::app()->docWindow()->showDocumentation(object, overwrite);
    m_bChildDisplayedDoc = true;
}

void UMLView::showDocumentation(UMLWidget * widget, bool overwrite)
{
    UMLApp::app()->docWindow()->showDocumentation(widget, overwrite);
    m_bChildDisplayedDoc = true;
}

void UMLView::showDocumentation(AssociationWidget * widget, bool overwrite)
{
    UMLApp::app()->docWindow()->showDocumentation(widget, overwrite);
    m_bChildDisplayedDoc = true;
}

void UMLView::updateDocumentation(bool clear)
{
    UMLApp::app()->docWindow()->updateDocumentation(clear);
}

void UMLView::updateContainment(UMLCanvasObject *self)
{
    if (self == NULL)
        return;
    // See if the object has a widget representation in this view.
    // While we're at it, also see if the new parent has a widget here.
    UMLWidget *selfWidget = NULL, *newParentWidget = NULL;
    UMLPackage *newParent = self->umlPackage();
    foreach(UMLWidget* w, m_WidgetList) {
        UMLObject *o = w->umlObject();
        if (o == self)
            selfWidget = w;
        else if (newParent != NULL && o == newParent)
            newParentWidget = w;
    }
    if (selfWidget == NULL)
        return;
    // Remove possibly obsoleted containment association.
    foreach(AssociationWidget* a, m_AssociationList) {
        if (a->associationType() != Uml::AssociationType::Containment)
            continue;
        // Container is at role A, containee at B.
        // We only look at association for which we are B.
        UMLWidget *wB = a->widgetForRole(B);
        UMLObject *roleBObj = wB->umlObject();
        if (roleBObj != self)
            continue;
        UMLWidget *wA = a->widgetForRole(A);
        UMLObject *roleAObj = wA->umlObject();
        if (roleAObj == newParent) {
            // Wow, all done. Great!
            return;
        }
        removeAssoc(a);  // AutoDelete is true
        // It's okay to break out because there can only be a single
        // containing object.
        break;
    }
    if (newParentWidget == NULL)
        return;
    // Create the new containment association.
    AssociationWidget *a = new AssociationWidget(umlScene(), newParentWidget,
            Uml::AssociationType::Containment, selfWidget);
    a->calculateEndingPoints();
    a->setActivated(true);
    m_AssociationList.append(a);
}

void UMLView::createAutoAssociations(UMLWidget * widget)
{
    if (widget == NULL ||
            (m_Type != Uml::DiagramType::Class &&
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
    if (tmpUmlObj == NULL)
        return;
    UMLCanvasObject *umlObj = dynamic_cast<UMLCanvasObject*>(tmpUmlObj);
    if (umlObj == NULL)
        return;
    const UMLAssociationList& umlAssocs = umlObj->getAssociations();

    Uml::IDType myID = umlObj->id();
    foreach(UMLAssociation* assoc , umlAssocs) {
        UMLCanvasObject *other = NULL;
        UMLObject *roleAObj = assoc->getObject(A);
        if (roleAObj == NULL) {
            DEBUG(DBG_SRC) << "roleA object is NULL at UMLAssoc "
                           << ID2STR(assoc->id());
            continue;
        }
        UMLObject *roleBObj = assoc->getObject(B);
        if (roleBObj == NULL) {
            DEBUG(DBG_SRC) << "roleB object is NULL at UMLAssoc "
                           << ID2STR(assoc->id());
            continue;
        }
        if (roleAObj->id() == myID) {
            other = static_cast<UMLCanvasObject*>(roleBObj);
        } else if (roleBObj->id() == myID) {
            other = static_cast<UMLCanvasObject*>(roleAObj);
        } else {
            DEBUG(DBG_SRC) << "Can not find own object "
                           << ID2STR(myID) << " in UMLAssoc "
                           << ID2STR(assoc->id());
            continue;
        }
        // Now that we have determined the "other" UMLObject, seek it in
        // this view's UMLWidgets.
        Uml::IDType otherID = other->id();

        bool breakFlag = false;
        UMLWidget* pOtherWidget = 0;
        foreach(pOtherWidget ,  m_WidgetList) {
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
        Uml::AssociationType assocType = assoc->getAssocType();
        AssociationWidget * assocwidget = findAssocWidget(assocType, widgetA, widgetB);
        if (assocwidget) {
            assocwidget->calculateEndingPoints();  // recompute assoc lines
            continue;
        }
        // Check that the assoc is allowed.
        if (!AssocRules::allowAssociation(assocType, widgetA, widgetB, false)) {
            DEBUG(DBG_SRC) << "not transferring assoc of type " << assocType;
            continue;
        }
        // Create the AssociationWidget.
        assocwidget = new AssociationWidget(umlScene());
        assocwidget->setWidget(widgetA, A);
        assocwidget->setWidget(widgetB, B);
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
    if (t == UMLObject::ot_Package || t == UMLObject::ot_Class || t == UMLObject::ot_Interface || t == UMLObject::ot_Component) {
        // for each of the object's containedObjects
        UMLPackage *umlPkg = static_cast<UMLPackage*>(umlObj);
        UMLObjectList lst = umlPkg->containedObjects();
        foreach(UMLObject* obj,  lst) {
            // if the containedObject has a widget representation on this view then
            Uml::IDType id = obj->id();
            foreach(UMLWidget *w , m_WidgetList) {
                if (w->id() != id)
                    continue;
                // if the containedWidget is not physically located inside this widget
                if (widget->rect().contains(w->rect()))
                    continue;
                // create the containment AssocWidget
                AssociationWidget *a = new AssociationWidget(umlScene(), widget,
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
    if (parent == NULL)
        return;
    // if the parentPackage has a widget representation on this view then
    Uml::IDType pkgID = parent->id();

    bool breakFlag = false;
    UMLWidget* pWidget = 0;
    foreach(pWidget , m_WidgetList) {
        if (pWidget->id() == pkgID) {
            breakFlag = true;
            break;
        }
    }
    if (!breakFlag || pWidget->rect().contains(widget->rect()))
        return;
    // create the containment AssocWidget
    AssociationWidget *a = new AssociationWidget(umlScene(), pWidget, Uml::AssociationType::Containment, widget);
    a->calculateEndingPoints();
    a->setActivated(true);
    if (! addAssociation(a))
        delete a;
}

void UMLView::createAutoAttributeAssociations(UMLWidget *widget)
{
    if (widget == NULL || m_Type != Uml::DiagramType::Class || !m_Options.classState.showAttribAssocs)
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
    if (tmpUmlObj == NULL)
        return;
    // if the underlying model object is really a UMLClassifier then
    if (tmpUmlObj->baseType() == UMLObject::ot_Datatype) {
        UMLClassifier *dt = static_cast<UMLClassifier*>(tmpUmlObj);
        while (dt->originType() != NULL) {
            tmpUmlObj = dt->originType();
            if (tmpUmlObj->baseType() != UMLObject::ot_Datatype)
                break;
            dt = static_cast<UMLClassifier*>(tmpUmlObj);
        }
    }
    if (tmpUmlObj->baseType() != UMLObject::ot_Class)
        return;
    UMLClassifier * klass = static_cast<UMLClassifier*>(tmpUmlObj);
    // for each of the UMLClassifier's UMLAttributes
    UMLAttributeList attrList = klass->getAttributeList();
    foreach(UMLAttribute* attr, attrList) {
        createAutoAttributeAssociation(attr->getType(), attr, widget);
        /*
         * The following code from attachment 19935 of http://bugs.kde.org/140669
         * creates Aggregation/Composition to the template parameters.
         * The current solution uses Dependency instead, see handling of template
         * instantiation at Import_Utils::createUMLObject().
        UMLClassifierList templateList = attr->getTemplateParams();
        for (UMLClassifierListIt it(templateList); it.current(); ++it) {
            createAutoAttributeAssociation(it,attr,widget);
        }
         */
    }
}

void UMLView::createAutoAttributeAssociation(UMLClassifier *type, UMLAttribute *attr,
        UMLWidget *widget /*, UMLClassifier * klass*/)
{
    if (type == NULL) {
        // DEBUG(DBG_SRC) << klass->getName() << ": type is NULL for "
        //                << "attribute " << attr->getName();
        return;
    }
    Uml::AssociationType assocType = Uml::AssociationType::Composition;
    UMLWidget *w = findWidget(type->id());
    AssociationWidget *aw = NULL;
    // if the attribute type has a widget representation on this view
    if (w) {
        aw = findAssocWidget(widget, w, attr->name());
        if (aw == NULL &&
                // if the current diagram type permits compositions
                AssocRules::allowAssociation(assocType, widget, w, false)) {
            // Create a composition AssocWidget, or, if the attribute type is
            // stereotyped <<CORBAInterface>>, create a UniAssociation widget.
            if (type->stereotype() == "CORBAInterface")
                assocType = Uml::AssociationType::UniAssociation;
            AssociationWidget *a = new AssociationWidget(umlScene(), widget, assocType, w, attr);
            a->calculateEndingPoints();
            a->setVisibility(attr->visibility(), B);
            /*
            if (assocType == Uml::AssociationType::Aggregation || assocType == Uml::AssociationType::UniAssociation)
            a->setMulti("0..1", B);
            */
            a->setRoleName(attr->name(), B);
            a->setActivated(true);
            if (! addAssociation(a))
                delete a;
        }
    }
    // if the attribute type is a Datatype then
    if (type->baseType() == UMLObject::ot_Datatype) {
        UMLClassifier *dt = static_cast<UMLClassifier*>(type);
        // if the Datatype is a reference (pointer) type
        if (dt->isReference()) {
            //Uml::AssociationType assocType = Uml::AssociationType::Composition;
            UMLClassifier *c = dt->originType();
            UMLWidget *w = c ? findWidget(c->id()) : 0;
            // if the referenced type has a widget representation on this view
            if (w) {
                aw = findAssocWidget(widget, w, attr->name());
                if (aw == NULL &&
                        // if the current diagram type permits aggregations
                        AssocRules::allowAssociation(Uml::AssociationType::Aggregation, widget, w, false)) {
                    // create an aggregation AssocWidget from the ClassifierWidget
                    // to the widget of the referenced type
                    AssociationWidget *a = new AssociationWidget
                    (umlScene(), widget, Uml::AssociationType::Aggregation, w, attr);
                    a->calculateEndingPoints();
                    a->setVisibility(attr->visibility(), B);
                    //a->setChangeability(true, B);
                    a->setMultiplicity("0..1", B);
                    a->setRoleName(attr->name(), B);
                    a->setActivated(true);
                    if (! addAssociation(a))
                        delete a;
                }
            }
        }
    }
}

void UMLView::createAutoConstraintAssociations(UMLWidget *widget)
{
    if (widget == NULL || m_Type != Uml::DiagramType::EntityRelationship)
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
    if (tmpUmlObj == NULL)
        return;
    // check if the underlying model object is really a UMLEntity
    UMLCanvasObject *umlObj = dynamic_cast<UMLCanvasObject*>(tmpUmlObj);
    if (umlObj == NULL)
        return;
    // finished checking whether this widget has a UMLCanvas Object

    if (tmpUmlObj->baseType() != UMLObject::ot_Entity)
        return;
    UMLEntity *entity = static_cast<UMLEntity*>(tmpUmlObj);

    // for each of the UMLEntity's UMLForeignKeyConstraints
    UMLClassifierListItemList constrList = entity->getFilteredList(UMLObject::ot_ForeignKeyConstraint);

    foreach(UMLClassifierListItem* cli, constrList) {
        UMLEntityConstraint *eConstr = static_cast<UMLEntityConstraint*>(cli);

        UMLForeignKeyConstraint* fkc = static_cast<UMLForeignKeyConstraint*>(eConstr);
        if (fkc == NULL) {
            return;
        }

        UMLEntity* refEntity = fkc->getReferencedEntity();
        if (refEntity == NULL) {
            return;
        }

        createAutoConstraintAssociation(refEntity , fkc , widget);
    }
}

void UMLView::createAutoConstraintAssociation(UMLEntity* refEntity, UMLForeignKeyConstraint* fkConstraint, UMLWidget* widget)
{
    if (refEntity == NULL) {
        return;
    }

    Uml::AssociationType assocType = Uml::AssociationType::Relationship;
    UMLWidget *w = findWidget(refEntity->id());
    AssociationWidget *aw = NULL;

    if (w) {
        aw = findAssocWidget(widget, w, fkConstraint->name());
        if (aw == NULL &&
                // if the current diagram type permits relationships
                AssocRules::allowAssociation(assocType, widget, w, false)) {

            // for foreign key contstraint, we need to create the association type Uml::AssociationType::Relationship.
            // The referenced entity is the "1" part (Role A) and the entity holding the relationship is the "many" part. ( Role B)
            AssociationWidget *a = new AssociationWidget(umlScene(), w, assocType, widget);
            a->setUMLObject(fkConstraint);
            a->calculateEndingPoints();
            //a->setVisibility(attr->getVisibility(), B);
            a->setRoleName(fkConstraint->name(), B);
            a->setActivated(true);
            if (! addAssociation(a))
                delete a;
        }
    }

}

void UMLView::findMaxBoundingRectangle(const FloatingTextWidget* ft, int& px, int& py, int& qx, int& qy)
{
    if (ft == NULL || !ft->isVisible())
        return;

    int x = ft->getX();
    int y = ft->getY();
    int x1 = x + ft->getWidth() - 1;
    int y1 = y + ft->getHeight() - 1;

    if (px == -1 || x < px)
        px = x;
    if (py == -1 || y < py)
        py = y;
    if (qx == -1 || x1 > qx)
        qx = x1;
    if (qy == -1 || y1 > qy)
        qy = y1;
}

void UMLView::copyAsImage(QPixmap*& pix)
{
    //get the smallest rect holding the diagram
    QRect rect = diagramRect();
    QPixmap diagram(rect.width(), rect.height());

    //only draw what is selected
    m_bDrawSelectedOnly = true;
    selectAssociations(true);
    getDiagram(rect, diagram);

    //now get the selection cut
    int px = -1, py = -1, qx = -1, qy = -1;

    //first get the smallest rect holding the widgets
    foreach(UMLWidget* temp , m_SelectedList) {
        int x = temp->getX();
        int y = temp->getY();
        int x1 = x + temp->width() - 1;
        int y1 = y + temp->height() - 1;
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
    foreach(AssociationWidget *a , m_AssociationList) {
        if (! a->getSelected())
            continue;
        const FloatingTextWidget* multiA = const_cast<FloatingTextWidget*>(a->getMultiWidget(A));
        const FloatingTextWidget* multiB = const_cast<FloatingTextWidget*>(a->getMultiWidget(B));
        const FloatingTextWidget* roleA = const_cast<FloatingTextWidget*>(a->getRoleWidget(A));
        const FloatingTextWidget* roleB = const_cast<FloatingTextWidget*>(a->getRoleWidget(B));
        const FloatingTextWidget* changeA = const_cast<FloatingTextWidget*>(a->getChangeWidget(A));
        const FloatingTextWidget* changeB = const_cast<FloatingTextWidget*>(a->getChangeWidget(B));
        findMaxBoundingRectangle(multiA, px, py, qx, qy);
        findMaxBoundingRectangle(multiB, px, py, qx, qy);
        findMaxBoundingRectangle(roleA, px, py, qx, qy);
        findMaxBoundingRectangle(roleB, px, py, qx, qy);
        findMaxBoundingRectangle(changeA, px, py, qx, qy);
        findMaxBoundingRectangle(changeB, px, py, qx, qy);
    }//end foreach

    QRect imageRect;  //area with respect to diagramRect()
    //i.e. all widgets on the canvas.  Was previously with
    //respect to whole canvas

    imageRect.setLeft(px - rect.left());
    imageRect.setTop(py - rect.top());
    imageRect.setRight(qx - rect.left());
    imageRect.setBottom(qy - rect.top());

    pix = new QPixmap(imageRect.width(), imageRect.height());
    QPainter output(pix);
    output.drawPixmap(QPoint(0, 0), diagram, imageRect);
    m_bDrawSelectedOnly = false;
}

void UMLView::setMenu()
{
    slotRemovePopupMenu();
    ListPopupMenu::MenuType menu = ListPopupMenu::mt_Undefined;
    switch (type()) {
    case Uml::DiagramType::Class:
        menu = ListPopupMenu::mt_On_Class_Diagram;
        break;

    case Uml::DiagramType::UseCase:
        menu = ListPopupMenu::mt_On_UseCase_Diagram;
        break;

    case Uml::DiagramType::Sequence:
        menu = ListPopupMenu::mt_On_Sequence_Diagram;
        break;

    case Uml::DiagramType::Collaboration:
        menu = ListPopupMenu::mt_On_Collaboration_Diagram;
        break;

    case Uml::DiagramType::State:
        menu = ListPopupMenu::mt_On_State_Diagram;
        break;

    case Uml::DiagramType::Activity:
        menu = ListPopupMenu::mt_On_Activity_Diagram;
        break;

    case Uml::DiagramType::Component:
        menu = ListPopupMenu::mt_On_Component_Diagram;
        break;

    case Uml::DiagramType::Deployment:
        menu = ListPopupMenu::mt_On_Deployment_Diagram;
        break;

    case Uml::DiagramType::EntityRelationship:
        menu = ListPopupMenu::mt_On_EntityRelationship_Diagram;
        break;

    default:
        uWarning() << "unknown diagram type " << type();
        menu = ListPopupMenu::mt_Undefined;
        break;
    }//end switch
    if (menu != ListPopupMenu::mt_Undefined) {
        // DEBUG(DBG_SRC) << "create popup for MenuType " << ListPopupMenu::toString(menu);
        m_pMenu = new ListPopupMenu(this, menu, this);
        connect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotMenuSelection(QAction*)));
        m_pMenu->popup(mapToGlobal(contentsToViewport(worldMatrix().map(m_Pos))));
    }
}

void UMLView::slotRemovePopupMenu()
{
    if (m_pMenu) {
        disconnect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotMenuSelection(QAction*)));
        delete m_pMenu;
        m_pMenu = 0;
    }
}

void UMLView::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::mt_Undefined;
    if (m_pMenu != NULL) {  // popup from this class
        sel = m_pMenu->getMenuType(action);
    } else { // popup from umldoc
        sel = m_doc->popupMenuSelection(action);
    }
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

    case ListPopupMenu::mt_FloatText: {
        FloatingTextWidget* ft = new FloatingTextWidget(umlScene());
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

    case ListPopupMenu::mt_Cut:
        //FIXME make this work for diagram's right click menu
        if (m_SelectedList.count() &&
                UMLApp::app()->editCutCopy(true)) {
            deleteSelection();
            m_doc->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Copy:
        //FIXME make this work for diagram's right click menu
        m_SelectedList.count() && UMLApp::app()->editCutCopy(true);
        break;

    case ListPopupMenu::mt_Paste:
        m_PastePoint = m_Pos;
        m_Pos.setX(2000);
        m_Pos.setY(2000);
        UMLApp::app()->slotEditPaste();

        m_PastePoint.setX(0);
        m_PastePoint.setY(0);
        break;

    case ListPopupMenu::mt_Initial_State: {
        StateWidget* state = new StateWidget(umlScene(), StateWidget::Initial);
        setupNewWidget(state);
    }
    break;

    case ListPopupMenu::mt_End_State: {
        StateWidget* state = new StateWidget(umlScene(), StateWidget::End);
        setupNewWidget(state);
    }
    break;

    case ListPopupMenu::mt_Junction: {
        StateWidget* state = new StateWidget(umlScene(), StateWidget::Junction);
        setupNewWidget(state);
    }
    break;

    case ListPopupMenu::mt_DeepHistory: {
        StateWidget* state = new StateWidget(umlScene(), StateWidget::DeepHistory);
        setupNewWidget(state);
    }
    break;

    case ListPopupMenu::mt_ShallowHistory: {
        StateWidget* state = new StateWidget(umlScene(), StateWidget::ShallowHistory);
        setupNewWidget(state);
    }
    break;

    case ListPopupMenu::mt_Choice: {
        StateWidget* state = new StateWidget(umlScene(), StateWidget::Choice);
        setupNewWidget(state);
    }
    break;

    case ListPopupMenu::mt_StateFork: {
        StateWidget* state = new StateWidget(umlScene(), StateWidget::Fork);
        setupNewWidget(state);
    }
    break;

    case ListPopupMenu::mt_StateJoin: {
        StateWidget* state = new StateWidget(umlScene(), StateWidget::Join);
        setupNewWidget(state);
    }
    break;

    case ListPopupMenu::mt_State: {
        bool ok = false;
        QString name = KInputDialog::getText(i18n("Enter State Name"),
                                             i18n("Enter the name of the new state:"),
                                             i18n("new state"), &ok, UMLApp::app());
        if (ok) {
            StateWidget* state = new StateWidget(umlScene());
            state->setName(name);
            setupNewWidget(state);
        }
    }
    break;

    case ListPopupMenu::mt_Initial_Activity: {
        ActivityWidget* activity = new ActivityWidget(umlScene(), ActivityWidget::Initial);
        setupNewWidget(activity);
    }
    break;


    case ListPopupMenu::mt_End_Activity: {
        ActivityWidget* activity = new ActivityWidget(umlScene(), ActivityWidget::End);
        setupNewWidget(activity);
    }
    break;

    case ListPopupMenu::mt_Branch: {
        ActivityWidget* activity = new ActivityWidget(umlScene(), ActivityWidget::Branch);
        setupNewWidget(activity);
    }
    break;

    case ListPopupMenu::mt_Activity: {
        bool ok = false;
        QString name = KInputDialog::getText(i18n("Enter Activity Name"),
                                             i18n("Enter the name of the new activity:"),
                                             i18n("new activity"), &ok, UMLApp::app());
        if (ok) {
            ActivityWidget* activity = new ActivityWidget(umlScene(), ActivityWidget::Normal);
            activity->setName(name);
            setupNewWidget(activity);
        }
    }
    break;

    case ListPopupMenu::mt_SnapToGrid:
        toggleSnapToGrid();
        m_doc->setModified();
        break;

    case ListPopupMenu::mt_ShowSnapGrid:
        toggleShowGrid();
        m_doc->setModified();
        break;

    case ListPopupMenu::mt_Properties:
        if (showPropDialog() == true)
            m_doc->setModified();
        break;

    case ListPopupMenu::mt_Delete:
        m_doc->removeDiagram(getID());
        break;

    case ListPopupMenu::mt_Rename: {
        bool ok = false;
        QString newName = KInputDialog::getText(i18n("Enter Diagram Name"),
                                                i18n("Enter the new name of the diagram:"),
                                                name(), &ok, UMLApp::app());
        if (ok) {
            setName(newName);
            m_doc->signalDiagramRenamed(this);
        }
    }
    break;

    default:
        uWarning() << "unknown ListPopupMenu::MenuType " << ListPopupMenu::toString(sel);
        break;
    }
}

void UMLView::slotCutSuccessful()
{
    if (m_bStartedCut) {
        deleteSelection();
        m_bStartedCut = false;
    }
}

void UMLView::slotShowView()
{
    m_doc->changeCurrentView(getID());
}

QPoint UMLView::getPastePoint()
{
    QPoint point = m_PastePoint;
    point.setX(point.x() - m_Pos.x());
    point.setY(point.y() - m_Pos.y());
    return point;
}

void UMLView::resetPastePoint()
{
    m_PastePoint = m_Pos;
}

int UMLView::snappedX(int x)
{
    if (getSnapToGrid()) {
        int gridX = getSnapX();
        int modX = x % gridX;
        x -= modX;
        if (modX >= gridX / 2)
            x += gridX;
    }
    return x;
}

int UMLView::snappedY(int y)
{
    if (getSnapToGrid()) {
        int gridY = getSnapY();
        int modY = y % gridY;
        y -= modY;
        if (modY >= gridY / 2)
            y += gridY;
    }
    return y;
}

bool UMLView::showPropDialog()
{
    bool success = false;
    QPointer<UMLViewDialog> dlg = new UMLViewDialog(this, this);
    if (dlg->exec() == QDialog::Accepted) {
        success = true;
    }
    delete dlg;
    return success;
}

QFont UMLView::getFont() const
{
    return m_Options.uiState.font;
}

void UMLView::setFont(QFont font, bool changeAllWidgets /* = false */)
{
    m_Options.uiState.font = font;
    if (!changeAllWidgets)
        return;
    foreach(UMLWidget* w, m_WidgetList) {
        w->setFont(font);
    }
}

void UMLView::setClassWidgetOptions(ClassOptionsPage * page)
{
    foreach(UMLWidget* pWidget , m_WidgetList) {
        WidgetBase::WidgetType wt = pWidget->baseType();
        if (wt == WidgetBase::wt_Class || wt == WidgetBase::wt_Interface) {
            page->setWidget(static_cast<ClassifierWidget *>(pWidget));
            page->updateUMLWidget();
        }
    }
}

void UMLView::checkSelections()
{
    UMLWidget * pWA = 0, * pWB = 0;
    //check messages
    foreach(UMLWidget *pTemp , m_SelectedList) {
        if (pTemp->baseType() == WidgetBase::wt_Message && pTemp->getSelected()) {
            MessageWidget * pMessage = static_cast<MessageWidget *>(pTemp);
            pWA = pMessage->objectWidget(A);
            pWB = pMessage->objectWidget(B);
            if (!pWA->getSelected()) {
                pWA->setSelectedFlag(true);
                m_SelectedList.append(pWA);
            }
            if (!pWB->getSelected()) {
                pWB->setSelectedFlag(true);
                m_SelectedList.append(pWB);
            }
        }//end if
    }//end for
    //check Associations

    foreach(AssociationWidget *pAssoc , m_AssociationList) {
        if (pAssoc->getSelected()) {
            pWA = pAssoc->widgetForRole(A);
            pWB = pAssoc->widgetForRole(B);
            if (!pWA->getSelected()) {
                pWA->setSelectedFlag(true);
                m_SelectedList.append(pWA);
            }
            if (!pWB->getSelected()) {
                pWB->setSelectedFlag(true);
                m_SelectedList.append(pWB);
            }
        }//end if
    }//end foreach
}

bool UMLView::checkUniqueSelection()
{
    // if there are no selected items, we return true
    if (m_SelectedList.count() <= 0)
        return true;

    // get the first item and its base type
    UMLWidget * pTemp = (UMLWidget *) m_SelectedList.first();
    WidgetBase::WidgetType tmpType = pTemp->baseType();

    // check all selected items, if they have the same BaseType
    foreach(pTemp , m_SelectedList) {
        if (pTemp->baseType() != tmpType) {
            return false; // the base types are different, the list is not unique
        }
    } // for ( through all selected items )

    return true; // selected items are unique
}

void UMLView::clearDiagram()
{
    if (KMessageBox::Continue == KMessageBox::warningContinueCancel(this, i18n("You are about to delete "
            "the entire diagram.\nAre you sure?"),
            i18n("Delete Diagram?"), KGuiItem(i18n("&Delete"), "edit-delete"))) {
        removeAllWidgets();
    }
}

void UMLView::toggleSnapToGrid()
{
    setSnapToGrid(!getSnapToGrid());
}

void UMLView::toggleSnapComponentSizeToGrid()
{
    setSnapComponentSizeToGrid(!getSnapComponentSizeToGrid());
}

void UMLView::toggleShowGrid()
{
    setShowSnapGrid(!getShowSnapGrid());
}

void UMLView::setSnapToGrid(bool bSnap)
{
    m_bUseSnapToGrid = bSnap;
    emit sigSnapToGridToggled(getSnapToGrid());
}

void UMLView::setSnapComponentSizeToGrid(bool bSnap)
{
    m_bUseSnapComponentSizeToGrid = bSnap;
    updateComponentSizes();
    emit sigSnapComponentSizeToGridToggled(getSnapComponentSizeToGrid());
}

bool UMLView::getShowSnapGrid() const
{
    return m_bShowSnapGrid;
}

void UMLView::setShowSnapGrid(bool bShow)
{
    m_bShowSnapGrid = bShow;
    canvas()->setAllChanged();
    emit sigShowGridToggled(getShowSnapGrid());
}

bool UMLView::getShowOpSig() const
{
    return m_Options.classState.showOpSig;
}

void UMLView::setShowOpSig(bool bShowOpSig)
{
    m_Options.classState.showOpSig = bShowOpSig;
}

bool UMLView::isOpen() const
{
    return m_isOpen;
}

void UMLView::setIsOpen(bool isOpen)
{
    m_isOpen = isOpen;
}

/**
 * Sets the zoom of the diagram.
 */
void UMLView::setZoom(int zoom)
{
    if (zoom < 10) {
        zoom = 10;
    } else if (zoom > 500) {
        zoom = 500;
    }

    QMatrix wm;
    wm.scale(zoom / 100.0, zoom / 100.0);
    setWorldMatrix(wm);

    m_nZoom = currentZoom();
    resizeCanvasToItems();
}

/**
 * return the current zoom factor
 */
int UMLView::currentZoom()
{
    return (int)(worldMatrix().m11()*100.0);
}

void UMLView::zoomIn()
{
    QMatrix wm = worldMatrix();
    wm.scale(1.5, 1.5); // adjust zooming step here
    setZoom((int)(wm.m11()*100.0));
}

void UMLView::zoomOut()
{
    QMatrix wm = worldMatrix();
    wm.scale(2.0 / 3.0, 2.0 / 3.0); //adjust zooming step here
    setZoom((int)(wm.m11()*100.0));
}

void UMLView::fileLoaded()
{
    setZoom(zoom());
    resizeCanvasToItems();
}

void UMLView::setCanvasSize(int width, int height)
{
    setCanvasWidth(width);
    setCanvasHeight(height);
    canvas()->resize(width, height);
}

void UMLView::resizeCanvasToItems()
{
    QRect canvasSize = diagramRect();
    int canvasWidth = canvasSize.right() + 5;
    int canvasHeight = canvasSize.bottom() + 5;

    //Find out the bottom right visible pixel and size to at least that
    int contentsX, contentsY;
    int contentsWMX, contentsWMY;
    viewportToContents(viewport()->width(), viewport()->height(), contentsX, contentsY);
    inverseWorldMatrix().map(contentsX, contentsY, &contentsWMX, &contentsWMY);

    if (canvasWidth < contentsWMX) {
        canvasWidth = contentsWMX;
    }

    if (canvasHeight < contentsWMY) {
        canvasHeight = contentsWMY;
    }

    setCanvasSize(canvasWidth, canvasHeight);
}

/**
 * Overrides standard method from QWidget to resize canvas when
 * it's shown.
 */
void UMLView::show()
{
    QWidget::show();
    resizeCanvasToItems();
}

void UMLView::updateComponentSizes()
{
    // update sizes of all components

    foreach(UMLWidget *obj , m_WidgetList) {
        obj->updateComponentSize();
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
void UMLView::forceUpdateWidgetFontMetrics(QPainter * painter)
{
    foreach(UMLWidget *obj, m_WidgetList) {
        obj->forceUpdateFontMetrics(painter);
    }
}

void UMLView::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement viewElement = qDoc.createElement("diagram");
    viewElement.setAttribute("xmi.id", ID2STR(m_nID));
    viewElement.setAttribute("name", name());
    viewElement.setAttribute("type", m_Type);
    viewElement.setAttribute("documentation", m_Documentation);
    //optionstate uistate
    viewElement.setAttribute("fillcolor", m_Options.uiState.fillColor.name());
    viewElement.setAttribute("linecolor", m_Options.uiState.lineColor.name());
    viewElement.setAttribute("linewidth", m_Options.uiState.lineWidth);
    viewElement.setAttribute("usefillcolor", m_Options.uiState.useFillColor);
    viewElement.setAttribute("font", m_Options.uiState.font.toString());
    QString backgroundColor = m_Options.uiState.backgroundColor.name();
    QString gridDotColor    = m_Options.uiState.gridDotColor.name();
    UMLViewCanvas* canvs = dynamic_cast<UMLViewCanvas*>(canvas());
    if (canvs) {
        backgroundColor = canvs->backgroundColor().name();
        gridDotColor    = canvs->gridDotColor().name();
    }
    viewElement.setAttribute("backgroundcolor", backgroundColor);
    viewElement.setAttribute("griddotcolor",    gridDotColor);
    //optionstate classstate
    viewElement.setAttribute("showattsig", m_Options.classState.showAttSig);
    viewElement.setAttribute("showatts", m_Options.classState.showAtts);
    viewElement.setAttribute("showopsig", m_Options.classState.showOpSig);
    viewElement.setAttribute("showops", m_Options.classState.showOps);
    viewElement.setAttribute("showpackage", m_Options.classState.showPackage);
    viewElement.setAttribute("showattribassocs", m_Options.classState.showAttribAssocs);
    viewElement.setAttribute("showpubliconly", m_Options.classState.showPublicOnly);
    viewElement.setAttribute("showscope", m_Options.classState.showVisibility);
    viewElement.setAttribute("showstereotype", m_Options.classState.showStereoType);
    //misc
    viewElement.setAttribute("localid", ID2STR(m_nLocalID));
    viewElement.setAttribute("showgrid", m_bShowSnapGrid);
    viewElement.setAttribute("snapgrid", m_bUseSnapToGrid);
    viewElement.setAttribute("snapcsgrid", m_bUseSnapComponentSizeToGrid);
    viewElement.setAttribute("snapx", m_nSnapX);
    viewElement.setAttribute("snapy", m_nSnapY);
    viewElement.setAttribute("zoom", m_nZoom);
    viewElement.setAttribute("canvasheight", m_nCanvasHeight);
    viewElement.setAttribute("canvaswidth", m_nCanvasWidth);
    viewElement.setAttribute("isopen", isOpen());

    //now save all the widgets
    QDomElement widgetElement = qDoc.createElement("widgets");
    foreach(UMLWidget *widget , m_WidgetList) {
        // Having an exception is bad I know, but gotta work with
        // system we are given.
        // We DON'T want to record any text widgets which are belonging
        // to associations as they are recorded later in the "associations"
        // section when each owning association is dumped. -b.t.
        if ((widget->baseType() != WidgetBase::wt_Text && widget->baseType() != WidgetBase::wt_FloatingDashLine) ||
                static_cast<FloatingTextWidget*>(widget)->link() == NULL)
            widget->saveToXMI(qDoc, widgetElement);
    }
    viewElement.appendChild(widgetElement);
    //now save the message widgets
    QDomElement messageElement = qDoc.createElement("messages");
    foreach(UMLWidget* widget , m_MessageList) {
        widget->saveToXMI(qDoc, messageElement);
    }
    viewElement.appendChild(messageElement);
    //now save the associations
    QDomElement assocElement = qDoc.createElement("associations");
    if (m_AssociationList.count()) {
        // We guard against ( m_AssociationList.count() == 0 ) because
        // this code could be reached as follows:
        //  ^  UMLView::saveToXMI()
        //  ^  UMLDoc::saveToXMI()
        //  ^  UMLDoc::addToUndoStack()
        //  ^  UMLDoc::setModified()
        //  ^  UMLDoc::createDiagram()
        //  ^  UMLDoc::newDocument()
        //  ^  UMLApp::newDocument()
        //  ^  main()
        //
        AssociationWidgetListIt a_it(m_AssociationList);
        AssociationWidget * assoc = 0;
        foreach(assoc , m_AssociationList) {
            assoc->saveToXMI(qDoc, assocElement);
        }
    }
    viewElement.appendChild(assocElement);
    qElement.appendChild(viewElement);
}

bool UMLView::loadFromXMI(QDomElement & qElement)
{
    QString id = qElement.attribute("xmi.id", "-1");
    m_nID = STR2ID(id);
    if (m_nID == Uml::id_None)
        return false;
    setName(qElement.attribute("name", ""));
    QString type = qElement.attribute("type", "0");
    m_Documentation = qElement.attribute("documentation", "");
    QString localid = qElement.attribute("localid", "0");
    //optionstate uistate
    QString font = qElement.attribute("font", "");
    if (!font.isEmpty()) {
        m_Options.uiState.font.fromString(font);
        m_Options.uiState.font.setUnderline(false);
    }
    QString fillcolor = qElement.attribute("fillcolor", "");
    QString linecolor = qElement.attribute("linecolor", "");
    QString linewidth = qElement.attribute("linewidth", "");
    QString usefillcolor = qElement.attribute("usefillcolor", "0");
    QString backgroundColor = qElement.attribute("backgroundcolor", "");
    QString gridDotColor = qElement.attribute("griddotcolor", "");
    m_Options.uiState.useFillColor = (bool)usefillcolor.toInt();
    //optionstate classstate
    QString temp = qElement.attribute("showattsig", "0");
    m_Options.classState.showAttSig = (bool)temp.toInt();
    temp = qElement.attribute("showatts", "0");
    m_Options.classState.showAtts = (bool)temp.toInt();
    temp = qElement.attribute("showopsig", "0");
    m_Options.classState.showOpSig = (bool)temp.toInt();
    temp = qElement.attribute("showops", "0");
    m_Options.classState.showOps = (bool)temp.toInt();
    temp = qElement.attribute("showpackage", "0");
    m_Options.classState.showPackage = (bool)temp.toInt();
    temp = qElement.attribute("showattribassocs", "0");
    m_Options.classState.showAttribAssocs = (bool)temp.toInt();
    temp = qElement.attribute("showscope", "0");
    m_Options.classState.showVisibility = (bool)temp.toInt();
    temp = qElement.attribute("showstereotype", "0");
    m_Options.classState.showStereoType = (bool)temp.toInt();
    temp = qElement.attribute("showpubliconly", "0");
    m_Options.classState.showPublicOnly = (bool)temp.toInt();
    //misc
    QString showgrid = qElement.attribute("showgrid", "0");
    m_bShowSnapGrid = (bool)showgrid.toInt();

    QString snapgrid = qElement.attribute("snapgrid", "0");
    m_bUseSnapToGrid = (bool)snapgrid.toInt();

    QString snapcsgrid = qElement.attribute("snapcsgrid", "0");
    m_bUseSnapComponentSizeToGrid = (bool)snapcsgrid.toInt();

    QString snapx = qElement.attribute("snapx", "10");
    m_nSnapX = snapx.toInt();

    QString snapy = qElement.attribute("snapy", "10");
    m_nSnapY = snapy.toInt();

    QString zoom = qElement.attribute("zoom", "100");
    m_nZoom = zoom.toInt();

    QString height = qElement.attribute("canvasheight", QString("%1").arg(UMLView::defaultCanvasSize));
    m_nCanvasHeight = height.toInt();

    QString width = qElement.attribute("canvaswidth", QString("%1").arg(UMLView::defaultCanvasSize));
    m_nCanvasWidth = width.toInt();

    QString isOpen = qElement.attribute("isopen", "1");
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
        default:
            m_Type = Uml::DiagramType::Undefined;
            break;
        }
    } else {
        m_Type = Uml::DiagramType::Value(nType);
    }
    if (!fillcolor.isEmpty())
        m_Options.uiState.fillColor = QColor(fillcolor);
    if (!linecolor.isEmpty())
        m_Options.uiState.lineColor = QColor(linecolor);
    if (!linewidth.isEmpty())
        m_Options.uiState.lineWidth = linewidth.toInt();
    if (!backgroundColor.isEmpty())
        m_Options.uiState.backgroundColor = QColor(backgroundColor);
    if (!gridDotColor.isEmpty())
        m_Options.uiState.gridDotColor = QColor(gridDotColor);
    if (!backgroundColor.isEmpty() & !gridDotColor.isEmpty()) {
        UMLViewCanvas* canvs = dynamic_cast<UMLViewCanvas*>(canvas());
        if (canvs) {
            DEBUG(DBG_SRC) << "Set background color to [" << backgroundColor << "] and grid dot color to [" << gridDotColor << "]";
            canvs->setColors(QColor(backgroundColor), QColor(gridDotColor));
        }
    }
    m_nLocalID = STR2ID(localid);

    QDomNode node = qElement.firstChild();
    bool widgetsLoaded = false, messagesLoaded = false, associationsLoaded = false;
    while (!node.isNull()) {
        QDomElement element = node.toElement();
        if (!element.isNull()) {
            if (element.tagName() == "widgets")
                widgetsLoaded = loadWidgetsFromXMI(element);
            else if (element.tagName() == "messages")
                messagesLoaded = loadMessagesFromXMI(element);
            else if (element.tagName() == "associations")
                associationsLoaded = loadAssociationsFromXMI(element);
        }
        node = node.nextSibling();
    }

    if (!widgetsLoaded) {
        uWarning() << "failed umlview load on widgets";
        return false;
    }
    if (!messagesLoaded) {
        uWarning() << "failed umlview load on messages";
        return false;
    }
    if (!associationsLoaded) {
        uWarning() << "failed umlview load on associations";
        return false;
    }
    return true;
}

bool UMLView::loadWidgetsFromXMI(QDomElement & qElement)
{
    UMLWidget* widget = 0;
    QDomNode node = qElement.firstChild();
    QDomElement widgetElement = node.toElement();
    while (!widgetElement.isNull()) {
        widget = loadWidgetFromXMI(widgetElement);
        if (widget) {
            m_WidgetList.append(widget);
            widget->clipSize();
            // In the interest of best-effort loading, in case of a
            // (widget == NULL) we still go on.
            // The individual widget's loadFromXMI method should
            // already have generated an error message to tell the
            // user that something went wrong.
        }
        node = widgetElement.nextSibling();
        widgetElement = node.toElement();
    }

    return true;
}

UMLWidget* UMLView::loadWidgetFromXMI(QDomElement& widgetElement)
{
    if (!m_doc) {
        uWarning() << "m_doc is NULL";
        return 0L;
    }

    QString tag  = widgetElement.tagName();
    QString idstr  = widgetElement.attribute("xmi.id", "-1");
    UMLWidget* widget = Widget_Factory::makeWidgetFromXMI(tag, idstr, umlScene());

    if (widget == NULL)
        return NULL;
    if (!widget->loadFromXMI(widgetElement)) {
        widget->cleanup();
        delete widget;
        return 0;
    }
    return widget;
}

bool UMLView::loadMessagesFromXMI(QDomElement & qElement)
{
    MessageWidget * message = 0;
    QDomNode node = qElement.firstChild();
    QDomElement messageElement = node.toElement();
    while (!messageElement.isNull()) {
        QString tag = messageElement.tagName();
        if (tag == "messagewidget" ||
                tag == "UML:MessageWidget") {   // for bkwd compatibility
            message = new MessageWidget(umlScene(), sequence_message_asynchronous,
                                        Uml::id_Reserved);
            if (!message->loadFromXMI(messageElement)) {
                delete message;
                return false;
            }
            m_MessageList.append(message);
            FloatingTextWidget *ft = message->floatingTextWidget();
            if (ft)
                m_WidgetList.append(ft);
            else if (message->sequenceMessageType() != sequence_message_creation)
                DEBUG(DBG_SRC) << "ft is NULL for message " << ID2STR(message->id());
        }
        node = messageElement.nextSibling();
        messageElement = node.toElement();
    }
    return true;
}

bool UMLView::loadAssociationsFromXMI(QDomElement & qElement)
{
    QDomNode node = qElement.firstChild();
    QDomElement assocElement = node.toElement();
    int countr = 0;
    while (!assocElement.isNull()) {
        QString tag = assocElement.tagName();
        if (tag == "assocwidget" ||
                tag == "UML:AssocWidget") {  // for bkwd compatibility
            countr++;
            AssociationWidget *assoc = new AssociationWidget(umlScene());
            if (!assoc->loadFromXMI(assocElement)) {
                uError() << "could not loadFromXMI association widget:"
                    << assoc << ", bad XMI file? Deleting from umlview.";
                delete assoc;
                /* return false;
                   Returning false here is a little harsh when the
                   rest of the diagram might load okay.
                 */
            } else {
                assoc->clipSize();
                if (!addAssociation(assoc, false)) {
                    uError() << "Could not addAssociation(" << assoc << ") to umlview, deleting.";
                    //               assoc->cleanup();
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

void UMLView::addObject(UMLObject *object)
{
    m_bCreateObject = true;
    if (m_doc->addUMLObject(object))
        m_doc->signalUMLObjectCreated(object);  // m_bCreateObject is reset by slotObjectCreated()
    else
        m_bCreateObject = false;
}

bool UMLView::loadUisDiagramPresentation(QDomElement & qElement)
{
    for (QDomNode node = qElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement elem = node.toElement();
        QString tag = elem.tagName();
        if (! UMLDoc::tagEq(tag, "Presentation")) {
            uError() << "ignoring unknown UisDiagramPresentation tag " << tag;
            continue;
        }
        QDomNode n = elem.firstChild();
        QDomElement e = n.toElement();
        QString idStr;
        int x = 0, y = 0, w = 0, h = 0;
        while (!e.isNull()) {
            tag = e.tagName();
            DEBUG(DBG_SRC) << "Presentation: tag = " << tag;
            if (UMLDoc::tagEq(tag, "Presentation.geometry")) {
                QDomNode gnode = e.firstChild();
                QDomElement gelem = gnode.toElement();
                QString csv = gelem.text();
                QStringList dim = csv.split(',');
                x = dim[0].toInt();
                y = dim[1].toInt();
                w = dim[2].toInt();
                h = dim[3].toInt();
            } else if (UMLDoc::tagEq(tag, "Presentation.style")) {
                // TBD
            } else if (UMLDoc::tagEq(tag, "Presentation.model")) {
                QDomNode mnode = e.firstChild();
                QDomElement melem = mnode.toElement();
                idStr = melem.attribute("xmi.idref", "");
            } else {
                DEBUG(DBG_SRC) << "ignoring tag " << tag;
            }
            n = n.nextSibling();
            e = n.toElement();
        }
        Uml::IDType id = STR2ID(idStr);
        UMLObject *o = m_doc->findObjectById(id);
        if (o == NULL) {
            uError() << "Cannot find object for id " << idStr;
        } else {
            UMLObject::ObjectType ot = o->baseType();
            DEBUG(DBG_SRC) << "Create widget for model object of type " << UMLObject::toString(ot);
            UMLWidget *widget = NULL;
            switch (ot) {
            case UMLObject::ot_Class:
                widget = new ClassifierWidget(umlScene(), static_cast<UMLClassifier*>(o));
                break;
            case UMLObject::ot_Association: {
                UMLAssociation *umla = static_cast<UMLAssociation*>(o);
                Uml::AssociationType at = umla->getAssocType();
                UMLObject* objA = umla->getObject(Uml::A);
                UMLObject* objB = umla->getObject(Uml::B);
                if (objA == NULL || objB == NULL) {
                    uError() << "intern err 1";
                    return false;
                }
                UMLWidget *wA = findWidget(objA->id());
                UMLWidget *wB = findWidget(objB->id());
                if (wA != NULL && wB != NULL) {
                    AssociationWidget *aw =
                        new AssociationWidget(umlScene(), wA, at, wB, umla);
                    aw->syncToModel();
                    m_AssociationList.append(aw);
                } else {
                    uError() << "cannot create assocwidget from ("
                        << wA << ", " << wB << ")";
                }
                break;
            }
            case UMLObject::ot_Role: {
                //UMLRole *robj = static_cast<UMLRole*>(o);
                //UMLAssociation *umla = robj->getParentAssociation();
                // @todo properly display role names.
                //       For now, in order to get the role names displayed
                //       simply delete the participating diagram objects
                //       and drag them from the list view to the diagram.
                break;
            }
            default:
                uError() << "Cannot create widget of type " << ot;
            }
            if (widget) {
                DEBUG(DBG_SRC) << "Widget: x=" << x << ", y=" << y << ", w=" << w << ", h=" << h;
                widget->setX(x);
                widget->setY(y);
                widget->setSize(w, h);
                m_WidgetList.append(widget);
            }
        }
    }
    return true;
}

bool UMLView::loadUISDiagram(QDomElement & qElement)
{
    QString idStr = qElement.attribute("xmi.id", "");
    if (idStr.isEmpty())
        return false;
    m_nID = STR2ID(idStr);
    UMLListViewItem *ulvi = NULL;
    for (QDomNode node = qElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement elem = node.toElement();
        QString tag = elem.tagName();
        if (tag == "uisDiagramName") {
            setName(elem.text());
            if (ulvi)
                ulvi->setText(name());
        } else if (tag == "uisDiagramStyle") {
            QString diagramStyle = elem.text();
            if (diagramStyle != "ClassDiagram") {
                uError() << "diagram style " << diagramStyle << " is not yet implemented";
                continue;
            }
            m_doc->setMainViewID(m_nID);
            m_Type = Uml::DiagramType::Class;
            UMLListView *lv = UMLApp::app()->listView();
            ulvi = new UMLListViewItem(lv->theLogicalView(), name(),
                                       UMLListViewItem::lvt_Class_Diagram, m_nID);
        } else if (tag == "uisDiagramPresentation") {
            loadUisDiagramPresentation(elem);
        } else if (tag != "uisToolName") {
            DEBUG(DBG_SRC) << "ignoring tag " << tag;
        }
    }
    return true;
}

void UMLView::alignLeft()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;

    int smallestX = getSmallestX(widgetList);

    foreach(UMLWidget *widget , widgetList) {
        widget->setX(smallestX);
        widget->adjustAssocs(widget->getX(), widget->getY());
    }
}

void UMLView::alignRight()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;
    int biggestX = getBiggestX(widgetList);

    foreach(UMLWidget *widget , widgetList) {
        widget->setX(biggestX - widget->getWidth());
        widget->adjustAssocs(widget->getX(), widget->getY());
    }
}

void UMLView::alignTop()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;

    int smallestY = getSmallestY(widgetList);

    foreach(UMLWidget *widget , widgetList) {
        widget->setY(smallestY);
        widget->adjustAssocs(widget->getX(), widget->getY());
    }
}

void UMLView::alignBottom()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;
    int biggestY = getBiggestY(widgetList);

    foreach(UMLWidget *widget , widgetList) {
        widget->setY(biggestY - widget->getHeight());
        widget->adjustAssocs(widget->getX(), widget->getY());
    }
}

void UMLView::alignVerticalMiddle()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;

    int smallestX = getSmallestX(widgetList);
    int biggestX = getBiggestX(widgetList);
    int middle = int((biggestX - smallestX) / 2) + smallestX;

    foreach(UMLWidget *widget , widgetList) {
        widget->setX(middle - int(widget->getWidth() / 2));
        widget->adjustAssocs(widget->getX(), widget->getY());
    }
}

void UMLView::alignHorizontalMiddle()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;

    int smallestY = getSmallestY(widgetList);
    int biggestY = getBiggestY(widgetList);
    int middle = int((biggestY - smallestY) / 2) + smallestY;

    foreach(UMLWidget *widget , widgetList) {
        widget->setY(middle - int(widget->getHeight() / 2));
        widget->adjustAssocs(widget->getX(), widget->getY());
    }
}

void UMLView::alignVerticalDistribute()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;

    int smallestY = getSmallestY(widgetList);
    int biggestY = getBiggestY(widgetList);
    int heightsSum = getHeightsSum(widgetList);
    int distance = int(((biggestY - smallestY) - heightsSum) / (widgetList.count() - 1.0) + 0.5);

    sortWidgetList(widgetList, hasWidgetSmallerY);

    int i = 1;
    UMLWidget* widgetPrev = NULL;
    foreach(UMLWidget *widget , widgetList) {
        if (i == 1) {
            widgetPrev = widget;
        } else {
            widget->setY(widgetPrev->getY() + widgetPrev->getHeight() + distance);
            widget->adjustAssocs(widget->getX(), widget->getY());
            widgetPrev = widget;
        }
        i++;
    }
}

void UMLView::alignHorizontalDistribute()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;

    int smallestX = getSmallestX(widgetList);
    int biggestX = getBiggestX(widgetList);
    int widthsSum = getWidthsSum(widgetList);
    int distance = int(((biggestX - smallestX) - widthsSum) / (widgetList.count() - 1.0) + 0.5);

    sortWidgetList(widgetList, hasWidgetSmallerX);

    int i = 1;
    UMLWidget* widgetPrev = NULL;
    foreach(UMLWidget *widget ,  widgetList) {
        if (i == 1) {
            widgetPrev = widget;
        } else {
            widget->setX(widgetPrev->getX() + widgetPrev->getWidth() + distance);
            widget->adjustAssocs(widget->getX(), widget->getY());
            widgetPrev = widget;
        }
        i++;
    }

}

bool UMLView::hasWidgetSmallerX(const UMLWidget* widget1, const UMLWidget* widget2)
{
    return widget1->getX() < widget2->getX();
}

bool UMLView::hasWidgetSmallerY(const UMLWidget* widget1, const UMLWidget* widget2)
{
    return widget1->getY() < widget2->getY();
}

int UMLView::getSmallestX(const UMLWidgetList &widgetList)
{
    UMLWidgetListIt it(widgetList);

    int smallestX = 0;

    int i = 1;
    foreach(UMLWidget *widget ,  widgetList) {
        if (i == 1) {
            smallestX = widget->getX();
        } else {
            if (smallestX > widget->getX())
                smallestX = widget->getX();
        }
        i++;
    }

    return smallestX;
}

int UMLView::getSmallestY(const UMLWidgetList &widgetList)
{

    if (widgetList.isEmpty())
        return -1;

    int smallestY = 0;

    int i = 1;
    foreach(UMLWidget *widget ,  widgetList) {
        if (i == 1) {
            smallestY = widget->getY();
        } else {
            if (smallestY > widget->getY())
                smallestY = widget->getY();
        }
        i++;
    }

    return smallestY;
}

int UMLView::getBiggestX(const UMLWidgetList &widgetList)
{
    if (widgetList.isEmpty())
        return -1;

    int biggestX = 0;

    int i = 1;
    foreach(UMLWidget *widget , widgetList) {
        if (i == 1) {
            biggestX = widget->getX();
            biggestX += widget->getWidth();
        } else {
            if (biggestX < widget->getX() + widget->getWidth())
                biggestX = widget->getX() + widget->getWidth();
        }
        i++;
    }

    return biggestX;
}

int UMLView::getBiggestY(const UMLWidgetList &widgetList)
{
    if (widgetList.isEmpty())
        return -1;

    int biggestY = 0;

    int i = 1;
    foreach(UMLWidget *widget , widgetList) {
        if (i == 1) {
            biggestY = widget->getY();
            biggestY += widget->getHeight();
        } else {
            if (biggestY < widget->getY() + widget->getHeight())
                biggestY = widget->getY() + widget->getHeight();
        }
        i++;
    }

    return biggestY;
}

int UMLView::getHeightsSum(const UMLWidgetList &widgetList)
{
    int heightsSum = 0;

    foreach(UMLWidget *widget , widgetList) {
        heightsSum += widget->getHeight();
    }

    return heightsSum;
}

int UMLView::getWidthsSum(const UMLWidgetList &widgetList)
{
    int widthsSum = 0;

    foreach(UMLWidget *widget , widgetList) {
        widthsSum += widget->getWidth();
    }

    return widthsSum;
}

template<typename Compare>
void UMLView::sortWidgetList(UMLWidgetList &widgetList, Compare comp)
{
    QVector<UMLWidget*> widgetVector;

    for (UMLWidgetList::iterator it = widgetList.begin(); it != widgetList.end(); ++it) {
        widgetVector.push_back(*it);
    }
    qSort(widgetVector.begin(), widgetVector.end(), comp);

    widgetList.clear();

    for (QVector<UMLWidget*>::iterator it = widgetVector.begin(); it != widgetVector.end(); ++it) {
        widgetList.append(*it);
    }
}

#include "umlview.moc"
