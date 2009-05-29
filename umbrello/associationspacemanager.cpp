/***************************************************************************
 * Copyright (C) 2009 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "associationspacemanager.h"
#include "newassociationwidget.h"
#include "newlinepath.h"
#include "umlwidget.h"

AssociationSpaceManager::AssociationSpaceManager(UMLWidget *widget)
{
    m_umlWidget = widget;
    Q_ASSERT(widget);
}

void AssociationSpaceManager::addAssociationWidget(New::AssociationWidget *assoc)
{
}

void AssociationSpaceManager::addAssociationWidget(New::AssociationWidget *assoc, Uml::Region region)
{
}

void AssociationSpaceManager::removeAssociatinWidget(New::AssociationWidget *assoc)
{
}

QPointF AssociationSpaceManager::endPoint(New::AssociationWidget *assoc) const
{
    UMLWidget *widA = assoc->widgetForRole(Uml::A);
    UMLWidget *widB = assoc->widgetForRole(Uml::B);
    QPointF retVal;

    if (widA == m_umlWidget) {
        retVal = assoc->associationLine()->point(0);
    }
    else if (widB == m_umlWidget) {
        New::AssociationLine *line = assoc->associationLine();
        retVal = line->point(line->count() - 1);
    }
    else {
        uWarning() << "Passed association " << assoc->name() << " is not related to this AssociationSpaceManager";
    }
    retVal = assoc->mapToScene(retVal);
    return retVal;
}

QPointF AssociationSpaceManager::penultimateEndPoint(New::AssociationWidget *assoc) const
{
    UMLWidget *widA = assoc->widgetForRole(Uml::A);
    UMLWidget *widB = assoc->widgetForRole(Uml::B);
    QPointF retVal;

    if (widA == m_umlWidget) {
        retVal = assoc->associationLine()->point(1);
    }
    else if (widB == m_umlWidget) {
        New::AssociationLine *line = assoc->associationLine();
        retVal = line->point(line->count() - 2);
    }
    else {
        uWarning() << "Passed association " << assoc->name() << " is not related to this AssociationSpaceManager";
    }
    retVal = assoc->mapToScene(retVal);
    return retVal;
}

Uml::Region AssociationSpaceManager::nearestRegion(New::AssociationWidget *assoc) const
{
    QPointF penultimate = penultimateEndPoint(assoc);
    QRectF widRect = m_umlWidget->mapToScene(m_umlWidget->rect()).boundingRect();
    bool left = false, top = false;
    qreal horDist = 0., verDist = 0.;

    if (qAbs(widRect.left() - penultimate.x()) < qAbs(widRect.right() - penultimate.x())) {
        left = true;
        horDist = qAbs(widRect.left() - penultimate.x());
    }
    else {
        left = false;
        horDist = qAbs(widRect.right() - penultimate.x());
    }

    if (qAbs(widRect.top() - penultimate.y()) < qAbs(widRect.bottom() - penultimate.y())) {
        top = true;
        verDist = qAbs(widRect.top() - penultimate.y());
    }
    else {
        top = false;
        verDist = qAbs(widRect.bottom() - penultimate.y());
    }

    if (qFuzzyCompare(horDist, verDist)) {
        if (left) {
            return top ? Uml::NorthWest : Uml::SouthWest;
        }
        return top ? Uml::NorthEast : Uml::SouthEast;
    }
    else if (horDist < verDist) {
        return left ? Uml::West : Uml::East;
    }
    else {
        return top ? Uml::North : Uml::South;
    }
}

void AssociationSpaceManager::arrange(Uml::Region region)
{
}

void AssociationSpaceManager::arrangeAllRegions()
{
}
