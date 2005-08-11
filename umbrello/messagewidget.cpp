/*
 *  copyright (C) 2002-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

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
#include "operation.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "listpopupmenu.h"

MessageWidget::MessageWidget(UMLView * view, ObjectWidget* a, ObjectWidget* b,
                             int y, Uml::Sequence_Message_Type sequenceMessageType,
                             Uml::IDType id /* = Uml::id_None */)
        : UMLWidget(view, id) {
    init();
    m_pOw[Uml::A] = a;
    m_pOw[Uml::B] = b;
    m_nY = y;
    m_sequenceMessageType = sequenceMessageType;
    if (m_sequenceMessageType == Uml::sequence_message_creation) {
        y -= m_pOw[Uml::B]->getHeight() / 2;
        m_pOw[Uml::B]->setY(y);
    }

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
    UMLWidget::setPen(p);
    if (m_sequenceMessageType == Uml::sequence_message_synchronous) {
        drawSynchronous(p, offsetX, offsetY);
    } else if (m_sequenceMessageType == Uml::sequence_message_asynchronous) {
        drawAsynchronous(p, offsetX, offsetY);
    } else if (m_sequenceMessageType == Uml::sequence_message_creation) {
        drawCreation(p, offsetX, offsetY);
    } else {
        kdWarning() << "Unknown message type" << endl;
    }
}

void MessageWidget::drawSolidArrowhead(QPainter& p, int x, int y, Qt::ArrowType direction) {
    int arrowheadExtentX = 4;
    if (direction == Qt::RightArrow) {
        arrowheadExtentX = -arrowheadExtentX;
    }
    QPointArray points;
    points.putPoints(0, 3, x, y, x + arrowheadExtentX, y - 3, x + arrowheadExtentX, y + 3);
    p.setBrush( QBrush(p.pen().color()) );
    p.drawPolygon(points);
}

