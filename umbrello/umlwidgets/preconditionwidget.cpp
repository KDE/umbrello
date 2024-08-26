/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "preconditionwidget.h"

// app includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "listpopupmenu.h"
#include "objectwidget.h"
#include "uml.h"
#include "umlscene.h"
#include "uniqueid.h"
#include "idchangelog.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QPainter>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(PreconditionWidget)

#define PRECONDITION_MARGIN 5
#define PRECONDITION_WIDTH 30
#define PRECONDITION_HEIGHT 10

/**
 * Creates a Precondition widget.
 *
 * @param scene   The parent of the widget.
 * @param a       The role A widget for this precondition.
 * @param id      The ID to assign (-1 will prompt a new ID).
 */
PreconditionWidget::PreconditionWidget(UMLScene* scene, ObjectWidget* a, Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_Precondition, id),
    m_objectWidget(a)
{
    m_ignoreSnapToGrid = true;
    m_ignoreSnapComponentSizeToGrid = true;
    m_resizable =  true ;
    setVisible(true);
    //updateResizability();
    // calculateWidget();
    if (y() < minY())
        m_nY = minY();
    else if (y() > maxY())
        m_nY = maxY();
    else
        m_nY = y();

    connect(m_objectWidget, SIGNAL(sigWidgetMoved(Uml::ID::Type)), this, SLOT(slotWidgetMoved(Uml::ID::Type)));
    calculateDimensions();
}

/**
 * Destructor.
 */
PreconditionWidget::~PreconditionWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void PreconditionWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    int w = width();
    int h = height();

    if (m_objectWidget) {
        int x = m_objectWidget->x() + m_objectWidget->width() / 2;
        x -= w/2;
        setX(x);
        int y = this->y();

        //test if y isn't above the object
        if (y <= m_objectWidget->y() + m_objectWidget->height()) {
            y = m_objectWidget->y() + m_objectWidget->height() + 15;
        }
        if (y + h >= m_objectWidget->getEndLineY()) {
            y = m_objectWidget->getEndLineY() - h;
        }
        setY(y);
    }
    setPenFromSettings(painter);
    if (UMLWidget::useFillColor()) {
        painter->setBrush(UMLWidget::fillColor());
    }
    {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const QString precondition_value = QStringLiteral("{ ") + name() + QStringLiteral(" }");
        //int middleX = w / 2;
        int textStartY = (h / 2) - (fontHeight / 2);
        painter->drawRoundedRect(0, 0, w, h, (h * 60) / w, 60);
        painter->setPen(textColor());
        painter->setFont(UMLWidget::font());
        painter->drawText(PRECONDITION_MARGIN, textStartY,
                       w - PRECONDITION_MARGIN * 2, fontHeight, Qt::AlignCenter, precondition_value);
    }

    UMLWidget::paint(painter, option, widget);
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF PreconditionWidget::minimumSize() const
{
    int width = 10, height = 10;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int textWidth = fm.horizontalAdvance(name()) + 25;
    height = fontHeight;
    width = textWidth > PRECONDITION_WIDTH ? textWidth : PRECONDITION_WIDTH;
    height = height > PRECONDITION_HEIGHT ? height : PRECONDITION_HEIGHT;
    width += PRECONDITION_MARGIN * 2;
    height += PRECONDITION_MARGIN * 2;

    return QSizeF(width, height);
}

/**
 * Calculate the geometry of the widget.
 */
void PreconditionWidget::calculateWidget()
{
    calculateDimensions();

    setVisible(true);

    setX(m_nPosX);
    setY(m_nY);
}

/**
 * Activates a PreconditionWidget.  Connects the WidgetMoved signal from
 * its m_objectWidget pointer so that PreconditionWidget can adjust to the move of
 * the object widget.
 */
