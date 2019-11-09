/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2019                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "associationwidgetrole.h"

#include "floatingtextwidget.h"
#include "umlwidget.h"
#include "umlscene.h"

AssociationWidgetRole::AssociationWidgetRole()
  : multiplicityWidget(nullptr)
  , changeabilityWidget(nullptr)
  , roleWidget(nullptr)
  , umlWidget(nullptr)
  , m_WidgetRegion(Uml::Region::Error)
  , m_nIndex(0)
  , m_nTotalCount(0)
  , visibility(Uml::Visibility::Public)
  , changeability(Uml::Changeability::Changeable)
  , m_q(nullptr)
{
}

void AssociationWidgetRole::cleanup()
{
    if (umlWidget) {
        umlWidget->removeAssoc(m_q);
        umlWidget = nullptr;
    }
    if (roleWidget) {
        roleWidget->umlScene()->removeWidget(roleWidget);
        roleWidget = nullptr;
    }
    if (multiplicityWidget) {
        multiplicityWidget->umlScene()->removeWidget(multiplicityWidget);
        multiplicityWidget = nullptr;
    }
    if (changeabilityWidget) {
        changeabilityWidget->umlScene()->removeWidget(changeabilityWidget);
        changeabilityWidget = nullptr;
    }
}

void AssociationWidgetRole::setFont(const QFont &font)
{
    if (roleWidget)
        roleWidget->setFont(font);
    if (multiplicityWidget)
        multiplicityWidget->setFont(font);
    if (changeabilityWidget)
        changeabilityWidget->setFont(font);
}

/**
 * Check owned floating texts
 *
 * @param p Point to be checked
 *
 * @return pointer to widget at the provided point p
 * @return 0 if no widget found
 */
UMLWidget* AssociationWidgetRole::onWidget(const QPointF &p)
{
    if (multiplicityWidget && multiplicityWidget->onWidget(p))
        return multiplicityWidget;
    else if (changeabilityWidget && changeabilityWidget->onWidget(p))
        return changeabilityWidget;
    else if (roleWidget && roleWidget->onWidget(p))
        return roleWidget;
    return nullptr;
}

/**
 * Sets the state of whether the widget is selected.
 *
 * @param _select   The state of whether the widget is selected.
 */
void AssociationWidgetRole::setSelected(bool select)
{
    if (roleWidget)
       roleWidget->setSelected(select);
    if (multiplicityWidget )
       multiplicityWidget->setSelected(select);
    if (changeabilityWidget)
       changeabilityWidget->setSelected(select);
}

void AssociationWidgetRole::clipSize()
{
    if (multiplicityWidget)
        multiplicityWidget->clipSize();

    if (roleWidget)
        roleWidget->clipSize();

    if (changeabilityWidget)
        changeabilityWidget->clipSize();
}

void AssociationWidgetRole::saveToXMI1(QDomDocument &qDoc, QDomElement &qElement, const QString &suffix)
{
    qElement.setAttribute(QString(QLatin1String("index%1")).arg(suffix), m_nIndex);
    qElement.setAttribute(QString(QLatin1String("totalcount%1")).arg(suffix), m_nTotalCount);

    if (multiplicityWidget)
        multiplicityWidget->saveToXMI1(qDoc, qElement);
    if (roleWidget)
        roleWidget->saveToXMI1(qDoc, qElement);
    if (changeabilityWidget)
        changeabilityWidget->saveToXMI1(qDoc, qElement);
}

bool AssociationWidgetRole::loadFromXMI1(QDomElement &qElement, const QString &suffix)
{
    QString index = qElement.attribute(QString(QLatin1String("index%1")).arg(suffix), QLatin1String("0"));
    QString totalcount = qElement.attribute(QString(QLatin1String("totalcount%1")).arg(suffix), QLatin1String("0"));
    m_nIndex = index.toInt();
    m_nTotalCount = totalcount.toInt();
    // for remaining see AssociationWidget::loadFromXMI
    return true;
}
