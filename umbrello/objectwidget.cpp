/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "objectwidget.h"

// local includes
#include "dialogs/classpropdlg.h"
#include "docwindow.h"
#include "listpopupmenu.h"
#include "seqlinewidget.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlscene.h"
#include "umlview.h"

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

// qt includes
#include <QtGui/QValidator>

/// Size used for drawing Actor
const QSizeF ObjectWidget::ActorSize = QSizeF(20, 40);

/**
 * The number of pixels margin between the lowest message and the
 * bottom of the vertical line
 */
const qreal ObjectWidget::SequenceLineMargin = 20;

/**
 * Creates an ObjectWidget.
 *
 * @param object    The object it will be representing.
 * @param lid       The local id for the object.
 */
ObjectWidget::ObjectWidget(UMLObject *object, const Uml::IDType& lid)
        : NewUMLRectWidget(0, object)
{
    m_baseType = Uml::wt_Object;
    m_localID = lid;
    m_multipleInstance = false;
    m_drawAsActor = false;
    m_showDestruction = false;

    // Currently sequential line is null and is setup in SceneChange
    // notification so that the sequential line is present only in
    // sequential diagram
    m_sequentialLine = 0;

    // Create a group to manage the text to be displayed.
    createTextItemGroup();
}

/// Destructor
ObjectWidget::~ObjectWidget()
{
}

/**
 * Sets the local ID for this widget. See @ref ObjectWidget::localID
 * for more information about this id.
 */
void ObjectWidget::setLocalID(const Uml::IDType& id)
{
    m_localID = id;
}

/**
 * Sets whether this ObjectWidget represents multiple instance.
 */
void ObjectWidget::setMultipleInstance(bool multiple)
{
    // make sure only calling this in relation to an object on a collab. diagram
    if(umlScene() && umlScene()->getType() != Uml::dt_Collaboration) {
        return;
    }
    m_multipleInstance = multiple;
    updateTextItemGroups();
}

/**
 * Sets whether the object should be drawn as an actor or just a
 * rectangle.
 */
void ObjectWidget::setDrawAsActor( bool drawAsActor )
{
    m_drawAsActor = drawAsActor;
    updateGeometry();
}

/**
 * Sets the destruction visibility on the end of sequential line.
 */
void ObjectWidget::setShowDestruction( bool bShow )
{
    m_showDestruction = bShow;
    if( m_sequentialLine ) {
        m_sequentialLine->updateDestructionBoxVisibility();
    }
}

/**
 * @return Y coordinate of the space between the diagram top and
 *         the upper edge of the ObjectWidget.
 */
qreal ObjectWidget::topMargin() const
{
    return 80 - size().height();
}

/// @retval True if widget can be moved upwards vertically.
bool ObjectWidget::canTabUp() const
{
    qreal y = pos().y();
    return (y > topMargin());
}

/// @return Y coordinate of the endpoint of the sequence line.
qreal ObjectWidget::lineEndY() const
{
    if (m_sequentialLine) {
        QPointF lineBottom = m_sequentialLine->boundingRect().bottomLeft();
        lineBottom = m_sequentialLine->mapToParent(lineBottom);
        return lineBottom.y();
    }

    uError() << "Line is null";
    return boundingRect().bottom();
}

/**
 * Sets the y position of the bottom of the vertical line.
 *
 * @param yPosition The y coordinate for the bottom of the line.
 */
void ObjectWidget::setLineEndY(qreal yPosition)
{
    if (m_sequentialLine) {
        m_sequentialLine->setEndOfLine(yPosition);
    }
}

/**
 * Adds \a message linked to this widget to the MessageList of this
 * ObjectWidget.
 */
void ObjectWidget::messageAdded(MessageWidget* message)
{
    if ( m_messages.count(message) ) {
        uError() << message->name() << ": duplicate entry !";
    }
    else {
        m_messages.append(message);
    }
}

/**
 * Removes \a message linked to this widget from the MessageList of
 * this ObjectWidget.
 */
void ObjectWidget::messageRemoved(MessageWidget* message)
{
    if ( m_messages.removeAll(message) == false ) {
        uError() << message->name() << ": missing entry !";
    }
}

/**
 * @return Whether a message is overlapping with another message.
 *
 * Used by MessageWidget::draw() methods.
 *
 * @param y              The top of your message.
 * @param messageWidget  A pointer to your message so it doesn't
 *                       check against itself.
 */
bool ObjectWidget::messageOverlap(qreal y, MessageWidget* messageWidget) const
{
    foreach ( MessageWidget* message , m_messages ) {
        if (message == messageWidget) {
            continue;
        }
        const int msgY = message->y();
        const int msgHeight = msgY + message->size().height();
        if (y >= msgY && y <= msgHeight) {
            return true;
        }
    }
    return false;
}

/**
 * Adjusts the end of sequential line to nice position to accomodate
 * the MessageWidgets nicely.
 */
