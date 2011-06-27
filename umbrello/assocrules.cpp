/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "assocrules.h"

// local includes
#include "debug_utils.h"
#include "uml.h"
#include "umlview.h"
#include "umlwidget.h"
#include "umlobject.h"
#include "associationwidgetlist.h"
#include "associationwidget.h"
#include "statewidget.h"
#include "activitywidget.h"
#include "signalwidget.h"
#include "forkjoinwidget.h"
#include "umlscene.h"

// kde includes
#include <typeinfo>
#include <kmessagebox.h>

/**
 * Constructor.
 */
AssocRules::AssocRules()
{
}

/**
 * Destructor.
 */
AssocRules::~AssocRules()
{
}

/**
 * Returns whether an association is going to be allowed for the given
 * values. This method is used to test if you can start an association.
 */
bool allowAssociation( Uml::AssociationType assocType, const std::type_info &type )
{
    Q_UNUSED(assocType); Q_UNUSED(type);
    return false;
}

/**
 * Returns whether an association is going to be allowed for the given
 * values. This method is used to test if you can start an association.
 */
bool AssocRules::allowAssociation( Uml::AssociationType assocType, UMLWidget * widget )
{
    WidgetBase::WidgetType widgetType = widget->baseType();
    bool bValid = false;
    for (int i = 0; i < m_nNumRules; ++i) {
        if (assocType == m_AssocRules[i].assoc_type) {
            if (widgetType == m_AssocRules[i].widgetA_type
                    || widgetType == m_AssocRules[i].widgetB_type ) {
                bValid =  true;
            }
        }
    }
    if( !bValid ) {
        // Special case: Subsystem realizes interface in component diagram
        UMLView *view = UMLApp::app()->currentView();
        if (view && view->umlScene()->type() == Uml::DiagramType::Component &&
            widgetType == WidgetBase::wt_Package &&
            (assocType == Uml::AssociationType::Generalization || assocType == Uml::AssociationType::Realization))
            bValid = true;
        else
            return false;
    }
    AssociationWidgetList list = widget->associationWidgetList();

    switch( assocType ) {
    case Uml::AssociationType::Association:
    case Uml::AssociationType::UniAssociation:
    case Uml::AssociationType::Dependency:
    case Uml::AssociationType::Coll_Message:
    case Uml::AssociationType::Generalization://can have many sub/super types
    case Uml::AssociationType::Aggregation:
    case Uml::AssociationType::Relationship:
    case Uml::AssociationType::Composition:
    case Uml::AssociationType::Containment:
        return true;//doesn't matter whats already connected to widget
        break;

    case Uml::AssociationType::Association_Self:
        return true;// we should really check that connection is to same object
        break;

    case Uml::AssociationType::Realization:  // one connected to widget only (a or b)
        foreach ( AssociationWidget* assoc, list ) {
            if( assoc->associationType() == Uml::AssociationType::Realization )
                return false;
        }
        return true;
        break;

    case Uml::AssociationType::State:
        {
            StateWidget *pState = dynamic_cast<StateWidget*>(widget);
            return (pState == NULL || pState->stateType() != StateWidget::End);
        }
        break;

    case Uml::AssociationType::Activity:
    case Uml::AssociationType::Exception:
        {
            ActivityWidget *pActivity = dynamic_cast<ActivityWidget*>(widget);
            return (pActivity == NULL || pActivity->activityType() != ActivityWidget::End);
        }
        break;

    case Uml::AssociationType::Anchor:
        return true;
        break;

    case Uml::AssociationType::Category2Parent:
        if ( widgetType == WidgetBase::wt_Category )
            return true;
        break;

    case Uml::AssociationType::Child2Category:
        if ( widgetType == WidgetBase::wt_Entity )
            return true;
        break;

    default:
        uWarning() << "allowAssociation() on unknown type";
        break;
    }
    return false;
}

/**
 * Returns whether an association is valid with the given variables.
 * This method is used to finish an association.
 * When we know what we are going to connect both ends of the association to, we can
 * use this method.
 */
