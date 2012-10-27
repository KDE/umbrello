/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// onw header
#include "messagewidget.h"

//app includes
#include "classifier.h"
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "listpopupmenu.h"
#include "objectwidget.h"
#include "operation.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "uniqueid.h"
#include "widget_utils.h"

//qt includes
#include <QTimer>

//kde includes
#include <klocale.h>


const qreal MessageWidget::SynchronousBoxWidth = 17;
const qreal MessageWidget::FoundLostCircleRadius = 10;
const qreal MessageWidget::ArrowWidth = 8;
const qreal MessageWidget::ArrowHeight = 10;
const qreal MessageWidget::SelfLoopBoxWidth = 50;

/**
 * Constructs a MessageWidget.
 *
 * @param view                The parent to this class.
 * @param a                   The role A widget for this message.
 * @param b                   The role B widget for this message.
 * @param sequenceMessageType Whether synchronous or asynchronous
 * @param id                  A unique id used for deleting this object cleanly.
 *                            The default (-1) will prompt generation of a new ID.
 */
MessageWidget::MessageWidget(ObjectWidget* a, ObjectWidget* b,
                             Uml::Sequence_Message_Type sequenceMessageType,
                             Uml::IDType id /* = Uml::id_None */)
  : UMLWidget(WidgetBase::wt_Message, id)
{
    init();
    m_objectWidgets[Uml::A] = a;
    m_objectWidgets[Uml::B] = b;

    m_sequenceMessageType = sequenceMessageType;
}

/**
 * Constructs a MessageWidget.
 *
 * @param sequenceMessageType The Uml::Sequence_Message_Type of this message widget
 * @param id                  The ID to assign (-1 will prompt a new ID.)
 */
MessageWidget::MessageWidget(Uml::Sequence_Message_Type seqMsgType,
                             Uml::IDType id)
  : UMLWidget(WidgetBase::wt_Message, id)
{
    init();
    m_objectWidgets[Uml::A] = new ObjectWidget(0);
    m_objectWidgets[Uml::B] = new ObjectWidget(0);
    m_sequenceMessageType = seqMsgType;
}

/**
 * Constructs a Lost or Found MessageWidget.
 *
 * @param view         The parent to this class.
 * @param a            The role A widget for this message.
 * @param clickedPos   The position clicked by the user
 *                     (static position for found/lost message type)
 *
 * @param seqMsgType   Whether lost or found
 * @param id           The ID to assign (-1 will prompt a new ID.)
 */
MessageWidget::MessageWidget(ObjectWidget* a, const QPointF& clickedPos,
                             Uml::Sequence_Message_Type seqMsgType,
                             Uml::IDType id)
  : UMLWidget(WidgetBase::wt_Message, id)
{
    init();
    setID(id);
    m_objectWidgets[Uml::A] = a;
    m_objectWidgets[Uml::B] = a;

    m_sequenceMessageType = seqMsgType;
    m_clickedPoint = clickedPos;
}

/**
 * A private method for common initialization used in all constructors.
 */
void MessageWidget::init()
{
//:DEPRECATED:    setIgnoreSnapToGrid(true);
//:DEPRECATED:    setIgnoreSnapComponentSizeToGrid(true);
    m_objectWidgets[Uml::A] = 0;
    m_objectWidgets[Uml::B] = 0;

    Uml::TextRole tr = Uml::TextRole::Seq_Message;
    m_floatingTextWidget = new FloatingTextWidget(tr);
    m_floatingTextWidget->setFont(font());
    m_floatingTextWidget->setLink(this);
    m_floatingTextWidget->hide(); // Hide initially until a text is set

    m_isMsgSceneSetBefore = false;
    // Some delayed initialization is done when a new UMLScene is set
    // on this widget for the first time. (see slotDelayedInit)
}

/**
 * Destructor. Inform Object widgets about destruction.
 */
MessageWidget::~MessageWidget()
{
    if (m_objectWidgets[Uml::A]) {
        m_objectWidgets[Uml::A]->messageRemoved(this);
    }

    if (!isSelf() && m_objectWidgets[Uml::B]) {
        m_objectWidgets[Uml::B]->messageRemoved(this);
    }
}

