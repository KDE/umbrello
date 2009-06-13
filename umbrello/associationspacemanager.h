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

    void add(New::AssociationWidget *assoc, Uml::Region region);
    Uml::Region remove(New::AssociationWidget *assoc);

    QPointF referencePoint(New::AssociationWidget *assoc) const;

    void arrange(Uml::Region region);
    void arrangeAllRegions();

    Uml::Region region(New::AssociationWidget *assoc) const;
    bool registered(New::AssociationWidget* assoc) const;

    QSet<New::AssociationWidget*> associationWidgets() const;

private:
    QMap<Uml::Region, QList<New::AssociationWidget*> > m_regionAssociationsMap;
    QSet<New::AssociationWidget*> m_registeredAssociationSet;
    UMLWidget *m_umlWidget;
};

#endif
