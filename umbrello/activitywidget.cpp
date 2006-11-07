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
#include "activitywidget.h"

// qt includes
#include <qpainter.h>

// kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>

// app includes
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "umlview.h"
#include "listpopupmenu.h"
#include "dialogs/activitydialog.h"

//Added by qt3to4:
#include <QMouseEvent>
#include <QPolygon>

ActivityWidget::ActivityWidget(UMLView * view, ActivityType activityType, Uml::IDType id )
        : UMLWidget(view, id)
{
    UMLWidget::setBaseType( Uml::wt_Activity );
    setActivityType( activityType );
    updateComponentSize();
}

ActivityWidget::~ActivityWidget() {}

void ActivityWidget::draw(QPainter & p, int offsetX, int offsetY) {
    int w = width();
    int h = height();
    switch ( m_ActivityType )
    {
    case Normal :
        UMLWidget::setPen(p);
        if ( UMLWidget::getUseFillColour() ) {
            p.setBrush( UMLWidget::getFillColour() );
        }
        {
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            //int middleX = w / 2;
            int textStartY = (h / 2) - (fontHeight / 2);
            p.drawRoundRect(offsetX, offsetY, w, h, (h * 60) / w, 60);
            p.setPen(Qt::black);
            p.setFont( UMLWidget::getFont() );
            p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + textStartY,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, getName());
        }
        UMLWidget::setPen(p);
        break;
    case Initial :
        UMLWidget::setPen(p);
        p.setBrush( WidgetBase::getLineColor() );
        p.drawEllipse( offsetX, offsetY, w, h );
        break;
    case End :
        UMLWidget::setPen(p);
        p.setBrush( WidgetBase::getLineColor() );
        p.drawEllipse( offsetX, offsetY, w, h );
        p.setBrush( Qt::white );
        p.drawEllipse( offsetX + 1, offsetY + 1, w - 2, h - 2 );
        p.setBrush( WidgetBase::getLineColor() );
        p.drawEllipse( offsetX + 3, offsetY + 3, w - 6, h - 6 );
        break;
    case Branch :
        UMLWidget::setPen(p);
        p.setBrush( UMLWidget::getFillColour() );
        {
            QPolygon array( 4 );
            array[ 0 ] = QPoint( offsetX + w / 2, offsetY );
            array[ 1 ] = QPoint( offsetX + w, offsetY  + h / 2 );
            array[ 2 ] = QPoint( offsetX + w / 2, offsetY + h );
            array[ 3 ] = QPoint( offsetX, offsetY + h / 2 );
            p.drawPolygon( array );
            p.drawPolyline( array );
        }
        break;
    case Fork_DEPRECATED :  // to be removed
        p.fillRect( offsetX, offsetY, width(), height(), QBrush( Qt::darkYellow ));
        break;
    }
    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);
}

QSize ActivityWidget::calculateSize() {
    int width = 10, height = 10;
    if ( m_ActivityType == Normal ) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const int textWidth = fm.width(getName());
        height = fontHeight;
        width = textWidth > ACTIVITY_WIDTH ? textWidth : ACTIVITY_WIDTH;
        height = height > ACTIVITY_HEIGHT ? height : ACTIVITY_HEIGHT;
        width += ACTIVITY_MARGIN * 2;
        height += ACTIVITY_MARGIN * 2;
    } else if ( m_ActivityType == Branch ) {
        width = height = 20;
    }
    return QSize(width, height);
}

ActivityWidget::ActivityType ActivityWidget::getActivityType() const {
    return m_ActivityType;
}

void ActivityWidget::setActivityType( ActivityType activityType ) {
    m_ActivityType = activityType;
    UMLWidget::m_bResizable = (m_ActivityType == Normal);
}

void ActivityWidget::slotMenuSelection(int sel) {
    bool done = false;

    bool ok = false;
    QString name = m_Text;

    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        name = KInputDialog::getText( i18n("Enter Activity Name"), i18n("Enter the name of the new activity:"), m_Text, &ok );
        if( ok && name.length() > 0 )
            m_Text = name;
        done = true;
        break;

    case ListPopupMenu::mt_Properties:
        showProperties();
        done = true;
        break;
    }

    if( !done )
        UMLWidget::slotMenuSelection( sel );
}

bool ActivityWidget::showProperties() {
    DocWindow *docwindow = UMLApp::app()->getDocWindow();
    docwindow->updateDocumentation(false);

    ActivityDialog dialog(m_pView, this);
    bool modified = false;
    if (dialog.exec() && dialog.getChangesMade()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->getDocument()->setModified(true);
        modified = true;
    }

    return modified;
}

bool ActivityWidget::isActivity(WorkToolBar::ToolBar_Buttons tbb,
                                ActivityType& resultType)
{
    bool status = true;
    switch (tbb) {
    case WorkToolBar::tbb_Initial_Activity:
        resultType = Initial;
        break;
    case WorkToolBar::tbb_Activity:
        resultType = Normal;
        break;
    case WorkToolBar::tbb_End_Activity:
        resultType = End;
        break;
    case WorkToolBar::tbb_Branch:
        resultType = Branch;
        break;
    case WorkToolBar::tbb_Fork:
        kError() << "ActivityWidget::isActivity returns Fork_DEPRECATED" << endl;
        resultType = Fork_DEPRECATED;
        break;
    default:
        status = false;
        break;
    }
    return status;
}

void ActivityWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement activityElement = qDoc.createElement( "activitywidget" );
    UMLWidget::saveToXMI( qDoc, activityElement );
    activityElement.setAttribute( "activityname", m_Text );
    activityElement.setAttribute( "documentation", m_Doc );
    activityElement.setAttribute( "activitytype", m_ActivityType );
    qElement.appendChild( activityElement );
}

bool ActivityWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "activityname", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    QString type = qElement.attribute( "activitytype", "1" );
    setActivityType( (ActivityType)type.toInt() );
    return true;
}


#include "activitywidget.moc"

