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
#include "umldoc.h"
#include "uml.h"
#include "listpopupmenu.h"

MessageWidget::MessageWidget(UMLView * view, ObjectWidget* a, ObjectWidget* b,
			     FloatingText* ft, int y,
			     Uml::Sequence_Message_Type sequenceMessageType, Uml::IDType id)
  : UMLWidget(view, id) {
	init();
	m_pOw[Uml::A] = a;
	m_pOw[Uml::B] = b;
	m_pFText = ft;
	m_nY = y;
	m_sequenceMessageType = sequenceMessageType;

	//CHECK: This is contorted - it should be in the caller's responsibility:
	ft->setUMLObject(b->getUMLObject());
	ft -> setLink(this);

	connect(m_pOw[Uml::A], SIGNAL(sigWidgetMoved(Uml::IDType)), this, SLOT(slotWidgetMoved(Uml::IDType)));
	connect(m_pOw[Uml::B], SIGNAL(sigWidgetMoved(Uml::IDType)), this, SLOT(slotWidgetMoved(Uml::IDType)));
	calculateWidget();
	y = y < getMinHeight() ? getMinHeight() : y;
	y = y > getMaxHeight() ? getMaxHeight() : y;
	m_nY = y;

	connect(this, SIGNAL(sigMessageMoved()), m_pOw[Uml::A], SLOT(slotMessageMoved()) );
	connect(this, SIGNAL(sigMessageMoved()), m_pOw[Uml::B], SLOT(slotMessageMoved()) );
	m_pOw[Uml::A] -> messageAdded(this);
	m_pOw[Uml::B] -> messageAdded(this);
}

MessageWidget::MessageWidget(UMLView * view, Uml::Sequence_Message_Type seqMsgType, Uml::IDType id)
  : UMLWidget(view, id) {
	init();
	m_sequenceMessageType = seqMsgType;
}

void MessageWidget::init() {
	UMLWidget::setBaseType(Uml::wt_Message);
	m_bIgnoreSnapToGrid = true;
	m_bIgnoreSnapComponentSizeToGrid = true;
	m_pOw[Uml::A] = m_pOw[Uml::B] = NULL;
	m_pFText = NULL;
	m_nY = 0;
	m_inSelection = false;
	setVisible(true);
}

MessageWidget::~MessageWidget() {
}

void MessageWidget::draw(QPainter& p, int offsetX, int offsetY) {
	if(!m_pOw[Uml::A] || !m_pOw[Uml::B]) {
		return;
	}
	UMLWidget::draw(p, offsetX, offsetY);
	if (m_sequenceMessageType == Uml::sequence_message_synchronous) {
		drawSynchronous(p, offsetX, offsetY);
	} else if (m_sequenceMessageType == Uml::sequence_message_asynchronous) {
		drawAsynchronous(p, offsetX, offsetY);
	} else {
		kdWarning() << "Unknown message type" << endl;
	}
}

