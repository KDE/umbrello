/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
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
#include <klocale.h>
#include <kdebug.h>

// application specific includes
#include "umlview.h"
#include "listpopupmenu.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "docwindow.h"
#include "assocrules.h"
#include "umlviewcanvas.h"
#include "dialogs/classoptionspage.h"
#include "dialogs/umlviewdialog.h"
#include "inputdialog.h"
#include "clipboard/idchangelog.h"
#include "clipboard/umldrag.h"
#include "floatingtext.h"
#include "classwidget.h"
#include "class.h"
#include "packagewidget.h"
#include "package.h"
#include "componentwidget.h"
#include "component.h"
#include "nodewidget.h"
#include "node.h"
#include "artifactwidget.h"
#include "artifact.h"
#include "interfacewidget.h"
#include "interface.h"
#include "datatypewidget.h"
#include "datatype.h"
#include "enumwidget.h"
#include "enum.h"
#include "actorwidget.h"
#include "actor.h"
#include "usecasewidget.h"
#include "usecase.h"
#include "notewidget.h"
#include "boxwidget.h"
#include "associationwidget.h"
#include "objectwidget.h"
#include "messagewidget.h"
#include "statewidget.h"
#include "activitywidget.h"
#include "seqlinewidget.h"

#include "umllistviewitemlist.h"
#include "umllistviewitem.h"
#include "umllistview.h"
#include "umlobjectlist.h"
#include "association.h"

#include "umlwidget.h"

#include "toolbarstatefactory.h"


// control the manual DoubleBuffering of QCanvas
// with a define, so that this memory X11 effect can
// be tested more easily
#define MANUAL_CONTROL_DOUBLE_BUFFERING

// static members
const int UMLView::defaultCanvasSize = 1300;


