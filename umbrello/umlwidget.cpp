/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpainter.h>
#include <qcolor.h>

#include <kdebug.h>
#include <kcursor.h>
#include <kcolordlg.h>
#include <kfontdialog.h>

#include "umlwidget.h"
#include "uml.h"
#include "umllistview.h"
#include "umlview.h"
#include "concept.h"
#include "associationwidget.h"
#include "dialogs/settingsdlg.h"
#include "floatingtext.h"
#include "objectwidget.h"
#include "messagewidget.h"

UMLWidget::UMLWidget( UMLView * view, UMLObject * o, UMLWidgetData * pData ) : QObject( view), QCanvasRectangle( view -> canvas() ) {
	m_pObject = o;
	m_pData = pData;
	setName(m_pObject->getName());
	m_pView = view;
	if(pData)
		m_pData->m_nId = m_pObject->getID();
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLWidget::UMLWidget( UMLView * view, int id, UMLWidgetData * pData ) : QObject( view), QCanvasRectangle( view -> canvas() ) {
	m_pView = view;
	m_pObject = 0;
	m_pData = pData;
	if(pData)
		m_pData->m_nId = id;
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLWidget::UMLWidget(UMLView * view,  UMLWidgetData * pData ) : QObject( view), QCanvasRectangle( view -> canvas() ) {
	m_pView = view;
	m_pObject = 0;
	m_pData = pData;
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLWidget::~UMLWidget() {
	//slotRemovePopupMenu();
	cleanup();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPoint UMLWidget::doMouseMove(QMouseEvent* me) {
	int newX = 0, newY = 0, count;
	int moveX, moveY;
	int maxX = m_pView->canvas()->width();
	int maxY = m_pView->canvas()->height();

	if( !m_bSelected ) {
		m_pView->setSelected( this, me );
	}
	m_bSelected = true;
	count = m_pView->getSelectCount();

	//If not m_bStartMove means moving as part of selection
	//me->pos() will have the amount we need to move.
	if(!m_bStartMove) {
		moveX = (int)me->x();
		moveY = (int)me->y();
	} else {
		//we started the move so..
		//move any others we are selected
		moveX = (int)me->x() - m_nOldX - m_nPressOffsetX;
		moveY = (int)me->y() - m_nOldY - m_nPressOffsetY;

		//if mouse moves off the edge of the canvas this moves the widget to 0 or canvasSize
		if( ((int)x() + moveX) < 0 ) {
			moveX = moveX - (int)x();
		}
		if( ((int)y() + moveY) < 0 ) {
			moveY = moveY - (int)y();
		}

		if( count > 1 ) {
			if( m_pView -> getType() == dt_Sequence ) {
				m_pView -> moveSelected( this, moveX, 0 );
			} else {
				m_pView -> moveSelected( this, moveX, moveY );
			}
		}
	}
	newX = (int)x() + moveX;
	newY = (int)y() + moveY;

	newX = newX<0 ? 0 : newX;
	newY = newY<0 ? 0 : newY;
	newX = newX>maxX ? maxX : newX;
	newY = newY>maxY ? maxY : newY;

	if (m_nOldX != newX || m_nOldY != newY) {
		m_bMoved = true;
	}
	return QPoint(newX, newY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLWidget::mouseMoveEvent(QMouseEvent* me) {
	if( m_bMouseDown || me->button() == LeftButton ) {
		QPoint newPosition = doMouseMove(me);
		int newX = newPosition.x();
		int newY = newPosition.y();

		m_nOldX = newX;
		m_nOldY = newY;
		setX( newX );
		setY( newY );
		adjustAssocs(newX, newY);
		m_pView->resizeCanvasToItems();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLWidget::mousePressEvent(QMouseEvent *me) {
	m_nPressOffsetX = me -> x() - (int)this -> x();
	m_nPressOffsetY = me -> y() - (int)this -> y();
	m_nOldX = (int)x();
	m_nOldY = (int)y();
	m_bMouseDown = false;
	m_bMoved = false;
	int count = m_pView -> getSelectCount();

	m_bStartMove = false;

	if( m_pView -> m_CurrentCursor != WorkToolBar::tbb_Arrow ) {
		//anything else needed??
		return;
	}
	if( me -> state() == ShiftButton )
	{
		/* we have to save the shift state, because in ReleaseEvent it is lost */
		m_bShiftPressed = true;
		if( me -> button() == LeftButton ) {
			m_bMouseDown = true;
			m_bStartMove = true;
			setSelected( ( m_bSelected?false:true ) );
			m_pView -> setSelected( this, me );
			return;
		} else if( me -> button() == RightButton ) {
			if( !m_bSelected)
				m_pView -> setSelected( this, me );
			setSelected( true );
			return;
		}
	} else {
		/* we have to save the shift state, because in ReleaseEvent it is lost */
		m_bShiftPressed = false;
	}//end shift
	if( me -> button() == LeftButton ) {
		m_bMouseDown = true;
		m_bStartMove = true;

		/* we want multiple selected objects be moved without pressing shift */
		if (count > 1 && m_bSelected == true)
			return;

		bool _select = m_bSelected?false:true;
		m_pView -> clearSelected();
		m_bSelected = _select;
		setSelected( m_bSelected );
		m_pView -> setSelected( this, me );
	} else if( me -> button() == RightButton ) {
		m_pView -> clearSelected();
		setSelected( true );
		m_pView -> setSelected( this, me );
	} else {

		m_pView -> clearSelected();
		m_pView -> resetToolbar();
		setSelected( false );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
Uml::UMLWidget_Type UMLWidget::getBaseType()
{
	return m_pData->m_Type;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void UMLWidget::slotChangeWidget(UMLObject * o) {
	if(m_pObject != o)
		return;
	if(m_pObject)
		setName(m_pObject->getName());//sync names
	calculateSize();
	adjustAssocs( (int)x(), (int)y() );//adjust assoc lines
	if(isVisible())
		update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////

int UMLWidget::getID() {
	return m_pData->m_nId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLWidget::mouseReleaseEvent(QMouseEvent *me) {
	int count = m_pView -> getSelectCount();
	m_bStartMove = false;
	m_bMouseDown = false;

	if (me->button() == LeftButton)
	{
		/* if multiple elements were not moved with the left mouse button,
		 * deselect all and select only the current */
		if ((m_bMoved == false) && (count > 1) && (m_bShiftPressed == false))
		{
			m_pView -> clearSelected();
			m_bSelected = true;
			setSelected( m_bSelected );
			m_pView -> setSelected( this, me );
		}
		m_bShiftPressed = false; // reset the state
	}
	if( me->button() == RightButton ) {
		if (m_pMenu) {
			return;
		}
		startPopupMenu( me->globalPos() );
		return;
	}//end if right button

	if (m_bMoved) {
		m_pView->getDocument()->setModified(true);
	}

	if ( me->button() == LeftButton && me->stateAfter() != ShiftButton ) {
		m_pView->setAssoc(this);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLWidget::init() {
	m_bResizing = false;
	m_bMouseOver = false;


	m_bMouseDown = false;
	m_bSelected = false;
	m_bStartMove = false;
	m_bMoved = false;
	m_bShiftPressed = false;
	m_bActivated = false;
	m_nPressOffsetX = m_nPressOffsetY = 0;
	m_pMenu = 0;
	m_nPosX = m_nOldX = m_nOldY = m_nOldID = 0;
	m_nOldH = m_nOldW = 0;
	connect( m_pView, SIGNAL( sigRemovePopupMenu() ), this, SLOT( slotRemovePopupMenu() ) );
	connect( m_pView, SIGNAL( sigClearAllSelected() ), this, SLOT( slotClearAllSelected() ) );
	connect( m_pView, SIGNAL(sigColorChanged(int)), this, SLOT(slotColorChanged(int)));
	connect( m_pView -> getDocument(), SIGNAL(sigWidgetUpdated(UMLObject *)), this, SLOT(slotChangeWidget(UMLObject *)));
	setZ( 1 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLWidget::slotMenuSelection(int sel) {
	QFont font;
	QColor newColour;
	switch(sel) {
		case ListPopupMenu::mt_Rename:
			m_pView -> getDocument() -> renameUMLObject(m_pObject);
			adjustAssocs( (int)x(), (int)y() );//adjust assoc lines
			break;

		case ListPopupMenu::mt_Delete:
			//remove self from diagram
			m_pView -> removeWidget(this);
			break;


		case ListPopupMenu::mt_Properties:
			if(m_pData->m_Type == wt_Actor || m_pData->m_Type == wt_UseCase ||
			        (m_pData->m_Type == wt_Class && m_pView -> getType() == dt_Class))
				m_pView->getDocument() -> showProperties(this);
			else
				m_pView->getDocument() -> showProperties(m_pObject);
			adjustAssocs( (int)x(), (int)y() );//adjust assoc lines
			break;

		case ListPopupMenu::mt_Line_Color:
			if( KColorDialog::getColor(newColour) ) {
				m_pData->setLineColour(newColour);
				m_pData->setUsesDiagramLineColour(false);

			}
			break;

		case ListPopupMenu::mt_Fill_Color:
			if ( KColorDialog::getColor(newColour) ) {
				m_pData->setFillColour(newColour);
				m_pData->setUsesDiagramFillColour(false);
			}
			break;

		case ListPopupMenu::mt_Use_Fill_Color:
			m_pData->setUseFillColor(m_pData->m_bUseFillColor?false:true);
			m_pData->setUsesDiagramUseFillColour(false);
			m_pView->selectionUseFillColor(m_pData->m_bUseFillColor);
			break;

		case ListPopupMenu::mt_Delete_Selection:
			m_pView -> deleteSelection();
			break;

		case ListPopupMenu::mt_Change_Font:
			font = getFont();
			if( KFontDialog::getFont( font, false, m_pView ) )
				setFont( font );
			break;

		case ListPopupMenu::mt_Change_Font_Selection:
			font = getFont();
			if( KFontDialog::getFont( font, false, m_pView ) )
				m_pView -> selectionSetFont( font );
			break;

		case ListPopupMenu::mt_Cut:
			m_pView -> setStartedCut();
			m_pView -> getDocument() -> editCut();
			break;

		case ListPopupMenu::mt_Copy:
			m_pView -> getDocument() -> editCopy();
			break;

		case ListPopupMenu::mt_Paste:
			m_pView -> getDocument() -> editPaste();
			break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLWidget::slotWidgetMoved(int /*id*/) {}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLWidget::serialize(QDataStream *s, bool archive, int fileversion) {
	if(archive) {
		synchronizeData();
	}
	return m_pData->serialize(s, archive, fileversion);
}

/** Returns the amount of bytes needed to serialize this m_pObject */
/* If the serialization method of this class is changed this function will have to be CHANGED TOO*/
/*This function is used by the Copy and Paste Functionality*/
/*The Size in bytes of a serialized QString Object is long sz:
		if ( (sz =str.length()*sizeof(QChar)) && !(const char*)str.unicode() )
		{
			sz = size of Q_UINT32; //  typedef unsigned int	Q_UINT32;		// 32 bit unsigned
		}
	This calculation is valid only for QT 2.1.x or superior, this is totally incompatible with QT 2.0.x or QT 1.x or inferior
	That means the copy and paste functionality will work on with QT 2.1.x or superior
*/

long UMLWidget::getClipSizeOf() {
	synchronizeData();

	return m_pData->getClipSizeOf();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLWidget::slotColorChanged(int m_pViewID) {
	//only change if on the diagram concerned
	if(m_pView->getID() != m_pViewID) {
		return;
	}
	if ( m_pData->getUsesDiagramFillColour() ) {
		m_pData->setFillColour( m_pView->getFillColour() );
	}
	if ( m_pData->getUsesDiagramLineColour() ) {
		m_pData->setLineColour( m_pView->getLineColor() );
	}
	if ( m_pData->getUsesDiagramUseFillColour() ) {
		m_pData->setUseFillColor( m_pView->getUseFillColor() );
	}
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLWidget::mouseDoubleClickEvent( QMouseEvent * me ) {
	if( me -> button() != LeftButton )
		return;
	if(m_pView -> m_CurrentCursor == WorkToolBar::tbb_Arrow && m_pData->m_Type >= wt_Actor &&
	        m_pData->m_Type <= wt_Object)
		slotMenuSelection(ListPopupMenu::mt_Properties);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QColor UMLWidget::getFillColour() {
	return m_pData->getFillColour();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QColor UMLWidget::getLineColor() {
	return m_pData->getLineColour();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLWidget::setUseFillColor(bool fc) {
	m_pData->setUseFillColor(fc);
	m_pData->setUsesDiagramUseFillColour(false);
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLWidget::setLineColour(QColor colour) {
	m_pData->setLineColour(colour);
	m_pData->setUsesDiagramLineColour(false);
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLWidget::setFillColour(QColor colour) {
	m_pData->setFillColour(colour);
	m_pData->setUsesDiagramFillColour(false);
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLWidget::drawSelected(QPainter * p, int offsetX, int offsetY) {
	int w = width();
	int h = height();
	int s = 4;
	QBrush brush(blue);
	p -> fillRect(offsetX, offsetY, s,  s, brush);
	p -> fillRect(offsetX, offsetY + h - s, s, s, brush);
	p -> fillRect(offsetX + w - s, offsetY, s, s, brush);
	p -> fillRect(offsetX + w - s, offsetY + h - s, s, s, brush);

}



bool UMLWidget::activate(IDChangeLog* /*ChangeLog  = 0 */) {
	setSize( m_pData -> getWidth(), m_pData -> getHeight() );
	m_bActivated = true;
	if( m_pObject )
		m_Name = m_pObject -> getName();
	calculateSize();
	if( m_pView -> getPastePoint().x() != 0 ) {
		FloatingText * ft = 0;
		QPoint point = m_pView -> getPastePoint();
		int x = point.x() + m_pData -> getX();
		int y = point.y() + m_pData -> getY();
		x = x < 0?0:x;
		y = y < 0?0:y;
		if( m_pView -> getType() == dt_Sequence ) {
			switch( getBaseType() ) {
				case wt_Object:
				case wt_Message:
					setY( m_pData -> getY() );
					setX( x );
					break;

				case wt_Text:
					ft = static_cast<FloatingText *>( this );
					switch( ft  -> getRole() ) {
						case tr_Seq_Message:
							setX( x );
							setY( m_pData -> getY() );
							break;

						default:
							setX( m_pData -> getX() );
							setY( m_pData -> getY() );
							break;
					}//end switch role
					break;

				default:
					setY( y );
					break;
			}//end switch base type
		}//end if sequence
		else {
			setX( x );
			setY( y );
		}
		moveEvent( 0 );//for object widget to move lines
	}//end if pastepoint
	else {
		setX( m_pData -> getX() );
		setY( m_pData -> getY() );
	}
	if ( m_pView -> getPaste() )
		m_pView -> createAutoAssociations( this );
	return true;
}

/** Read property of bool m_bActivated. */
const bool& UMLWidget::isActivated() {
	return m_bActivated;
}

UMLWidgetData* UMLWidget::getData() {
	//	synchronizeData();
	return m_pData;
}

void UMLWidget::synchronizeData() {
	m_pData->setX( (int)x() );
	m_pData->setY( (int)y() );
	m_pData -> setWidth( width() );
	m_pData -> setHeight( height() );
}

void UMLWidget::setActivated(bool Active /*=true*/) {
	m_bActivated = Active;
}

void UMLWidget::adjustAssocs(int x, int y)
{
	AssociationWidgetListIt assoc_it(m_pData->m_Assocs);
	AssociationWidget* assocwidget = 0;
	while((assocwidget=assoc_it.current())) {
		++assoc_it;
		assocwidget->widgetMoved(this, x, y);
	}
}



void UMLWidget::startPopupMenu(QPoint At) {
	slotRemovePopupMenu();
	//if in a multi- selection to a specific m_pMenu for that
	int count = m_pView -> getSelectCount();
	//a MessageWidget when slected will select its text widget and vice versa
	//so take that into account for popup menu.
	if( m_bSelected ) {
		if( m_pView -> getType() == dt_Sequence ) {
			if( getBaseType() == wt_Message && count == 2 ) {
				m_pMenu = new ListPopupMenu(static_cast<QWidget*>(m_pView), this);
			} else if( getBaseType() == wt_Text &&
				   ((FloatingText*)this) -> getRole() == tr_Seq_Message && count == 2 ) {
				m_pMenu = new ListPopupMenu(static_cast<QWidget*>(m_pView), this);
			} else if( count > 1 ) {
				m_pMenu = new ListPopupMenu(static_cast<QWidget*>(m_pView), this, true );
			} else {
				m_pMenu = new ListPopupMenu(static_cast<QWidget*>(m_pView), this);
			}
		} else if( count > 1 ) {
			m_pMenu = new ListPopupMenu(static_cast<QWidget*>(m_pView), this, true );
		} else {
			m_pMenu = new ListPopupMenu(static_cast<QWidget*>(m_pView), this);
		}
	} else {
		m_pMenu = new ListPopupMenu(static_cast<QWidget*>(m_pView), this);
	}

	m_pMenu->popup(At);

	connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuSelection(int)));

	m_bMouseDown = m_bStartMove = false;
}

void UMLWidget::slotRemovePopupMenu() {
	if(m_pMenu) {
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuSelection(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
}

bool UMLWidget::onWidget(const QPoint & p) {


	return ( x() <= p.x() && x() + width() >= p.x() &&
	         y() <= p.y() && y() + height() >= p.y() );

}

void UMLWidget::draw( QPainter & /*p*/, int /*offsetX*/, int /*offsetY*/ ) {

	//override this function in child classes and paint to the painter
}

void UMLWidget::drawShape(QPainter &p ) {
	draw( p, (int)x(), (int)y() );
}

void UMLWidget::setSelected(bool _select) {
	if( _select ) {
		if( m_pView -> getSelectCount() == 0 ) {
			switch( m_pData -> m_Type ) {
				case wt_Actor:
				case wt_UseCase:
				case wt_Class:
					m_pView -> showDocumentation( m_pObject, false );
					break;

				case wt_Object:
				case wt_State:
				case wt_Activity:
				case wt_Note:
					m_pView -> showDocumentation( this, false );
					break;

				default:
					break;
			}//end switch
		}//end if
		if( m_pData -> m_Type != wt_Text )
			setZ( 2 );//keep text on top so don't touch it's Z value
	}//end if
	else {
		if( m_pData -> m_Type != wt_Text )
			setZ( 1 );
		if( m_bSelected )
			m_pView -> updateDocumentation( true );
	}
	m_bSelected = _select;
	update();
}

void UMLWidget::slotClearAllSelected()
{
	setSelected( false );
}

void UMLWidget::setView(UMLView * v) {
	//remove signals from old view - was probably 0 anyway
	disconnect( m_pView, SIGNAL( sigRemovePopupMenu() ), this, SLOT( slotRemovePopupMenu() ) );
	disconnect( m_pView, SIGNAL( sigClearAllSelected() ), this, SLOT( slotClearAllSelected() ) );
	disconnect( m_pView, SIGNAL(sigColorChanged(int)), this, SLOT(slotColorChanged(int)));
	m_pView = v;
	connect( m_pView, SIGNAL( sigRemovePopupMenu() ), this, SLOT( slotRemovePopupMenu() ) );
	connect( m_pView, SIGNAL( sigClearAllSelected() ), this, SLOT( slotClearAllSelected() ) );
	connect( m_pView, SIGNAL(sigColorChanged(int)), this, SLOT(slotColorChanged(int)));
}

void UMLWidget::setX( int x ) {
	m_pData -> setX( x );
	if( m_pView -> getSnapToGrid() ) {
		int gridX = m_pView -> getSnapX();

		int modX = m_pData -> getX() % gridX;
		x -= modX;
		if( modX >= ( gridX / 2 ) )
			x += gridX;
	}
	QCanvasItem::setX( x );
}

void UMLWidget::setY( int y ) {
	m_pData -> setY( y );
	if( m_pView -> getSnapToGrid() ) {
		int gridY = m_pView -> getSnapY();
		int modY = m_pData -> getY() % gridY;
		y -= modY;
		if( modY >= ( gridY / 2 ) )
			y += gridY;
	}
	QCanvasItem::setY( y );
}

void UMLWidget::setName(QString strName) {
	m_Name = strName;
	calculateSize();
	adjustAssocs( m_pData -> getX(), m_pData -> getY() );
}


void UMLWidget::cleanup() {
	if( m_pData ) {
		delete m_pData;
		m_pData = 0;
	}
}

void UMLWidget::slotSnapToGrid( ) {
	setX( m_pData -> getX() );
	setY( m_pData -> getY() );
}





#include "umlwidget.moc"
