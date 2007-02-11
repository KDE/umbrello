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
#include "combinedfragmentwidget.h"

// qt includes
#include <qpainter.h>

// kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <kmessagebox.h>

// app includes
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "umlview.h"
#include "listpopupmenu.h"
#include "dialogs/activitydialog.h"
#include "dialog_utils.h"

//Added by qt3to4:
#include <QMouseEvent>
#include <QPolygon>

CombinedFragmentWidget::CombinedFragmentWidget(UMLView * view, CombinedFragmentType combinedfragmentType, Uml::IDType id ) : UMLWidget(view, id)
{
    UMLWidget::setBaseType( Uml::wt_CombinedFragment );
    setCombinedFragmentType( combinedfragmentType );
    updateComponentSize();
}

CombinedFragmentWidget::~CombinedFragmentWidget() {}

void CombinedFragmentWidget::draw(QPainter & p, int offsetX, int offsetY) {
    int w = width();
    int h = height();
    int line_width = 45;
    int old_Y;

    UMLWidget::setPen(p);

     if ( m_CombinedFragment == Ref )
     {
	if ( UMLWidget::getUseFillColour() ) {
		p.setBrush( UMLWidget::getFillColour() );
	}
     }
{
	const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	const int fontHeight  = fm.lineSpacing();
	const QString combined_fragment_value =  getName();
	int textStartY = (h / 2) - (fontHeight / 2);
	p.drawRect(offsetX, offsetY, w, h );

	p.setPen(Qt::black);
	p.setFont( UMLWidget::getFont() );
        QString temp = "loop";

    switch ( m_CombinedFragment )
    {
        case Ref :
		p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY + textStartY, w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignCenter, combined_fragment_value);
	
		p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY , w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, "ref");
        break;

        case Opt :
		p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY ,
			w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, "opt");
        break;

        case Break :
		p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY ,
			w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, "break");
        break;

        case Loop :
                if (combined_fragment_value != "-")
                {
                     temp += " [" + combined_fragment_value + "]";
                     line_width += (combined_fragment_value.size() + 2) * 8;
                }
		p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY ,w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, temp);

        break;

        case Neg :
		p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY ,
			w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, "neg");
        break;

        case Crit :
		p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY ,
			w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, "critical");
        break;

        case Ass :
		p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY ,
			w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, "assert");
        break;

        case Alt :
                p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY ,
			w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, "alt");
                // dash line
                m_dashLine->setX(getX());
                old_Y = m_dashLine->getYMin();
                m_dashLine->setYMin(getY());
                m_dashLine->setYMax(getY() + getHeight());
                m_dashLine->setY(getY() + m_dashLine->getY() - old_Y);
                m_dashLine->setSize(w, 0);
                
        break;

        case Par :
                p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY ,
			w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, "parallel");
                // dash line
                m_dashLine->setX(getX());
                old_Y = m_dashLine->getYMin();
                m_dashLine->setYMin(getY());
                m_dashLine->setYMax(getY() + getHeight());
                m_dashLine->setY(getY() + m_dashLine->getY() - old_Y);
                m_dashLine->setSize(w, 0);
                
        break;

	default : break;
    }
}
    p.setPen(Qt::red);
    p.drawLine(offsetX,      offsetY + 20, offsetX + line_width, offsetY + 20);
    p.drawLine(offsetX + line_width, offsetY + 20, offsetX + line_width + 10, offsetY + 10);
    p.drawLine(offsetX + line_width + 10, offsetY + 10, offsetX + line_width + 10, offsetY);

    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);
}

QSize CombinedFragmentWidget::calculateSize() {
    int width = 10, height = 10;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int textWidth = fm.width(getName());
    height = fontHeight;
    width = textWidth + 60 > COMBINED_FRAGMENT_WIDTH ? textWidth + 60: COMBINED_FRAGMENT_WIDTH;
    if ( m_CombinedFragment == Loop )
         width += textWidth * 0.4;
    height = height > COMBINED_FRAGMENT_HEIGHT ? height : COMBINED_FRAGMENT_HEIGHT;
    width += COMBINED_FRAGMENT_MARGIN * 2;
    height += COMBINED_FRAGMENT_MARGIN * 2;
    return QSize(width, height);
}

