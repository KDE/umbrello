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
#include <QPointF>

class UMLWidget;
namespace New {
    class AssociationWidget;
}

class AssociationSpaceManager
{
public:
    AssociationSpaceManager(UMLWidget *widget);

    void addAssociationWidget(New::AssociationWidget *assoc);
    void addAssociationWidget(New::AssociationWidget *assoc, Uml::Region region);
    void removeAssociatinWidget(New::AssociationWidget *assoc);

    QPointF endPoint(New::AssociationWidget *assoc) const;
    QPointF penultimateEndPoint(New::AssociationWidget *assoc) const;

    Uml::Region nearestRegion(New::AssociationWidget *assoc) const;

    void arrange(Uml::Region region);
    void arrangeAllRegions();

private:
    QMap<Uml::Region, QList<New::AssociationWidget*> > m_regionAssociationsMap;
    UMLWidget *m_umlWidget;
};

#endif
