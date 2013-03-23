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

#include "associationline.h"
#include "associationwidget.h"
#include "debug_utils.h"
#include "umlwidget.h"

RegionPair::RegionPair(Uml::Region::Enum f, Uml::Region::Enum s)
  : first(f),
    second(s)
{
}

bool RegionPair::isValid() const
{
    if (first != Uml::Region::Error || second != Uml::Region::Error) {
        return true;
    }
    return false;
}

Uml::Region::Enum& RegionPair::operator[](Uml::RoleType::Enum role)
{
    if (role == Uml::RoleType::A) {
        return first;
    }
    return second;
}

const Uml::Region::Enum& RegionPair::operator[](Uml::RoleType::Enum role) const
{
    if (role == Uml::RoleType::A) {
        return first;
    }
    return second;
}

AssociationSpaceManager::PointPair::PointPair(const QPointF& p1, const QPointF& p2)
  : first(p1),
    second(p2)
{
}

QPointF& AssociationSpaceManager::PointPair::operator[](Uml::RoleType::Enum role)
{
    if (role == Uml::RoleType::A) {
        return first;
    }
    return second;
}

const QPointF& AssociationSpaceManager::PointPair::operator[](Uml::RoleType::Enum role) const
{
    if (role == Uml::RoleType::A) {
        return first;
    }
    return second;
}

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
 * UMLWidget along specified region passed for space distribution management.
 *
 * @param assoc   The AssociationWidget to be registered.
 * @param regions The regions with which the AssociationWidget has to be
 *                registered. regions[role(assoc)] is where the destination
 *                region should be specified. (for self associatons both roles
 *                specify corresponding regions)
 *
 * @note Self associations are stored separately.
 * @note This method does not call arrange(region) as that is the decision to
 *       be taken dynamically by users of this object.
 * @note Refer @ref RegionPair to understand why pair is used.
 */
void AssociationSpaceManager::add(AssociationWidget *assoc,
                                  const RegionPair& regions)
{
    if (!regions.isValid()) {
        uDebug() << "Invalid regions, so not adding";
        return;
    }

    if (isRegistered(assoc)) {
        uDebug() << assoc->name() << " is already registered!";
        return;
    }

    if (assoc->isSelf()) {
        SelfAssociationItem item;
        item.associationWidget = assoc;
        item.regions = regions;
        Q_ASSERT(regions[Uml::RoleType::B] != Uml::Region::Error);
        m_selfAssociationsList << item;
    } else {
        m_regionsAssociationsMap[regions[role(assoc)]] << assoc;
    }

    m_registeredAssociationSet << assoc;
}

/**
 * This method unregisters the AssociationWidget by removing it from regions
 * specific list or m_selfAssociationsList in case of self associations.
 *
 * @return The last regions occupied by AssociationWidget.
 *
 * @note The AssociationWidget is however @b not deleted.
 * @note Also the arrange method is not called.
 * @note Refer @ref RegionPair to understand why pair is used.
 */
void AssociationSpaceManager::remove(AssociationWidget *assoc)
{
    if (!isRegistered(assoc)) {
        uDebug() << assoc->name() << " is not registered!";
        return;
    }

    if (assoc->isSelf()) {
        bool removed = false;
        for (int i = 0; i < m_selfAssociationsList.size(); ++i) {
            if (m_selfAssociationsList[i].associationWidget == assoc) {
                m_selfAssociationsList.removeAt(i);
                removed = true;
                break;
            }
        }
        Q_ASSERT(removed); Q_UNUSED(removed);
    } else {
        RegionPair reg = region(assoc);
        Q_ASSERT(reg.isValid());
        m_regionsAssociationsMap[reg[role(assoc)]].removeOne(assoc);
    }

    m_registeredAssociationSet.remove(assoc);
}

/**
 * This method returns the points for given \a assoc which acts as reference
 * for arranging the association widget lines of particular region.
 *
 * In case of non-self association,
 * the reference point is either the penultimate point or other widget's center
 * based on whether number of points is greater than two or equal to two
 * respectively, and is stored in PointPair[role(assoc)].
 *
 * In case of self associations,
 * the reference point pair is always the penultimate points from both ends as
 * a self association line has atleast 4 points.
 */