/**
 * Implements operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
void MessageWidget::lwSetFont(QFont font)
{
    UMLWidget::setFont(font);
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @todo Move to LinkWidget.
 */
UMLClassifier *MessageWidget::operationOwner()
{
    UMLObject *pObject = m_objectWidgets[Uml::B]->umlObject();
    if (pObject == NULL)
        return NULL;
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(pObject);
    return c;
}

/**
 * Implements operation from LinkWidget.
 * Motivated by FloatingTextWidget.
 */
UMLOperation *MessageWidget::operation()
{
    if (umlObject()) {
        if (umlObject()->baseType() == UMLObject::ot_Operation) {
            return static_cast<UMLOperation*>(umlObject());
        }
        uDebug() << "umlObject() is not null and is not an operation";
    }

    return 0;
}

/**
 * Implements operation from LinkWidget.
 * Motivated by FloatingTextWidget.
 */
void MessageWidget::setOperation(UMLOperation *op)
{
    if (umlObject() && m_floatingTextWidget)
        disconnect(umlObject(), SIGNAL(modified()), m_floatingTextWidget, SLOT(setMessageText()));
    setUMLObject(op);
    if (umlObject() && m_floatingTextWidget)
        connect(umlObject(), SIGNAL(modified()), m_floatingTextWidget, SLOT(setMessageText()));
    m_floatingTextWidget->setMessageText();
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
QString MessageWidget::customOpText()
{
    return m_customOperation;
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
void MessageWidget::setCustomOpText(const QString &opText)
{
    m_customOperation = opText;
    m_floatingTextWidget->setMessageText();
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param ft   The text widget which to update.
 */
void MessageWidget::setMessageText(FloatingTextWidget *ft)
{
    if (!ft)
        return;
    QString displayText = m_sequenceNumber + ": " + LinkWidget::operationText(umlScene());
    ft->setText(displayText);
    show();
    setTextPosition();
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param ft        The text widget which to update.
 * @param newText   The new text to set.
 */
void MessageWidget::setText(FloatingTextWidget *ft, const QString &newText)
{
    ft->setText(newText);
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param seqNum   Return this MessageWidget's sequence number string.
 * @param op       Return this MessageWidget's operation string.
 */
UMLClassifier * MessageWidget::seqNumAndOp(QString& seqNum, QString& op)
{
    seqNum = m_sequenceNumber;
    UMLOperation *pOperation = operation();
    if (pOperation) {
        op = pOperation->toString(Uml::SignatureType::SigNoVis);
    } else {
        op = m_customOperation;
    }
    UMLObject *o = m_objectWidgets[Uml::B]->umlObject();
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(o);
    return c;
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param seqNum   The new sequence number string to set.
 * @param op       The new operation string to set.
 */
void MessageWidget::setSeqNumAndOp(const QString &seqNum, const QString &op)
{
    setSequenceNumber(seqNum);
    m_customOperation = op;   ///FIXME m_pOperation
}

/**
 * Constrains the FloatingTextWidget X and Y values supplied.
 * Overrides operation from LinkWidget.
 *
 * @param textX        Candidate X value (may be modified by the constraint.)
 * @param textY        Candidate Y value (may be modified by the constraint.)
 * @param textWidth    Width of the text.
 * @param textHeight   Height of the text.
 * @param tr           Uml::TextRole of the text.
 */
void MessageWidget::constrainTextPos(qreal &textX, qreal &textY, qreal textWidth, qreal textHeight,
                                     Uml::TextRole tr)
{
    Q_UNUSED(textX); Q_UNUSED(textY);  //:TODO:
    Q_UNUSED(textWidth); Q_UNUSED(textHeight); Q_UNUSED(tr);
    // textX = constrainedX(textX, textWidth, tr);
    // // Constrain Y.
    // const qreal minTextY = getMinY();
    // const qreal maxTextY = getMaxY() - textHeight - 5;
    // if (textY < minTextY)
    //     textY = minTextY;
    // else if (textY > maxTextY)
    //     textY = maxTextY;
//     setY( textY + textHeight );   // NB: side effect
}

// End of link widget interface methods

/**
 * Getter for property m_sequenceNumber.
 * @return sequence number as string
 */
QString MessageWidget::sequenceNumber() const
{
    return m_sequenceNumber;
}

void MessageWidget::setSequenceNumber(const QString &sequenceNumber)
{
    m_sequenceNumber = sequenceNumber;
    // Update floating widget text.
    if (m_floatingTextWidget) {
        setMessageText(m_floatingTextWidget);
    }
}

/**
 * Sets the related widget on the given side.
 *
 * @param ow        The ObjectWidget we are related to.
 * @param role      The Uml::Role_Type to be set for the ObjectWidget
 */
void MessageWidget::setObjectWidget(ObjectWidget * ow, Uml::Role_Type role)
{
    m_objectWidgets[role] = ow;
    // Simulate an Object move to calculate the widget size.
    handleObjectMove(ow);
}

/**
 * Sets the text widget it is related to.
 *
 * @param f The text widget we are related to.
 */
void MessageWidget::setFloatingTextWidget(FloatingTextWidget * f)
{
    m_floatingTextWidget = f;
    setTextPosition();
}

/**
 * Check to see if the given ObjectWidget is involved in the
 * message.
 *
 * @param   w    The ObjectWidget to check for.
 * @retval  true If the ObjectWidget is contained.
 */
bool MessageWidget::hasObjectWidget(ObjectWidget * w) const
{
    return (m_objectWidgets[Uml::A] == w ||
            m_objectWidgets[Uml::B] == w);
}

/**
 * This method determines whether the message is for "Self" for
 * an ObjectWidget.
 *
 * @retval True If both ObjectWidgets for this widget exists and
 *              are same.
 */
bool MessageWidget::isSelf() const
{
    return (m_objectWidgets[Uml::A] && m_objectWidgets[Uml::B] &&
            m_objectWidgets[Uml::A] == m_objectWidgets[Uml::B]);
}

/**
 * This method is called when an ObjectWidget associated with this
 * widget moves.
 *
 * It sets the appropriate position and size for this MessageWidget
 * based on the position of the ObjectWidgets.
 */
void MessageWidget::handleObjectMove(ObjectWidget *wid)
{
    if (!hasObjectWidget(wid)) {
        uError() << "ObjectWidget " << (void*)wid
                 << " doesn't belong to this MessageWidget";
        return;
    }

    qreal y = qMax(pos().y(), minY());
    y = qMin(y, maxY());
    qreal roleAX = 0;
    if (m_objectWidgets[Uml::A]) {
        roleAX = m_objectWidgets[Uml::A]->sequentialLineX();
    }

    qreal roleBX = 0;
    if (m_objectWidgets[Uml::B]) {
        roleBX = m_objectWidgets[Uml::B]->sequentialLineX();
    }

    if (m_sequenceMessageType == Uml::sequence_message_synchronous) {
        if (isSelf()) {
            setPos(roleAX - .5 * SynchronousBoxWidth, y);
            setWidth(SynchronousBoxWidth + SelfLoopBoxWidth);
        }
        else {
            if (roleAX <= roleBX) {
                setPos(roleAX, y);
                setWidth(roleBX - roleAX + .5 * SynchronousBoxWidth);
            }
            else {
                qreal x = roleBX -.5 * SynchronousBoxWidth;
                setPos(x, y);
                setWidth(roleAX - x);
            }
        }
    }

    else if (m_sequenceMessageType == Uml::sequence_message_asynchronous) {
        if (isSelf()) {
            setPos(roleAX, y);
            setWidth(SelfLoopBoxWidth);
        }

        else {
            setPos(qMin(roleAX, roleBX), y);
            setWidth(qAbs(roleAX - roleBX));
        }
    }

    else if (m_sequenceMessageType == Uml::sequence_message_creation) {
        setPos(pos().x(), y);
        // creation code
    }

    else { //lost || found
        setPos(qMin(m_clickedPoint.x(), roleAX), y);
        setWidth(qAbs(m_clickedPoint.x() - roleAX));
    }

    m_clickedPoint.ry() = qMax(m_clickedPoint.y(), minY());
    m_clickedPoint.ry() = qMin(m_clickedPoint.y(), maxY());
}

/**
 * Reimplemented from UMLWidget::paint to draw the
 * MessageWidget based on the messagetype.
 */
void MessageWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(lineColor(), lineWidth()));

    switch (m_sequenceMessageType) {
    case Uml::sequence_message_asynchronous:
        drawAsynchronous(painter);
    break;

    case Uml::sequence_message_synchronous:
        drawSynchronous(painter);
    break;

    case Uml::sequence_message_creation:
        drawCreation(painter);
    break;

    case Uml::sequence_message_lost:
        drawLost(painter);
    break;

    case Uml::sequence_message_found:
        drawFound(painter);
    break;
    }
}

/**
 * Returns the minimum height this widget should be set at on a
 * sequence diagrams.  Takes into account the widget positions it is
 * related to.
 */
qreal MessageWidget::minY() const
{
    if (!m_objectWidgets[Uml::A] || !m_objectWidgets[Uml::B]) {
        return 0;
    }
    if (m_sequenceMessageType == Uml::sequence_message_creation) {
        return m_objectWidgets[Uml::A]->y() + m_objectWidgets[Uml::A]->height();
    }

    qreal heightA = m_objectWidgets[Uml::A]->y() + m_objectWidgets[Uml::A]->height();
    qreal heightB = m_objectWidgets[Uml::B]->y() + m_objectWidgets[Uml::B]->height();

    return qMax(heightA, heightB);
}

/**
 * Returns the maximum height this widget should be set at on a
 * sequence diagrams.  Takes into account the widget positions it is
 * related to.
 */
qreal MessageWidget::maxY() const
{
    if( !m_objectWidgets[Uml::A] || !m_objectWidgets[Uml::B] ) {
        return 0;
    }
    qreal heightA = m_objectWidgets[Uml::A]->lineEndY();
    qreal heightB = m_objectWidgets[Uml::B]->lineEndY();

    return qMin(heightA, heightB);
}

/**
 * Reimplemented from UMLWidget::setupContextMenuActions to add
 * a submenu to change the text of the FloatingTextWidget of this
 * message widget.
 */
void MessageWidget::setupContextMenuActions(ListPopupMenu &menu)
{
    //TODO: Investigate the constructor parameters.
    ListPopupMenu* floatingtextSubMenu = new ListPopupMenu(&menu, m_floatingTextWidget, 0, 0);
    floatingtextSubMenu->setTitle( i18n( "Operation" ) );

    menu.addMenu( floatingtextSubMenu );
    // The floatingtextSubMenu is destroyed automatically when menu is destroyed.
}

bool MessageWidget::loadFromXMI(QDomElement& qElement)
{
    if ( !UMLWidget::loadFromXMI(qElement) ) {
        uDebug() << "Loading UMLwidget failed!";  //:TODO:
        return false;
    }

    QString textid = qElement.attribute( "textid", "-1" );
    QString widgetaid = qElement.attribute( "widgetaid", "-1" );
    QString widgetbid = qElement.attribute( "widgetbid", "-1" );

    m_customOperation = qElement.attribute( "operation", "" );
    m_sequenceNumber = qElement.attribute( "seqnum", "" );
    QString sequenceMessageType = qElement.attribute( "sequencemessagetype",
                                                      QString::number(Uml::sequence_message_asynchronous));
    m_sequenceMessageType = (Uml::Sequence_Message_Type)sequenceMessageType.toInt();

    if (m_sequenceMessageType == Uml::sequence_message_lost || m_sequenceMessageType == Uml::sequence_message_found) {
        m_clickedPoint.setX(qElement.attribute( "xclicked", "-1" ).toDouble());
        m_clickedPoint.setY(qElement.attribute( "yclicked", "-1" ).toDouble());
    }

    Uml::IDType widgetAId = STR2ID(widgetaid);
    Uml::IDType widgetBId = STR2ID(widgetbid);
    Uml::IDType textId    = STR2ID(textid);

    m_objectWidgets[Uml::A]->setLocalID(widgetAId);
    m_objectWidgets[Uml::B]->setLocalID(widgetBId);

    Uml::TextRole tr = Uml::TextRole::Seq_Message;
    if (widgetAId == widgetBId)
        tr = Uml::TextRole::Seq_Message_Self;

    //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if ( !element.isNull() ) {
        QString tag = element.tagName();
        if (tag == "floatingtext") {
            m_floatingTextWidget = new FloatingTextWidget( tr, textId );
            m_floatingTextWidget->setText(operationText(umlScene()));
            if( ! m_floatingTextWidget->loadFromXMI(element) ) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete m_floatingTextWidget;
                m_floatingTextWidget = 0;
            }
        } else {
            uError() << "unknown tag " << tag;
        }
    }
    return true;
}

void MessageWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement messageElement = qDoc.createElement( "messagewidget" );
    UMLWidget::saveToXMI( qDoc, messageElement );
    messageElement.setAttribute( "widgetaid", ID2STR(m_objectWidgets[Uml::A]->localID()) );
    messageElement.setAttribute( "widgetbid", ID2STR(m_objectWidgets[Uml::B]->localID()) );
    UMLOperation *pOperation = operation();
    if (pOperation)
        messageElement.setAttribute( "operation", ID2STR(pOperation->id()) );
    else
        messageElement.setAttribute( "operation", m_customOperation );
    messageElement.setAttribute( "seqnum", m_sequenceNumber );
    messageElement.setAttribute( "sequencemessagetype", m_sequenceMessageType );
    if (m_sequenceMessageType == Uml::sequence_message_lost || m_sequenceMessageType == Uml::sequence_message_found) {
        messageElement.setAttribute( "xclicked", m_clickedPoint.x());
        messageElement.setAttribute( "yclicked", m_clickedPoint.y());
    }

    // save the corresponding message text
    if (m_floatingTextWidget && !m_floatingTextWidget->text().isEmpty()) {
        messageElement.setAttribute( "textid", ID2STR(m_floatingTextWidget->id()));
        m_floatingTextWidget->saveToXMI( qDoc, messageElement );
    }

    qElement.appendChild( messageElement );
}

void MessageWidget::updateGeometry()
{
    QSizeF minSize = UMLWidget::DefaultMinimumSize;
    QSizeF maxSize = UMLWidget::DefaultMaximumSize;

    switch(m_sequenceMessageType) {
    case Uml::sequence_message_asynchronous:
        if (isSelf()) {
            minSize.setWidth(MessageWidget::SelfLoopBoxWidth);
            maxSize.setWidth(MessageWidget::SelfLoopBoxWidth);
        }
        else {
            minSize.setWidth(MessageWidget::ArrowWidth);
            minSize.setHeight(MessageWidget::ArrowWidth);
            maxSize.setHeight(minSize.height());
        }
        break;

    case Uml::sequence_message_synchronous:
        minSize.setWidth(MessageWidget::SynchronousBoxWidth);
        if (isSelf()) {
            minSize.rwidth() += MessageWidget::SelfLoopBoxWidth;
            maxSize.setWidth(minSize.width());
        }
        else {
            minSize.rwidth() += MessageWidget::ArrowWidth;
        }
        break;

    case Uml::sequence_message_creation:
        // creation code.
        break;

    case Uml::sequence_message_found:
    case Uml::sequence_message_lost:
        minSize.setWidth(MessageWidget::ArrowWidth + MessageWidget::FoundLostCircleRadius);
        minSize.setHeight(qMax(MessageWidget::ArrowWidth, MessageWidget::FoundLostCircleRadius));

        maxSize.setHeight(minSize.height());
        break;

    }

    setMinimumSize(minSize);
    setMaximumSize(maxSize);

    UMLWidget::updateGeometry();
}

QVariant MessageWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if (change == SizeHasChanged) {
        // code to ensure proper position of FloatingText

    }
    return UMLWidget::attributeChange(change, oldValue);
}

