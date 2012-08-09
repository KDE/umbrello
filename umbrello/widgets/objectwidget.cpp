/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "objectwidget.h"

// local includes
#include "classpropdlg.h"
#include "debug_utils.h"
#include "listpopupmenu.h"
#include "messagewidget.h"
#include "objectwidgetcontroller.h"
#include "seqlinewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
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
ObjectWidget::ObjectWidget(UMLScene * scene, UMLObject *o, Uml::IDType lid)
  : UMLWidget(scene, WidgetBase::wt_Object, o),
    m_multipleInstance(false),
    m_drawAsActor(false),
    m_showDestruction(false)
{
    m_nLocalID = Uml::id_None;
    if( m_scene != NULL && m_scene->type() == Uml::DiagramType::Sequence ) {
        m_pLine = new SeqLineWidget( m_scene, this );
        //Sets specific widget controller for sequence diagrams
        delete m_widgetController;
        m_widgetController = new ObjectWidgetController(this);
    } else {
        m_pLine = 0;
    }
    if( lid != Uml::id_None )
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
void ObjectWidget::setLocalID(Uml::IDType id)
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
Uml::IDType ObjectWidget::localID() const
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
 * Override default method.
 */
void ObjectWidget::paint(QPainter & p, int offsetX, int offsetY)
{
    if ( m_drawAsActor )
        drawActor( p, offsetX, offsetY );
    else
        drawObject( p, offsetX, offsetY );

    setPenFromSettings(p);
    if(m_selected)
        drawSelected(&p, offsetX, offsetY);
}

/**
 * Handles a popup menu selection.
 */
void ObjectWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);
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
UMLSceneSize ObjectWidget::minimumSize()
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

    return UMLSceneSize(width, height);
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
void ObjectWidget::setX(int x)
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
void ObjectWidget::setY(int y)
{
    UMLWidget::setY(y);
    moveEvent(0);
}

/**
 * Overrides the standard operation.
 */
void ObjectWidget::moveEvent(QMoveEvent *m)
{
    Q_UNUSED(m)
    emit sigWidgetMoved( m_nLocalID );
    if (m_pLine) {
        m_pLine->setStartPoint(x() + width() / 2, y() + height());
    }
}

/**
 * Handles a color change signal.
 */
void ObjectWidget::slotFillColorChanged(Uml::IDType /*viewID*/)
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
void ObjectWidget::drawObject(QPainter & p, int offsetX, int offsetY)
{
    QFont oldFont = p.font();
    QFont font = UMLWidget::font();
    font.setUnderline( true );
    p.setFont( font );

    setPenFromSettings(p);
    if(UMLWidget::useFillColor())
        p.setBrush(UMLWidget::fillColor());
    else
        p.setBrush(m_scene->activeView()->viewport()->palette().color(QPalette::Background));
    const int w = width();
    const int h = height();

    const QString t = m_instanceName + " : " + name();
    int multiInstOfst = 0;
    if ( m_multipleInstance ) {
        p.drawRect(offsetX + 10, offsetY + 10, w - 10, h - 10);
        p.drawRect(offsetX + 5, offsetY + 5, w - 10, h - 10);
        multiInstOfst = 10;
    }
    p.drawRect(offsetX, offsetY, w - multiInstOfst, h - multiInstOfst);
    p.setPen(textColor());
    p.drawText(offsetX + O_MARGIN, offsetY + O_MARGIN,
               w - O_MARGIN * 2 - multiInstOfst, h - O_MARGIN * 2 - multiInstOfst,
               Qt::AlignCenter, t);

    p.setFont( oldFont );
}

/**
 * Draw the object as an actor.
 */
void ObjectWidget::drawActor(QPainter & p, int offsetX, int offsetY)
{
    const QFontMetrics &fm = getFontMetrics(FT_UNDERLINE);

    setPenFromSettings(p);
    if ( UMLWidget::useFillColor() )
        p.setBrush( UMLWidget::fillColor() );
    const int w = width();
    const int textStartY = A_HEIGHT + A_MARGIN;
    const int fontHeight  = fm.lineSpacing();

    const int middleX = offsetX + w / 2;
    const int thirdH = A_HEIGHT / 3;

    //draw actor
    p.drawEllipse(middleX - A_WIDTH / 2, offsetY,  A_WIDTH, thirdH);//head
    p.drawLine(middleX, offsetY + thirdH, middleX, offsetY + thirdH * 2);//body
    p.drawLine(middleX, offsetY + 2 * thirdH,
               middleX - A_WIDTH / 2, offsetY + A_HEIGHT);//left leg
    p.drawLine(middleX, offsetY +  2 * thirdH,
               middleX + A_WIDTH / 2, offsetY + A_HEIGHT);//right leg
    p.drawLine(middleX - A_WIDTH / 2, offsetY + thirdH + thirdH / 2,
               middleX + A_WIDTH / 2, offsetY + thirdH + thirdH / 2);//arms
    //draw text
    p.setPen(textColor());
    QString t = m_instanceName + " : " + name();
    p.drawText(offsetX + A_MARGIN, offsetY + textStartY,
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
 * Used by MessageWidget::draw() methods.
 *
 * @param y               top of your message
 * @param messageWidget   pointer to your message so it doesn't check against itself
 */
bool ObjectWidget::messageOverlap(int y, MessageWidget* messageWidget)
{
    foreach (MessageWidget* message, m_messages) {
        const int msgY = message->y();
        const int msgHeight = msgY + message->height();
        if (y >= msgY && y <= msgHeight && message != messageWidget) {
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
 * Saves to the "objectwidget" XMI element.
 */
void ObjectWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement objectElement = qDoc.createElement( "objectwidget" );
    UMLWidget::saveToXMI( qDoc, objectElement );
    objectElement.setAttribute( "drawasactor", m_drawAsActor );
    objectElement.setAttribute( "multipleinstance", m_multipleInstance );
    objectElement.setAttribute( "localid", ID2STR(m_nLocalID) );
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
    m_nLocalID = STR2ID(localid);
    m_showDestruction = (bool)decon.toInt();
    return true;
}

#include "objectwidget.moc"
