/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//qt includes
#include <qpainter.h>
//kde includes
#include <klineeditdlg.h>
#include <kdebug.h>
#include <kcursor.h>
//app includes
#include "messagewidget.h"
#include "floatingtext.h"
#include "objectwidget.h"
#include "classifier.h"
#include "dialogs/selectopdlg.h"
#include "umlview.h"
#include "listpopupmenu.h"

MessageWidget::MessageWidget(UMLView* view, UMLWidgetData* pData) : UMLWidget(view, pData) {
	init();
	m_pData->setType(wt_Message);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MessageWidget::MessageWidget(UMLView* view, UMLWidget* a, UMLWidget* b, FloatingText* ft, int id, int y,
			     Sequence_Message_Type sequenceMessageType)
	: UMLWidget(view, id, new MessageWidgetData(view->getOptionState(), sequenceMessageType)) {
	init();
	m_pFText = ft;
	m_pWA = a;
	m_pWB = b;
	m_nY = y;
	m_pData->setType(wt_Message);
	((MessageWidgetData*)m_pData)->setWidgetAID( ((ObjectWidget *)m_pWA)-> getLocalID() );
	((MessageWidgetData*)m_pData)->setWidgetBID( ((ObjectWidget *)m_pWB)-> getLocalID() );
	ft->setUMLObject(b->getUMLObject());
	ft -> setMessage(this);
	connect(m_pWA, SIGNAL(sigWidgetMoved(int)), this, SLOT(slotWidgetMoved(int)));
	connect(m_pWB, SIGNAL(sigWidgetMoved(int)), this, SLOT(slotWidgetMoved(int)));
	calculateWidget();
	y = y < getMinHeight() ? getMinHeight() : y;
	y = y > getMaxHeight() ? getMaxHeight() : y;
	m_nY = y;

	connect(this, SIGNAL(sigMessageMoved()), m_pWA, SLOT(slotMessageMoved()) );
	connect(this, SIGNAL(sigMessageMoved()), m_pWB, SLOT(slotMessageMoved()) );
	static_cast<ObjectWidget*>(m_pWA)->messageAdded(this);
	static_cast<ObjectWidget*>(m_pWB)->messageAdded(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MessageWidget::MessageWidget(UMLView* view, Sequence_Message_Type sequenceMessageType)
	: UMLWidget(view, new MessageWidgetData(view->getOptionState(), sequenceMessageType)) {
	m_pData->setType(wt_Message);
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MessageWidget::init() {
	m_bIgnoreSnapToGrid = true;
	m_bIgnoreSnapComponentSizeToGrid = true;
	m_pWA = m_pWB = m_pFText = 0;
	m_nY = 0;
	setVisible(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MessageWidget::~MessageWidget() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MessageWidget::draw(QPainter& p, int offsetX, int offsetY) {
	MessageWidgetData* widgetdata = static_cast<MessageWidgetData*>(m_pData);

	if(!m_pWA || !m_pWB) {
		return;
	}

	if (widgetdata->getSequenceMessageType() == sequence_message_synchronous) {
		drawSynchronous(p, offsetX, offsetY);
	} else if (widgetdata->getSequenceMessageType() == sequence_message_asynchronous) {
		drawAsynchronous(p, offsetX, offsetY);
	} else {
		kdWarning() << "Unknown message type" << endl;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MessageWidget::drawSynchronous(QPainter& p, int offsetX, int offsetY) {
	p.setPen( m_pData->getLineColour() );
	int x1 = (int)m_pWA->x();
	int x2 = (int)m_pWB->x();
	int w = width() - 1;
	int h = height();

	if(m_pWA == m_pWB) {
		p.fillRect( offsetX, offsetY, 17, h,  QBrush(white) );			//box
		p.drawRect(offsetX, offsetY, 17, h);					//box
		p.drawLine(offsetX + 17, offsetY + 3, offsetX + w, offsetY + 3);	//arrow body lines
		p.drawLine(offsetX + w, offsetY + 3, offsetX + w, offsetY + h - 3);
		p.drawLine(offsetX + w, offsetY + h - 3, offsetX + 17, offsetY + h - 3);
		p.drawLine(offsetX + 17, offsetY + h - 3, offsetX + 17 + 4, offsetY + h);	//arrow head
		p.drawLine(offsetX + 17, offsetY + h - 3, offsetX + 17 + 4, offsetY + h - 6);
	} else if(x1 < x2) {
		QPen pen = p.pen();
		p.fillRect( offsetX + w - 16, offsetY, 17, h,  QBrush(white) );		//box
		p.drawRect(offsetX + w - 16, offsetY, 17, h);				//box
		p.drawLine(offsetX, offsetY + 4, offsetX + w - 16, offsetY + 4);	//arrow line
		QPointArray points;
		points.putPoints(0, 3, offsetX + w - 17,offsetY + 4, offsetX + w - 17 - 4,   //arrow head
				 offsetY + 1, offsetX + w - 17 - 4,offsetY + 7);
		p.setBrush( QBrush(pen.color()) );
		p.drawPolygon(points);

		p.drawLine(offsetX, offsetY + h - 3, offsetX + 4, offsetY + h);		 //return arrow
		p.drawLine(offsetX, offsetY + h - 3, offsetX + 4, offsetY + h - 6);	 //return arrow

		pen.setStyle(Qt::DotLine);
		p.setPen(pen);

		p.drawLine(offsetX, offsetY + h - 3, offsetX + w - 16, offsetY + h - 3);  //return line
	} else	{
		QPen pen = p.pen();
		p.fillRect( offsetX, offsetY, 17, h,  QBrush(white) );			//box
		p.drawRect(offsetX, offsetY, 17, h);					//box
		p.drawLine(offsetX + 18, offsetY + 4, offsetX + w, offsetY + 4);	//arrow line
		QPointArray points;
		points.putPoints(0, 3, offsetX + 17,offsetY + 4, offsetX + 17 + 4,offsetY + 1,  //arrow head
				 offsetX + 17 + 4,offsetY + 7);
		p.setBrush( QBrush(pen.color()) );
		p.drawPolygon(points);

		p.drawLine(offsetX + w, offsetY + h - 3, offsetX + w - 4, offsetY + h);		 //return arrow
		p.drawLine(offsetX + w, offsetY + h - 3, offsetX + w - 4, offsetY + h - 6);	 //return arrow

		pen.setStyle(Qt::DotLine);
		p.setPen(pen);

		p.drawLine(offsetX + 18, offsetY + h - 3, offsetX + w, offsetY + h - 3);  //return line
	}

	if(m_bSelected) {
		drawSelected(&p, offsetX, offsetY, true);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MessageWidget::drawAsynchronous(QPainter& p, int offsetX, int offsetY) {
	p.setPen(m_pData->getLineColour());
	int x1 = (int)m_pWA->x();
	int x2 = (int)m_pWB->x();
	int w = width() - 1;
	int h = height() - 1;
	if(m_pWA == m_pWB) {
		p.drawLine(offsetX, offsetY, offsetX + w, offsetY);
		p.drawLine(offsetX + w, offsetY, offsetX + w, offsetY + h - 3);
		p.drawLine(offsetX + w, offsetY + h - 3, offsetX, offsetY + h - 3);
		p.drawLine(offsetX, offsetY + h - 3, offsetX + 4, offsetY + h);
		p.drawLine(offsetX, offsetY + h - 3, offsetX + 4, offsetY + h - 6);
	} else if(x1 < x2) {
		p.drawLine(offsetX, offsetY + 4, offsetX + w, offsetY + 4);
		p.drawLine(offsetX + w, offsetY + 4, offsetX + w - 4, offsetY + 1);
		p.drawLine(offsetX + w, offsetY + 4, offsetX + w - 4, offsetY + 7);
	} else	{
		p.drawLine(offsetX, offsetY + 4, offsetX + w, offsetY + 4);
		p.drawLine(offsetX, offsetY + 4, offsetX + 4, offsetY + 1);
		p.drawLine(offsetX, offsetY + 4, offsetX + 4, offsetY + 7);
	}

	if (m_bSelected && m_pWA == m_pWB) {
		drawSelected(&p, offsetX, offsetY, true);
	} else if (m_bSelected) {
		drawSelected(&p, offsetX, offsetY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MessageWidget::moveEvent(QMoveEvent* /*m*/) {
	if (!m_pFText) {
		return;
	}
	if (m_pView->getSelectCount() > 2) {
		return;
	}
	m_pFText->setPositionFromMessage();

	emit sigMessageMoved();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MessageWidget::resizeEvent(QResizeEvent */*re*/) {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MessageWidget::calculateWidget() {
	calculateDimensions();

	setVisible(true);

	setX(m_nPosX);
	setY(m_nY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MessageWidget::slotWidgetMoved(int id) {
	if(m_pWA -> getID() == id || m_pWB -> getID() == id) {
		m_nY = (int)y();
		m_nY = m_nY < getMinHeight()?getMinHeight():m_nY;
		m_nY = m_nY > getMaxHeight()?getMaxHeight():m_nY;
		calculateWidget();
	}
	if( !m_pFText )
		return;
	if( m_pView -> getSelectCount() > 1 )
		return;
	m_pFText->setPositionFromMessage();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool MessageWidget::contains(UMLWidget * w) {
	if(m_pWA == w || m_pWB == w)
		return true;
	else
		return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MessageWidget::slotMenuSelection(int sel) {
	if(sel == ListPopupMenu::mt_Delete)
		m_pView -> removeWidget(this);//this will cleanup this widget and the text widget
	else
		m_pFText -> slotMenuSelection(sel);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void MessageWidget::mouseDoubleClickEvent(QMouseEvent * /*me*/) {
	if(m_pView -> getCurrentCursor() == WorkToolBar::tbb_Arrow)
		m_pFText -> slotMenuSelection(ListPopupMenu::mt_Select_Operation);
}

bool MessageWidget::activate(IDChangeLog * Log /*= 0*/) {
	bool status = UMLWidget::activate(Log);
	if(!status)
		return false;
	MessageWidgetData* widgetdata = static_cast<MessageWidgetData*>(m_pData);
	if( !m_pFText ) {
		m_pFText = new FloatingText( m_pView, tr_Seq_Message, "" );
		m_pFText->getData()->setFont(widgetdata->getFont());
	}
	if(widgetdata->getTextID() != -1)
	{
		m_pFText -> setID( widgetdata->getTextID() );   //this is wrong//change when anchors back
	}                                                                     //need to assign new id
	else {
		int newid = m_pView->getDocument()->getUniqueID();
		m_pFText -> setID(newid);
		widgetdata->setTextID( newid );
	}
	QString seq = widgetdata->getSequenceNumber();
	m_pFText -> setSeqNum(seq);
	m_pFText -> setOperation( widgetdata->getOperation() );
	QString messageText = m_pFText->getText();
	m_pFText->setActivated();
	m_pFText->setVisible( messageText.length() > 1 );

	m_pWA = m_pView -> findWidget( widgetdata -> getWidgetAID() );
	m_pWB = m_pView -> findWidget( widgetdata -> getWidgetBID() );

	if(!m_pWA || !m_pWB) {
		kdDebug() << "Can't make message" << endl;
		return false;
	}
	m_pFText -> setUMLObject( m_pWB -> getUMLObject() );
	m_pFText -> setMessage(this);
	connect(m_pWA, SIGNAL(sigWidgetMoved(int)), this, SLOT(slotWidgetMoved(int)));
	connect(m_pWB, SIGNAL(sigWidgetMoved(int)), this, SLOT(slotWidgetMoved(int)));
	calculateDimensions();

	connect(this, SIGNAL(sigMessageMoved()), m_pWA, SLOT(slotMessageMoved()) );
	connect(this, SIGNAL(sigMessageMoved()), m_pWB, SLOT(slotMessageMoved()) );
	static_cast<ObjectWidget*>(m_pWA)->messageAdded(this);
	static_cast<ObjectWidget*>(m_pWB)->messageAdded(this);

	return status;
}

/** Synchronizes the Widget's m_pData member with its display properties, for exmaple:
the X and Y position of the widget, etc */
void MessageWidget::synchronizeData() {
	UMLWidget::synchronizeData();
	if(isActivated()) {
		MessageWidgetData* p_widgetdata = 0;
		if((p_widgetdata = dynamic_cast<MessageWidgetData*>(m_pData))) {
			if(dynamic_cast<ObjectWidget *>(m_pWA)) {
				p_widgetdata->setWidgetAID( static_cast<ObjectWidget*>(m_pWA)->getLocalID() );
			}
			if(dynamic_cast<ObjectWidget *>(m_pWB)) {
				p_widgetdata->setWidgetBID( static_cast<ObjectWidget*>(m_pWB)->getLocalID() );
			}
			p_widgetdata->setSequenceNumber(m_pFText -> getSeqNum());
			p_widgetdata->setOperation(m_pFText -> getOperation());
			p_widgetdata->setTextID(m_pFText -> getID());
		}
	}
}

void MessageWidget::calculateDimensions() {
	MessageWidgetData* widgetdata = static_cast<MessageWidgetData*>(m_pData);

	if (widgetdata->getSequenceMessageType() == sequence_message_synchronous) {
		calculateDimensionsSynchronous();
	} else if (widgetdata->getSequenceMessageType() == sequence_message_asynchronous) {
		calculateDimensionsAsynchronous();
	} else {
		kdWarning() << "Unknown message type" << endl;
	}
}

void MessageWidget::calculateDimensionsSynchronous() {
	int x = 0;

	int x1 = (int)m_pWA->x();
	int x2 = (int)m_pWB->x();
	int w1 = m_pWA->width() / 2;
	int w2 =  m_pWB->width() / 2;
	x1 += w1;
	x2 += w2;

	int widgetWidth = 0;
	int widgetHeight = 0;
	if( m_pWA == m_pWB ) {
		widgetWidth = 50;
		x = x1 - 2;
	} else if( x1 < x2 ) {
		x = x1;
		widgetWidth = x2 - x1 + 8;
	} else {
		x = x2 - 8;
		widgetWidth = x1 - x2 + 8;
	}

	if ( height() < 20 ) {
		widgetHeight = 20;
	} else {
		widgetHeight = height();
	}

	m_nPosX = x;
	setSize(widgetWidth, widgetHeight);
	adjustAssocs( (int)this->x(), (int)this->y() );//adjust assoc lines
}

void MessageWidget::calculateDimensionsAsynchronous() {
	int x = 0;

	int x1 = (int)m_pWA->x();
	int x2 = (int)m_pWB->x();
	int w1 = m_pWA -> width() / 2;
	int w2 =  m_pWB -> width() / 2;
	x1 += w1;
	x2 += w2;

	int widgetWidth = 0;
	int widgetHeight = 8;
	if( m_pWA == m_pWB ) {
		widgetWidth = 50;
		x = x1;
		if( height() < 20 ) {
			widgetHeight = 20;
		} else {
			widgetHeight = height();
		}
	} else if( x1 < x2 ) {
		x = x1;
		widgetWidth = x2 - x1;
	} else {
		x = x2;
		widgetWidth = x1 - x2;
	}
	x += 1;
	widgetWidth -= 2;
	m_nPosX = x;
	setSize(widgetWidth, widgetHeight);
	adjustAssocs( (int)this->x(), (int)this->y() );//adjust assoc lines
}

void MessageWidget::cleanup() {
	if (m_pWA) {
		disconnect(this, SIGNAL(sigMessageMoved()), m_pWA, SLOT(slotMessageMoved()) );
		static_cast<ObjectWidget*>(m_pWA)->messageRemoved(this);
	}
	if (m_pWB) {
		disconnect(this, SIGNAL(sigMessageMoved()), m_pWB, SLOT(slotMessageMoved()) );
		static_cast<ObjectWidget*>(m_pWB)->messageRemoved(this);
	}

	UMLWidget::cleanup();
	if (m_pFText) {
		m_pView->removeWidget(m_pFText);
	}
}

void MessageWidget::mouseMoveEvent(QMouseEvent *me) {
	int newX = 0, newY = 0, count;
	int moveX, moveY;
	if( m_bResizing ) {
		int heightA = (int)((ObjectWidget*)m_pWA) -> getEndLineY();
		int heightB = (int)((ObjectWidget*)m_pWB) -> getEndLineY();
		int height = heightA;
		if( heightA > heightB )
			height = heightB;
		moveY = (int)me -> y() - m_nPressOffsetY - m_nOldY;
		newY = m_nOldH + moveY;
		newY = newY < 20 ? 20 : newY;
		newY = newY > height - (int)y()? height - (int)y() : newY;
		setSize( width(), newY );
		return;
	}
	if( !m_bSelected )
		m_pView -> setSelected( this, me );
	m_bSelected = true;
	count = m_pView -> getSelectCount();

	if( !m_bMouseDown )
		if( me -> button() != LeftButton )
			return;
	//If not m_bStartMove means moving as part of selection
	//me->pos() will have the amount we need to move.
	if(!m_bStartMove) {
		moveX = (int)me -> x();
		moveY = (int)me -> y();
	} else {
		//we started the move so..
		//move any others we are selected
		moveX = (int)me -> x() - m_nOldX - m_nPressOffsetX;
		moveY = (int)me -> y() - m_nOldY - m_nPressOffsetY;
		if( ((int)x() + moveX) < 0 )
			moveX = 0;
		if( ( (int)y() + moveY) < 0 )
			moveY = 0;
		if( count > 2 )
			m_pView -> moveSelected( this, moveX, 0 );
	}
	newX = (int)x() + moveX;
	newY = (int)y() + moveY;

	newX = newX < 0?0:newX;
	newY = newY < 0?0:newY;
	if( count > 2 )
		newY = (int)this -> y();
	//only change y if not selected
	newX = m_nPosX;
	newY = newY < getMinHeight() ? getMinHeight() : newY;
	newY = newY > getMaxHeight() ? getMaxHeight() : newY;

	if (m_nOldX != newX || m_nOldY != newY) {
		m_bMoved = true;
	}
	m_nOldX = newX;
	m_nOldY = newY;
	setX( newX );
	setY( newY );
	adjustAssocs(newX, newY);
	moveEvent(0);
}

void MessageWidget::setSelected(bool _select) {
	if( m_nOldID == -10 )//used to stop a recursive call
	{
		m_nOldID = 0;
		return;
	}
	UMLWidget::setSelected( _select );
	if( !m_pFText )
		return;
	if( m_bSelected && m_pFText -> getSelected() )
		return;
	if( !m_bSelected && !m_pFText -> getSelected() )
		return;

	m_nOldID = -10;
	m_pView -> setSelected( m_pFText, 0 );
	m_pFText -> setSelected( m_bSelected );
}

int MessageWidget::getMinHeight() {
	if( !m_pWA || !m_pWB ) {
		return 0;
	}
	int heightA = (int)m_pWA->y() + m_pWA->height();
	int heightB = (int)m_pWB->y() + m_pWB->height();
	int height = heightA;
	if( heightA < heightB ) {
		height = heightB;
	}
	return height;
}

int MessageWidget::getMaxHeight() {
	if( !m_pWA || !m_pWB ) {
		return 0;
	}
	int heightA = (int)((ObjectWidget*)m_pWA)->getEndLineY();
	int heightB = (int)((ObjectWidget*)m_pWB)->getEndLineY();
	int height = heightA;
	if( heightA > heightB ) {
		height = heightB;
	}
	return (height - this->height());
}

void MessageWidget::mousePressEvent(QMouseEvent* me) {
	UMLWidget::mousePressEvent(me);
	if ( m_pView->getCurrentCursor() != WorkToolBar::tbb_Arrow ) {
		return;
	}
	//resize only interests a message to self or an asynchronous message
	MessageWidgetData* widgetdata = static_cast<MessageWidgetData*>(m_pData);
	if ( widgetdata->getSequenceMessageType() == sequence_message_asynchronous && m_pWA != m_pWB ) {
		return;
	}
	int m = 10;
	//see if clicked on bottom right corner
	if( (m_nOldX + m_nPressOffsetX) >= ((int)x() + width() - m) &&
	    (m_nOldY + m_nPressOffsetY) >= ((int)y() + height() - m) && me->button() == LeftButton) {
		m_bResizing = true;
		m_pView->setCursor(KCursor::sizeVerCursor());
		m_nOldH = height();
	} else {
		m_bResizing = false;
		m_pView -> setCursor(KCursor::arrowCursor());
	}
}

void MessageWidget::mouseReleaseEvent( QMouseEvent * me ) {
	UMLWidget::mouseReleaseEvent( me );
	m_bResizing = false;
	m_pView->setCursor( KCursor::arrowCursor() );
}

void MessageWidget::setWidgetA(UMLWidget * wa) {
	m_pWA = wa;
	((MessageWidgetData*)m_pData)->setWidgetAID( ((ObjectWidget *)wa) -> getLocalID() );
}

void MessageWidget::setWidgetB(UMLWidget * wb) {
	m_pWB = wb;
	((MessageWidgetData*)m_pData)->setWidgetBID( ((ObjectWidget *)wb) -> getLocalID() );
}

#include "messagewidget.moc"
