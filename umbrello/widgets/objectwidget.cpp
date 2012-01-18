/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "objectwidget.h"

// system includes
#include <QtCore/QPointer>
#include <QtGui/QPainter>
#include <QtGui/QValidator>
#include <klocale.h>
#include <kinputdialog.h>

// local includes
#include "debug_utils.h"
#include "objectwidgetcontroller.h"
#include "seqlinewidget.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "umlobject.h"
#include "listpopupmenu.h"
#include "docwindow.h"
#include "classpropdlg.h"

/**
 * The number of pixels margin between the lowest message
 * and the bottom of the vertical line
 */
static const int sequenceLineMargin = 20;

ObjectWidget::ObjectWidget(UMLScene * scene, UMLObject *o, Uml::IDType lid)
  : UMLWidget(scene, WidgetBase::wt_Object, o)
{
    init();
    if( lid != Uml::id_None )
        m_nLocalID = lid;
}

void ObjectWidget::init()
{
    m_nLocalID = Uml::id_None;
    m_InstanceName = "";
    m_multipleInstance = false;
    m_drawAsActor = false;
    m_showDestruction = false;
    if( m_scene != NULL && m_scene->type() == Uml::DiagramType::Sequence ) {
        m_pLine = new SeqLineWidget( m_scene, this );

        //Sets specific widget controller for sequence diagrams
        delete m_widgetController;
        m_widgetController = new ObjectWidgetController(this);
    } else {
        m_pLine = NULL;
    }
}

ObjectWidget::~ObjectWidget()
{
}

void ObjectWidget::paint(QPainter & p , int offsetX, int offsetY)
{
    if ( m_drawAsActor )
        drawActor( p, offsetX, offsetY );
    else
        drawObject( p, offsetX, offsetY );

    setPenFromSettings(p);
    if(m_selected)
        drawSelected(&p, offsetX, offsetY);
}

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
                    m_InstanceName,
                    &ok,
                    m_scene,
                    validator);
            if (ok) {
                m_InstanceName = name;
                updateComponentSize();
                moveEvent( 0 );
                update();
                UMLApp::app()->document()->setModified(true);
            }
            delete validator;
            break;
        }
    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        updateComponentSize();
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

UMLSceneSize ObjectWidget::minimumSize()
{
    int width, height;
    const QFontMetrics &fm = getFontMetrics(FT_UNDERLINE);
    const int fontHeight  = fm.lineSpacing();
    const QString t = m_InstanceName + " : " + m_pObject->name();
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

void ObjectWidget::setDrawAsActor( bool drawAsActor )
{
    m_drawAsActor = drawAsActor;
    updateComponentSize();
}

void ObjectWidget::setMultipleInstance(bool multiple)
{
    //make sure only calling this in relation to an object on a collab. diagram
    if(m_scene->type() != Uml::DiagramType::Collaboration)
        return;
    m_multipleInstance = multiple;
    updateComponentSize();
    update();
}

bool ObjectWidget::activate(IDChangeLog* ChangeLog /*= 0*/)
{
    if (! UMLWidget::activate(ChangeLog))
        return false;
    if (m_showDestruction && m_pLine)
        m_pLine->setupDestructionBox();
    moveEvent(0);
    return true;
}

void ObjectWidget::setX( int x )
{
    UMLWidget::setX(x);
    moveEvent(0);
}

void ObjectWidget::setY( int y )
{
    UMLWidget::setY(y);
    moveEvent(0);
}

void ObjectWidget::moveEvent(QMoveEvent *m)
{
    Q_UNUSED(m)
    emit sigWidgetMoved( m_nLocalID );
    if (m_pLine) {
        const int x = getX();    // for debugging: gdb has a problem evaluating getX() etc
        const int w = width();
        const int y = getY();
        const int h = height();
        m_pLine->setStartPoint(x + w / 2, y + h);
    }
}

void ObjectWidget::slotColorChanged(Uml::IDType /*viewID*/)
{
    UMLWidget::setFillColor( m_scene->fillColor() );
    UMLWidget::setLineColor( m_scene->lineColor() );

    if( m_pLine)
        m_pLine->setPen( QPen( UMLWidget::lineColor(), UMLWidget::lineWidth(), Qt::DashLine ) );
}

void ObjectWidget::cleanup()
{
    UMLWidget::cleanup();
    if( m_pLine ) {
        m_pLine -> cleanup();
        delete m_pLine;
    }
}

void ObjectWidget::showPropertiesDialog()
{
    DocWindow *docwindow = UMLApp::app()->docWindow();
    docwindow->updateDocumentation(false);
    QPointer<ClassPropDlg> dlg = new ClassPropDlg((QWidget*)UMLApp::app(), this);
    if (dlg->exec()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
    }
    dlg->close();
    delete dlg;
}

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
        p.setBrush( m_scene->viewport()->palette().color(QPalette::Background) );
    const int w = width();
    const int h = height();

    const QString t = m_InstanceName + " : " + m_pObject->name();
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
    QString t = m_InstanceName + " : " + m_pObject->name();
    p.drawText(offsetX + A_MARGIN, offsetY + textStartY,
               w - A_MARGIN * 2, fontHeight, Qt::AlignCenter, t);
}