AssociationSpaceManager::PointPair
AssociationSpaceManager::referencePoints(AssociationWidget *assoc) const
{
    AssociationLine *line = assoc->associationLine();
    if (!assoc->isSelf()) {
        PointPair retVal;
        Uml::RoleType::Enum myRole = role(assoc);
        Q_ASSERT(line->count() >= 2);
        if (line->count() == 2) {
            Uml::RoleType::Enum otherWidRole = Uml::RoleType::fromInt(1 - myRole);
            UMLWidget *otherWid = assoc->widgetForRole(otherWidRole);
            retVal[myRole] = otherWid->sceneRect().center();
        } else {
            retVal[myRole] = (myRole == Uml::RoleType::A ?
                    assoc->mapToScene(line->point(1)) :
                    assoc->mapToScene(line->point(line->count()-2)));
        }

        return retVal;
    } else {
        Q_ASSERT(line->count() >= 4);

        return PointPair(assoc->mapToScene(line->point(1)),
                assoc->mapToScene(line->point(line->count()-2)));
    }
}

/**
 * One more helper data structure to build a list sorted by distances and also
 * store endIndex to ease further alignment.
 */
struct Tuple
{
    AssociationWidget *associationWidget;
    qreal distance;
    int endIndex;
};

/**
 * @internal
 * This method is used as secondary sorting criteria for a list of Tuple.
 */
uint hash(AssociationWidget *assoc)
{
    return qHash(QString(assoc->id().c_str()));
}
/**
 * This method arranges the AssociationWidget line end points for given
 * regions based on its x or y value of the reference point depending upon
 * the region.
 *
 * @see AssociationSpaceManager::referencePoints
 */
void AssociationSpaceManager::arrange(Uml::Region::Enum region)
{
    QRectF rect = m_umlWidget->sceneRect();
    QList<AssociationWidget*> &listRef = m_regionsAssociationsMap[region];

    PointPair selfEndPoints;
    switch (region) {
        case Uml::Region::West:
            selfEndPoints[Uml::RoleType::A] = selfEndPoints[Uml::RoleType::B] = rect.topLeft();
            selfEndPoints[Uml::RoleType::A].ry() += .25 * rect.height();
            selfEndPoints[Uml::RoleType::B].ry() += .75 * rect.height();
            break;

        case Uml::Region::North:
            selfEndPoints[Uml::RoleType::A] = selfEndPoints[Uml::RoleType::B] = rect.topLeft();
            selfEndPoints[Uml::RoleType::A].rx() += .25 * rect.width();
            selfEndPoints[Uml::RoleType::B].rx() += .75 * rect.width();
            break;

        case Uml::Region::East:
            selfEndPoints[Uml::RoleType::A] = selfEndPoints[Uml::RoleType::B] = rect.topRight();
            selfEndPoints[Uml::RoleType::A].ry() += .25 * rect.height();
            selfEndPoints[Uml::RoleType::B].ry() += .75 * rect.height();
            break;

        case Uml::Region::South:
            selfEndPoints[Uml::RoleType::A] = selfEndPoints[Uml::RoleType::B] = rect.bottomLeft();
            selfEndPoints[Uml::RoleType::A].rx() += .25 * rect.width();
            selfEndPoints[Uml::RoleType::B].rx() += .75 * rect.width();
            break;

        default: ;
    }


    // Holds whether arrangement is based on x(horizontal) or not (which means
    // its vertically arranged based on y).
    bool xBasis = (region == Uml::Region::North || region == Uml::Region::South);

    // This intermediate tuple stores both self and non self associations in
    // sorted fashion, primary sort criteria being x or y value based on region
    // and secondary criteria being hash(association) for consistent sorting.
    QList<Tuple> assocDistEndTuples;
    foreach (AssociationWidget* assoc, listRef) {
        // Obtain reference point first.
        QPointF lineStart = referencePoints(assoc)[role(assoc)];
        // Get x or y coord based on xBasis variable.
        qreal distance = (xBasis ? lineStart.x() : lineStart.y());
        uint assocHash = hash(assoc);
        int i = 0;
        // Find appropriate position to insert this new value in.
        while (i < assocDistEndTuples.size() &&
                assocDistEndTuples[i].distance < distance) {
            ++i;
        }
        while (i < assocDistEndTuples.size() &&
                hash(assocDistEndTuples[i].associationWidget) < assocHash) {
            ++i;
        }

        Tuple t;
        t.associationWidget = assoc;
        t.distance = distance;
        t.endIndex = (assoc->roleForWidget(m_umlWidget) == Uml::RoleType::A ?
                0 : t.associationWidget->associationLine()->count()-1);

        assocDistEndTuples.insert(i, t);
    }

    foreach (SelfAssociationItem item, m_selfAssociationsList) {
        AssociationWidget *self = item.associationWidget;
        Uml::RoleType::Enum r;
        if (item.regions[Uml::RoleType::A] == item.regions[Uml::RoleType::B] &&
                region == item.regions[Uml::RoleType::A]) {
            // both ends of self can't be same corner
            Q_ASSERT(region <= Uml::Region::South);

            self->associationLine()->setEndPoints(
                    self->mapFromScene(selfEndPoints[Uml::RoleType::A]),
                    self->mapFromScene(selfEndPoints[Uml::RoleType::B]));
            continue;
        } else if (item.regions[Uml::RoleType::A] == region) {
            r = Uml::RoleType::A;
        } else if (item.regions[Uml::RoleType::B] == region) {
            r = Uml::RoleType::B;
        } else {
            continue;
        }

        QPointF actual = referencePoints(self)[role(self)];
        qreal dist = (xBasis ? actual.x() : actual.y());

        uint assocHash = hash(self);
        int i = 0;
        while (i < assocDistEndTuples.size() &&
                assocDistEndTuples[i].distance < dist) {
            ++i;
        }
        while (i < assocDistEndTuples.size() &&
                hash(assocDistEndTuples[i].associationWidget) < assocHash) {
            ++i;
        }

        Tuple t;
        t.associationWidget = self;
        t.distance = dist;
        t.endIndex = (r == Uml::RoleType::A ? 0 : self->associationLine()->count()-1);
        assocDistEndTuples.insert(i, t);
    }

    if (assocDistEndTuples.isEmpty()) {
        return;
    }
    listRef.clear();
    // Do the actual distribution now.
    const qreal totalSpace = xBasis ? rect.width() : rect.height();
    const qreal slotSize = totalSpace / assocDistEndTuples.size();
    qreal pos = (.5 * slotSize) + (xBasis ? rect.left() : rect.top());
    foreach (Tuple t, assocDistEndTuples) {
        QPointF end(pos, pos);
        switch (region) {
            case Uml::Region::North: end.setY(rect.top()); break;
            case Uml::Region::East: end.setX(rect.right()); break;
            case Uml::Region::South: end.setY(rect.bottom()); break;
            case Uml::Region::West: end.setX(rect.left()); break;

            case Uml::Region::NorthWest: end = rect.topLeft(); break;
            case Uml::Region::NorthEast: end = rect.topRight(); break;
            case Uml::Region::SouthEast: end = rect.bottomRight(); break;
            case Uml::Region::SouthWest: end = rect.bottomLeft(); break;

            default: break;
        }
        end = t.associationWidget->mapFromScene(end);
        t.associationWidget->associationLine()->setPoint(t.endIndex, end);
        pos += slotSize;


        if (!t.associationWidget->isSelf()) {
            listRef << t.associationWidget;
        }
    }
}