void MessageWidget::drawSynchronous(QPainter& p, int offsetX, int offsetY) {
	int x1 = m_pOw[Uml::A]->getX();
	int x2 = m_pOw[Uml::B]->getX();
	int w = getWidth() - 1;
	int h = getHeight();

	bool messageOverlaps = m_pOw[Uml::A] -> messageOverlap( getY(), this );

	if(m_pOw[Uml::A] == m_pOw[Uml::B]) {
		p.fillRect( offsetX, offsetY, 17, h,  QBrush(white) );			//box
		p.drawRect(offsetX, offsetY, 17, h);					//box
		p.drawLine(offsetX + 17, offsetY + 3, offsetX + w, offsetY + 3);	//arrow body lines
		p.drawLine(offsetX + w, offsetY + 3, offsetX + w, offsetY + h - 3);
		p.drawLine(offsetX + w, offsetY + h - 3, offsetX + 17, offsetY + h - 3);
		p.drawLine(offsetX + 17, offsetY + h - 3, offsetX + 17 + 4, offsetY + h);	//arrow head
		p.drawLine(offsetX + 17, offsetY + h - 3, offsetX + 17 + 4, offsetY + h - 6);
	} else if(x1 < x2) {
		if (messageOverlaps)  {
			offsetX += 8;
			w -= 8;
		}
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
		if (messageOverlaps)  {
			offsetX -= 8; //reset for drawSelected()
		}
	} else	{
		if (messageOverlaps)  {
			w -=8;
		}
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

void MessageWidget::drawAsynchronous(QPainter& p, int offsetX, int offsetY) {
	int x1 = m_pOw[Uml::A]->getX();
	int x2 = m_pOw[Uml::B]->getX();
	int w = getWidth() - 1;
	int h = getHeight() - 1;
	bool messageOverlapsA = m_pOw[Uml::A] -> messageOverlap( getY(), this );
	//bool messageOverlapsB = m_pOw[Uml::B] -> messageOverlap( getY(), this );

	if(m_pOw[Uml::A] == m_pOw[Uml::B]) {
		if (messageOverlapsA)  {
			offsetX += 7;
			w -= 7;
		}
		p.drawLine(offsetX, offsetY, offsetX + w, offsetY);
		p.drawLine(offsetX + w, offsetY, offsetX + w, offsetY + h - 3);
		p.drawLine(offsetX + w, offsetY + h - 3, offsetX, offsetY + h - 3);
		p.drawLine(offsetX, offsetY + h - 3, offsetX + 4, offsetY + h);
		p.drawLine(offsetX, offsetY + h - 3, offsetX + 4, offsetY + h - 6);
		if (messageOverlapsA)  {
			offsetX -= 7; //reset for drawSelected()
		}
	} else if(x1 < x2) {
		if (messageOverlapsA)  {
			offsetX += 7;
			w -= 7;
		}
		p.drawLine(offsetX, offsetY + 4, offsetX + w, offsetY + 4);
		p.drawLine(offsetX + w, offsetY + 4, offsetX + w - 4, offsetY + 1);
		p.drawLine(offsetX + w, offsetY + 4, offsetX + w - 4, offsetY + 7);
		if (messageOverlapsA)  {
			offsetX -= 7;
		}
	} else	{
		if (messageOverlapsA)  {
			w -= 7;
		}
		p.drawLine(offsetX, offsetY + 4, offsetX + w, offsetY + 4);
		p.drawLine(offsetX, offsetY + 4, offsetX + 4, offsetY + 1);
		p.drawLine(offsetX, offsetY + 4, offsetX + 4, offsetY + 7);
	}

	if (m_bSelected && m_pOw[Uml::A] == m_pOw[Uml::B]) {
		drawSelected(&p, offsetX, offsetY, true);
	} else if (m_bSelected) {
		drawSelected(&p, offsetX, offsetY);
	}
}

void MessageWidget::setTextPosition() {
	if (m_pFText == NULL) {
		kdDebug() << "MessageWidget::setTextPosition: m_pFText is NULL"
			  << endl;
		return;
	}
	if (m_pFText->getText().isEmpty())
		return;
	m_pFText->calculateSize();
	const int xLowerBound = getX() + 5;
	int ftX = m_pFText->getX();
	if (ftX < xLowerBound || m_pFText->getRole() == Uml::tr_Seq_Message_Self)
		ftX = xLowerBound;
	else {
		const int objB_seqLineX = m_pOw[Uml::B]->getX() + m_pOw[Uml::B]->getWidth() / 2;
		const int xUpperBound = objB_seqLineX - m_pFText->getWidth() - 5;
		if (xUpperBound <= xLowerBound)
			ftX = xLowerBound;
		else if (ftX > xUpperBound)
			ftX = xUpperBound;
	}
	m_pFText->setLinePos(ftX, getY() - m_pFText->getHeight());
}

void MessageWidget::updateMessagePos(int textHeight, int& newX, int& newY) {
	newX = getX() + 5;
	int minHeight = getMinHeight();
	if (newY < minHeight)
		newY = minHeight;
	int maxHeight = getMaxHeight() - textHeight - 5;
	if (newY > maxHeight)
		newY = maxHeight;
	setX( newX - 5 );  // what good is this? coordinate remains the same
	setY( newY + textHeight );
}

void MessageWidget::setupAfterFTsetLink(FloatingText* /*ft*/) {
	setTextPosition();
}

void MessageWidget::moveEvent(QMoveEvent* /*m*/) {
	if (!m_pFText) {
		return;
	}
	if (m_pView->getSelectCount() > 2) {
		return;
	}
	setTextPosition();

	emit sigMessageMoved();
}

void MessageWidget::resizeEvent(QResizeEvent */*re*/) {
}

void MessageWidget::calculateWidget() {
	calculateDimensions();

	setVisible(true);

	setX(m_nPosX);
	setY(m_nY);
}

void MessageWidget::slotWidgetMoved(Uml::IDType id) {
	if(m_pOw[Uml::A] -> getLocalID() == id || m_pOw[Uml::B] -> getLocalID() == id) {
		m_nY = getY();
		m_nY = m_nY < getMinHeight()?getMinHeight():m_nY;
		m_nY = m_nY > getMaxHeight()?getMaxHeight():m_nY;
		calculateWidget();
	}
	if( !m_pFText )
		return;
	if( m_pView -> getSelectCount() > 1 )
		return;
	setTextPosition();
}

bool MessageWidget::contains(ObjectWidget * w) {
	if(m_pOw[Uml::A] == w || m_pOw[Uml::B] == w)
		return true;
	else
		return false;
}

void MessageWidget::slotMenuSelection(int sel) {
	if(sel == ListPopupMenu::mt_Delete) {
		// This will clean up this widget and the text widget:
		m_pView -> removeWidget(this);
	} else
		m_pFText -> slotMenuSelection(sel);
}

void MessageWidget::mouseDoubleClickEvent(QMouseEvent * /*me*/) {
	if(m_pView -> getCurrentCursor() == WorkToolBar::tbb_Arrow)
		m_pFText -> slotMenuSelection(ListPopupMenu::mt_Select_Operation);
}

bool MessageWidget::activate(IDChangeLog * Log /*= 0*/) {
	bool status = UMLWidget::activate(Log);
	if(!status)
		return false;
	if (m_pOw[Uml::A] == NULL || m_pOw[Uml::B] == NULL) {
		kdDebug() << "MessageWidget::activate: can't make message" << endl;
		return false;
	}
	if( !m_pFText ) {
		Uml::Text_Role tr = Uml::tr_Seq_Message;
		if (m_pOw[Uml::A] ==m_pOw[Uml::B])
			tr = Uml::tr_Seq_Message_Self;
		m_pFText = new FloatingText( m_pView, tr, "" );
		m_pFText->setFont(UMLWidget::getFont());
	}
	m_pFText -> setLink(this);
	m_pFText -> setText("");
	m_pFText->setActivated();
	QString messageText = m_pFText->getText();
	m_pFText->setVisible( messageText.length() > 1 );
	m_pFText -> setUMLObject( m_pOw[Uml::B] -> getUMLObject() );
	connect(m_pOw[Uml::A], SIGNAL(sigWidgetMoved(Uml::IDType)), this, SLOT(slotWidgetMoved(Uml::IDType)));
	connect(m_pOw[Uml::B], SIGNAL(sigWidgetMoved(Uml::IDType)), this, SLOT(slotWidgetMoved(Uml::IDType)));
	if ( ! UMLApp::app()->getDocument()->loading() )
	{ // calculate the dimensions only if no XMI file is loaded
		// - this functions derives the dimension properties of this widget ( as said by the
		//   function
		// - thus it calls also the UMLWidget::adjustAssocs() function - which is bad
		// Despite in case of a _NEW_ message widget, this calculation is important
		calculateDimensions();
	}

	connect(this, SIGNAL(sigMessageMoved()), m_pOw[Uml::A], SLOT(slotMessageMoved()) );
	connect(this, SIGNAL(sigMessageMoved()), m_pOw[Uml::B], SLOT(slotMessageMoved()) );
	m_pOw[Uml::A] -> messageAdded(this);
	m_pOw[Uml::B] -> messageAdded(this);

	emit sigMessageMoved();
	return status;
}

void MessageWidget::setMessageText(FloatingText *ft) {
	QString displayText = m_SequenceNumber + ": " + m_Operation;
	ft->setText(displayText);
	setTextPosition();
}

void MessageWidget::setText(FloatingText *ft, QString newText) {
	ft->setText(newText);
	UMLApp::app()->getDocument()->setModified(true);
}

void MessageWidget::setSeqNumAndOp(QString seqNum, QString op) {
	setSequenceNumber( seqNum );
	setOperation( op );
}

void MessageWidget::setSequenceNumber( QString sequenceNumber ) {
	m_SequenceNumber = sequenceNumber;
}

QString MessageWidget::getSequenceNumber() const {
	return m_SequenceNumber;
}

void MessageWidget::setOperation( QString operation ) {
	m_Operation = operation;
}

QString MessageWidget::getOperation() const {
	return m_Operation;
}

UMLClassifier *MessageWidget::getOperationOwner(FloatingText *ft) {
	UMLObject *pObject = ft->getUMLObject();
	if (pObject == NULL)
		return NULL;
	UMLClassifier *c = dynamic_cast<UMLClassifier*>(pObject);
	return c;
}

void MessageWidget::setOperationText(FloatingText *ft, QString opText) {
	setOperation(opText);
	ft->setMessageText();
}

UMLClassifier * MessageWidget::getSeqNumAndOp(FloatingText *ft, QString& seqNum,
							        QString& op) {
	seqNum = m_SequenceNumber;
	op = m_Operation;
	UMLObject *o = ft->getUMLObject();
	UMLClassifier *c = dynamic_cast<UMLClassifier*>(o);
	return c;
}

void MessageWidget::calculateDimensions() {
	if (m_sequenceMessageType == Uml::sequence_message_synchronous) {
		calculateDimensionsSynchronous();
	} else if (m_sequenceMessageType == Uml::sequence_message_asynchronous) {
		calculateDimensionsAsynchronous();
	} else {
		kdWarning() << "Unknown message type" << endl;
	}
}

void MessageWidget::calculateDimensionsSynchronous() {
	int x = 0;

	int x1 = m_pOw[Uml::A]->getX();
	int x2 = m_pOw[Uml::B]->getX();
	int w1 = m_pOw[Uml::A]->getWidth() / 2;
	int w2 =  m_pOw[Uml::B]->getWidth() / 2;
	x1 += w1;
	x2 += w2;

	int widgetWidth = 0;
	int widgetHeight = 0;
	if( m_pOw[Uml::A] == m_pOw[Uml::B] ) {
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
	adjustAssocs( this->getX(), this->getY() );//adjust assoc lines
}

void MessageWidget::calculateDimensionsAsynchronous() {
	int x = 0;

	int x1 = m_pOw[Uml::A]->getX();
	int x2 = m_pOw[Uml::B]->getX();
	int w1 = m_pOw[Uml::A] -> getWidth() / 2;
	int w2 =  m_pOw[Uml::B] -> getWidth() / 2;
	x1 += w1;
	x2 += w2;

	int widgetWidth = 0;
	int widgetHeight = 8;
	if( m_pOw[Uml::A] == m_pOw[Uml::B] ) {
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
	adjustAssocs( this->getX(), this->getY() );//adjust assoc lines
}

void MessageWidget::cleanup() {
	if (m_pOw[Uml::A]) {
		disconnect(this, SIGNAL(sigMessageMoved()), m_pOw[Uml::A], SLOT(slotMessageMoved()) );
		m_pOw[Uml::A]->messageRemoved(this);
	}
	if (m_pOw[Uml::B]) {
		disconnect(this, SIGNAL(sigMessageMoved()), m_pOw[Uml::B], SLOT(slotMessageMoved()) );
		m_pOw[Uml::B]->messageRemoved(this);
	}

	UMLWidget::cleanup();
	if (m_pFText) {
		m_pView->removeWidget(m_pFText);
		m_pFText = NULL;
	}
}

void MessageWidget::mouseMoveEvent(QMouseEvent *me) {
	int newX = 0, newY = 0;
	int moveX, moveY;
	if( m_bResizing ) {
		int heightA = (int)((ObjectWidget*)m_pOw[Uml::A]) -> getEndLineY();
		int heightB = (int)((ObjectWidget*)m_pOw[Uml::B]) -> getEndLineY();
		int height = heightA;
		if ( heightA > heightB )  {
			height = heightB;
		}
		moveY = (int)me -> y() - m_nPressOffsetY - m_nOldY;
		newY = m_nOldH + moveY;
		newY = newY < 20 ? 20 : newY;
		setSize( width(), newY );
		emit sigMessageMoved();
		return;
	}
	if( !m_bSelected )
		m_pView -> setSelected( this, me );
	m_bSelected = true;
	if( !m_bMouseDown )
		if( me -> button() != LeftButton )
			return;
	int count = m_pView -> getSelectCount();

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
		if( (getX() + moveX) < 0 )
			moveX = 0;
		if( ( getY() + moveY) < 0 )
			moveY = 0;
		if( count > 2 )
			m_pView -> moveSelected( this, moveX, 0 );
	}
	newY = getY() + moveY;
	newY = newY < 0?0:newY;
	if( count > 2 )
		newY = this -> getY();  //only change y if not selected
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
	if( m_inSelection )//used to stop a recursive call
	{
		m_inSelection = false;
		return;
	}
	UMLWidget::setSelected( _select );
	if( !m_pFText )
		return;
	if( m_bSelected && m_pFText -> getSelected() )
		return;
	if( !m_bSelected && !m_pFText -> getSelected() )
		return;

	m_inSelection = true;
	m_pView -> setSelected( m_pFText, 0 );
	m_pFText -> setSelected( m_bSelected );
}

int MessageWidget::getMinHeight() {
	if( !m_pOw[Uml::A] || !m_pOw[Uml::B] ) {
		return 0;
	}
	int heightA = m_pOw[Uml::A]->getY() + m_pOw[Uml::A]->getHeight();
	int heightB = m_pOw[Uml::B]->getY() + m_pOw[Uml::B]->getHeight();
	int height = heightA;
	if( heightA < heightB ) {
		height = heightB;
	}
	return height;
}

int MessageWidget::getMaxHeight() {
	if( !m_pOw[Uml::A] || !m_pOw[Uml::B] ) {
		return 0;
	}
	int heightA = (int)((ObjectWidget*)m_pOw[Uml::A])->getEndLineY();
	int heightB = (int)((ObjectWidget*)m_pOw[Uml::B])->getEndLineY();
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
	if ( m_sequenceMessageType == Uml::sequence_message_asynchronous &&
	     m_pOw[Uml::A] != m_pOw[Uml::B] ) {
		return;
	}
	int m = 10;
	//see if clicked on bottom right corner
	if( (m_nOldX + m_nPressOffsetX) >= (getX() + width() - m) &&
	    (m_nOldY + m_nPressOffsetY) >= (getY() + height() - m) && me->button() == LeftButton) {
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

void MessageWidget::setWidget(ObjectWidget * ow, Uml::Role_Type role) {
	m_pOw[role] = ow;
}

ObjectWidget* MessageWidget::getWidget(Uml::Role_Type role) {
	return m_pOw[role];
}

void MessageWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement messageElement = qDoc.createElement( "messagewidget" );
	UMLWidget::saveToXMI( qDoc, messageElement );
	messageElement.setAttribute( "textid", m_pFText->getID() );
	messageElement.setAttribute( "widgetaid", m_pOw[Uml::A]->getLocalID() );
	messageElement.setAttribute( "widgetbid", m_pOw[Uml::B]->getLocalID() );
	messageElement.setAttribute( "operation", m_Operation );
	messageElement.setAttribute( "seqnum", m_SequenceNumber );
	messageElement.setAttribute( "sequencemessagetype", m_sequenceMessageType );

	// save the corresponding message text
	if( m_pFText )
		m_pFText -> saveToXMI( qDoc, messageElement );

	qElement.appendChild( messageElement );
}

bool MessageWidget::loadFromXMI(QDomElement& qElement) {
	if ( !UMLWidget::loadFromXMI(qElement) ) {
		return false;
	}
	QString textid = qElement.attribute( "textid", "-1" );
	QString widgetaid = qElement.attribute( "widgetaid", "-1" );
	QString widgetbid = qElement.attribute( "widgetbid", "-1" );
	m_Operation = qElement.attribute( "operation", "" );
	m_SequenceNumber = qElement.attribute( "seqnum", "" );
	QString sequenceMessageType = qElement.attribute( "sequencemessagetype", "1001" );
	m_sequenceMessageType = (Uml::Sequence_Message_Type)sequenceMessageType.toInt();

	Uml::IDType aId = STR2ID(widgetaid);
	Uml::IDType bId = STR2ID(widgetbid);

	UMLWidget *pWA = m_pView -> findWidget( aId );
	if (pWA == NULL) {
		kdDebug() << "MessageWidget::loadFromXMI: role A object " << aId
			  << " not found" << endl;
		return false;
	}
	UMLWidget *pWB = m_pView -> findWidget( bId );
	if (pWB == NULL) {
		kdDebug() << "MessageWidget::loadFromXMI: role B object " << bId
			  << " not found" << endl;
		return false;
	}
	m_pOw[Uml::A] = dynamic_cast<ObjectWidget*>(pWA);
	if (m_pOw[Uml::A] == NULL) {
		kdDebug() << "MessageWidget::loadFromXMI: role A widget " << aId
			  << " is not an ObjectWidget" << endl;
		return false;
	}
	m_pOw[Uml::B] = dynamic_cast<ObjectWidget*>(pWB);
	if (m_pOw[Uml::B] == NULL) {
		kdDebug() << "MessageWidget::loadFromXMI: role B widget " << bId
			  << " is not an ObjectWidget" << endl;
		return false;
	}

	Uml::IDType textId = STR2ID(textid);
	if (textId != Uml::id_None) {
		UMLWidget *flotext = m_pView -> findWidget( textId );
		if (flotext != NULL) {
			// This only happens when loading files produced by
			// umbrello-1.3-beta2.
			m_pFText = static_cast<FloatingText*>(flotext);
			m_pFText->setLink(this);
			return true;
		}
	} else {
		// no textid stored -> get unique new one
		textId = UMLApp::app()->getDocument()->getUniqueID();
	}

	Uml::Text_Role tr = Uml::tr_Seq_Message;
	if (m_pOw[Uml::A] == m_pOw[Uml::B])
		tr = Uml::tr_Seq_Message_Self;
	m_pFText = new FloatingText( m_pView, tr, m_Operation, textId );
	bool textLoadSuccess = false;

	//now load child elements
	QDomNode node = qElement.firstChild();
	QDomElement element = node.toElement();
	while( !element.isNull() ) {
		QString tag = element.tagName();
		if (tag == "floatingtext"  ) {
			if( ! m_pFText->loadFromXMI(element) ) {
				// Most likely cause: The FloatingText is empty.
				node = element.nextSibling();
				element = node.toElement();
				continue;
			}
			else {
				textLoadSuccess = true;
				break; // no more to load
			}
		}
	}

	if ( ! textLoadSuccess ) {
		// set FloatingText from tag m_Operation
		// m_pFText->setText(m_Operation);
		m_pFText->setFont(UMLWidget::getFont());
		m_pFText->setActivated();
		m_pFText->setVisible( m_Operation.length() > 1 );
		m_pFText -> setUMLObject( m_pOw[Uml::B] -> getUMLObject() );
	}

	// always need this
	m_pFText->setLink(this);

	return true;
}

#include "messagewidget.moc"
