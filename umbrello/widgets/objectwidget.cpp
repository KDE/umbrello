/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "objectwidget.h"

// local includes
#include "classpropdlg.h"
#include "debug_utils.h"
#include "listpopupmenu.h"
#include "messagewidget.h"
#include "seqlinewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlscene.h"
#include "umlview.h"

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

// qt includes
#include <QPointer>
#include <QPainter>
#include <QValidator>

#define O_MARGIN 5
#define O_WIDTH 40
#define A_WIDTH 20
#define A_HEIGHT 40
#define A_MARGIN 5

/**
 * The number of pixels margin between the lowest message
 * and the bottom of the vertical line
 */
static const int sequenceLineMargin = 20;

/**
 * Creates an ObjectWidget.
 *
 * @param scene   The parent to this object.
 * @param o       The object it will be representing.
 * @param lid     The local id for the object.
 */
ObjectWidget::ObjectWidget(UMLScene * scene, UMLObject *o, Uml::ID::Type lid)
  : UMLWidget(scene, WidgetBase::wt_Object, o),
    m_multipleInstance(false),
    m_drawAsActor(false),
    m_showDestruction(false),
    m_isOnDestructionBox(false)
{
    m_nLocalID = Uml::ID::None;
    if( m_scene != NULL && m_scene->type() == Uml::DiagramType::Sequence ) {
        m_pLine = new SeqLineWidget( m_scene, this );
    } else {
        m_pLine = 0;
    }
    if( lid != Uml::ID::None )
        m_nLocalID = lid;
}

/**
 * Destructor.
 */
ObjectWidget::~ObjectWidget()
{
}

/**
 * Sets the local id of the object.
 *
 * @param id   The local id of the object.
 */
void ObjectWidget::setLocalID(Uml::ID::Type id)
{
    m_nLocalID = id;
}

/**
 * Returns the local ID for this object.  This ID is used so that
 * many objects of the same @ref UMLObject instance can be on the
 * same diagram.
 *
 * @return  The local ID.
 */
Uml::ID::Type ObjectWidget::localID() const
{
    return m_nLocalID;
}

/**
 * Sets whether representing a multi-instance object.
 *
 * @param multiple  Object state. true- multi, false - single.
 */
void ObjectWidget::setMultipleInstance(bool multiple)
{
    //make sure only calling this in relation to an object on a collab. diagram
    if(m_scene->type() != Uml::DiagramType::Collaboration)
        return;
    m_multipleInstance = multiple;
    updateGeometry();
    update();
}

/**
 * Returns whether object is representing a multi-object.
 *
 * @return  True if object is representing a multi-object.
 */
bool ObjectWidget::multipleInstance() const
{
    return m_multipleInstance;
}

/**
 * Overridden from UMLWidget.
 * Moves the widget to a new position using the difference between the
 * current position and the new position.
 * Y position is ignored, and widget is only moved along X axis.
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position
 *                          (isn't used).
 */
void ObjectWidget::moveWidgetBy(qreal diffX, qreal diffY)
{
    Q_UNUSED(diffY);
    setX(x() + diffX);
}

/**
 * Overridden from UMLWidget.
 * Modifies the value of the diffX and diffY variables used to move the widgets.
 * All the widgets are constrained to be moved only in X axis (diffY is set to 0).
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 */
void ObjectWidget::constrainMovementForAllWidgets(qreal &diffX, qreal &diffY)
{
    Q_UNUSED(diffX);
    diffY = 0;
}

/**
 * Override default method.
 */
void ObjectWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if ( m_drawAsActor )
        paintActor(painter);
    else
        paintObject(painter);

    setPenFromSettings(painter);
    if(m_selected)
        paintSelected(painter);
}

/**
 * Handles a popup menu selection.
 */
void ObjectWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename_Object:
        {
            bool ok;
            QRegExpValidator* validator = new QRegExpValidator(QRegExp(".*"), 0);
            QString name = KInputDialog::getText
                   (i18n("Rename Object"),
                    i18n("Enter object name:"),
                    m_instanceName,
                    &ok,
                    m_scene->activeView(),
                    validator);
            if (ok) {
                m_instanceName = name;
                updateGeometry();
                moveEvent( 0 );
                update();
                UMLApp::app()->document()->setModified(true);
            }
            delete validator;
            break;
        }
    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        updateGeometry();
        moveEvent( 0 );
        update();
        break;

    case ListPopupMenu::mt_Up:
        tabUp();
        break;

    case ListPopupMenu::mt_Down:
        tabDown();
        break;

    default:
        UMLWidget::slotMenuSelection(action);
        break;
    }
}

/**
 * Overrides method from UMLWidget
 */
