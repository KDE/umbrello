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
#include "signalwidget.h"

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
// #include "dialogs/signaldialog.h"
#include "listpopupmenu.h"

SignalWidget::SignalWidget(UMLView * view, SignalType signalType, Uml::IDType id)
        : UMLWidget(view, id) {
    UMLWidget::setBaseType(Uml::wt_Signal);
    m_SignalType = signalType;
    m_Text = "Signal";
    updateComponentSize();
}

SignalWidget::~SignalWidget() {}

void SignalWidget::draw(QPainter & p, int offsetX, int offsetY) {
    UMLWidget::setPen(p);
    const int w = width();
    const int h = height();
    switch (m_SignalType)
    {
    case Send :
        if(UMLWidget::getUseFillColour())
            p.setBrush(UMLWidget::getFillColour());
        {
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            int textStartY = (h / 2) - (fontHeight / 2);
            p.drawRoundRect(offsetX, offsetY, w, h, (h*40)/w, (w*40)/h);
            p.setPen(Qt::black);
            QFont font = UMLWidget::getFont();
            font.setBold( false );
            p.setFont( font );
            p.drawText(offsetX + SIGNAL_MARGIN, offsetY + textStartY,
                           w - SIGNAL_MARGIN * 2, fontHeight,
                           Qt::AlignCenter, getName());
            UMLWidget::setPen(p);
        }
        break;
    case Accept :
        p.setBrush( WidgetBase::getLineColor() );
        p.drawEllipse( offsetX, offsetY, w, h );
        break;
    case Time :
        p.setBrush( WidgetBase::getLineColor() );
        p.drawEllipse( offsetX, offsetY, w, h );
        p.setBrush( Qt::white );
        p.drawEllipse( offsetX + 1, offsetY + 1, w - 2, h - 2 );
        p.setBrush( WidgetBase::getLineColor() );
        p.drawEllipse( offsetX + 3, offsetY + 3, w - 6, h - 6 );
        break;
    default:
        kWarning() << "Unknown signal type:" << m_SignalType << endl;
        break;
    }
    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);
}

QSize SignalWidget::calculateSize() {
    int width = 10, height = 10;
    if ( m_SignalType == Send ) {
        const QFontMetrics &fm = getFontMetrics(FT_BOLD);
        const int fontHeight  = fm.lineSpacing();
        int textWidth = fm.width(getName());
        height = fontHeight;

        width = textWidth > SIGNAL_WIDTH?textWidth:SIGNAL_WIDTH;
        height = height > SIGNAL_HEIGHT?height:SIGNAL_HEIGHT;
        width += SIGNAL_MARGIN * 2;
        height += SIGNAL_MARGIN * 2;
    }

    return QSize(width, height);
}

void SignalWidget::setName(const QString &strName) {
    m_Text = strName;
    updateComponentSize();
    adjustAssocs( getX(), getY() );
}

QString SignalWidget::getName() const {
    return m_Text;
}

SignalWidget::SignalType SignalWidget::getSignalType() const {
    return m_SignalType;
}

void SignalWidget::setSignalType( SignalType signalType ) {
    m_SignalType = signalType;
}

bool SignalWidget::showProperties() {
//     DocWindow *docwindow = UMLApp::app()->getDocWindow();
//     docwindow->updateDocumentation(false);
// 
//     SignalDialog dialog(m_pView, this);
//     bool modified = false;
//     if (dialog.exec() && dialog.getChangesMade()) {
//         docwindow->showDocumentation(this, true);
//         UMLApp::app()->getDocument()->setModified(true);
//         modified = true;
//     }
// 
    return true;
}

void SignalWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement signalElement = qDoc.createElement( "signalwidget" );
    UMLWidget::saveToXMI( qDoc, signalElement );
    signalElement.setAttribute( "signalname", m_Text );
    signalElement.setAttribute( "documentation", m_Doc );
    signalElement.setAttribute( "signaltype", m_SignalType );

}

bool SignalWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "signalname", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    QString type = qElement.attribute( "signaltype", "1" );
    m_SignalType = (SignalType)type.toInt();
    return true;
}


#include "signalwidget.moc"

