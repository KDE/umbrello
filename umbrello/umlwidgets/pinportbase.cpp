/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "pinportbase.h"

// app includes
#include "port.h"
#include "package.h"
#include "debug_utils.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "floatingtextwidget.h"
#include <debug_utils.h>

// qt includes
#include <QPainter>
#include <QToolTip>

// sys includes
#include <cmath>

PinPortBase::PinPortBase(UMLScene *scene, WidgetType type, UMLObject *o)
  : UMLWidget(scene, type, o)
{
    init();
}

PinPortBase::PinPortBase(UMLScene *scene, WidgetType type, UMLWidget *owner, Uml::ID::Type id)
  : UMLWidget(scene, type, id)
{
    init(owner);
}

/**
 * Standard destructor.
 */
PinPortBase::~PinPortBase()
{
}

/**
 * Performs initializations which are common to PinWidget and PortWidget.
 */
void PinPortBase::init(UMLWidget *owner)
{
    m_ignoreSnapToGrid = true;
    m_ignoreSnapComponentSizeToGrid = true;
    m_resizable = false;
    setParentItem(owner);
    m_pName = 0;
    const int edgeLength = 15;  // old: (m_baseType == wt_Pin ? 10 : 15);
    const QSizeF fixedSize(edgeLength, edgeLength);
    setMinimumSize(fixedSize);
    setMaximumSize(fixedSize);
    setSize(fixedSize);

    m_connectedSide = Top;
    setPos(0, - height() ); // place above parent
}

UMLWidget* PinPortBase::ownerWidget() const
{
    return dynamic_cast<UMLWidget*>(parentItem());
}

/**
 * Overrides method from UMLWidget in order to set a tooltip.
 * The tooltip is set to the name().
 * The reason for using a tooltip for the name is that the size of this
 * widget is not large enough to accommodate the average name.
 */
void PinPortBase::updateWidget()
{
    QString strName = name();
    uDebug() << " port name is " << strName;
    if (m_pName) {
        m_pName->setText(strName);
    } else {
        setToolTip(strName);
    }
}

/**
 * Overrides method from UMLWidget to set the name.
 */
void PinPortBase::setName(const QString &strName)
{
    UMLWidget::setName(strName);
    updateGeometry();
    if (m_pName) {
        m_pName->setText(strName);
    }
}

bool PinPortBase::isAboveParent() const
{
    return m_connectedSide == Top;
}

bool PinPortBase::isBelowParent() const
{
    return m_connectedSide == Bottom;
}

bool PinPortBase::isLeftOfParent() const
{
    return m_connectedSide == Left;
}

bool PinPortBase::isRightOfParent() const
{
    return m_connectedSide == Right;
}

qreal PinPortBase::getNewXOnJumpToTopOrBottom() const
{
    return isLeftOfParent() ? 0 : ownerWidget()->width() - width();
}

void PinPortBase::jumpToTopOfParent()
{
    setPos(QPointF(getNewXOnJumpToTopOrBottom(), - height()));
    m_connectedSide = Top;
}

void PinPortBase::jumpToBottomOfParent()
{
    setPos(QPointF(getNewXOnJumpToTopOrBottom(), ownerWidget()->height()));
    m_connectedSide = Bottom;
}

qreal PinPortBase::getNewYOnJumpToSide() const
{
    return isAboveParent() ? 0 : ownerWidget()->height() - height();
}

void PinPortBase::jumpToLeftOfParent()
{
    setPos(QPointF(-width(), getNewYOnJumpToSide()));
    m_connectedSide = Left;
}

void PinPortBase::jumpToRightOfParent()
{
    setPos(QPointF(ownerWidget()->width(), getNewYOnJumpToSide()));
    m_connectedSide = Right;
}

/**
 * Overridden from UMLWidget.
 * Moves the widget to a new position using the difference between the
 * current position and the new position.
 * Movement is constrained such that the port is always attached to its
 * owner widget.
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 */
void PinPortBase::moveWidgetBy(qreal diffX, qreal diffY)
{
    const qreal newX = x() + diffX;
    const qreal newY = y() + diffY;
    UMLWidget* owner = ownerWidget();
    if (isAboveParent() || isBelowParent()) {
        if (newX < 0.0) {
            if (- diffX > width()) {
                jumpToLeftOfParent();
            }
            else {
                setX(0);
            }
        }
        else if (newX > owner->width() - width()) {
            if (diffX > width()) {
                jumpToRightOfParent();
            }
            else {
                setX(owner->width() - width());
            }
        }
        else {
            setX(newX);
        }
    }
    else if (isLeftOfParent() || isRightOfParent()) {
        if (newY < 0.0) {
            if (- diffY > height()) {
                jumpToTopOfParent();
            }
            else {
                setY(0);
            }
        }
        else if (newY > owner->height() - height()) {
            if (diffY > height()) {
                jumpToBottomOfParent();
            }
            else {
                setY(owner->height() - height());
            }
        }
        else {
            setY(newY);
        }
    }
    else {
        // error: client is not attached to parent
        jumpToTopOfParent();
    }
}

/**
 * Receive notification when parent is resized.
 * We need to track parent resize to always stay attached to it.
 */
