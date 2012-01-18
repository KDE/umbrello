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
#include "signalwidget.h"

// app includes
#include "basictypes.h"
#include "debug_utils.h"
#include "docwindow.h"
#include "floatingtextwidget.h"
#include "linkwidget.h"
#include "listpopupmenu.h"
#include "uml.h"
#include "umldoc.h"
#include "uniqueid.h"
#include "umlview.h"
#include "umlwidget.h"

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

// qt includes
#include <QtCore/QEvent>
#include <QtGui/QPolygon>


/**
 * Creates a Signal widget.
 *
 * @param scene              The parent of the widget.
 * @param signalType        The type of Signal.
 * @param id                The ID to assign (-1 will prompt a new ID.)
 */
SignalWidget::SignalWidget(UMLScene *scene, SignalType signalType, Uml::IDType id)
  : UMLWidget(scene, WidgetBase::wt_Signal, id)
{
    m_signalType = signalType;
    m_pName = NULL;
    if (signalType == SignalWidget::Time) {
        m_pName = new FloatingTextWidget(scene, Uml::TextRole::Floating,"");
        scene->setupNewWidget(m_pName);
        m_pName->setX(0);
        m_pName->setY(0);
    }
}

/**
 * destructor
 */
SignalWidget::~SignalWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void SignalWidget::paint(QPainter & p, int offsetX, int offsetY)
{
    setPenFromSettings(p);
    const int w = width();
    const int h = height();
    QPolygon a;
    switch (m_signalType)
    {
    case Send :
        if(UMLWidget::useFillColor())
            p.setBrush(UMLWidget::fillColor());
        {

            a.setPoints( 5, offsetX           ,offsetY,
                            offsetX + (w*2)/3 ,offsetY,
                            offsetX + w       ,(h/2)+offsetY,
                            offsetX + (w*2)/3 ,h+offsetY,
                            offsetX           ,h+offsetY );
            p.drawPolygon( a );
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            int textStartY = (h / 2) - (fontHeight / 2);

            p.setPen(textColor());
            QFont font = UMLWidget::font();
            font.setBold( false );
            p.setFont( font );
            p.drawText(offsetX + SIGNAL_MARGIN, offsetY + textStartY,
                           w - SIGNAL_MARGIN * 2, fontHeight,
                           Qt::AlignCenter, getName());
            setPenFromSettings(p);
        }
        break;
    case Accept :
        if(UMLWidget::useFillColor())
            p.setBrush(UMLWidget::fillColor());
        {
            a.setPoints( 5, offsetX ,      offsetY,
                            offsetX + w/3, (h/2)+offsetY,
                            offsetX ,      h+offsetY,
                            offsetX + w,   h+offsetY,
                            offsetX + w,   offsetY );

            p.drawPolygon( a );
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            int textStartY = (h / 2) - (fontHeight / 2);

            p.setPen(textColor());
            QFont font = UMLWidget::font();
            font.setBold( false );
            p.setFont( font );
            p.drawText(offsetX + SIGNAL_MARGIN, offsetY + textStartY,
                           w - SIGNAL_MARGIN * 2 + (w/3), fontHeight,
                           Qt::AlignCenter, getName());
            setPenFromSettings(p);
        }
        break;
    case Time :
        if(UMLWidget::useFillColor())
            p.setBrush(UMLWidget::fillColor());
        {
            a.setPoints( 4, offsetX ,    offsetY,
                            offsetX + w, offsetY+h,
                            offsetX ,    offsetY+h,
                            offsetX + w, offsetY);

            p.drawPolygon( a );
            //const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            //const int fontHeight  = fm.lineSpacing();
            //int textStartY = (h / 2) - (fontHeight / 2);
            p.setPen(textColor());
            QFont font = UMLWidget::font();
            font.setBold( false );
            p.setFont( font );

            setPenFromSettings(p);
        }
        if (m_pName->getX() == 0 && m_pName->getY() == 0) {
            //the floating text has not been linked with the signal
            m_pName->setX(offsetX + w/2 - m_pName->getWidth()/2);
            m_pName->setY(offsetY + h);
        }
        m_pName->setVisible( ( m_pName->text().length() > 0 ) );
        m_pName->updateComponentSize();

        break;
    default:
        uWarning() << "Unknown signal type:" << m_signalType;
        break;
    }
    if(m_selected)
        drawSelected(&p, offsetX, offsetY);
}

/**
 * Overrides the UMLWidget method.
 */
