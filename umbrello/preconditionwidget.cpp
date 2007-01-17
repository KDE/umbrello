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
#include "preconditionwidget.h"

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

PreconditionWidget::PreconditionWidget(UMLView * view, Uml::IDType id )
        : UMLWidget(view, id)
{
    UMLWidget::setBaseType( Uml::wt_Precondition );
    updateComponentSize();
}

PreconditionWidget::~PreconditionWidget() {}

void PreconditionWidget::draw(QPainter & p, int offsetX, int offsetY) {
    int w = width();
    int h = height();
   
    UMLWidget::setPen(p);
    if ( UMLWidget::getUseFillColour() ) {
        p.setBrush( UMLWidget::getFillColour() );
    }
    {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
	const QString precondition_value = "{ " + getName() + " }";
        //int middleX = w / 2;
        int textStartY = (h / 2) - (fontHeight / 2);
        p.drawRoundRect(offsetX, offsetY, w, h, (h * 60) / w, 60);
        p.setPen(Qt::black);
        p.setFont( UMLWidget::getFont() );
        p.drawText(offsetX + PRECONDITION_MARGIN, offsetY + textStartY,
                       w - PRECONDITION_MARGIN * 2, fontHeight, Qt::AlignCenter, precondition_value);
    }
    UMLWidget::setPen(p);
    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);
}

QSize PreconditionWidget::calculateSize() {
    int width = 10, height = 10;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int textWidth = fm.width(getName()) + 25;
    height = fontHeight;
    width = textWidth > PRECONDITION_WIDTH ? textWidth : PRECONDITION_WIDTH;
    height = height > PRECONDITION_HEIGHT ? height : PRECONDITION_HEIGHT;
    width += PRECONDITION_MARGIN * 2;
    height += PRECONDITION_MARGIN * 2;
   
    return QSize(width, height);
}


void PreconditionWidget::slotMenuSelection(int sel) {
    bool done = false;

    bool ok = false;
    QString name = m_Text;

    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        name = KInputDialog::getText( i18n("Enter Precondition Name"), i18n("Enter the precondition :"), m_Text, &ok );
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

bool PreconditionWidget::showProperties() {
    /*DocWindow *docwindow = UMLApp::app()->getDocWindow();
    docwindow->updateDocumentation(false);

    ActivityDialog dialog(m_pView, this);
    bool modified = false;
    if (dialog.exec() && dialog.getChangesMade()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->getDocument()->setModified(true);
        modified = true;
    }

    return modified;*/
    return true;
}

void PreconditionWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement preconditionElement = qDoc.createElement( "preconditionwidget" );
    UMLWidget::saveToXMI( qDoc, preconditionElement );
    preconditionElement.setAttribute( "preconditionname", m_Text );
    preconditionElement.setAttribute( "documentation", m_Doc );
    qElement.appendChild( preconditionElement );
}

bool PreconditionWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "preconditionname", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    return true;
}


#include "preconditionwidget.moc"

