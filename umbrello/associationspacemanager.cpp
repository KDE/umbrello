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

RegionPair::RegionPair(Uml::Region f, Uml::Region s) : first(f), second(s)
{
}

bool RegionPair::isValid() const
{
    return first != Uml::reg_Error;
}

bool RegionPair::operator<(const RegionPair& rhs) const
{
    return id() < rhs.id();
}

int RegionPair::id() const
{
    return (100*first) + second;
}

/**
 * Constructs a new space manager object for given widget.
 */
AssociationSpaceManager::AssociationSpaceManager(UMLWidget *widget)
{
    m_umlWidget = widget;
    QList<New::AssociationWidget*> list;
    for (int i = Uml::reg_West; i <= Uml::reg_SouthWest; ++i) {
        for (int j = Uml::reg_Error; j <= Uml::reg_SouthWest; ++j) {
            RegionPair p((Uml::Region)i, (Uml::Region)j);
            m_regionsAssociationsMap[p] = list;
        }
        const Uml::Region r = (Uml::Region)i;
    }
    Q_ASSERT(widget);
}

/**
 * This method is used to register the AssociationWidget associatied with this
 * UMLWidget along specified region passed.
 *
 * @param  assoc  The AssociationWidget to be registered.
 * @param regions The regions with which the AssociationWidget has to be
 *                registered.
 *
 * @note This method does not call arrange(region) as that is the decision to
 *       be taken dynamically.
 * @note Refer @ref RegionPair to understand why pair is used.
 */
void AssociationSpaceManager::add(New::AssociationWidget *assoc,
        RegionPair regions)
{
    if (!regions.isValid()) return;

    if (isRegistered(assoc)) {
        uDebug() << assoc->name() << " is already registered!";
        return;
    }

    m_regionsAssociationsMap[regions] << assoc;
    m_registeredAssociationSet << assoc;
}

/**
 * This method unregisters the AssociationWidget by removing it from regions
 * specific list.
 *
 * @return The last regions occupied by AssociationWidget.
 *
 * @note The AssociationWidget is however @b not deleted.
 * @note Also the arrange method is not called.
 * @note Refer @ref RegionPair to understand why pair is used.
 */
RegionPair AssociationSpaceManager::remove(New::AssociationWidget *assoc)
{
    if (!isRegistered(assoc)) {
        uDebug() << assoc->name() << " is not registered!";
        return Uml::reg_Error;
    }

    RegionPair reg = regions(assoc);
    //TODO: Remove these checks after extensive testing.
    Q_ASSERT(reg.isValid());

    m_regionsAssociationsMap[reg].removeOne(assoc);
    m_registeredAssociationSet.remove(assoc);

    return reg;
}

/**
 * This method returns the points for given \a assoc which acts as reference
 * for arranging the association widget lines of particular region.
 *
 * In case of non-self association,
 * the reference point is either the penultimate point or other widget's center
 * based on whether number of points is greater than two or equal to two
 * respectively, and is stored in PointPair.first.
 *
 * In case of self associations,
 * the reference point pair is always the penultimate points from both ends as
 * a self association line has atleast 4 points.
 */
PointPair AssociationSpaceManager::referencePoints(New::AssociationWidget *assoc) const
{
    UMLWidget *widA = assoc->widgetForRole(Uml::A);
    UMLWidget *widB = assoc->widgetForRole(Uml::B);
    New::AssociationLine *line = assoc->associationLine();
    if (!assoc->isSelf()) {
        QPointF retVal;
        Q_ASSERT(line->count() >= 2);
        if (line->count() == 2) {
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
                retVal = assoc->mapToScene(line->point(1));
            } else if (widB == m_umlWidget) {
                retVal = assoc->mapToScene(line->point(line->count()-2));
            } else {
                uWarning() << "Passed association " << assoc->name()
                    << " is not managed by this AssociationSpaceManager";
            }
        }

        return PointPair(retVal, QPointF());
    } else {
        Q_ASSERT(line->count() >= 4);

        return PointPair(assoc->mapToScene(line->point(1)),
                assoc->mapToScene(line->point(line->count()-2)));
    }
}

/**
 * This method arranges the AssociationWidget line end points for given
 * regions based on its x or y value of the reference point depending upon
 * the region.
 *
 * @see AssociationSpaceManager::referencePoints
 * @note Refer @ref RegionPair to understand why pair is used.
 */