void SignalWidget::setX(int newX)
{
    m_oldX = getX();
    UMLWidget::setX(newX);
}

/**
 * Overrides the UMLWidget method.
 */
void SignalWidget::setY(int newY)
{
    m_oldY = getY();
    UMLWidget::setY(newY);
}

/**
 * Sets the name of the signal.
 */
void SignalWidget::setName(const QString &strName)
{
    m_Text = strName;
    updateComponentSize();
    if (getSignalType() == SignalWidget::Time) {
        m_pName->setText(m_Text);
    }
}

/**
 * Returns the name of the Signal.
 */
QString SignalWidget::getName() const
{
    return m_Text;
}

/**
 * Returns the type of Signal.
 */
SignalWidget::SignalType SignalWidget::getSignalType() const
{
    return m_signalType;
}

/**
 * Sets the type of Signal.
 */
void SignalWidget::setSignalType( SignalType signalType )
{
    m_signalType = signalType;
}

/**
 * Show a properties dialog for a UMLWidget.
 */
void SignalWidget::showPropertiesDialog()
{
}

/**
 * Overrides mouseMoveEvent.
 */
void SignalWidget::mouseMoveEvent(QMouseEvent* me)
{
    UMLWidget::mouseMoveEvent(me);
    int diffX = m_oldX - getX();
    int diffY = m_oldY - getY();
    if (m_pName!=NULL) {
        m_pName->setX(m_pName->getX() - diffX);
        m_pName->setY(m_pName->getY() - diffY);
    }
}

/**
 * Loads a "signalwidget" XMI element.
 */
bool SignalWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "signalname", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    QString type = qElement.attribute( "signaltype", "" );
    QString textid = qElement.attribute( "textid", "-1" );
    Uml::IDType textId = STR2ID(textid);

    setSignalType((SignalType)type.toInt());
    if (getSignalType() == Time) {

        if (textId != Uml::id_None) {
            UMLWidget *flotext = m_scene -> findWidget( textId );
            if (flotext != NULL) {
            // This only happens when loading files produced by
            // umbrello-1.3-beta2.
                m_pName = static_cast<FloatingTextWidget*>(flotext);
                return true;
            }
        } else {
            // no textid stored -> get unique new one
            textId = UniqueID::gen();
        }
    }
     //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if ( !element.isNull() ) {
        QString tag = element.tagName();
        if (tag == "floatingtext") {
            m_pName = new FloatingTextWidget( m_scene, Uml::TextRole::Floating, m_Text, textId );
            if( ! m_pName->loadFromXMI(element) ) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete m_pName;
                m_pName = NULL;
            }
        } else {
            uError() << "unknown tag " << tag;
        }
    }
   return true;
}

/**
 * Creates the "signalwidget" XMI element.
 */
void SignalWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement signalElement = qDoc.createElement( "signalwidget" );
    UMLWidget::saveToXMI( qDoc, signalElement );
    signalElement.setAttribute( "signalname", m_Text );
    signalElement.setAttribute( "documentation", m_Doc );
    signalElement.setAttribute( "signaltype", m_signalType );
    if (m_pName && !m_pName->text().isEmpty()) {
        signalElement.setAttribute( "textid", ID2STR(m_pName->id()) );
        m_pName -> saveToXMI( qDoc, signalElement );
    }
    qElement.appendChild( signalElement );
}

/**
 * Show a properties dialog for a SignalWidget.
 *
 */
void SignalWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString name = m_Text;

    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);
    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        name = KInputDialog::getText( i18n("Enter signal name"), i18n("Enter the signal name :"), m_Text, &ok );
        if( ok && name.length() > 0 )
            setName(name);
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Overrides method from UMLWidget
 */
UMLSceneSize SignalWidget::minimumSize()
{
        int width = SIGNAL_WIDTH, height = SIGNAL_HEIGHT;
        const QFontMetrics &fm = getFontMetrics(FT_BOLD);
        const int fontHeight  = fm.lineSpacing();
        int textWidth = fm.width(getName());

        if (m_signalType == Accept)
             textWidth = int((float)textWidth * 1.3f);
        height  = fontHeight;
        if (m_signalType != Time)
        {
              width   = textWidth > SIGNAL_WIDTH?textWidth:SIGNAL_WIDTH;
              height  = height > SIGNAL_HEIGHT?height:SIGNAL_HEIGHT;
        }
        width  += SIGNAL_MARGIN * 2;
        height += SIGNAL_MARGIN * 2;

    return UMLSceneSize(width, height);
}

#include "signalwidget.moc"
