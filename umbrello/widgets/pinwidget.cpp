/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "pinwidget.h"

// app includes
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "listpopupmenu.h"
#include "umlscene.h"
#include "uniqueid.h"

// kde includes
#include <kinputdialog.h>
#include <klocale.h>

// qt includes
#include <QPainter>

#define PIN_MARGIN 5
#define PIN_WIDTH 1
#define PIN_HEIGHT 1

DEBUG_REGISTER_DISABLED(PinWidget)

/**
 * Creates a Pin widget.
 *
 * @param scene   The parent of the widget.
 * @param a       The widget to which this pin is attached.
 * @param id      The ID to assign (-1 will prompt a new ID).
 */
PinWidget::PinWidget(UMLScene* scene, UMLWidget* a, Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_Pin, id),
    m_pOw(a)
{
    m_ignoreSnapToGrid = true;
    m_ignoreSnapComponentSizeToGrid = true;
    m_resizable = false;
    setMinimumSize(UMLSceneSize(10, 10));
    setMaximumSize(UMLSceneSize(10, 10));
    setSize(10, 10);
    m_nY = y() < getMinY() ? getMinY() : y();

    m_pName = new FloatingTextWidget(scene, Uml::TextRole::Floating, "");
    scene->setupNewWidget(m_pName);
    m_pName->setX(0);
    m_pName->setY(0);

    activate();
    setVisible(true);
}

/**
 * Destructor.
 */
PinWidget::~PinWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void PinWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    int w = 10;
    int h = 10;
    int widthActivity = m_pOw->width();
    int heightActivity = m_pOw->height();
    int y = 0;
    int x = m_pOw->x() + (widthActivity/2);
    int offsetX = this->x();
    int offsetY = this->y();

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();

    if ((offsetY + heightActivity/2) <= m_pOw->y() + heightActivity){
        y = m_pOw->y()-5;
        if (m_pName->x() == 0 && m_pName->y() == 0) {
            //the floating text has not been linked with the signal
            m_pName->setX(x + 5 - m_Text.length()/2);
            m_pName->setY(y -fontHeight);
        }
    } else if((offsetY + heightActivity/2) > m_pOw->y() + heightActivity){
       y = (m_pOw->y() + heightActivity)-5;
        if (m_pName->x() == 0 && m_pName->y() == 0) {
            //the floating text has not been linked with the signal
            m_pName->setX(x + 5 - m_Text.length()/2);
            m_pName->setY(y + fontHeight);
        }
    }

    if (offsetX + widthActivity/4 <= m_pOw->x() + widthActivity/2
         && (offsetY > m_pOw->y() +5 && offsetY < m_pOw->y() + heightActivity - 5)){
        x = m_pOw->x() -5;
        y = m_pOw->y() + (heightActivity/2) -5;
        if (m_pName->x() == 0 && m_pName->y() == 0) {
            m_pName->setX(x - m_Text.length());
            m_pName->setY(y - fontHeight);
        }
    } else if (offsetX + widthActivity/4 > m_pOw->x() + widthActivity/2
         && (offsetY > m_pOw->y() +5 && offsetY < m_pOw->y() + heightActivity - 5)){
        x = m_pOw->x() + widthActivity -5;
        y = m_pOw->y() + (heightActivity/2) -5;
        if (m_pName->x() == 0 && m_pName->y() == 0) {
            //the floating text has not been linked with the signal
            m_pName->setX(x + 10);
            m_pName->setY(y - fontHeight);
        }
    }

    m_oldX = this->x();
    setX(x);
    m_oldY = this->y();
    setY(y);

//test if y isn't above the object
//     if (y <= m_pOw[Uml::A]->y() + height_Activity-5 && x == m_pOw[Uml::A]->x() + (width_Activity/2)) {
//         y = m_pOw[Uml::A]->y() + height_Activity + 15;
//     }
//     if (y + h >= m_pOw[Uml::A]->getEndLineY()) {
//         y = m_pOw[Uml::A]->getEndLineY() - h;
//     }

    setPenFromSettings(painter);
    if (UMLWidget::useFillColor()) {
        painter->setBrush(UMLWidget::fillColor());
    }
    painter->drawRect(0, 0, w, h);
    //make sure it's always above the others
    setZValue(20);
    setPenFromSettings(painter);
    m_pName->setVisible((m_pName->text().length() > 0));
    m_pName->updateGeometry();

    UMLWidget::paint(painter, option, widget);
}

/**
 * Sets the name of the pin.
 */
void PinWidget::setName(const QString &strName)
{
    m_Text = strName;
    updateGeometry();
    m_pName->setText(m_Text);
}

/**
 * Returns the minimum height this widget should be set at on
 * a sequence diagrams.  Takes into account the widget positions
 * it is related to.
 */
int PinWidget::getMinY()
{
    if (!m_pOw) {
        return 0;
    }
    int heightA = m_pOw->y() + m_pOw->height();
    return heightA;
}

/**
 * Overrides mouseMoveEvent.
 */
void PinWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* me)
{
    UMLWidget::mouseMoveEvent(me);
    int diffX = m_oldX - x();
    int diffY = m_oldY - y();
    if (m_pName!=NULL && !(m_pName->text()).isEmpty()) {
        m_pName->setX(m_pName->x() - diffX);
        m_pName->setY(m_pName->y() - diffY);
    }
}

/**
 * Captures any popup menu signals for menus it created.
 */
void PinWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        {
            bool ok = false;
            QString name = m_Text;
            name = KInputDialog::getText(i18n("Enter Pin Name"),
                                         i18n("Enter the pin name :"),
                                         m_Text, &ok);
            if (ok) {
                setName(name);
            }
        }
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Saves the widget to the "pinwidget" XMI element.
 */
void PinWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement PinElement = qDoc.createElement("pinwidget");
    PinElement.setAttribute("widgetaid", Uml::ID::toString(m_pOw->id()));
    UMLWidget::saveToXMI(qDoc, PinElement);
    if (m_pName && !m_pName->text().isEmpty()) {
        PinElement.setAttribute("textid", Uml::ID::toString(m_pName->id()));
        m_pName -> saveToXMI(qDoc, PinElement);
    }
    qElement.appendChild(PinElement);
}

/**
 * Loads the widget from the "pinwidget" XMI element.
 */
bool PinWidget::loadFromXMI(QDomElement& qElement)
{
    if(!UMLWidget::loadFromXMI(qElement))
        return false;
    QString widgetaid = qElement.attribute("widgetaid", "-1");

    Uml::ID::Type aId = Uml::ID::fromString(widgetaid);

    UMLWidget *pWA = m_scene -> findWidget(aId);
    if (pWA == NULL) {
        DEBUG(DBG_SRC) << "role A object " << Uml::ID::toString(aId) << " not found";
        return false;
    }

    m_pOw = pWA;

    QString textid = qElement.attribute("textid", "-1");
    Uml::ID::Type textId = Uml::ID::fromString(textid);
    if (textId != Uml::ID::None) {
        UMLWidget *flotext = m_scene -> findWidget(textId);
        if (flotext != NULL) {
            // This only happens when loading files produced by
            // umbrello-1.3-beta2.
            m_pName = static_cast<FloatingTextWidget*>(flotext);
            //return true;
        }
    } else {
        // no textid stored -> get unique new one
        textId = UniqueID::gen();
    }

    //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == "floatingtext") {
            m_pName = new FloatingTextWidget(m_scene, Uml::TextRole::Floating, m_Text, textId);
            if(! m_pName->loadFromXMI(element)) {
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

#include "pinwidget.moc"