bool PreconditionWidget::activate(IDChangeLog  *Log /*= nullptr*/)
{
    m_scene->resetPastePoint();
    UMLWidget::activate(Log);

    if (m_objectWidget == nullptr &&
            !(m_widgetAId.empty() || m_widgetAId == Uml::ID::None || m_widgetAId == Uml::ID::Reserved)) {
        UMLWidget *w = umlScene()->findWidget(m_widgetAId);
        m_objectWidget  = w->asObjectWidget();
        if (!m_objectWidget) {
            DEBUG() << "role A widget " << Uml::ID::toString(m_widgetAId) << " could not be found";
            return false;
        }
        connect(m_objectWidget, SIGNAL(sigWidgetMoved(Uml::ID::Type)), this, SLOT(slotWidgetMoved(Uml::ID::Type)));
    }

    calculateDimensions();
    return true;
}

/**
 * Resolve references of this precondition so it references the correct
 * new object widget after paste.
 */
void PreconditionWidget::resolveObjectWidget(IDChangeLog* log)
{
    m_widgetAId = log->findNewID(m_widgetAId);
    activate(log);
}

/**
 * Calculates the size of the widget.
 */
void PreconditionWidget::calculateDimensions()
{
    int x = 0;
    int w = 0;
    int h = 0;
    QSizeF q = minimumSize();
    w = q.width() > width() ? q.width() : width();
    h = q.height() > height() ? q.height() : height();

    if (m_objectWidget) {
        int x1 = m_objectWidget->x();
        int w1 = m_objectWidget->width() / 2;
        x1 += w1;
        x = x1 - w/2;
        m_nPosX = x;
    }

    setSize(w, h);
}

/**
 * Slot when widget is moved.
 */
void PreconditionWidget::slotWidgetMoved(Uml::ID::Type id)
{
    const Uml::ID::Type idA = m_objectWidget ? m_objectWidget->localID() : Uml::ID::None;
    if (idA != id) {
        DEBUG() << "id=" << Uml::ID::toString(id) << ": ignoring for idA=" << Uml::ID::toString(idA);
        return;
    }
    m_nY = y();
    if (m_nY < minY())
        m_nY = minY();
    if (m_nY > maxY())
        m_nY = maxY();

    calculateDimensions();
    if (m_scene->selectedCount(true) > 1)
        return;
}

/**
 * Returns the minimum height this widget should be set at on
 * a sequence diagrams. Takes into account the widget positions
 * it is related to.
 */
int PreconditionWidget::minY() const
{
    if (m_objectWidget) {
        return m_objectWidget->y() + m_objectWidget->height();
    }
    return 0;
}

/**
 * Returns the maximum height this widget should be set at on
 * a sequence diagrams. Takes into account the widget positions
 * it is related to.
 */
int PreconditionWidget::maxY() const
{
    if (m_objectWidget) {
        return ((int)m_objectWidget->getEndLineY() - height());
    }
    return 0;
}

/**
 * Captures any popup menu signals for menus it created.
 */
void PreconditionWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        {
            QString text = name();
            bool ok = Dialog_Utils::askNewName(WidgetBase::wt_Precondition, text);
            if (ok && !text.isEmpty()) {
                setName(text);
            }
            calculateWidget();
        }
        break;

    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Saves the widget to the "preconditionwidget" XMI element.
 */
void PreconditionWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("preconditionwidget"));
    UMLWidget::saveToXMI(writer);

    writer.writeAttribute(QStringLiteral("widgetaid"), Uml::ID::toString(m_objectWidget->localID()));
    writer.writeAttribute(QStringLiteral("preconditionname"), name());
    writer.writeAttribute(QStringLiteral("documentation"), documentation());
    writer.writeEndElement();
}

/**
 * Loads the widget from the "preconditionwidget" XMI element.
 */
bool PreconditionWidget::loadFromXMI(QDomElement& qElement)
{
    if(!UMLWidget::loadFromXMI(qElement))
        return false;
    setName(qElement.attribute(QStringLiteral("preconditionname")));
    setDocumentation(qElement.attribute(QStringLiteral("documentation")));
    QString widgetaid = qElement.attribute(QStringLiteral("widgetaid"), QStringLiteral("-1"));
    m_widgetAId = Uml::ID::fromString(widgetaid);
    return true;
}

ObjectWidget *PreconditionWidget::objectWidget() const
{
    return m_objectWidget;
}

void PreconditionWidget::setObjectWidget(ObjectWidget *objectWidget)
{
    m_objectWidget = objectWidget;
}
