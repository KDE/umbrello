/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "usecasewidget.h"

// app includes
#include "textitemgroup.h"
#include "textitem.h"
#include "umlview.h"
#include "usecase.h"

// qt includes
#include <QtGui/QPainter>

const qreal UseCaseWidget::Margin = 5;

/**
 *  Creates a UseCase widget.
 *
 *  @param  o The UMLObject to represent.
 */
UseCaseWidget::UseCaseWidget(UMLUseCase *o) : NewUMLRectWidget(o)
{
    m_baseType = Uml::wt_UseCase;
    m_textItemGroup = new TextItemGroup(this);
}

/**
 *  destructor
 */
UseCaseWidget::~UseCaseWidget()
{
}

void UseCaseWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    painter->drawEllipse(rect());
    // The text part is drawn by the TextItemGroup and TextItem within it.
}

/**
 *   Saves this UseCase to file.
 */
void UseCaseWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement usecaseElement = qDoc.createElement( "usecasewidget" );
    NewUMLRectWidget::saveToXMI( qDoc, usecaseElement );
    qElement.appendChild( usecaseElement );
}

QSizeF UseCaseWidget::sizeHint(Qt::SizeHint which)
{
    if(which == Qt::MinimumSize) {
        return m_minimumSize;
    }
    return NewUMLRectWidget::sizeHint(which);
}

void UseCaseWidget::updateGeometry()
{
    if(umlObject()) {
        m_textItemGroup->ensureTextItemCount(UseCaseWidget::TextItemCount);

        TextItem *nameItem = m_textItemGroup->textItemAt(UseCaseWidget::NameItemIndex);
        // Hide and then change visuals to compress many updates to one.
        nameItem->hide();
        // Apply the properties
        nameItem->setDefaultTextColor(fontColor());
        nameItem->setFont(font());
        nameItem->setAlignment(Qt::AlignCenter);
        nameItem->setBackgroundBrush(Qt::NoBrush);
        nameItem->setText(name());
        nameItem->setItalic(umlObject()->getAbstract());
        // Now show the item back
        nameItem->show();

        m_minimumSize = m_textItemGroup->calculateMinimumSize();
        m_minimumSize += QSizeF(UseCaseWidget::Margin * 2, UseCaseWidget::Margin * 2);
    }
    NewUMLRectWidget::updateGeometry();
}

void UseCaseWidget::sizeHasChanged(const QSizeF& oldSize)
{
    const QSizeF groupSize = size() - QSizeF(UseCaseWidget::Margin * 2, UseCaseWidget::Margin * 2);
    const QPointF offset(UseCaseWidget::Margin, UseCaseWidget::Margin);

    m_textItemGroup->alignVertically(groupSize);
    m_textItemGroup->setPos(offset);

    NewUMLRectWidget::sizeHasChanged(oldSize);
}
