/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ASSOCRULES_H
#define ASSOCRULES_H

#include "umlnamespace.h"
namespace std
    { class type_info; }

class UMLWidget;

/**
 * Used to determine rules for associations.
 *
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssocRules {
public:
    /**
     * Constructor.
     */
    AssocRules();

    /**
     * Deconstructor.
     */
    ~AssocRules();

    /**
     * Returns whether an association is going to be allowed for the given
     * values. This method is used to test if you can start an association.
     */
    static bool allowAssociation( Uml::Association_Type assocType, UMLWidget * widget );

    static bool allowAssociation( Uml::Association_Type assocType, const std::type_info & );

    /**
     * Returns whether an association is valid with the given variables.
     * This method is used to finish an association.
     */
    static bool allowAssociation( Uml::Association_Type assocType,
                                  UMLWidget * widgetA, UMLWidget * widgetB,
                                  bool extendedCheck = true );

    /**
     * Returns whether to allow a role text for the given association type.
     */
    static bool allowRole( Uml::Association_Type assocType );

    /**
     * Returns whether to allow a multiplicity text for the given
     * association and widget type.
     */
    static bool allowMultiplicity( Uml::Association_Type assocType, Uml::Widget_Type widgetType );

    /**
     * Returns whether to allow an association to self for given variables.
     */
    static bool allowSelf( Uml::Association_Type assocType, Uml::Widget_Type widgetType );

    /**
     * Returns whether an implements association should be a Realisation or
     * a Generalisation.
     * as defined in m_AssocRules.
     */
    static Uml::Association_Type isGeneralisationOrRealisation(UMLWidget* widgetA, UMLWidget* widgetB);

private:

    /**
     * Structure to help determine association rules.
     */
    struct Assoc_Rule {
        Uml::Association_Type assoc_type; ///< association type
        Uml::Widget_Type widgetA_type; ///< type of role A widget
        Uml::Widget_Type widgetB_type; ///< type of role B widget
        bool role;                   ///< role text
        bool multiplicity;           ///< multipliciy text on association
        /// can have an association of same type going between widget each way
        bool directional;

        bool self;                   ///< association to self
    };

    /**
     * Container that holds all the rules.
     */
    static Assoc_Rule m_AssocRules[];

    /**
     * The number of rules known about.
     */
    static int m_nNumRules;
};

#endif
