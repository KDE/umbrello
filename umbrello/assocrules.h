/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ASSOCRULES_H
#define ASSOCRULES_H

#include "umlobject.h"
#include "widgetbase.h"

class UMLWidget;

/**
 * Used to determine rules for associations.
 *
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class AssocRules
{
public:

    AssocRules();
    ~AssocRules();

    static bool allowAssociation(Uml::AssociationType::Enum assocType, UMLWidget * widget);
    static bool allowAssociation(Uml::AssociationType::Enum assocType,
                                  UMLWidget * widgetA, UMLWidget * widgetB);

    static bool allowRole(Uml::AssociationType::Enum assocType);

    static bool allowMultiplicity(Uml::AssociationType::Enum assocType, WidgetBase::WidgetType widgetType);

    static bool allowSelf(Uml::AssociationType::Enum assocType, WidgetBase::WidgetType widgetType);

    static Uml::AssociationType::Enum isGeneralisationOrRealisation(UMLWidget* widgetA, UMLWidget* widgetB);

    typedef enum {All, Java} Language;
    typedef enum {Any, UML1, UML2 } Level;

private:
    class Assoc_Rule {  ///< Structure to help determine association rules.
    public:
        bool isValid() const;
        Language language;                 ///< language rule applies to
        Uml::AssociationType::Enum assoc_type;   ///< association type
        WidgetBase::WidgetType widgetA_type;    ///< type of role A widget
        WidgetBase::WidgetType widgetB_type;    ///< type of role B widget
        bool role;                         ///< role text
        bool multiplicity;                 ///< multiplicity text on association
        bool bidirectional;                ///< can have an association of same type going between widget each way
        bool self;                         ///< association to self
        Level level;                       ///< UML level
    };

    static const Assoc_Rule m_AssocRules[];  ///< Container that holds all the rules.
    static const int        m_nNumRules;     ///< The number of rules known about.
};

#endif