QVariant MessageWidget::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (umlScene()) {
        if (change == ItemPositionChange && umlScene()->isMouseMovingItems()) {
            QPointF newPoint = value.toPointF();
            if (umlScene()->mouseGrabberItem() == this) {
                newPoint.rx() = x(); // No horizontal movement
            }
            newPoint.ry() = qMax(newPoint.y(), minY());
            return newPoint;
        }
        else if (change == ItemPositionHasChanged) {
            if (m_objectWidgets[Uml::A]) {
                m_objectWidgets[Uml::A]->adjustSequentialLineEnd();
            }

            if (m_objectWidgets[Uml::B]) {
                m_objectWidgets[Uml::B]->adjustSequentialLineEnd();
            }
        }
        else if (change == ItemSceneHasChanged) {
            if (!m_isMsgSceneSetBefore) {
                m_isMsgSceneSetBefore = true;
                // Use timer to disambiguate situation where virtual
                // functions might not call be called
                // appropriately.

                // Also, set the timeout to higher value than the one
                // used in WidgetBase::itemChange so that the base's
                // delayed initialization is done before.

                QTimer::singleShot(100, this, SLOT(slotDelayedInit()));
            }
        }
    }
    return UMLWidget::itemChange(change, value);
}

void MessageWidget::setTextPosition()
{
    if (m_floatingTextWidget == NULL) {
        uDebug() << "m_floatingTextWidget is NULL" << endl;
        return;
    }
    if (m_floatingTextWidget->displayText().isEmpty()) {
        return;
    }
    qreal ftX = constrainedX(m_floatingTextWidget->x(), m_floatingTextWidget->width(), m_floatingTextWidget->textRole());
    qreal ftY = y() - m_floatingTextWidget->height();
    m_floatingTextWidget->setPos(ftX, ftY);
}

