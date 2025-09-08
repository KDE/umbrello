/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "datatypewidget.h"

// app includes
#include "umlclassifier.h"
#include "classifierlistitem.h"
#include "debug_utils.h"
#include "operation.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// qt includes
#include <QPainter>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(DatatypeWidget)

/**
 * Constructs an DatatypeWidget.
 *
 * @param scene   The parent of this DatatypeWidget.
 * @param d       The UMLClassifier this will be representing.
 */
DatatypeWidget::DatatypeWidget(UMLScene *scene, UMLClassifier *d) 
  : UMLWidget(scene, WidgetBase::wt_Datatype, d)
{
    setSize(100, 30);
}

/**
 * Standard deconstructor.
 */
DatatypeWidget::~DatatypeWidget()
{
}

/**
 * Overrides standard method.
 */
void DatatypeWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setPenFromSettings(painter);
    if (UMLWidget::useFillColor())  {
        painter->setBrush(UMLWidget::fillColor());
    } else {
        painter->setBrush(m_scene->backgroundColor());
    }

    int w = width();
    int h = height();

    QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    int fontHeight  = fm.lineSpacing();

    painter->drawRect(0, 0, w, h);
    painter->setPen(textColor());

    QFont font = UMLWidget::font();
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(DATATYPE_MARGIN, 0,
               w - DATATYPE_MARGIN* 2, fontHeight,
               Qt::AlignCenter, m_umlObject->stereotype(true));

    font.setItalic(m_umlObject->isAbstract());
    painter->setFont(font);
    painter->drawText(DATATYPE_MARGIN, fontHeight,
               w - DATATYPE_MARGIN * 2, fontHeight, Qt::AlignCenter, name());

    UMLWidget::paint(painter, option, widget);
}

/**
 * Loads from a "datatypewidget" XMI element.
 */
bool DatatypeWidget::loadFromXMI(QDomElement & qElement)
{
    return UMLWidget::loadFromXMI(qElement);
}

/**
 * Saves to the "datatypewidget" XMI element.
 */
void DatatypeWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("datatypewidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeEndElement();
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF DatatypeWidget::minimumSize() const
{
    if (!m_umlObject)  {
        return UMLWidget::minimumSize();
    }
    int width, height;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight = fm.lineSpacing();

    int lines = 1;//always have one line - for name
    lines++; //for the stereotype

    height = width = 0;
    height += lines * fontHeight;

    //now set the width of the classifier
    //set width to name to start with
    //set width to name to start with
    width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(m_umlObject->fullyQualifiedName()).width();
    int w = getFontMetrics(FT_BOLD).boundingRect(m_umlObject->stereotype(true)).width();

    width = w > width?w:width;

    //allow for width margin
    width += DATATYPE_MARGIN * 2;

    return QSizeF(width, height);
}
