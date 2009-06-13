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

/**
 * Constructs a new space manager object for given widget.
 */
AssociationSpaceManager::AssociationSpaceManager(UMLWidget *widget)
{
    m_umlWidget = widget;
    Q_ASSERT(widget);
}

/**
 * This method is used to register the AssociationWidget associatied with this UMLWidget along
 * specified region passed.
 *
 * @param  assoc The AssociationWidget to be registered.
 * @param region The region with which the AssociationWidget has to be registered. If region =
 *               Uml::Error, then region is computed using @ref nearestRegion() function.
 *
 * @note This method does not call arrange(region) as that is the decision to be taken dynamically.
 * @note region should not be Uml::Center.
 */
Uml::Region AssociationSpaceManager::add(New::AssociationWidget *assoc,
        Uml::Region region)
{
    Q_ASSERT(region != Uml::Center);

    if (registered(assoc)) {
        uDebug() << assoc->name() << " is already registered!";
        return Uml::Error;
    }

    if (region == Uml::Error) {
        region = nearestRegion(assoc);
        Q_ASSERT(region != Uml::Error);
    }

    QList<New::AssociationWidget*> &listRef = m_regionAssociationsMap[region];
    listRef << assoc;
    m_registeredAssociationSet << assoc;

    return region;
}

/**
 * This method unregisters the AssociationWidget by removing it from region specific list.
 * @return The last region occupied by AssociationWidget.
 *
 * @note The AssociationWidget is however @b not deleted.
 * @note Also the arrange method is not called.
 */
Uml::Region AssociationSpaceManager::remove(New::AssociationWidget *assoc)
{
    if (!registered(assoc)) {
        uDebug() << assoc->name() << " is not registered!";
        return Uml::Error;
    }

    Uml::Region reg = region(assoc);
    //TODO: Remove these checks after extensive testing.
    Q_ASSERT(reg != Uml::Error);
    Q_ASSERT(reg != Uml::Center);

    QList<New::AssociationWidget*> &listRef = m_regionAssociationsMap[reg];
    listRef.removeOne(assoc);
    m_registeredAssociationSet.remove(assoc);

    return reg;
}

/**
 * This method returns the appropriate end point location in scene coordinates based on whether the
 * association is pointing to widget or pointing away from it (that is whether it is Uml::A or
 * Uml::B respectively).
 *
 * @param assoc The AssociationWidget for which end point should be returned.
 * @return The end point of assoc in scene coordinates which is associated to m_umlWidget.
 *
 * @see AssociationSpaceManager::penultimateEndPoint()
 */
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

/**
 * This method returns the appropriate penultimate end point location in scene coordinates based on
 * whether the association is pointing to widget or pointing away from it (that is whether it is
 * Uml::A or Uml::B respectively).
 *
 * @param assoc The AssociationWidget for which penultimate end point should be returned.
 * @return The penultimate end point of assoc in scene coordinates which is associated to
 *         m_umlWidget.
 */
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
        uWarning() << "Passed association " << assoc->name() << " is not managed by this AssociationSpaceManager";
    }
    retVal = assoc->mapToScene(retVal);
    return retVal;
}

/**
 * This method returns the point for given \a assoc which acts as reference
 * for arranging the association widget lines of particular region.
 *
 * The reference point is either the penultimate point or other widget's center
 * based on whether number of points is greater than two or equal to two
 * respectively.
 */
QPointF AssociationSpaceManager::referencePoint(New::AssociationWidget *assoc) const
{
    UMLWidget *widA = assoc->widgetForRole(Uml::A);
    UMLWidget *widB = assoc->widgetForRole(Uml::B);
    QPointF retVal;
    const int pointCount = assoc->associationLine()->count();
    Q_ASSERT(pointCount >= 2);
    if (pointCount == 2) {
        if (widA == m_umlWidget) {
            retVal = widB->sceneRect().center();
        } else if (widB == m_umlWidget) {
            retVal = widA->sceneRect().center();
        } else {
            uWarning() << "Passed association " << assoc->name() << " is not managed by this AssociationSpaceManager";
        }
    } else {
        if (widA == m_umlWidget) {
            retVal = assoc->mapToScene(assoc->associationLine()->point(1));
        } else if (widB == m_umlWidget) {
            retVal = assoc->mapToScene(assoc->associationLine()->point(pointCount-2));
        } else {
            uWarning() << "Passed association " << assoc->name() << " is not managed by this AssociationSpaceManager";
        }
    }
    return retVal;
}

/**
 * This method calculates the region which is closest for the AssociationWidget based on distance
 * of penultimate point from m_umlWidget.
 * The distance calculation happens in scene coordinates.
 */
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

/**
 * This method arranges the AssociationWidget line end points for a given region based on its
 * distance of penultimate point from edge.
 */
