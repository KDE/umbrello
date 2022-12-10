/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header file
#include "usecasewidget.h"

// app includes
#include "debug_utils.h"
#include "usecase.h"
#include "umlview.h"

// qt includes
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(UseCaseWidget)

/**
 *  Creates a UseCase widget.
 *  @param  scene  The parent of the widget.
 *  @param  o      The UMLUseCase to represent.
 */
UseCaseWidget::UseCaseWidget(UMLScene * scene, UMLUseCase *o)
  : UMLWidget(scene, WidgetBase::wt_UseCase, o)
{
}

/**
 * Destructor.
 */
UseCaseWidget::~UseCaseWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void UseCaseWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    setPenFromSettings(painter);
    if (UMLWidget::useFillColor())
        painter->setBrush(UMLWidget::fillColor());
    QFont font = UMLWidget::font();
    font.setUnderline(false);
    font.setBold(false);
    font.setItalic(m_umlObject->isAbstract());
    painter->setFont(font);
    const QFontMetricsF &fm = getFontMetrics(FT_NORMAL);
    const qreal fontHeight  = fm.lineSpacing();
    const qreal w = width();
    const qreal h = height();
    bool drawStereotype = umlObject() && !umlObject()->stereotype().isEmpty();
    painter->drawEllipse(QRectF(0, 0, w, h));
    painter->setPen(textColor());

    QString txt;
    if (drawStereotype)
    {
        // Prepend text of stereotype to other text:
        txt = umlObject()->stereotype(true);
    }
    if (!txt.isEmpty())
        txt.append(QStringLiteral("\n"));
    QString name_txt = name();

    // Replace user-entered "\n" with real line breaks:
    name_txt.replace(QStringLiteral("\\n"),QStringLiteral("\n"));
    txt += name_txt;
    qreal dy = 0.0;
    if (drawStereotype)
        dy = fontHeight/2.0;

    QRectF rectangle(UC_MARGIN, UC_MARGIN - dy, w - UC_MARGIN*2, h - UC_MARGIN*2);
    painter->drawText(rectangle, Qt::AlignCenter | Qt::TextWordWrap, txt);
    setPenFromSettings(painter);

    UMLWidget::paint(painter, option, widget);
}

/**
 * Saves this UseCase to file.
 */
void UseCaseWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("usecasewidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeEndElement();
}

/**
 * Overrides method from UMLWidget
 */
QSizeF UseCaseWidget::minimumSize() const
{
    const UMLWidget::FontType ft = (m_umlObject->isAbstract() ? FT_BOLD_ITALIC : FT_BOLD);
    const QFontMetrics &fm = UMLWidget::getFontMetrics(ft);
    const int fontHeight = fm.lineSpacing();
    const int textWidth = fm.width(name());
    bool drawStereotype = umlObject() && !umlObject()->stereotype().isEmpty();
    int width = (textWidth / 3) > UC_WIDTH ? textWidth / 3 : UC_WIDTH;
    int height = UC_HEIGHT + (drawStereotype ? 2 * fontHeight : fontHeight) + UC_MARGIN;
    width += UC_MARGIN * 2;

    return QSizeF(width, height);
}