void AssociationSpaceManager::arrange(RegionPair regions)
{
    if (!regions.isValid()) return;

    QRectF rect = m_umlWidget->sceneRect();
    QList<New::AssociationWidget*> &listRef = m_regionsAssociationsMap[regions];
    if (listRef.isEmpty()) {
        return; // nothing to arrange.
    }

    if (regions.second != Uml::reg_Error) {
        if (regions.first == regions.second) {
            foreach (New::AssociationWidget *assoc, listRef) {
                QPointF p1, p2;

                switch (regions.first) {
                    case Uml::reg_North:
                        p1 = assoc->mapFromScene(rect.topLeft() +
                                QPointF(.25 * rect.width(), 0));
                        p2 = assoc->mapFromScene(rect.topLeft() +
                                QPointF(.75 * rect.width(), 0));
                        break;

                    case Uml::reg_South:
                        p1 = assoc->mapFromScene(rect.bottomLeft() +
                                QPointF(.25 * rect.width(), 0));
                        p2 = assoc->mapFromScene(rect.bottomLeft() +
                                QPointF(.75 * rect.width(), 0));
                        break;

                    case Uml::reg_West:
                        p1 = assoc->mapFromScene(rect.topLeft() +
                                QPointF(0, .25 * rect.height()));
                        p2 = assoc->mapFromScene(rect.topLeft() +
                                QPointF(0, .75 * rect.height()));
                        break;

                    case Uml::reg_East:
                        p1 = assoc->mapFromScene(rect.topRight() +
                                QPointF(0, .25 * rect.height()));
                        p2 = assoc->mapFromScene(rect.topRight() +
                                QPointF(0, .75 * rect.height()));
                        break;

                    case Uml::reg_NorthWest:
                        p1 = p2 = assoc->mapFromScene(rect.topLeft());
                        break;

                    case Uml::reg_NorthEast:
                        p1 = p2 = assoc->mapFromScene(rect.topRight());
                        break;

                    case Uml::reg_SouthEast:
                        p1 = p2 = assoc->mapFromScene(rect.bottomRight());
                        break;

                    case Uml::reg_SouthWest:
                        p1 = p2 = assoc->mapFromScene(rect.bottomLeft());
                        break;

                    default:;
                }

                New::AssociationLine *ll = assoc->associationLine();
                assoc->associationLine()->setEndPoints(p1, p2);
            } // foreach loop
            return;
        } // if (regions.first == regions.second)
        else {
            bool axBasis = (regions.first == Uml::reg_North ||
                    regions.first == Uml::reg_South);
            bool bxBasis = (regions.second == Uml::reg_North ||
                    regions.second == Uml::reg_South);
            Q_ASSERT(m_regionsAssociationsMap.contains(RegionPair(regions.first)));
            Q_ASSERT(m_regionsAssociationsMap.contains(RegionPair(regions.second)));
            QList<New::AssociationWidget*> &aOtherRef =
                m_regionsAssociationsMap[RegionPair(regions.first)];
            QList<New::AssociationWidget*> &bOtherRef =
                m_regionsAssociationsMap[RegionPair(regions.second)];

            foreach (New::AssociationWidget *assoc, listRef) {
                PointPair pair = referencePoints(assoc);
                qreal aDist = axBasis ? pair.first.x() : pair.first.y();

                int i = 0;
                while (i < aOtherRef.size() - 1) {
                    QPointF lineStart = referencePoints(aOtherRef[i]).first;
                    qreal dist = (axBasis ? lineStart.x() : lineStart.y());
                    if (dist > aDist) break;
                    ++i;
                }
                QPointF p1, p2;
                if (i != 0) {
                    Uml::Role_Type r = aOtherRef[i]->roleForWidget(m_umlWidget);
                    if (r == Uml::A) {
                        p1 = aOtherRef[i]->associationLine()->startPoint();
                    } else {
                        p1 = aOtherRef[i]->associationLine()->endPoint();
                    }
                    p1 = aOtherRef[i]->mapToScene(p1);
                }
                if (i+1 < aOtherRef.size()) {
                    Uml::Role_Type r = aOtherRef[i+1]->roleForWidget(m_umlWidget);
                    if (r == Uml::A) {
                        p2 = aOtherRef[i+1]->associationLine()->startPoint();
                    } else {
                        p2 = aOtherRef[i+1]->associationLine()->endPoint();
                    }
                    p2 = aOtherRef[i]->mapToScene(p2);
                }
                switch (regions.first) {
                    case Uml::reg_North:
                        p1 = (i == 0 ? rect.topLeft() : p1);
                        p2 = (i+1 >= aOtherRef.size() ? rect.topRight() : p2);
                        break;
                    case Uml::reg_West:
                        p1 = (i == 0 ? rect.topLeft() : p1);
                        p2 = (i+1 >= aOtherRef.size() ? rect.bottomLeft() : p2);
                        break;
                    case Uml::reg_East:
                        p1 = (i == 0 ? rect.topRight() : p1);
                        p2 = (i+1 >= aOtherRef.size() ? rect.bottomRight() : p2);
                        break;
                    case Uml::reg_South:
                        p1 = (i == 0 ? rect.bottomLeft() : p1);
                        p2 = (i+1 >= aOtherRef.size() ? rect.bottomRight() : p2);
                        break;
                    case Uml::reg_NorthWest: p1 = p2 = rect.topLeft(); break;
                    case Uml::reg_NorthEast: p1 = p2 = rect.topRight(); break;
                    case Uml::reg_SouthEast: p1 = p2 = rect.bottomRight(); break;
                    case Uml::reg_SouthWest: p1 = p2 = rect.bottomLeft(); break;
                    default:break;
                };
                QPointF start = assoc->mapFromScene((p1 + p2) / 2);



                qreal bDist = bxBasis ? pair.second.x() : pair.second.y();
                i = 0;
                while (i < bOtherRef.size()-1) {
                    QPointF lineStart = referencePoints(bOtherRef[i]).first;
                    qreal dist = (bxBasis ? lineStart.x() : lineStart.y());
                    if (dist > bDist) break;
                    ++i;
                }
                p1 = p2 = QPointF();
                if (i != 0) {
                    Uml::Role_Type r = bOtherRef[i]->roleForWidget(m_umlWidget);
                    if (r == Uml::A) {
                        p1 = bOtherRef[i]->associationLine()->startPoint();
                    } else {
                        p1 = bOtherRef[i]->associationLine()->endPoint();
                    }
                    p1 = bOtherRef[i]->mapToScene(p1);
                }
                if (i+1 < bOtherRef.size()) {
                    Uml::Role_Type r = bOtherRef[i+1]->roleForWidget(m_umlWidget);
                    if (r == Uml::A) {
                        p2 = bOtherRef[i+1]->associationLine()->startPoint();
                    } else {
                        p2 = bOtherRef[i+1]->associationLine()->endPoint();
                    }
                    p2 = bOtherRef[i]->mapToScene(p2);
                }
                switch (regions.second) {
                    case Uml::reg_North:
                        p1 = (i == 0 ? rect.topLeft() : p1);
                        p2 = (i+1 >= bOtherRef.size() ? rect.topRight() : p2);
                        break;
                    case Uml::reg_West:
                        p1 = (i == 0 ? rect.topLeft() : p1);
                        p2 = (i+1 >= bOtherRef.size() ? rect.bottomLeft() : p2);
                        break;
                    case Uml::reg_East:
                        p1 = (i == 0 ? rect.topRight() : p1);
                        p2 = (i+1 >= bOtherRef.size() ? rect.bottomRight() : p2);
                        break;
                    case Uml::reg_South:
                        p1 = (i == 0 ? rect.bottomLeft() : p1);
                        p2 = (i+1 >= bOtherRef.size() ? rect.bottomRight() : p2);
                        break;
                    case Uml::reg_NorthWest: p1 = p2 = rect.topLeft(); break;
                    case Uml::reg_NorthEast: p1 = p2 = rect.topRight(); break;
                    case Uml::reg_SouthEast: p1 = p2 = rect.bottomRight(); break;
                    case Uml::reg_SouthWest: p1 = p2 = rect.bottomLeft(); break;
                    default:break;
                };
                QPointF end = assoc->mapFromScene((p1 + p2) / 2);

                assoc->associationLine()->setEndPoints(start, end);
            }
        }
        return;
    } // if (regions.second != Uml::reg_Error)

    // Holds whether arrangement is based on x(horizontal) or not (which means
    // its vertically arranged based on y).
    bool xBasis = (regions.first == Uml::reg_North ||
            regions.first == Uml::reg_South);
    // assocDistances contains a list of pairs of New::AssociationWidget and
    // its x or y value depending on region.
    QList<QPair<New::AssociationWidget*, qreal> > assocDistances;
    // This for loop computes the pair values and inserts them in sorted
    // manner based on pair.second variable.
    foreach (New::AssociationWidget* assoc, listRef) {
        // Obtain reference point first.
        QPointF lineStart = referencePoints(assoc).first;
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
        switch (regions.first) {
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
 * @return The RegionPair where assoc's end points resides.
 * @note Refer @ref RegionPair to understand why pair is used.
 */
RegionPair AssociationSpaceManager::regions(New::AssociationWidget *assoc) const
{
    if (!isRegistered(assoc)) {
        return Uml::reg_Error;
    }
    QMapIterator<RegionPair, QList<New::AssociationWidget*> >
        it(m_regionsAssociationsMap);
    while (it.hasNext()) {
        it.next();
        if (it.value().contains(assoc)) {
            return it.key();
        }
    }
    return RegionPair();
}

/**
 * @return Registration status of assoc.
 */
bool AssociationSpaceManager::isRegistered(New::AssociationWidget* assoc) const
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