void AssociationSpaceManager::arrange(Uml::Region region)
{
    if (region == Uml::Error || region == Uml::Center) return;

    QRectF rect = m_umlWidget->sceneRect();
    QPointF lineEnd;

    bool xBasis = false;
    switch (region) {
        case Uml::North:
            xBasis = true;
        case Uml::NorthWest:
        case Uml::West:
            lineEnd = rect.topLeft(); break;

        case Uml::NorthEast:
        case Uml::East:
            lineEnd = rect.topRight(); break;

        case Uml::South:
            xBasis = true;
        case Uml::SouthWest:
            lineEnd = rect.bottomLeft(); break;

        case Uml::SouthEast:
            lineEnd = rect.bottomRight(); break;

        default: ;
    }

    QList<New::AssociationWidget*> &listRef = m_regionAssociationsMap[region];
    if (listRef.isEmpty()) {
        return;
    }

    QList<QPair<New::AssociationWidget*, qreal> > assocDistances;
    foreach (New::AssociationWidget* assoc, listRef) {
        QPointF lineStart = referencePoint(assoc);
        QPointF pointDist = lineEnd - lineStart;
        // qreal distance = (xBasis ? pointDist.x() : pointDist.y());
        qreal distance = (xBasis ? lineStart.x() : lineStart.y());
        int i = 0;
        while (i < assocDistances.size() && assocDistances[i].second < distance) {
            ++i;
        }
        assocDistances.insert(i, qMakePair(assoc, distance));
    }
    listRef.clear();
    QListIterator<QPair<New::AssociationWidget*, qreal> > it(assocDistances);
    while (it.hasNext()) {
        listRef.append(it.next().first);
    }

    const qreal totalSpace = xBasis ? rect.width() : rect.height();
    const qreal slotSize = totalSpace / listRef.size();
    qreal pos = (.5 * slotSize) + (xBasis ? rect.left() : rect.top());
    foreach (New::AssociationWidget *assoc, listRef) {
        QPointF end(pos, pos);
        switch (region) {
            case Uml::North: end.setY(rect.top()); break;
            case Uml::East: end.setX(rect.right()); break;
            case Uml::South: end.setY(rect.bottom()); break;
            case Uml::West: end.setX(rect.left()); break;

            case Uml::NorthWest: end = rect.topLeft(); break;
            case Uml::NorthEast: end = rect.topRight(); break;
            case Uml::SouthEast: end = rect.bottomRight(); break;
            case Uml::SouthWest: end = rect.bottomLeft(); break;

            default: break;
        }
        end = assoc->mapFromScene(end);
        New::AssociationLine *line = assoc->associationLine();
        int endIndex = assoc->roleForWidget(m_umlWidget) == Uml::A ? 0 : line->count()-1;
        line->setPoint(endIndex, end);
        pos += slotSize;
    }
}

/**
 * This utility method arranges the AssociationWidget line end points for a all regions based on
 * its distance of penultimate point from edge.
 */
void AssociationSpaceManager::arrangeAllRegions()
{
    arrange(Uml::North);
    arrange(Uml::East);
    arrange(Uml::South);
    arrange(Uml::West);
    arrange(Uml::NorthEast);
    arrange(Uml::SouthEast);
    arrange(Uml::SouthWest);
    arrange(Uml::NorthWest);
}

/**
 * @return The Uml::Region where assoc's end point resides.
 */
Uml::Region AssociationSpaceManager::region(New::AssociationWidget *assoc) const
{
    if (!registered(assoc)) {
        return Uml::Error;
    }
    QMapIterator<Uml::Region, QList<New::AssociationWidget*> > it(m_regionAssociationsMap);
    while (it.hasNext()) {
        it.next();
        if (it.value().contains(assoc)) {
            return it.key();
        }
    }
    return Uml::Error;
}

/**
 * @return Registration status of assoc.
 */
bool AssociationSpaceManager::registered(New::AssociationWidget* assoc) const
{
    return m_registeredAssociationSet.contains(assoc);
}

/**
 * Computes nearest region for all New::AssociationWidget's, moves them to calculated regions and
 * arranges them.
 *
 * The logic involved is to remove all resitered associations and add it back by calling
 * AssociationSpaceManager::add() method without passing second argument. That will recalculate the
 * new regions based on its distances of penultimate end point and finally, arrangeAllRegions()
 * method will do the actual arrangement of associations.
 */
void AssociationSpaceManager::adjust()
{
    QSet<New::AssociationWidget*> registered = m_registeredAssociationSet;
    foreach (New::AssociationWidget *a, registered) {
        remove(a);
    }
    foreach (New::AssociationWidget *a, registered) {
        add(a);
    }
    arrangeAllRegions();
}

QSet<New::AssociationWidget*> AssociationSpaceManager::associationWidgets() const
{
    return m_registeredAssociationSet;
}

#include "associationspacemanager.moc"

