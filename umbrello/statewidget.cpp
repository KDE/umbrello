/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "statewidget.h"

// qt includes
#include <qevent.h>

// kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>

// app includes
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "umlwidget.h"
#include "umlview.h"
#include "dialogs/statedialog.h"
#include "listpopupmenu.h"

StateWidget::StateWidget(UMLView * view, StateType stateType, Uml::IDType id)
        : UMLWidget(view, id) {
    UMLWidget::setBaseType(Uml::wt_State);
    m_StateType = stateType;
    m_Text = "State";
    updateComponentSize();
}

StateWidget::~StateWidget() {}

void StateWidget::draw(QPainter & p, int offsetX, int offsetY) {
    UMLWidget::setPen(p);
    const int w = width();
    const int h = height();
    switch (m_StateType)
    {
    case Normal :
        if(UMLWidget::getUseFillColour())
            p.setBrush(UMLWidget::getFillColour());
        {
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            int textStartY = (h / 2) - (fontHeight / 2);
            const int count = m_Activities.count();
            if( count == 0 ) {
                p.drawRoundRect(offsetX, offsetY, w, h, (h*40)/w, (w*40)/h);
                p.setPen(Qt::black);
                QFont font = UMLWidget::getFont();
                font.setBold( false );
                p.setFont( font );
                p.drawText(offsetX + STATE_MARGIN, offsetY + textStartY,
                           w - STATE_MARGIN * 2, fontHeight,
                           Qt::AlignCenter, getName());
                UMLWidget::setPen(p);
            } else {
                p.drawRoundRect(offsetX, offsetY, w, h, (h*40)/w, (w*40)/h);
                textStartY = offsetY + STATE_MARGIN;
                p.setPen(Qt::black);
                QFont font = UMLWidget::getFont();
                font.setBold( true );
                p.setFont( font );
                p.drawText(offsetX + STATE_MARGIN, textStartY, w - STATE_MARGIN * 2,
                           fontHeight, Qt::AlignCenter, getName());
                font.setBold( false );
                p.setFont( font );
                UMLWidget::setPen(p);
                int linePosY = textStartY + fontHeight;

                QStringList::Iterator end(m_Activities.end());
                for( QStringList::Iterator it(m_Activities.begin()); it != end; ++it ) {
                    textStartY += fontHeight;
                    p.drawLine( offsetX, linePosY, offsetX + w - 1, linePosY );
                    p.setPen(Qt::black);
                    p.drawText(offsetX + STATE_MARGIN, textStartY, w - STATE_MARGIN * 2 - 1,
                               fontHeight, Qt::AlignCenter, *it);
                    UMLWidget::setPen(p);
                    linePosY += fontHeight;
                }//end for
            }//end else
        }
        break;
    case Initial :
        p.setBrush( WidgetBase::getLineColor() );
        p.drawEllipse( offsetX, offsetY, w, h );
        break;
    case End :
        p.setBrush( WidgetBase::getLineColor() );
        p.drawEllipse( offsetX, offsetY, w, h );
        p.setBrush( Qt::white );
        p.drawEllipse( offsetX + 1, offsetY + 1, w - 2, h - 2 );
        p.setBrush( WidgetBase::getLineColor() );
        p.drawEllipse( offsetX + 3, offsetY + 3, w - 6, h - 6 );
        break;
    default:
        kWarning() << "Unknown state type:" << m_StateType << endl;
        break;
    }
    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);
}

QSize StateWidget::calculateSize() {
    int width = 10, height = 10;
    if ( m_StateType == Normal ) {
        const QFontMetrics &fm = getFontMetrics(FT_BOLD);
        const int fontHeight  = fm.lineSpacing();
        int textWidth = fm.width(getName());
        const int count = m_Activities.count();
        height = fontHeight;
        if( count > 0 ) {
            height = fontHeight * ( count + 1);

            QStringList::Iterator end(m_Activities.end());
            for( QStringList::Iterator it(m_Activities.begin()); it != end; ++it ) {
                int w = fm.width( *it );
                if( w > textWidth )
                    textWidth = w;
            }//end for
        }//end if
        width = textWidth > STATE_WIDTH?textWidth:STATE_WIDTH;
        height = height > STATE_HEIGHT?height:STATE_HEIGHT;
        width += STATE_MARGIN * 2;
        height += STATE_MARGIN * 2;
    }

    return QSize(width, height);
}