CombinedFragmentWidget::CombinedFragmentType CombinedFragmentWidget::getCombinedFragmentType() const {
    return m_CombinedFragment;
}

void CombinedFragmentWidget::setCombinedFragmentType( CombinedFragmentType combinedfragmentType ) {

    m_CombinedFragment = combinedfragmentType;
    UMLWidget::m_bResizable =  true ; //(m_CombinedFragment == Normal);

    // creates a dash line if the combined fragment type is alternative or parallel
    if(m_CombinedFragment == Alt || m_CombinedFragment == Par)
    {
        m_dashLine = new FloatingDashLineWidget(m_pView);
        if(m_CombinedFragment == Alt)
        {
            m_dashLine->setText("else");
        }
        m_dashLine->setX(getX());
        m_dashLine->setYMin(getY());
        m_dashLine->setYMax(getY() + getHeight());
        m_dashLine->setY(getY() + getHeight() / 2);
        m_dashLine->setSize(getWidth(), 0);
        m_pView->setupNewWidget(m_dashLine);
    }
}

CombinedFragmentWidget::CombinedFragmentType CombinedFragmentWidget::getCombinedFragmentType(QString type) const {
    if(type == "Reference")
        return (CombinedFragmentWidget::Ref);
    if(type == "Option")
        return (CombinedFragmentWidget::Opt);
    if(type == "Break")
        return (CombinedFragmentWidget::Break);
    if(type == "Loop")
        return (CombinedFragmentWidget::Loop);
    if(type == "Negative")
        return (CombinedFragmentWidget::Neg);
    if(type == "Critical")
        return (CombinedFragmentWidget::Crit);
    if(type == "Assertion")
        return (CombinedFragmentWidget::Ass);
    if(type == "Alternative")
        return (CombinedFragmentWidget::Alt);
    if(type == "Parallel")
        return (CombinedFragmentWidget::Par);

}

void CombinedFragmentWidget::setCombinedFragmentType( QString combinedfragmentType ) {

    setCombinedFragmentType(getCombinedFragmentType(combinedfragmentType) );
}

bool CombinedFragmentWidget::showProperties() {
//     DocWindow *docwindow = UMLApp::app()->getDocWindow();
//     docwindow->updateDocumentation(false);
// 
//     ActivityDialog dialog(m_pView, this);
//     bool modified = false;
//     if (dialog.exec() && dialog.getChangesMade()) {
//         docwindow->showDocumentation(this, true);
//         UMLApp::app()->getDocument()->setModified(true);
//         modified = true;
//     }

    return true;
}

// bool CombinedFragmentWidget::isCombinedFragment(WorkToolBar::ToolBar_Buttons tbb, CombinedFragmentType& resultType)
// {
//     bool status = true;
//     switch (tbb) {
// ;
//     case WorkToolBar::tbb_Seq_Combined_Fragment:
//         resultType = m_CombinedFragment;
//         break;
//     default:
//         status = false;
//         break;
//     }
//     return status;
// }

void CombinedFragmentWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement combinedFragmentElement = qDoc.createElement( "combinedFragmentwidget" );
    UMLWidget::saveToXMI( qDoc, combinedFragmentElement );
    combinedFragmentElement.setAttribute( "combinedFragmentname", m_Text );
    combinedFragmentElement.setAttribute( "documentation", m_Doc );
    combinedFragmentElement.setAttribute( "CombinedFragmenttype", m_CombinedFragment );
    qElement.appendChild( combinedFragmentElement );
}

bool CombinedFragmentWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "combinedFragmentname", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    QString type = qElement.attribute( "CombinedFragmenttype", "");
    //KMessageBox::error(0, i18n(" type %1",type),i18n("type"));
    setCombinedFragmentType( (CombinedFragmentType)type.toInt() );
    return true;
}


#include "combinedfragmentwidget.moc"

