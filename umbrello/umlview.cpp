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
#include <klineeditdlg.h>
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

#include "clipboard/idchangelog.h"
#include "clipboard/umldrag.h"

#include "classwidget.h"
#include "packagewidget.h"
#include "componentwidget.h"
#include "nodewidget.h"
#include "artifactwidget.h"
#include "interfacewidget.h"
#include "actorwidget.h"
#include "usecasewidget.h"
#include "notewidget.h"
#include "boxwidget.h"
#include "associationwidget.h"
#include "associationwidgetdata.h"
#include "objectwidget.h"
#include "floatingtext.h"
#include "messagewidget.h"
#include "statewidget.h"
#include "activitywidget.h"
#include "seqlinewidget.h"

#include "umllistviewitemdatalist.h"
#include "umllistviewitemdata.h"
#include "umlobjectlist.h"

#include "umlwidgetdata.h"
#include "floatingtextdata.h"

// static members
const int UMLView::defaultCanvasSize = 1300;


// constructor
UMLView::UMLView(QWidget* parent, UMLViewData* pData, UMLDoc* doc) : QCanvasView(parent, "AnUMLView" ) {
	m_pData = pData;
	m_pDoc = doc;
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::init() {
	//Setup up booleans
	m_bPaste = false;
	m_bDrawRect = false;
	m_bActivated = false;
	m_bCreateObject = false;
	m_bDrawSelectedOnly = false;
	m_bPopupShowing = false;
	m_bStartedCut = false;
	m_bMouseButtonPressed = false;
	//clear pointers
	m_pMoveAssoc = 0;
	m_pOnWidget = 0;
	m_pAssocLine = 0;
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
	m_SelectionRect.setAutoDelete( true );
	m_CurrentCursor = WorkToolBar::tbb_Arrow;
	//setup signals
	connect( this, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu() ) );
	connect( UMLApp::app(), SIGNAL( sigCutSuccessful() ),
	         this, SLOT( slotCutSuccessful() ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLView::~UMLView() {
	if( m_pData )
		delete m_pData;
	if(m_pIDChangesLog) {
		delete    m_pIDChangesLog;
		m_pIDChangesLog = 0;
	}
	if( m_pAssocLine )
		delete m_pAssocLine;
	m_SelectionRect.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLDoc* UMLView::getDocument() const {
	return m_pDoc;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::print(KPrinter *pPrinter, QPainter & pPainter) {
	int height, width;
	int offsetX = 0, offsetY = 0, widthX = 0, heightY = 0;
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

	// respect the margin
	pPainter.translate(marginX, marginY);

	// clip away everything outside of the margin
	pPainter.setClipRect(marginX, marginY,
						 width, metrics.height() - marginY * 2);

	//loop until all of the picture is printed
	int numPagesX = (int)ceilf((float)rect.width()/(float)width);
	int numPagesY = (int)ceilf((float)rect.height()/(float)height);
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
	// next painting will most probably be to a different device (i.e. the screen)
	forceUpdateWidgetFontMetrics(0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::contentsMouseReleaseEvent(QMouseEvent* ome) {

	m_bMouseButtonPressed = false;
	QMouseEvent* me = new QMouseEvent(QEvent::MouseButtonRelease, inverseWorldMatrix().map(ome->pos()),
					  ome->button(),ome->state());

	if(m_bDrawRect) {
		viewport()->setMouseTracking( false );
		m_bDrawRect = false;
		m_SelectionRect.clear();
	}
	if( m_pAssocLine ) {
		delete m_pAssocLine;
		m_pAssocLine = 0;
	}
	m_Pos.setX(me->x());
	m_Pos.setY(me->y());

	if( allocateMouseReleaseEvent(me) ) {
		return;
	}
	if( m_CurrentCursor == WorkToolBar::tbb_Arrow || me -> state() != LeftButton ) {
		viewport()->setMouseTracking( false );
		if (me->state() == RightButton) {
			setMenu();
		}
		return;
	}
	//create an initial activity widget
	if( m_CurrentCursor == WorkToolBar::tbb_Initial_Activity ) {
		ActivityWidget * temp = new ActivityWidget( this , ActivityWidget::Initial );
		temp->setID( getDocument() -> getUniqueID() );//needed for associations
		temp->setX( m_Pos.x() );
		temp->setY ( m_Pos.y() );
		temp->setVisible( true );
		temp->setActivated();
		temp->setFont( m_pData -> getFont() );
		temp->slotColorChanged( m_pData->getID() );
		resizeCanvasToItems();
		getDocument()->setModified();
		return;
	}
	//create end activity
	if( m_CurrentCursor == WorkToolBar::tbb_End_Activity ) {
		ActivityWidget * temp = new ActivityWidget( this , ActivityWidget::End );
		temp -> setID( getDocument() -> getUniqueID() );//needed for associations
		temp ->setX( m_Pos.x() );
		temp -> setY ( m_Pos.y() );
		temp -> setVisible( true );
		temp->setActivated();
		temp -> setFont( m_pData -> getFont() );
		temp->slotColorChanged( m_pData->getID() );
		resizeCanvasToItems();
		getDocument()->setModified();
		return;
	}
	//create branch/merge activity
	if( m_CurrentCursor == WorkToolBar::tbb_Branch ) {
		ActivityWidget * temp = new ActivityWidget( this , ActivityWidget::Branch );
		temp -> setID( getDocument() -> getUniqueID() );//needed for associations
		temp ->setX( m_Pos.x() );
		temp -> setY ( m_Pos.y() );
		temp -> setVisible( true );
		temp->setActivated();
		temp -> setFont( m_pData -> getFont() );
		temp->slotColorChanged( m_pData->getID() );
		resizeCanvasToItems();
		getDocument()->setModified();
		return;
	}
	//create fork/join activity
	if( m_CurrentCursor == WorkToolBar::tbb_Fork ) {
		ActivityWidget * temp = new ActivityWidget( this , ActivityWidget::Fork );
		temp -> setID( getDocument() -> getUniqueID() );//needed for associations
		temp ->setX( m_Pos.x() );
		temp -> setY ( m_Pos.y() );
		temp -> setVisible( true );
		temp->setActivated();
		temp -> setFont( m_pData -> getFont() );
		temp->slotColorChanged( m_pData->getID() );
		resizeCanvasToItems();
		getDocument()->setModified();
		return;
	}
	//create a activity widget
	if( m_CurrentCursor == WorkToolBar::tbb_Activity )
	{
		bool ok = false;
		QString name = KLineEditDlg::getText( i18n("Enter Activity Name"), i18n("Enter the name of the new activity:"), i18n("new activity"), &ok );
		if( ok ) {
			ActivityWidget * temp = new ActivityWidget( this , ActivityWidget::Normal );
			temp->setName( name );
			temp->setID( getDocument() -> getUniqueID() );//needed for associations
			temp->setX( m_Pos.x() );
			temp->setY ( m_Pos.y() );
			temp->setVisible( true );
			temp->setActivated();
			temp->setFont( m_pData -> getFont() );
			temp->slotColorChanged( m_pData->getID() );
		}
		resizeCanvasToItems();
		getDocument()->setModified();
		return;
	}
	//create a state widget
	if( m_CurrentCursor == WorkToolBar::tbb_State ) {
		bool ok = false;
		QString name = KLineEditDlg::getText( i18n("Enter State Name"), i18n("Enter the name of the new state:"), i18n("new state"), &ok );
		if( ok ) {
			StateWidget * temp = new StateWidget( this , StateWidget::Normal );
			temp -> setName( name );
			temp -> setID( getDocument() -> getUniqueID() );//needed for associations
			temp ->setX( m_Pos.x() );
			temp -> setY ( m_Pos.y() );
			temp -> setVisible( true );
			temp->setActivated();
			temp -> setFont( m_pData -> getFont() );
			temp->slotColorChanged( m_pData->getID() );
		}
		resizeCanvasToItems();
		getDocument()->setModified();
		return;
	}
	//create an initial state widget
	if( m_CurrentCursor == WorkToolBar::tbb_Initial_State ) {
		StateWidget * temp = new StateWidget( this , StateWidget::Initial );
		temp -> setID( getDocument() -> getUniqueID() );//needed for associations
		temp ->setX( m_Pos.x() );
		temp -> setY ( m_Pos.y() );
		temp -> setVisible( true );
		temp->setActivated();
		temp -> setFont( m_pData -> getFont() );
		temp->slotColorChanged( m_pData->getID() );
		resizeCanvasToItems();
		getDocument()->setModified();
		return;
	}
	//create end state
	if( m_CurrentCursor == WorkToolBar::tbb_End_State ) {
		StateWidget * temp = new StateWidget( this , StateWidget::End );
		temp -> setID( getDocument() -> getUniqueID() );//needed for associations
		temp ->setX( m_Pos.x() );
		temp -> setY ( m_Pos.y() );
		temp -> setVisible( true );
		temp->setActivated();
		temp -> setFont( m_pData -> getFont() );
		temp->slotColorChanged( m_pData->getID() );
		resizeCanvasToItems();
		getDocument()->setModified();
		return;
	}
	//Create a NoteBox widget
	if(m_CurrentCursor == WorkToolBar::tbb_Note) {
		//no need to register with document but get a id from it
		//id used when checking to delete object and assocs
		NoteWidget *temp= new NoteWidget(this, getDocument()->getUniqueID());
		temp ->setX( m_Pos.x() );
		temp -> setY ( m_Pos.y() );
		temp -> setVisible( true );
		temp->setActivated();
		temp -> setFont( m_pData -> getFont() );
		temp->slotColorChanged( m_pData->getID() );
		resizeCanvasToItems();
		getDocument()->setModified();
		return;
	}
	//Create a Box widget
	if(m_CurrentCursor == WorkToolBar::tbb_Box) {
		//no need to register with document but get a id from it
		//id used when checking to delete object and assocs
		BoxWidget* newBox = new BoxWidget(this, getDocument()->getUniqueID());
		newBox->setX( m_Pos.x() );
		newBox->setY( m_Pos.y() );
		newBox->setVisible( true );
		newBox->setActivated();
		newBox->setFont( m_pData->getFont() );
		newBox->slotColorChanged( m_pData->getID() );
		resizeCanvasToItems();
		getDocument()->setModified();
		return;
	}
	//Create a Floating Text widget
	if(m_CurrentCursor == WorkToolBar::tbb_Text) {
		FloatingText * ft = new FloatingText(this, tr_Floating, "");
		ft -> changeTextDlg();
		//if no text entered delete
		if(!FloatingText::isTextValid(ft -> getText()))
			delete ft;
		else {
			ft -> setX( m_Pos.x() );
			ft -> setY( m_Pos.y() );
			ft -> setVisible( true );
			ft -> setID(getDocument() -> getUniqueID());
			ft->setActivated();
			ft -> setFont( m_pData -> getFont() );
			ft->slotColorChanged( m_pData->getID() );
		}
		resizeCanvasToItems();
		getDocument()->setModified();
		return;
	}
	//Create a Message on a Sequence diagram
	if(m_CurrentCursor == WorkToolBar::tbb_Seq_Message_Synchronous ||
	   m_CurrentCursor == WorkToolBar::tbb_Seq_Message_Asynchronous) {
		UMLWidget* clickedOnWidget = onWidgetLine( me->pos() );
		if(clickedOnWidget) {
			if(!m_pFirstSelectedWidget) { //we are starting a new message
				m_pFirstSelectedWidget = clickedOnWidget;
				viewport()->setMouseTracking( true );
				m_pAssocLine = new QCanvasLine( canvas() );
				m_pAssocLine->setPoints( me->x(), me->y(), me->x(), me->y() );
				m_pAssocLine->setPen( QPen( m_pData -> getLineColor(), 0, DashLine ) );
				m_pAssocLine->setVisible( true );
				return;
			} else { //clicked on second sequence line to create message
				FloatingText* messageText = new FloatingText(this, tr_Seq_Message, "");
				messageText->setFont( m_pData -> getFont() );
				messageText->setID(getDocument() -> getUniqueID());

				MessageWidget* message;

				if (m_CurrentCursor == WorkToolBar::tbb_Seq_Message_Synchronous) {
					message = new MessageWidget(this, m_pFirstSelectedWidget,
										   clickedOnWidget, messageText,
										   getDocument()->getUniqueID(),
										   me->y(),
										   sequence_message_synchronous);
				} else {
					message = new MessageWidget(this, m_pFirstSelectedWidget,
										   clickedOnWidget, messageText,
										   getDocument()->getUniqueID(),
										   me->y(),
										   sequence_message_asynchronous);
				}
				connect(this, SIGNAL(sigColorChanged(int)),
					message, SLOT(slotColorChanged(int)));

				messageText->setActivated();
				message->setActivated();
				m_pFirstSelectedWidget = 0;
				resizeCanvasToItems();
				getDocument()->setModified();
				return;
			}
		} else { //did not click on widget line, clear the half made message
			m_pFirstSelectedWidget = 0;
			return;
		}//end if clickedOnWidget
	} else if ( m_CurrentCursor < WorkToolBar::tbb_Actor || m_CurrentCursor > WorkToolBar::tbb_State ) {
		m_pFirstSelectedWidget = 0;
		return;
	}
	//if we are creating an object, we really create a class
	if(m_CurrentCursor == WorkToolBar::tbb_Object) {
		m_CurrentCursor = WorkToolBar::tbb_Class;
	}
	m_bCreateObject = true;
	getDocument()->createUMLObject(convert_TBB_OT(m_CurrentCursor));
	resizeCanvasToItems();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::slotToolBarChanged(int c) {
	m_CurrentCursor = (WorkToolBar::ToolBar_Buttons)c;
	m_pFirstSelectedWidget = 0;
	m_bPaste = false;
	m_bDrawRect = false;
	if( m_pAssocLine ) {
		delete m_pAssocLine;
		m_pAssocLine = 0;
	}
	viewport() -> setMouseTracking( false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::showEvent(QShowEvent */*se*/) {
	UMLApp* theApp = UMLApp::app();
	WorkToolBar* tb = theApp->getWorkToolBar();
	connect(tb,SIGNAL(sigButtonChanged(int)), this, SLOT(slotToolBarChanged(int)));
	connect(this,SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));
	connect(getDocument(), SIGNAL(sigObjectCreated(UMLObject *)),
		this, SLOT(slotObjectCreated(UMLObject *)));
	resetToolbar();

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::hideEvent(QHideEvent */*he*/) {
	UMLApp* theApp = UMLApp::app();
	WorkToolBar* tb = theApp->getWorkToolBar();
	disconnect(tb,SIGNAL(sigButtonChanged(int)), this, SLOT(slotToolBarChanged(int)));
	disconnect(this,SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));
	disconnect(getDocument(), SIGNAL(sigObjectCreated(UMLObject *)), this, SLOT(slotObjectCreated(UMLObject *)));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListView * UMLView::getListView() {
	return getDocument()->listView;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::slotObjectCreated(UMLObject* o) {
	m_bPaste = false;
	int type  = o->getBaseType();
	//check to see if we want the message
	//may be wanted by someone else e.g. list view
	if(m_bCreateObject) {
		UMLWidget* newWidget = 0;
		if(type == ot_Actor) {
			newWidget = new ActorWidget(this, o);
		} else if(type == ot_UseCase) {
			newWidget = new UseCaseWidget(this, o);
		} else if(type == ot_Package) {
			newWidget = new PackageWidget(this, o);
		} else if(type == ot_Component) {
			newWidget = new ComponentWidget(this, o);
			if (getType() == dt_Deployment) {
				newWidget->getData()->setIsInstance(true);
			}
		} else if(type == ot_Artifact) {
			newWidget = new ArtifactWidget(this, o);
		} else if(type == ot_Node) {
			newWidget = new NodeWidget(this, o);
		} else if(type == ot_Interface) {
		        InterfaceWidget* interfaceWidget = new InterfaceWidget(this, o);
			Diagram_Type diagramType = getType();
			if (diagramType == dt_Component || diagramType == dt_Deployment) {
				interfaceWidget->setDrawAsCircle(true);
			}
			newWidget = (UMLWidget*)interfaceWidget;
		} else if(type == ot_Class ) { // CORRECT?
			//see if we really want an object widget or class widget
			if(getType() == dt_Class) {
				newWidget = new ClassWidget(this, o);
			} else {
				newWidget = new ObjectWidget(this, o, m_pData -> getUniqueID() );
			}
		} else {
			kdWarning() << "ERROR: trying to create an invalid widget" << endl;
		}
		int y=m_Pos.y();

		if (newWidget->getBaseType() == wt_Object && this->getType() == dt_Sequence) {
			y = 80 - newWidget->height();
		}
		newWidget->setX( m_Pos.x() );
		newWidget->setY( y );
		newWidget->moveEvent( 0 );//needed for ObjectWidget to set seq. line properly
		newWidget->setVisible( true );
		newWidget->setActivated();
		newWidget->setFont( m_pData -> getFont() );
		newWidget->slotColorChanged( m_pData->getID() );
		m_bCreateObject = false;
		switch( type ) {
			case ot_Actor:
			case ot_UseCase:
			case ot_Class:
			case ot_Package:
			case ot_Component:
			case ot_Node:
			case ot_Artifact:
			case ot_Interface:
				createAutoAssociations(newWidget);
				break;
		}
		resizeCanvasToItems();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::slotObjectRemoved(UMLObject * o) {
	m_bPaste = false;
	int id = o -> getID();
	QObjectList * l = queryList( "UMLWidget");
	QObjectListIt it( *l );
	UMLWidget *obj;

	while ((obj=(UMLWidget*)it.current()) != 0 ) {
		++it;
		if(obj -> getID() == id)
		{
			removeWidget(obj);
		}
	}
	delete l;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::contentsDragEnterEvent(QDragEnterEvent *e) {
	UMLListViewItemDataList list;
	bool status = UMLDrag::decodeClip3(e, list);
	if(!status) {
		return;
	}
	UMLListViewItemDataListIt it(list);

	UMLListViewItemData* data = it.current();

	ListView_Type lvtype = data -> getType();

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
			status = false;
			break;
		default:
			break;
	}
	//can't drag anything onto state/activity diagrams
	if( diagramType == dt_State || diagramType == dt_Activity) {
		status = false;
	}
	//make sure can find UMLObject
	if(status && !(temp = getDocument()->findUMLObject(data -> getID()) ) ) {
		kdDebug() << " object not found" << endl;
		status = false;
	}
	//make sure dragging item onto correct diagram
	// concept - class,seq,coll diagram
	// actor,usecase - usecase diagram
	if(status) {
		UMLObject_Type ot = temp->getBaseType();
		if(diagramType == dt_UseCase && (ot != ot_Actor && ot != ot_UseCase) ) {
			status = false;
		}
		if((diagramType == dt_Sequence || diagramType == dt_Class ||
		    diagramType == dt_Collaboration) &&
		   (ot != ot_Class && ot != ot_Package && ot != ot_Interface) ) {
			status = false;
		}
		if (diagramType == dt_Deployment &&
		    (ot != ot_Interface && ot != ot_Component && ot != ot_Class && ot != ot_Node)) {
			status = false;
		}
		if (diagramType == dt_Component &&
		    (ot != ot_Interface && ot != ot_Component && ot != ot_Artifact)) {
			status = false;
		}
		if((diagramType == dt_UseCase || diagramType == dt_Class ||
		    diagramType == dt_Component || diagramType == dt_Deployment)
		   && widgetOnDiagram(data->getID()) ) {
			status = false;
		}
	}
	e->accept(status);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::contentsDropEvent(QDropEvent *e) {
	UMLListViewItemDataList list;
	bool status = UMLDrag::decodeClip3(e, list);
	if(!status) {
		return;
	}

	UMLListViewItemDataListIt it(list);
	UMLListViewItemData* data = it.current();
	ListView_Type lvtype = data->getType();
	UMLObject* o = 0;
	if(lvtype >= lvt_UseCase_Diagram && lvtype <= lvt_Sequence_Diagram) {
		status = false;
	}
	if(status && !( o = getDocument()->findUMLObject(data->getID()) ) ) {
		kdDebug() << " object not found" << endl;
		status = false;
	}
	if(status) {
		m_bCreateObject = true;
		m_Pos = e->pos();

		slotObjectCreated(o);

		getDocument() -> setModified(true);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLWidget * UMLView::onWidgetLine( QPoint point ) {
	SeqLineWidget * pLine = 0;
	for( pLine = m_SeqLineList.first(); pLine; pLine = m_SeqLineList.next() ) {
		if( pLine -> onWidget( point ) ) {
			return pLine -> getObjectWidget();
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::checkMessages(UMLWidget * w) {
	if(getType() != dt_Sequence)
		return;


	QObjectList * l = queryList( "UMLWidget");
	QObjectListIt it( *l );
	MessageWidget *obj;
	while ( (obj=(MessageWidget*)it.current()) != 0 ) {
		++it;
		if(obj -> getBaseType() == wt_Message) {
			if(obj -> contains(w)) {
				//make sure message doesn't have any associations
				removeAssociations(obj);
				obj -> cleanup();
				//make sure not in selected list
				m_SelectedList.remove(obj);

				delete obj;
			}
		}
	}
	delete l;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLView::widgetOnDiagram(int id) {
	QObjectList * l = queryList( "UMLWidget");
	QObjectListIt it( *l );
	UMLWidget *obj;

	while ( (obj=(UMLWidget*)it.current()) != 0 ) {
		++it;
		if(id == obj -> getID()) {
			delete l;
			return true;
		}
	}
	delete l;
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::contentsMouseMoveEvent(QMouseEvent* ome) {
        //Autoscroll
	if (m_bMouseButtonPressed) {
		int vx = ome->x();
		int vy = ome->y();
		int contsX = contentsX();
		int contsY = contentsY();
		int visw = visibleWidth();
		int vish = visibleHeight();
		int dtr = visw - (vx-contsX);
		int dtb = vish - (vy-contsY);
		int dtt =  (vy-contsY);
		int dtl =  (vx-contsX);
		if (dtr < 30) scrollBy(30-dtr,0);
		if (dtb < 30) scrollBy(0,30-dtb);
		if (dtl < 30) scrollBy(-(30-dtl),0);
		if (dtt < 30) scrollBy(0,-(30-dtt));
	}


	QMouseEvent *me = new QMouseEvent(QEvent::MouseMove,
					inverseWorldMatrix().map(ome->pos()),
					ome->button(),
					ome->state());

	m_LineToPos = me->pos();
	if( m_pFirstSelectedWidget ) {
		if( m_pAssocLine ) {
			QPoint sp = m_pAssocLine -> startPoint();
			m_pAssocLine -> setPoints( sp.x(), sp.y(), me->x(), me->y() );
		}
		return;
	}
	if(m_bDrawRect) {

		if( m_SelectionRect.count() == 4) {

			QCanvasLine * line = m_SelectionRect.at( 0 );
			line -> setPoints( m_Pos.x(), m_Pos.y(), me->x(), m_Pos.y() );

			line = m_SelectionRect.at( 1 );
			line -> setPoints( me->x(), m_Pos.y(), me->x(), me->y() );

			line = m_SelectionRect.at( 2 );
			line -> setPoints( me->x(), me->y(), m_Pos.x(), me->y() );

			line = m_SelectionRect.at( 3 );
			line -> setPoints( m_Pos.x(), me->y(), m_Pos.x(), m_Pos.y() );

			selectWidgets();
		}
	}

	allocateMouseMoveEvent(me);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLWidget * UMLView::findWidget( int id ) {
	QObjectList * l = queryList( "UMLWidget");
	QObjectListIt it( *l );
	UMLWidget *obj;
	while ( (obj=(UMLWidget*)it.current()) != 0 ) {
		++it;
		if( obj -> getBaseType() == wt_Object ) {
			if( static_cast<ObjectWidget *>( obj ) ->
			        getLocalID() == id ) {

				delete l;
				return obj;
			}
		} else if( obj -> getID() == id ) {
			delete l;
			return obj;
		}
	}
	delete l;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::removeWidget(UMLWidget * o) {
	if(!o)
		return;
	removeAssociations(o);

	UMLWidget_Type t = o->getBaseType();
	if(getType() == dt_Sequence && t == wt_Object)
		checkMessages(o);

	if( m_pOnWidget == o ) {
		getDocument() -> getDocWindow() -> updateDocumentation( true );
		m_pOnWidget = 0;
	}

	o -> cleanup();
	m_SelectedList.remove(o);
	disconnect( this, SIGNAL( sigRemovePopupMenu() ), o, SLOT( slotRemovePopupMenu() ) );
	disconnect( this, SIGNAL( sigClearAllSelected() ), o, SLOT( slotClearAllSelected() ) );
	disconnect( this, SIGNAL(sigColorChanged(int)), o, SLOT(slotColorChanged(int)));
	delete o;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::setFillColour(QColor colour) {
	m_pData->setFillColor(colour);
	emit sigColorChanged( getID() );
	canvas()->setAllChanged();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::setLineColor(QColor color) {
	m_pData -> setLineColor(color );
	emit sigColorChanged( getID() );
	emit sigLineColorChanged( color );
	canvas() -> setAllChanged();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::contentsMouseDoubleClickEvent(QMouseEvent* ome) {

	QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonDblClick,inverseWorldMatrix().map(ome->pos()),
					  ome->button(),ome->state());
	if ( allocateMouseDoubleClickEvent(me) ) {
		return;
	}
	clearSelected();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QRect UMLView::getDiagramRect() {
	int startx, starty, endx, endy;
	startx = starty = INT_MAX;
	endx = endy = 0;
	QObjectList * l = queryList( "UMLWidget");
	QObjectListIt it( *l );
	UMLWidget *obj;
	while ( (obj=(UMLWidget*)it.current()) != 0 ) {
		if (obj->isVisible()) {
			int objEndX = static_cast<int>(obj -> x()) + obj -> width();
			int objEndY = static_cast<int>(obj -> y()) + obj -> height();
			int objStartX = static_cast<int>(obj -> x());
			int objStartY = static_cast<int>(obj -> y());
			if (startx >= objStartX)
				startx = objStartX;
			if (starty >= objStartY)
				starty = objStartY;
			if(endx <= objEndX)
				endx = objEndX;
			if(endy <= objEndY)
				endy = objEndY;
		}
		++it;
	}
	//if seq. diagram, make sure print all of the lines
	if(getType() == dt_Sequence ) {
		SeqLineWidget * pLine = 0;
		for( pLine = m_SeqLineList.first(); pLine; pLine = m_SeqLineList.next() ) {
			int y = pLine -> getObjectWidget() -> getEndLineY();
			endy = endy < y?y:endy;
		}

	}
	delete l;

	/* now we need another look at the associations, because they are no
	 * UMLWidgets */
	l = queryList("AssociationWidget");
	QObjectListIt assoc_it (* l);
	AssociationWidget * assoc_obj;
	QRect rect;

	while ((assoc_obj = (AssociationWidget *) assoc_it.current()) != 0)
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
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::clearSelected() {
	m_SelectedList.clear();
	emit sigClearAllSelected();
	//getDocument() -> enableCutCopy(false);
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
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::selectionUseFillColor(bool useFC) {
	UMLWidget * temp = 0;
	for(temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next())
		temp -> setUseFillColor(useFC);
}

void UMLView::selectionSetFont( QFont font )
{
	UMLWidget * temp = 0;
	for(temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next())
		temp -> setFont( font );
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

	//now delete any selected associations
	AssociationWidgetListIt assoc_it(m_Associations);
	AssociationWidget* assocwidget = 0;
	while((assocwidget=assoc_it.current())) {
		++assoc_it;
		if( assocwidget-> getSelected() )
			removeAssoc(assocwidget);
	}

	/* we also have to remove selected messages from sequence diagrams */
	QPtrList<MessageWidget> msgWgtList = getMessageWidgetList();
	MessageWidget * cur_msgWgt;

	/* loop through all messages and check the selection state */
	for (cur_msgWgt = msgWgtList.first(); cur_msgWgt;
				cur_msgWgt = msgWgtList.next())
	{
		if (cur_msgWgt->getSelected() == true)
		{
			removeWidget(cur_msgWgt); // remove msg, because it is selected
		}
	}

	//make sure list empty - it should be anyway, just a check.
	m_SelectedList.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::selectAll() {
	m_Pos.setX(0);
	m_Pos.setY(0);
	m_LineToPos.setX(canvas()->width());
	m_LineToPos.setY(canvas()->height());
	selectWidgets();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::contentsMousePressEvent(QMouseEvent* ome) {
	m_bMouseButtonPressed = true;
	QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonPress,
					  inverseWorldMatrix().map(ome->pos()),
					  ome->button(),
					  ome->state());
	int x, y;
	if( m_pAssocLine ) {
		delete m_pAssocLine;
		m_pAssocLine = 0;
	}
	viewport()->setMouseTracking(true);
	emit sigRemovePopupMenu();

	//bit of a kludge to allow you to click over sequence diagram messages when
	//adding a new message
	if ( m_CurrentCursor != WorkToolBar::tbb_Seq_Message_Synchronous &&
	     m_CurrentCursor != WorkToolBar::tbb_Seq_Message_Asynchronous && allocateMousePressEvent(me) ) {
		return;
	}


	x = me->x();
	y = me->y();
	m_Pos.setX( x );
	m_Pos.setY( y );
	m_LineToPos.setX( x );
	m_LineToPos.setY( y );
	if(m_bPaste) {
		m_bPaste = false;
		//Execute m_bPaste action when pasting widget from another diagram
		//This needs to be changed to use UMLClipboard
		//clipboard -> m_bPaste(this, pos);
	}
	clearSelected();

	if(m_CurrentCursor == WorkToolBar::tbb_Arrow) {
		QCanvasLine* line = new QCanvasLine( canvas() );
		line->setPoints(x, y, x, y);
		line->setPen( QPen(QColor("grey"), 0, DotLine) );
		line->setVisible(true);
		line->setZ(100);
		m_SelectionRect.append(line);//four lines needed for rect.

		line = new QCanvasLine( canvas() );
		line->setPoints(x, y, x, y);
		line->setPen( QPen(QColor("grey"), 0, DotLine) );
		line->setVisible(true);
		line->setZ(100);
		m_SelectionRect.append(line);


		line = new QCanvasLine( canvas() );
		line->setPoints(x, y, x, y);
		line->setPen( QPen(QColor("grey"), 0, DotLine) );
		line->setVisible(true);
		line->setZ(100);
		m_SelectionRect.append(line);


		line = new QCanvasLine( canvas() );
		line->setPoints(x, y, x, y);
		line->setPen( QPen(QColor("grey"), 0, DotLine) );
		line->setVisible(true);
		line->setZ(100);
		m_SelectionRect.append(line);
		m_bDrawRect = true;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLView::selectWidgets() {
	clearSelected();
	UMLWidget * temp;
	int x,y,w,h;
	int px = 0, qx = 0, py = 0, qy = 0;

	QRect rect;
	px = m_Pos.x();
	py = m_Pos.y();
	qx = m_LineToPos.x();
	qy = m_LineToPos.y();
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
	QObjectList * l = queryList( "UMLWidget");
	QObjectListIt it( *l );
	while ( (temp=(UMLWidget*)it.current()) != 0 ) {
		x = (int)temp -> x();
		y =  (int)temp -> y();
		w = temp -> width();
		h = temp -> height();
		QRect rect2(x, y, w, h);
		++it;
		//see if any part of widget is in the rectangle
		//made of points pos and m_LineToPos
		if(rect.intersects(rect2)) {
			//if it is text that is part of an association then select the association
			//and the objects that are connected to it.
			if(temp -> getBaseType() == wt_Text && ((FloatingText *)temp) -> getRole() != tr_Floating ) {


				int t = ((FloatingText *)temp) -> getRole();
				if( t == tr_Seq_Message ) {
					MessageWidget * mw = (MessageWidget *)((FloatingText *)temp) -> getMessage();
					mw -> setSelected(true);
					m_SelectedList.remove(mw);//make sure not in there
					m_SelectedList.append(mw);
					UMLWidget * ow = mw -> getWidgetA();
					ow -> setSelected(true);
					m_SelectedList.remove(ow);//make sure not in there
					m_SelectedList.append(ow);
					ow = mw -> getWidgetB();
					ow -> setSelected(true);
					m_SelectedList.remove(ow);//make sure not in there
					m_SelectedList.append(ow);
				} else {
					AssociationWidget * a = static_cast<AssociationWidget *>( (static_cast<FloatingText *>(temp)) -> getAssoc() );
					if (a) {

					a -> setSelected(true);
					//select the two widgets
					UMLWidget * uw = a->getWidgetA();
					uw -> setSelected(true);
					m_SelectedList.remove(uw);//make sure not in there
					m_SelectedList.append(uw);
					uw = a->getWidgetB();
					uw -> setSelected(true);
					m_SelectedList.remove(uw);//make sure not in there
					m_SelectedList.append(uw);
					//select all the text
					FloatingText * text = a -> getMultiAWidget();
					if( text ) {
						text  -> setSelected( true )
						;
						m_SelectedList.remove( text );
						m_SelectedList.append( text );
					}
					text = a -> getMultiBWidget();
					if( text ) {
						text  -> setSelected( true )
						;
						m_SelectedList.remove( text );
						m_SelectedList.append( text );
					}
					text = a -> getRoleAWidget();
					if( text ) {
						text  -> setSelected( true )
						;
						m_SelectedList.remove( text );
						m_SelectedList.append( text );
					}
					text = a -> getRoleBWidget();
					if( text ) {
						text  -> setSelected( true );
						m_SelectedList.remove( text );
						m_SelectedList.append( text );
					}
					text = a -> getChangeWidgetA();
					if( text ) {
						text  -> setSelected( true );
						m_SelectedList.remove( text );
						m_SelectedList.append( text );
					}
					text = a -> getChangeWidgetB();
					if( text ) {
						text  -> setSelected( true );
						m_SelectedList.remove( text );
						m_SelectedList.append( text );
					}
					} // if (a)
				}//end else
			}//end if text
			else if(temp -> getBaseType() == wt_Message) {
				UMLWidget * ow = ((MessageWidget *)temp) -> getWidgetA();
				ow -> setSelected(true);
				m_SelectedList.remove(ow);//make sure not in there
				m_SelectedList.append(ow);
				ow = ((MessageWidget *)temp) -> getWidgetB();
				ow -> setSelected(true);
				m_SelectedList.remove(ow);//make sure not in there
				m_SelectedList.append(ow);
			}
			if(temp -> isVisible()) {
				m_SelectedList.remove(temp);//just make sure hasn't already been added automatically
				m_SelectedList.append(temp);
				temp -> setSelected(true);
			}
		}
	}
	delete l;
	selectAssociations( true );

	//now do the same for the messagewidgets
	QObjectList * lw = queryList( "UMLWidget");
	QObjectListIt itw( *lw );
	temp = 0;
	while ( (temp=(UMLWidget*)itw.current()) != 0 ) {
		++itw;
		if( temp -> getBaseType() == wt_Message ) {
			MessageWidget * w = (MessageWidget *)temp;
			if( w -> getWidgetA() -> getSelected() &&
			        w -> getWidgetB() -> getSelected() ) {
				w -> setSelected( true );
				m_SelectedList.remove( w );
				m_SelectedList.append( w );
			}//end if
		}
	}//end while
	delete lw;
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

	AssociationWidgetListIt assoc_it(m_Associations);
	while((a = assoc_it.current())) {
		++assoc_it;
		if(a->getAssocType() == at_Anchor) {
			UMLWidget *wa = 0,*wb = 0, *destination= 0, *source = 0;

			wa = a->getWidgetA();
			wb = a-> getWidgetB();

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

	// save
	if(! fileDialog.selectedURL().isEmpty())
	{
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
				int want_save = KMessageBox::questionYesNo(0, i18n("The selected file %1 exists.\nDo you want to overwrite it?").arg(m_ImageURL.fileName()),
									i18n("File Already Exists"),
									i18n("Yes"), i18n("No"));
				if (want_save == KMessageBox::No)
					// another possibility would be to show the save dlg again
					return;
			}
		} else {
			s = tmpfile.name();
		}

		QRect rect = getDiagramRect();
		if (rect.isEmpty()) {
			KMessageBox::sorry(0, i18n("Can not save an empty diagram"),
			                   i18n("Diagram Save Error!"));
		} else {
			//  eps requested
			if (imageMimetype == "image/x-eps") {
				printToFile(s,true);
			}else{
				QPixmap diagram(rect.width(), rect.height());
				getDiagram(rect, diagram);
				diagram.save(s, mimeTypeToImageType(imageMimetype).ascii());
			}
			if (!m_ImageURL.isLocalFile()) {
				if(!KIO::NetAccess::upload(tmpfile.name(), m_ImageURL)) {
					KMessageBox::error(0,
					                   i18n("There was a problem saving file: %1").arg(m_ImageURL.path()),
					                   i18n("Save Error"));
				}
				tmpfile.unlink();
			} //!isLocalFile
		} //rect.isEmpty()

	} //!m_ImageURL.isEmpty()
}//exportImage()

void UMLView::slotActivate() {
	getDocument()->changeCurrentView(getID());
}

UMLObjectList* UMLView::getUMLObjects() {
	int type;
	UMLObjectList* list = new UMLObjectList;
	list->setAutoDelete(FALSE);

	QObjectList * l = queryList( "UMLWidget");
	QObjectListIt it( *l );

	UMLWidget *obj;
	while ( (obj=(UMLWidget*)it.current()) != 0 ) {
		++it;
		type = obj -> getBaseType();
		switch( type ) //use switch for easy future exspansion
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
	delete l;
	return list;
}

bool UMLView::activate() {
	QObjectList * l = queryList( "UMLWidget");
	QObjectListIt it( *l );
	UMLWidget *obj;

	//Activate Regular widgets then activate  messages
	while ( (obj=(UMLWidget*)it.current()) != 0 ) {
		++it;
		//If this UMLWidget is already activated or is a MessageWidget then skip it

		if(obj->isActivated() || obj->getBaseType() == wt_Message) {
			continue;

		}
		if(!obj->activate()) {
			continue;
		}
		obj -> setVisible( true );
	}//end while

	QObjectListIt it2( *l );
	//Activate Message widgets
	while ( (obj=(UMLWidget*)it2.current()) != 0 ) {
		++it2;
		//If this UMLWidget is already activated or isnt a MessageWidget then skip it
		if(obj->isActivated() || obj->getBaseType() != wt_Message)
			continue;

		if(!obj->activate(getDocument()->getChangeLog())) {
			kdDebug() << "Couldn't activate message widget" << endl;
			continue;
		}
		obj -> setVisible( true );

	}//end while
	delete l;

	//Activate All associationswidgets
	AssociationWidgetListIt assoc_it(m_Associations);
	AssociationWidget *assocwidget;
	//first get total count
	while((assocwidget = assoc_it.current())) {
		++assoc_it;
		if( !assocwidget->isActivated() ) {
			assocwidget->activate();
			if( m_PastePoint.x() != 0 ) {
				int x = m_PastePoint.x() - m_Pos.x();
				int y = m_PastePoint.y() - m_Pos.y();
				assocwidget -> moveEntireAssoc( x, y );
			}
		}//end if
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

bool UMLView::getSelectedWidgetDatas(UMLWidgetDataList& WidgetDataList) {
	UMLWidget * temp = 0;
	UMLWidget_Type type;
	for(temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next()) {
		type = temp->getBaseType();
		if (type == wt_Text) {
			if( ((FloatingText*)temp)->getRole() == tr_Floating ) {
				WidgetDataList.append(temp->getData());
			}
		} else {
			WidgetDataList.append(temp->getData());
		}
	}
	//Insert WidgetData after regular widgets because when they are created
	//in a m_bPaste operation they requiere the widgets it associates to be created first
	for(temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next()) {
		type = temp->getBaseType();
		if(type == wt_Message)
			WidgetDataList.append(temp->getData());
	}
	return true;
}

bool UMLView::getSelectedAssocDatas(AssociationWidgetDataList & assocWidgetDataList) {
	AssociationWidgetListIt assoc_it(m_Associations);
	AssociationWidget* assocwidget = 0;
	while((assocwidget=assoc_it.current())) {
		++assoc_it;
		if( assocwidget -> getSelected() )
			assocWidgetDataList.append((AssociationWidgetData*)assocwidget);
	}
	return true;
}

bool UMLView::createWidget(UMLWidgetData* WidgetData) {
	if( !WidgetData )
		return false;
	UMLWidget * widget = 0;
	UMLObject * object = getDocument() -> findUMLObject( WidgetData -> getId() );
	switch( WidgetData->getType() ) {
		case wt_Actor:
			widget = new ActorWidget(this , object, WidgetData);
			break;

		case wt_UseCase:
			widget = new UseCaseWidget(this, object, WidgetData);
			break;

		case wt_Class:
			widget = new ClassWidget(this, object, WidgetData);
			break;

		case wt_Package:
			widget = new PackageWidget(this, object, WidgetData);
			break;

		case wt_Component:
			widget = new ComponentWidget(this, object, WidgetData);
			break;

		case wt_Node:
			widget = new NodeWidget(this, object, WidgetData);
			break;

		case wt_Artifact:
			widget = new ArtifactWidget(this, object, WidgetData);
			break;

		case wt_Interface:
			widget = new InterfaceWidget(this, object, WidgetData);
			break;

		case wt_Text:
			widget = new FloatingText(this, WidgetData);
			break;

		case wt_Note:
			widget = new NoteWidget(this, WidgetData);
			break;

		case wt_Box:
			widget = new BoxWidget(this, WidgetData);
			break;

		case wt_Object:
			widget = new ObjectWidget(this, object, WidgetData);
			break;

		case wt_Message:
			widget = new MessageWidget(this, WidgetData);
			break;

		case wt_State:
			widget = new StateWidget( this, WidgetData );
			break;

		case wt_Activity:
			widget = new ActivityWidget( this, WidgetData );
			break;

		default:
			kdWarning() << "trying to create widget of unkent type" << endl;
			return false;
			break;
	}
	widget -> setX( WidgetData -> getX() );
	widget -> setY( WidgetData -> getY() );
	return true;
}


bool UMLView::addWidget( UMLWidgetData * pWidgetData ) {
	if( !pWidgetData ) {
		return false;
	}

	IDChangeLog * log = getDocument() -> getChangeLog();
	if( !log || !m_pIDChangesLog) {

		kdDebug()<<"A log is not open"<<endl;
		return false;

	}
	if( pWidgetData -> getX() < m_Pos.x() )
		m_Pos.setX( pWidgetData -> getX() );
	if( pWidgetData -> getY() < m_Pos.y() )
		m_Pos.setY( pWidgetData -> getY() );
	//see if we need a new id to match object
	int newID = -1;
	ObjectWidgetData* pObjectData = 0;
	MessageWidgetData * pMessageData = 0;
	UMLObject * pObject = 0;
	int waID = -1, wbID = -1, newWAID = -1, newWBID = -1, newTextID = - 1;
	switch( pWidgetData -> getType() ) {

		case wt_Class:
		case wt_Package:
		case wt_Component:
		case wt_Node:
		case wt_Artifact:
		case wt_Interface:
		case wt_Actor:
		case wt_UseCase:
			newID = log->findNewID( pWidgetData -> getId() );
			if( newID == -1 )
				return false;
			pWidgetData -> setId( newID );
			pObject = getDocument() -> findUMLObject( newID );
			if( !pObject ) {
				kdDebug() << "addWidget::Can't find UMLObject" << endl;
				return false;
			}
			//make sure it doesn't already exist.
			if( findWidget( newID ) ) {
				return true;//don't stop paste just because widget found.
			}
			break;

		case wt_Message:
			newID = getDocument()->assignNewID( pWidgetData -> getId() );

			pWidgetData -> setId( newID );
			pMessageData = static_cast<MessageWidgetData *>( pWidgetData );
			waID = pMessageData -> getWidgetAID();
			wbID = pMessageData -> getWidgetBID();
			newWAID = m_pIDChangesLog ->findNewID( waID );
			newWBID = m_pIDChangesLog ->findNewID( wbID );

			if( newWAID == -1 || newWBID == -1 ) {
				kdDebug()<<"Error with ids : "<<newWAID<<" "<<newWBID<<endl;
				return false;
			}
			pMessageData -> setWidgetAID( newWAID );
			pMessageData -> setWidgetBID( newWBID );
			if( pMessageData -> getTextID() != - 1 ) {
				newTextID = getDocument()->assignNewID( pMessageData->getTextID() );
				pMessageData -> setTextID( newTextID );
			} else
				pMessageData -> setTextID( getDocument() -> getUniqueID() );

			break;

		case wt_Object:
			if((pObjectData = static_cast<ObjectWidgetData*>(pWidgetData))) {
				newID = log->findNewID( pWidgetData -> getId() );
				if (newID == -1) {
					return false;
				}
				pObjectData -> setId( newID );
				int nNewLocalID = m_pData -> getUniqueID();
				int nOldLocalID = pObjectData -> getLocalID();
				m_pIDChangesLog->addIDChange( nOldLocalID, nNewLocalID );
				pObjectData -> setLocalID( nNewLocalID );
				pObject = getDocument() -> findUMLObject( newID );
				if( !pObject ) {
					kdDebug() << "addWidget::Can't find UMLObject" << endl;
					return false;
				}
			}
			break;

		case wt_Note:
		case wt_Box:
		case wt_Text:
		case wt_State:
		case wt_Activity:
			newID = getDocument()->assignNewID( pWidgetData->getId() );
			pWidgetData->setId(newID);
			break;

		default:
			kdDebug() << "Trying to add an invalid widget type" << endl;
			return false;
			break;
	}
	return createWidget( pWidgetData );
}

bool UMLView::addAssociation( AssociationWidgetData* AssocData ) {
	if(!AssocData)
		return false;

	IDChangeLog * log = getDocument() -> getChangeLog();

	if( !log )
		return false;

	int ida = -1, idb = -1;
	Association_Type type = AssocData -> getAssocType();
	IDChangeLog* localLog = getLocalIDChangeLog();
	if( getType() == dt_Collaboration || getType() == dt_Sequence ) {
		//check local log first
		ida = localLog->findNewID( AssocData->getWidgetAID() );

		idb = localLog->findNewID( AssocData->getWidgetBID() );
		//if either is still not found and assoc type is anchor
		//we are probably linking to a notewidet - else an error
		if( ida == -1 && type == at_Anchor )
			ida = log->findNewID(AssocData->getWidgetAID());
		if( idb == -1 && type == at_Anchor )
			idb = log->findNewID(AssocData->getWidgetBID());
	} else {
		ida = log->findNewID(AssocData->getWidgetAID());
		idb = log->findNewID(AssocData->getWidgetBID());
	}
	if(ida == -1 || idb == -1) {
		return false;
	}
	AssocData->setWidgetAID(ida);
	AssocData->setWidgetBID(idb);
	UMLWidget * m_pWidgetA = findWidget(AssocData->getWidgetAID());
	UMLWidget * m_pWidgetB = findWidget(AssocData->getWidgetBID());
	//make sure valid widget ids

	if(!m_pWidgetA || !m_pWidgetB)
		return false;

	//make sure valid
	if( !AssocRules::allowAssociation( AssocData -> getAssocType(), m_pWidgetA, m_pWidgetB ) ) {
		return true;//in a paste we still need to be true
	}
	//make sure there isn't already the same assoc
	AssociationWidgetListIt assoc_it(m_Associations);

	AssociationWidget* assocwidget = 0;
	while((assocwidget=assoc_it.current())) {
		++assoc_it;
		if( *AssocData == (AssociationWidgetData&)*assocwidget )
			return true;
	}

	return createAssoc( AssocData );
}


/** Creates an association from a AssociationWidgetData from the clipboard,
Activate must be called to make this assoc visible */
bool UMLView::createAssoc(AssociationWidgetData* AssocData) {
	if(!AssocData) {
		return false;
	}
	AssociationWidget* assoc = new AssociationWidget(this, *AssocData);
	addAssocInViewAndDoc(assoc);
	return true;
}

void UMLView::addAssocInViewAndDoc(AssociationWidget* a) {

 	// append in view
 	m_Associations.append(a);

 	// append in document
 	getDocument() -> addAssociation (a->getAssociation());

}

bool UMLView::activateAfterLoad(bool bUseLog) {
	bool status = true;
	AssociationWidgetData * assocData = 0;
	UMLWidgetData * widgetData = 0;
	if ( !m_bActivated ) {
		if( bUseLog ) {
			beginPartialWidgetPaste();
		}
		//create and activate regular widgets first
		UMLWidgetDataListIt w_it( m_pData -> getWidgetList() );
		while( ( widgetData = w_it.current() ) ) {
			++w_it;
			if( bUseLog ) {
				addWidget( widgetData );
			} else {
				createWidget( widgetData );
			}
		}

		//create the message widgets now
		UMLWidgetDataListIt m_it( m_pData -> getMessageList() );
		widgetData = 0;
		while( ( widgetData = m_it.current() ) ) {
			++m_it;
			if( bUseLog ) {
				addWidget( widgetData );
			} else {
				createWidget( widgetData );
			}
		}

		//now create the associations
		AssociationWidgetDataListIt a_it( m_pData -> getAssociationList() );
		while( ( assocData = a_it.current() ) ) {
			++a_it;
			if( bUseLog ) {
				addAssociation( assocData );
			} else {
				createAssoc( assocData );
			}
		}

		//now activate them all

		status = activate();

		if( bUseLog ) {
			endPartialWidgetPaste();
		}
		resizeCanvasToItems();
		setZoom( m_pData->getZoom() );
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
  		getDocument() -> getDocWindow() -> updateDocumentation( true );
  		m_pMoveAssoc = 0;
  	}
 	removeAssocInViewAndDoc(pAssoc, true);
}

void UMLView::removeAssocInViewAndDoc(AssociationWidget* a, bool deleteLater) {
 	if(!a)
 		return;
 	// Remove the association from the UMLDoc.
 	// NO! now done by the association->cleanup.
 	// getDocument()->removeAssociation(a->getAssociation());

 	// Remove the association in this view.
 	a->cleanup();
 	m_Associations.remove(a);
 	if (deleteLater) {
 		a->deleteLater();
	}
	else
	 	delete a;
}

bool UMLView::getAssocWidgets(AssociationWidgetList & Associations) {
	Associations = m_Associations;
	return true;
}

bool UMLView::setAssoc(UMLWidget *pWidget) {

	Association_Type type = convert_TBB_AT(m_CurrentCursor);
	m_bDrawRect = false;
	m_SelectionRect.clear();
	//if this we are not concerned here so return
	if(m_CurrentCursor < WorkToolBar::tbb_Generalization || m_CurrentCursor > WorkToolBar::tbb_Anchor) {
		return true;
	}
	clearSelected();

	if(!m_pFirstSelectedWidget) {
		if( !AssocRules::allowAssociation( type, pWidget ) ) {
			KMessageBox::error(0, i18n("Incorrect use of associations."), i18n("Association Error"));
			return true;
		}
		//set up position
		QPoint pos;
		pos.setX((int)pWidget -> x() + (pWidget->width() / 2));

		pos.setY((int)pWidget -> y() + (pWidget->height() / 2));
		setPos(pos);
		m_LineToPos = pos;
		m_pFirstSelectedWidget = pWidget;
		viewport() -> setMouseTracking( true );
		if( m_pAssocLine )
			delete m_pAssocLine;
		m_pAssocLine = 0;
		m_pAssocLine = new QCanvasLine( canvas() );
		m_pAssocLine -> setPoints( pos.x(), pos.y(), pos.x(), pos.y() );
		m_pAssocLine -> setPen( QPen( m_pData -> getLineColor(), 0, DashLine ) );

		m_pAssocLine -> setVisible( true );

		return true;
	} else {
		//The following reasigment is just to make things clearer
		UMLWidget* widgetA = m_pFirstSelectedWidget;
		UMLWidget* widgetB = pWidget;
		UMLWidget_Type at = widgetA -> getBaseType();
		bool valid = true;
		if (type == at_Generalization) {
			type = AssocRules::isGeneralisationOrRealisation(widgetA, widgetB);
		}
		if (widgetA == widgetB) {
			valid = AssocRules::allowSelf( type, at );
		} else {
			valid =  AssocRules::allowAssociation( type, widgetA, widgetB );
		}
		if( valid ) {
			AssociationWidget *temp = new AssociationWidget(this, widgetA, type, widgetB);
			addAssocInViewAndDoc(temp);
		} else {
			KMessageBox::error(0, i18n("Incorrect use of associations."), i18n("Association Error"));
		}
		m_pFirstSelectedWidget = 0;

		if( m_pAssocLine ) {
			delete m_pAssocLine;
			m_pAssocLine = 0;
		}
		return true;
	}//end if firstAssocSet

	if( m_pAssocLine ) {
		delete m_pAssocLine;
		m_pAssocLine = 0;
	}
	resetToolbar();
	m_pFirstSelectedWidget = 0;
	return false;
}

/** Removes all the associations related to Widget */
void UMLView::removeAssociations(UMLWidget* Widget) {

	AssociationWidgetListIt assoc_it(m_Associations);
	AssociationWidget* assocwidget = 0;
	while((assocwidget=assoc_it.current())) {
		++assoc_it;
		if(assocwidget->contains(Widget)) {
			removeAssoc(assocwidget);

		}
	}
}

void UMLView::selectAssociations(bool bSelect) {
	AssociationWidgetListIt assoc_it(m_Associations);
	AssociationWidget* assocwidget = 0;
	while((assocwidget=assoc_it.current())) {
		++assoc_it;
		if(bSelect &&
		  assocwidget->getWidgetA() && assocwidget->getWidgetA()->getSelected() &&
		  assocwidget->getWidgetB() && assocwidget->getWidgetB()->getSelected() ) {
			assocwidget->setSelected(true);
		} else {
			assocwidget->setSelected(false);
		}
	}//end while
}

void UMLView::getWidgetAssocs(UMLObject* Obj, AssociationWidgetList & Associations) {
	if( ! Obj )
		return;

	AssociationWidgetListIt assoc_it(m_Associations);
	AssociationWidget * assocwidget;
	while((assocwidget = assoc_it.current())) {
		if(assocwidget->getWidgetA()->getUMLObject() == Obj
		        || assocwidget->getWidgetB()->getUMLObject() == Obj)
			Associations.append(assocwidget);
		++assoc_it;
	}//end while
}

void UMLView::closeEvent ( QCloseEvent * e ) {
	QWidget::closeEvent(e);
}

void UMLView::removeAllAssociations() {
	//Remove All associations
	AssociationWidgetListIt assoc_it(m_Associations);
	AssociationWidget* assocwidget = 0;
	while((assocwidget=assoc_it.current()))
	{
		++assoc_it;
		removeAssocInViewAndDoc(assocwidget);
	}
}


void UMLView::removeAllWidgets() {
	//FIXME bug 59774
	//this queryList returns a too high count and list items which cause the program to crash
	//when you touch them on diagrams that contain associations /and/ have been saved and loaded
	QObjectList * l = queryList( "UMLWidget");
	QObjectListIt it( *l );
	UMLWidget * temp = 0;
	while ( (temp=(UMLWidget*)it.current()) != 0 ) {
		++it;
		if( !( temp -> getBaseType() == wt_Text && ((FloatingText *)temp)-> getRole() != tr_Floating ) ) {
			removeWidget( temp );
		}
	}
	delete l;
}


void UMLView::setName(QString& strName) {
	m_pData -> setName( strName );
}

QString UMLView::getName() {
	return m_pData -> getName();
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

Uml::UMLObject_Type UMLView::convert_TBB_OT(WorkToolBar::ToolBar_Buttons tbb) {
	UMLObject_Type ot = ot_UMLObject;
	switch(tbb) {
		case WorkToolBar::tbb_Actor:
			ot = ot_Actor;
			break;

		case WorkToolBar::tbb_UseCase:
			ot = ot_UseCase;
			break;

		case WorkToolBar::tbb_Class:
			ot = ot_Class;
			break;

		case WorkToolBar::tbb_Package:
			ot = ot_Package;
			break;

		case WorkToolBar::tbb_Component:
			ot = ot_Component;
			break;

		case WorkToolBar::tbb_Node:
			ot = ot_Node;
			break;

		case WorkToolBar::tbb_Artifact:
			ot = ot_Artifact;
			break;

		case WorkToolBar::tbb_Interface:
			ot = ot_Interface;
			break;
		default:
			break;
	}
	return ot;
}

bool UMLView::allocateMousePressEvent(QMouseEvent * me) {
	m_pMoveAssoc = 0;
	m_pOnWidget = 0;

	QObjectList* l = queryList("UMLWidget");
	QObjectListIt it( *l );
	UMLWidget* obj = 0;
	UMLWidget* backup = 0;
	UMLWidget* boxBackup = 0;
	while ( (obj=(UMLWidget*)it.current()) != 0 ) {
		if( obj -> isVisible() && obj -> onWidget( me -> pos() ) ) {
			//Give text object priority,
			//they can easily get into a position where
			//you can't touch them.
			//Give Boxes lowest priority, we want to be able to move things that
			//are on top of them.
			if( obj -> getBaseType() == wt_Text ) {
				m_pOnWidget = obj;
				obj ->  mousePressEvent( me );
				delete l;
				return true;
			} else if (obj->getBaseType() == wt_Box) {
				boxBackup = obj;
			} else {
				backup = obj;
			}
		}
		++it;
	}//end while
	//if backup is set, then let it have the event
	delete l;
	if (!backup) {
		backup = boxBackup;
	}
	if(backup) {
		backup -> mousePressEvent( me );
		m_pOnWidget = backup;

		return true;

	}
	///*******now check associations*************///

	AssociationWidgetListIt assoc_it(m_Associations);
	AssociationWidget* assocwidget = 0;
	while((assocwidget=assoc_it.current())) {
		if( assocwidget -> onAssociation( me -> pos() )) {
			assocwidget->mousePressEvent(me);
			m_pMoveAssoc = assocwidget;
			return true;
		}
		++assoc_it;
	}
	m_pMoveAssoc = 0;
	m_pOnWidget = 0;
	return false;
}

bool UMLView::allocateMouseReleaseEvent(QMouseEvent * me) {
	//values will already be set through press event.
	//may not be over it, but should still get the event.

	if( m_pOnWidget ) {
		m_pOnWidget -> mouseReleaseEvent( me );
		return true;
	}

	if( m_pMoveAssoc ) {
		m_pMoveAssoc -> mouseReleaseEvent( me );
		return true;
	}

	m_pOnWidget = 0;
	m_pMoveAssoc = 0;
	return false;
}

bool UMLView::allocateMouseDoubleClickEvent(QMouseEvent * me) {
	//values will already be set through press and release events
	if( m_pOnWidget && m_pOnWidget -> onWidget( me -> pos() )) {
		m_pOnWidget -> mouseDoubleClickEvent( me );
		return true;
	}
	if( m_pMoveAssoc && m_pMoveAssoc -> onAssociation( me -> pos() )) {
		m_pMoveAssoc -> mouseDoubleClickEvent( me );
		return true;
	}
	m_pOnWidget = 0;
	m_pMoveAssoc = 0;
	return false;
}

bool UMLView::allocateMouseMoveEvent(QMouseEvent * me) {
	//tracking may have been set by someone else
	//only move if we set it.  We only want it if
	//left mouse button down.
	if(m_pOnWidget) {
		m_pOnWidget -> mouseMoveEvent( me );
		return true;
	}
	if(m_pMoveAssoc) {
		m_pMoveAssoc -> mouseMoveEvent( me );
		return true;
	}
	return false;
}



void UMLView::showDocumentation( UMLObject * object, bool overwrite ) {
	getDocument() -> getDocWindow() -> showDocumentation( object, overwrite );
}

void UMLView::showDocumentation( UMLWidget * widget, bool overwrite ) {
	getDocument() -> getDocWindow() -> showDocumentation( widget, overwrite );
}

void UMLView::showDocumentation( AssociationWidget * widget, bool overwrite ) {
	getDocument() -> getDocWindow() -> showDocumentation( widget, overwrite );
}

void UMLView::updateDocumentation( bool clear ) {
	getDocument() -> getDocWindow() -> updateDocumentation( clear );
}

void UMLView::createAutoAssociations( UMLWidget * widget ) {
	if( !widget )
		return;
	AssociationWidget * docData = 0;

	UMLWidget * widgetA = 0;
	UMLWidget * widgetB = 0;

	AssociationWidgetList list;

	getDocument() -> getAssciationListAllViews( this, widget -> getUMLObject(), list );
	AssociationWidgetListIt it( list );

	while( ( docData = it.current() ) ) {
		if( docData -> getAssocType() != at_Anchor ) {
			//see if both widgets are on this diagram
			//if yes - create an association
			widgetA = widget;
			widgetB = findWidget( docData -> getWidgetBID() );
			if( docData -> getWidgetAID() != widget -> getID() ) {
				widgetA = findWidget( docData -> getWidgetAID() );
				widgetB = widget;
			}

			if( widgetA && widgetB ) {
				AssociationWidget * temp = new AssociationWidget( this, widgetA ,
				                            docData -> getAssocType(), widgetB );

				// CHECK: why is this needed at all? I mean, the association
				// is "new" so there shouldnt be any preset roles, etc.
				/*
				FloatingTextData * data = docData -> getMultiDataA();
				if( data )
					temp -> setMultiA( data -> getText() );
				data =  docData -> getMultiDataB();

  				if( data )
  					temp -> setMultiB( data -> getText() );

 				data = docData -> getRoleAData();
  				if( data )
 					temp -> setRoleNameA( data -> getText() );

 				data = docData -> getRoleBData();
 				if( data )
 					temp -> setRoleNameB( data -> getText() );
 					*/

 				/*
 				data = docData -> getChangeDataA();
 				if( data )
 					temp -> setChangeWidgetA( data -> getText() );

 				data = docData -> getChangeDataB();
 				if( data )
 					temp -> setChangeWidgetB( data -> getText() );
 				*/

 				// CHECK
 				// m_Associations.append( temp );
 				addAssocInViewAndDoc( temp );
  			}
  		}//end if !at_Anchor
  		++it;
	}//end while docAssoc
}

void UMLView::copyAsImage(QPixmap*& pix) {

	int px, py, qx, qy;
	int x, y, x1, y1;
	//get the smallest rect holding the diagram
	QRect rect = getDiagramRect();
	QPixmap diagram( rect.width(), rect.height() );

	//only draw what is selected
	m_bDrawSelectedOnly = true;
	selectAssociations(true);
	getDiagram(rect, diagram);

	//now get the selection cut
	//first get the smallest rect holding the widgets
	px = py = qx = qy = -1;
	UMLWidget * temp = 0;

	for(temp=(UMLWidget *)m_SelectedList.first();temp;temp=(UMLWidget *)m_SelectedList.next()) {
		x = (int)temp -> x();
		y = (int)temp -> y();
		x1 = x + temp -> width() - 1;
		y1 = y + temp -> height() - 1;
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
	AssociationWidget *assocwidget;
	AssociationWidgetListIt assoc_it(m_Associations);

	//get each type of associations
	//This needs to be reimplemented to increase the rectangle
	//if a part of any association is not included
	while((assocwidget = assoc_it.current()))
	{
		++assoc_it;
		if(assocwidget->getSelected())
		{
			temp = const_cast<FloatingText*>(assocwidget->getMultiAWidget());
			if(temp && temp->isVisible()) {
				x = (int)temp->x();
				y = (int)temp->y();
				x1 = x + temp->width() - 1;
				y1 = y + temp->height() - 1;

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
			}//end if temp

			temp = const_cast<FloatingText*>(assocwidget->getMultiBWidget());

			if(temp && temp->isVisible()) {
				x = (int)temp->x();
				y = (int)temp->y();
				x1 = x + temp->width() - 1;
				y1 = y + temp->height() - 1;

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
			}//end if temp

			temp = const_cast<FloatingText*>(assocwidget->getRoleAWidget());
			if(temp && temp -> isVisible()) {
				x = (int)temp->x();
				y = (int)temp->y();
				x1 = x + temp->width() - 1;
				y1 = y + temp->height() - 1;

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
			}//end if temp

			temp = const_cast<FloatingText*>(assocwidget->getRoleBWidget());
			if(temp && temp -> isVisible()) {
				x = (int)temp->x();
				y = (int)temp->y();
				x1 = x + temp->width() - 1;
				y1 = y + temp->height() - 1;
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

			}//end if temp

                        temp = const_cast<FloatingText*>(assocwidget->getChangeWidgetA());
                        if(temp && temp->isVisible()) {
                                x = (int)temp->x();
                                y = (int)temp->y();
                                x1 = x + temp->width() - 1;
                                y1 = y + temp->height() - 1;

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
                        }//end if temp

                        temp = const_cast<FloatingText*>(assocwidget->getChangeWidgetB());
                        if(temp && temp->isVisible()) {
                                x = (int)temp->x();
                                y = (int)temp->y();
                                x1 = x + temp->width() - 1;
                                y1 = y + temp->height() - 1;

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
                        }//end if temp




		}//end if selected
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
			getDocument()->loadUndoData();
			break;

		case ListPopupMenu::mt_Redo:
			getDocument()->loadRedoData();
			break;

		case ListPopupMenu::mt_Clear:
			clearDiagram();
			break;

		case ListPopupMenu::mt_Export_Image:
			exportImage();
			break;

		case ListPopupMenu::mt_FloatText:
			ft = new FloatingText(this, tr_Floating, "");
			ft -> changeTextDlg();
			//if no text entered delete
			if(!FloatingText::isTextValid(ft -> getText()))

				delete ft;
			else {
				ft->setX( m_Pos.x() );
				ft->setY( m_Pos.y() );
				ft->setVisible( true );
				ft->setID(getDocument() -> getUniqueID());
				ft->setActivated();
			}
			break;

		case ListPopupMenu::mt_UseCase:
			m_bCreateObject = true;
			getDocument()->createUMLObject( ot_UseCase );
			break;

		case ListPopupMenu::mt_Actor:
			m_bCreateObject = true;
			getDocument()->createUMLObject( ot_Actor );
			break;

		case ListPopupMenu::mt_Class:
		case ListPopupMenu::mt_Object:
			m_bCreateObject = true;
			getDocument()->createUMLObject( ot_Class);
			break;

		case ListPopupMenu::mt_Package:
			m_bCreateObject = true;
			getDocument()->createUMLObject(ot_Package);
			break;

		case ListPopupMenu::mt_Component:
			m_bCreateObject = true;
			getDocument()->createUMLObject(ot_Component);
			break;

		case ListPopupMenu::mt_Node:
			m_bCreateObject = true;
			getDocument()->createUMLObject(ot_Node);
			break;

		case ListPopupMenu::mt_Artifact:
			m_bCreateObject = true;
			getDocument()->createUMLObject(ot_Artifact);
			break;

		case ListPopupMenu::mt_Interface:
			m_bCreateObject = true;
			getDocument()->createUMLObject(ot_Interface);
			break;

		case ListPopupMenu::mt_Paste:
			m_PastePoint = m_Pos;
			m_Pos.setX( 2000 );
			m_Pos.setY( 2000 );
			getDocument() -> editPaste();

			m_PastePoint.setX( 0 );
			m_PastePoint.setY( 0 );
			break;

		case ListPopupMenu::mt_Initial_State:
			state = new StateWidget( this , StateWidget::Initial );
			state -> setID( getDocument() -> getUniqueID() );//needed for associations
			state -> setX( m_Pos.x() );
			state -> setY ( m_Pos.y() );
			state -> setVisible( true );
			state -> setActivated();
			break;

		case ListPopupMenu::mt_End_State:
			state = new StateWidget( this , StateWidget::End );
			state -> setID( getDocument() -> getUniqueID() );//needed for associations
			state -> setX( m_Pos.x() );
			state -> setY ( m_Pos.y() );
			state -> setVisible( true );
			state -> setActivated();
			break;

		case ListPopupMenu::mt_State:
			name = KLineEditDlg::getText( i18n("Enter State Name"), i18n("Enter the name of the new state:"), i18n("new state"), &ok );
			if( ok ) {
				state = new StateWidget( this , StateWidget::Normal );
				state -> setName( name );
				state -> setID( getDocument() -> getUniqueID() );//needed for associations
				state -> setX( m_Pos.x() );
				state -> setY ( m_Pos.y() );
				state -> setVisible( true );
				state -> setActivated();
			}
			break;

		case ListPopupMenu::mt_Initial_Activity:
			activity = new ActivityWidget( this , ActivityWidget::Initial );
			activity -> setID( getDocument() -> getUniqueID() );//needed for associations
			activity -> setX( m_Pos.x() );
			activity -> setY ( m_Pos.y() );
			activity -> setVisible( true );
			activity -> setActivated();
			break;


		case ListPopupMenu::mt_End_Activity:
			activity = new ActivityWidget( this , ActivityWidget::End );
			activity -> setID( getDocument() -> getUniqueID() );//needed for associations
			activity -> setX( m_Pos.x() );
			activity -> setY ( m_Pos.y() );
			activity -> setVisible( true );
			activity -> setActivated();
			break;

		case ListPopupMenu::mt_Branch:
			activity = new ActivityWidget( this , ActivityWidget::Branch );
			activity -> setID( getDocument() -> getUniqueID() );//needed for associations
			activity -> setX( m_Pos.x() );
			activity -> setY ( m_Pos.y() );
			activity -> setVisible( true );
			activity -> setActivated();
			break;

		case ListPopupMenu::mt_Activity:
			name = KLineEditDlg::getText( i18n("Enter Activity Name"), i18n("Enter the name of the new activity:"), i18n("new activity"), &ok );
			if( ok ) {
				activity = new ActivityWidget( this , ActivityWidget::Normal );
				activity -> setName( name );
				activity -> setID( getDocument() -> getUniqueID() );//needed for associations
				activity -> setX( m_Pos.x() );
				activity -> setY ( m_Pos.y() );
				activity -> setVisible( true );
				activity -> setActivated();
			}
			break;

		case ListPopupMenu::mt_SnapToGrid:
			toggleSnapToGrid();
			break;

		case ListPopupMenu::mt_ShowSnapGrid:
			toggleShowGrid();
			break;

		case ListPopupMenu::mt_Properties:
			showPropDialog();
			break;

		default:
			break;
	}
}


void UMLView::synchronizeData() {
	//get all the data for associations
        /* FIXME: Get rid of this, and instead do it like so:
           The AssociationWidgetData list in m_pData is only filled
           initially, and then there's no need to keep it in synch
           because the AssociationWidgetData::saveToXMI() directly
           saves the AssociationWidgets.
         */
	m_pData->getAssociationList().clear();
	AssociationWidgetListIt assoc_it(m_Associations);
	AssociationWidget* assocwidget = 0;
	while((assocwidget=assoc_it.current())) {
		++assoc_it;
		assocwidget->synchronizeData();
                AssociationWidgetData* assocData = (AssociationWidgetData*)assocwidget;
		m_pData->getAssociationList().append( assocData );
	}
	//get all the data for messagewidgets
	m_pData->getMessageList().clear();
	QObjectList * l = queryList( "UMLWidget" );
	QObjectListIt it( *l );
	UMLWidget *obj = 0;
	while ( (obj=(UMLWidget*)it.current()) != 0 ) {
		++it;
		if(obj->getBaseType() == wt_Message) {
			obj->synchronizeData();
			m_pData->getMessageList().append( obj -> getData() );
		}
	}//end while

	delete l;
	m_pData->getWidgetList().clear();
	//get all the data for the rest of the widgets - for text only store of role tr_Floating
	QObjectList * wl = queryList( "UMLWidget" );
	QObjectListIt wit( *wl );
	obj = 0;
	while ( (obj=(UMLWidget*)wit.current()) != 0 ) {
		++wit;
		Uml::UMLWidget_Type type = obj -> getBaseType();
		if (type == wt_Text) {
			if( ((FloatingText*)obj)->getRole() == tr_Floating ) {
				obj->synchronizeData();
				m_pData->getWidgetList().append( obj->getData() );
			}
		} else if (type != wt_Message) {
			obj->synchronizeData();
			m_pData->getWidgetList().append( obj->getData() );
		}
	}//end while
	delete wl;
}

void UMLView::slotCutSuccessful() {
	if( m_bStartedCut ) {
		deleteSelection();
		m_bStartedCut = false;
	}
}

void UMLView::slotShowView() {
	getDocument() -> changeCurrentView( m_pData -> getID() );
}

QPoint UMLView::getPastePoint() {
	QPoint point = m_PastePoint;
	point.setX( point.x() - m_Pos.x() );
	point.setY( point.y() - m_Pos.y() );
	return point;
}

bool UMLView::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	synchronizeData();
	return m_pData -> saveToXMI( qDoc, qElement );
}

bool UMLView::showPropDialog() {
	UMLViewDialog dlg( this, this );
	if( dlg.exec() ) {
		return true;
	}
	return false;
}


void UMLView::setFont( QFont font ) {
	m_pData -> setFont( font );
	UMLWidget * pWidget = 0;
	QObjectList * wl = queryList( "UMLWidget" );
	QObjectListIt wit( *wl );
	while ( ( pWidget = static_cast<UMLWidget *>( wit.current() ) )  != 0 ) {
		++wit;
		pWidget -> setFont( font );
	}
	delete wl;
	canvas() -> setAllChanged();
}

void UMLView::setClassWidgetOptions( ClassOptionsPage * page ) {
	UMLWidget * pWidget = 0;
	QObjectList * wl = queryList( "UMLWidget" );
	QObjectListIt wit( *wl );
	while ( ( pWidget = static_cast<UMLWidget *>( wit.current() ) )  != 0 ) {
		++wit;
		if( pWidget -> getBaseType() == Uml::wt_Class ) {
			page -> setWidget( static_cast<ClassWidget *>( pWidget ) );
			page -> updateUMLWidget();
		}
	}
	delete wl;
}


void UMLView::checkSelections() {
	UMLWidget * pWA = 0, * pWB = 0, * pTemp = 0;
	//check messages
	for(pTemp=(UMLWidget *)m_SelectedList.first();pTemp;pTemp=(UMLWidget *)m_SelectedList.next()) {
		if( pTemp->getBaseType() == wt_Message && pTemp -> getSelected() ) {
			MessageWidget * pMessage = static_cast<MessageWidget *>( pTemp );
			pWA = pMessage -> getWidgetA();
			pWB = pMessage -> getWidgetB();
			if( !pWA -> getSelected() ) {
				pWA -> m_bSelected = true;
				m_SelectedList.append( pWA );
			}
			if( !pWB -> getSelected() ) {
				pWB -> m_bSelected = true;
				m_SelectedList.append( pWB );
			}
		}//end if
	}//end for
	//check Associations
	AssociationWidgetListIt it(m_Associations);
	AssociationWidget * pAssoc = 0;
	while((pAssoc = it.current())) {
		++it;
		if( pAssoc -> getSelected() ) {
			pWA = pAssoc -> getWidgetA();
			pWB = pAssoc -> getWidgetB();
			if( !pWA -> getSelected() ) {
				pWA -> m_bSelected = true;
				m_SelectedList.append( pWA );
			}
			if( !pWB -> getSelected() ) {
				pWB -> m_bSelected = true;
				m_SelectedList.append( pWB );
			}
		}//end if
	}//end while
}

QPtrList<MessageWidget> UMLView::getMessageWidgetList() {
	UMLWidget * pWidget = 0;
	QObjectList * wl = queryList( "UMLWidget" );
	QObjectListIt wit( *wl );
	QPtrList<MessageWidget> list;
	while ( ( pWidget = static_cast<UMLWidget *>( wit.current() ) )  != 0 ) {
		++wit;
		if( pWidget -> getBaseType() == Uml::wt_Message ) {
			list.append( (MessageWidget*)pWidget );
		}
	}
	delete wl;
	return list;
}

void UMLView::clearDiagram() {
	if( KMessageBox::Yes == KMessageBox::warningYesNo( this, i18n("You are about to delete "
								       "the entire diagram.\nAre you sure?"),
							    i18n("Delete Diagram?") ) ) {
		removeAllWidgets();
	}
}

void UMLView::toggleSnapToGrid() {
	setSnapToGrid( (m_pData->getSnapToGrid()?false:true) );
}

void UMLView::toggleSnapComponentSizeToGrid() {
	setSnapComponentSizeToGrid( !m_pData->getSnapComponentSizeToGrid() );
}

void UMLView::toggleShowGrid() {
	setShowSnapGrid( (m_pData->getShowSnapGrid()?false:true) );
}

void UMLView::setSnapToGrid(bool bSnap) {
	m_pData->setSnapToGrid( bSnap );
	emit sigSnapToGridToggled( m_pData->getSnapToGrid() );
}

void UMLView::setSnapComponentSizeToGrid(bool bSnap) {
	m_pData->setSnapComponentSizeToGrid( bSnap );
	updateComponentSizes();
	emit sigSnapComponentSizeToGridToggled( m_pData->getSnapComponentSizeToGrid() );
}

void UMLView::setShowSnapGrid(bool bShow) {
	m_pData->setShowSnapGrid( bShow );
	canvas()->setAllChanged();
	emit sigShowGridToggled( m_pData->getShowSnapGrid() );
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

	m_pData->setZoom( currentZoom() );
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
	setZoom( m_pData->getZoom() );
	resizeCanvasToItems();
}

void UMLView::setCanvasSize(int width, int height) {
	m_pData->setCanvasWidth(width);
	m_pData->setCanvasHeight(height);
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
	QObjectList * l = queryList( "UMLWidget");
	QObjectListIt it( *l );
	UMLWidget *obj;
	while ( (obj=(UMLWidget*)it.current()) != 0 ) {
		++it;
		obj->updateComponentSize();
	}
	delete l;
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
	QObjectList * l = queryList( "UMLWidget");
	QObjectListIt it( *l );
	UMLWidget *obj;

	while ((obj=(UMLWidget*)it.current()) != 0 ) {
		++it;
		obj->forceUpdateFontMetrics(painter);
	}
}

#include "umlview.moc"