QSizeF ObjectWidget::minimumSize()
{
    int width, height;
    const QFontMetrics &fm = getFontMetrics(FT_UNDERLINE);
    const int fontHeight  = fm.lineSpacing();
    const QString t = m_instanceName + " : " + name();
    const int textWidth = fm.width(t);
    if ( m_drawAsActor ) {
        width = textWidth > A_WIDTH?textWidth:A_WIDTH;
        height = A_HEIGHT + fontHeight + A_MARGIN;
        width += A_MARGIN * 2;
    } else {
        width = textWidth > O_WIDTH?textWidth:O_WIDTH;
        height = fontHeight + O_MARGIN * 2;
        width += O_MARGIN * 2;
        if (m_multipleInstance) {
            width += 10;
            height += 10;
        }
    }//end else drawasactor

    return QSizeF(width, height);
}

/**
 * Sets whether to draw as an Actor.
 *
 * @param drawAsActor   True if widget shall be drawn as an actor.
 */
void ObjectWidget::setDrawAsActor(bool drawAsActor)
{
    m_drawAsActor = drawAsActor;
    updateGeometry();
}

/**
 * Returns whether to draw as an Actor or not.
 *
 * @return  True if widget is drawn as an actor.
 */
bool ObjectWidget::drawAsActor() const
{
    return m_drawAsActor;
}

/**
 * Activate the object after serializing it from a QDataStream
 */
bool ObjectWidget::activate(IDChangeLog* ChangeLog /*= 0*/)
{
    if (! UMLWidget::activate(ChangeLog))
        return false;
    if (m_showDestruction && m_pLine)
        m_pLine->setupDestructionBox();
    moveEvent(0);
    return true;
}

/**
 * Sets the x-coordinate.
 * Reimplements the method from UMLWidget.
 *
 * @param x The x-coordinate to be set.
 */
void ObjectWidget::setX(qreal x)
{
    UMLWidget::setX(x);
    moveEvent(0);
}

/**
 * Sets the y-coordinate.
 * Reimplements the method from UMLWidget.
 *
 * @param y The y-coordinate to be set.
 */
void ObjectWidget::setY(qreal y)
{
    UMLWidget::setY(y);
    moveEvent(0);
}

/**
 * Return the x coordinate of the widgets center.
 * @return The x-coordinate of the widget center.
 */
qreal ObjectWidget::centerX()
{
    return x() + width()/2;
}

/**
 * Overrides the standard operation.
 */
void ObjectWidget::moveEvent(QGraphicsSceneMouseEvent *m)
{
    Q_UNUSED(m)
    emit sigWidgetMoved( m_nLocalID );
    if (m_pLine) {
        m_pLine->setStartPoint(x() + width() / 2, y() + height());
    }
}

/**
 * Overrides the standard operation.
 */
void ObjectWidget::mousePressEvent(QGraphicsSceneMouseEvent *me)
{
    UMLWidget::mousePressEvent(me);
    m_isOnDestructionBox = false;
    SeqLineWidget *pLine = sequentialLine();

    if (pLine->onDestructionBox(me->scenePos())) {
        m_isOnDestructionBox = true;
        qreal oldX = x() + width() / 2;
        qreal oldY = getEndLineY() - 10;
        m_oldPos = QPointF(oldX, oldY);
    }
}

/**
 * Overrides the standard operation.
 */
void ObjectWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* me)
{
    if (me->button() != Qt::LeftButton) {
        return;
    }

    if (m_inResizeArea) {
        resize(me);
        return;
    }

    int diffY = me->scenePos().y() - m_oldPos.y();

    if (m_isOnDestructionBox) {
        moveDestructionBy (diffY);
    }
    else {
        UMLWidget::mouseMoveEvent(me);
    }
}

/**
 * Moves the destruction Box to a new position using the difference between the
 * current position and the new position.
 * The destruction box is only moved along Y axis.
 *
 * @param diffY The difference between current Y position and new Y position
 */
void ObjectWidget::moveDestructionBy(qreal diffY)
{
    // endLine = length of the life line + diffY - 10 to center on the destruction box
    qreal endLine = getEndLineY() + diffY - 10;
    SeqLineWidget *pLine = sequentialLine();
    pLine->setEndOfLine(endLine);
    m_oldPos.setY(endLine);
}

/**
 * Handles a color change signal.
 */
void ObjectWidget::slotFillColorChanged(Uml::ID::Type /*viewID*/)
{
    UMLWidget::setFillColor( m_scene->fillColor() );
    UMLWidget::setLineColor( m_scene->lineColor() );

    if( m_pLine)
        m_pLine->setPen( QPen( UMLWidget::lineColor(), UMLWidget::lineWidth(), Qt::DashLine ) );
}