bool AssocRules::allowAssociation( Uml::AssociationType assocType,
                                   UMLWidget * widgetA, UMLWidget * widgetB,
                                   bool extendedCheck )
{
    WidgetBase::WidgetType widgetTypeA = widgetA->baseType();
    WidgetBase::WidgetType widgetTypeB = widgetB->baseType();
    bool bValid = false;

    if ( widgetA->umlObject() == widgetB->umlObject() ) {
        return allowSelf( assocType, widgetTypeA );
    }

    for (int i = 0; i < m_nNumRules; ++i) {
        if (assocType == m_AssocRules[i].assoc_type) {
            if( (widgetTypeA == m_AssocRules[i].widgetA_type &&
                    widgetTypeB == m_AssocRules[i].widgetB_type) ||
                    (widgetTypeB == m_AssocRules[i].widgetA_type &&
                     widgetTypeA == m_AssocRules[i].widgetB_type ) )
                bValid = true;
        }
    }
    // we can bail here for quick checks, as occur in loading files
    // for paste or regular creation operations, we need to go further
    if (!extendedCheck) {
        return bValid;
    }

    if (!bValid) {
        return false;
    }

    AssociationWidgetList list = widgetB->associationWidgetList();

    switch( assocType ) {
    case Uml::AssociationType::Association_Self:
        if ( widgetA->umlObject() == widgetB->umlObject() )
            return true;
        break;

    case Uml::AssociationType::Association:
    case Uml::AssociationType::UniAssociation:
    case Uml::AssociationType::Dependency:
    case Uml::AssociationType::Coll_Message:
    case Uml::AssociationType::Aggregation:
    case Uml::AssociationType::Relationship:
        return true;  // doesn't matter what's already connected to widget
        break;

    case Uml::AssociationType::Composition:   // can't have mutual composition
    case Uml::AssociationType::Containment:   // can't have mutual containment
    case Uml::AssociationType::Generalization://can have many sub/super types but can't sup/sub each
        foreach ( AssociationWidget * assoc, list ) {
            if( ( widgetA == assoc->widgetForRole(Uml::A) ||
                        widgetA == assoc->widgetForRole(Uml::B) )
                    && assoc->associationType() == assocType )
                return false;
        }
        return true;
        break;

    case Uml::AssociationType::Realization: // can only connect to abstract (interface) classes
        foreach( AssociationWidget * assoc, list ) {
            if( ( widgetA == assoc->widgetForRole(Uml::A) ||
                        widgetA == assoc->widgetForRole(Uml::B) )
                    && assoc->associationType() == Uml::AssociationType::Realization ) {
                return false;
            }
        }
        if (widgetB->baseType() == WidgetBase::wt_Class) {
            return widgetB->umlObject()->isAbstract();
        } else if (widgetB->baseType() == WidgetBase::wt_Interface ||
                   widgetB->baseType() == WidgetBase::wt_Package) {
            return true;
        }
        break;

    case Uml::AssociationType::State:
        {
            StateWidget *stateA = dynamic_cast<StateWidget*>(widgetA);
            StateWidget *stateB = dynamic_cast<StateWidget*>(widgetB);
            if (stateA && stateB) {
                if (stateB->stateType() == StateWidget::Initial)
                    return false;
                if (stateB->stateType() == StateWidget::End &&
                    stateA->stateType() != StateWidget::Normal)
                    return false;
            }
        }
        return true;
        break;

    case Uml::AssociationType::Activity:
    case Uml::AssociationType::Exception:
        {

            ActivityWidget *actA = dynamic_cast<ActivityWidget*>(widgetA);
            ActivityWidget *actB = dynamic_cast<ActivityWidget*>(widgetB);

            bool isSignal = false;
            bool isObjectNode = false;

            if (widgetTypeA == WidgetBase::wt_Signal)
                isSignal = true;
            else if (widgetTypeA == WidgetBase::wt_ObjectNode)
                isObjectNode = true;

            // no transitions to initial activity allowed
            if (actB && actB->activityType() == ActivityWidget::Initial) {
                return false;
            }
            // actType -1 here means "not applicable".
            int actTypeA = -1;
            if (actA)
                actTypeA = actA->activityType();
            int actTypeB = -1;
            if (actB)
                actTypeB = actB->activityType();
            // only from a signalwidget a objectnode widget, a normal activity, branch or fork activity, to the end
            if ((actTypeB == ActivityWidget::End || actTypeB == ActivityWidget::Final) &&
                actTypeA != ActivityWidget::Normal &&
                actTypeA != ActivityWidget::Branch &&
                dynamic_cast<ForkJoinWidget*>(widgetA) == NULL && !isSignal &&!isObjectNode) {
                return false;
            }
            // only Forks and Branches can have more than one "outgoing" transition
            if (actA != NULL && actTypeA != ActivityWidget::Branch) {
                AssociationWidgetList list = widgetA->associationWidgetList();
                foreach (AssociationWidget* assoc , list ) {
                    if (assoc->widgetForRole(Uml::A) == widgetA) {
                        return false;
                    }
                }
            }
        }
        return true;
        break;

    case Uml::AssociationType::Anchor:
        return true;
        break;

    case Uml::AssociationType::Category2Parent:
        if ( widgetTypeA == WidgetBase::wt_Category && widgetTypeB == WidgetBase::wt_Entity ) {
            return true;
        }
        break;

    case Uml::AssociationType::Child2Category:
        if ( widgetTypeA == WidgetBase::wt_Entity && widgetTypeB == WidgetBase::wt_Category ) {
            return true;
        }
        break;

    default:
        uWarning() << "allowAssociation() on unknown type";
        break;
    }
    return false;
}