/**
 * Shortcut for calling m_floatingTextWidget->setLink() followed by
 * this->setTextPosition().
 */
void MessageWidget::setLinkAndTextPos()
{
    if (m_floatingTextWidget) {
        m_floatingTextWidget->setLink(this);
        setTextPosition();
    }
}

/**
 * Returns the textX arg with constraints applied.  Auxiliary to
 * setTextPosition() and constrainTextPos().
 */
qreal MessageWidget::constrainedX(qreal textX, qreal textWidth, Uml::TextRole tr) const
{
    qreal result = textX;
    const qreal minTextX = x() + 5;
    if (textX < minTextX || tr == Uml::TextRole::Seq_Message_Self) {
        result = minTextX;
    } else {
        ObjectWidget *objectAtRight = NULL;
        if (m_objectWidgets[Uml::B]->x() > m_objectWidgets[Uml::A]->x())
            objectAtRight = m_objectWidgets[Uml::B];
        else
            objectAtRight = m_objectWidgets[Uml::A];
        const qreal objRight_seqLineX = objectAtRight->x() + objectAtRight->width() / 2;
        const qreal maxTextX = objRight_seqLineX - textWidth - 5;
        if (maxTextX <= minTextX)
            result = minTextX;
        else if (textX > maxTextX)
            result = maxTextX;
    }
    return result;
}

