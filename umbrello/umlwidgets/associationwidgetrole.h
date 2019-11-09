/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2019                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ASSOCIATIONWIDGETROLE_H
#define ASSOCIATIONWIDGETROLE_H

#include "basictypes.h"

#include <QPointer>

class AssociationWidget;
class FloatingTextWidget;
class UMLWidget;

/**
 * The AssociationWidgetRole struct gathers all information pertaining to the role.
 * The AssociationWidget class contains two AssociationWidgetRole objects, one for
 * each side of the association (A and B).
 */
class AssociationWidgetRole {
public:
    FloatingTextWidget* multiplicityWidget;   ///< information regarding multiplicity
    FloatingTextWidget* changeabilityWidget;  ///< information regarding changeability
    FloatingTextWidget* roleWidget;           ///< role's label of this association

    QPointer<UMLWidget> umlWidget;    ///< UMLWidget at this role's side of this association

    Uml::Region::Enum     m_WidgetRegion;   ///< region of this role's widget

    int m_nIndex;        ///< the index of where the line is on the region for this role
    int m_nTotalCount;   ///< total amount of associations on the region this role's line is on

    // The following items are only used if m_pObject is not set.
    Uml::Visibility::Enum     visibility;
    Uml::Changeability::Enum  changeability;
    QString                   roleDocumentation;

    AssociationWidgetRole();

    void setParent(AssociationWidget *parent) { m_q = parent; }
    void cleanup();

    void setFont(const QFont &font);

private:
    AssociationWidget *m_q;
};

#endif // ASSOCIATIONWIDGETROLE_H
