/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "combinedfragmentwidget.h"

// qt includes
#include <QtGui/QPainter>
#include <QtCore/QString>

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "umlview.h"
#include "listpopupmenu.h"
#include "dialog_utils.h"

CombinedFragmentWidget::CombinedFragmentWidget(UMLScene * scene, CombinedFragmentType combinedfragmentType, Uml::IDType id )
  : UMLWidget(scene, WidgetBase::wt_CombinedFragment, id)
{
    setCombinedFragmentType( combinedfragmentType );
}

CombinedFragmentWidget::~CombinedFragmentWidget()
{
    for(QList<FloatingDashLineWidget*>::iterator it=m_dashLines.begin() ; it!=m_dashLines.end() ; ++it) {
        delete(*it);
    }
}

void CombinedFragmentWidget::paint(QPainter & p, int offsetX, int offsetY)
{
    int w = width();
    int h = height();
    int line_width = 45;
    int old_Y;

    setPenFromSettings(p);

    if ( m_CombinedFragment == Ref ) {
        if ( UMLWidget::useFillColor() ) {
            p.setBrush( UMLWidget::fillColor() );
        }
    }
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const QString combined_fragment_value =  name();
    int textStartY = (h / 2) - (fontHeight / 2);
    p.drawRect(offsetX, offsetY, w, h );

    p.setPen(textColor());
    p.setFont( UMLWidget::font() );
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
                     temp += " [" + combined_fragment_value + ']';
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
                if (combined_fragment_value != "-")
                {
                     temp = '[' + combined_fragment_value + ']';
            p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY + 20,w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, temp);
                    if (m_dashLines.size() == 1 && m_dashLines.first()->getY() < offsetY + 20 + fontHeight )
                        m_dashLines.first()->setY(offsetY + h/2);
                }
                p.drawText(offsetX + COMBINED_FRAGMENT_MARGIN, offsetY ,
            w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, "alt");
                // dash lines
                m_dashLines.first()->paint(p,getX(),getY());
                for(QList<FloatingDashLineWidget*>::iterator it=m_dashLines.begin() ; it!=m_dashLines.end() ; ++it) {
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
                if (m_dashLines.size() != 0) {
                    m_dashLines.first()->paint(p,getX(),getY());
                    for(QList<FloatingDashLineWidget*>::iterator it=m_dashLines.begin() ; it!=m_dashLines.end() ; ++it) {
                        (*it)->setX(getX());
                        old_Y = (*it)->getYMin();
                        (*it)->setYMin(getY());
                        (*it)->setYMax(getY() + getHeight());
                        (*it)->setY(getY() + (*it)->getY() - old_Y);
                        (*it)->setSize(w, 0);
                    }
                }
        break;

    default : break;
    }

    p.setPen(Qt::red);
    p.drawLine(offsetX,      offsetY + 20, offsetX + line_width, offsetY + 20);
    p.drawLine(offsetX + line_width, offsetY + 20, offsetX + line_width + 10, offsetY + 10);
    p.drawLine(offsetX + line_width + 10, offsetY + 10, offsetX + line_width + 10, offsetY);

    if(m_selected)
        drawSelected(&p, offsetX, offsetY);
}

UMLSceneSize CombinedFragmentWidget::minimumSize()
{
    int width = 10, height = 10;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int textWidth = fm.width(name());
    height = fontHeight;
    width = textWidth + 60 > COMBINED_FRAGMENT_WIDTH ? textWidth + 60: COMBINED_FRAGMENT_WIDTH;
    if ( m_CombinedFragment == Loop )
         width += int((float)textWidth * 0.4f);
    if ( m_CombinedFragment == Alt )
         height += fontHeight + 40;
    height = height > COMBINED_FRAGMENT_HEIGHT ? height : COMBINED_FRAGMENT_HEIGHT;
    width += COMBINED_FRAGMENT_MARGIN * 2;
    height += COMBINED_FRAGMENT_MARGIN * 2;

    return UMLSceneSize(width, height);
}

CombinedFragmentWidget::CombinedFragmentType CombinedFragmentWidget::combinedFragmentType() const
{
    return m_CombinedFragment;
}