/**
 * Returns whether to allow a role text for the given association type.
 */
bool AssocRules::allowRole( Uml::AssociationType assocType )
{
    for( int i = 0; i < m_nNumRules; ++i )
        if( assocType == m_AssocRules[ i ].assoc_type )
            return m_AssocRules[ i ].role;
    return false;
}

/**
 * Returns whether to allow a multiplicity text for the given
 * association and widget type.
 */
bool AssocRules::allowMultiplicity( Uml::AssociationType assocType, WidgetBase::WidgetType widgetType )
{
    for( int i = 0; i < m_nNumRules; ++i )
        if( assocType == m_AssocRules[ i ].assoc_type )
            if( widgetType == m_AssocRules[ i ].widgetA_type || widgetType == m_AssocRules[ i ].widgetB_type )
                return m_AssocRules[ i ].multiplicity;
    return false;
}

/**
 * Returns whether to allow an association to self for given variables.
 */
bool AssocRules::allowSelf( Uml::AssociationType assocType, WidgetBase::WidgetType widgetType )
{
    for( int i = 0; i < m_nNumRules; ++i )
        if( assocType == m_AssocRules[ i ].assoc_type )
            if( widgetType == m_AssocRules[ i ].widgetA_type || widgetType == m_AssocRules[ i ].widgetB_type )
                return m_AssocRules[ i ].self;

    return false;
}

/**
 * Returns whether an implements association should be a Realisation or
 * a Generalisation.
 * as defined in m_AssocRules.
 */
Uml::AssociationType AssocRules::isGeneralisationOrRealisation(UMLWidget* widgetA, UMLWidget* widgetB)
{
    WidgetBase::WidgetType widgetTypeA = widgetA->baseType();
    WidgetBase::WidgetType widgetTypeB = widgetB->baseType();
    for (int i = 0; i < m_nNumRules; ++i) {
        if (m_AssocRules[i].assoc_type == Uml::AssociationType::Realization &&
                widgetTypeA == m_AssocRules[i].widgetA_type &&
                widgetTypeB == m_AssocRules[i].widgetB_type) {
            return Uml::AssociationType::Realization;
        }
    }
    return Uml::AssociationType::Generalization;
}

