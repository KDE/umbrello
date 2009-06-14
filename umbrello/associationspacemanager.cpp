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
 * This method is used to register the AssociationWidget associatied with this
 * UMLWidget along specified region passed.
 *
 * @param  assoc The AssociationWidget to be registered.
 * @param region The region with which the AssociationWidget has to be
 *               registered.
 * @note This method does not call arrange(region) as that is the decision to
 *       be taken dynamically.
 * @note region should not be Uml::reg_Error.
 */
void AssociationSpaceManager::add(New::AssociationWidget *assoc,
        Uml::Region region)
{
    Q_ASSERT(region != Uml::reg_Error);

    if (registered(assoc)) {
        uDebug() << assoc->name() << " is already registered!";
        return;
    }

    QList<New::AssociationWidget*> &listRef = m_regionAssociationsMap[region];
    listRef << assoc;
    m_registeredAssociationSet << assoc;
}

/**
 * This method unregisters the AssociationWidget by removing it from region
 * specific list.
 *
 * @return The last region occupied by AssociationWidget.
 *
 * @note The AssociationWidget is however @b not deleted.
 * @note Also the arrange method is not called.
 */
Uml::Region AssociationSpaceManager::remove(New::AssociationWidget *assoc)
{
    if (!registered(assoc)) {
        uDebug() << assoc->name() << " is not registered!";
        return Uml::reg_Error;
    }

    Uml::Region reg = region(assoc);
    //TODO: Remove these checks after extensive testing.
    Q_ASSERT(reg != Uml::reg_Error);

    QList<New::AssociationWidget*> &listRef = m_regionAssociationsMap[reg];
    listRef.removeOne(assoc);
    m_registeredAssociationSet.remove(assoc);

    return reg;
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
            uWarning() << "Passed association " << assoc->name()
                       << " is not managed by this AssociationSpaceManager";
        }
    } else {
        if (widA == m_umlWidget) {
            retVal = assoc->mapToScene(assoc->associationLine()->point(1));
        } else if (widB == m_umlWidget) {
            retVal = assoc->mapToScene(assoc->associationLine()->point(pointCount-2));
        } else {
            uWarning() << "Passed association " << assoc->name()
                       << " is not managed by this AssociationSpaceManager";
        }
    }
    return retVal;
}

/**
 * This method arranges the AssociationWidget line end points for a given
 * region based on its x or y value of the reference point depending upon
 * the region.
 *
 * @see AssociationSpaceManager::referencePoint
 */
void AssociationSpaceManager::arrange(Uml::Region region)
{
    if (region == Uml::reg_Error) return;

    QRectF rect = m_umlWidget->sceneRect();

    // Holds whether arrangement is based on x(horizontal) or not (which means
    // its vertically arranged based on y).
    bool xBasis = (region == Uml::reg_North || region == Uml::reg_South);

    QList<New::AssociationWidget*> &listRef = m_regionAssociationsMap[region];
    if (listRef.isEmpty()) {
        return; // nothing to arrange.
    }

    // assocDistances contains a list of pairs of New::AssociationWidget and
    // its x or y value depending on region.
    QList<QPair<New::AssociationWidget*, qreal> > assocDistances;
    // This for loop computes the pair values and inserts them in sorted
    // manner based on pair.second variable.
    foreach (New::AssociationWidget* assoc, listRef) {
        // Obtain reference point first.
        QPointF lineStart = referencePoint(assoc);
        // Get x or y coord based on xBasis variable.
        qreal distance = (xBasis ? lineStart.x() : lineStart.y());
        int i = 0;
        // Find appropriate position to insert this new value in.
        while (i < assocDistances.size() && assocDistances[i].second < distance) {
            ++i;
        }
        assocDistances.insert(i, qMakePair(assoc, distance));
    }

    // Now order the New::AssociationWidget in listRef as per ordering in
    // assocDistances list, which is sorted based on x or y value.
    listRef.clear();
    QListIterator<QPair<New::AssociationWidget*, qreal> > it(assocDistances);
    while (it.hasNext()) {
        listRef.append(it.next().first);
    }

    // Do the actual distribution now.
    const qreal totalSpace = xBasis ? rect.width() : rect.height();
    const qreal slotSize = totalSpace / listRef.size();
    qreal pos = (.5 * slotSize) + (xBasis ? rect.left() : rect.top());
    foreach (New::AssociationWidget *assoc, listRef) {
        QPointF end(pos, pos);
        switch (region) {
            case Uml::reg_North: end.setY(rect.top()); break;
            case Uml::reg_East: end.setX(rect.right()); break;
            case Uml::reg_South: end.setY(rect.bottom()); break;
            case Uml::reg_West: end.setX(rect.left()); break;

            case Uml::reg_NorthWest: end = rect.topLeft(); break;
            case Uml::reg_NorthEast: end = rect.topRight(); break;
            case Uml::reg_SouthEast: end = rect.bottomRight(); break;
            case Uml::reg_SouthWest: end = rect.bottomLeft(); break;

            default: break;
        }
        end = assoc->mapFromScene(end);
        New::AssociationLine *line = assoc->associationLine();
        int endIndex = (assoc->roleForWidget(m_umlWidget) == Uml::A ?
            0 : line->count()-1);
        line->setPoint(endIndex, end);
        pos += slotSize;
    }
}

/**
 * Shortcut for calling arrange(region) for all regions.
 * @see AssociationSpaceManager::arrange
 */
void AssociationSpaceManager::arrangeAllRegions()
{
    for (int i = Uml::reg_West; i <= Uml::reg_SouthWest; ++i) {
        arrange((Uml::Region)i);
    }
}

/**
 * @return The Uml::Region where assoc's end point resides.
 */
Uml::Region AssociationSpaceManager::region(New::AssociationWidget *assoc) const
{
    if (!registered(assoc)) {
        return Uml::reg_Error;
    }
    QMapIterator<Uml::Region, QList<New::AssociationWidget*> >
        it(m_regionAssociationsMap);
    while (it.hasNext()) {
        it.next();
        if (it.value().contains(assoc)) {
            return it.key();
        }
    }
    return Uml::reg_Error;
}

/**
 * @return Registration status of assoc.
 */
bool AssociationSpaceManager::registered(New::AssociationWidget* assoc) const
{
    return m_registeredAssociationSet.contains(assoc);
}

/**
 * @return The set containing all the New::AssociationWidget managed by this
 *         AssociationSpaceManager.
 */
QSet<New::AssociationWidget*> AssociationSpaceManager::associationWidgets() const
{
    return m_registeredAssociationSet;
}

#include "associationspacemanager.moc"