void PinPortBase::notifyParentResize()
{
    UMLWidget* owner = ownerWidget();
    if (isRightOfParent()) {
        setPos(owner->width(), qMin(y(), owner->height() - height()));
    }
    else if (isBelowParent()) {
        setPos(qMin(x(), owner->width() - width()), owner->height());
    }
}

/**
 * Overrides standard method.
 */
void PinPortBase::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setPenFromSettings(painter);
    if (UMLWidget::useFillColor())  {
        painter->setBrush(UMLWidget::fillColor());
    } else {
        painter->setBrush(m_scene->backgroundColor());
    }

    painter->drawRect(0, 0, width(), height());

    UMLWidget::paint(painter, option, widget);
}

QRectF PinPortBase::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}

/**
 * Captures any popup menu signals for menus it created.
 */
void PinPortBase::slotMenuSelection(QAction* action)
{
    uDebug() << "PinPortBase::slotMenuSelection";
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_NameAsTooltip:
        if (m_pName) {
            action->setChecked(true);
            delete m_pName;
            m_pName = 0;
            setToolTip(name());
        } else {
            action->setChecked(false);
            m_pName = new FloatingTextWidget(m_scene, Uml::TextRole::Floating, name());
            m_pName->setParentItem(this);
            m_pName->setText(name());  // to get geometry update
            m_pName->activate();
            UMLWidget* owner = ownerWidget();
            if (owner == 0) {
                uError() << "PinPortBase::slotMenuSelection: ownerWidget() returns NULL";
                setX(x());
                setY(y());
            } else {
                const qreal w = width();
                const qreal h = height();
                if (x() < owner->x())
                    m_pName->setX(-m_pName->width());
                else if (x() >= owner->x() + owner->width())
                    m_pName->setX(w);
                else
                    m_pName->setX(-m_pName->width() / 2.0 + w / 2.0);
                if (y() < owner->y())
                    m_pName->setY(-m_pName->height() - 2);
                else if (y() >= owner->y() + owner->height())
                    m_pName->setY(h);
                else
                    m_pName->setY(-m_pName->height() / 2.0 + h / 2.0);
            }
            m_pName->update();
            setToolTip(QString());
            QToolTip::hideText();
        }
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

FloatingTextWidget *PinPortBase::floatingTextWidget() {
    return m_pName;
}

void PinPortBase::setFloatingTextWidget(FloatingTextWidget *ft) {
    m_pName = ft;
    if (m_pName)
        m_pName->setParentItem(this);
}

/**
 * Override method from UMLWidget in order to additionally check m_pName.
 *
 * @param p Point to be checked.
 *
 * @return 'this' if UMLWidget::onWidget(p) returns non 0;
 *         m_pName if m_pName is non NULL and m_pName->onWidget(p) returns non 0;
 *         else NULL.
 */
UMLWidget* PinPortBase::onWidget(const QPointF &p)
{
    if (UMLWidget::onWidget(p) != 0)
        return this;
    if (m_pName) {
        uDebug() << "floatingtext: " << m_pName->text();
        return m_pName->onWidget(p);
    }
    return 0;
}

/**
 * Reimplement function from UMLWidget
 */
UMLWidget* PinPortBase::widgetWithID(Uml::ID::Type id)
{
    if (UMLWidget::widgetWithID(id))
        return this;
    if (m_pName && m_pName->widgetWithID(id))
        return m_pName;
    return 0;
}


/**
 * Saves the widget to the "pinwidget" or "portwidget" XMI element.
 */
void PinPortBase::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement element = qDoc.createElement(baseType() == wt_Pin ? QLatin1String("pinwidget")
                                                                  : QLatin1String("portwidget"));
    Q_ASSERT(ownerWidget() != 0);
    element.setAttribute(QLatin1String("widgetaid"), Uml::ID::toString(ownerWidget()->id()));
    UMLWidget::saveToXMI(qDoc, element);
    if (m_pName && !m_pName->text().isEmpty()) {
        m_pName->saveToXMI(qDoc, element);
    }
    qElement.appendChild(element);
}

/**
 * Loads from a "pinwidget" or from a "portwidget" XMI element.
 */
bool PinPortBase::loadFromXMI(QDomElement & qElement)
{
    if (!UMLWidget::loadFromXMI(qElement))
        return false;

    QString widgetaid = qElement.attribute(QLatin1String("widgetaid"), QLatin1String("-1"));
    Uml::ID::Type aId = Uml::ID::fromString(widgetaid);
    UMLWidget *owner = m_scene->findWidget(aId);
    if (owner == 0) {
        DEBUG(DBG_SRC) << "owner object " << Uml::ID::toString(aId) << " not found";
        return false;
    }
    setParentItem(owner);

    // Optional child element: floatingtext
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == QLatin1String("floatingtext")) {
            m_pName = new FloatingTextWidget(m_scene, Uml::TextRole::Floating,
                                             name(), Uml::ID::Reserved);
            if (!m_pName->loadFromXMI(element)) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete m_pName;
                m_pName = 0;
            } else {
                m_pName->setParentItem(this);
                m_pName->activate();
                m_pName->update();
            }
        } else {
            uError() << "unknown tag " << tag;
        }
    }
    return true;
}

