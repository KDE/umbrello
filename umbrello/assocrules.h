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

class UMLRectWidget;

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

    static bool allowAssociation( Uml::Association_Type assocType, UMLRectWidget * widget );

    static bool allowAssociation( Uml::Association_Type assocType, const std::type_info & );

    static bool allowAssociation( Uml::Association_Type assocType,
                                  UMLRectWidget * widgetA, UMLRectWidget * widgetB,
                                  bool extendedCheck = true );

    static bool allowRole( Uml::Association_Type assocType );

    static bool allowMultiplicity( Uml::Association_Type assocType, Uml::Widget_Type widgetType );

    static bool allowSelf( Uml::Association_Type assocType, Uml::Widget_Type widgetType );

    static Uml::Association_Type isGeneralisationOrRealisation(UMLRectWidget* widgetA, UMLRectWidget* widgetB);

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