void MessageWidget::updateResizability()
{
    //TODO
}

void MessageWidget::drawSynchronous(QPainter *painter)
{
    qreal x1 = m_objectWidgets[Uml::A]->sequentialLineX();
    qreal x2 = m_objectWidgets[Uml::B]->sequentialLineX();
    const QSizeF sz = size();
    const QSizeF ArrowSize(ArrowWidth, ArrowHeight);

    if (isSelf()) {
        QRectF syncBox(0, 0, SynchronousBoxWidth, sz.height());
        QRectF syncPath(SynchronousBoxWidth, .5 * MessageWidget::ArrowHeight,
                        sz.width() - SynchronousBoxWidth,
                        sz.height() - MessageWidget::ArrowHeight);
        painter->drawRect(syncBox);
        painter->drawRect(syncPath);
        Widget_Utils::drawArrowHead(painter, syncPath.bottomLeft(),
                                    ArrowSize, Qt::LeftArrow, false);
        return;
    }

    // else part

    bool solid = true;
    bool nonSolid = false;

    if (x1 < x2) {
        QRectF syncBox(0, 0, MessageWidget::SynchronousBoxWidth, sz.height());
        syncBox.moveRight(sz.width()); // align the box to the right.
        painter->drawRect(syncBox);

        QLineF callLine(0, .5 * MessageWidget::ArrowHeight,
                        syncBox.left(), .5 * MessageWidget::ArrowHeight);
        painter->drawLine(callLine);
        painter->setBrush(painter->pen().color());
        Widget_Utils::drawArrowHead(painter, callLine.p2(), ArrowSize,
                                    Qt::RightArrow, solid);

        qreal returnLineY = sz.height() - .5 * MessageWidget::ArrowHeight;
        QLineF returnLine(0, returnLineY, syncBox.left(), returnLineY);
        // Draw arrowhead first only to use the current pen style for the head.
        Widget_Utils::drawArrowHead(painter, returnLine.p1(), ArrowSize,
                                    Qt::LeftArrow, nonSolid);
        // Now set a dashed pen style for the return line drawing.
        QPen pen(painter->pen());
        pen.setStyle(Qt::DashLine);
        painter->drawLine(returnLine);
    }
    else {
        QRectF syncBox(0, 0, MessageWidget::SynchronousBoxWidth, sz.height());
        painter->drawRect(syncBox);

        QLineF callLine(syncBox.right(), .5 * MessageWidget::ArrowHeight,
                        sz.width(), .5 * MessageWidget::ArrowHeight);
        painter->drawLine(callLine);
        painter->setBrush(painter->pen().color());
        Widget_Utils::drawArrowHead(painter, callLine.p1(), ArrowSize,
                                    Qt::LeftArrow, solid);

        qreal returnLineY = sz.height() - .5 * MessageWidget::ArrowHeight;
        QLineF returnLine(syncBox.right(), returnLineY, sz.width(), returnLineY);
        // Draw arrowhead first only to use the current pen style for the head.
        Widget_Utils::drawArrowHead(painter, returnLine.p2(), ArrowSize,
                                    Qt::RightArrow, nonSolid);
        // Now set a dashed pen style for the return line drawing.
        QPen pen(painter->pen());
        pen.setStyle(Qt::DashLine);
        painter->drawLine(returnLine);
    }
}

