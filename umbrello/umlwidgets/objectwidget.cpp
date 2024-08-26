/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header file
#include "objectwidget.h"

// local includes
#include "classpropertiesdialog.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "docwindow.h"
#include "listpopupmenu.h"
#include "messagewidget.h"
#include "seqlinewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlscene.h"
#include "umlview.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QPointer>
#include <QPainter>
#include <QValidator>
#include <QXmlStreamWriter>

#define O_MARGIN  5
#define O_WIDTH  40
#define A_WIDTH  20
#define A_HEIGHT 40
#define A_MARGIN  5

DEBUG_REGISTER_DISABLED(ObjectWidget)

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
 */
ObjectWidget::ObjectWidget(UMLScene * scene, UMLObject *o)
  : UMLWidget(scene, WidgetBase::wt_Object, o),
    m_multipleInstance(false),
    m_drawAsActor(false),
    m_showDestruction(false),
    m_isOnDestructionBox(false)
{
    if (m_scene && m_scene->isSequenceDiagram()) {
        m_pLine = new SeqLineWidget( m_scene, this );
        m_pLine->setStartPoint(x() + width() / 2, y() + height());
    } else {
        m_pLine = nullptr;
    }
}

/**
 * Destructor.
 */
ObjectWidget::~ObjectWidget()
{
    cleanup();
}

/**
 * Sets whether representing a multi-instance object.
 *
 * @param multiple  Object state. true- multi, false - single.
 */
void ObjectWidget::setMultipleInstance(bool multiple)
{
    //make sure only calling this in relation to an object on a collab. diagram
    if (m_scene && m_scene->isCollaborationDiagram()) {
        m_multipleInstance = multiple;
        updateGeometry();
        update();
    }
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

void ObjectWidget::setSelected(bool state)
{
    UMLWidget::setSelected(state);
    if (m_pLine) {
        QPen pen = m_pLine->pen();
        int lineWidth = (int)UMLWidget::lineWidth();
        if (state)
            lineWidth = lineWidth ? lineWidth * 2 : 2;
        pen.setWidth(lineWidth);
        m_pLine->setPen(pen);
    }
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
    setX(x() + diffX);
    if (m_scene && (m_scene->type() != Uml::DiagramType::Sequence)) {
        setY(y() + diffY);
    }
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
    if (m_scene && m_scene->isSequenceDiagram()) {
        diffY = 0;
    }
}

/**
 * Override default method.
 */
void ObjectWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (m_drawAsActor)
        paintActor(painter);
    else
        paintObject(painter);

    setPenFromSettings(painter);

    UMLWidget::paint(painter, option, widget);
}

/**
 * Handles a popup menu selection.
 */
void ObjectWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        updateGeometry();
        moveEvent(nullptr);
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
QSizeF ObjectWidget::minimumSize() const
{
    int width, height;
    const QFontMetrics &fm = getFontMetrics(FT_UNDERLINE);
    const int fontHeight  = fm.lineSpacing();
    const QString t = m_instanceName + QStringLiteral(" : ") + name();
    const int textWidth = fm.horizontalAdvance(t);
    if (m_drawAsActor) {
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
bool ObjectWidget::activate(IDChangeLog *ChangeLog /*= nullptr*/)
{
    if (! UMLWidget::activate(ChangeLog))
        return false;
    if (m_showDestruction && m_pLine)
        m_pLine->setupDestructionBox();
    moveEvent(nullptr);
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
    moveEvent(nullptr);
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
    if (!UMLApp::app()->document()->loading())
        moveEvent(nullptr);
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
void ObjectWidget::moveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    Q_EMIT sigWidgetMoved(m_nLocalID);
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
    if (m_pLine && m_pLine->onDestructionBox(me->scenePos())) {
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
    if (m_inResizeArea) {
        DEBUG() << "resizing...";
        resize(me);
        moveEvent(nullptr);
        return;
    }

    if (m_isOnDestructionBox) {
        qreal diffY = me->scenePos().y() - m_oldPos.y();
        moveDestructionBy(diffY);
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
    UMLWidget::setFillColor(m_scene->fillColor());
    UMLWidget::setLineColor(m_scene->lineColor());

    if(m_pLine)
        m_pLine->setPen(QPen(UMLWidget::lineColor(), UMLWidget::lineWidth(), Qt::DashLine));
}

/**
 * Used to cleanup any other widget it may need to delete.
 */
void ObjectWidget::cleanup()
{
    UMLWidget::cleanup();
    if(m_pLine) {
        m_pLine->cleanup();
        delete m_pLine;
        m_pLine = nullptr;
    }
}

/**
 * Show a properties dialog for an ObjectWidget.
 */
bool ObjectWidget::showPropertiesDialog()
{
    bool result = false;
    UMLApp::app()->docWindow()->updateDocumentation(false);
    QPointer<ClassPropertiesDialog> dlg = new ClassPropertiesDialog((QWidget*)UMLApp::app(), this);
    if (dlg->exec()) {
        UMLApp::app()->docWindow()->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
        result = true;
    }
    dlg->close();
    delete dlg;
    return result;
}

/**
 * Draw the object as an object (default).
 */
void ObjectWidget::paintObject(QPainter *painter)
{
    QFont oldFont = painter->font();
    QFont font = UMLWidget::font();
    font.setUnderline(true);
    painter->setFont(font);

    setPenFromSettings(painter);
    if(UMLWidget::useFillColor())
        painter->setBrush(UMLWidget::fillColor());
    else
        painter->setBrush(m_scene->backgroundColor());
    const int w = width();
    const int h = height();

    const QString t = m_instanceName + QStringLiteral(" : ") + name();
    int multiInstOfst = 0;
    if (m_multipleInstance) {
        painter->drawRect(10, 10, w - 10, h - 10);
        painter->drawRect(5, 5, w - 10, h - 10);
        multiInstOfst = 10;
    }
    painter->drawRect(0, 0, w - multiInstOfst, h - multiInstOfst);
    painter->setPen(textColor());
    painter->drawText(O_MARGIN, O_MARGIN,
               w - O_MARGIN * 2 - multiInstOfst, h - O_MARGIN * 2 - multiInstOfst,
               Qt::AlignCenter, t);

    painter->setFont(oldFont);
}

/**
 * Draw the object as an actor.
 */
void ObjectWidget::paintActor(QPainter *painter)
{
    const QFontMetrics &fm = getFontMetrics(FT_UNDERLINE);

    setPenFromSettings(painter);
    if (UMLWidget::useFillColor())
        painter->setBrush(UMLWidget::fillColor());
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
    QString t = m_instanceName + QStringLiteral(" : ") + name();
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
    setY(newY);
    adjustAssocs(x(), newY);
}

/**
 * Move the object down on a sequence diagram.
 */
void ObjectWidget::tabDown()
{
    int newY = y() + height();
    setY(newY);
    adjustAssocs(x(), newY);
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
    if(m_pLine)
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
    if (m_pLine) {
        m_pLine->setEndOfLine(yPosition);
    }
}

/**
 * Returns the end Y co-ord of the sequence line.
 *
 * @return  Y coordinate of the endpoint of the sequence line.
 */
int ObjectWidget::getEndLineY()
{
    int y = this->y() + height();
    if(m_pLine)
        y += m_pLine->getLineLength();
    if (m_showDestruction)
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
    if (m_messages.count(message)) {
        logError1("ObjectWidget::messageAdded(%1) duplicate entry", message->name());
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
    if (m_messages.removeAll(message) == false) {
        logError1("ObjectWidget::messageAdded(%1) missing entry", message->name());
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
    if (m_pLine) {
        int lowestMessage = 0;
        for(MessageWidget *message : m_messages) {
            int messageHeight = message->y() + message->height();
            if (lowestMessage < messageHeight) {
                lowestMessage = messageHeight;
            }
        }
        m_pLine->setEndOfLine(lowestMessage + sequenceLineMargin);
    }
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
    for(MessageWidget *message : m_messages) {
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
 * Overridden from UMLWidget
 * Set color of object widget and sequence line on sequence diagrams.
 */
void ObjectWidget::setLineColorCmd(const QColor &color)
{
    UMLWidget::setLineColorCmd(color);
    if (m_pLine) {
        m_pLine->setLineColorCmd(color);
    }
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
QCursor ObjectWidget::resizeCursor() const
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
void ObjectWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("objectwidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeAttribute(QStringLiteral("drawasactor"), QString::number(m_drawAsActor));
    writer.writeAttribute(QStringLiteral("multipleinstance"), QString::number(m_multipleInstance));
    writer.writeAttribute(QStringLiteral("decon"), QString::number(m_showDestruction));
    writer.writeEndElement();
}

/**
 * Loads from a "objectwidget" XMI element.
 */
bool ObjectWidget::loadFromXMI(QDomElement& qElement)
{
    if(!UMLWidget::loadFromXMI(qElement))
        return false;
    QString draw = qElement.attribute(QStringLiteral("drawasactor"), QStringLiteral("0"));
    QString multi = qElement.attribute(QStringLiteral("multipleinstance"), QStringLiteral("0"));
    QString decon = qElement.attribute(QStringLiteral("decon"), QStringLiteral("0"));

    m_drawAsActor = (bool)draw.toInt();
    m_multipleInstance = (bool)multi.toInt();
    m_showDestruction = (bool)decon.toInt();
    return true;
}