/**
 * Used to cleanup any other widget it may need to delete.
 */
void ObjectWidget::cleanup()
{
    UMLWidget::cleanup();
    if( m_pLine ) {
        m_pLine->cleanup();
        delete m_pLine;
    }
}

/**
 * Show a properties dialog for an ObjectWidget.
 */
void ObjectWidget::showPropertiesDialog()
{
    umlScene()->updateDocumentation(false);
    QPointer<ClassPropDlg> dlg = new ClassPropDlg((QWidget*)UMLApp::app(), this);
    if (dlg->exec()) {
        umlScene()->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
    }
    dlg->close();
    delete dlg;
}

/**
 * Draw the object as an object (default).
 */
void ObjectWidget::paintObject(QPainter *painter)
{
    QFont oldFont = painter->font();
    QFont font = UMLWidget::font();
    font.setUnderline( true );
    painter->setFont( font );

    setPenFromSettings(painter);
    if(UMLWidget::useFillColor())
        painter->setBrush(UMLWidget::fillColor());
    else
        painter->setBrush(m_scene->activeView()->viewport()->palette().color(QPalette::Background));
    const int w = width();
    const int h = height();

    const QString t = m_instanceName + " : " + name();
    int multiInstOfst = 0;
    if ( m_multipleInstance ) {
        painter->drawRect(10, 10, w - 10, h - 10);
        painter->drawRect(5, 5, w - 10, h - 10);
        multiInstOfst = 10;
    }
    painter->drawRect(0, 0, w - multiInstOfst, h - multiInstOfst);
    painter->setPen(textColor());
    painter->drawText(O_MARGIN, O_MARGIN,
               w - O_MARGIN * 2 - multiInstOfst, h - O_MARGIN * 2 - multiInstOfst,
               Qt::AlignCenter, t);

    painter->setFont( oldFont );
}

/**
 * Draw the object as an actor.
 */
void ObjectWidget::paintActor(QPainter *painter)
{
    const QFontMetrics &fm = getFontMetrics(FT_UNDERLINE);

    setPenFromSettings(painter);
    if ( UMLWidget::useFillColor() )
        painter->setBrush( UMLWidget::fillColor() );
    const int w = width();
    const int textStartY = A_HEIGHT + A_MARGIN;
    const int fontHeight  = fm.lineSpacing();

    const int middleX = w / 2;
    const int thirdH = A_HEIGHT / 3;

    //draw actor
    painter->drawEllipse(middleX - A_WIDTH / 2, 0,  A_WIDTH, thirdH);//head
    painter->drawLine(middleX, thirdH, middleX, thirdH * 2);//body
    painter->drawLine(middleX, 2 * thirdH,
               middleX - A_WIDTH / 2, A_HEIGHT);//left leg
    painter->drawLine(middleX, 2 * thirdH,
               middleX + A_WIDTH / 2, A_HEIGHT);//right leg
    painter->drawLine(middleX - A_WIDTH / 2, thirdH + thirdH / 2,
               middleX + A_WIDTH / 2, thirdH + thirdH / 2);//arms
    //draw text
    painter->setPen(textColor());
    QString t = m_instanceName + " : " + name();
    painter->drawText(A_MARGIN, textStartY,
               w - A_MARGIN * 2, fontHeight, Qt::AlignCenter, t);
}

/**
 * Move the object up on a sequence diagram.
 */
void ObjectWidget::tabUp()
{
    int newY = y() - height();
    if (newY < topMargin())
        newY = topMargin();
    setY( newY );
    moveEvent( 0 );
    adjustAssocs( x(), newY);
}

/**
 * Move the object down on a sequence diagram.
 */
void ObjectWidget::tabDown()
{
    int newY = y() + height();
    setY( newY );
    moveEvent( 0 );
    adjustAssocs( x(), newY);
}

/**
 * Returns the top margin constant (Y axis value)
 *
 * @return  Y coordinate of the space between the diagram top
 *          and the upper edge of the ObjectWidget.
 */
int ObjectWidget::topMargin()
{
    return 80 - height();
}

/**
 * Returns whether or not the widget can be moved vertically up.
 *
 * @return  True if widget can be moved upwards vertically.
 */
bool ObjectWidget::canTabUp()
{
    return (y() > topMargin());
}

/**
 * Sets whether to show deconstruction on sequence line.
 *
 * @param bShow   True if destruction on line shall be shown.
 */
void ObjectWidget::setShowDestruction(bool bShow)
{
    m_showDestruction = bShow;
    if( m_pLine )
        m_pLine->setupDestructionBox();
}

/**
 * Returns whether to show deconstruction on sequence line.
 *
 * @return  True if destruction on sequence line is shown.
 */
