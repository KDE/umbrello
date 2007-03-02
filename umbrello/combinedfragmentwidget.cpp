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
#include <qstring.h>

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
                // dash lines
                for(QList<FloatingDashLineWidget*>::iterator it=m_dashLines->begin() ; it!=m_dashLines->end() ; it++) {
                    (*it)->setX(getX());
                    old_Y = (*it)->getYMin();
                    (*it)->setYMin(getY());
                    (*it)->setYMax(getY() + getHeight());
                    (*it)->setY(getY() + (*it)->getY() - old_Y);
                    (*it)->setSize(w, 0);
                }
                
        break;

        case Par :
                p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY ,
			w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, "parallel");
                // dash lines
                for(QList<FloatingDashLineWidget*>::iterator it=m_dashLines->begin() ; it!=m_dashLines->end() ; it++) {
                    (*it)->setX(getX());
                    old_Y = (*it)->getYMin();
                    (*it)->setYMin(getY());
                    (*it)->setYMax(getY() + getHeight());
                    (*it)->setY(getY() + (*it)->getY() - old_Y);
                    (*it)->setSize(w, 0);
                }
                
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
        m_dashLines = new QList<FloatingDashLineWidget*>();
        m_dashLines->push_back(new FloatingDashLineWidget(m_pView));
        if(m_CombinedFragment == Alt)
        {
            m_dashLines->back()->setText("else");
        }
        m_dashLines->back()->setX(getX());
        m_dashLines->back()->setYMin(getY());
        m_dashLines->back()->setYMax(getY() + getHeight());
        m_dashLines->back()->setY(getY() + getHeight() / 2);
        m_dashLines->back()->setSize(getWidth(), 0);
        m_pView->setupNewWidget(m_dashLines->back());
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

void CombinedFragmentWidget::askNameForWidgetType(UMLWidget* &targetWidget, const QString& dialogTitle,
                      const QString& dialogPrompt, const QString& defaultName) {

    bool pressedOK = false;
    const QStringList list = QStringList() << "Reference" << "Option" << "Break" << "Loop" << "Negative" << "Critical" << "Assertion" << "Alternative" << "Parallel" ;
    const QStringList select = QStringList() << "Reference" << "Option" << "Break" << "Loop" << "Negative" << "Critical" << "Assertion" << "Alternative" << "Parallel" ;
    QStringList result = KInputDialog::getItemList (dialogTitle, dialogPrompt, list, select, false, &pressedOK, UMLApp::app());

    if (pressedOK) {
        QString type = result.join("");
        dynamic_cast<CombinedFragmentWidget*>(targetWidget)->setCombinedFragmentType(type);
        if (type == "Reference")
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the name of the diagram referenced"), i18n("Enter the name of the diagram referenced"), i18n("Diagram name"));
        if (type == "Loop")
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the guard of the loop"), i18n("Enter the guard of the loop"), i18n("-"));
    } else {
        targetWidget->cleanup();
        delete targetWidget;
        targetWidget = NULL;
    }
}

void CombinedFragmentWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement combinedFragmentElement = qDoc.createElement( "combinedFragmentwidget" );
    UMLWidget::saveToXMI( qDoc, combinedFragmentElement );
    combinedFragmentElement.setAttribute( "combinedFragmentname", m_Text );
    combinedFragmentElement.setAttribute( "documentation", m_Doc );
    combinedFragmentElement.setAttribute( "CombinedFragmenttype", m_CombinedFragment );

    // save the corresponding floating dash lines
    /*QString dashlineId("dashlineId");
    int i=1;
    for (list<FloatingDashLineWidget*>::iterator it = m_dashLines->begin() ; it != m_dashLines->end() ; it++) {
        combinedFragmentElement.setAttribute( dashlineId.append(i), ID2STR((*it)->getID()) );
        (*it) -> saveToXMI( qDoc, combinedFragmentElement );
        i++;
    }*/

    qElement.appendChild( combinedFragmentElement );
}

bool CombinedFragmentWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "combinedFragmentname", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    QString type = qElement.attribute( "CombinedFragmenttype", "");

    // TODO : load floating dash lines of the combined fragment
    /*QString dashlineId("dashlineId");
    int i=1;
    for (list<FloatingDashLineWidget>::iterator it = m_dashLines->begin() ; it != m_dashLines->end() ; it++) {
        QString dashId = qElement.attribute( dashlineId.append(i), "");
        Uml::IDType dashlineId = STR2ID(dashId);
        if (dashlineId != Uml::id_None) {
            UMLWidget *floatdashline = m_pView -> findWidget( dashlineId );
            if (floatdashline != NULL) {
                it = static_cast<FloatingDashLineWidget*>(floatdashline);
                return true;
            }
        }
        i++;
    }*/

    //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if ( !element.isNull() ) {
        QString tag = element.tagName();
        if (tag == "floatingdashlinewidget") {
            /*m_dashLines.push_back(new FloatingDashLineWidget( m_pView , dashlineId ));
            if( ! m_dashLine->loadFromXMI(element) ) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete m_dashLine;
                m_dashLine = NULL;
            }*/
        } else {
            kError() << "MessageWidget::loadFromXMI: unknown tag "
            << tag << endl;
        }
    }
    setCombinedFragmentType( (CombinedFragmentType)type.toInt() );
    
    return true;
}

void CombinedFragmentWidget::slotMenuSelection(int sel) {
    UMLWidget::slotMenuSelection(sel);
    switch (sel) {
        // for alternative or parallel combined fragments
        case ListPopupMenu::mt_AddInteractionOperand:
            m_dashLines->push_back(new FloatingDashLineWidget(m_pView));
            if(m_CombinedFragment == Alt)
            {
                m_dashLines->back()->setText("else");
            }
            m_dashLines->back()->setX(getX());
            m_dashLines->back()->setYMin(getY());
            m_dashLines->back()->setYMax(getY() + getHeight());
            m_dashLines->back()->setY(getY() + getHeight() / 2);
            m_dashLines->back()->setSize(getWidth(), 0);
            m_pView->setupNewWidget(m_dashLines->back());
            break;
        default:
            break;
    }
}


#include "combinedfragmentwidget.moc"

