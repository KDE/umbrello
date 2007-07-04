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
//#include <pen.h>

// kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <cmath>

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

    // Only for the final activity
    float x;
    float y;
    QPen pen = p.pen();

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
            int textStartY = (h / 2) - (fontHeight / 2);
            p.drawRoundRect(offsetX, offsetY, w, h, (h * 60) / w, 60);
            p.setPen(Qt::black);
            p.setFont( UMLWidget::getFont() );
            p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + textStartY,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, getName());
        }
        break;

    case Initial :
        p.setPen( QPen(m_LineColour, 1) );
        p.setBrush( WidgetBase::getLineColor() );
        p.drawEllipse( offsetX, offsetY, w, h );
        break;

    case Final :

        UMLWidget::setPen(p);
        p.setBrush( Qt::white );
        pen.setWidth( 2 );
        pen.setColor ( Qt::red );
        p.setPen( pen );
        p.drawEllipse( offsetX, offsetY, w, h );
        x = offsetX + w/2 ;
        y = offsetY + h/2 ;
        {
            const float w2 = 0.7071 * (float)w / 2.0;
            p.drawLine(x - w2 + 1, y - w2 + 1, x + w2, y + w2);
            p.drawLine(x + w2 - 1, y - w2 + 1, x - w2, y + w2);
        }
        break;

    case End :
        p.setPen( QPen(m_LineColour, 1) );
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

    case Invok :
        UMLWidget::setPen(p);
        if ( UMLWidget::getUseFillColour() ) {
            p.setBrush( UMLWidget::getFillColour() );
        }
        {
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            int textStartY = (h / 2) - (fontHeight / 2);
            p.drawRoundRect(offsetX, offsetY, w, h, (h * 60) / w, 60);
            p.setPen(Qt::black);
            p.setFont( UMLWidget::getFont() );
            p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + textStartY,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, getName());

        }
        x = offsetX + w - (w/5);
        y = offsetY + h - (h/3);

        p.drawLine(x,      y,      x,      y + 20);
        p.drawLine(x - 10, y + 10, x + 10, y + 10);
        p.drawLine(x - 10, y + 10, x - 10, y + 20);
        p.drawLine(x + 10, y + 10, x + 10, y + 20);
        break;

    case Param :
        UMLWidget::setPen(p);
        if ( UMLWidget::getUseFillColour() ) {
            p.setBrush( UMLWidget::getFillColour() );
        }
        {
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            QString preCond= "<<precondition>> "+getPreText();
            QString postCond= "<<postcondition>> "+getPostText();
            int textStartY = (h / 2) - (fontHeight / 2);
            p.drawRoundRect(offsetX, offsetY, w, h, (h * 60) / w, 60);
            p.setPen(Qt::black);
            p.setFont( UMLWidget::getFont() );
            p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + fontHeight + 10,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, preCond);
            p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + fontHeight * 2 + 10,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, postCond);
            p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + (fontHeight / 2),
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, getName());
        }

        break;

    }
    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);
}

void ActivityWidget::constrain(int& width, int& height) {
    if (m_ActivityType == Normal || m_ActivityType == Invok || m_ActivityType == Param) {
        QSize minSize = calculateSize();
        if (width < minSize.width())
            width = minSize.width();
        if (height < minSize.height())
            height = minSize.height();
        return;
    }
    if (width > height)
        width = height;
    else if (height > width)
        height = width;
    if (m_ActivityType == Branch) {
        if (width < 20) {
            width = 20;
            height = 20;
        } else if (width > 50) {
            width = 50;
            height = 50;
        }
    } else {
        if (width < 15) {
            width = 15;
            height = 15;
        } else if (width > 30) {
            width = 30;
            height = 30;
        }
    }
}

QSize ActivityWidget::calculateSize() {
    int width, height;
    if ( m_ActivityType == Normal || m_ActivityType == Invok || m_ActivityType == Param ) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();

        int textWidth = fm.width(getName());
        height = fontHeight;
        height = height > ACTIVITY_HEIGHT ? height : ACTIVITY_HEIGHT;
        height += ACTIVITY_MARGIN * 2;

        textWidth = textWidth > ACTIVITY_WIDTH ? textWidth : ACTIVITY_WIDTH;

        if (m_ActivityType == Invok) {
             height += 40;
        } else if (m_ActivityType == Param) {
            QString maxSize;

            maxSize = getName().length() > getPostText().length() ? getName() : getPostText();
            maxSize = maxSize.length() > getPreText().length() ? maxSize : getPreText();

            textWidth = fm.width(maxSize);
            textWidth = textWidth + 50;
            height += 100;
        }

        width = textWidth > ACTIVITY_WIDTH ? textWidth : ACTIVITY_WIDTH;

        width += ACTIVITY_MARGIN * 4;

    } else {
        width = height = 20;
    }
    return QSize(width, height);
}

ActivityWidget::ActivityType ActivityWidget::getActivityType() const {
    return m_ActivityType;
}

void ActivityWidget::setActivityType( ActivityType activityType ) {
    m_ActivityType = activityType;
    updateComponentSize();
    UMLWidget::m_bResizable = true;
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

void ActivityWidget::showProperties() {
    DocWindow *docwindow = UMLApp::app()->getDocWindow();
    docwindow->updateDocumentation(false);

    ActivityDialog dialog(m_pView, this);
    if (dialog.exec() && dialog.getChangesMade()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->getDocument()->setModified(true);
    }
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
    case WorkToolBar::tbb_Final_Activity:
        resultType = Final;
        break;
    case WorkToolBar::tbb_Branch:
        resultType = Branch;
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
    activityElement.setAttribute( "precondition", preText );
    activityElement.setAttribute( "postcondition", postText );
    activityElement.setAttribute( "activitytype", m_ActivityType );
    qElement.appendChild( activityElement );
}

bool ActivityWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "activityname", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    preText = qElement.attribute( "precondition", "" );
    postText = qElement.attribute( "postcondition", "" );

    QString type = qElement.attribute( "activitytype", "1" );
    setActivityType( (ActivityType)type.toInt() );

    return true;
}

void ActivityWidget::setPreText(const QString& aPreText)
{
    preText=aPreText;
    updateComponentSize();
    adjustAssocs( getX(), getY() );
}

QString ActivityWidget::getPreText()
{
    return preText;
}

void ActivityWidget::setPostText(const QString& aPostText)
{
    postText=aPostText;
    updateComponentSize();
    adjustAssocs( getX(), getY() );
}

QString ActivityWidget::getPostText()
{
    return postText;
}
#include "activitywidget.moc"