bool ObjectWidget::showDestruction() const
{
    return m_showDestruction;
}

/**
 * Sets the y position of the bottom of the vertical line.
 *
 * @param yPosition The y coordinate for the bottom of the line.
 */
void ObjectWidget::setEndLine(int yPosition)
{
    m_pLine->setEndOfLine(yPosition);
}

/**
 * Returns the end Y co-ord of the sequence line.
 *
 * @return  Y coordinate of the endpoint of the sequence line.
 */
int ObjectWidget::getEndLineY()
{
    int y = this->y() + height();
    if( m_pLine)
        y += m_pLine->getLineLength();
    if ( m_showDestruction )
        y += 10;
    return y;
}

/**
 * Add a message widget to the list.
 *
 * @param message   Pointer to the MessageWidget to add.
 */
void ObjectWidget::messageAdded(MessageWidget* message)
{
    if ( m_messages.count(message) ) {
        uError() << message->name() << ": duplicate entry !";
        return ;
    }
    m_messages.append(message);
}

/**
 * Remove a message widget from the list.
 *
 * @param message   Pointer to the MessageWidget to remove.
 */
void ObjectWidget::messageRemoved(MessageWidget* message)
{
    if ( m_messages.removeAll(message) == false ) {
        uError() << message->name() << ": missing entry !";
        return ;
    }
}

/**
 * Called when a message widget with an end on this object has
 * moved up or down.
 * Sets the bottom of the line to a nice position.
 */
void ObjectWidget::slotMessageMoved()
{
    int lowestMessage = 0;
    foreach (MessageWidget* message, m_messages) {
        int messageHeight = message->y() + message->height();
        if (lowestMessage < messageHeight) {
            lowestMessage = messageHeight;
        }
    }
    m_pLine->setEndOfLine(lowestMessage + sequenceLineMargin);
}

/**
 * Returns whether a message is overlapping with another message.
 * Used by MessageWidget::paint() methods.
 *
 * @param y               top of your message
 * @param messageWidget   pointer to your message so it doesn't check against itself
 */
bool ObjectWidget::messageOverlap(qreal y, MessageWidget* messageWidget)
{
    foreach (MessageWidget* message, m_messages) {
        if (message == messageWidget) {
            continue;
        }
        const qreal msgY = message->y();
        const qreal msgHeight = msgY + message->height();
        if (y >= msgY && y <= msgHeight) {
            return true;
        }
    }
    return false;
}

/**
 * Return the SeqLineWidget.
 * Returns a non NULL pointer if this ObjectWidget is part of a
 * sequence diagram.
 */
SeqLineWidget *ObjectWidget::sequentialLine() const
{
    return m_pLine;
}

/**
 * Overridden from UMLWidget.
 * Returns the cursor to be shown when resizing the widget.
 * The cursor shown is KCursor::sizeHorCursor().
 *
 * @return The cursor to be shown when resizing the widget.
 */
QCursor ObjectWidget::resizeCursor()
{
    return Qt::SizeHorCursor;
}

/**
 * Overridden from UMLWidget.
 * Resizes the width of the object widget.
 * Object widgets can only be resized horizontally, so height isn't modified.
 *
 * @param newW   The new width for the widget.
 * @param newH   The new height for the widget (isn't used).
 */
void ObjectWidget::resizeWidget(qreal newW, qreal newH)
{
    Q_UNUSED(newH);
    setSize(newW, height());
}

/**
 * Saves to the "objectwidget" XMI element.
 */
void ObjectWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement objectElement = qDoc.createElement( "objectwidget" );
    UMLWidget::saveToXMI( qDoc, objectElement );
    objectElement.setAttribute( "drawasactor", m_drawAsActor );
    objectElement.setAttribute( "multipleinstance", m_multipleInstance );
    objectElement.setAttribute( "localid", Uml::ID::toString(m_nLocalID) );
    objectElement.setAttribute( "decon", m_showDestruction );
    qElement.appendChild( objectElement );
}

/**
 * Loads from a "objectwidget" XMI element.
 */
bool ObjectWidget::loadFromXMI(QDomElement& qElement)
{
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    QString draw = qElement.attribute( "drawasactor", "0" );
    QString multi = qElement.attribute( "multipleinstance", "0" );
    QString localid = qElement.attribute( "localid", "0" );
    QString decon = qElement.attribute( "decon", "0" );

    m_drawAsActor = (bool)draw.toInt();
    m_multipleInstance = (bool)multi.toInt();
    m_nLocalID = Uml::ID::fromString(localid);
    m_showDestruction = (bool)decon.toInt();
    return true;
}

#include "objectwidget.moc"