void MessageWidget::drawAsynchronous(QPainter *painter)
{
    qreal x1 = m_objectWidgets[Uml::A]->sequentialLineX();
    qreal x2 = m_objectWidgets[Uml::B]->sequentialLineX();
    const QSizeF sz = size();
    const QSizeF ArrowSize(ArrowWidth, ArrowHeight);

    const bool nonSolid = false;
    if (isSelf()) {
        QLineF lines[3];
        lines[0].setLine(0, 0, sz.width(), 0);
        lines[1].setPoints(lines[0].p2(), QPointF(lines[0].p2().x(), sz.height() - .5 * ArrowHeight));
        lines[2].setPoints(lines[1].p2(), QPointF(0, lines[1].p2().y()));

        painter->drawLines(lines, 3);
        Widget_Utils::drawArrowHead(painter, lines[2].p2(), ArrowSize,
                                    Qt::LeftArrow, nonSolid);
        return;
    }

    QLineF line(0, .5 * ArrowHeight, sz.width(), .5 * ArrowHeight);
    painter->drawLine(line);

    Qt::ArrowType dir = x1 < x2 ? Qt::RightArrow : Qt::LeftArrow;
    QPointF headPos = x1 < x2 ? line.p2() : line.p1();
    Widget_Utils::drawArrowHead(painter, headPos, ArrowSize, dir, nonSolid);
}

