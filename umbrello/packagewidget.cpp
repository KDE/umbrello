/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "packagewidget.h"

// qt/kde includes
#include <qpainter.h>
#include <kdebug.h>

// app includes
#include "package.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"

// Inline and class documentation

/**
 * Defines a graphical version of the Package.  Most of the
 * functionality will come from the @ref UMLPackage class.
 *
 * @short A graphical version of a Package.
 * @author Jonathan Riddell
 * @author Gopala Krishna (ported to use TextItem)
 *
 * @see NewUMLRectWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

// End Inline and class documentation

const qreal PackageWidget::Margin = 5.;

PackageWidget::PackageWidget(UMLPackage *o) :
    NewUMLRectWidget(o),
    m_minimumSize(100, 30)
{
    m_baseType = Uml::wt_Package;
    m_textItemGroup = new TextItemGroup(this);
}

PackageWidget::~PackageWidget()
{
}

void PackageWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("packagewidget");
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

QSizeF PackageWidget::sizeHint(Qt::SizeHint which)
{
    if(which == Qt::MinimumSize) {
        return m_minimumSize;
    }

    return NewUMLRectWidget::sizeHint(which);
}

void PackageWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(brush());
    painter->setPen(QPen(lineColor(), lineWidth()));

    painter->drawRect(m_topRect);
    painter->drawRect(m_packageTextRect);
    if (umlObject()->getStereotype() == "subsystem") {
        const qreal fHalf = m_topRect.height() / 2;
        const qreal symY = fHalf;
        const qreal symX = 38;
        painter->drawLine(symX, symY, symX, symY + fHalf - 2);          // left leg
        painter->drawLine(symX + 8, symY, symX + 8, symY + fHalf - 2);  // right leg
        painter->drawLine(symX, symY, symX + 8, symY);                  // waist
        painter->drawLine(symX + 4, symY, symX + 4, symY - fHalf + 2);  // head
    }
}

void PackageWidget::updateGeometry()
{
    if(umlObject()) {
        int totalItemCount = 2; // Stereotype and name

        TextItem dummy("");
        dummy.setDefaultTextColor(fontColor());
        dummy.setFont(font());
        dummy.setAcceptHoverEvents(true);
        // dummy.setHoverBrush(hoverBrush);
        dummy.setAlignment(Qt::AlignCenter);
        dummy.setBackgroundBrush(Qt::NoBrush);

        if(m_textItemGroup->size() != totalItemCount) {
            while(m_textItemGroup->size() < totalItemCount) {
                m_textItemGroup->appendTextItem(new TextItem(""));
            }
            while(m_textItemGroup->size() > totalItemCount) {
                m_textItemGroup->deleteTextItemAt(0);
            }
        }

        TextItem *stereo = m_textItemGroup->textItemAt(PackageWidget::StereoTypeItemIndex);
        stereo->setText(umlObject()->getStereotype(true));
        dummy.copyAttributesTo(stereo); // apply the attributes
        stereo->setBold(true);
        stereo->setVisible(umlObject()->getStereotype().isEmpty() == false);

        TextItem *nameItem = m_textItemGroup->textItemAt(PackageWidget::NameItemIndex);
        nameItem->setText(name());
        dummy.copyAttributesTo(nameItem); // apply the attributes

        m_minimumSize = m_textItemGroup->calculateMinimumSize();
        m_minimumSize.rwidth() += PackageWidget::Margin * 2;
        if(m_minimumSize.width() < 70) {
            m_minimumSize.rwidth() = 70;
        }

        m_minimumSize.rheight() += stereo->height();

        m_topRect.setRect(0, 0, 50, stereo->height());
        m_packageTextRect.setTopLeft(QPointF(0, m_topRect.bottom()));
    }
    NewUMLRectWidget::updateGeometry();
}

void PackageWidget::sizeHasChanged(const QSizeF& oldSize)
{
    m_packageTextRect.setBottomRight(rect().bottomRight());
    QPointF offset(PackageWidget::Margin, m_packageTextRect.top());
    uDebug() << offset;
    QSizeF groupSize = m_packageTextRect.size();
    groupSize.rwidth() -= 2 * PackageWidget::Margin;

    m_textItemGroup->alignVertically(groupSize);
    m_textItemGroup->setPos(offset);

    NewUMLRectWidget::sizeHasChanged(oldSize);
}