void ObjectWidget::adjustSequentialLineEnd()
{
    qreal lowestMessage = 0;
    foreach ( MessageWidget* message, m_messages ) {
        qreal messageHeight = message->y() + message->size().height();
        if (lowestMessage < messageHeight) {
            lowestMessage = messageHeight;
        }
    }
    m_sequentialLine->setEndOfLine(lowestMessage + ObjectWidget::SequenceLineMargin);
}

/**
 * Reimplemented from NewUMLRectWidget::loadFromXMI to load
 * ObjectWidget from XMI.
 *
 * @note Instance name is loaded from NewUMLRectWidget::loadFromXMI
 */
bool ObjectWidget::loadFromXMI( QDomElement & qElement )
{
    if( !NewUMLRectWidget::loadFromXMI( qElement ) )
        return false;

    QString draw = qElement.attribute( "drawasactor", "0" );
    QString multi = qElement.attribute( "multipleinstance", "0" );
    QString localid = qElement.attribute( "localid", "0" );
    QString decon = qElement.attribute( "decon", "0" );

    setDrawAsActor((bool)draw.toInt());
    setMultipleInstance((bool)draw.toInt());
    setLocalID(STR2ID(localid));
    setShowDestruction((bool)decon.toInt());

    return true;
}

/**
 * Reimplemented from NewUMLRectWidget::saveToXMI to save ObjectWidget
 * data into 'objectwidget' XMI element.
 *
 * @note Instance name is saved by NewUMLRectWidget::saveToXMI
 */
void ObjectWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement objectElement = qDoc.createElement( "objectwidget" );
    NewUMLRectWidget::saveToXMI( qDoc, objectElement );

    objectElement.setAttribute( "drawasactor", m_drawAsActor );
    objectElement.setAttribute( "multipleinstance", m_multipleInstance );
    objectElement.setAttribute( "localid", ID2STR(m_localID) );
    objectElement.setAttribute( "decon", m_showDestruction );
    qElement.appendChild( objectElement );
}

/**
 * Reimplemented from NewUMLRectWidget::paint to draw the object
 * widget.
 *
 * An actor is drawn if m_drawAsActor is true. Otherwise a rectangle
 * is drawn. This information is already predetermined and stored in
 * m_objectWidgetPath.
 */
void ObjectWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    painter->drawPath(m_objectWidgetPath);
}

/**
 * Reimplemented from NewUMLRectWidget::showPropertiesDialog to
 * display the dialog box to change properties of this ObjectWidget.
 */
void ObjectWidget::showPropertiesDialog()
{
    DocWindow *docwindow = UMLApp::app()->getDocWindow();
    docwindow->updateDocumentation(false);

    ClassPropDlg dlg((QWidget*)UMLApp::app(), this);
    if (dlg.exec()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->getDocument()->setModified(true);
    }
}

/**
 * Reimplemented from NewUMLRectWidget::slotMenuSelection to handle
 * some ObjectWidget specific actions.
 */
void ObjectWidget::slotMenuSelection(QAction* action)
{
    // The ListPopupMenu is passed in as action's parent
    ListPopupMenu *menu = qobject_cast<ListPopupMenu*>(action->parent());
    ListPopupMenu::Menu_Type sel = menu->getMenuType(action);

    switch(sel) {
    case ListPopupMenu::mt_Rename_Object:
        {
            bool ok;
            QRegExpValidator validator(QRegExp(".*"), 0);
            QString name = KInputDialog::getText(i18n("Rename Object"),
                                                 i18n("Enter object name:"),
                                                 instanceName(),
                                                 &ok,
                                                 umlScene()->activeView(),
                                                 &validator);
            if (ok) {
                setInstanceName(name);
                UMLApp::app()->getDocument()->setModified(true);
            }
            break;
        }

    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        break;

    case ListPopupMenu::mt_Up:
        tabUp();
        break;

    case ListPopupMenu::mt_Down:
        tabDown();
        break;

    default:
        NewUMLRectWidget::slotMenuSelection(action);
        break;
    }
}

/**
 * Reimplemented from NewUMLRectWidget::updateGeometry to calculate
 * the minimum size for this widget based on whether the Object is
 * drawn as an actor or just a rectangle.
 *
 * Also sets maximum size with height restricted to minimum height, so
 * that the widget can only be widened.
 *
 * @todo Implement Multiple instance drawing.
 */
void ObjectWidget::updateGeometry()
{
    TextItemGroup *grp = textItemGroupAt(0);
    QSizeF minSize = grp->minimumSize();
    if (m_drawAsActor) {
        minSize.rheight() += ObjectWidget::ActorSize.height();
        if (minSize.width() < ObjectWidget::ActorSize.width()) {
            minSize.setWidth(ObjectWidget::ActorSize.width());
        }
    }
    setMinimumSize(minSize);
    setMaximumSize(QSizeF(NewUMLRectWidget::DefaultMaximumSize.width(),
                          minSize.height()));

    NewUMLRectWidget::updateGeometry();
}