/**
 * @return The RegionPair where assoc's end points resides.
 * @note Refer @ref RegionPair to understand why pair is used.
 */
RegionPair AssociationSpaceManager::region(AssociationWidget *assoc) const
{
    if (!isRegistered(assoc)) {
        return Uml::Region::Error;
    }
    RegionPair result;
    if (assoc->isSelf()) {
        foreach (SelfAssociationItem item, m_selfAssociationsList) {
            if (item.associationWidget == assoc) {
                result = item.regions;
                break;
            }
        }
    } else {
        QMapIterator<Uml::Region::Enum, QList<AssociationWidget*> >
            it(m_regionsAssociationsMap);
        while (it.hasNext()) {
            it.next();
            if (it.value().contains(assoc)) {
                result[role(assoc)] = it.key();
                break;
            }
        }
    }
    return result;
}

/**
 * @return The role of m_umlWidget in \a assoc.
 */
Uml::RoleType::Enum AssociationSpaceManager::role(AssociationWidget *assoc) const
{
    if (assoc->widgetForRole(Uml::RoleType::A) == m_umlWidget) {
        return Uml::RoleType::A;
    } else if (assoc->widgetForRole(Uml::RoleType::B) == m_umlWidget) {
        return Uml::RoleType::B;
    }
    Q_ASSERT(0);
    return Uml::RoleType::A;
}

/**
 * @return Registration status of assoc.
 */
bool AssociationSpaceManager::isRegistered(AssociationWidget* assoc) const
{
    return m_registeredAssociationSet.contains(assoc);
}

/**
 * @return The set containing all the AssociationWidget managed by this
 *         AssociationSpaceManager.
 */
QSet<AssociationWidget*> AssociationSpaceManager::associationWidgets() const
{
    return m_registeredAssociationSet;
}

#include "associationspacemanager.moc"