void ObjectWidget::tabUp()
{
    int newY = getY() - height();
    if (newY < topMargin())
        newY = topMargin();
    setY( newY );
    moveEvent( 0 );
    adjustAssocs( getX(), newY);
}

void ObjectWidget::tabDown()
{
    int newY = getY() + height();
    setY( newY );
    moveEvent( 0 );
    adjustAssocs( getX(), newY);
}

int ObjectWidget::topMargin()
{
    return 80 - height();
}

bool ObjectWidget::canTabUp()
{
    int y = getY();
    return (y > topMargin());
}

void ObjectWidget::setShowDestruction( bool bShow )
{
    m_showDestruction = bShow;
    if( m_pLine )
        m_pLine -> setupDestructionBox();
}

void ObjectWidget::setEndLine(int yPosition)
{
    m_pLine->setEndOfLine(yPosition);
}

int ObjectWidget::getEndLineY()
{
    int y = this -> getY() + getHeight();
    if( m_pLine)
        y += m_pLine -> getLineLength();
    if ( m_showDestruction )
        y += 10;
    return y;
}

void ObjectWidget::messageAdded(MessageWidget* message)
{
    if ( messageWidgetList.count(message) ) {
        uError() << message->name() << ": duplicate entry !";
        return ;
    }
    messageWidgetList.append(message);
}

void ObjectWidget::messageRemoved(MessageWidget* message)
{
    if ( messageWidgetList.removeAll(message) == false ) {
        uError() << message->name() << ": missing entry !";
        return ;
    }
}

void ObjectWidget::slotMessageMoved()
{
    int lowestMessage = 0;
    foreach ( MessageWidget* message, messageWidgetList ) {
        int messageHeight = message->getY() + message->getHeight();
        if (lowestMessage < messageHeight) {
            lowestMessage = messageHeight;
        }
    }
    m_pLine->setEndOfLine(lowestMessage + sequenceLineMargin);
}

bool ObjectWidget::messageOverlap(int y, MessageWidget* messageWidget)
{
    foreach ( MessageWidget* message , messageWidgetList ) {
        const int msgY = message->getY();
        const int msgHeight = msgY + message->getHeight();
        if (y >= msgY && y <= msgHeight && message != messageWidget) {
            return true;
        }
    }
    return false;
}

SeqLineWidget *ObjectWidget::getSeqLine()
{
    return m_pLine;
}

void ObjectWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement objectElement = qDoc.createElement( "objectwidget" );
    UMLWidget::saveToXMI( qDoc, objectElement );
    objectElement.setAttribute( "drawasactor", m_drawAsActor );
    objectElement.setAttribute( "multipleinstance", m_multipleInstance );
    objectElement.setAttribute( "localid", ID2STR(m_nLocalID) );
    objectElement.setAttribute( "decon", m_showDestruction );
    qElement.appendChild( objectElement );
}

bool ObjectWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_InstanceName = qElement.attribute( "instancename", "" );
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