void MessageWidget::drawArrow(QPainter& p, int x, int y, int w,
                              Qt::ArrowType direction, bool useDottedLine /* = false */) {
    int arrowheadStartX = x;
    int arrowheadExtentX = 4;
    if (direction == Qt::RightArrow) {
        arrowheadStartX += w;
        arrowheadExtentX = -arrowheadExtentX;
    }
    // draw upper half of arrowhead
    p.drawLine(arrowheadStartX, y, arrowheadStartX + arrowheadExtentX, y - 3);
    // draw lower half of arrowhead
    p.drawLine(arrowheadStartX, y, arrowheadStartX + arrowheadExtentX, y + 3);
    // draw arrow line
    if (useDottedLine) {
        QPen pen = p.pen();
        pen.setStyle(Qt::DotLine);
        p.setPen(pen);
    }
    p.drawLine(x, y, x + w, y);
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
        offsetX += 17;
        w -= 17;
        offsetY += 3;
        const int lowerLineY = offsetY + h - 6;
        // draw upper line segment (leaving the life line)
        p.drawLine(offsetX, offsetY, offsetX + w, offsetY);
        // draw line segment parallel to (and at the right of) the life line
        p.drawLine(offsetX + w, offsetY, offsetX + w, lowerLineY);
        // draw lower line segment (back to the life line)
        drawArrow(p, offsetX, lowerLineY, w, Qt::LeftArrow);
        offsetX -= 17;
        offsetY -= 3;
    } else if(x1 < x2) {
        if (messageOverlaps)  {
            offsetX += 8;
            w -= 8;
        }
        QPen pen = p.pen();
        int startX = offsetX + w - 16;
        p.fillRect(startX, offsetY, 17, h,  QBrush(white));		//box
        p.drawRect(startX, offsetY, 17, h);				//box
        p.drawLine(offsetX, offsetY + 4, startX, offsetY + 4);		//arrow line
        drawSolidArrowhead(p, startX - 1, offsetY + 4, Qt::RightArrow);
        drawArrow(p, offsetX, offsetY + h - 3, w - 16, Qt::LeftArrow, true); // return arrow
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
        drawSolidArrowhead(p, offsetX + 17, offsetY + 4, Qt::LeftArrow);
        drawArrow(p, offsetX + 18, offsetY + h - 3, w - 18, Qt::RightArrow, true); // return arrow
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
        const int lowerLineY = offsetY + h - 3;
        // draw upper line segment (leaving the life line)
        p.drawLine(offsetX, offsetY, offsetX + w, offsetY);
        // draw line segment parallel to (and at the right of) the life line
        p.drawLine(offsetX + w, offsetY, offsetX + w, lowerLineY);
        // draw lower line segment (back to the life line)
        drawArrow(p, offsetX, lowerLineY, w, Qt::LeftArrow);
        if (messageOverlapsA)  {
            offsetX -= 7; //reset for drawSelected()
        }
    } else if(x1 < x2) {
        if (messageOverlapsA)  {
            offsetX += 7;
            w -= 7;
        }
        drawArrow(p, offsetX, offsetY + 4, w, Qt::RightArrow);
        if (messageOverlapsA)  {
            offsetX -= 7;
        }
    } else	{
        if (messageOverlapsA)  {
            w -= 7;
        }
        drawArrow(p, offsetX, offsetY + 4, w, Qt::LeftArrow);
    }

    if (m_bSelected && m_pOw[Uml::A] == m_pOw[Uml::B]) {
        drawSelected(&p, offsetX, offsetY, true);
    } else if (m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

void MessageWidget::drawCreation(QPainter& p, int offsetX, int offsetY) {
    int x1 = m_pOw[Uml::A]->getX();
    int x2 = m_pOw[Uml::B]->getX();
    int w = getWidth() - 1;
    //int h = getHeight() - 1;
    bool messageOverlapsA = m_pOw[Uml::A] -> messageOverlap( getY(), this );
    //bool messageOverlapsB = m_pOw[Uml::B] -> messageOverlap( getY(), this );

    const int lineY = offsetY + 4;
    if (x1 < x2) {
        if (messageOverlapsA)  {
            offsetX += 7;
            w -= 7;
        }
        drawArrow(p, offsetX, lineY, w, Qt::RightArrow);
        if (messageOverlapsA)  {
            offsetX -= 7;
        }
    } else	{
        if (messageOverlapsA)  {
            w -= 7;
        }
        drawArrow(p, offsetX, lineY, w, Qt::LeftArrow);
    }

    if (m_bSelected && m_pOw[Uml::A] == m_pOw[Uml::B]) {
        drawSelected(&p, offsetX, offsetY, true);
    } else if (m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

bool MessageWidget::onWidget(const QPoint & p) {
    if (m_sequenceMessageType == Uml::sequence_message_asynchronous) {
        return UMLWidget::onWidget(p);
    }
    // Synchronous message:
    // Consists of top arrow (call) and bottom arrow (return.)
    if (p.x() < getX() || p.x() > getX() + getWidth())
        return false;
    const int tolerance = 5;  // pixels
    const int pY = p.y();
    const int topArrowY = getY() + 3;
    const int bottomArrowY = getY() + getHeight() - 3;
    if (pY < topArrowY - tolerance || pY > bottomArrowY + tolerance)
        return false;
    if (getHeight() <= 2 * tolerance)
        return true;
    if (pY > topArrowY + tolerance && pY < bottomArrowY - tolerance)
        return false;
    return true;
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
    int ftX = constrainX(m_pFText->getX(), m_pFText->getWidth(), m_pFText->getRole());
    int ftY = getY() - m_pFText->getHeight();
    m_pFText->setX( ftX );
    m_pFText->setY( ftY );
}

int MessageWidget::constrainX(int textX, int textWidth, Uml::Text_Role tr) {
    int result = textX;
    const int minTextX = getX() + 5;
    if (textX < minTextX || tr == Uml::tr_Seq_Message_Self) {
        result = minTextX;
    } else {
        ObjectWidget *objectAtRight = NULL;
        if (m_pOw[Uml::B]->getX() > m_pOw[Uml::A]->getX())
            objectAtRight = m_pOw[Uml::B];
        else
            objectAtRight = m_pOw[Uml::A];
        const int objRight_seqLineX = objectAtRight->getX() + objectAtRight->getWidth() / 2;
        const int maxTextX = objRight_seqLineX - textWidth - 5;
        if (maxTextX <= minTextX)
            result = minTextX;
        else if (textX > maxTextX)
            result = maxTextX;
    }
    return result;
}

void MessageWidget::constrainTextPos(int &textX, int &textY, int textWidth, int textHeight,
                                     Uml::Text_Role tr) {
    textX = constrainX(textX, textWidth, tr);
    // Constrain Y.
    const int minTextY = getMinHeight();
    const int maxTextY = getMaxHeight() - textHeight - 5;
    if (textY < minTextY)
        textY = minTextY;
    else if (textY > maxTextY)
        textY = maxTextY;
    setY( textY + textHeight );   // NB: side effect
}

void MessageWidget::setLinkAndTextPos() {
    if (m_pFText == NULL)
        return;
    m_pFText->setLink(this);
    setTextPosition();
}

void MessageWidget::moveEvent(QMoveEvent* /*m*/) {
    //kdDebug() << "MessageWidget::moveEvent: m_pFText is " << m_pFText << endl;
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
    setMessageText(m_pFText);
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
    } else {
        if (m_pFText == NULL) {
            Uml::Text_Role tr = Uml::tr_Seq_Message;
            if (m_pOw[Uml::A] == m_pOw[Uml::B])
                tr = Uml::tr_Seq_Message_Self;
            m_pFText = new FloatingText( m_pView, tr );
            m_pFText->setFont(UMLWidget::getFont());
            setLinkAndTextPos();
            m_pView->getWidgetList().append(m_pFText);
        }
        m_pFText -> slotMenuSelection(sel);
    }
}

void MessageWidget::mouseDoubleClickEvent(QMouseEvent * /*me*/) {
    if (m_pView->getCurrentCursor() == WorkToolBar::tbb_Arrow &&
            m_pFText != NULL)
        m_pFText -> slotMenuSelection(ListPopupMenu::mt_Select_Operation);
}

bool MessageWidget::activate(IDChangeLog * Log /*= 0*/) {
    m_pView->resetPastePoint();
    bool status = UMLWidget::activate(Log);
    if(!status)
        return false;
    if (m_pOw[Uml::A] == NULL || m_pOw[Uml::B] == NULL) {
        kdDebug() << "MessageWidget::activate: can't make message" << endl;
        return false;
    }
    if( !m_pFText ) {
        Uml::Text_Role tr = Uml::tr_Seq_Message;
        if (m_pOw[Uml::A] == m_pOw[Uml::B])
            tr = Uml::tr_Seq_Message_Self;
        m_pFText = new FloatingText( m_pView, tr, "" );
        m_pFText->setFont(UMLWidget::getFont());
    }
    setLinkAndTextPos();
    m_pFText -> setText("");
    m_pFText->setActivated();
    QString messageText = m_pFText->getText();
    m_pFText->setVisible( messageText.length() > 1 );

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
    if (ft == NULL)
        return;
    QString displayText = m_SequenceNumber + ": " + getOperationText(m_pView);
    ft->setText(displayText);
    setTextPosition();
}

void MessageWidget::setText(FloatingText *ft, const QString &newText) {
    ft->setText(newText);
    UMLApp::app()->getDocument()->setModified(true);
}

void MessageWidget::setSeqNumAndOp(const QString &seqNum, const QString &op) {
    setSequenceNumber( seqNum );
    m_CustomOp = op;   ///FIXME m_pOperation
}

void MessageWidget::setSequenceNumber( const QString &sequenceNumber ) {
    m_SequenceNumber = sequenceNumber;
}

QString MessageWidget::getSequenceNumber() const {
    return m_SequenceNumber;
}

void MessageWidget::lwSetFont (QFont font) {
    UMLWidget::setFont( font );
}

UMLClassifier *MessageWidget::getOperationOwner() {
    UMLObject *pObject = m_pOw[Uml::B]->getUMLObject();
    if (pObject == NULL)
        return NULL;
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(pObject);
    return c;
}

UMLOperation *MessageWidget::getOperation() {
    return static_cast<UMLOperation*>(m_pObject);
}

void MessageWidget::setOperation(UMLOperation *op) {
    m_pObject = op;
}

QString MessageWidget::getCustomOpText() {
    return m_CustomOp;
}

void MessageWidget::setCustomOpText(const QString &opText) {
    m_CustomOp = opText;
    m_pFText->setMessageText();
}

UMLClassifier * MessageWidget::getSeqNumAndOp(QString& seqNum, QString& op) {
    seqNum = m_SequenceNumber;
    UMLOperation *pOperation = getOperation();
    if (pOperation != NULL) {
        Uml::Signature_Type sigType;
        if (m_pView->getShowOpSig())
            sigType = Uml::st_SigNoScope;
        else
            sigType = Uml::st_NoSigNoScope;
        op = pOperation->toString(sigType);
    } else {
        op = m_CustomOp;
    }
    UMLObject *o = m_pOw[Uml::B]->getUMLObject();
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(o);
    return c;
}

void MessageWidget::calculateDimensions() {
    if (m_sequenceMessageType == Uml::sequence_message_synchronous) {
        calculateDimensionsSynchronous();
    } else if (m_sequenceMessageType == Uml::sequence_message_asynchronous) {
        calculateDimensionsAsynchronous();
    } else if (m_sequenceMessageType == Uml::sequence_message_creation) {
        calculateDimensionsCreation();
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
    adjustAssocs( getX(), getY() );//adjust assoc lines
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
    adjustAssocs( getX(), getY() );//adjust assoc lines
}

void MessageWidget::calculateDimensionsCreation() {
    int x = 0;

    int x1 = m_pOw[Uml::A]->getX();
    int x2 = m_pOw[Uml::B]->getX();
    int w1 = m_pOw[Uml::A] -> getWidth() / 2;
    int w2 =  m_pOw[Uml::B] -> getWidth();
    x1 += w1;
    if (x1 > x2)
        x2 += w2;

    int widgetWidth = 0;
    int widgetHeight = 8;
    if ( x1 < x2 ) {
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
    adjustAssocs( getX(), getY() );//adjust assoc lines
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
        newY = getY();  //only change y if not selected
    newY = newY < getMinHeight() ? getMinHeight() : newY;
    newY = newY > getMaxHeight() ? getMaxHeight() : newY;

    if (m_nOldX != newX || m_nOldY != newY) {
        m_bMoved = true;
    }
    m_nOldY = newY;
    setY( newY );
    adjustAssocs(newX, newY);
    if (m_sequenceMessageType == Uml::sequence_message_creation) {
        const int objWidgetHalfHeight = m_pOw[Uml::B]->getHeight() / 2;
        m_pOw[Uml::B]->UMLWidget::setY( newY - objWidgetHalfHeight );
    }
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
    if (!m_pOw[Uml::A] || !m_pOw[Uml::B] ||
            m_sequenceMessageType == Uml::sequence_message_creation) {
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
    messageElement.setAttribute( "widgetaid", ID2STR(m_pOw[Uml::A]->getLocalID()) );
    messageElement.setAttribute( "widgetbid", ID2STR(m_pOw[Uml::B]->getLocalID()) );
    UMLOperation *pOperation = getOperation();
    if (pOperation)
        messageElement.setAttribute( "operation", ID2STR(pOperation->getID()) );
    else
        messageElement.setAttribute( "operation", m_CustomOp );
    messageElement.setAttribute( "seqnum", m_SequenceNumber );
    messageElement.setAttribute( "sequencemessagetype", m_sequenceMessageType );

    // save the corresponding message text
    if (m_pFText && !m_pFText->getText().isEmpty()) {
        messageElement.setAttribute( "textid", ID2STR(m_pFText->getID()) );
        m_pFText -> saveToXMI( qDoc, messageElement );
    }

    qElement.appendChild( messageElement );
}

bool MessageWidget::loadFromXMI(QDomElement& qElement) {
    if ( !UMLWidget::loadFromXMI(qElement) ) {
        return false;
    }
    QString textid = qElement.attribute( "textid", "-1" );
    QString widgetaid = qElement.attribute( "widgetaid", "-1" );
    QString widgetbid = qElement.attribute( "widgetbid", "-1" );
    m_CustomOp = qElement.attribute( "operation", "" );
    m_SequenceNumber = qElement.attribute( "seqnum", "" );
    QString sequenceMessageType = qElement.attribute( "sequencemessagetype", "1001" );
    m_sequenceMessageType = (Uml::Sequence_Message_Type)sequenceMessageType.toInt();

    Uml::IDType aId = STR2ID(widgetaid);
    Uml::IDType bId = STR2ID(widgetbid);

    UMLWidget *pWA = m_pView -> findWidget( aId );
    if (pWA == NULL) {
        kdDebug() << "MessageWidget::loadFromXMI: role A object "
        << ID2STR(aId) << " not found" << endl;
        return false;
    }
    UMLWidget *pWB = m_pView -> findWidget( bId );
    if (pWB == NULL) {
        kdDebug() << "MessageWidget::loadFromXMI: role B object "
        << ID2STR(bId) << " not found" << endl;
        return false;
    }
    m_pOw[Uml::A] = dynamic_cast<ObjectWidget*>(pWA);
    if (m_pOw[Uml::A] == NULL) {
        kdDebug() << "MessageWidget::loadFromXMI: role A widget "
        << ID2STR(aId) << " is not an ObjectWidget" << endl;
        return false;
    }
    m_pOw[Uml::B] = dynamic_cast<ObjectWidget*>(pWB);
    if (m_pOw[Uml::B] == NULL) {
        kdDebug() << "MessageWidget::loadFromXMI: role B widget "
        << ID2STR(bId) << " is not an ObjectWidget" << endl;
        return false;
    }

    UMLClassifier *c = dynamic_cast<UMLClassifier*>( pWB->getUMLObject() );
    if (c) {
        Uml::IDType opId = STR2ID(m_CustomOp);
        UMLOperation *op = dynamic_cast<UMLOperation*>( c->findChildObject(opId, true) );
        if (op) {
            // If the UMLOperation is set, m_CustomOp isn't used anyway.
            // Just setting it empty for the sake of sanity.
            m_CustomOp = QString::null;
        } else {
            // Previous umbrello versions saved the operation text
            // instead of the xmi.id of the operation.
            // For backward compatibility, attempt to determine the
            // m_pOperation from the operation text:
            Model_Utils::OpDescriptor od;
            Model_Utils::Parse_Status st = Model_Utils::parseOperation(m_CustomOp, od, c);
            if (st == Model_Utils::PS_OK) {
                bool isExistingOp = false;
                UMLObject *o = c->createOperation(od.m_name, &isExistingOp, &od.m_args);
                op = static_cast<UMLOperation*>(o);
                if (od.m_pReturnType) {
                    op->setType(od.m_pReturnType);
                }
                m_CustomOp = QString::null;
            }
        }
        setOperation(op);
    }

    Uml::IDType textId = STR2ID(textid);
    if (textId != Uml::id_None) {
        UMLWidget *flotext = m_pView -> findWidget( textId );
        if (flotext != NULL) {
            // This only happens when loading files produced by
            // umbrello-1.3-beta2.
            m_pFText = static_cast<FloatingText*>(flotext);
            setLinkAndTextPos();
            return true;
        }
    } else {
        // no textid stored -> get unique new one
        textId = UMLApp::app()->getDocument()->getUniqueID();
    }

    Uml::Text_Role tr = Uml::tr_Seq_Message;
    if (m_pOw[Uml::A] == m_pOw[Uml::B])
        tr = Uml::tr_Seq_Message_Self;

    //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if ( !element.isNull() ) {
        QString tag = element.tagName();
        if (tag == "floatingtext") {
            m_pFText = new FloatingText( m_pView, tr, getOperationText(m_pView), textId );
            if( ! m_pFText->loadFromXMI(element) ) {
                // Most likely cause: The FloatingText is empty.
                delete m_pFText;
                m_pFText = NULL;
            }
        } else {
            kdError() << "MessageWidget::loadFromXMI: unknown tag "
            << tag << endl;
        }
    }

    // always need this
    setLinkAndTextPos();

    return true;
}

#include "messagewidget.moc"
