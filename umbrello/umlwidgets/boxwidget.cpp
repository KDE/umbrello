/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "boxwidget.h"

// app includes
#include "debug_utils.h"
#include "umlapp.h"
#include "umldoc.h"

// qt includes
#include <QColorDialog>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(BoxWidget)

/**
 * Constructs a BoxWidget.
 *
 * @param scene   The parent to this widget.
 * @param id      The ID to assign (-1 will prompt a new ID.)
 * @param type    The WidgetType (wt_Box.)
 */
BoxWidget::BoxWidget(UMLScene * scene, Uml::ID::Type id, WidgetType type)
  : UMLWidget(scene, type, id)
{
    setSize(100, 80);
    m_usesDiagramLineColor = false;  // boxes be black
    setLineColor(QColor("black"));
    setZValue(-10);
}

/**
 * Destructor.
 */
BoxWidget::~BoxWidget()
{
}

/**
 * Draws a rectangle.
 */
void BoxWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    setPenFromSettings(painter);
    painter->drawRect(0, 0, width(), height());

    UMLWidget::paint(painter, option, widget);
}

/**
 * Saves the widget to the "boxwidget" XMI element.
 * Note: For loading from XMI, the inherited parent method is used.
 */
void BoxWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("boxwidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeEndElement();
}

/**
 * Show a properties dialog for a BoxWidget.
 */
bool BoxWidget::showPropertiesDialog()
{
    QColor newColor = QColorDialog::getColor(lineColor()); // krazy:exclude=qclasses
    if (!newColor.isValid())
        return false;
    if (newColor != lineColor()) {
        setLineColor(newColor);
        setUsesDiagramLineColor(false);
        umlDoc()->setModified(true);
    }
    return true;
}

void BoxWidget::toForeground()
{
}