void MessageWidget::drawFound(QPainter *painter)
{
    qreal x1 = m_objectWidgets[Uml::A]->sequentialLineX();
    qreal x2 = m_clickedPoint.x();
    const QSizeF sz = size();
    const QSizeF ArrowSize(ArrowWidth, ArrowHeight);
    const bool nonSolid = false;

    painter->setBrush(painter->pen().color());
    QRectF circle(0, 0, FoundLostCircleRadius, FoundLostCircleRadius);
    if (x1 < x2) {
        circle.moveRight(sz.width());
        painter->drawEllipse(circle);

        QLineF line(0, circle.center().y(), circle.left(), circle.center().y());
        painter->drawLine(line);

        Widget_Utils::drawArrowHead(painter, line.p1(), ArrowSize,
                                    Qt::LeftArrow, nonSolid);
    }
    else {
        painter->drawEllipse(circle);

        QLineF line(circle.right(), circle.center().y(), sz.width(), circle.center().y());
        painter->drawLine(line);

        Widget_Utils::drawArrowHead(painter, line.p2(), ArrowSize,
                                    Qt::RightArrow, nonSolid);
    }
}

void MessageWidget::drawLost(QPainter *painter)
{
    qreal x1 = m_objectWidgets[Uml::A]->sequentialLineX();
    qreal x2 = m_clickedPoint.x();
    const QSizeF sz = size();
    const QSizeF ArrowSize(ArrowWidth, ArrowHeight);
    const bool nonSolid = false;

    painter->setBrush(painter->pen().color());
    QRectF circle(0, 0, FoundLostCircleRadius, FoundLostCircleRadius);
    if (x1 < x2) {
        circle.moveRight(sz.width());
        painter->drawEllipse(circle);

        QLineF line(0, circle.center().y(), circle.left(), circle.center().y());
        painter->drawLine(line);

        Widget_Utils::drawArrowHead(painter, line.p2(), ArrowSize,
                                    Qt::RightArrow, nonSolid);
    }
    else {
        painter->drawEllipse(circle);

        QLineF line(circle.right(), circle.center().y(), sz.width(), circle.center().y());
        painter->drawLine(line);

        Widget_Utils::drawArrowHead(painter, line.p1(), ArrowSize,
                                    Qt::LeftArrow, nonSolid);
    }
}

