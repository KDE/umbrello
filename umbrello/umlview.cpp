/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlview.h"

// system includes
#include <climits>
#include <math.h>

// include files for Qt
#include <qpixmap.h>
#include <qpicture.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qobjectlist.h>
#include <qobjectdict.h>
#include <qdragobject.h>
#include <qpaintdevicemetrics.h>
#include <qfileinfo.h>
#include <qptrlist.h>
#include <qcolor.h>
#include <qwmatrix.h>
#include <qregexp.h>

//kde include files
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kdebug.h>

// application specific includes
#include "umlviewimageexporter.h"
#include "listpopupmenu.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "docwindow.h"
#include "assocrules.h"
#include "umlrole.h"
#include "umlviewcanvas.h"
#include "dialogs/classoptionspage.h"
#include "dialogs/umlviewdialog.h"
#include "clipboard/idchangelog.h"
#include "clipboard/umldrag.h"
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
#include "forkjoinwidget.h"
#include "activitywidget.h"
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


// control the manual DoubleBuffering of QCanvas
// with a define, so that this memory X11 effect can
// be tested more easily
#define MANUAL_CONTROL_DOUBLE_BUFFERING

// static members
const int UMLView::defaultCanvasSize = 1300;

using namespace Uml;


// constructor
UMLView::UMLView(UMLFolder *parentFolder) : QCanvasView(UMLApp::app()->getMainViewWidget()) {
    init();
    m_pDoc = UMLApp::app()->getDocument();
    m_pFolder = parentFolder;
}

void UMLView::init() {
    // Initialize loaded/saved data
    m_nID = Uml::id_None;
    m_pDoc = NULL;
    m_Documentation = "";
    m_Type = dt_Undefined;
    m_bUseSnapToGrid = false;
    m_bUseSnapComponentSizeToGrid = false;
    m_bShowSnapGrid = false;
    m_nSnapX = 10;
    m_nSnapY = 10;
    m_nZoom = 100;
    m_nCanvasWidth = UMLView::defaultCanvasSize;
    m_nCanvasHeight = UMLView::defaultCanvasSize;
    m_nCollaborationId = 0;

    // Initialize other data
    m_AssociationList.setAutoDelete( true );

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

    m_pImageExporter = new UMLViewImageExporter(this);

    //setup graphical items
    viewport() -> setBackgroundMode( Qt::NoBackground );
    setCanvas( new UMLViewCanvas( this ) );
    // don't set the quite frequent update rate for each
    // diagram, as that causes also an update of invisible
    // diagrams, which can cost high CPU load for many
    // diagrams.
    // Instead: set the updatePeriod to 20 on Show event,
    //          and switch update back off on Hide event
    canvas() -> setUpdatePeriod( -1 );
    resizeContents(defaultCanvasSize, defaultCanvasSize);
    canvas() -> resize(defaultCanvasSize, defaultCanvasSize);
    setAcceptDrops(true);
    viewport() -> setAcceptDrops(true);
    setDragAutoScroll(false);

    viewport() -> setMouseTracking(false);

    //setup signals
    connect( this, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu() ) );
    connect( UMLApp::app(), SIGNAL( sigCutSuccessful() ),
             this, SLOT( slotCutSuccessful() ) );

    // Create the ToolBarState factory. This class is not a singleton, because it
    // needs a pointer to this object.
    m_pToolBarStateFactory = new ToolBarStateFactory(this);
    m_pToolBarState = m_pToolBarStateFactory->getState(WorkToolBar::tbb_Arrow);

}