AssocRules::Assoc_Rule AssocRules::m_AssocRules []= {
    // Association                            widgetA                    widgetB                     role    multi   direct. self
    //---------------------------------------+--------------------------+---------------------------+-------+-------+-------+--------
    { Uml::AssociationType::Association_Self, WidgetBase::wt_Class,      WidgetBase::wt_Class,       true,   true,   true,   true  },
    { Uml::AssociationType::Association_Self, WidgetBase::wt_Object,     WidgetBase::wt_Object,      true,   true,   true,   true  },
    { Uml::AssociationType::Association_Self, WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   true,   true,   true,   true  },
    { Uml::AssociationType::Association,      WidgetBase::wt_Class,      WidgetBase::wt_Class,       true,   true,   true,   true  },
    { Uml::AssociationType::Association,      WidgetBase::wt_Object,     WidgetBase::wt_Object,      true,   true,   true,   true  },
    { Uml::AssociationType::Association,      WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   true,   true,   true,   true  },
    { Uml::AssociationType::Association,      WidgetBase::wt_Interface,  WidgetBase::wt_Class,       true,   true,   true,   false },
    { Uml::AssociationType::Association,      WidgetBase::wt_Class,      WidgetBase::wt_Interface,   true,   true,   true,   false },
    { Uml::AssociationType::Association,      WidgetBase::wt_Datatype,   WidgetBase::wt_Class,       true,   true,   true,   false },
    { Uml::AssociationType::Association,      WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    true,   true,   true,   false },
    { Uml::AssociationType::Association,      WidgetBase::wt_Enum,       WidgetBase::wt_Class,       true,   true,   true,   false },
    { Uml::AssociationType::Association,      WidgetBase::wt_Class,      WidgetBase::wt_Enum,        true,   true,   true,   false },
    { Uml::AssociationType::Association,      WidgetBase::wt_Actor,      WidgetBase::wt_UseCase,     true,   false,  false,  false },
    { Uml::AssociationType::Association,      WidgetBase::wt_UseCase,    WidgetBase::wt_UseCase,     true,   false,  false,  false },
    { Uml::AssociationType::Association,      WidgetBase::wt_Actor,      WidgetBase::wt_Actor,       true,   false,  false,  false },
    { Uml::AssociationType::Association,      WidgetBase::wt_Actor,      WidgetBase::wt_UseCase,     true,   false,  false,  false },
    { Uml::AssociationType::Association,      WidgetBase::wt_Component,  WidgetBase::wt_Interface,   true,   false,  false,  false },
    { Uml::AssociationType::Association,      WidgetBase::wt_Interface,  WidgetBase::wt_Artifact,    true,   false,  false,  false },
    { Uml::AssociationType::Association,      WidgetBase::wt_Node,       WidgetBase::wt_Node,        true,   false,  false,  false },
    { Uml::AssociationType::UniAssociation,   WidgetBase::wt_Class,      WidgetBase::wt_Class,       true,   true,   true,   true  },
    { Uml::AssociationType::UniAssociation,   WidgetBase::wt_Object,     WidgetBase::wt_Object,      true,   true,   true,   true  },
    { Uml::AssociationType::UniAssociation,   WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   true,   true,   true,   true  },
    { Uml::AssociationType::UniAssociation,   WidgetBase::wt_Interface,  WidgetBase::wt_Class,       true,   true,   true,   true  },
    { Uml::AssociationType::UniAssociation,   WidgetBase::wt_Class,      WidgetBase::wt_Interface,   true,   true,   true,   true  },
    { Uml::AssociationType::UniAssociation,   WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    true,   true,   true,   true  },
    { Uml::AssociationType::UniAssociation,   WidgetBase::wt_Class,      WidgetBase::wt_Enum,        true,   true,   true,   true  },
    { Uml::AssociationType::UniAssociation,   WidgetBase::wt_Actor,      WidgetBase::wt_Actor,       true,   false,  false,  false },
    { Uml::AssociationType::UniAssociation,   WidgetBase::wt_UseCase,    WidgetBase::wt_UseCase,     true,   false,  false,  false },
    { Uml::AssociationType::UniAssociation,   WidgetBase::wt_UseCase,    WidgetBase::wt_Actor,       true,   false,  false,  false },
    { Uml::AssociationType::Generalization,   WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    false,  false,  false,  false },
    { Uml::AssociationType::Generalization,   WidgetBase::wt_Class,      WidgetBase::wt_Class,       false,  false,  false,  false },
    { Uml::AssociationType::Generalization,   WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   false,  false,  false,  false },
    { Uml::AssociationType::Generalization,   WidgetBase::wt_UseCase,    WidgetBase::wt_UseCase,     false,  false,  false,  false },
    { Uml::AssociationType::Generalization,   WidgetBase::wt_Actor,      WidgetBase::wt_Actor,       false,  false,  false,  false },
    { Uml::AssociationType::Generalization,   WidgetBase::wt_Component,  WidgetBase::wt_Interface,   false,  false,  false,  false },
    { Uml::AssociationType::Aggregation,      WidgetBase::wt_Class,      WidgetBase::wt_Class,       true,   true,   false,  true  },
    { Uml::AssociationType::Aggregation,      WidgetBase::wt_Class,      WidgetBase::wt_Interface,   true,   true,   false,  false },
    { Uml::AssociationType::Aggregation,      WidgetBase::wt_Class,      WidgetBase::wt_Enum,        true,   true,   false,  false },
    { Uml::AssociationType::Aggregation,      WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    true,   true,   false,  false },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Class,      WidgetBase::wt_Class,       true,   false,  false,  true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_UseCase,    WidgetBase::wt_UseCase,     true,   false,  false,  false },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Actor,      WidgetBase::wt_Actor,       true,   false,  false,  false },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Actor,      WidgetBase::wt_UseCase,     true,   false,  false,  false },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Package,    WidgetBase::wt_Package,     true,   true,   true,   true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Package,    WidgetBase::wt_Class,       true,   true,   true,   true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Class,      WidgetBase::wt_Package,     true,   true,   true,   true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Package,    WidgetBase::wt_Interface,   true,   true,   true,   true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Interface,  WidgetBase::wt_Package,     true,   true,   true,   true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   true,   true,   true,   true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Interface,  WidgetBase::wt_Class,       true,   true,   true,   true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Class,      WidgetBase::wt_Interface,   true,   true,   true,   true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    true,   true,   true,   true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Class,      WidgetBase::wt_Enum,        true,   true,   true,   true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Component,  WidgetBase::wt_Component,   true,   true,   true,   true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Component,  WidgetBase::wt_Interface,   true,   true,   true,   true  },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Component,  WidgetBase::wt_Artifact,    true,   false,  false,  false },
    { Uml::AssociationType::Dependency,       WidgetBase::wt_Node,       WidgetBase::wt_Component,   true,   false,  false,  false },
    { Uml::AssociationType::Realization,      WidgetBase::wt_Class,      WidgetBase::wt_Interface,   false,  false,  false,  false },
    { Uml::AssociationType::Realization,      WidgetBase::wt_Interface,  WidgetBase::wt_Package,     false,  false,  false,  false },
    { Uml::AssociationType::Realization,      WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   false,  false,  false,  false },
    { Uml::AssociationType::Realization,      WidgetBase::wt_Component,  WidgetBase::wt_Interface,   false,  false,  false,  false },
    { Uml::AssociationType::Realization,      WidgetBase::wt_Package,    WidgetBase::wt_Interface,   false,  false,  false,  false },
    { Uml::AssociationType::Composition,      WidgetBase::wt_Class,      WidgetBase::wt_Class,       true,   true,   false,  true  },
    { Uml::AssociationType::Composition,      WidgetBase::wt_Class,      WidgetBase::wt_Interface,   true,   true,   false,  false },
    { Uml::AssociationType::Composition,      WidgetBase::wt_Class,      WidgetBase::wt_Enum,        true,   true,   false,  false },
    { Uml::AssociationType::Composition,      WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    false,  false,  false,  false },
    { Uml::AssociationType::Composition,      WidgetBase::wt_Class,      WidgetBase::wt_Class,       false,  false,  false,  false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Package,    WidgetBase::wt_Class,       false,  false,  true,   false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Package,    WidgetBase::wt_Interface,   false,  false,  true,   false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Package,    WidgetBase::wt_Enum,        false,  false,  true,   false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Package,    WidgetBase::wt_Package,     false,  false,  true,   false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Package,    WidgetBase::wt_Component,   false,  false,  true,   false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Class,      WidgetBase::wt_Class,       false,  false,  true,   false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Class,      WidgetBase::wt_Interface,   false,  false,  true,   false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Class,      WidgetBase::wt_Enum,        false,  false,  true,   false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Interface,  WidgetBase::wt_Class,       false,  false,  true,   false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   false,  false,  true,   false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Interface,  WidgetBase::wt_Enum,        false,  false,  true,   false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Component,  WidgetBase::wt_Component,   false,  false,  true,   false },
    { Uml::AssociationType::Containment,      WidgetBase::wt_Component,  WidgetBase::wt_Artifact,    false,  false,  true,   false },
    { Uml::AssociationType::Coll_Message,     WidgetBase::wt_Object,     WidgetBase::wt_Object,      true,   false,  true,   true  },
    { Uml::AssociationType::State,            WidgetBase::wt_State,      WidgetBase::wt_State,       true,   false,  true,   true  },
    { Uml::AssociationType::State,            WidgetBase::wt_ForkJoin,   WidgetBase::wt_State,       true,   false,  true,   true  },
    { Uml::AssociationType::State,            WidgetBase::wt_State,      WidgetBase::wt_ForkJoin,    true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_Signal,     WidgetBase::wt_Activity,    true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_Activity,   WidgetBase::wt_Signal,      true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_ObjectNode, WidgetBase::wt_Activity,    true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_Activity,   WidgetBase::wt_ObjectNode,  true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_Activity,   WidgetBase::wt_Activity,    true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_ForkJoin,   WidgetBase::wt_Activity,    true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_Activity,   WidgetBase::wt_ForkJoin,    true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_Signal,     WidgetBase::wt_ForkJoin,    true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_ForkJoin,   WidgetBase::wt_Signal,      true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_ForkJoin,   WidgetBase::wt_ObjectNode,  true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_ObjectNode, WidgetBase::wt_ForkJoin,    true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_Pin,        WidgetBase::wt_Activity,    true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_Pin,        WidgetBase::wt_Pin,         true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_Activity,   WidgetBase::wt_Pin,         true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_Pin,        WidgetBase::wt_ForkJoin,    true,   false,  true,   true  },
    { Uml::AssociationType::Activity,         WidgetBase::wt_ForkJoin,   WidgetBase::wt_Pin,         true,   false,  true,   true  },
    { Uml::AssociationType::Anchor,           WidgetBase::wt_Class,      WidgetBase::wt_Note,        false,  false,  false,  false },
    { Uml::AssociationType::Anchor,           WidgetBase::wt_Package,    WidgetBase::wt_Note,        false,  false,  false,  false },
    { Uml::AssociationType::Anchor,           WidgetBase::wt_Interface,  WidgetBase::wt_Note,        false,  false,  false,  false },
    { Uml::AssociationType::Anchor,           WidgetBase::wt_Datatype,   WidgetBase::wt_Note,        false,  false,  false,  false },
    { Uml::AssociationType::Anchor,           WidgetBase::wt_Enum,       WidgetBase::wt_Note,        false,  false,  false,  false },
    { Uml::AssociationType::Anchor,           WidgetBase::wt_Object,     WidgetBase::wt_Note,        false,  false,  false,  false },
    { Uml::AssociationType::Anchor,           WidgetBase::wt_Actor,      WidgetBase::wt_Note,        false,  false,  false,  false },
    { Uml::AssociationType::Anchor,           WidgetBase::wt_UseCase,    WidgetBase::wt_Note,        false,  false,  false,  false },
    { Uml::AssociationType::Anchor,           WidgetBase::wt_Message,    WidgetBase::wt_Note,        false,  false,  false,  false },
    { Uml::AssociationType::Anchor,           WidgetBase::wt_State,      WidgetBase::wt_Note,        false,  false,  false,  false },
    { Uml::AssociationType::Anchor,           WidgetBase::wt_Activity,   WidgetBase::wt_Note,        false,  false,  false,  false },
    { Uml::AssociationType::Anchor,           WidgetBase::wt_Entity,     WidgetBase::wt_Note,        false,  false,  false,  false },
    { Uml::AssociationType::Relationship,     WidgetBase::wt_Entity,     WidgetBase::wt_Entity,      true,   true,   true,   true  },
    { Uml::AssociationType::Exception,        WidgetBase::wt_Activity,   WidgetBase::wt_Activity,    true,   false,  true,   true  },
    { Uml::AssociationType::Exception,        WidgetBase::wt_Activity,   WidgetBase::wt_Signal,      true,   false,  true,   true  },
    { Uml::AssociationType::Exception,        WidgetBase::wt_Signal,     WidgetBase::wt_Activity,    true,   false,  true,   true  },
    { Uml::AssociationType::Exception,        WidgetBase::wt_Signal,     WidgetBase::wt_Signal,      true,   false,  true,   true  },
    { Uml::AssociationType::Category2Parent,  WidgetBase::wt_Category,   WidgetBase::wt_Entity,      false,  false,  true,   false },
    { Uml::AssociationType::Child2Category,   WidgetBase::wt_Entity,     WidgetBase::wt_Category,    false,  false,  true,   false }
};

int AssocRules::m_nNumRules = sizeof( m_AssocRules ) / sizeof( AssocRules::Assoc_Rule );