// constructor
UMLView::UMLView(QWidget* parent, UMLDoc* doc) : QCanvasView(parent, "AnUMLView") {
	init();
	m_pDoc = doc;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::init() {
	// Initialize loaded/saved data
	m_nID = -1;
	m_pDoc = NULL;
	m_Documentation = "";
	m_Name = "umlview";
	m_Type = dt_Undefined;
	m_nLocalID = 30000;
	m_bUseSnapToGrid = false;
	m_bUseSnapComponentSizeToGrid = false;
	m_bShowSnapGrid = false;
	m_nSnapX = 10;
	m_nSnapY = 10;
	m_nZoom = 100;
	m_nCanvasWidth = UMLView::defaultCanvasSize;
	m_nCanvasHeight = UMLView::defaultCanvasSize;

	// Initialize other data
	m_AssociationList.setAutoDelete( true );
	//Setup up booleans
	m_bPaste = false;
	m_bDrawRect = false;
	m_bActivated = false;
	m_bCreateObject = false;
	m_bDrawSelectedOnly = false;
	m_bPopupShowing = false;
	m_bStartedCut = false;
	//m_bMouseButtonPressed = false;
	//clear pointers
	m_pMoveAssoc = 0;
	m_pOnWidget = 0;
	m_pAssocLine = 0;
	m_PastePoint = QPoint(0, 0);
	m_pIDChangesLog = 0;
	m_pFirstSelectedWidget = 0;
	m_pMenu = 0;
	//setup graphical items
	viewport() -> setBackgroundMode( NoBackground );
	setCanvas( new UMLViewCanvas( this ) );
	canvas() -> setUpdatePeriod( 20 );
	resizeContents(defaultCanvasSize, defaultCanvasSize);
	canvas() -> resize(defaultCanvasSize, defaultCanvasSize);
	setAcceptDrops(TRUE);
	viewport() -> setAcceptDrops(TRUE);
	setDragAutoScroll(false);

	viewport() -> setMouseTracking(false);
	//m_SelectionRect.setAutoDelete( true );

	// TODO: Still needed at some places.
	m_CurrentCursor = WorkToolBar::tbb_Arrow;

	//setup signals
	connect( this, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu() ) );
	connect( UMLApp::app(), SIGNAL( sigCutSuccessful() ),
	         this, SLOT( slotCutSuccessful() ) );

	// Create the ToolBarState factory. This class is not a singleton, because it
	// needs a pointer to this object.
	m_pToolBarStateFactory = new ToolBarStateFactory(this);
	m_pToolBarState = m_pToolBarStateFactory->getState(WorkToolBar::tbb_Arrow);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLView::~UMLView() {
	if(m_pIDChangesLog) {
		delete    m_pIDChangesLog;
		m_pIDChangesLog = 0;
	}

	if( m_pAssocLine )
	{
		delete m_pAssocLine;
		m_pAssocLine = NULL;
	}
	//m_SelectionRect.clear();

	// before we can delete the QCanvas, all widgets must be explicitly
	// removed
	// otherwise the implicit remove of the contained widgets will cause
	// events which would demand a valid connected QCanvas
	// ==> this causes umbrello to crash for some - larger?? - projects
	// first avoid all events, which would cause some update actions
	// on deletion of each removed widget
	blockSignals( true );
	removeAllWidgets();
	if ( canvas() != NULL ) {
		// Qt Doc for QCanvasView::~QCanvasView () states:
		// "Destroys the canvas view. The associated canvas is not deleted."
		// we should do it now
		delete canvas();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLDoc* UMLView::getDocument() const {
	return m_pDoc;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::print(KPrinter *pPrinter, QPainter & pPainter) {
	int height, width;
	//get the size of the page
	QPaintDeviceMetrics metrics(pPrinter);
	QFontMetrics fm = pPainter.fontMetrics(); // use the painter font metrics, not the screen fm!
	int fontHeight  = fm.lineSpacing();
	int marginX = pPrinter->margins().width();
	int marginY = pPrinter->margins().height();

	// The printer will probably use a different font with different font metrics,
	// force the widgets to update accordingly on paint
	forceUpdateWidgetFontMetrics(&pPainter);

	//double margin at botton of page as it doesn't print down there
	//on my printer, so play safe as default.
	if(pPrinter->orientation() == KPrinter::Portrait) {
		width = metrics.width() - marginX * 2;
		height = metrics.height() - fontHeight - 4 - marginY * 3;
	} else {
		marginX *= 2;
		width = metrics.width() - marginX * 2;
		height = metrics.height() - fontHeight - 4 - marginY * 2;
	}
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
			pPainter.drawText(0, height + 4, width, fontHeight, AlignLeft, string);

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
	kdDebug() << "drawHeight: " << drawHeight << ", width: " << rect.width()
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
	pPainter.setViewport( marginX, marginY, width, height );

	// get Diagram
	getDiagram(QRect(rect.x(), rect.y(), windowWidth, diagramHeight), pPainter);

	//draw foot note
	QString string = i18n("Diagram: %2 Page %1").arg( 1).arg(getName());
	QColor textColor(50, 50, 50);
	pPainter.setPen(textColor);
	pPainter.drawLine(rect.x(), footTop    , windowWidth, footTop);
	pPainter.drawText(rect.x(), footTop + 3, windowWidth, fontHeight, AlignLeft, string);

	// now restore scaling
	pPainter.restore();

#endif
	// next painting will most probably be to a different device (i.e. the screen)
	forceUpdateWidgetFontMetrics(0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::contentsMouseReleaseEvent(QMouseEvent* ome) {
	m_pToolBarState->mouseRelease(ome);

	// TODO: Move to the toolbar states.
	resizeCanvasToItems();


	// TODO: Not inserted into the toolbar state. Is this really needed?
	/*
	if ( m_CurrentCursor < WorkToolBar::tbb_Actor || m_CurrentCursor > WorkToolBar::tbb_State ) {
		m_pFirstSelectedWidget = 0;
		return;
	}
	*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::slotToolBarChanged(int c)
{
	m_pToolBarState = m_pToolBarStateFactory->getState((WorkToolBar::ToolBar_Buttons)c);
	m_pToolBarState->init();

	// TODO This should be deleted once.
	m_CurrentCursor = (WorkToolBar::ToolBar_Buttons)c;

	m_pFirstSelectedWidget = 0;
	m_bPaste = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::showEvent(QShowEvent* /*se*/) {

#	ifdef MANUAL_CONTROL_DOUBLE_BUFFERING
	kdWarning() << "Show Event for " << getName() << endl;
	canvas()->setDoubleBuffering( true );
#	endif

	UMLApp* theApp = UMLApp::app();
	WorkToolBar* tb = theApp->getWorkToolBar();
	connect(tb,SIGNAL(sigButtonChanged(int)), this, SLOT(slotToolBarChanged(int)));
	connect(this,SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));
	connect(m_pDoc, SIGNAL(sigObjectCreated(UMLObject *)),
		this, SLOT(slotObjectCreated(UMLObject *)));
	resetToolbar();

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::hideEvent(QHideEvent* /*he*/) {
	UMLApp* theApp = UMLApp::app();
	WorkToolBar* tb = theApp->getWorkToolBar();
	disconnect(tb,SIGNAL(sigButtonChanged(int)), this, SLOT(slotToolBarChanged(int)));
	disconnect(this,SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));
	disconnect(m_pDoc, SIGNAL(sigObjectCreated(UMLObject *)), this, SLOT(slotObjectCreated(UMLObject *)));

#	ifdef MANUAL_CONTROL_DOUBLE_BUFFERING
	kdWarning() << "Hide Event for " << getName() << endl;
	canvas()->setDoubleBuffering( false );
#	endif
}

void UMLView::slotObjectCreated(UMLObject* o) {
	m_bPaste = false;
	int type  = o->getBaseType();
	//check to see if we want the message
	//may be wanted by someone else e.g. list view
	if (!m_bCreateObject)  {
		return;
	}

	int y = m_Pos.y();

	UMLWidget* newWidget = 0;
	if(type == ot_Actor) {
		if (getType() == dt_Sequence) {
			ObjectWidget *ow = new ObjectWidget(this, o, getLocalID() );
			ow->setDrawAsActor(true);
			if (m_Type == dt_Sequence) {
				y = ow->topMargin();
			}
			newWidget = ow;
		} else
			newWidget = new ActorWidget(this, static_cast<UMLActor*>(o));
	} else if(type == ot_UseCase) {
		newWidget = new UseCaseWidget(this, static_cast<UMLUseCase*>(o));
	} else if(type == ot_Package) {
		newWidget = new PackageWidget(this, static_cast<UMLPackage*>(o));
	} else if(type == ot_Component) {
		newWidget = new ComponentWidget(this, static_cast<UMLComponent*>(o));
		if (getType() == dt_Deployment) {
			newWidget->setIsInstance(true);
		}
	} else if(type == ot_Node) {
		newWidget = new NodeWidget(this,static_cast<UMLNode*>(o));
	} else if(type == ot_Artifact) {
		newWidget = new ArtifactWidget(this, static_cast<UMLArtifact*>(o));
	} else if(type == ot_Node) {
		newWidget = new NodeWidget(this, static_cast<UMLNode*>(o));
	} else if(type == ot_Datatype) {
		newWidget = new DatatypeWidget(this, static_cast<UMLDatatype*>(o));
	} else if(type == ot_Enum) {
		newWidget = new EnumWidget(this, static_cast<UMLEnum*>(o));
	} else if(type == ot_Interface) {
	        InterfaceWidget* interfaceWidget = new InterfaceWidget(this, static_cast<UMLInterface*>(o));
		Diagram_Type diagramType = getType();
		if (diagramType == dt_Component || diagramType == dt_Deployment) {
			interfaceWidget->setDrawAsCircle(true);
		}
		newWidget = interfaceWidget;
	} else if(type == ot_Datatype) {
		newWidget = new DatatypeWidget(this, static_cast<UMLDatatype*>(o));
	} else if(type == ot_Class ) { // CORRECT?
		//see if we really want an object widget or class widget
		if(getType() == dt_Class) {
			newWidget = new ClassWidget(this, static_cast<UMLClass*>(o));
		} else {
			ObjectWidget *ow = new ObjectWidget(this, o, getLocalID() );
			if (m_Type == dt_Sequence) {
				y = ow->topMargin();
			}
			newWidget = ow;
		}
	} else {
		kdWarning() << "trying to create an invalid widget" << endl;
		return;
	}

	newWidget->setX( m_Pos.x() );
	newWidget->setY( y );
	newWidget->setVisible( true );
	newWidget->setActivated();
	newWidget->setFont( getFont() );
	newWidget->slotColorChanged( getID() );
	newWidget->slotLineWidthChanged( getID() );
	m_bCreateObject = false;
	m_WidgetList.append(newWidget);
	switch( type ) {
		case ot_Actor:
		case ot_UseCase:
		case ot_Class:
		case ot_Package:
		case ot_Component:
		case ot_Node:
		case ot_Artifact:
		case ot_Interface:
		case ot_Enum:
		case ot_Datatype:
			createAutoAssociations(newWidget);
			break;
	}
	resizeCanvasToItems();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::slotObjectRemoved(UMLObject * o) {
	m_bPaste = false;
	int id = o->getID();
	UMLWidgetListIt it( m_WidgetList );
	UMLWidget *obj;

	while ((obj = it.current()) != 0 ) {
		++it;
		if(obj -> getID() != id)
			continue;
		removeWidget(obj);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::contentsDragEnterEvent(QDragEnterEvent *e) {
	UMLDrag::LvTypeAndID_List tidList;
	if(!UMLDrag::getClip3TypeAndID(e, tidList)) {
		return;
	}
	UMLDrag::LvTypeAndID_It tidIt(tidList);
	UMLDrag::LvTypeAndID * tid = tidIt.current();
	if (!tid) {
		kdDebug() << "UMLView::contentsDragEnterEvent: "
			  << "UMLDrag::getClip3TypeAndID returned empty list" << endl;
		return;
	}
	ListView_Type lvtype = tid->type;
	int id = tid->id;

	Diagram_Type diagramType = getType();

	UMLObject* temp = 0;
	//if dragging diagram - set false
	switch( lvtype ) {
		case lvt_UseCase_Diagram:
		case lvt_Sequence_Diagram:
		case lvt_Class_Diagram:
		case lvt_Collaboration_Diagram:
		case lvt_State_Diagram:
		case lvt_Activity_Diagram:
			e->accept(false);
			return;
		default:
			break;
	}
	//can't drag anything onto state/activity diagrams
	if( diagramType == dt_State || diagramType == dt_Activity) {
		e->accept(false);
		return;
	}
	//make sure can find UMLObject
	if( !(temp = m_pDoc->findUMLObject(id) ) ) {
		kdDebug() << " object not found" << endl;
		e->accept(false);
		return;
	}
	//make sure dragging item onto correct diagram
	// concept - class,seq,coll diagram
	// actor,usecase - usecase diagram
	UMLObject_Type ot = temp->getBaseType();
	if(diagramType == dt_UseCase && (ot != ot_Actor && ot != ot_UseCase) ) {
		e->accept(false);
		return;
	}
	if(diagramType == dt_Class &&
	   (ot != ot_Class && ot != ot_Package && ot != ot_Interface && ot != ot_Enum && ot != ot_Datatype)) {
		e->accept(false);
		return;
	}
	if((diagramType == dt_Sequence || diagramType == dt_Collaboration) &&
	   ot != ot_Class && ot != ot_Actor) {
		e->accept(false);
		return;
	}
	if (diagramType == dt_Deployment &&
	    (ot != ot_Interface && ot != ot_Component && ot != ot_Class && ot != ot_Node)) {
		e->accept(false);
		return;
	}
	if (diagramType == dt_Component &&
	    (ot != ot_Interface && ot != ot_Component && ot != ot_Artifact)) {
		e->accept(false);
		return;
	}
	if((diagramType == dt_UseCase || diagramType == dt_Class ||
	    diagramType == dt_Component || diagramType == dt_Deployment)
	   && widgetOnDiagram(id) ) {
		e->accept(false);
		return;
	}
	e->accept(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::contentsDropEvent(QDropEvent *e) {
	UMLDrag::LvTypeAndID_List tidList;
	if( !UMLDrag::getClip3TypeAndID(e, tidList) ) {
		return;
	}
	UMLDrag::LvTypeAndID_It tidIt(tidList);
	UMLDrag::LvTypeAndID * tid = tidIt.current();
	if (!tid) {
		kdDebug() << "UMLView::contentsDropEvent: "
			  << "UMLDrag::getClip3TypeAndID returned empty list" << endl;
		return;
	}
	ListView_Type lvtype = tid->type;
	int id = tid->id;

	if(lvtype >= lvt_UseCase_Diagram && lvtype <= lvt_Sequence_Diagram) {
		return;
	}
	UMLObject* o = m_pDoc->findUMLObject(id);
	if( !o ) {
		kdDebug() << " object not found" << endl;
		return;
	}
	m_bCreateObject = true;
	m_Pos = e->pos();

	slotObjectCreated(o);

	m_pDoc -> setModified(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectWidget * UMLView::onWidgetLine( QPoint point ) {
	SeqLineWidget * pLine = 0;
	for( pLine = m_SeqLineList.first(); pLine; pLine = m_SeqLineList.next() ) {
		if( pLine -> onWidget( point ) ) {
			return pLine -> getObjectWidget();
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLView::widgetOnDiagram(int id) {
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

////////////////////////////////////////////////////////////////////////////////////////////////////

void UMLView::contentsMouseMoveEvent(QMouseEvent* ome)
{
	m_pToolBarState->mouseMove(ome);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// search both our UMLWidget AND MessageWidget lists
UMLWidget * UMLView::findWidget( int id ) {

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

////////////////////////////////////////////////////////////////////////////////////////////////////
AssociationWidget * UMLView::findAssocWidget( int id ) {
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
////////////////////////////////////////////////////////////////////////////////////////////////////
AssociationWidget * UMLView::findAssocWidget(Association_Type at,
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
		// Allow for the swapped roles of generalization/realization assocwidgets.
		// When the swapped roles bug is fixed, this code can disappear.
		if (pWidgetA->getID() == assoc->getWidgetID(B) &&
		    pWidgetB->getID() == assoc->getWidgetID(A)) {
			kdDebug() << "UMLView::findAssocWidget: found assoctype " << at
				  << "with swapped roles (A: " << pWidgetA->getName()
				  << ", B: " << pWidgetB->getName() << ")" << endl;
			return assoc;
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::removeWidget(UMLWidget * o) {
	if(!o)
		return;
	removeAssociations(o);

	UMLWidget_Type t = o->getBaseType();
	if(getType() == dt_Sequence && t == wt_Object)
		checkMessages( static_cast<ObjectWidget*>(o) );

	if( m_pOnWidget == o ) {
		UMLApp::app() -> getDocWindow() -> updateDocumentation( true );
		m_pOnWidget = 0;
	}

	o -> cleanup();
	m_SelectedList.remove(o);
	disconnect( this, SIGNAL( sigRemovePopupMenu() ), o, SLOT( slotRemovePopupMenu() ) );
	disconnect( this, SIGNAL( sigClearAllSelected() ), o, SLOT( slotClearAllSelected() ) );
	disconnect( this, SIGNAL(sigColorChanged(int)), o, SLOT(slotColorChanged(int)));
	if (t == wt_Message)
		m_MessageList.remove(static_cast<MessageWidget*>(o));
	else
		m_WidgetList.remove(o);
	delete o;
	m_pDoc->setModified();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::setFillColor(QColor color) {
	m_Options.uiState.fillColor = color;
	emit sigColorChanged( getID() );
	canvas()->setAllChanged();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::setLineColor(QColor color) {
	m_Options.uiState.lineColor = color;
	emit sigColorChanged( getID() );
	emit sigLineColorChanged( getID() );
	canvas() -> setAllChanged();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::setLineWidth(uint width) {
	m_Options.uiState.lineWidth = width;
	emit sigLineWidthChanged( getID() );
	canvas() -> setAllChanged();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::contentsMouseDoubleClickEvent(QMouseEvent* ome)
{
	m_pToolBarState->mouseDoubleClick(ome);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
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
	if(getType() == dt_Sequence ) {
		SeqLineWidget * pLine = 0;
		for( pLine = m_SeqLineList.first(); pLine; pLine = m_SeqLineList.next() ) {
			int y = pLine -> getObjectWidget() -> getEndLineY();
			endy = endy < y?y:endy;
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

	// Margin:
	startx -= 24;
	starty -= 20;
	endx += 24;
	endy += 20;

	return QRect(startx, starty,  endx - startx, endy - starty);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::clearSelected() {
	m_SelectedList.clear();
	emit sigClearAllSelected();
	//m_pDoc -> enableCutCopy(false);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::moveSelected(UMLWidget * w, int x, int y) {
	QMouseEvent me(QMouseEvent::MouseMove, QPoint(x,y), LeftButton, ShiftButton);
	UMLWidget * temp = 0;
	//loop through list and move all widgets
	//don't move the widget that started call
	for(temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next())
		if(temp != w)
			temp -> mouseMoveEvent(&me);

	// Move any selected associations.
	AssociationWidgetListIt assoc_it( m_AssociationList );
	AssociationWidget* assocwidget = NULL;
	while ((assocwidget = assoc_it.current()) != NULL) {
		++assoc_it;
		if (assocwidget->getSelected())
			assocwidget->moveEntireAssoc(x, y);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::selectionUseFillColor(bool useFC) {
	UMLWidget * temp = 0;
	for(temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next())
		temp -> setUseFillColour(useFC);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::selectionSetFont( QFont font )
{
	UMLWidget * temp = 0;
	for(temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next())
		temp -> setFont( font );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::selectionSetLineColor( QColor color )
{
	UMLWidget * temp = 0;
	for(temp=(UMLWidget *) m_SelectedList.first();
				temp;
					temp=(UMLWidget *)m_SelectedList.next()) {
		temp -> setLineColour( color );
		temp -> setUsesDiagramLineColour(false);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::selectionSetLineWidth( uint width )
{
	UMLWidget * temp = 0;
	for(temp=(UMLWidget *) m_SelectedList.first();
				temp;
					temp=(UMLWidget *)m_SelectedList.next()) {
		temp -> setLineWidth( width );
		temp -> setUsesDiagramLineWidth(false);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::selectionSetFillColor( QColor color )
{
	UMLWidget * temp = 0;
	for(temp=(UMLWidget *) m_SelectedList.first();
				temp;
					temp=(UMLWidget *)m_SelectedList.next()) {
		temp -> setFillColour( color );
		temp -> setUsesDiagramFillColour(false);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::selectionToggleShow(int sel)
{
	// loop through all selected items
	for(UMLWidget *temp = (UMLWidget *)m_SelectedList.first();
			temp; temp=(UMLWidget *)m_SelectedList.next()) {
		UMLWidget_Type type = temp->getBaseType();
		ClassifierWidget *cw = NULL;
		if (type == wt_Class || type == wt_Interface)
			cw = static_cast<ClassWidget *>(temp);

		// toggle the show setting sel
		switch (sel)
		{
			// some setting are only avaible for class, some for interface and some
			// for both
		   case ListPopupMenu::mt_Show_Attributes_Selection:
			if (type == wt_Class)
				(static_cast<ClassWidget*>(temp)) -> toggleShowAtts();
			break;
		   case ListPopupMenu::mt_Show_Operations_Selection:
			if (cw)
				cw -> toggleShowOps();
			break;
		   case ListPopupMenu::mt_Scope_Selection:
			if (cw)
				cw -> toggleShowScope();
			break;
		   case ListPopupMenu::mt_DrawAsCircle_Selection:
			if (type == wt_Interface)
				(static_cast<InterfaceWidget*>(temp)) -> toggleDrawAsCircle();
			break;
		   case ListPopupMenu::mt_Show_Operation_Signature_Selection:
			if (cw)
				cw -> toggleShowOpSigs();
			break;
		   case ListPopupMenu::mt_Show_Attribute_Signature_Selection:
			if (type == wt_Class)
				(static_cast<ClassWidget*>(temp)) -> toggleShowAttSigs();
			break;
		   case ListPopupMenu::mt_Show_Packages_Selection:
			if (cw)
				cw -> toggleShowPackage();
			break;
		   case ListPopupMenu::mt_Show_Stereotypes_Selection:
			if (type == wt_Class)
				(static_cast<ClassWidget*>(temp)) -> toggleShowStereotype();
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
////////////////////////////////////////////////////////////////////////////////////////////////////
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
			((FloatingText *)temp) -> getRole() != tr_Floating )
		{
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

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::selectAll()
{
	selectWidgets(0, 0, canvas()->width(), canvas()->height());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::contentsMousePressEvent(QMouseEvent* ome)
{
	m_pToolBarState->mousePress(ome);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::makeSelected (UMLWidget * uw) {
	if (uw == NULL)
		return;
	uw -> setSelected(true);
	m_SelectedList.remove(uw);  // make sure not in there
	m_SelectedList.append(uw);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::selectWidgets(int px, int py, int qx, int qy) {
	clearSelected();

	QRect rect;
	/*int px = m_Pos.x();
	int py = m_Pos.y();
	int qx = m_LineToPos.x();
	int qy = m_LineToPos.y();
	*/
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
		//made of points pos and m_LineToPos
		if( !rect.intersects(rect2) )
			continue;
		//if it is text that is part of an association then select the association
		//and the objects that are connected to it.
		if (temp -> getBaseType() == wt_Text) {
			FloatingText *ft = static_cast<FloatingText*>(temp);
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
		if( w -> getWidget(A) -> getSelected() &&
		        w -> getWidget(B) -> getSelected() ) {
			makeSelected( w );
		}//end if
	}//end while
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void  UMLView::getDiagram(const QRect &rect, QPixmap & diagram) {
	QPixmap pixmap(rect.x() + rect.width(), rect.y() + rect.height());
	QPainter painter(&pixmap);
	getDiagram(canvas()->rect(),painter);
	bitBlt(&diagram, QPoint(0, 0), &pixmap, rect);
}

void  UMLView::getDiagram(const QRect &area, QPainter & painter) {
	//unselect all before grab
	UMLWidget* temp = 0;
	for (temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next()) {
		temp -> setSelected(false);
	}
	if (m_pMoveAssoc) {
		m_pMoveAssoc -> setSelected( false );
	}

	// we don't want to get the grid
	bool showSnapGrid = getShowSnapGrid();
	setShowSnapGrid(false);

	canvas()->drawArea(area, &painter);

	setShowSnapGrid(showSnapGrid);

	canvas()->setAllChanged();
	//select again
	for (temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next()) {
		temp -> setSelected( true );
	}
	if (m_pMoveAssoc) {
		m_pMoveAssoc -> setSelected( true );
	}
	return;
}

void UMLView::updateNoteWidgets() {
	AssociationWidget * a = 0;

	AssociationWidgetListIt assoc_it( m_AssociationList );
	while((a = assoc_it.current())) {
		++assoc_it;
		if(a->getAssocType() != at_Anchor)
			continue;
		UMLWidget *wa = a->getWidget(A);
		UMLWidget *wb = a->getWidget(B);
		UMLWidget *destination= 0, *source = 0;
		bool copyText = false;
		if(wa->getBaseType() == wt_Note) {
			kdDebug() << "A note" << endl;
			source = wb;
			destination = wa;
			copyText = true;
		} else if(wb->getBaseType() == wt_Note) {
			kdDebug() << "B note" << endl;
			source = wa;
			destination = wb;
			copyText = true;
		}

		if(copyText == true && ((NoteWidget*)destination)->getLinkState() == true
		        && source->getUMLObject()->getDoc() != NULL) {
			((NoteWidget*)destination)->setDoc(source->getUMLObject()->getDoc());

		}
	}
}

QString imageTypeToMimeType(QString imagetype) {
	if (QString("BMP") == imagetype) return "image/x-bmp";
	if (QString("JPEG") == imagetype) return "image/jpeg";
	if (QString("PBM") == imagetype) return "image/x-portable-bitmap";
	if (QString("PGM") == imagetype) return "image/x-portable-greymap";
	if (QString("PNG") == imagetype) return "image/png";
	if (QString("PPM") == imagetype) return "image/x-portable-pixmap";
	if (QString("XBM") == imagetype) return "image/x-xbm";
	if (QString("XPM") == imagetype) return "image/x-xpm";
	if (QString("EPS") == imagetype) return "image/x-eps";
	if (QString("SVG") == imagetype) return "image/svg+xml";
	return QString::null;
}

QString mimeTypeToImageType(QString mimetype) {
	if (QString("image/x-bmp") == mimetype) return "BMP";
	if (QString("image/jpeg") == mimetype) return "JPEG";
	if (QString("image/x-portable-bitmap") == mimetype) return "PBM";
	if (QString("image/x-portable-greymap") == mimetype) return "PGM";
	if (QString("image/png") == mimetype) return "PNG";
	if (QString("image/x-portable-pixmap") == mimetype) return "PPM";
	if (QString("image/x-xbm") == mimetype) return "XBM";
	if (QString("image/x-xpm") == mimetype) return "XPM";
	if (QString("image/x-eps") == mimetype) return "EPS";
	if (QString("image/svg+xml") == mimetype) return "SVG";
	return QString::null;
}

void UMLView::fixEPS(QString filename, QRect rect) {
	// now open the file and make a correct eps out of it
	QFile epsfile(filename);
	QString fileContent;
	if (epsfile.open(IO_ReadOnly )) {
		// read
		QTextStream ts(&epsfile);
		fileContent = ts.read();
		epsfile.close();

		// write new content to file
		if (epsfile.open(IO_WriteOnly | IO_Truncate)) {
			// read information
			QRegExp rx("%%BoundingBox:\\s*(-?[\\d\\.]+)\\s*(-?[\\d\\.]+)\\s*(-?[\\d\\.]+)\\s*(-?[\\d\\.]+)");
			int pos = rx.search(fileContent);
			float left = rx.cap(1).toFloat();
			float top = rx.cap(4).toFloat();

			// modify content
			fileContent.replace(pos,rx.cap(0).length(),
					    QString("%%BoundingBox: %1 %2 %3 %4").arg(left).arg(top-rect.height()).arg(left+rect.width()).arg(top));

			ts << fileContent;
			epsfile.close();
		}
	}
}

void UMLView::printToFile(QString filename,bool isEPS) {
	// print the image to a normal postscript file,
	// do not clip so that everything ends up in the file
	// regardless of "paper size"

	// because we want to work with postscript
	// user-coordinates, set to the resolution
	// of the printer (which should be 72dpi here)
	QPrinter *printer = new QPrinter(QPrinter::PrinterResolution);
	printer->setOutputToFile(true);
	printer->setOutputFileName(filename);

	// do not call printer.setup(); because we want no user
	// interaction here
	QPainter *painter = new QPainter(printer);

	// make sure the widget sizes will be according to the
	// actually used printer font, important for getDiagramRect()
	// and the actual painting
	forceUpdateWidgetFontMetrics(painter);

	QRect rect = getDiagramRect();
	painter->translate(-rect.x(),-rect.y());
	getDiagram(rect,*painter);

	// delete painter and printer before we try to open and fix the file
	delete painter;
	delete printer;
	if (isEPS) fixEPS(filename,rect);
	// next painting will most probably be to a different device (i.e. the screen)
	forceUpdateWidgetFontMetrics(0);

}

void UMLView::exportImage() {
	UMLApp *app = UMLApp::app();
	QStringList fmt = QImage::outputFormatList();
	QString imageMimetype = "image/png";

	// get all supported mimetypes
	QStringList mimetypes;
	// special image types that are handled separately
	mimetypes.append("image/x-eps");
	mimetypes.append("image/svg+xml");
	// "normal" image types that are present
	QString m;
	QStringList::Iterator it;
	for( it = fmt.begin(); it != fmt.end(); ++it ) {
		m = imageTypeToMimeType(*it);
		if (!m.isNull()) mimetypes.append(m);
	}

	// configure & show the file dialog
	KFileDialog fileDialog(QString::null, QString::null, this,
			       ":export-image",true);
	fileDialog.setCaption(i18n("Save As"));
	fileDialog.setOperationMode(KFileDialog::Saving);
	if (app)
		imageMimetype = app->getImageMimetype();
	fileDialog.setMimeFilter(mimetypes, imageMimetype);

	// set a sensible default filename
	if (m_ImageURL.isEmpty()) {
	  fileDialog.setSelection(getName()+"."+mimeTypeToImageType(imageMimetype).lower());
	} else {
	  fileDialog.setURL(m_ImageURL);
	  fileDialog.setSelection(m_ImageURL.fileName());
	}
	fileDialog.exec();

	if (fileDialog.selectedURL().isEmpty())
		return;
	// save
	imageMimetype = fileDialog.currentMimeFilter();
	if (app) app->setImageMimetype(imageMimetype);
	m_ImageURL = fileDialog.selectedURL();

	QString s;
	KTempFile tmpfile;

	if (m_ImageURL.isLocalFile()) {
		QString file = m_ImageURL.path(-1);
		QFileInfo info(file);
		QString ext = info.extension(false);
		QString extDef = mimeTypeToImageType(imageMimetype).lower();
		if(ext != extDef) {
			m_ImageURL.setFileName(m_ImageURL.fileName() + "."+extDef);
		}
		s = m_ImageURL.path();
		info = QFileInfo(s);
		if (info.exists())
		{
			int want_save = KMessageBox::warningContinueCancel(0, i18n("The selected file %1 exists.\nDo you want to overwrite it?").arg(m_ImageURL.fileName()),
								i18n("File Already Exists"), i18n("&Overwrite"));
			if (want_save == KMessageBox::Cancel)
				// another possibility would be to show the save dlg again
				return;
		}
	} else {
		s = tmpfile.name();
	}

	QRect rect = getDiagramRect();
	if (rect.isEmpty()) {
		KMessageBox::sorry(0, i18n("Can not save an empty diagram"),
		                   i18n("Diagram Save Error."));
	} else {
		//  eps requested
		if (imageMimetype == "image/x-eps") {
			printToFile(s,true);
		} else if (imageMimetype == "image/svg+xml") {
			QPicture* diagram = new QPicture();

			// do not call printer.setup(); because we want no user
			// interaction here
			QPainter* painter = new QPainter();
			painter->begin( diagram );

			// make sure the widget sizes will be according to the
			// actually used printer font, important for getDiagramRect()
			// and the actual painting
			forceUpdateWidgetFontMetrics(painter);

			QRect rect = getDiagramRect();
			painter->translate(-rect.x(),-rect.y());
			getDiagram(rect,*painter);
			painter->end();
			diagram->save(s, mimeTypeToImageType(imageMimetype).ascii());

			// delete painter and printer before we try to open and fix the file
			delete painter;
			delete diagram;
			// next painting will most probably be to a different device (i.e. the screen)
			forceUpdateWidgetFontMetrics(0);
		} else {
			QPixmap diagram(rect.width(), rect.height());
			getDiagram(rect, diagram);
			diagram.save(s, mimeTypeToImageType(imageMimetype).ascii());
		}
		if (!m_ImageURL.isLocalFile()) {
			if (!KIO::NetAccess::upload( tmpfile.name(), m_ImageURL
#if KDE_IS_VERSION(3,1,90)
								, UMLApp::app()
#endif
						    )) {
				KMessageBox::error(0,
				                   i18n("There was a problem saving file: %1").arg(m_ImageURL.path()),
				                   i18n("Save Error"));
			}
			tmpfile.unlink();
		} //!isLocalFile
	} //rect.isEmpty()
}//exportImage()

void UMLView::slotActivate() {
	m_pDoc->changeCurrentView(getID());
}

UMLObjectList* UMLView::getUMLObjects() {
	int type;
	UMLObjectList* list = new UMLObjectList;
	list->setAutoDelete(FALSE);

	UMLWidgetListIt it( m_WidgetList );

	UMLWidget *obj;
	while ( (obj = it.current()) != 0 ) {
		++it;
		type = obj -> getBaseType();
		switch( type ) //use switch for easy future expansion
		{
			case wt_Actor:
			case wt_Class:
			case wt_Package:
			case wt_Component:
			case wt_Node:
			case wt_Artifact:
			case wt_UseCase:
			case wt_Object:
				list->append( obj->getUMLObject() );
				break;
		}
	}
	return list;
}

bool UMLView::activate() {
	UMLWidgetListIt it( m_WidgetList );
	UMLWidget *obj;

	//Activate Regular widgets then activate  messages
	while ( (obj = it.current()) != 0 ) {
		++it;
		//If this UMLWidget is already activated or is a MessageWidget then skip it
		if(obj->isActivated() || obj->getBaseType() == wt_Message)
			continue;
		if (!m_pDoc->loading() && !obj->activate())
			continue;
		obj -> setVisible( true );
	}//end while

	MessageWidgetListIt it2( m_MessageList );
	//Activate Message widgets
	while ( (obj = (UMLWidget*)it2.current()) != 0 ) {
		++it2;
		//If this MessageWidget is already activated then skip it
		if(obj->isActivated())
			continue;

		if(!m_pDoc->loading() || !obj->activate(m_pDoc->getChangeLog())) {
			kdDebug() << "Couldn't activate message widget" << endl;
			continue;
		}
		obj -> setVisible( true );

	}//end while

	//Activate All associationswidgets
	AssociationWidgetListIt assoc_it( m_AssociationList );
	AssociationWidget *assocwidget;
	//first get total count
	while((assocwidget = assoc_it.current())) {
		++assoc_it;
		if( assocwidget->isActivated() )
			continue;
		assocwidget->activate();
		if( m_PastePoint.x() != 0 ) {
			int x = m_PastePoint.x() - m_Pos.x();
			int y = m_PastePoint.y() - m_Pos.y();
			assocwidget -> moveEntireAssoc( x, y );
		}
	}//end while

	return true;
}


bool UMLView::getSelectedWidgets(UMLWidgetList&WidgetList)
{
	UMLWidget * temp = 0;
	int type;
	for(temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next()) {
		type = temp->getBaseType();
		if (type == wt_Text) {
			if( ((FloatingText*)temp)->getRole() == tr_Floating ) {
				WidgetList.append(temp);
			}
		} else {
			WidgetList.append(temp);
		}
	}//end for
	return true;
}

bool UMLView::getSelectedAssocs(AssociationWidgetList & assocWidgetList) {
	AssociationWidgetListIt assoc_it( m_AssociationList );
	AssociationWidget* assocwidget = 0;
	while((assocwidget=assoc_it.current())) {
		++assoc_it;
		if( assocwidget -> getSelected() )
			assocWidgetList.append(assocwidget);
	}
	return true;
}

bool UMLView::addWidget( UMLWidget * pWidget , bool isPasteOperation ) {
	if( !pWidget ) {
		return false;
	}
	if (!isPasteOperation && findWidget(pWidget->getID())) {
		kdDebug() << "UMLView::addWidget: Not adding "
			  << "(id=" << pWidget->getID()
			  << "/type=" << pWidget->getBaseType()
			  << "/name=" << pWidget->getName()
			  << ") because it's already there" << endl;
		return false;
	}
	UMLWidget_Type type = pWidget->getBaseType();
	//kdDebug() << "UMLView::addWidget called for basetype " << type << endl;
	IDChangeLog * log = m_pDoc -> getChangeLog();
	if( isPasteOperation && (!log || !m_pIDChangesLog)) {
		kdError()<<" Cant addWidget to view in paste op because a log is not open"<<endl;
		return false;
	}
	int wX = pWidget -> getX();
	int wY = pWidget -> getY();
	bool xIsOutOfRange = (wX <= 0 || wX >= FloatingText::restrictPositionMax);
	bool yIsOutOfRange = (wY <= 0 || wY >= FloatingText::restrictPositionMax);
	if (xIsOutOfRange || yIsOutOfRange) {
		QString name = pWidget->getName();
		if (name.isEmpty()) {
			FloatingText *ft = dynamic_cast<FloatingText*>(pWidget);
			if (ft)
				name = ft->getDisplayText();
		}
		kdDebug() << "UMLView::addWidget (" << name << " type="
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
		case wt_Datatype:
		case wt_Actor:
		case wt_UseCase:
			{
				int id = pWidget -> getID();
				int newID = log->findNewID( id );
				if( newID == -1 ) {  // happens after a cut
					if (id == -1)
						return false;
					newID = id; //don't stop paste
				} else
					pWidget -> setID( newID );
				UMLObject * pObject = m_pDoc -> findUMLObject( newID );
				if( !pObject ) {
					kdDebug() << "addWidget: Can't find UMLObject for id "
						  << newID << endl;
					return false;
				}
				pWidget -> setUMLObject( pObject );
				//make sure it doesn't already exist.
				if( findWidget( newID ) )
					return true;//don't stop paste just because widget found.
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
				int newID = m_pDoc->assignNewID( pWidget->getID() );
				pWidget->setID(newID);
				if (type != wt_Message) {
					m_WidgetList.append( pWidget );
					return true;
				}
				// CHECK
				// Handling of wt_Message:
				MessageWidget *pMessage = static_cast<MessageWidget *>( pWidget );
				if (pMessage == NULL) {
					kdDebug() << "UMLView::addWidget(): pMessage is NULL" << endl;
					return false;
				}
				ObjectWidget *objWidgetA = pMessage -> getWidget(A);
				ObjectWidget *objWidgetB = pMessage -> getWidget(B);
				int waID = objWidgetA -> getLocalID();
				int wbID = objWidgetB -> getLocalID();
				int newWAID = m_pIDChangesLog ->findNewID( waID );
				int newWBID = m_pIDChangesLog ->findNewID( wbID );
				if( newWAID == -1 || newWBID == -1 ) {
					kdDebug() << "Error with ids : " << newWAID << " " << newWBID << endl;
					return false;
				}
				// Assumption here is that the A/B objectwidgets and the textwidget
				// are pristine in the sense that we may freely change their local IDs.
				objWidgetA -> setLocalID( newWAID );
				objWidgetB -> setLocalID( newWBID );
				FloatingText *ft = pMessage->getFloatingText();
				if (ft == NULL)
					kdDebug() << "UMLView::addWidget: FloatingText of Message is NULL" << endl;
				else if (ft->getID() == -1)
					ft->setID( m_pDoc->getUniqueID() );
				else {
					int newTextID = m_pDoc->assignNewID( ft->getID() );
					ft->setID( newTextID );
				}
				m_MessageList.append( pMessage );
			}
			break;

		case wt_Object:
			{
				ObjectWidget* pObjectWidget = static_cast<ObjectWidget*>(pWidget);
				if (pObjectWidget == NULL) {
					kdDebug() << "UMLView::addWidget(): pObjectWidget is NULL" << endl;
					return false;
				}
				int newID = log->findNewID( pWidget -> getID() );
				if (newID == -1) {
					return false;
				}
				pObjectWidget -> setID( newID );
				int nNewLocalID = getLocalID();
				int nOldLocalID = pObjectWidget -> getLocalID();
				m_pIDChangesLog->addIDChange( nOldLocalID, nNewLocalID );
				pObjectWidget -> setLocalID( nNewLocalID );
				UMLObject *pObject = m_pDoc -> findUMLObject( newID );
				if( !pObject ) {
					kdDebug() << "addWidget::Can't find UMLObject" << endl;
					return false;
				}
				pWidget -> setUMLObject( pObject );
				m_WidgetList.append( pWidget );
			}
			break;

		default:
			kdDebug() << "Trying to add an invalid widget type" << endl;
			return false;
			break;
	}

	return true;
}

// Add the association, and its child widgets to this view
bool UMLView::addAssociation( AssociationWidget* pAssoc , bool isPasteOperation) {

	if(!pAssoc)
		return false;

	if( isPasteOperation )
	{
		IDChangeLog * log = m_pDoc -> getChangeLog();

		if(!log )
			return false;

		int ida = -1, idb = -1;
		Association_Type type = pAssoc -> getAssocType();
		IDChangeLog* localLog = getLocalIDChangeLog();
		if( getType() == dt_Collaboration || getType() == dt_Sequence ) {
			//check local log first
			ida = localLog->findNewID( pAssoc->getWidgetID(A) );
			idb = localLog->findNewID( pAssoc->getWidgetID(B) );
			//if either is still not found and assoc type is anchor
			//we are probably linking to a notewidet - else an error
			if( ida == -1 && type == at_Anchor )
				ida = log->findNewID(pAssoc->getWidgetID(A));
			if( idb == -1 && type == at_Anchor )
				idb = log->findNewID(pAssoc->getWidgetID(B));
		} else {
			int oldIdA = pAssoc->getWidgetID(A);
			int oldIdB = pAssoc->getWidgetID(B);
			ida = log->findNewID( oldIdA );
			if (ida == -1) {  // happens after a cut
				if (oldIdA == -1)
					return false;
				ida = oldIdA;
			}
			idb = log->findNewID( oldIdB );
			if (idb == -1) {  // happens after a cut
				if (oldIdB == -1)
					return false;
				idb = oldIdB;
			}
		}
		if(ida == -1 || idb == -1) {
			return false;
		}
		// cant do this anymore.. may cause problem for pasting
//		pAssoc->setWidgetID(ida, A);
//		pAssoc->setWidgetID(idb, B);
		pAssoc->setWidget(findWidget(ida), A);
		pAssoc->setWidget(findWidget(idb), B);
	}

	UMLWidget * m_pWidgetA = findWidget(pAssoc->getWidgetID(A));
	UMLWidget * m_pWidgetB = findWidget(pAssoc->getWidgetID(B));
	//make sure valid widget ids
	if(!m_pWidgetA || !m_pWidgetB)
		return false;

	//make sure valid
	if( !isPasteOperation &&
	    !AssocRules::allowAssociation(pAssoc->getAssocType(), m_pWidgetA, m_pWidgetB, false) ) {
		kdDebug() << "UMLView::addAssociation: allowAssociation returns false "
			  << "for AssocType " << pAssoc->getAssocType() << endl;
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

	FloatingText *pNameWidget = pAssoc->getNameWidget();
	FloatingText *pRoleAWidget = pAssoc->getRoleWidget(A);
	FloatingText *pRoleBWidget = pAssoc->getRoleWidget(B);
	FloatingText *pMultiAWidget = pAssoc->getMultiWidget(A);
	FloatingText *pMultiBWidget = pAssoc->getMultiWidget(B);

	if(pNameWidget)
		addWidget(pNameWidget);
	if(pRoleAWidget)
		addWidget(pRoleAWidget);
	if(pRoleBWidget)
		addWidget(pRoleBWidget);
	if(pMultiAWidget)
		addWidget(pMultiAWidget);
	if(pMultiBWidget)
		addWidget(pMultiBWidget);

	return true;
}

void UMLView::addAssocInViewAndDoc(AssociationWidget* a) {

	// append in view
	if(addAssociation(a, false))
	{
		// if view went ok, then append in document
		getDocument() -> addAssociation (a->getAssociation());
	} else {
		kdError() << "cannot addAssocInViewAndDoc(), deleting" << endl;
		delete a;
	}

}

bool UMLView::activateAfterLoad(bool bUseLog) {
	bool status = true;
	if ( !m_bActivated ) {
		if( bUseLog ) {
			beginPartialWidgetPaste();
		}

		//now activate them all
		status = activate();

		if( bUseLog ) {
			endPartialWidgetPaste();
		}
		resizeCanvasToItems();
		setZoom( getZoom() );
	}//end if active
	if(status) {
		m_bActivated = true;
	}
	return true;
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

IDChangeLog* UMLView::getLocalIDChangeLog() {
	return m_pIDChangesLog;
}

void UMLView::removeAssoc(AssociationWidget* pAssoc) {
	if(!pAssoc)
		return;
	if( pAssoc == m_pMoveAssoc ) {
		UMLApp::app() -> getDocWindow() -> updateDocumentation( true );
		m_pMoveAssoc = 0;
	}
	// Remove the association in this view.
//	pAssoc->cleanup();
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
	// Remove assoc in doc.
	m_pDoc->removeAssociation(a->getAssociation());
	// Remove assoc in view.
	removeAssoc(a);
}

bool UMLView::setAssoc(UMLWidget *pWidget) {
	Association_Type type = convert_TBB_AT(m_CurrentCursor);
	m_bDrawRect = false;
	m_SelectionRect.clear();
	//if this we are not concerned here so return
	if (m_CurrentCursor < WorkToolBar::tbb_Generalization ||
	    m_CurrentCursor > WorkToolBar::tbb_Anchor) {
		return false;
	}
	clearSelected();

	if(!m_pFirstSelectedWidget) {
		if( !AssocRules::allowAssociation( type, pWidget ) ) {
			KMessageBox::error(0, i18n("Incorrect use of associations."), i18n("Association Error"));
			return false;
		}
		//set up position
		QPoint pos;
		pos.setX(pWidget -> getX() + (pWidget->getWidth() / 2));

		pos.setY(pWidget -> getY() + (pWidget->getHeight() / 2));
		setPos(pos);
		m_LineToPos = pos;
		m_pFirstSelectedWidget = pWidget;
		viewport() -> setMouseTracking( true );

		// TODO Reachable?
		if( m_pAssocLine )
		{
			kdDebug() << "delete m_pAssocLine is reachable" << endl;
			delete m_pAssocLine;
			m_pAssocLine = NULL;
		}

		m_pAssocLine = new QCanvasLine( canvas() );
		m_pAssocLine -> setPoints( pos.x(), pos.y(), pos.x(), pos.y() );
		m_pAssocLine -> setPen( QPen( getLineColor(), getLineWidth(), DashLine ) );

		m_pAssocLine -> setVisible( true );

		return true;
	}
	// If we get here we have a FirstSelectedWidget.
	// The following reassignment is just to make things clearer.
	UMLWidget* widgetA = m_pFirstSelectedWidget;
	UMLWidget* widgetB = pWidget;
	UMLWidget_Type at = widgetA -> getBaseType();
	bool valid = true;
	if (type == at_Generalization)
		type = AssocRules::isGeneralisationOrRealisation(widgetA, widgetB);
	if (widgetA == widgetB)
	{
		valid = AssocRules::allowSelf( type, at );
		if(valid && type == at_Association)
		{
			type = at_Association_Self;
		}
	}
	else
		valid =  AssocRules::allowAssociation( type, widgetA, widgetB );
	if( valid ) {
		AssociationWidget *temp = new AssociationWidget(this, widgetA, type, widgetB);
		addAssocInViewAndDoc(temp);
		if (type == at_Containment) {
			UMLListView *lv = UMLApp::app()->getListView();
			UMLObject *newContainer = widgetA->getUMLObject();
			UMLObject *objToBeMoved = widgetB->getUMLObject();
			if (newContainer && objToBeMoved) {
				UMLListViewItem *newLVParent = lv->findUMLObject(newContainer);
				UMLObject_Type ot = objToBeMoved->getBaseType();
				lv->moveObject( objToBeMoved->getID(),
						UMLListView::convert_OT_LVT(ot),
						newLVParent );
			}
		}
		m_pDoc->setModified();
	} else {
		KMessageBox::error(0, i18n("Incorrect use of associations."), i18n("Association Error"));
	}
	m_pFirstSelectedWidget = 0;

	if( m_pAssocLine ) {
		delete m_pAssocLine;
		m_pAssocLine = 0;
	}
	return valid;
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
		if(assocwidget->getWidget(A)->getUMLObject() == Obj
		        || assocwidget->getWidget(B)->getUMLObject() == Obj)
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
		      ((FloatingText *)temp)-> getRole() != tr_Floating ) ) {
			removeWidget( temp );
		}
	}
	m_WidgetList.clear();
}


WorkToolBar::ToolBar_Buttons UMLView::getCurrentCursor() const {
	return m_CurrentCursor;
}


Uml::Association_Type UMLView::convert_TBB_AT(WorkToolBar::ToolBar_Buttons tbb) {
	Association_Type at = at_Unknown;
	switch(tbb) {
		case WorkToolBar::tbb_Anchor:
			at = at_Anchor;
			break;

		case WorkToolBar::tbb_Association:
			at = at_Association;
			break;

		case WorkToolBar::tbb_UniAssociation:
			at = at_UniAssociation;

			break;

		case WorkToolBar::tbb_Generalization:
			at = at_Generalization;
			break;

		case WorkToolBar::tbb_Composition:
			at = at_Composition;
			break;

		case WorkToolBar::tbb_Aggregation:
			at = at_Aggregation;
			break;

		case WorkToolBar::tbb_Dependency:
			at = at_Dependency;
			break;

		case WorkToolBar::tbb_Containment:
			at = at_Containment;
			break;

		case WorkToolBar::tbb_Seq_Message_Synchronous:
		case WorkToolBar::tbb_Seq_Message_Asynchronous:
			at = at_Seq_Message;
			break;

		case WorkToolBar::tbb_Coll_Message:
			at = at_Coll_Message;
			break;

		case WorkToolBar::tbb_State_Transition:
			at = at_State;
			break;

		case WorkToolBar::tbb_Activity_Transition:
			at = at_Activity;
			break;

	        default:
			break;
	}
	return at;
}

void UMLView::showDocumentation( UMLObject * object, bool overwrite ) {
	UMLApp::app() -> getDocWindow() -> showDocumentation( object, overwrite );
}

void UMLView::showDocumentation( UMLWidget * widget, bool overwrite ) {
	UMLApp::app() -> getDocWindow() -> showDocumentation( widget, overwrite );
}

void UMLView::showDocumentation( AssociationWidget * widget, bool overwrite ) {
	UMLApp::app() -> getDocWindow() -> showDocumentation( widget, overwrite );
}

void UMLView::updateDocumentation( bool clear ) {
	UMLApp::app() -> getDocWindow() -> updateDocumentation( clear );
}

void UMLView::createAutoAssociations( UMLWidget * widget ) {
	if( !widget )
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
	//   if this object is capable of containing nested objects then
	//     for each of the object's containedObjects
	//       if the containedObject has a widget representation on this view then
	//         create the containment AssocWidget
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
	int myID = umlObj->getID();
	while ((assoc = it.current()) != NULL) {
		++it;
		UMLCanvasObject *other = NULL;
		UMLObject *roleAObj = assoc->getObject(A);
		if (roleAObj == NULL) {
			kdDebug() << "createAutoAssociations: roleA object is NULL at UMLAssoc "
				  << assoc->getID() << endl;
			continue;
		}
		UMLObject *roleBObj = assoc->getObject(B);
		if (roleBObj == NULL) {
			kdDebug() << "createAutoAssociations: roleB object is NULL at UMLAssoc "
				  << assoc->getID() << endl;
			continue;
		}
		if (roleAObj->getID() == myID) {
			other = static_cast<UMLCanvasObject*>(roleBObj);
		} else if (roleBObj->getID() == myID) {
			other = static_cast<UMLCanvasObject*>(roleAObj);
		} else {
			kdDebug() << "createAutoAssociations: Can't find own object "
				  << myID << " in UMLAssoc " << assoc->getID() << endl;
			continue;
		}
		// Now that we have determined the "other" UMLObject, seek it in
		// this view's UMLWidgets.
		int otherID = other->getID();
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
			kdDebug() << "createAutoAssociations: not transferring assoc "
				  << "of type " << assocType << endl;
			continue;
		}
		// Create the AssociationWidget.
		assocwidget = new AssociationWidget( this );
		assocwidget->setWidget(widgetA, A);
		assocwidget->setWidget(widgetB, B);
		assocwidget->setAssocType(assocType);
		// Call calculateEndingPoints() before setting the FloatingTexts
		// because their positions are computed according to the
		// assocwidget line positions.
		assocwidget->calculateEndingPoints();
		assocwidget->setVisibility(assoc->getVisibility(A), A);
		assocwidget->setVisibility(assoc->getVisibility(B), B);
		assocwidget->setChangeability(assoc->getChangeability(A), A);
		assocwidget->setChangeability(assoc->getChangeability(B), B);
		assocwidget->setMulti(assoc->getMulti(A), A);
		assocwidget->setMulti(assoc->getMulti(B), B);
		assocwidget->setRoleName(assoc->getRoleName(A), A);
		assocwidget->setRoleName(assoc->getRoleName(B), B);
		assocwidget->setActivated(true);
		if (! addAssociation(assocwidget))
			delete assocwidget;
	}
	// if this object is capable of containing nested objects then
	Uml::UMLObject_Type t = umlObj->getBaseType();
	if (t == ot_Package || t == ot_Class || t == ot_Interface) {
		// for each of the object's containedObjects
		UMLPackage *umlPkg = static_cast<UMLPackage*>(umlObj);
		UMLObjectList& lst = umlPkg->containedObjects();
		for (UMLObject *obj = lst.first(); obj; obj = lst.next()) {
			// if the containedObject has a widget representation on this view then
			int id = obj->getID();
			for (UMLWidget *w = m_WidgetList.first(); w; w = m_WidgetList.next()) {
				if (w->getID() != id)
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
	int pkgID = parent->getID();
	UMLWidget *pWidget;
	UMLWidgetListIt wit(m_WidgetList);
	while ((pWidget = wit.current()) != NULL) {
		++wit;
		if (pWidget->getID() == pkgID)
			break;
	}
	if (pWidget == NULL)
		return;
	// create the containment AssocWidget
	AssociationWidget *a = new AssociationWidget(this, pWidget, at_Containment, widget);
	a->calculateEndingPoints();
	a->setActivated(true);
	if (! addAssociation(a))
		delete a;
}

void UMLView::findMaxBoundingRectangle(const FloatingText* ft, int& px, int& py, int& qx, int& qy)
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
		const FloatingText* multiA = const_cast<FloatingText*>(a->getMultiWidget(A));
		const FloatingText* multiB = const_cast<FloatingText*>(a->getMultiWidget(B));
		const FloatingText* roleA = const_cast<FloatingText*>(a->getRoleWidget(A));
		const FloatingText* roleB = const_cast<FloatingText*>(a->getRoleWidget(B));
		const FloatingText* changeA = const_cast<FloatingText*>(a->getChangeWidget(A));
		const FloatingText* changeB = const_cast<FloatingText*>(a->getChangeWidget(B));
		findMaxBoundingRectangle(multiA, px, py, qx, qy);
		findMaxBoundingRectangle(multiB, px, py, qx, qy);
		findMaxBoundingRectangle(roleA, px, py, qx, qy);
		findMaxBoundingRectangle(roleB, px, py, qx, qy);
		findMaxBoundingRectangle(changeA, px, py, qx, qy);
		findMaxBoundingRectangle(changeB, px, py, qx, qy);
	}//end while

	QRect imageRect;  //area with respect to getDiagramRect()
	                  //i.e. all widgets on the canvas.  Was previously with
	                  //repect to whole canvas

	imageRect.setLeft( px - rect.left() );
	imageRect.setTop( py - rect.top() );
	imageRect.setRight( qx - rect.left() );
	imageRect.setBottom( qy - rect.top() );

	pix = new QPixmap(imageRect.width(), imageRect.height());
	bitBlt(pix, QPoint(0, 0), &diagram, imageRect);
	m_bDrawSelectedOnly = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
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

		default:
			kdWarning() << "setMenu() called on unknown diagram type" << endl;
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
	FloatingText * ft = 0;
	StateWidget * state = 0;
	ActivityWidget * activity = 0;
	bool ok = false;
	QString name = "";

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
			exportImage();
			break;

		case ListPopupMenu::mt_FloatText:
			ft = new FloatingText(this);
			ft -> changeTextDlg();
			//if no text entered delete
			if(!FloatingText::isTextValid(ft -> getText()))

				delete ft;
			else {
				ft->setX( m_Pos.x() );
				ft->setY( m_Pos.y() );
				ft->setVisible( true );
				ft->setID(m_pDoc -> getUniqueID());
				ft->setActivated();
				setupNewWidget(ft);
			}
			break;

		case ListPopupMenu::mt_UseCase:
			m_bCreateObject = true;
			m_pDoc->createUMLObject( ot_UseCase );
			break;

		case ListPopupMenu::mt_Actor:
			m_bCreateObject = true;
			m_pDoc->createUMLObject( ot_Actor );
			break;

		case ListPopupMenu::mt_Class:
		case ListPopupMenu::mt_Object:
			m_bCreateObject = true;
			m_pDoc->createUMLObject( ot_Class);
			break;

		case ListPopupMenu::mt_Package:
			m_bCreateObject = true;
			m_pDoc->createUMLObject(ot_Package);
			break;

		case ListPopupMenu::mt_Component:
			m_bCreateObject = true;
			m_pDoc->createUMLObject(ot_Component);
			break;

		case ListPopupMenu::mt_Node:
			m_bCreateObject = true;
			m_pDoc->createUMLObject(ot_Node);
			break;

		case ListPopupMenu::mt_Artifact:
			m_bCreateObject = true;
			m_pDoc->createUMLObject(ot_Artifact);
			break;

		case ListPopupMenu::mt_Interface:
			m_bCreateObject = true;
			m_pDoc->createUMLObject(ot_Interface);
			break;

		case ListPopupMenu::mt_Enum:
			m_bCreateObject = true;
			m_pDoc->createUMLObject(ot_Enum);
			break;

		case ListPopupMenu::mt_Datatype:
			m_bCreateObject = true;
			m_pDoc->createUMLObject(ot_Datatype);
			break;

		case ListPopupMenu::mt_Cut:
			if ( m_SelectedList.count() &&
			     UMLApp::app()->editCutCopy(true) ) {
				deleteSelection();
				m_pDoc->setModified(true);
			}
			break;

		case ListPopupMenu::mt_Copy:
			break;

		case ListPopupMenu::mt_Paste:
			m_PastePoint = m_Pos;
			m_Pos.setX( 2000 );
			m_Pos.setY( 2000 );
			UMLApp::app() -> slotEditPaste();

			m_PastePoint.setX( 0 );
			m_PastePoint.setY( 0 );
			break;

		case ListPopupMenu::mt_Initial_State:
			state = new StateWidget( this, StateWidget::Initial );
			state -> setX( m_Pos.x() );
			state -> setY ( m_Pos.y() );
			state -> setVisible( true );
			state -> setActivated();
			setupNewWidget( state );
			break;

		case ListPopupMenu::mt_End_State:
			state = new StateWidget( this, StateWidget::End );
			state -> setX( m_Pos.x() );
			state -> setY ( m_Pos.y() );
			state -> setVisible( true );
			state -> setActivated();
			setupNewWidget( state );
			break;

		case ListPopupMenu::mt_State:
			name = KInputDialog::getText( i18n("Enter State Name"), i18n("Enter the name of the new state:"), i18n("new state"), &ok, UMLApp::app() );
			if( ok ) {
				state = new StateWidget( this );
				state -> setName( name );
				state -> setX( m_Pos.x() );
				state -> setY ( m_Pos.y() );
				state -> setVisible( true );
				state -> setActivated();
				setupNewWidget( state );
			}
			break;

		case ListPopupMenu::mt_Initial_Activity:
			activity = new ActivityWidget( this, ActivityWidget::Initial );
			activity -> setX( m_Pos.x() );
			activity -> setY ( m_Pos.y() );
			activity -> setVisible( true );
			activity -> setActivated();
			setupNewWidget(activity);
			break;


		case ListPopupMenu::mt_End_Activity:
			activity = new ActivityWidget( this, ActivityWidget::End );
			activity -> setX( m_Pos.x() );
			activity -> setY ( m_Pos.y() );
			activity -> setVisible( true );
			activity -> setActivated();
			setupNewWidget(activity);
			break;

		case ListPopupMenu::mt_Branch:
			activity = new ActivityWidget( this, ActivityWidget::Branch );
			activity -> setX( m_Pos.x() );
			activity -> setY ( m_Pos.y() );
			activity -> setVisible( true );
			activity -> setActivated();
			setupNewWidget(activity);
			break;

		case ListPopupMenu::mt_Activity:
			name = KInputDialog::getText( i18n("Enter Activity Name"), i18n("Enter the name of the new activity:"),
						      i18n("new activity"), &ok, UMLApp::app() );
			if( ok ) {
				activity = new ActivityWidget( this, ActivityWidget::Normal );
				activity -> setName( name );
				activity -> setX( m_Pos.x() );
				activity -> setY ( m_Pos.y() );
				activity -> setVisible( true );
				activity -> setActivated();
				setupNewWidget(activity);
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


void UMLView::setFont( QFont font ) {
	m_Options.uiState.font = font;
}

void UMLView::setClassWidgetOptions( ClassOptionsPage * page ) {
	UMLWidget * pWidget = 0;
	UMLWidgetListIt wit( m_WidgetList );
	while ( (pWidget = wit.current()) != 0 ) {
		++wit;
		if( pWidget -> getBaseType() == Uml::wt_Class ) {
			page -> setWidget( static_cast<ClassWidget *>( pWidget ) );
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
	UMLWidget_Type tmpType = pTemp -> getBaseType();

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

void UMLView::setShowSnapGrid(bool bShow) {
	m_bShowSnapGrid = bShow;
	canvas()->setAllChanged();
	emit sigShowGridToggled( getShowSnapGrid() );
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
	viewElement.setAttribute( "xmi.id", m_nID );
	viewElement.setAttribute( "name", m_Name );
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
	viewElement.setAttribute( "showscope", m_Options.classState.showScope );
	viewElement.setAttribute( "showstereotype", m_Options.classState.showStereoType );
	//misc
	viewElement.setAttribute( "localid", m_nLocalID );
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
		    static_cast<FloatingText*>(widget)->getLink() == NULL)
			widget -> saveToXMI( qDoc, widgetElement );
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
		// kdDebug() << "UMLView::saveToXMI() saved "
		//	<< m_AssociationList.count() << " assocData." << endl;
	}
	viewElement.appendChild( assocElement );
	qElement.appendChild( viewElement );
}

bool UMLView::loadFromXMI( QDomElement & qElement ) {
	QString id = qElement.attribute( "xmi.id", "-1" );
	m_nID = id.toInt();
	if( m_nID == -1 )
		return false;
	m_Name = qElement.attribute( "name", "" );
	QString type = qElement.attribute( "type", "-1" );
	m_Documentation = qElement.attribute( "documentation", "" );
	QString localid = qElement.attribute( "localid", "0" );
	//optionstate uistate
	QString font = qElement.attribute( "font", "" );
	if( !font.isEmpty() )
		m_Options.uiState.font.fromString( font );
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
	m_Options.classState.showScope = (bool)temp.toInt();
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

	m_Type = (Uml::Diagram_Type)type.toInt();
	if( !fillcolor.isEmpty() )
		m_Options.uiState.fillColor = QColor( fillcolor );
	if( !linecolor.isEmpty() )
		m_Options.uiState.lineColor = QColor( linecolor );
	if( !linewidth.isEmpty() )
		m_Options.uiState.lineWidth = linewidth.toInt();
	m_nLocalID = localid.toInt();
	//load the widgets
	QDomNode node = qElement.firstChild();
	QDomElement element = node.toElement();
	if( !element.isNull() && element.tagName() != "widgets" )
		return false;
	if( !loadWidgetsFromXMI( element ) ) {
		kdWarning() << "failed umlview load on widgets" << endl;
		return false;
	}

	//load the message widgets
	node = element.nextSibling();
	element = node.toElement();
	if( !element.isNull() && element.tagName() != "messages" )
		return false;
	if( !loadMessagesFromXMI( element ) ) {
		kdWarning() << "failed umlview load on messages" << endl;
		return false;
	}

	//load the associations
	node = element.nextSibling();
	element = node.toElement();
	if( !element.isNull() && element.tagName() != "associations" )
		return false;
	if( !loadAssociationsFromXMI( element ) ) {
		kdWarning() << "failed umlview load on associations" << endl;
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
		kdWarning() << "UMLView::loadWidgetFromXMI(): m_pDoc is NULL" << endl;
		return 0L;
	}

	UMLWidget* widget = 0;
	QString tag  = widgetElement.tagName();

	if (tag == "statewidget" || tag == "notewidget" || tag == "boxwidget" ||
	    tag == "floatingtext" || tag == "activitywidget" ||
	    // tests for backward compatibility:
	    tag == "UML:StateWidget" || tag == "UML:NoteWidget" ||
	    tag == "UML:FloatingTextWidget" || tag == "UML:ActivityWidget")
	{
	// Loading of widgets wich do NOT reprsent any UMLObject, --> just graphic stuff with
	// no real model information
	//FIXME while boxes and texts are just diagram objects, activities and states should
	// be UMLObjects
		if (tag == "statewidget"
		    || tag == "UML:StateWidget") {         // for bkwd compatibility
			widget = new StateWidget(this, StateWidget::Normal, 0);
		} else if (tag == "notewidget"
		    || tag == "UML:NoteWidget") {          // for bkwd compatibility
			widget = new NoteWidget(this, 0);
		} else if (tag == "boxwidget") {
			widget = new BoxWidget(this, 0);
		} else if (tag == "floatingtext"
		    || tag == "UML:FloatingTextWidget") {  // for bkwd compatibility
			widget = new FloatingText(this, Uml::tr_Floating, "", 0);
		} else if (tag == "activitywidget"
		    || tag == "UML:ActivityWidget") {      // for bkwd compatibility
			widget = new ActivityWidget(this, ActivityWidget::Normal, 0);
		}
	}
	else
	{
	// Find the UMLObject and create the Widget to represent it
		QString idstr  = widgetElement.attribute( "xmi.id", "-1" );
		int id = idstr.toInt();
		UMLObject *o(0);
		if( id < 0 || !( o = m_pDoc->findUMLObject(id)) )
		{
			kdWarning()<<"UMLView::loadWidgetFromXMI( ) - ERROR - cannot find Object with id "<<id<<endl;
			return 0L;
		}

		if (tag == "actorwidget"
		    || tag == "UML:ActorWidget") {           // for bkwd compatibility
			widget = new ActorWidget(this, static_cast<UMLActor*>(o));
		} else if (tag == "usecasewidget"
			   || tag == "UML:UseCaseWidget") {  // for bkwd compatibility
			widget = new UseCaseWidget(this, static_cast<UMLUseCase*>(o));
		} else if (tag == "classwidget"
		           || tag == "UML:ClassWidget"       // for bkwd compatibility
			   || tag == "UML:ConceptWidget") {  // for bkwd compatibility
			widget = new ClassWidget(this, static_cast<UMLClass*>(o));
		} else if (tag == "packagewidget") {
			widget = new PackageWidget(this, static_cast<UMLPackage*>(o));
		} else if (tag == "componentwidget") {
			widget = new ComponentWidget(this, static_cast<UMLComponent*>(o));
		} else if (tag == "nodewidget") {
			widget = new NodeWidget(this, static_cast<UMLNode*>(o));
		} else if (tag == "artifactwidget") {
			widget = new ArtifactWidget(this, static_cast<UMLArtifact*>(o));
		} else if (tag == "interfacewidget") {
			widget = new InterfaceWidget(this, static_cast<UMLInterface*>(o));
		} else if (tag == "datatypewidget") {
			widget = new DatatypeWidget(this, static_cast<UMLDatatype*>(o));
		} else if (tag == "enumwidget") {
			widget = new EnumWidget(this, static_cast<UMLEnum*>(o));
		} else if (tag == "objectwidget"
			   || tag == "UML:ObjectWidget") {  // for bkwd compatibility
			widget = new ObjectWidget(this, o );
		} else {
			kdWarning() << "Trying to create an unknown widget:" << tag << endl;
			return 0L;
		}
	}
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
			message = new MessageWidget(this, sequence_message_asynchronous, 0);
			if( !message -> loadFromXMI( messageElement ) ) {
				delete message;
				return false;
			}
			m_MessageList.append( message );
			FloatingText *ft = message->getFloatingText();
			if (ft)
				m_WidgetList.append( ft );
			else
				kdError() << "UMLView::loadMessagesFromXMI: ft is NULL"
					  << " for message " << message->getID() << endl;
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
				kdError() << "couldn't loadFromXMI association widget:"
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
					kdError()<<"Couldnt addAssociation("<<assoc<<") to umlview, deleting."<<endl;
//					assoc->cleanup();
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
	m_pDoc->addObject(object);
}


#include "umlview.moc"