UMLView::~UMLView() {
    delete m_pImageExporter;

    if(m_pIDChangesLog) {
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
    blockSignals( true );
    removeAllWidgets();

    delete m_pToolBarStateFactory;
    m_pToolBarStateFactory = NULL;

    // Qt Doc for QCanvasView::~QCanvasView () states:
    // "Destroys the canvas view. The associated canvas is not deleted."
    // we should do it now
    delete canvas();
}

QString UMLView::getName() const {
    return m_Name;
}

void UMLView::setName(const QString &name) {
    m_Name = name;
}

int UMLView::generateCollaborationId() {
    return ++m_nCollaborationId;
}

void UMLView::print(KPrinter *pPrinter, QPainter & pPainter) {
    int height, width;
    //get the size of the page
    pPrinter->setFullPage( true );
    QPaintDeviceMetrics metrics(pPrinter);
    QFontMetrics fm = pPainter.fontMetrics(); // use the painter font metrics, not the screen fm!
    int fontHeight  = fm.lineSpacing();
    uint left, right, top, bottom;
    // fetch printer margins individual for all four page sides, as at least top and bottom are not the same
    pPrinter->margins ( &top, &left, &bottom, &right );
    // give a little extra space at each side
    left += 2;
    right += 2;
    top += 2;
    bottom += 2;

    if(pPrinter->orientation() == KPrinter::Landscape) {
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

    width = metrics.width() - left - right;
    height = metrics.height() - top - bottom;

    //get the smallest rect holding the diagram
    QRect rect = getDiagramRect();
    //now draw to printer

#if 0
    int offsetX = 0, offsetY = 0, widthX = 0, heightY = 0;
    // respect the margin
    pPainter.translate(marginX, marginY);

    // clip away everything outside of the margin
    pPainter.setClipRect(marginX, marginY,
                         width, metrics.height() - marginY * 2);

    //loop until all of the picture is printed
    int numPagesX = (int)ceil((double)rect.width()/(double)width);
    int numPagesY = (int)ceil((double)rect.height()/(double)height);
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
            pPainter.translate(-offsetX,-offsetY);
            getDiagram(QRect(offsetX, offsetY,widthX, heightY),
                       pPainter);
            // undo the translation so the coordinates for the painter
            // correspond to the page again
            pPainter.translate(offsetX,offsetY);

            //draw foot note
            QString string = i18n("Diagram: %2 Page %1").arg(page + 1).arg(getName());
            QColor textColor(50, 50, 50);
            pPainter.setPen(textColor);
            pPainter.drawLine(0, height + 2, width, height + 2);
            pPainter.drawText(0, height + 4, width, fontHeight, Qt::AlignLeft, string);

            if(pageX+1 < numPagesX || pageY+1 < numPagesY) {
                pPrinter -> newPage();
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
    int footHeight = (2*fontHeight) + 7;
    int footTop    = rect.y() + diagramHeight  + 4+fontHeight;
    int drawHeight = diagramHeight  + footHeight;

    // set window of painter to dimensions of diagram
    // set window to viewport relation so that x:y isn't changed
    double dScaleX = (double)rect.width()/ (double)width;
    double dScaleY = (double)drawHeight/ (double)height;
    // select the scaling factor so that the larger dimension
    // fits on the printer page -> use the larger scaling factor
    // -> the virtual diagram window has some additional space at the
    // shorter dimension
    double dScaleUse = ( dScaleX > dScaleY )?dScaleX:dScaleY;

    int windowWidth  = (int)ceil(dScaleUse*width);
    int windowHeight = (int)ceil(dScaleUse*height);
#ifdef DEBUG_PRINTING
    kDebug() << "drawHeight: " << drawHeight << ", width: " << rect.width()
              << "\nPageHeight: " << height << ", PageWidht: " << width
              << "\nScaleY: " << dScaleY << ", ScaleX: " << dScaleX
              << "\ndScaleUse: " << dScaleUse
              << "\nVirtualSize: Width: " << windowWidth << ", Height: " << windowHeight
              << "\nFoot Top: " << footTop
              << endl;
#endif
    // set virtual drawing area window - where diagram fits 100% in
    pPainter.setWindow( rect.x(), rect.y(), windowWidth, windowHeight );

    // set viewport - the physical mapping
    // --> Qt's QPainter will map all drawed elements from diagram area ( window )
    //     to printer area ( viewport )
    pPainter.setViewport( left, top, width, height );

    // get Diagram
    getDiagram(QRect(rect.x(), rect.y(), windowWidth, diagramHeight), pPainter);

    //draw foot note
    QString string = i18n("Diagram: %2 Page %1").arg( 1).arg(getName());
    QColor textColor(50, 50, 50);
    pPainter.setPen(textColor);
    pPainter.drawLine(rect.x(), footTop    , windowWidth, footTop);
    pPainter.drawText(rect.x(), footTop + 3, windowWidth, fontHeight, Qt::AlignLeft, string);

    // now restore scaling
    pPainter.restore();

#endif
    // next painting will most probably be to a different device (i.e. the screen)
    forceUpdateWidgetFontMetrics(0);
}

void UMLView::setupNewWidget(UMLWidget *w) {
    w->setX( m_Pos.x() );
    w->setY( m_Pos.y() );
    w->setVisible( true );
    w->setActivated();
    w->setFont( getFont() );
    w->slotColorChanged( getID() );
    w->slotLineWidthChanged( getID() );
    resizeCanvasToItems();
    m_WidgetList.append( w );
    m_pDoc->setModified();
}

void UMLView::contentsMouseReleaseEvent(QMouseEvent* ome) {
    m_pToolBarState->mouseRelease(ome);
}

void UMLView::slotToolBarChanged(int c) {
    m_pToolBarState->cleanBeforeChange();
    m_pToolBarState = m_pToolBarStateFactory->getState((WorkToolBar::ToolBar_Buttons)c);
    m_pToolBarState->init();

    m_bPaste = false;
}

void UMLView::showEvent(QShowEvent* /*se*/) {

# ifdef MANUAL_CONTROL_DOUBLE_BUFFERING
    //kWarning() << "Show Event for " << getName() << endl;
    canvas()->setDoubleBuffering( true );
    // as the diagram gets now visible again,
    // the update of the diagram elements shall be
    // at the normal value of 20
    canvas()-> setUpdatePeriod( 20 );
# endif

    UMLApp* theApp = UMLApp::app();
    WorkToolBar* tb = theApp->getWorkToolBar();
    connect(tb,SIGNAL(sigButtonChanged(int)), this, SLOT(slotToolBarChanged(int)));
    connect(this,SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));
    connect(m_pDoc, SIGNAL(sigObjectCreated(UMLObject *)),
            this, SLOT(slotObjectCreated(UMLObject *)));
    connect(this, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
            UMLApp::app()->getDocWindow(), SLOT(slotAssociationRemoved(AssociationWidget*)));
    connect(this, SIGNAL(sigWidgetRemoved(UMLWidget*)),
            UMLApp::app()->getDocWindow(), SLOT(slotWidgetRemoved(UMLWidget*)));
    resetToolbar();

}

void UMLView::hideEvent(QHideEvent* /*he*/) {
    UMLApp* theApp = UMLApp::app();
    WorkToolBar* tb = theApp->getWorkToolBar();
    disconnect(tb,SIGNAL(sigButtonChanged(int)), this, SLOT(slotToolBarChanged(int)));
    disconnect(this,SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));
    disconnect(m_pDoc, SIGNAL(sigObjectCreated(UMLObject *)), this, SLOT(slotObjectCreated(UMLObject *)));
    disconnect(this, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
               UMLApp::app()->getDocWindow(), SLOT(slotAssociationRemoved(AssociationWidget*)));
    disconnect(this, SIGNAL(sigWidgetRemoved(UMLWidget*)),
               UMLApp::app()->getDocWindow(), SLOT(slotWidgetRemoved(UMLWidget*)));

# ifdef MANUAL_CONTROL_DOUBLE_BUFFERING
    //kWarning() << "Hide Event for " << getName() << endl;
    canvas()->setDoubleBuffering( false );
    // a periodic update of all - also invisible - diagrams
    // can cause a very high CPU load if more than 100diagrams
    // are inside a project - and this without any need
    // => switch the update off for hidden diagrams
    canvas()-> setUpdatePeriod( -1 );
# endif
}

void UMLView::slotObjectCreated(UMLObject* o) {
    m_bPaste = false;
    //check to see if we want the message
    //may be wanted by someone else e.g. list view
    if (!m_bCreateObject)  {
        return;
    }

    UMLWidget* newWidget = Widget_Factory::createWidget(this, o);
    if (newWidget == NULL)
        return;
    newWidget->setVisible( true );
    newWidget->setActivated();
    newWidget->setFont( getFont() );
    newWidget->slotColorChanged( getID() );
    newWidget->slotLineWidthChanged( getID() );
    newWidget->updateComponentSize();
    if (m_Type == Uml::dt_Sequence) {
        // Set proper position on the sequence line widget which is
        // attached to the object widget.
        ObjectWidget *ow = dynamic_cast<ObjectWidget*>(newWidget);
        if (ow)
            ow->moveEvent(NULL);
    }
    m_bCreateObject = false;
    m_WidgetList.append(newWidget);
    switch (o->getBaseType()) {
    case ot_Actor:
    case ot_UseCase:
    case ot_Class:
    case ot_Package:
    case ot_Component:
    case ot_Node:
    case ot_Artifact:
    case ot_Interface:
    case ot_Enum:
    case ot_Entity:
    case ot_Datatype:
        createAutoAssociations(newWidget);
        // We need to invoke createAutoAttributeAssociations()
        // on all other widgets again because the newly created
        // widget might saturate some latent attribute assocs.
        for (UMLWidgetListIt it(m_WidgetList); it.current(); ++it) {
            UMLWidget *w = it.current();
            if (w != newWidget)
                createAutoAttributeAssociations(w);
        }
        break;
    default:
        break;
    }
    resizeCanvasToItems();
}

void UMLView::slotObjectRemoved(UMLObject * o) {
    m_bPaste = false;
    Uml::IDType id = o->getID();
    UMLWidgetListIt it( m_WidgetList );
    UMLWidget *obj;

    while ((obj = it.current()) != 0 ) {
        ++it;
        if(obj -> getID() != id)
            continue;
        removeWidget(obj);
    }
}

void UMLView::contentsDragEnterEvent(QDragEnterEvent *e) {
    UMLDrag::LvTypeAndID_List tidList;
    if(!UMLDrag::getClip3TypeAndID(e, tidList)) {
        return;
    }
    UMLDrag::LvTypeAndID_It tidIt(tidList);
    UMLDrag::LvTypeAndID * tid = tidIt.current();
    if (!tid) {
        kDebug() << "UMLView::contentsDragEnterEvent: "
                  << "UMLDrag::getClip3TypeAndID returned empty list" << endl;
        return;
    }
    ListView_Type lvtype = tid->type;
    Uml::IDType id = tid->id;

    Diagram_Type diagramType = getType();

    UMLObject* temp = 0;
    //if dragging diagram - might be a drag-to-note
    if (Model_Utils::typeIsDiagram(lvtype)) {
        e->accept(true);
        return;
    }
    //can't drag anything onto state/activity diagrams
    if( diagramType == dt_State || diagramType == dt_Activity) {
        e->accept(false);
        return;
    }
    //make sure can find UMLObject
    if( !(temp = m_pDoc->findObjectById(id) ) ) {
        kDebug() << "object " << ID2STR(id) << " not found" << endl;
        e->accept(false);
        return;
    }
    //make sure dragging item onto correct diagram
    // concept - class,seq,coll diagram
    // actor,usecase - usecase diagram
    Object_Type ot = temp->getBaseType();
    bool bAccept = true;
    switch (diagramType) {
        case dt_UseCase:
            if ((widgetOnDiagram(id) && ot == ot_Actor) ||
                (ot != ot_Actor && ot != ot_UseCase))
                bAccept = false;
            break;
        case dt_Class:
            if (widgetOnDiagram(id) ||
                (ot != ot_Class &&
                 ot != ot_Package &&
                 ot != ot_Interface &&
                 ot != ot_Enum &&
                 ot != ot_Datatype)) {
                bAccept = false;
            }
            break;
        case dt_Sequence:
        case dt_Collaboration:
            if (ot != ot_Class &&
                ot != ot_Interface &&
                ot != ot_Actor)
                bAccept = false;
            break;
        case dt_Deployment:
            if (widgetOnDiagram(id))
                bAccept = false;
            else if (ot != ot_Interface &&
                     ot != ot_Package &&
                     ot != ot_Component &&
                     ot != ot_Class &&
                     ot != ot_Node)
                bAccept = false;
            else if (ot == ot_Package &&
                     temp->getStereotype() != "subsystem")
                bAccept = false;
            break;
        case dt_Component:
            if (widgetOnDiagram(id) ||
                (ot != ot_Interface &&
                 ot != ot_Package &&
                 ot != ot_Component &&
                 ot != ot_Artifact &&
                 ot != ot_Class))
                bAccept = false;
            if (ot == ot_Class && !temp->getAbstract())
                bAccept = false;
            break;
        case dt_EntityRelationship:
            if (ot != ot_Entity)
                bAccept = false;
            break;
        default:
            break;
    }
    e->accept(bAccept);
}

void UMLView::contentsDropEvent(QDropEvent *e) {
    UMLDrag::LvTypeAndID_List tidList;
    if( !UMLDrag::getClip3TypeAndID(e, tidList) ) {
        return;
    }
    UMLDrag::LvTypeAndID_It tidIt(tidList);
    UMLDrag::LvTypeAndID * tid = tidIt.current();
    if (!tid) {
        kDebug() << "UMLView::contentsDropEvent: "
                  << "UMLDrag::getClip3TypeAndID returned empty list" << endl;
        return;
    }
    ListView_Type lvtype = tid->type;
    Uml::IDType id = tid->id;

    if (Model_Utils::typeIsDiagram(lvtype)) {
        UMLWidget *w = NULL;
        for (w = m_WidgetList.first(); w; w = m_WidgetList.next()) {
            if (w->getBaseType() == Uml::wt_Note && w->onWidget(e->pos()))
                break;
        }
        if (w) {
            NoteWidget *note = static_cast<NoteWidget*>(w);
            note->setDiagramLink(id);
        }
        return;
    }
    UMLObject* o = m_pDoc->findObjectById(id);
    if( !o ) {
        kDebug() << "UMLView::contentsDropEvent: object id=" << ID2STR(id)
                  << " not found" << endl;
        return;
    }
    m_bCreateObject = true;
    m_Pos = (e->pos() * 100 ) / m_nZoom;

    slotObjectCreated(o);

    m_pDoc -> setModified(true);
}

ObjectWidget * UMLView::onWidgetLine( const QPoint &point ) {
    UMLWidget *obj;
    for (UMLWidgetListIt it(m_WidgetList); (obj = it.current()) != NULL; ++it) {
        ObjectWidget *ow = dynamic_cast<ObjectWidget*>(obj);
        if (ow == NULL)
            continue;
        SeqLineWidget *pLine = ow->getSeqLine();
        if (pLine == NULL) {
            kError() << "UMLView::onWidgetLine: SeqLineWidget of " << ow->getName()
                << " (id=" << ID2STR(ow->getLocalID()) << ") is NULL" << endl;
            continue;
        }
        if (pLine->onWidget(point))
            return ow;
    }
    return 0;
}

UMLWidget *UMLView::getWidgetAt(const QPoint& p) {
    int relativeSize = 10000;  // start with an arbitrary large number
    UMLWidget *obj, *retObj = NULL;
    UMLWidgetListIt it(m_WidgetList);
    for (UMLWidgetListIt it(m_WidgetList); (obj = it.current()) != NULL; ++it) {
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

void UMLView::checkMessages(ObjectWidget * w) {
    if(getType() != dt_Sequence)
        return;

    MessageWidgetListIt it( m_MessageList );
    MessageWidget *obj;
    while ( (obj = it.current()) != 0 ) {
        ++it;
        if(! obj -> contains(w))
            continue;
        //make sure message doesn't have any associations
        removeAssociations(obj);
        obj -> cleanup();
        //make sure not in selected list
        m_SelectedList.remove(obj);
        m_MessageList.remove(obj);
        delete obj;
    }
}

bool UMLView::widgetOnDiagram(Uml::IDType id) {
    UMLWidget *obj;

    UMLWidgetListIt it( m_WidgetList );
    while ( (obj = it.current()) != 0 ) {
        ++it;
        if(id == obj -> getID())
            return true;
    }

    MessageWidgetListIt mit( m_MessageList );
    while ( (obj = (UMLWidget*)mit.current()) != 0 ) {
        ++mit;
        if(id == obj -> getID())
            return true;
    }

    return false;
}

void UMLView::contentsMouseMoveEvent(QMouseEvent* ome) {
    m_pToolBarState->mouseMove(ome);
}

// search both our UMLWidget AND MessageWidget lists
UMLWidget * UMLView::findWidget( Uml::IDType id ) {

    UMLWidgetListIt it( m_WidgetList );
    UMLWidget * obj = NULL;
    while ( (obj = it.current()) != 0 ) {
        ++it;
        // object widgets are special..the widget id is held by 'localId' attribute (crappy!)
        if( obj -> getBaseType() == wt_Object ) {
            if( static_cast<ObjectWidget *>( obj ) -> getLocalID() == id )
                return obj;
        } else if( obj -> getID() == id ) {
            return obj;
        }
    }

    MessageWidgetListIt mit( m_MessageList );
    while ( (obj = (UMLWidget*)mit.current()) != 0 ) {
        ++mit;
        if( obj -> getID() == id )
            return obj;
    }

    return 0;
}



AssociationWidget * UMLView::findAssocWidget( Uml::IDType id ) {
    AssociationWidget *obj;
    AssociationWidgetListIt it( m_AssociationList );
    while ( (obj = it.current()) != 0 ) {
        ++it;
        UMLAssociation* umlassoc = obj -> getAssociation();
        if ( umlassoc && umlassoc->getID() == id ) {
            return obj;
        }
    }
    return 0;
}

AssociationWidget * UMLView::findAssocWidget(UMLWidget *pWidgetA,
                                             UMLWidget *pWidgetB, const QString& roleNameB) {
    AssociationWidget *assoc;
    AssociationWidgetListIt it(m_AssociationList);
    while ((assoc = it.current()) != 0) {
        ++it;
        const Association_Type testType = assoc->getAssocType();
        if (testType != Uml::at_Association &&
            testType != Uml::at_UniAssociation &&
            testType != Uml::at_Composition &&
            testType != Uml::at_Aggregation)
            continue;
        if (pWidgetA->getID() == assoc->getWidgetID(A) &&
            pWidgetB->getID() == assoc->getWidgetID(B) &&
            assoc->getRoleName(Uml::B) == roleNameB)
            return assoc;
    }
    return 0;
}


AssociationWidget * UMLView::findAssocWidget(Uml::Association_Type at,
        UMLWidget *pWidgetA, UMLWidget *pWidgetB) {
    AssociationWidget *assoc;
    AssociationWidgetListIt it(m_AssociationList);
    while ((assoc = it.current()) != 0) {
        ++it;
        Association_Type testType = assoc->getAssocType();
        if (testType != at)
            continue;
        if (pWidgetA->getID() == assoc->getWidgetID(A) &&
                pWidgetB->getID() == assoc->getWidgetID(B))
            return assoc;
    }
    return 0;
}

void UMLView::removeWidget(UMLWidget * o) {
    if(!o)
        return;

    emit sigWidgetRemoved(o);

    removeAssociations(o);

    Widget_Type t = o->getBaseType();
    if(getType() == dt_Sequence && t == wt_Object)
        checkMessages( static_cast<ObjectWidget*>(o) );

    o -> cleanup();
    m_SelectedList.remove(o);
    disconnect( this, SIGNAL( sigRemovePopupMenu() ), o, SLOT( slotRemovePopupMenu() ) );
    disconnect( this, SIGNAL( sigClearAllSelected() ), o, SLOT( slotClearAllSelected() ) );
    disconnect( this, SIGNAL(sigColorChanged(Uml::IDType)), o, SLOT(slotColorChanged(Uml::IDType)));
    if (t == wt_Message)
        m_MessageList.remove(static_cast<MessageWidget*>(o));
    else
        m_WidgetList.remove(o);
    m_pDoc->setModified();
    delete o;
}

bool UMLView::getUseFillColor() const {
    return m_Options.uiState.useFillColor;
}

void UMLView::setUseFillColor(bool ufc) {
    m_Options.uiState.useFillColor = ufc;
}

QColor UMLView::getFillColor() const {
    return m_Options.uiState.fillColor;
}

void UMLView::setFillColor(const QColor &color) {
    m_Options.uiState.fillColor = color;
    emit sigColorChanged( getID() );
    canvas()->setAllChanged();
}

QColor UMLView::getLineColor() const {
    return m_Options.uiState.lineColor;
}

void UMLView::setLineColor(const QColor &color) {
    m_Options.uiState.lineColor = color;
    emit sigColorChanged( getID() );
    canvas() -> setAllChanged();
}

uint UMLView::getLineWidth() const {
    return m_Options.uiState.lineWidth;
}

void UMLView::setLineWidth(uint width) {
    m_Options.uiState.lineWidth = width;
    emit sigLineWidthChanged( getID() );
    canvas() -> setAllChanged();
}

void UMLView::contentsMouseDoubleClickEvent(QMouseEvent* ome) {
    m_pToolBarState->mouseDoubleClick(ome);
}

QRect UMLView::getDiagramRect() {
    int startx, starty, endx, endy;
    startx = starty = INT_MAX;
    endx = endy = 0;
    UMLWidgetListIt it( m_WidgetList );
    UMLWidget *obj;
    while ( (obj = it.current()) != 0 ) {
        ++it;
        if (! obj->isVisible())
            continue;
        int objEndX = obj -> getX() + obj -> getWidth();
        int objEndY = obj -> getY() + obj -> getHeight();
        int objStartX = obj -> getX();
        int objStartY = obj -> getY();
        if (startx >= objStartX)
            startx = objStartX;
        if (starty >= objStartY)
            starty = objStartY;
        if(endx <= objEndX)
            endx = objEndX;
        if(endy <= objEndY)
            endy = objEndY;
    }
    //if seq. diagram, make sure print all of the lines
    if (getType() == dt_Sequence ) {
        for (UMLWidgetListIt it(m_WidgetList); (obj = it.current()) != NULL; ++it) {
            ObjectWidget *ow = dynamic_cast<ObjectWidget*>(obj);
            if (ow == NULL)
                continue;
            int y = ow->getEndLineY();
            if (endy < y)
                endy = y;
        }
    }

    /* now we need another look at the associations, because they are no
     * UMLWidgets */
    AssociationWidgetListIt assoc_it (m_AssociationList);
    AssociationWidget * assoc_obj;
    QRect rect;

    while ((assoc_obj = assoc_it.current()) != 0)
    {
        /* get the rectangle around all segments of the assoc */
        rect = assoc_obj->getAssocLineRectangle();

        if (startx >= rect.x())
            startx = rect.x();
        if (starty >= rect.y())
            starty = rect.y();
        if (endx <= rect.x() + rect.width())
            endx = rect.x() + rect.width();
        if (endy <= rect.y() + rect.height())
            endy = rect.y() + rect.height();
        ++assoc_it; // next assoc
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

void UMLView::setSelected(UMLWidget * w, QMouseEvent * /*me*/) {
    //only add if wasn't in list
    if(!m_SelectedList.remove(w))
        m_SelectedList.append(w);
    int count = m_SelectedList.count();
    //only call once - if we select more, no need to keep clearing  window

    // if count == 1, widget will update the doc window with their data when selected
    if( count == 2 )
        updateDocumentation( true );//clear doc window

    /* selection changed, we have to make sure the copy and paste items
     * are correctly enabled/disabled */
    UMLApp::app()->slotCopyChanged();
}

void UMLView::clearSelected() {
    m_SelectedList.clear();
    emit sigClearAllSelected();
    //m_pDoc -> enableCutCopy(false);
}

//TODO Only used in MLApp::handleCursorKeyReleaseEvent
void UMLView::moveSelectedBy(int dX, int dY) {
    for (UMLWidget *w = m_SelectedList.first(); w; w = m_SelectedList.next())
        w->moveBy(dX, dY);
}

void UMLView::selectionUseFillColor(bool useFC) {
    UMLWidget * temp = 0;
    for(temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next())
        temp -> setUseFillColour(useFC);
}

void UMLView::selectionSetFont( const QFont &font )
{
    UMLWidget * temp = 0;
    for(temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next())
        temp -> setFont( font );
}

void UMLView::selectionSetLineColor( const QColor &color )
{
    UMLWidget * temp = 0;
    for (temp = m_SelectedList.first(); temp; temp = m_SelectedList.next()) {
        temp->setLineColor(color);
        temp->setUsesDiagramLineColour(false);
    }
    AssociationWidgetList assoclist = getSelectedAssocs();
    for (AssociationWidget *aw = assoclist.first(); aw; aw = assoclist.next()) {
        aw->setLineColor(color);
        aw->setUsesDiagramLineColour(false);
    }
}

void UMLView::selectionSetLineWidth( uint width )
{
    UMLWidget * temp = 0;
    for (temp = m_SelectedList.first(); temp; temp = m_SelectedList.next()) {
        temp->setLineWidth(width);
        temp->setUsesDiagramLineWidth(false);
    }
    AssociationWidgetList assoclist = getSelectedAssocs();
    for (AssociationWidget *aw = assoclist.first(); aw; aw = assoclist.next()) {
        aw->setLineWidth(width);
        aw->setUsesDiagramLineWidth(false);
    }
}

void UMLView::selectionSetFillColor( const QColor &color )
{
    UMLWidget * temp = 0;
    for(temp=(UMLWidget *) m_SelectedList.first();
            temp;
            temp=(UMLWidget *)m_SelectedList.next()) {
        temp -> setFillColour( color );
        temp -> setUsesDiagramFillColour(false);
    }
}

void UMLView::selectionToggleShow(int sel)
{
    // loop through all selected items
    for(UMLWidget *temp = (UMLWidget *)m_SelectedList.first();
            temp; temp=(UMLWidget *)m_SelectedList.next()) {
        Widget_Type type = temp->getBaseType();
        ClassifierWidget *cw = dynamic_cast<ClassifierWidget*>(temp);

        // toggle the show setting sel
        switch (sel)
        {
            // some setting are only available for class, some for interface and some
            // for both
        case ListPopupMenu::mt_Show_Attributes_Selection:
            if (type == wt_Class)
                cw -> toggleShowAtts();
            break;
        case ListPopupMenu::mt_Show_Operations_Selection:
            if (cw)
                cw -> toggleShowOps();
            break;
        case ListPopupMenu::mt_Visibility_Selection:
            if (cw)
                cw -> toggleShowVisibility();
            break;
        case ListPopupMenu::mt_DrawAsCircle_Selection:
            if (type == wt_Interface)
                cw -> toggleDrawAsCircle();
            break;
        case ListPopupMenu::mt_Show_Operation_Signature_Selection:
            if (cw)
                cw -> toggleShowOpSigs();
            break;
        case ListPopupMenu::mt_Show_Attribute_Signature_Selection:
            if (type == wt_Class)
                cw -> toggleShowAttSigs();
            break;
        case ListPopupMenu::mt_Show_Packages_Selection:
            if (cw)
                cw -> toggleShowPackage();
            break;
        case ListPopupMenu::mt_Show_Stereotypes_Selection:
            if (type == wt_Class)
                cw -> toggleShowStereotype();
            break;
        case ListPopupMenu::mt_Show_Public_Only_Selection:
            if (cw)
                cw -> toggleShowPublicOnly();
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
    UMLWidget * temp = 0;
    for(temp=(UMLWidget *) m_SelectedList.first();
            temp;
            temp=(UMLWidget *)m_SelectedList.next())
    {
        if( temp -> getBaseType() == wt_Text &&
                ((FloatingTextWidget *)temp) -> getRole() != tr_Floating )
        {
            m_SelectedList.remove(); // remove advances the iterator to the next position,
            m_SelectedList.prev();      // let's allow for statement do the advancing
            temp -> hide();
        } else {
            removeWidget(temp);
        }
    }

    // Delete any selected associations.
    AssociationWidgetListIt assoc_it( m_AssociationList );
    AssociationWidget* assocwidget = 0;
    while((assocwidget=assoc_it.current())) {
        ++assoc_it;
        if( assocwidget-> getSelected() )
            removeAssoc(assocwidget);
        // MARK
    }

    /* we also have to remove selected messages from sequence diagrams */
    MessageWidget * cur_msgWgt;

    /* loop through all messages and check the selection state */
    for (cur_msgWgt = m_MessageList.first(); cur_msgWgt;
            cur_msgWgt = m_MessageList.next())
    {
        if (cur_msgWgt->getSelected() == true)
        {
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

Uml::IDType UMLView::getLocalID() {
    m_nLocalID = UniqueID::gen();
    return m_nLocalID;
}

bool UMLView::isSavedInSeparateFile() {
    if (getOptionState().generalState.tabdiagrams) {
        // Umbrello currently does not support external folders
        // when tabbed diagrams are enabled.
        return false;
    }
    const QString msgPrefix("UMLView::isSavedInSeparateFile(" + getName() + "): ");
    UMLListView *listView = UMLApp::app()->getListView();
    UMLListViewItem *lvItem = listView->findItem(m_nID);
    if (lvItem == NULL) {
        kError() << msgPrefix
                  << "listView->findUMLObject(this) returns false" << endl;
        return false;
    }
    UMLListViewItem *parentItem = dynamic_cast<UMLListViewItem*>( lvItem->parent() );
    if (parentItem == NULL) {
        kError() << msgPrefix
                  << "parent item in listview is not a UMLListViewItem (?)" << endl;
        return false;
    }
    const Uml::ListView_Type lvt = parentItem->getType();
    if (! Model_Utils::typeIsFolder(lvt))
        return false;
    UMLFolder *modelFolder = dynamic_cast<UMLFolder*>(parentItem->getUMLObject());
    if (modelFolder == NULL) {
        kError() << msgPrefix
                  << "parent model object is not a UMLFolder (?)" << endl;
        return false;
    }
    QString folderFile = modelFolder->getFolderFile();
    return !folderFile.isEmpty();
}

void UMLView::contentsMousePressEvent(QMouseEvent* ome) {
    m_pToolBarState->mousePress(ome);
    //TODO should be managed by widgets when are selected. Right now also has some
    //problems, such as clicking on a widget, and clicking to move that widget shows
    //documentation of the diagram instead of keeping the widget documentation.
    //When should diagram documentation be shown? When clicking on an empty
    //space in the diagram with arrow tool?
    if (!m_bChildDisplayedDoc) {
      UMLApp::app() -> getDocWindow() -> showDocumentation( this, true );
    }
    m_bChildDisplayedDoc = false;
}

void UMLView::makeSelected (UMLWidget * uw) {
    if (uw == NULL)
        return;
    uw -> setSelected(true);
    m_SelectedList.remove(uw);  // make sure not in there
    m_SelectedList.append(uw);
}

void UMLView::selectWidgetsOfAssoc (AssociationWidget * a) {
    if (!a)
        return;
    a -> setSelected(true);
    //select the two widgets
    makeSelected( a->getWidget(A) );
    makeSelected( a->getWidget(B) );
    //select all the text
    makeSelected( a->getMultiWidget(A) );
    makeSelected( a->getMultiWidget(B) );
    makeSelected( a->getRoleWidget(A) );
    makeSelected( a->getRoleWidget(B) );
    makeSelected( a->getChangeWidget(A) );
    makeSelected( a->getChangeWidget(B) );
}

void UMLView::selectWidgets(int px, int py, int qx, int qy) {
    clearSelected();

    QRect rect;
    if(px <= qx) {
        rect.setLeft(px);
        rect.setRight(qx);
    } else {
        rect.setLeft(qx);
        rect.setRight(px);
    }
    if(py <= qy) {
        rect.setTop(py);
        rect.setBottom(qy);
    } else {
        rect.setTop(qy);
        rect.setBottom(py);
    }
    UMLWidgetListIt it(m_WidgetList);
    UMLWidget * temp = NULL;
    while ( (temp = it.current()) != 0 ) {
        int x = temp -> getX();
        int y = temp -> getY();
        int w = temp -> getWidth();
        int h = temp -> getHeight();
        QRect rect2(x, y, w, h);
        ++it;
        //see if any part of widget is in the rectangle
        if( !rect.intersects(rect2) )
            continue;
        //if it is text that is part of an association then select the association
        //and the objects that are connected to it.
        if (temp -> getBaseType() == wt_Text) {
            FloatingTextWidget *ft = static_cast<FloatingTextWidget*>(temp);
            Text_Role t = ft -> getRole();
            LinkWidget *lw = ft->getLink();
            MessageWidget * mw = dynamic_cast<MessageWidget*>(lw);
            if (mw) {
                makeSelected( mw );
                makeSelected( mw->getWidget(A) );
                makeSelected( mw->getWidget(B) );
            } else if (t != tr_Floating) {
                AssociationWidget * a = dynamic_cast<AssociationWidget*>(lw);
                if (a)
                    selectWidgetsOfAssoc( a );
            }
        } else if(temp -> getBaseType() == wt_Message) {
            MessageWidget *mw = static_cast<MessageWidget*>(temp);
            makeSelected( mw -> getWidget(A) );
            makeSelected( mw -> getWidget(B) );
        }
        if(temp -> isVisible()) {
            makeSelected( temp );
        }
    }
    selectAssociations( true );

    //now do the same for the messagewidgets
    MessageWidgetListIt itw( m_MessageList );
    MessageWidget *w = 0;
    while ( (w = itw.current()) != 0 ) {
        ++itw;
        if ( w -> getWidget(A) -> getSelected() &&
                w -> getWidget(B) -> getSelected() ) {
            makeSelected( w );
        }//end if
    }//end while
}

void  UMLView::getDiagram(const QRect &rect, QPixmap & diagram) {
    QPixmap pixmap(rect.x() + rect.width(), rect.y() + rect.height());
    QPainter painter(&pixmap);
    getDiagram(canvas()->rect(),painter);
    bitBlt(&diagram, QPoint(0, 0), &pixmap, rect);
}

void  UMLView::getDiagram(const QRect &area, QPainter & painter) {
    //TODO unselecting and selecting later doesn't work now as the selection is
    //cleared in UMLViewImageExporter. Check if the anything else than the
    //following is needed and, if it works, remove the clearSelected in
    //UMLViewImageExporter and UMLViewImageExporterModel
    UMLWidget* widget = 0;
    for (widget=(UMLWidget*)m_SelectedList.first(); widget; widget=(UMLWidget*)m_SelectedList.next()) {
        widget->setSelected(false);
    }
    AssociationWidgetList selectedAssociationsList = getSelectedAssocs();
    AssociationWidget* association = 0;
    for (association=selectedAssociationsList.first(); association;
                                association=selectedAssociationsList.next()) {
        association->setSelected(false);
    }

    // we don't want to get the grid
    bool showSnapGrid = getShowSnapGrid();
    setShowSnapGrid(false);

    canvas()->drawArea(area, &painter);

    setShowSnapGrid(showSnapGrid);

    canvas()->setAllChanged();
    //select again
    for (widget=(UMLWidget *)m_SelectedList.first(); widget; widget=(UMLWidget *)m_SelectedList.next()) {
        widget->setSelected( true );
    }
    for (association=selectedAssociationsList.first(); association;
                                association=selectedAssociationsList.next()) {
        association->setSelected(true);
    }

    return;
}

UMLViewImageExporter* UMLView::getImageExporter() {
    return m_pImageExporter;
}

void UMLView::slotActivate() {
    m_pDoc->changeCurrentView(getID());
}

UMLObjectList UMLView::getUMLObjects() {
    UMLObjectList list;
    for (UMLWidgetListIt it(m_WidgetList); it.current(); ++it) {
        UMLWidget *w = it.current();
        switch (w->getBaseType()) //use switch for easy future expansion
        {
        case wt_Actor:
        case wt_Class:
        case wt_Interface:
        case wt_Package:
        case wt_Component:
        case wt_Node:
        case wt_Artifact:
        case wt_UseCase:
        case wt_Object:
            list.append( w->getUMLObject() );
            break;
        default:
            break;
        }
    }
    return list;
}

void UMLView::activate() {
    UMLWidgetListIt it( m_WidgetList );
    UMLWidget *obj;

    //Activate Regular widgets then activate  messages
    while ( (obj = it.current()) != 0 ) {
        ++it;
        //If this UMLWidget is already activated or is a MessageWidget then skip it
        if(obj->isActivated() || obj->getBaseType() == wt_Message)
            continue;

        if (obj->activate()) {
            obj->setVisible(true);
        } else {
            m_WidgetList.remove(obj);
            delete obj;
        }
    }//end while

    MessageWidgetListIt it2( m_MessageList );
    //Activate Message widgets
    while ( (obj = (UMLWidget*)it2.current()) != 0 ) {
        ++it2;
        //If this MessageWidget is already activated then skip it
        if(obj->isActivated())
            continue;

        obj->activate(m_pDoc->getChangeLog());
        obj->setVisible( true );

    }//end while

    // Activate all association widgets
    AssociationWidget *aw;
    for (AssociationWidgetListIt ait(m_AssociationList);
            (aw = ait.current()); ++ait) {
        if (aw->activate()) {
            if (m_PastePoint.x() != 0) {
                int x = m_PastePoint.x() - m_Pos.x();
                int y = m_PastePoint.y() - m_Pos.y();
                aw->moveEntireAssoc(x, y);
            }
        } else {
            m_AssociationList.remove(aw);
        }
    }
}

int UMLView::getSelectCount(bool filterText) const {
    if (!filterText)
        return m_SelectedList.count();
    int counter = 0;
    const UMLWidget * temp = 0;
    for (UMLWidgetListIt iter(m_SelectedList); (temp = iter.current()) != 0; ++iter) {
        if (temp->getBaseType() == wt_Text) {
            const FloatingTextWidget *ft = static_cast<const FloatingTextWidget*>(temp);
            if (ft->getRole() == tr_Floating)
                counter++;
        } else {
            counter++;
        }
    }
    return counter;
}


bool UMLView::getSelectedWidgets(UMLWidgetList &WidgetList, bool filterText /*= true*/) {
    const UMLWidget * temp = 0;
    for (UMLWidgetListIt it(m_SelectedList); (temp = it.current()) != NULL; ++it) {
        if (filterText && temp->getBaseType() == wt_Text) {
            const FloatingTextWidget *ft = static_cast<const FloatingTextWidget*>(temp);
            if (ft->getRole() == tr_Floating)
                WidgetList.append(temp);
        } else {
            WidgetList.append(temp);
        }
    }//end for
    return true;
}

AssociationWidgetList UMLView::getSelectedAssocs() {
    AssociationWidgetList assocWidgetList;
    AssociationWidgetListIt assoc_it( m_AssociationList );
    AssociationWidget* assocwidget = 0;
    while((assocwidget=assoc_it.current())) {
        ++assoc_it;
        if( assocwidget -> getSelected() )
            assocWidgetList.append(assocwidget);
    }
    return assocWidgetList;
}

bool UMLView::addWidget( UMLWidget * pWidget , bool isPasteOperation ) {
    if( !pWidget ) {
        return false;
    }
    Widget_Type type = pWidget->getBaseType();
    if (isPasteOperation) {
        if (type == Uml::wt_Message)
            m_MessageList.append(static_cast<MessageWidget*>(pWidget));
        else
            m_WidgetList.append(pWidget);
        return true;
    }
    if (!isPasteOperation && findWidget(pWidget->getID())) {
        kError() << "UMLView::addWidget: Not adding "
                  << "(id=" << ID2STR(pWidget->getID())
                  << "/type=" << type << "/name=" << pWidget->getName()
                  << ") because it's already there" << endl;
        return false;
    }
    //kDebug() << "UMLView::addWidget called for basetype " << type << endl;
    IDChangeLog * log = m_pDoc -> getChangeLog();
    if( isPasteOperation && (!log || !m_pIDChangesLog)) {
        kError()<<" Cant addWidget to view in paste op because a log is not open"<<endl;
        return false;
    }
    int wX = pWidget -> getX();
    int wY = pWidget -> getY();
    bool xIsOutOfRange = (wX <= 0 || wX >= FloatingTextWidget::restrictPositionMax);
    bool yIsOutOfRange = (wY <= 0 || wY >= FloatingTextWidget::restrictPositionMax);
    if (xIsOutOfRange || yIsOutOfRange) {
        QString name = pWidget->getName();
        if (name.isEmpty()) {
            FloatingTextWidget *ft = dynamic_cast<FloatingTextWidget*>(pWidget);
            if (ft)
                name = ft->getDisplayText();
        }
        kDebug() << "UMLView::addWidget (" << name << " type="
                  << pWidget->getBaseType() << "): position (" << wX << ","
                  << wY << ") is out of range" << endl;
        if (xIsOutOfRange) {
            pWidget->setX(0);
            wX = 0;
        }
        if (yIsOutOfRange) {
            pWidget->setY(0);
            wY = 0;
        }
    }
    if( wX < m_Pos.x() )
        m_Pos.setX( wX );
    if( wY < m_Pos.y() )
        m_Pos.setY( wY );

    //see if we need a new id to match object
    switch( type ) {

    case wt_Class:
    case wt_Package:
    case wt_Component:
    case wt_Node:
    case wt_Artifact:
    case wt_Interface:
    case wt_Enum:
    case wt_Entity:
    case wt_Datatype:
    case wt_Actor:
    case wt_UseCase:
        {
            Uml::IDType id = pWidget -> getID();
            Uml::IDType newID = log->findNewID( id );
            if( newID == Uml::id_None ) {  // happens after a cut
                if (id == Uml::id_None)
                    return false;
                newID = id; //don't stop paste
            } else
                pWidget -> setID( newID );
            UMLObject * pObject = m_pDoc -> findObjectById( newID );
            if( !pObject ) {
                kDebug() << "addWidget: Can't find UMLObject for id "
                          << ID2STR(newID) << endl;
                return false;
            }
            pWidget -> setUMLObject( pObject );
            //make sure it doesn't already exist.
            if (findWidget(newID)) {
                kDebug() << "UMLView::addWidget: Not adding "
                          << "(id=" << ID2STR(pWidget->getID())
                          << "/type=" << pWidget->getBaseType()
                          << "/name=" << pWidget->getName()
                          << ") because it's already there" << endl;
                delete pWidget; // Not nice but if _we_ don't do it nobody else will
                return true;//don't stop paste just because widget found.
            }
            m_WidgetList.append( pWidget );
        }
        break;

    case wt_Message:
    case wt_Note:
    case wt_Box:
    case wt_Text:
    case wt_State:
    case wt_Activity:
        {
            Uml::IDType newID = m_pDoc->assignNewID( pWidget->getID() );
            pWidget->setID(newID);
            if (type != wt_Message) {
                m_WidgetList.append( pWidget );
                return true;
            }
            // CHECK
            // Handling of wt_Message:
            MessageWidget *pMessage = static_cast<MessageWidget *>( pWidget );
            if (pMessage == NULL) {
                kDebug() << "UMLView::addWidget(): pMessage is NULL" << endl;
                return false;
            }
            ObjectWidget *objWidgetA = pMessage -> getWidget(A);
            ObjectWidget *objWidgetB = pMessage -> getWidget(B);
            Uml::IDType waID = objWidgetA -> getLocalID();
            Uml::IDType wbID = objWidgetB -> getLocalID();
            Uml::IDType newWAID = m_pIDChangesLog ->findNewID( waID );
            Uml::IDType newWBID = m_pIDChangesLog ->findNewID( wbID );
            if( newWAID == Uml::id_None || newWBID == Uml::id_None ) {
                kDebug() << "Error with ids : " << ID2STR(newWAID)
                          << " " << ID2STR(newWBID) << endl;
                return false;
            }
            // Assumption here is that the A/B objectwidgets and the textwidget
            // are pristine in the sense that we may freely change their local IDs.
            objWidgetA -> setLocalID( newWAID );
            objWidgetB -> setLocalID( newWBID );
            FloatingTextWidget *ft = pMessage->getFloatingTextWidget();
            if (ft == NULL)
                kDebug() << "UMLView::addWidget: FloatingTextWidget of Message is NULL" << endl;
            else if (ft->getID() == Uml::id_None)
                ft->setID( UniqueID::gen() );
            else {
                Uml::IDType newTextID = m_pDoc->assignNewID( ft->getID() );
                ft->setID( newTextID );
            }
            m_MessageList.append( pMessage );
        }
        break;

    case wt_Object:
        {
            ObjectWidget* pObjectWidget = static_cast<ObjectWidget*>(pWidget);
            if (pObjectWidget == NULL) {
                kDebug() << "UMLView::addWidget(): pObjectWidget is NULL" << endl;
                return false;
            }
            Uml::IDType nNewLocalID = getLocalID();
            Uml::IDType nOldLocalID = pObjectWidget -> getLocalID();
            m_pIDChangesLog->addIDChange( nOldLocalID, nNewLocalID );
            pObjectWidget -> setLocalID( nNewLocalID );
            UMLObject *pObject = m_pDoc->findObjectById(pWidget->getID());
            if( !pObject ) {
                kDebug() << "addWidget::Can't find UMLObject" << endl;
                return false;
            }
            pWidget -> setUMLObject( pObject );
            m_WidgetList.append( pWidget );
        }
        break;

    default:
        kDebug() << "Trying to add an invalid widget type" << endl;
        return false;
        break;
    }

    return true;
}

// Add the association, and its child widgets to this view
bool UMLView::addAssociation(AssociationWidget* pAssoc , bool isPasteOperation) {

    if (!pAssoc) {
        return false;
    }
    const Association_Type type = pAssoc->getAssocType();

    if( isPasteOperation )
    {
        IDChangeLog * log = m_pDoc -> getChangeLog();

        if(!log )
            return false;

        Uml::IDType ida = Uml::id_None, idb = Uml::id_None;
        if( getType() == dt_Collaboration || getType() == dt_Sequence ) {
            //check local log first
            ida = m_pIDChangesLog->findNewID( pAssoc->getWidgetID(A) );
            idb = m_pIDChangesLog->findNewID( pAssoc->getWidgetID(B) );
            //if either is still not found and assoc type is anchor
            //we are probably linking to a notewidet - else an error
            if( ida == Uml::id_None && type == at_Anchor )
                ida = log->findNewID(pAssoc->getWidgetID(A));
            if( idb == Uml::id_None && type == at_Anchor )
                idb = log->findNewID(pAssoc->getWidgetID(B));
        } else {
            Uml::IDType oldIdA = pAssoc->getWidgetID(A);
            Uml::IDType oldIdB = pAssoc->getWidgetID(B);
            ida = log->findNewID( oldIdA );
            if (ida == Uml::id_None) {  // happens after a cut
                if (oldIdA == Uml::id_None)
                    return false;
                ida = oldIdA;
            }
            idb = log->findNewID( oldIdB );
            if (idb == Uml::id_None) {  // happens after a cut
                if (oldIdB == Uml::id_None)
                    return false;
                idb = oldIdB;
            }
        }
        if(ida == Uml::id_None || idb == Uml::id_None) {
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
    if (!isPasteOperation && !m_pDoc->loading() &&
        !AssocRules::allowAssociation(type, pWidgetA, pWidgetB, false)) {
        kWarning() << "UMLView::addAssociation: allowAssociation returns false "
                   << "for AssocType " << type << endl;
        return false;
    }

    //make sure there isn't already the same assoc
    AssociationWidgetListIt assoc_it( m_AssociationList );
    AssociationWidget* assocwidget = 0;
    while((assocwidget=assoc_it.current())) {
        ++assoc_it;
        if( *pAssoc == *assocwidget )
            // this is nuts. Paste operation wants to know if 'true'
            // for duplicate, but loadFromXMI needs 'false' value
            return (isPasteOperation? true: false);
    }

    m_AssociationList.append(pAssoc);

    FloatingTextWidget *ft[5] = { pAssoc->getNameWidget(),
                            pAssoc->getRoleWidget(A),
                            pAssoc->getRoleWidget(B),
                            pAssoc->getMultiWidget(A),
                            pAssoc->getMultiWidget(B) };
    for (int i = 0; i < 5; i++) {
        FloatingTextWidget *flotxt = ft[i];
        if (flotxt) {
            flotxt->updateComponentSize();
            addWidget(flotxt);
        }
    }

    return true;
}

void UMLView::activateAfterLoad(bool bUseLog) {
    if (m_bActivated)
        return;
    if( bUseLog ) {
        beginPartialWidgetPaste();
    }

    //now activate them all
    activate();

    if( bUseLog ) {
        endPartialWidgetPaste();
    }
    resizeCanvasToItems();
    setZoom( getZoom() );
    m_bActivated = true;
}

void UMLView::beginPartialWidgetPaste() {
    delete m_pIDChangesLog;
    m_pIDChangesLog = 0;

    m_pIDChangesLog = new IDChangeLog();
    m_bPaste = true;
}

void UMLView::endPartialWidgetPaste() {
    delete    m_pIDChangesLog;
    m_pIDChangesLog = 0;

    m_bPaste = false;
}

void UMLView::removeAssoc(AssociationWidget* pAssoc) {
    if(!pAssoc)
        return;

    emit sigAssociationRemoved(pAssoc);

    pAssoc->cleanup();
    m_AssociationList.remove(pAssoc); // will delete our association
    m_pDoc->setModified();
}

void UMLView::removeAssocInViewAndDoc(AssociationWidget* a) {
    // For umbrello 1.2, UMLAssociations can only be removed in two ways:
    // 1. Right click on the assocwidget in the view and select Delete
    // 2. Go to the Class Properties page, select Associations, right click
    //    on the association and select Delete
    if(!a)
        return;
    if (a->getAssocType() == at_Containment) {
        UMLObject *objToBeMoved = a->getWidget(B)->getUMLObject();
        if (objToBeMoved != NULL) {
            UMLListView *lv = UMLApp::app()->getListView();
            lv->moveObject( objToBeMoved->getID(),
                            Model_Utils::convert_OT_LVT(objToBeMoved),
                            lv->theLogicalView() );
            // UMLListView::moveObject() will delete the containment
            // AssociationWidget via UMLView::updateContainment().
        } else {
            kDebug() << "removeAssocInViewAndDoc(containment): "
                      << "objB is NULL" << endl;
        }
    } else {
        // Remove assoc in doc.
        m_pDoc->removeAssociation(a->getAssociation());
        // Remove assoc in view.
        removeAssoc(a);
    }
}

/** Removes all the associations related to Widget */
void UMLView::removeAssociations(UMLWidget* Widget) {
    AssociationWidgetListIt assoc_it(m_AssociationList);
    AssociationWidget* assocwidget = 0;
    while((assocwidget=assoc_it.current())) {
        ++assoc_it;
        if(assocwidget->contains(Widget)) {
            removeAssoc(assocwidget);
        }
    }
}

void UMLView::selectAssociations(bool bSelect) {
    AssociationWidgetListIt assoc_it(m_AssociationList);
    AssociationWidget* assocwidget = 0;
    while((assocwidget=assoc_it.current())) {
        ++assoc_it;
        if(bSelect &&
                assocwidget->getWidget(A) && assocwidget->getWidget(A)->getSelected() &&
                assocwidget->getWidget(B) && assocwidget->getWidget(B)->getSelected() ) {
            assocwidget->setSelected(true);
        } else {
            assocwidget->setSelected(false);
        }
    }//end while
}

void UMLView::getWidgetAssocs(UMLObject* Obj, AssociationWidgetList & Associations) {
    if( ! Obj )
        return;

    AssociationWidgetListIt assoc_it(m_AssociationList);
    AssociationWidget * assocwidget;
    while((assocwidget = assoc_it.current())) {
        if (assocwidget->getWidget(A)->getUMLObject() == Obj ||
                assocwidget->getWidget(B)->getUMLObject() == Obj)
            Associations.append(assocwidget);
        ++assoc_it;
    }//end while
}

void UMLView::closeEvent ( QCloseEvent * e ) {
    QWidget::closeEvent(e);
}

void UMLView::removeAllAssociations() {
    //Remove All association widgets
    AssociationWidgetListIt assoc_it(m_AssociationList);
    AssociationWidget* assocwidget = 0;
    while((assocwidget=assoc_it.current()))
    {
        ++assoc_it;
        removeAssoc(assocwidget);
    }
    m_AssociationList.clear();
}


void UMLView::removeAllWidgets() {
    // Remove widgets.
    UMLWidgetListIt it( m_WidgetList );
    UMLWidget * temp = 0;
    while ( (temp = it.current()) != 0 ) {
        ++it;
        // I had to take this condition back in, else umbrello
        // crashes on exit. Still to be analyzed.  --okellogg
        if( !( temp -> getBaseType() == wt_Text &&
                ((FloatingTextWidget *)temp)-> getRole() != tr_Floating ) ) {
            removeWidget( temp );
        }
    }
    m_WidgetList.clear();
}

void UMLView::showDocumentation( UMLObject * object, bool overwrite ) {
    UMLApp::app() -> getDocWindow() -> showDocumentation( object, overwrite );
    m_bChildDisplayedDoc = true;
}

void UMLView::showDocumentation( UMLWidget * widget, bool overwrite ) {
    UMLApp::app() -> getDocWindow() -> showDocumentation( widget, overwrite );
    m_bChildDisplayedDoc = true;
}

void UMLView::showDocumentation( AssociationWidget * widget, bool overwrite ) {
    UMLApp::app() -> getDocWindow() -> showDocumentation( widget, overwrite );
    m_bChildDisplayedDoc = true;
}

void UMLView::updateDocumentation( bool clear ) {
    UMLApp::app() -> getDocWindow() -> updateDocumentation( clear );
}

void UMLView::updateContainment(UMLCanvasObject *self) {
    if (self == NULL)
        return;
    // See if the object has a widget representation in this view.
    // While we're at it, also see if the new parent has a widget here.
    UMLWidget *selfWidget = NULL, *newParentWidget = NULL;
    UMLPackage *newParent = self->getUMLPackage();
    for (UMLWidgetListIt wit(m_WidgetList); wit.current(); ++wit) {
        UMLWidget *w = wit.current();
        UMLObject *o = w->getUMLObject();
        if (o == self)
            selfWidget = w;
        else if (newParent != NULL && o == newParent)
            newParentWidget = w;
    }
    if (selfWidget == NULL)
        return;
    // Remove possibly obsoleted containment association.
    for (AssociationWidgetListIt it(m_AssociationList); it.current(); ++it) {
        AssociationWidget *a = it.current();
        if (a->getAssocType() != Uml::at_Containment)
            continue;
        // Container is at role A, containee at B.
        // We only look at association for which we are B.
        UMLWidget *wB = a->getWidget(B);
        UMLObject *roleBObj = wB->getUMLObject();
        if (roleBObj != self)
            continue;
        UMLWidget *wA = a->getWidget(A);
        UMLObject *roleAObj = wA->getUMLObject();
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
    AssociationWidget *a = new AssociationWidget(this, newParentWidget,
                           Uml::at_Containment, selfWidget);
    a->calculateEndingPoints();
    a->setActivated(true);
    m_AssociationList.append(a);
}

void UMLView::createAutoAssociations( UMLWidget * widget ) {
    if (widget == NULL ||
        (m_Type != Uml::dt_Class &&
         m_Type != Uml::dt_Component &&
         m_Type != Uml::dt_Deployment &&
         m_Type != Uml::dt_EntityRelationship))
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
    UMLObject *tmpUmlObj = widget->getUMLObject();
    if (tmpUmlObj == NULL)
        return;
    UMLCanvasObject *umlObj = dynamic_cast<UMLCanvasObject*>(tmpUmlObj);
    if (umlObj == NULL)
        return;
    const UMLAssociationList& umlAssocs = umlObj->getAssociations();
    UMLAssociationListIt it(umlAssocs);
    UMLAssociation *assoc = NULL;
    Uml::IDType myID = umlObj->getID();
    while ((assoc = it.current()) != NULL) {
        ++it;
        UMLCanvasObject *other = NULL;
        UMLObject *roleAObj = assoc->getObject(A);
        if (roleAObj == NULL) {
            kDebug() << "createAutoAssociations: roleA object is NULL at UMLAssoc "
                      << ID2STR(assoc->getID()) << endl;
            continue;
        }
        UMLObject *roleBObj = assoc->getObject(B);
        if (roleBObj == NULL) {
            kDebug() << "createAutoAssociations: roleB object is NULL at UMLAssoc "
                      << ID2STR(assoc->getID()) << endl;
            continue;
        }
        if (roleAObj->getID() == myID) {
            other = static_cast<UMLCanvasObject*>(roleBObj);
        } else if (roleBObj->getID() == myID) {
            other = static_cast<UMLCanvasObject*>(roleAObj);
        } else {
            kDebug() << "createAutoAssociations: Can't find own object "
                      << ID2STR(myID) << " in UMLAssoc "
                      << ID2STR(assoc->getID()) << endl;
            continue;
        }
        // Now that we have determined the "other" UMLObject, seek it in
        // this view's UMLWidgets.
        Uml::IDType otherID = other->getID();
        UMLWidget *pOtherWidget;
        UMLWidgetListIt wit(m_WidgetList);
        while ((pOtherWidget = wit.current()) != NULL) {
            ++wit;
            if (pOtherWidget->getID() == otherID)
                break;
        }
        if (pOtherWidget == NULL)
            continue;
        // Both objects are represented in this view:
        // Assign widget roles as indicated by the UMLAssociation.
        UMLWidget *widgetA, *widgetB;
        if (myID == roleAObj->getID()) {
            widgetA = widget;
            widgetB = pOtherWidget;
        } else {
            widgetA = pOtherWidget;
            widgetB = widget;
        }
        // Check that the assocwidget does not already exist.
        Uml::Association_Type assocType = assoc->getAssocType();
        AssociationWidget * assocwidget = findAssocWidget(assocType, widgetA, widgetB);
        if (assocwidget) {
            assocwidget->calculateEndingPoints();  // recompute assoc lines
            continue;
        }
        // Check that the assoc is allowed.
        if (!AssocRules::allowAssociation(assocType, widgetA, widgetB, false)) {
            kDebug() << "createAutoAssociations: not transferring assoc "
                      << "of type " << assocType << endl;
            continue;
        }
        // Create the AssociationWidget.
        assocwidget = new AssociationWidget( this );
        assocwidget->setWidget(widgetA, A);
        assocwidget->setWidget(widgetB, B);
        assocwidget->setAssocType(assocType);
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
    // if this object is capable of containing nested objects then
    Uml::Object_Type t = umlObj->getBaseType();
    if (t == ot_Package || t == ot_Class || t == ot_Interface || t == ot_Component) {
        // for each of the object's containedObjects
        UMLPackage *umlPkg = static_cast<UMLPackage*>(umlObj);
        UMLObjectList lst = umlPkg->containedObjects();
        for (UMLObject *obj = lst.first(); obj; obj = lst.next()) {
            // if the containedObject has a widget representation on this view then
            Uml::IDType id = obj->getID();
            for (UMLWidget *w = m_WidgetList.first(); w; w = m_WidgetList.next()) {
                if (w->getID() != id)
                    continue;
                // if the containedWidget is not physically located inside this widget
                if (widget->rect().contains(w->rect()))
                    continue;
                // create the containment AssocWidget
                AssociationWidget *a = new AssociationWidget(this, widget,
                                       at_Containment, w);
                a->calculateEndingPoints();
                a->setActivated(true);
                if (! addAssociation(a))
                    delete a;
            }
        }
    }
    // if the UMLCanvasObject has a parentPackage then
    UMLPackage *parent = umlObj->getUMLPackage();
    if (parent == NULL)
        return;
    // if the parentPackage has a widget representation on this view then
    Uml::IDType pkgID = parent->getID();
    UMLWidget *pWidget;
    UMLWidgetListIt wit(m_WidgetList);
    while ((pWidget = wit.current()) != NULL) {
        ++wit;
        if (pWidget->getID() == pkgID)
            break;
    }
    if (pWidget == NULL || pWidget->rect().contains(widget->rect()))
        return;
    // create the containment AssocWidget
    AssociationWidget *a = new AssociationWidget(this, pWidget, at_Containment, widget);
    a->calculateEndingPoints();
    a->setActivated(true);
    if (! addAssociation(a))
        delete a;
}

void UMLView::createAutoAttributeAssociations(UMLWidget *widget) {
    if (widget == NULL || m_Type != Uml::dt_Class)
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
    UMLObject *tmpUmlObj = widget->getUMLObject();
    if (tmpUmlObj == NULL)
        return;
    // if the underlying model object is really a UMLClassifier then
    if (tmpUmlObj->getBaseType() == Uml::ot_Datatype) {
        UMLClassifier *dt = static_cast<UMLClassifier*>(tmpUmlObj);
        while (dt->originType() != NULL) {
            tmpUmlObj = dt->originType();
            if (tmpUmlObj->getBaseType() != Uml::ot_Datatype)
                break;
            dt = static_cast<UMLClassifier*>(tmpUmlObj);
        }
    }
    if (tmpUmlObj->getBaseType() != Uml::ot_Class)
        return;
    UMLClassifier * klass = static_cast<UMLClassifier*>(tmpUmlObj);
    // for each of the UMLClassifier's UMLAttributes
    UMLAttributeList attrList = klass->getAttributeList();
    for (UMLAttributeListIt ait(attrList); ait.current(); ++ait) {
        UMLAttribute *attr = ait.current();
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
                                             UMLWidget *widget /*, UMLClassifier * klass*/) {
    if (type == NULL) {
        // kDebug() << "UMLView::createAutoAttributeAssociations("
        //     << klass->getName() << "): type is NULL for "
        //     << "attribute " << attr->getName() << endl;
        return;
    }
    Uml::Association_Type assocType = Uml::at_Composition;
    UMLWidget *w = findWidget( type->getID() );
    AssociationWidget *aw = NULL;
    // if the attribute type has a widget representation on this view
    if (w) {
        aw = findAssocWidget(widget, w, attr->getName());
        if ( aw == NULL &&
               // if the current diagram type permits compositions
               AssocRules::allowAssociation(assocType, widget, w, false) ) {
            // Create a composition AssocWidget, or, if the attribute type is
            // stereotyped <<CORBAInterface>>, create a UniAssociation widget.
            if (type->getStereotype() == "CORBAInterface")
                assocType = at_UniAssociation;
            AssociationWidget *a = new AssociationWidget (this, widget, assocType, w, attr);
            a->calculateEndingPoints();
            a->setVisibility(attr->getVisibility(), B);
            /*
            if (assocType == at_Aggregation || assocType == at_UniAssociation)
            a->setMulti("0..1", B);
            */
            a->setRoleName(attr->getName(), B);
            a->setActivated(true);
            if (! addAssociation(a))
                delete a;
        }
    }
    // if the attribute type is a Datatype then
    if (type->getBaseType() == ot_Datatype) {
        UMLClassifier *dt = static_cast<UMLClassifier*>(type);
        // if the Datatype is a reference (pointer) type
        if (dt->isReference()) {
            //Uml::Association_Type assocType = Uml::at_Composition;
            UMLClassifier *c = dt->originType();
            UMLWidget *w = c ? findWidget( c->getID() ) : 0;
            // if the referenced type has a widget representation on this view
            if (w) {
                aw = findAssocWidget(widget, w, attr->getName());
                if (aw == NULL &&
                    // if the current diagram type permits aggregations
                    AssocRules::allowAssociation(at_Aggregation, widget, w, false)) {
                    // create an aggregation AssocWidget from the ClassifierWidget
                    // to the widget of the referenced type
                    AssociationWidget *a = new AssociationWidget
                            (this, widget, at_Aggregation, w, attr);
                    a->calculateEndingPoints();
                    a->setVisibility(attr->getVisibility(), B);
                    //a->setChangeability(true, B);
                    a->setMulti("0..1", B);
                    a->setRoleName(attr->getName(), B);
                    a->setActivated(true);
                    if (! addAssociation(a))
                        delete a;
                }
            }
        }
    }
}

void UMLView::findMaxBoundingRectangle(const FloatingTextWidget* ft, int& px, int& py, int& qx, int& qy)
{
    if (ft == NULL || !ft->isVisible())
        return;

    int x = ft -> getX();
    int y = ft -> getY();
    int x1 = x + ft -> getWidth() - 1;
    int y1 = y + ft -> getHeight() - 1;

    if (px == -1 || x < px)
        px = x;
    if (py == -1 || y < py)
        py = y;
    if (qx == -1 || x1 > qx)
        qx = x1;
    if (qy == -1 || y1 > qy)
        qy = y1;
}

void UMLView::copyAsImage(QPixmap*& pix) {
    //get the smallest rect holding the diagram
    QRect rect = getDiagramRect();
    QPixmap diagram( rect.width(), rect.height() );

    //only draw what is selected
    m_bDrawSelectedOnly = true;
    selectAssociations(true);
    getDiagram(rect, diagram);

    //now get the selection cut
    int px = -1, py = -1, qx = -1, qy = -1;

    //first get the smallest rect holding the widgets
    for (UMLWidget* temp = m_SelectedList.first(); temp; temp = m_SelectedList.next()) {
        int x = temp -> getX();
        int y = temp -> getY();
        int x1 = x + temp -> width() - 1;
        int y1 = y + temp -> height() - 1;
        if(px == -1 || x < px) {
            px = x;
        }
        if(py == -1 || y < py) {
            py = y;
        }
        if(qx == -1 || x1 > qx) {
            qx = x1;
        }
        if(qy == -1 || y1 > qy) {
            qy = y1;
        }
    }

    //also take into account any text lines in assocs or messages
    AssociationWidget *a;
    AssociationWidgetListIt assoc_it(m_AssociationList);

    //get each type of associations
    //This needs to be reimplemented to increase the rectangle
    //if a part of any association is not included
    while ((a = assoc_it.current()) != NULL) {
        ++assoc_it;
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
    }//end while

    QRect imageRect;  //area with respect to getDiagramRect()
    //i.e. all widgets on the canvas.  Was previously with
    //respect to whole canvas

    imageRect.setLeft( px - rect.left() );
    imageRect.setTop( py - rect.top() );
    imageRect.setRight( qx - rect.left() );
    imageRect.setBottom( qy - rect.top() );

    pix = new QPixmap(imageRect.width(), imageRect.height());
    bitBlt(pix, QPoint(0, 0), &diagram, imageRect);
    m_bDrawSelectedOnly = false;
}

void UMLView::setMenu() {
    slotRemovePopupMenu();
    ListPopupMenu::Menu_Type menu = ListPopupMenu::mt_Undefined;
    switch( getType() ) {
    case dt_Class:
        menu = ListPopupMenu::mt_On_Class_Diagram;
        break;

    case dt_UseCase:
        menu = ListPopupMenu::mt_On_UseCase_Diagram;
        break;

    case dt_Sequence:
        menu = ListPopupMenu::mt_On_Sequence_Diagram;
        break;

    case dt_Collaboration:
        menu = ListPopupMenu::mt_On_Collaboration_Diagram;
        break;

    case dt_State:
        menu = ListPopupMenu::mt_On_State_Diagram;
        break;

    case dt_Activity:
        menu = ListPopupMenu::mt_On_Activity_Diagram;
        break;

    case dt_Component:
        menu = ListPopupMenu::mt_On_Component_Diagram;
        break;

    case dt_Deployment:
        menu = ListPopupMenu::mt_On_Deployment_Diagram;
        break;

    case dt_EntityRelationship:
        menu = ListPopupMenu::mt_On_EntityRelationship_Diagram;
        break;

    default:
        kWarning() << "setMenu() called on unknown diagram type" << endl;
        menu = ListPopupMenu::mt_Undefined;
        break;
    }//end switch
    if( menu != ListPopupMenu::mt_Undefined ) {
        m_pMenu = new ListPopupMenu(this, menu, this);
        connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuSelection(int)));
        m_pMenu->popup( mapToGlobal( contentsToViewport(worldMatrix().map(m_Pos)) ) );
    }
}

void UMLView::slotRemovePopupMenu() {
    if(m_pMenu) {
        disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuSelection(int)));
        delete m_pMenu;
        m_pMenu = 0;
    }
}

void UMLView::slotMenuSelection(int sel) {
    switch( (ListPopupMenu::Menu_Type)sel ) {
    case ListPopupMenu::mt_Undo:
        m_pDoc->loadUndoData();
        break;

    case ListPopupMenu::mt_Redo:
        m_pDoc->loadRedoData();
        break;

    case ListPopupMenu::mt_Clear:
        clearDiagram();
        break;

    case ListPopupMenu::mt_Export_Image:
        m_pImageExporter->exportView();
        break;

    case ListPopupMenu::mt_FloatText:
        {
            FloatingTextWidget* ft = new FloatingTextWidget(this);
            ft->changeTextDlg();
            //if no text entered delete
            if(!FloatingTextWidget::isTextValid(ft->getText())) {
                delete ft;
            } else {
                ft->setID(UniqueID::gen());
                setupNewWidget(ft);
            }
        }
        break;

    case ListPopupMenu::mt_UseCase:
        m_bCreateObject = true;
        Object_Factory::createUMLObject( ot_UseCase );
        break;

    case ListPopupMenu::mt_Actor:
        m_bCreateObject = true;
        Object_Factory::createUMLObject( ot_Actor );
        break;

    case ListPopupMenu::mt_Class:
    case ListPopupMenu::mt_Object:
        m_bCreateObject = true;
        Object_Factory::createUMLObject( ot_Class);
        break;

    case ListPopupMenu::mt_Package:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Package);
        break;

    case ListPopupMenu::mt_Component:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Component);
        break;

    case ListPopupMenu::mt_Node:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Node);
        break;

    case ListPopupMenu::mt_Artifact:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Artifact);
        break;

    case ListPopupMenu::mt_Interface:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Interface);
        break;

    case ListPopupMenu::mt_Enum:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Enum);
        break;

    case ListPopupMenu::mt_Entity:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Entity);
        break;

    case ListPopupMenu::mt_Datatype:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Datatype);
        break;

    case ListPopupMenu::mt_Cut:
        //FIXME make this work for diagram's right click menu
        if ( m_SelectedList.count() &&
                UMLApp::app()->editCutCopy(true) ) {
            deleteSelection();
            m_pDoc->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Copy:
        //FIXME make this work for diagram's right click menu
        m_SelectedList.count() && UMLApp::app()->editCutCopy(true);
        break;

    case ListPopupMenu::mt_Paste:
        m_PastePoint = m_Pos;
        m_Pos.setX( 2000 );
        m_Pos.setY( 2000 );
        UMLApp::app()->slotEditPaste();

        m_PastePoint.setX( 0 );
        m_PastePoint.setY( 0 );
        break;

    case ListPopupMenu::mt_Initial_State:
        {
            StateWidget* state = new StateWidget( this, StateWidget::Initial );
            setupNewWidget( state );
        }
        break;

    case ListPopupMenu::mt_End_State:
        {
            StateWidget* state = new StateWidget( this, StateWidget::End );
            setupNewWidget( state );
        }
        break;

    case ListPopupMenu::mt_State:
        {
            bool ok = false;
            QString name = KInputDialog::getText( i18n("Enter State Name"),
                                                  i18n("Enter the name of the new state:"),
                                                  i18n("new state"), &ok, UMLApp::app() );
            if ( ok ) {
                StateWidget* state = new StateWidget( this );
                state->setName( name );
                setupNewWidget( state );
            }
        }
        break;

    case ListPopupMenu::mt_Initial_Activity:
        {
            ActivityWidget* activity = new ActivityWidget( this, ActivityWidget::Initial );
            setupNewWidget(activity);
        }
        break;


    case ListPopupMenu::mt_End_Activity:
        {
            ActivityWidget* activity = new ActivityWidget( this, ActivityWidget::End );
            setupNewWidget(activity);
        }
        break;

    case ListPopupMenu::mt_Branch:
        {
            ActivityWidget* activity = new ActivityWidget( this, ActivityWidget::Branch );
            setupNewWidget(activity);
        }
        break;

    case ListPopupMenu::mt_Activity:
        {
            bool ok = false;
            QString name = KInputDialog::getText( i18n("Enter Activity Name"),
                                                  i18n("Enter the name of the new activity:"),
                                                  i18n("new activity"), &ok, UMLApp::app() );
            if ( ok ) {
                ActivityWidget* activity = new ActivityWidget( this, ActivityWidget::Normal );
                activity->setName( name );
                setupNewWidget(activity);
            }
        }
        break;

    case ListPopupMenu::mt_SnapToGrid:
        toggleSnapToGrid();
        m_pDoc->setModified();
        break;

    case ListPopupMenu::mt_ShowSnapGrid:
        toggleShowGrid();
        m_pDoc->setModified();
        break;

    case ListPopupMenu::mt_Properties:
        if (showPropDialog() == true)
            m_pDoc->setModified();
        break;

    case ListPopupMenu::mt_Delete:
        m_pDoc->removeDiagram( getID() );
        break;

    case ListPopupMenu::mt_Rename:
        {
            bool ok = false;
            QString name = KInputDialog::getText( i18n("Enter Diagram Name"),
                                                  i18n("Enter the new name of the diagram:"),
                                                  getName(), &ok, UMLApp::app() );
            if (ok) {
                setName(name);
                m_pDoc->signalDiagramRenamed(this);
            }
        }
        break;

    default:
        break;
    }
}

void UMLView::slotCutSuccessful() {
    if( m_bStartedCut ) {
        deleteSelection();
        m_bStartedCut = false;
    }
}

void UMLView::slotShowView() {
    m_pDoc -> changeCurrentView( getID() );
}

QPoint UMLView::getPastePoint() {
    QPoint point = m_PastePoint;
    point.setX( point.x() - m_Pos.x() );
    point.setY( point.y() - m_Pos.y() );
    return point;
}

void UMLView::resetPastePoint() {
    m_PastePoint = m_Pos;
}

int UMLView::snappedX (int x) {
    if (getSnapToGrid()) {
        int gridX = getSnapX();
        int modX = x % gridX;
        x -= modX;
        if (modX >= gridX / 2)
            x += gridX;
    }
    return x;
}

int UMLView::snappedY (int y) {
    if (getSnapToGrid()) {
        int gridY = getSnapY();
        int modY = y % gridY;
        y -= modY;
        if (modY >= gridY / 2)
            y += gridY;
    }
    return y;
}

bool UMLView::showPropDialog() {
    UMLViewDialog dlg( this, this );
    if( dlg.exec() ) {
        return true;
    }
    return false;
}


QFont UMLView::getFont() const {
    return m_Options.uiState.font;
}

void UMLView::setFont(QFont font, bool changeAllWidgets /* = false */) {
    m_Options.uiState.font = font;
    if (!changeAllWidgets)
        return;
    for (UMLWidgetListIt wit(m_WidgetList); wit.current(); ++wit) {
        UMLWidget *w = wit.current();
        w->setFont(font);
    }
}

void UMLView::setClassWidgetOptions( ClassOptionsPage * page ) {
    UMLWidget * pWidget = 0;
    UMLWidgetListIt wit( m_WidgetList );
    while ( (pWidget = wit.current()) != 0 ) {
        ++wit;
        Uml::Widget_Type wt = pWidget->getBaseType();
        if (wt == Uml::wt_Class || wt == Uml::wt_Interface) {
            page -> setWidget( static_cast<ClassifierWidget *>(pWidget) );
            page -> updateUMLWidget();
        }
    }
}


void UMLView::checkSelections() {
    UMLWidget * pWA = 0, * pWB = 0, * pTemp = 0;
    //check messages
    for(pTemp=(UMLWidget *)m_SelectedList.first();pTemp;pTemp=(UMLWidget *)m_SelectedList.next()) {
        if( pTemp->getBaseType() == wt_Message && pTemp -> getSelected() ) {
            MessageWidget * pMessage = static_cast<MessageWidget *>( pTemp );
            pWA = pMessage -> getWidget(A);
            pWB = pMessage -> getWidget(B);
            if( !pWA -> getSelected() ) {
                pWA -> setSelectedFlag( true );
                m_SelectedList.append( pWA );
            }
            if( !pWB -> getSelected() ) {
                pWB -> setSelectedFlag( true );
                m_SelectedList.append( pWB );
            }
        }//end if
    }//end for
    //check Associations
    AssociationWidgetListIt it(m_AssociationList);
    AssociationWidget * pAssoc = 0;
    while((pAssoc = it.current())) {
        ++it;
        if( pAssoc -> getSelected() ) {
            pWA = pAssoc -> getWidget(A);
            pWB = pAssoc -> getWidget(B);
            if( !pWA -> getSelected() ) {
                pWA -> setSelectedFlag( true );
                m_SelectedList.append( pWA );
            }
            if( !pWB -> getSelected() ) {
                pWB -> setSelectedFlag( true );
                m_SelectedList.append( pWB );
            }
        }//end if
    }//end while
}

bool UMLView::checkUniqueSelection()
{
    // if there are no selected items, we return true
    if (m_SelectedList.count() <= 0)
        return true;

    // get the first item and its base type
    UMLWidget * pTemp = (UMLWidget *) m_SelectedList.first();
    Widget_Type tmpType = pTemp -> getBaseType();

    // check all selected items, if they have the same BaseType
    for ( pTemp = (UMLWidget *) m_SelectedList.first();
            pTemp;
            pTemp = (UMLWidget *) m_SelectedList.next() ) {
        if( pTemp->getBaseType() != tmpType)
        {
            return false; // the base types are different, the list is not unique
        }
    } // for ( through all selected items )

    return true; // selected items are unique
}

void UMLView::clearDiagram() {
    if( KMessageBox::Continue == KMessageBox::warningContinueCancel( this, i18n("You are about to delete "
            "the entire diagram.\nAre you sure?"),
            i18n("Delete Diagram?"),KGuiItem( i18n("&Delete"), "editdelete") ) ) {
        removeAllWidgets();
    }
}

void UMLView::toggleSnapToGrid() {
    setSnapToGrid( !getSnapToGrid() );
}

void UMLView::toggleSnapComponentSizeToGrid() {
    setSnapComponentSizeToGrid( !getSnapComponentSizeToGrid() );
}

void UMLView::toggleShowGrid() {
    setShowSnapGrid( !getShowSnapGrid() );
}

void UMLView::setSnapToGrid(bool bSnap) {
    m_bUseSnapToGrid = bSnap;
    emit sigSnapToGridToggled( getSnapToGrid() );
}

void UMLView::setSnapComponentSizeToGrid(bool bSnap) {
    m_bUseSnapComponentSizeToGrid = bSnap;
    updateComponentSizes();
    emit sigSnapComponentSizeToGridToggled( getSnapComponentSizeToGrid() );
}

bool UMLView::getShowSnapGrid() const {
    return m_bShowSnapGrid;
}

void UMLView::setShowSnapGrid(bool bShow) {
    m_bShowSnapGrid = bShow;
    canvas()->setAllChanged();
    emit sigShowGridToggled( getShowSnapGrid() );
}

bool UMLView::getShowOpSig() const {
    return m_Options.classState.showOpSig;
}

void UMLView::setShowOpSig(bool bShowOpSig) {
    m_Options.classState.showOpSig = bShowOpSig;
}

void UMLView::setZoom(int zoom) {
    if (zoom < 10) {
        zoom = 10;
    } else if (zoom > 500) {
        zoom = 500;
    }

    QWMatrix wm;
    wm.scale(zoom/100.0,zoom/100.0);
    setWorldMatrix(wm);

    m_nZoom = currentZoom();
    resizeCanvasToItems();
}

int UMLView::currentZoom() {
    return (int)(worldMatrix().m11()*100.0);
}

void UMLView::zoomIn() {
    QWMatrix wm = worldMatrix();
    wm.scale(1.5,1.5); // adjust zooming step here
    setZoom( (int)(wm.m11()*100.0) );
}

void UMLView::zoomOut() {
    QWMatrix wm = worldMatrix();
    wm.scale(2.0/3.0, 2.0/3.0); //adjust zooming step here
    setZoom( (int)(wm.m11()*100.0) );
}

void UMLView::fileLoaded() {
    setZoom( getZoom() );
    resizeCanvasToItems();
}

void UMLView::setCanvasSize(int width, int height) {
    setCanvasWidth(width);
    setCanvasHeight(height);
    canvas()->resize(width, height);
}

void UMLView::resizeCanvasToItems() {
    QRect canvasSize = getDiagramRect();
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

void UMLView::show() {
    QWidget::show();
    resizeCanvasToItems();
}

void UMLView::updateComponentSizes() {
    // update sizes of all components
    UMLWidgetListIt it( m_WidgetList );
    UMLWidget *obj;
    while ( (obj=(UMLWidget*)it.current()) != 0 ) {
        ++it;
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
void UMLView::forceUpdateWidgetFontMetrics(QPainter * painter) {
    UMLWidgetListIt it( m_WidgetList );
    UMLWidget *obj;

    while ((obj = it.current()) != 0 ) {
        ++it;
        obj->forceUpdateFontMetrics(painter);
    }
}

void UMLView::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement viewElement = qDoc.createElement( "diagram" );
    viewElement.setAttribute( "xmi.id", ID2STR(m_nID) );
    viewElement.setAttribute( "name", getName() );
    viewElement.setAttribute( "type", m_Type );
    viewElement.setAttribute( "documentation", m_Documentation );
    //optionstate uistate
    viewElement.setAttribute( "fillcolor", m_Options.uiState.fillColor.name() );
    viewElement.setAttribute( "linecolor", m_Options.uiState.lineColor.name() );
    viewElement.setAttribute( "linewidth", m_Options.uiState.lineWidth );
    viewElement.setAttribute( "usefillcolor", m_Options.uiState.useFillColor );
    viewElement.setAttribute( "font", m_Options.uiState.font.toString() );
    //optionstate classstate
    viewElement.setAttribute( "showattsig", m_Options.classState.showAttSig );
    viewElement.setAttribute( "showatts", m_Options.classState.showAtts);
    viewElement.setAttribute( "showopsig", m_Options.classState.showOpSig );
    viewElement.setAttribute( "showops", m_Options.classState.showOps );
    viewElement.setAttribute( "showpackage", m_Options.classState.showPackage );
    viewElement.setAttribute( "showscope", m_Options.classState.showVisibility );
    viewElement.setAttribute( "showstereotype", m_Options.classState.showStereoType );
    //misc
    viewElement.setAttribute( "localid", ID2STR(m_nLocalID) );
    viewElement.setAttribute( "showgrid", m_bShowSnapGrid );
    viewElement.setAttribute( "snapgrid", m_bUseSnapToGrid );
    viewElement.setAttribute( "snapcsgrid", m_bUseSnapComponentSizeToGrid );
    viewElement.setAttribute( "snapx", m_nSnapX );
    viewElement.setAttribute( "snapy", m_nSnapY );
    viewElement.setAttribute( "zoom", m_nZoom );
    viewElement.setAttribute( "canvasheight", m_nCanvasHeight );
    viewElement.setAttribute( "canvaswidth", m_nCanvasWidth );
    //now save all the widgets
    UMLWidget * widget = 0;
    UMLWidgetListIt w_it( m_WidgetList );
    QDomElement widgetElement = qDoc.createElement( "widgets" );
    while( ( widget = w_it.current() ) ) {
        ++w_it;
        // Having an exception is bad I know, but gotta work with
        // system we are given.
        // We DONT want to record any text widgets which are belonging
        // to associations as they are recorded later in the "associations"
        // section when each owning association is dumped. -b.t.
        if (widget->getBaseType() != wt_Text ||
                static_cast<FloatingTextWidget*>(widget)->getLink() == NULL)
            widget->saveToXMI( qDoc, widgetElement );
    }
    viewElement.appendChild( widgetElement );
    //now save the message widgets
    MessageWidgetListIt m_it( m_MessageList );
    QDomElement messageElement = qDoc.createElement( "messages" );
    while( ( widget = m_it.current() ) ) {
        ++m_it;
        widget -> saveToXMI( qDoc, messageElement );
    }
    viewElement.appendChild( messageElement );
    //now save the associations
    QDomElement assocElement = qDoc.createElement( "associations" );
    if ( m_AssociationList.count() ) {
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
        AssociationWidgetListIt a_it( m_AssociationList );
        AssociationWidget * assoc = 0;
        while( ( assoc = a_it.current() ) ) {
            ++a_it;
            assoc -> saveToXMI( qDoc, assocElement );
        }
        // kDebug() << "UMLView::saveToXMI() saved "
        //   << m_AssociationList.count() << " assocData." << endl;
    }
    viewElement.appendChild( assocElement );
    qElement.appendChild( viewElement );
}

bool UMLView::loadFromXMI( QDomElement & qElement ) {
    QString id = qElement.attribute( "xmi.id", "-1" );
    m_nID = STR2ID(id);
    if( m_nID == Uml::id_None )
        return false;
    setName( qElement.attribute( "name", "" ) );
    QString type = qElement.attribute( "type", "0" );
    m_Documentation = qElement.attribute( "documentation", "" );
    QString localid = qElement.attribute( "localid", "0" );
    //optionstate uistate
    QString font = qElement.attribute( "font", "" );
    if (!font.isEmpty()) {
        m_Options.uiState.font.fromString( font );
        m_Options.uiState.font.setUnderline(false);
    }
    QString fillcolor = qElement.attribute( "fillcolor", "" );
    QString linecolor = qElement.attribute( "linecolor", "" );
    QString linewidth = qElement.attribute( "linewidth", "" );
    QString usefillcolor = qElement.attribute( "usefillcolor", "0" );
    m_Options.uiState.useFillColor = (bool)usefillcolor.toInt();
    //optionstate classstate
    QString temp = qElement.attribute( "showattsig", "0" );
    m_Options.classState.showAttSig = (bool)temp.toInt();
    temp = qElement.attribute( "showatts", "0" );
    m_Options.classState.showAtts = (bool)temp.toInt();
    temp = qElement.attribute( "showopsig", "0" );
    m_Options.classState.showOpSig = (bool)temp.toInt();
    temp = qElement.attribute( "showops", "0" );
    m_Options.classState.showOps = (bool)temp.toInt();
    temp = qElement.attribute( "showpackage", "0" );
    m_Options.classState.showPackage = (bool)temp.toInt();
    temp = qElement.attribute( "showscope", "0" );
    m_Options.classState.showVisibility = (bool)temp.toInt();
    temp = qElement.attribute( "showstereotype", "0" );
    m_Options.classState.showStereoType = (bool)temp.toInt();
    //misc
    QString showgrid = qElement.attribute( "showgrid", "0" );
    m_bShowSnapGrid = (bool)showgrid.toInt();

    QString snapgrid = qElement.attribute( "snapgrid", "0" );
    m_bUseSnapToGrid = (bool)snapgrid.toInt();

    QString snapcsgrid = qElement.attribute( "snapcsgrid", "0" );
    m_bUseSnapComponentSizeToGrid = (bool)snapcsgrid.toInt();

    QString snapx = qElement.attribute( "snapx", "10" );
    m_nSnapX = snapx.toInt();

    QString snapy = qElement.attribute( "snapy", "10" );
    m_nSnapY = snapy.toInt();

    QString zoom = qElement.attribute( "zoom", "100" );
    m_nZoom = zoom.toInt();

    QString height = qElement.attribute( "canvasheight", QString("%1").arg(UMLView::defaultCanvasSize) );
    m_nCanvasHeight = height.toInt();

    QString width = qElement.attribute( "canvaswidth", QString("%1").arg(UMLView::defaultCanvasSize) );
    m_nCanvasWidth = width.toInt();

    int nType = type.toInt();
    if (nType == -1 || nType >= 400) {
        // Pre 1.5.5 numeric values
        // Values of "type" were changed in 1.5.5 to merge with Settings::Diagram
        switch (nType) {
            case 400:
                m_Type = Uml::dt_UseCase;
                break;
            case 401:
                m_Type = Uml::dt_Collaboration;
                break;
            case 402:
                m_Type = Uml::dt_Class;
                break;
            case 403:
                m_Type = Uml::dt_Sequence;
                break;
            case 404:
                m_Type = Uml::dt_State;
                break;
            case 405:
                m_Type = Uml::dt_Activity;
                break;
            case 406:
                m_Type = Uml::dt_Component;
                break;
            case 407:
                m_Type = Uml::dt_Deployment;
                break;
            case 408:
                m_Type = Uml::dt_EntityRelationship;
                break;
            default:
                m_Type = Uml::dt_Undefined;
                break;
        }
    } else {
        m_Type = (Uml::Diagram_Type)nType;
    }
    if( !fillcolor.isEmpty() )
        m_Options.uiState.fillColor = QColor( fillcolor );
    if( !linecolor.isEmpty() )
        m_Options.uiState.lineColor = QColor( linecolor );
    if( !linewidth.isEmpty() )
        m_Options.uiState.lineWidth = linewidth.toInt();
    m_nLocalID = STR2ID(localid);

    QDomNode node = qElement.firstChild();
    bool widgetsLoaded = false, messagesLoaded = false, associationsLoaded = false;
    while (!node.isNull()) {
        QDomElement element = node.toElement();
        if (!element.isNull()) {
            if (element.tagName() == "widgets")
                widgetsLoaded = loadWidgetsFromXMI( element );
            else if (element.tagName() == "messages")
                messagesLoaded = loadMessagesFromXMI( element );
            else if (element.tagName() == "associations")
                associationsLoaded = loadAssociationsFromXMI( element );
        }
        node = node.nextSibling();
    }

    if (!widgetsLoaded) {
        kWarning() << "failed umlview load on widgets" << endl;
        return false;
    }
    if (!messagesLoaded) {
        kWarning() << "failed umlview load on messages" << endl;
        return false;
    }
    if (!associationsLoaded) {
        kWarning() << "failed umlview load on associations" << endl;
        return false;
    }
    return true;
}

bool UMLView::loadWidgetsFromXMI( QDomElement & qElement ) {
    UMLWidget* widget = 0;
    QDomNode node = qElement.firstChild();
    QDomElement widgetElement = node.toElement();
    while( !widgetElement.isNull() ) {
        widget = loadWidgetFromXMI(widgetElement);
        if (widget) {
            m_WidgetList.append( widget );
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

UMLWidget* UMLView::loadWidgetFromXMI(QDomElement& widgetElement) {

    if ( !m_pDoc ) {
        kWarning() << "UMLView::loadWidgetFromXMI(): m_pDoc is NULL" << endl;
        return 0L;
    }

    QString tag  = widgetElement.tagName();
    QString idstr  = widgetElement.attribute( "xmi.id", "-1" );
    UMLWidget* widget = Widget_Factory::makeWidgetFromXMI(tag, idstr, this);
    if (widget == NULL)
        return NULL;
    if (!widget->loadFromXMI(widgetElement)) {
        widget->cleanup();
        delete widget;
        return 0;
    }
    return widget;
}

bool UMLView::loadMessagesFromXMI( QDomElement & qElement ) {
    MessageWidget * message = 0;
    QDomNode node = qElement.firstChild();
    QDomElement messageElement = node.toElement();
    while( !messageElement.isNull() ) {
        QString tag = messageElement.tagName();
        if (tag == "messagewidget" ||
                tag == "UML:MessageWidget" ) {  // for bkwd compatibility
            message = new MessageWidget(this, sequence_message_asynchronous,
                                        Uml::id_Reserved);
            if( !message -> loadFromXMI( messageElement ) ) {
                delete message;
                return false;
            }
            m_MessageList.append( message );
            FloatingTextWidget *ft = message->getFloatingTextWidget();
            if (ft)
                m_WidgetList.append( ft );
            else if (message->getSequenceMessageType() != sequence_message_creation)
                kDebug() << "UMLView::loadMessagesFromXMI: ft is NULL"
                          << " for message " << ID2STR(message->getID()) << endl;
        }
        node = messageElement.nextSibling();
        messageElement = node.toElement();
    }
    return true;
}

bool UMLView::loadAssociationsFromXMI( QDomElement & qElement ) {
    QDomNode node = qElement.firstChild();
    QDomElement assocElement = node.toElement();
    int countr = 0;
    while( !assocElement.isNull() ) {
        QString tag = assocElement.tagName();
        if (tag == "assocwidget" ||
                tag == "UML:AssocWidget") {  // for bkwd compatibility
            countr++;
            AssociationWidget *assoc = new AssociationWidget(this);
            if( !assoc->loadFromXMI( assocElement ) ) {
                kError() << "couldn't loadFromXMI association widget:"
                          << assoc << ", bad XMI file? Deleting from umlview."
                          << endl;
                delete assoc;
                /* return false;
                   Returning false here is a little harsh when the
                   rest of the diagram might load okay.
                 */
            } else {
                if(!addAssociation(assoc, false))
                {
                    kError()<<"Couldnt addAssociation("<<assoc<<") to umlview, deleting."<<endl;
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
    if (m_pDoc->addUMLObject(object))
        m_pDoc->signalUMLObjectCreated(object);  // m_bCreateObject is reset by slotObjectCreated()
    else
        m_bCreateObject = false;
}

bool UMLView::loadUisDiagramPresentation(QDomElement & qElement) {
    for (QDomNode node = qElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement elem = node.toElement();
        QString tag = elem.tagName();
        if (! Uml::tagEq(tag, "Presentation")) {
            kError() << "ignoring unknown UisDiagramPresentation tag "
                      << tag << endl;
            continue;
        }
        QDomNode n = elem.firstChild();
        QDomElement e = n.toElement();
        QString idStr;
        int x = 0, y = 0, w = 0, h = 0;
        while (!e.isNull()) {
            tag = e.tagName();
            kDebug() << "Presentation: tag = " << tag << endl;
            if (Uml::tagEq(tag, "Presentation.geometry")) {
                QDomNode gnode = e.firstChild();
                QDomElement gelem = gnode.toElement();
                QString csv = gelem.text();
                QStringList dim = QStringList::split(",", csv);
                x = dim[0].toInt();
                y = dim[1].toInt();
                w = dim[2].toInt();
                h = dim[3].toInt();
            } else if (Uml::tagEq(tag, "Presentation.style")) {
                // TBD
            } else if (Uml::tagEq(tag, "Presentation.model")) {
                QDomNode mnode = e.firstChild();
                QDomElement melem = mnode.toElement();
                idStr = melem.attribute("xmi.idref", "");
            } else {
                kDebug() << "UMLView::uisLoadFromXMI: ignoring tag "
                          << tag << endl;
            }
            n = n.nextSibling();
            e = n.toElement();
        }
        Uml::IDType id = STR2ID(idStr);
        UMLObject *o = m_pDoc->findObjectById(id);
        if (o == NULL) {
            kError() << "UMLView::uisLoadFromXMI: Cannot find object for id "
                      << idStr << endl;
        } else {
            Uml::Object_Type ot = o->getBaseType();
            kDebug() << "Create widget for model object of type " << ot << endl;
            UMLWidget *widget = NULL;
            switch (ot) {
            case Uml::ot_Class:
                widget = new ClassifierWidget(this, static_cast<UMLClassifier*>(o));
                break;
            case Uml::ot_Association:
                {
                    UMLAssociation *umla = static_cast<UMLAssociation*>(o);
                    Uml::Association_Type at = umla->getAssocType();
                    UMLObject* objA = umla->getObject(Uml::A);
                    UMLObject* objB = umla->getObject(Uml::B);
                    if (objA == NULL || objB == NULL) {
                        kError() << "intern err 1" << endl;
                        return false;
                    }
                    UMLWidget *wA = findWidget(objA->getID());
                    UMLWidget *wB = findWidget(objB->getID());
                    if (wA != NULL && wB != NULL) {
                        AssociationWidget *aw =
                            new AssociationWidget(this, wA, at, wB, umla);
                        aw->syncToModel();
                        m_AssociationList.append(aw);
                    } else {
                        kError() << "cannot create assocwidget from ("
                                  << wA << ", " << wB << ")" << endl;
                    }
                    break;
                }
            case Uml::ot_Role:
                {
                    UMLRole *robj = static_cast<UMLRole*>(o);
                    UMLAssociation *umla = robj->getParentAssociation();
                    // @todo properly display role names.
                    //       For now, in order to get the role names displayed
                    //       simply delete the participating diagram objects
                    //       and drag them from the list view to the diagram.
                    break;
                }
            default:
                kError() << "UMLView::uisLoadFromXMI: "
                          << "Cannot create widget of type "
                          << ot << endl;
            }
            if (widget) {
                kDebug() << "Widget: x=" << x << ", y=" << y
                          << ", w=" << w << ", h=" << h << endl;
                widget->setX(x);
                widget->setY(y);
                widget->setSize(w, h);
                m_WidgetList.append(widget);
            }
        }
    }
    return true;
}

bool UMLView::loadUISDiagram(QDomElement & qElement) {
    QString idStr = qElement.attribute( "xmi.id", "" );
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
            setName( elem.text() );
            if (ulvi)
                ulvi->setText( getName() );
        } else if (tag == "uisDiagramStyle") {
            QString diagramStyle = elem.text();
            if (diagramStyle != "ClassDiagram") {
                kError() << "UMLView::uisLoadFromXMI: diagram style " << diagramStyle
                          << " is not yet implemented" << endl;
                continue;
            }
            m_pDoc->setMainViewID(m_nID);
            m_Type = Uml::dt_Class;
            UMLListView *lv = UMLApp::app()->getListView();
            ulvi = new UMLListViewItem( lv->theLogicalView(), getName(),
                                        Uml::lvt_Class_Diagram, m_nID );
        } else if (tag == "uisDiagramPresentation") {
            loadUisDiagramPresentation(elem);
        } else if (tag != "uisToolName") {
            kDebug() << "UMLView::uisLoadFromXMI: ignoring tag " << tag << endl;
        }
    }
    return true;
}


#include "umlview.moc"
