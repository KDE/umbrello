/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "componentwidget.h"

// app includes
#include "component.h"
#include "debug_utils.h"
#include "umlscene.h"
#include "umlview.h"
#include "optionstate.h"
#include "umldoc.h"
#include "package.h"
#include "portwidget.h"

// qt includes
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(ComponentWidget)

/**
 * Constructs a ComponentWidget.
 *
 * @param scene      The parent of this ComponentWidget.
 * @param c The UMLComponent this will be representing.
 */
ComponentWidget::ComponentWidget(UMLScene * scene, UMLComponent *c)
  : UMLWidget(scene, WidgetBase::wt_Component, c)
{
    setSize(100, 30);
    //set defaults from m_scene
    if (m_scene) {
        //check to see if correct
        const Settings::OptionState& ops = m_scene->optionState();
        m_showStereotype = ops.classState.showStereoType;
    }
}

/**
 * Destructor.
 */
ComponentWidget::~ComponentWidget()
{
}

/**
 * Reimplemented from UMLWidget::paint to paint component
 * widget.
 */
void ComponentWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const UMLComponent *umlcomp = m_umlObject->asUMLComponent();
    if (umlcomp == 0)
        return;
    setPenFromSettings(painter);
    QPen origPen = painter->pen();
    QPen pen = origPen;
    if (umlcomp->getExecutable()) {
        pen.setWidth(origPen.width() + 2);
        painter->setPen(pen);
    }
    if (UMLWidget::useFillColor()) {
        painter->setBrush(UMLWidget::fillColor());
    } else {
        painter->setBrush(m_scene->backgroundColor());
    }

    const int w = width();
    const int h = height();
    const int halfHeight = h / 2;
    int   textXOffset = 0;
    QFont font = UMLWidget::font();
    font.setBold(true);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight = fm.lineSpacing();
    QString nameStr = name();
    const QString stereotype = m_umlObject->stereotype();

    if (Settings::optionState().generalState.uml2) {
        painter->drawRect(0, 0, w, h);
        // draw small component symbol in upper right corner
        painter->setPen(origPen);
        painter->drawRect(w - 17,  5, 11, 13);
        painter->drawRect(w - 19,  7,  2,  2);
        painter->drawRect(w - 19, 11,  2,  2);
        painter->setPen(pen);
    } else {
        painter->drawRect(2*COMPONENT_MARGIN, 0, w - 2*COMPONENT_MARGIN, h);
        painter->drawRect(0, halfHeight - fontHeight/2 - fontHeight, COMPONENT_MARGIN*4, fontHeight);
        painter->drawRect(0, halfHeight + fontHeight/2, COMPONENT_MARGIN*4, fontHeight);
        textXOffset = COMPONENT_MARGIN * 4;
    }

    painter->setPen(textColor());
    painter->setFont(font);

    int lines = 1;

    if (!stereotype.isEmpty()) {
        painter->drawText(textXOffset, halfHeight - fontHeight,
                   w - textXOffset, fontHeight, Qt::AlignCenter,
                   m_umlObject->stereotype(true));
        lines = 2;
    }

    if (UMLWidget::isInstance()) {
        font.setUnderline(true);
        painter->setFont(font);
        nameStr = UMLWidget::instanceName() + QLatin1String(" : ") + nameStr;
    }

    if (lines == 1) {
        painter->drawText(textXOffset, halfHeight - (fontHeight/2),
                   w - textXOffset, fontHeight, Qt::AlignCenter, nameStr);
    } else {
        painter->drawText(textXOffset, halfHeight,
                   w - textXOffset, fontHeight, Qt::AlignCenter, nameStr);
    }

    UMLWidget::paint(painter, option, widget);
}

/**
 * Overridden from UMLWidget due to emission of signal sigCompMoved()
 */
void ComponentWidget::moveWidgetBy(qreal diffX, qreal diffY)
{
    UMLWidget::moveWidgetBy(diffX, diffY);
    emit sigCompMoved(diffX, diffY);
}

/**
 * Override method from UMLWidget for adjustment of attached PortWidgets.
 */
void ComponentWidget::adjustAssocs(qreal dx, qreal dy)
{
    if (m_doc->loading()) {
        // don't recalculate the assocs during load of XMI
        // -> return immediately without action
        return;
    }
    UMLWidget::adjustAssocs(dx, dy);
    const UMLPackage *comp = m_umlObject->asUMLPackage();
    foreach (UMLObject *o, comp->containedObjects()) {
        uIgnoreZeroPointer(o);
        if (o->baseType() != UMLObject::ot_Port)
            continue;
        UMLWidget *portW = m_scene->widgetOnDiagram(o->id());
        if (portW)
            portW->adjustAssocs(dx, dy);
    }
}

/**
 * Override method from UMLWidget for adjustment of attached PortWidgets.
 */
void ComponentWidget::adjustUnselectedAssocs(qreal dx, qreal dy)
{
    if (m_doc->loading()) {
        // don't recalculate the assocs during load of XMI
        // -> return immediately without action
        return;
    }
    UMLWidget::adjustUnselectedAssocs(dx, dy);
    const UMLPackage *comp = m_umlObject->asUMLPackage();
    foreach (UMLObject *o, comp->containedObjects()) {
        uIgnoreZeroPointer(o);
        if (o->baseType() != UMLObject::ot_Port)
            continue;
        UMLWidget *portW = m_scene->widgetOnDiagram(o->id());
        if (portW)
            portW->adjustUnselectedAssocs(dx, dy);
    }
}

/**
 * Saves to the "componentwidget" XMI element.
 */
void ComponentWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("componentwidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeEndElement();
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF ComponentWidget::minimumSize() const
{
    if (!m_umlObject) {
        return QSizeF(70, 70);
    }
    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight = fm.lineSpacing();

    QString name = m_umlObject->name();
    if (UMLWidget::isInstance()) {
        name = UMLWidget::instanceName() + QLatin1String(" : ") + name;
    }

    int width = fm.width(name);

    int stereoWidth = 0;
    if (!m_umlObject->stereotype().isEmpty()) {
        stereoWidth = fm.width(m_umlObject->stereotype(true));
    }
    if (stereoWidth > width)
        width = stereoWidth;
    width += COMPONENT_MARGIN * 6;
    width = 70>width ? 70 : width; //minumin width of 70

    int height = (2*fontHeight) + (COMPONENT_MARGIN * 3);

    const UMLComponent *umlcomp = m_umlObject->asUMLComponent();
    if (umlcomp && umlcomp->getExecutable()) {
        width  += 2;
        height += 2;
    }

    return QSizeF(width, height);
}

