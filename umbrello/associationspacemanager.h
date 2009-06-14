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

#ifndef ASSOCIATIONSPACEMANAGER_H
#define ASSOCIATIONSPACEMANAGER_H

#include "umlnamespace.h"
#include <QObject>
#include <QPointF>

class UMLWidget;
namespace New {
    class AssociationWidget;
}

/**
 * This helper struct accomplishes task of being data store for both self and
 * non self associations.
 * A less than operator along with id() method enables RegionPair to be used
 * as key in QMap and QSet.
 *
 * For self association, first and second variable contains regions occupied by
 * the RoleA line end as well as RoleB line end respectively.
 *
 * For non self association, first variable contains region occupied by the
 * association line end corresponding to role of UMLWidget in association.
 */
struct RegionPair
{
    RegionPair(Uml::Region f = Uml::reg_Error, Uml::Region s = Uml::reg_Error);
    bool isValid() const;
    bool operator<(const RegionPair& rhs) const;

    Uml::Region first, second;

private:
    int id() const;
};

/**
 * @short A class to manage distribution of AssociationWidget around UMLWidget.
 *
 * This class mainly has the following duties
 *  - Store New::AssociationWidgets associated with the UMLWidget to which
 *    this belongs.
 *  - Arrange/distribute the AssociationLine endings for all regions of
 *    UMLWidget based on AssociationSpaceManager::referencePoint.
 *
 * The object of this class is stored in a UMLWidget.
 */
class AssociationSpaceManager : public QObject
{
Q_OBJECT;
public:
    AssociationSpaceManager(UMLWidget *widget);

    void add(New::AssociationWidget *assoc, RegionPair regions);
    RegionPair remove(New::AssociationWidget *assoc);

    void arrange(RegionPair regions);

    RegionPair regions(New::AssociationWidget *assoc) const;

    bool isRegistered(New::AssociationWidget* assoc) const;

    QSet<New::AssociationWidget*> associationWidgets() const;

private:
    QPointF referencePoint(New::AssociationWidget *assoc) const;

    QMap<RegionPair, QList<New::AssociationWidget*> > m_regionsAssociationsMap;
    QSet<New::AssociationWidget*> m_registeredAssociationSet;
    UMLWidget *m_umlWidget;
};

#endif
