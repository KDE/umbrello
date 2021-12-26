/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "packagewidget.h"

// app includes
#include "debug_utils.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlscene.h"
#include "umlview.h"

// qt includes
#include <QPainter>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(PackageWidget)

/**
 * Constructs a PackageWidget.
 *
 * @param scene   The parent of this PackageWidget.
 * @param o       The UMLObject this will be representing.
 */
PackageWidget::PackageWidget(UMLScene * scene, UMLPackage *o)
  : UMLWidget(scene, WidgetBase::wt_Package, o)
{
    setSize(100, 30);
    setZValue(1);  // above box but below UMLWidget because may embed widgets
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
PackageWidget::~PackageWidget()
{
}

/**
 * Overrides standard method.
 */
void PackageWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    setPenFromSettings(painter);
    if (UMLWidget::useFillColor())
        painter->setBrush(UMLWidget::fillColor());
    else
        painter->setBrush(m_scene->backgroundColor());

    int w = width();
    int h = height();
    QFont font = UMLWidget::font();
    font.setBold(true);
    //FIXME italic is true when a package is first created until you click elsewhere, not sure why
    font.setItalic(false);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight  = fm.lineSpacing();

    painter->drawRect(0, 0, 50, fontHeight);
    if (m_umlObject != 0
         && m_umlObject->stereotype() == QLatin1String("subsystem")) {

        const int fHalf = fontHeight / 2;
        const int symY = fHalf;
        const int symX = 38;
        painter->drawLine(symX, symY, symX, symY + fHalf - 2);          // left leg
        painter->drawLine(symX + 8, symY, symX + 8, symY + fHalf - 2);  // right leg
        painter->drawLine(symX, symY, symX + 8, symY);                  // waist
        painter->drawLine(symX + 4, symY, symX + 4, symY - fHalf + 2);  // head
    }
    painter->drawRect(0, fontHeight - 1, w, h - fontHeight);

    painter->setPen(textColor());
    painter->setFont(font);

    int lines = 1;
    if (m_umlObject != 0) {
        QString stereotype = m_umlObject->stereotype();
        if (!stereotype.isEmpty()) {
            painter->drawText(0, fontHeight + PACKAGE_MARGIN,
                       w, fontHeight, Qt::AlignCenter, m_umlObject->stereotype(true));
            lines = 2;
        }
    }

    painter->drawText(0, (fontHeight*lines) + PACKAGE_MARGIN,
               w, fontHeight, Qt::AlignCenter, name());

    UMLWidget::paint(painter, option, widget);
}

/**
 * Overrides method from UMLWidget
 */
QSizeF PackageWidget::minimumSize() const
{
    if (!m_umlObject) {
        return UMLWidget::minimumSize();
    }

    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight = fm.lineSpacing();

    int lines = 1;

    int width = fm.width(m_umlObject->name());

    int tempWidth = 0;
    if (!m_umlObject->stereotype().isEmpty()) {
        tempWidth = fm.width(m_umlObject->stereotype(true));
        lines = 2;
    }
    if (tempWidth > width)
        width = tempWidth;
    width += PACKAGE_MARGIN * 2;
    if (width < 70)
        width = 70;  // minumin width of 70

    int height = (lines*fontHeight) + fontHeight + (PACKAGE_MARGIN * 2);

    return QSizeF(width, height);
}

/**
 * Saves to the "packagewidget" XMI element.
 */
void PackageWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("packagewidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeEndElement();
}
