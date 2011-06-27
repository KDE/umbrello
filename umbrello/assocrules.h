/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ASSOCRULES_H
#define ASSOCRULES_H

#include "umlobject.h"
#include "widgetbase.h"

namespace std
    { class type_info; }

class UMLWidget;

/**
 * Used to determine rules for associations.
 *
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssocRules
{
public:

    AssocRules();
    ~AssocRules();

    static bool allowAssociation( Uml::AssociationType assocType, UMLWidget * widget );
    static bool allowAssociation( Uml::AssociationType assocType, const std::type_info & );
    static bool allowAssociation( Uml::AssociationType assocType,
                                  UMLWidget * widgetA, UMLWidget * widgetB,
                                  bool extendedCheck = true );

    static bool allowRole( Uml::AssociationType assocType );

    static bool allowMultiplicity( Uml::AssociationType assocType, WidgetBase::WidgetType widgetType );

    static bool allowSelf( Uml::AssociationType assocType, WidgetBase::WidgetType widgetType );

    static Uml::AssociationType isGeneralisationOrRealisation(UMLWidget* widgetA, UMLWidget* widgetB);

private:

    struct Assoc_Rule {  ///< Structure to help determine association rules.
        Uml::AssociationType assoc_type;   ///< association type
        WidgetBase::WidgetType widgetA_type;    ///< type of role A widget
        WidgetBase::WidgetType widgetB_type;    ///< type of role B widget
        bool role;                         ///< role text
        bool multiplicity;                 ///< multipliciy text on association
        bool directional;                  ///< can have an association of same type going between widget each way
        bool self;                         ///< association to self
    };

    static Assoc_Rule m_AssocRules[];  ///< Container that holds all the rules.
    static int        m_nNumRules;     ///< The number of rules known about.
};

#endif