void CombinedFragmentWidget::setCombinedFragmentType( CombinedFragmentType combinedfragmentType )
{
    m_CombinedFragment = combinedfragmentType;
    UMLWidget::m_resizable =  true ; //(m_CombinedFragment == Normal);
    // creates a dash line if the combined fragment type is alternative or parallel
    if(m_CombinedFragment == Alt  && m_dashLines.isEmpty())
    {
        m_dashLines.push_back(new FloatingDashLineWidget(m_scene));
        if(m_CombinedFragment == Alt)
        {
            m_dashLines.back()->setText("else");
        }
        m_dashLines.back()->setX(getX());
        m_dashLines.back()->setYMin(getY());
        m_dashLines.back()->setYMax(getY() + getHeight());
        m_dashLines.back()->setY(getY() + height()/2);
        m_dashLines.back()->setSize(getWidth(), 0);
        m_scene->setupNewWidget(m_dashLines.back());
    }
}

CombinedFragmentWidget::CombinedFragmentType CombinedFragmentWidget::combinedFragmentType(const QString& type) const
{
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
    // Shouldn't happen
    Q_ASSERT(0);
    return (CombinedFragmentWidget::Ref);
}

void CombinedFragmentWidget::setCombinedFragmentType( const QString& combinedfragmentType )
{
    setCombinedFragmentType(combinedFragmentType(combinedfragmentType) );
}

void CombinedFragmentWidget::askNameForWidgetType(UMLWidget* &targetWidget, const QString& dialogTitle,
    const QString& dialogPrompt, const QString& defaultName)
{
    Q_UNUSED(defaultName);
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
        if (type == "Alternative")
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the first alternative name"), i18n("Enter the first alternative name"), i18n("-"));
    } else {
        targetWidget->cleanup();
        delete targetWidget;
        targetWidget = NULL;
    }
}

void CombinedFragmentWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement combinedFragmentElement = qDoc.createElement( "combinedFragmentwidget" );
    UMLWidget::saveToXMI( qDoc, combinedFragmentElement );
    combinedFragmentElement.setAttribute( "combinedFragmentname", m_Text );
    combinedFragmentElement.setAttribute( "documentation", m_Doc );
    combinedFragmentElement.setAttribute( "CombinedFragmenttype", m_CombinedFragment );

    // save the corresponding floating dash lines
    for (QList<FloatingDashLineWidget*>::iterator it = m_dashLines.begin() ; it != m_dashLines.end() ; ++it) {
        (*it)-> saveToXMI( qDoc, combinedFragmentElement );
    }

    qElement.appendChild( combinedFragmentElement );
}

bool CombinedFragmentWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "combinedFragmentname", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    QString type = qElement.attribute( "CombinedFragmenttype", "");
    Uml::IDType dashlineId;
    QList<FloatingDashLineWidget*> listline;

    //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    while ( !element.isNull() ) {
        QString tag = element.tagName();
        if (tag == "floatingdashlinewidget") {
            FloatingDashLineWidget * fdlwidget = new FloatingDashLineWidget(m_scene);
            m_dashLines.push_back(fdlwidget);
            if( !fdlwidget->loadFromXMI(element) ) {
              // Most likely cause: The FloatingTextWidget is empty.
                delete m_dashLines.back();
                return false;
            }
            else {
                m_scene->setupNewWidget(fdlwidget);
            }
        } else {
            uError() << "unknown tag " << tag;
        }
        node = node.nextSibling();
        element = node.toElement();
    }
   // m_dashLines = listline;
    setCombinedFragmentType( (CombinedFragmentType)type.toInt() );

    return true;
}

void CombinedFragmentWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString name = m_Text;
    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);
    switch (sel) {
          // for alternative or parallel combined fragments
    case ListPopupMenu::mt_AddInteractionOperand:
        m_dashLines.push_back(new FloatingDashLineWidget(m_scene));
        if(m_CombinedFragment == Alt)
        {
            m_dashLines.back()->setText("else");
        }
        m_dashLines.back()->setX(getX());
        m_dashLines.back()->setYMin(getY());
        m_dashLines.back()->setYMax(getY() + getHeight());
        m_dashLines.back()->setY(getY() + getHeight() / 2);
        m_dashLines.back()->setSize(getWidth(), 0);
        m_scene->setupNewWidget(m_dashLines.back());
        break;

    case ListPopupMenu::mt_Rename:
        if (m_CombinedFragment == Alt) {
            name = KInputDialog::getText( i18n("Enter first alternative"), i18n("Enter first alternative :"), m_Text, &ok );
        }
        else if (m_CombinedFragment == Ref) {
        name = KInputDialog::getText( i18n("Enter referenced diagram name"), i18n("Enter referenced diagram name :"), m_Text, &ok );
        }
        else if (m_CombinedFragment == Loop) {
        name = KInputDialog::getText( i18n("Enter the guard of the loop"), i18n("Enter the guard of the loop:"), m_Text, &ok );
        }
        if( ok && name.length() > 0 )
            m_Text = name;
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

#include "combinedfragmentwidget.moc"