void StateWidget::setName(const QString &strName) {
    m_Text = strName;
    updateComponentSize();
    adjustAssocs( getX(), getY() );
}

QString StateWidget::getName() const {
    return m_Text;
}

StateWidget::StateType StateWidget::getStateType() const {
    return m_StateType;
}

void StateWidget::setStateType( StateType stateType ) {
    m_StateType = stateType;
}

void StateWidget::slotMenuSelection(int sel) {
    bool done = false;
    bool ok = false;
    QString name = getName();

    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        name = KInputDialog::getText( i18n("Enter State Name"), i18n("Enter the name of the new state:"), getName(), &ok );
        if( ok && name.length() > 0 )
            setName( name );
        done = true;
        break;

    case ListPopupMenu::mt_Properties:
        showProperties();
        done = true;
        break;
    case ListPopupMenu::mt_New_Activity:
        name = KInputDialog::getText( i18n("Enter Activity"), i18n("Enter the name of the new activity:"), i18n("new activity"), &ok );
        if( ok && name.length() > 0 )
            addActivity( name );
        done = true;
        break;
    }

    if( !done )
        UMLWidget::slotMenuSelection( sel );
}

bool StateWidget::addActivity( const QString &activity ) {
    m_Activities.append( activity );
    updateComponentSize();
    return true;
}

bool StateWidget::removeActivity( const QString &activity ) {
    int index = - 1;
    if( ( index = m_Activities.findIndex( activity ) ) == -1 )
        return false;
    m_Activities.remove( m_Activities.at( index ) );
    updateComponentSize();
    return true;
}

void StateWidget::setActivities( QStringList & list ) {
    m_Activities = list;
    updateComponentSize();
}

QStringList & StateWidget::getActivityList() {
    return m_Activities;
}

bool StateWidget::renameActivity( const QString &activity, const QString &newName ) {
    int index = - 1;
    if( ( index = m_Activities.findIndex( activity ) ) == -1 )
        return false;
    m_Activities[ index ] = newName;
    return true;
}

void StateWidget::showProperties() {
    DocWindow *docwindow = UMLApp::app()->getDocWindow();
    docwindow->updateDocumentation(false);

    StateDialog dialog(m_pView, this);
    if (dialog.exec() && dialog.getChangesMade()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->getDocument()->setModified(true);
    }
}

bool StateWidget::isState(WorkToolBar::ToolBar_Buttons tbb, StateType& resultType)
{
    bool status = true;
    switch (tbb) {
    case WorkToolBar::tbb_Initial_State:
        resultType = Initial;
        break;
    case WorkToolBar::tbb_State:
        resultType = Normal;
        break;
    case WorkToolBar::tbb_End_State:
        resultType = End;
        break;
    default:
        status = false;
        break;
    }
    return status;
}

void StateWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement stateElement = qDoc.createElement( "statewidget" );
    UMLWidget::saveToXMI( qDoc, stateElement );
    stateElement.setAttribute( "statename", m_Text );
    stateElement.setAttribute( "documentation", m_Doc );
    stateElement.setAttribute( "statetype", m_StateType );
    //save states activities
    QDomElement activitiesElement = qDoc.createElement( "Activities" );

    QStringList::Iterator end(m_Activities.end());
    for( QStringList::Iterator it(m_Activities.begin()); it != end; ++it ) {
        QDomElement tempElement = qDoc.createElement( "Activity" );
        tempElement.setAttribute( "name", *it );
        activitiesElement.appendChild( tempElement );
    }//end for
    stateElement.appendChild( activitiesElement );
    qElement.appendChild( stateElement );
}

bool StateWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "statename", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    QString type = qElement.attribute( "statetype", "1" );
    m_StateType = (StateType)type.toInt();
    //load states activities
    QDomNode node = qElement.firstChild();
    QDomElement tempElement = node.toElement();
    if( !tempElement.isNull() && tempElement.tagName() == "Activities" ) {
        QDomNode node = tempElement.firstChild();
        QDomElement activityElement = node.toElement();
        while( !activityElement.isNull() ) {
            if( activityElement.tagName() == "Activity" ) {
                QString name = activityElement.attribute( "name", "" );
                if( !name.isEmpty() )
                    m_Activities.append( name );
            }//end if
            node = node.nextSibling();
            activityElement = node.toElement();
        }//end while
    }//end if
    return true;
}


#include "statewidget.moc"