void MessageWidget::drawCreation(QPainter *painter)
{
    Q_UNUSED(painter);  //:TODO:
    // qreal x1 = m_objectWidgets[Uml::A]->x();
    // qreal x2 = m_objectWidgets[Uml::B]->x();
    // qreal w = width() - 1;
    // //qreal h = height() - 1;
    // bool messageOverlapsA = m_objectWidgets[Uml::A]->messageOverlap( y(), this );
    // //bool messageOverlapsB = m_objectWidgets[Uml::B]->messageOverlap( y(), this );

    // const qreal lineY = offsetY + 4;
    // if (x1 < x2) {
    //     if (messageOverlapsA)  {
    //         offsetX += 7;
    //         w -= 7;
    //     }
    //     drawArrow(p, offsetX, lineY, w, Qt::RightArrow);
    //     if (messageOverlapsA)  {
    //         offsetX -= 7;
    //     }
    // } else      {
    //     if (messageOverlapsA)  {
    //         w -= 7;
    //     }
    //     drawArrow(p, offsetX, lineY, w, Qt::LeftArrow);
    // }

    // if (isSelected())
    //     drawSelected(&p, offsetX, offsetY);
}

void MessageWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu *menu = ListPopupMenu::menuFromAction(action);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }
    ListPopupMenu::MenuType sel = menu->getMenuType(action);

    if(sel == ListPopupMenu::mt_Delete) {
        // This will clean up this widget and the text widget:
        umlScene()->removeWidget(this);
    } else {

        UMLWidget::slotMenuSelection( action );
    }
}

void MessageWidget::slotDelayedInit()
{
    // Get reference to pointers.
    ObjectWidget *&objA = m_objectWidgets[Uml::A];
    ObjectWidget *&objB = m_objectWidgets[Uml::B];

    updateResizability();

    if (!objA || !objB) {
        return;
    }

    UMLClassifier *c = dynamic_cast<UMLClassifier*>(objB->umlObject());
    UMLOperation *op = 0;
    if (c && !m_customOperation.isEmpty()) {
        Uml::IDType opId = STR2ID(m_customOperation);
        op = dynamic_cast<UMLOperation*>( c->findChildObjectById(opId, true) );
        if (op) {
            // If the UMLOperation is set, m_customOperation isn't used anyway.
            // Just setting it empty for the sake of sanity.
            m_customOperation.clear();
        }
    }

    if (op) {
        setOperation(op);  // This requires a valid m_floatingTextWidget.
    }

    if (objA == objB) {
        m_floatingTextWidget->setTextRole(Uml::TextRole::Seq_Message_Self);
    }
    setLinkAndTextPos();

    objA->messageAdded(this);
    handleObjectMove(objA);
    objA->adjustSequentialLineEnd();

    if (objB != objA) {
        objB->messageAdded(this);
        handleObjectMove(objB);
        objB->adjustSequentialLineEnd();
    }
}

#include "messagewidget.moc"