/**
 * Reimplemented from NewUMLRectWidget::updateTextItemGroups to update
 * the text content of this widget's text display.
 */
void ObjectWidget::updateTextItemGroups()
{
    TextItemGroup *grp = textItemGroupAt(0);
    grp->setTextItemCount(1);

    TextItem *nameItem = grp->textItemAt(0);
    QString objectText = instanceName() + QLatin1String(" : ") + name();
    nameItem->setText(objectText);

    NewUMLRectWidget::updateTextItemGroups();
}

/**
 * Reimplemented from NewUMLRectWidget::attributeChange
 *
 * To handle SizeHasChanged notification
 * - To align the text.
 * - To update the position of sequential line.
 * - To calculate the actor path if DrawAsActor is true.
 *
 * To handle LineHasChanged and LineColorHasChanged to set the same
 * settings to sequential line.
 *
 * @todo Implement Multiple instance drawing.
 */
QVariant ObjectWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if (change == SizeHasChanged) {
        UMLScene *uScene = umlScene();
        const QSizeF curSize = size();
        const qreal m = margin();
        const QRectF r = rect();

        TextItemGroup *grp = textItemGroupAt(0);
        const QSizeF grpSize = grp->minimumSize();

        // Create/delete Sequential line based on the type of diagram.
        if (uScene) {
            if (uScene->getType() == Uml::dt_Sequence) {
                if (!m_sequentialLine) {
                    m_sequentialLine = new SeqLineWidget(this);
                    m_sequentialLine->setLength(255);
                }
            }
            else {
                delete m_sequentialLine;
                m_sequentialLine = 0;
            }
        }

        // Firstly align the sequential line.
        if (m_sequentialLine) {
            m_sequentialLine->setPos(r.center().x(), r.bottom());
        }

        // Now update text position and also the path.

        // Reset the path
        m_objectWidgetPath = QPainterPath();

        if (m_drawAsActor) {
            QRectF grpRect(+m, curSize.height() - grpSize.height() - 2 * m,
                           curSize.width() - 2  * m, grpSize.height());
            grp->setGroupGeometry(grpRect);

            // NOTE: Copy-pasted from ActorWidget::attributeChange!!

            // Now calculate actorPath
            qreal actorHeight = r.top() - m;
            qreal actorWidth = r.width();

            // Make sure width of actor isn't too much, it looks ugly otherwise.
            if(actorWidth > .5 * actorHeight) {
                actorWidth = .5 * actorHeight;
            }
            //TODO: Probably use above similar approach to limit height.

            QRectF headEllipse;
            headEllipse.setTopLeft(QPointF(.5 * (curSize.width() - actorWidth), m));
            headEllipse.setSize(QSizeF(actorWidth, actorHeight / 3));
            m_objectWidgetPath.addEllipse(headEllipse);

            QLineF bodyLine(.5 * curSize.width(), headEllipse.bottom(),
                            .5 * curSize.width(), (2. / 3.) * actorHeight);
            m_objectWidgetPath.moveTo(bodyLine.p1());
            m_objectWidgetPath.lineTo(bodyLine.p2());

            QLineF leftLeg(.5 * curSize.width(), bodyLine.p2().y(),
                           headEllipse.left(), actorHeight);
            m_objectWidgetPath.moveTo(leftLeg.p1());
            m_objectWidgetPath.lineTo(leftLeg.p2());

            QLineF rightLeg(.5 * curSize.width(), bodyLine.p2().y(),
                            headEllipse.right(), actorHeight);
            m_objectWidgetPath.moveTo(rightLeg.p1());
            m_objectWidgetPath.lineTo(rightLeg.p2());

            QLineF arms(headEllipse.left(), .5 * actorHeight,
                        headEllipse.right(), .5 * actorHeight);
            m_objectWidgetPath.moveTo(arms.p1());
            m_objectWidgetPath.lineTo(arms.p2());
        }
        else {
            // Utilize entire space for text
            grp->setGroupGeometry(r.adjusted(+m, +m, -m, -m));
            m_objectWidgetPath.addRect(r);
        }
    } // End if(change == SizeHasChanged)

    if (m_sequentialLine) {
        if (change == LineColorHasChanged) {
            m_sequentialLine->setLineColor(lineColor());
        }
        else if (change == LineWidthHasChanged) {
            m_sequentialLine->setLineWidth(lineWidth());
        }
    }

    return NewUMLRectWidget::attributeChange(change, oldValue);
}

/**
 * Shifts the object a little higher, provided it is still in diagram
 * limits.
 */
void ObjectWidget::tabUp()
{
    if (canTabUp()) {
        moveBy(0, -size().height());
        adjustSequentialLineEnd();
    }
}

/**
 * Shifts this object a little lower.
 */
void ObjectWidget::tabDown()
{
    moveBy(0, size().height());
    adjustSequentialLineEnd();
}

#include "objectwidget.moc"
