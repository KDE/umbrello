/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "assocrules.h"

// qt/kde includes
#include <kdebug.h>
#include <typeinfo>

// local includes
#include "uml.h"
#include "umlview.h"
#include "umlwidget.h"
#include "umlobject.h"
#include "associationwidgetlist.h"
#include "associationwidget.h"
#include "statewidget.h"
#include "activitywidget.h"
#include "forkjoinwidget.h"


using namespace Uml;

AssocRules::AssocRules() {}

AssocRules::~AssocRules() {}

bool allowAssociation( Association_Type/* assocType*/, const std::type_info/* &type*/ )
{
    return false;

}

bool AssocRules::allowAssociation( Uml::Association_Type assocType, UMLWidget * widget ) {
    Widget_Type widgetType = widget -> getBaseType();
    bool bValid = false;
    for (int i = 0; i < m_nNumRules; i++) {
        if (assocType == m_AssocRules[i].assoc_type) {
            if (widgetType == m_AssocRules[i].widgetA_type
                    || widgetType == m_AssocRules[i].widgetB_type ) {
                bValid =  true;
            }
        }
    }
    if( !bValid ) {
        // Special case: Subsystem realizes interface in component diagram
        UMLView *view = UMLApp::app()->getCurrentView();
        if (view && view->getType() == dt_Component && widgetType == wt_Package &&
            (assocType == at_Generalization || assocType == at_Realization))
            bValid = true;
        else
            return false;
    }
    AssociationWidgetList list = widget -> getAssocList();
    AssociationWidgetListIt it( list );
    AssociationWidget * assoc = 0;
    switch( assocType ) {
    case at_Association:
    case at_UniAssociation:
    case at_Dependency:
    case at_Coll_Message:
    case at_Generalization://can have many sub/super types
    case at_Aggregation:
    case at_Relationship:
    case at_Composition:
    case at_Containment:
        return true;//doesn't matter whats already connected to widget
        break;

    case at_Association_Self:
        return true;// we should really check that connection is to same object
        break;

    case at_Realization:  // one connected to widget only (a or b)
        while( ( assoc = it.current() ) ) {
            if( assoc -> getAssocType() == at_Realization )
                return false;
            ++it;
        }
        return true;
        break;

    case at_State:
        {
            StateWidget *pState = dynamic_cast<StateWidget*>(widget);
            return (pState == NULL || pState->getStateType() != StateWidget::End);
        }
        break;

    case at_Activity:
        {
            ActivityWidget *pActivity = dynamic_cast<ActivityWidget*>(widget);
            return (pActivity == NULL || pActivity->getActivityType() != ActivityWidget::End);
        }
        break;

    case at_Anchor:
        return true;
        break;
    default:
        kWarning() << "allowAssociation() on unknown type" << endl;
        break;
    }
    return false;
}

// when we know what we are going to connect both ends of the association to, we can
// use this method.
bool AssocRules::allowAssociation( Uml::Association_Type assocType,
                                   UMLWidget * widgetA, UMLWidget * widgetB,
                                   bool extendedCheck ) {
    Widget_Type widgetTypeA = widgetA->getBaseType();
    Widget_Type widgetTypeB = widgetB->getBaseType();
    bool bValid = false;
    for (int i = 0; i < m_nNumRules; i++) {
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
    AssociationWidgetList list = widgetB -> getAssocList();
    AssociationWidgetListIt it( list );
    AssociationWidget * assoc = 0;
    switch( assocType ) {
    case at_Association:
    case at_Association_Self:
    case at_UniAssociation:
    case at_Dependency:
    case at_Coll_Message:
    case at_Aggregation:
    case at_Relationship:
        return true;  // doesn't matter what's already connected to widget
        break;

    case at_Composition:   // can't have mutual composition
    case at_Containment:   // can't have mutual containment
    case at_Generalization://can have many sub/super types but can't sup/sub each
        while( ( assoc = it.current() ) ) {
            if( ( widgetA == assoc -> getWidget(A) || widgetA == assoc -> getWidget(B) )
                    && assoc->getAssocType() == assocType )
                return false;
            ++it;
        }
        return true;
        break;

    case at_Realization: // can only connect to abstract (interface) classes
        while( ( assoc = it.current() ) ) {
            if( ( widgetA == assoc->getWidget(A) || widgetA == assoc->getWidget(B) )
                    && assoc->getAssocType() == at_Realization ) {
                return false;
            }
            ++it;
        }
        if (widgetB->getBaseType() == wt_Class) {
            return widgetB->getUMLObject()->getAbstract();
        } else if (widgetB->getBaseType() == wt_Interface ||
                   widgetB->getBaseType() == wt_Package) {
            return true;
        }
        break;

    case at_State:
        {
            StateWidget *stateA = dynamic_cast<StateWidget*>(widgetA);
            StateWidget *stateB = dynamic_cast<StateWidget*>(widgetB);
            if (stateA && stateB) {
                if (stateB->getStateType() == StateWidget::Initial)
                    return false;
                if (stateB->getStateType() == StateWidget::End &&
                    stateA->getStateType() != StateWidget::Normal)
                    return false;
            }
        }
        return true;
        break;

    case at_Activity:
        {
            ActivityWidget *actA = dynamic_cast<ActivityWidget*>(widgetA);
            ActivityWidget *actB = dynamic_cast<ActivityWidget*>(widgetB);
            // no transitions to initial activity allowed
            if (actB && actB->getActivityType() == ActivityWidget::Initial)
                return false;
            // actType -1 here means "not applicable".
            int actTypeA = -1;
            if (actA)
                actTypeA = actA->getActivityType();
            int actTypeB = -1;
            if (actB)
                actTypeB = actB->getActivityType();
            // only from a normal, branch or fork activity to the end
            if (actTypeB == ActivityWidget::End &&
                actTypeA != ActivityWidget::Normal &&
                actTypeA != ActivityWidget::Branch &&
                dynamic_cast<ForkJoinWidget*>(widgetA) == NULL) {
                return false;
            }
            // only Forks and Branches can have more than one "outgoing" transition
            if (actA != NULL && actTypeA != ActivityWidget::Branch) {
                AssociationWidgetList list = widgetA->getAssocList();
                for (AssociationWidget* assoc = list.first(); assoc; assoc = list.next()) {
                    if (assoc->getWidget(A) == widgetA) {
                        return false;
                    }
                }
            }
        }
        return true;
        break;

    case at_Anchor:
        return true;
        break;

    default:
        kWarning() << "allowAssociation() on unknown type" << endl;
        break;
    }
    return false;
}

bool AssocRules::allowRole( Uml::Association_Type assocType ) {
    for( int i = 0; i < m_nNumRules; i++ )
        if( assocType == m_AssocRules[ i ].assoc_type )
            return m_AssocRules[ i ].role;
    return false;
}

bool AssocRules::allowMultiplicity( Uml::Association_Type assocType, Uml::Widget_Type widgetType ) {
    for( int i = 0; i < m_nNumRules; i++ )
        if( assocType == m_AssocRules[ i ].assoc_type )
            if( widgetType == m_AssocRules[ i ].widgetA_type || widgetType == m_AssocRules[ i ].widgetB_type )
                return m_AssocRules[ i ].multiplicity;
    return false;
}

bool AssocRules::allowSelf( Uml::Association_Type assocType, Uml::Widget_Type widgetType ) {
    for( int i = 0; i < m_nNumRules; i++ )
        if( assocType == m_AssocRules[ i ].assoc_type )
            if( widgetType == m_AssocRules[ i ].widgetA_type || widgetType == m_AssocRules[ i ].widgetB_type )
                return m_AssocRules[ i ].self;

    return false;
}

Uml::Association_Type AssocRules::isGeneralisationOrRealisation(UMLWidget* widgetA, UMLWidget* widgetB) {
    Widget_Type widgetTypeA = widgetA->getBaseType();
    Widget_Type widgetTypeB = widgetB->getBaseType();
    for (int i = 0; i < m_nNumRules; i++) {
        if (m_AssocRules[i].assoc_type == at_Realization &&
                widgetTypeA == m_AssocRules[i].widgetA_type &&
                widgetTypeB == m_AssocRules[i].widgetB_type) {
            return at_Realization;
        }
    }
    return at_Generalization;
}

AssocRules::Assoc_Rule AssocRules::m_AssocRules []= {
    //  Association     widgetA         widgetB         role    multi   directional  self
    //---------------+----------------+----------------+-------+-------+-------+---------
    { at_Association_Self,wt_Class,     wt_Class,       true,   true,   true,   true  },
    { at_Association_Self,wt_Object,    wt_Object,      true,   true,   true,   true  },
    { at_Association_Self,wt_Interface, wt_Interface,   true,   true,   true,   true  },
    { at_Association,   wt_Class,       wt_Class,       true,   true,   true,   true },
    { at_Association,   wt_Object,      wt_Object,      true,   true,   true,   true },
    { at_Association,   wt_Interface,   wt_Interface,   true,   true,   true,   true },
    { at_Association,   wt_Interface,   wt_Class,       true,   true,   true,   false },
    { at_Association,   wt_Class,       wt_Interface,   true,   true,   true,   false },
    { at_Association,   wt_Datatype,    wt_Class,       true,   true,   true,   false },
    { at_Association,   wt_Class,       wt_Datatype,    true,   true,   true,   false },
    { at_Association,   wt_Enum,        wt_Class,       true,   true,   true,   false },
    { at_Association,   wt_Class,       wt_Enum,        true,   true,   true,   false },
    { at_Association,   wt_Actor,       wt_UseCase,     true,   false,  false,  false },
    { at_Association,   wt_UseCase,     wt_UseCase,     true,   false,  false,  false },
    { at_Association,   wt_Actor,       wt_Actor,       true,   false,  false,  false },
    { at_Association,   wt_Actor,       wt_UseCase,     true,   false,  false,  false },
    { at_Association,   wt_Component,   wt_Interface,   true,   false,  false,  false },
    { at_Association,   wt_Interface,   wt_Artifact,    true,   false,  false,  false },
    { at_Association,   wt_Node,        wt_Node,        true,   false,  false,  false },
    { at_UniAssociation,wt_Class,       wt_Class,       true,   true,   true,   true  },
    { at_UniAssociation,wt_Object,      wt_Object,      true,   true,   true,   true  },
    { at_UniAssociation,wt_Interface,   wt_Interface,   true,   true,   true,   true  },
    { at_UniAssociation,wt_Interface,   wt_Class,       true,   true,   true,   true  },
    { at_UniAssociation,wt_Class,       wt_Interface,   true,   true,   true,   true  },
    { at_UniAssociation,wt_Class,       wt_Datatype,    true,   true,   true,   true  },
    { at_UniAssociation,wt_Class,       wt_Enum,        true,   true,   true,   true  },
    { at_UniAssociation,wt_Actor,       wt_Actor,       true,   false,  false,  false },
    { at_UniAssociation,wt_UseCase,     wt_UseCase,     true,   false,  false,  false },
    { at_UniAssociation,wt_UseCase,     wt_Actor,       true,   false,  false,  false },
    { at_Generalization,wt_Class,       wt_Datatype,    false,  false,  false,  false },
    { at_Generalization,wt_Class,       wt_Class,       false,  false,  false,  false },
    { at_Generalization,wt_Interface,   wt_Interface,   false,  false,  false,  false },
    { at_Generalization,wt_UseCase,     wt_UseCase,     false,  false,  false,  false },
    { at_Generalization,wt_Actor,       wt_Actor,       false,  false,  false,  false },
    { at_Generalization,wt_Component,   wt_Interface,   false,  false,  false,  false },
    { at_Aggregation,   wt_Class,       wt_Class,       true,   true,   false,  true  },
    { at_Aggregation,   wt_Class,       wt_Interface,   true,   true,   false,  false },
    { at_Aggregation,   wt_Class,       wt_Enum,        true,   true,   false,  false },
    { at_Aggregation,   wt_Class,       wt_Datatype,    true,   true,   false,  false },
    { at_Dependency,    wt_Class,       wt_Class,       true,   false,  false,  true },
    { at_Dependency,    wt_UseCase,     wt_UseCase,     true,   false,  false,  false },
    { at_Dependency,    wt_Actor,       wt_Actor,       true,   false,  false,  false },
    { at_Dependency,    wt_Actor,       wt_UseCase,     true,   false,  false,  false },
    { at_Dependency,    wt_Package,     wt_Package,     true,   true,   true,   true  },
    { at_Dependency,    wt_Package,     wt_Class,       true,   true,   true,   true  },
    { at_Dependency,    wt_Class,       wt_Package,     true,   true,   true,   true  },
    { at_Dependency,    wt_Package,     wt_Interface,   true,   true,   true,   true  },
    { at_Dependency,    wt_Interface,   wt_Package,     true,   true,   true,   true  },
    { at_Dependency,    wt_Interface,   wt_Interface,   true,   true,   true,   true  },
    { at_Dependency,    wt_Interface,   wt_Class,       true,   true,   true,   true  },
    { at_Dependency,    wt_Class,       wt_Interface,   true,   true,   true,   true  },
    { at_Dependency,    wt_Class,       wt_Datatype,    true,   true,   true,   true  },
    { at_Dependency,    wt_Class,       wt_Enum,        true,   true,   true,   true  },
    { at_Dependency,    wt_Component,   wt_Component,   true,   true,   true,   true  },
    { at_Dependency,    wt_Component,   wt_Interface,   true,   true,   true,   true  },
    { at_Dependency,    wt_Component,   wt_Artifact,    true,   false,  false,  false },
    { at_Dependency,    wt_Node,        wt_Component,   true,   false,  false,  false },
    { at_Realization,   wt_Class,       wt_Interface,   false,  false,  false,  false },
    { at_Realization,   wt_Interface,   wt_Package,     false,  false,  false,  false },
    { at_Realization,   wt_Interface,   wt_Interface,   false,  false,  false,  false },
    { at_Realization,   wt_Component,   wt_Interface,   false,  false,  false,  false },
    { at_Realization,   wt_Package,     wt_Interface,   false,  false,  false,  false },
    { at_Composition,   wt_Class,       wt_Class,       true,   true,   false,  true  },
    { at_Composition,   wt_Class,       wt_Interface,   true,   true,   false,  false },
    { at_Composition,   wt_Class,       wt_Enum,        true,   true,   false,  false },
    { at_Composition,   wt_Class,       wt_Datatype,    false,  false,  false,  false },
    { at_Composition,   wt_Class,       wt_Class,       false,  false,  false,  false },
    { at_Containment,   wt_Package,     wt_Class,       false,  false,  true,   false },
    { at_Containment,   wt_Package,     wt_Interface,   false,  false,  true,   false },
    { at_Containment,   wt_Package,     wt_Enum,        false,  false,  true,   false },
    { at_Containment,   wt_Package,     wt_Package,     false,  false,  true,   false },
    { at_Containment,   wt_Package,     wt_Component,   false,  false,  true,   false },
    { at_Containment,   wt_Class,       wt_Class,       false,  false,  true,   false },
    { at_Containment,   wt_Class,       wt_Interface,   false,  false,  true,   false },
    { at_Containment,   wt_Class,       wt_Enum,        false,  false,  true,   false },
    { at_Containment,   wt_Interface,   wt_Class,       false,  false,  true,   false },
    { at_Containment,   wt_Interface,   wt_Interface,   false,  false,  true,   false },
    { at_Containment,   wt_Interface,   wt_Enum,        false,  false,  true,   false },
    { at_Containment,   wt_Component,   wt_Component,   false,  false,  true,   false },
    { at_Containment,   wt_Component,   wt_Artifact,    false,  false,  true,   false },
    { at_Coll_Message,  wt_Object,      wt_Object,      true,   false,  true,   true  },
    { at_State,         wt_State,       wt_State,       true,   false,  true,   true  },
    { at_State,         wt_ForkJoin,    wt_State,       true,   false,  true,   true  },
    { at_State,         wt_State,       wt_ForkJoin,    true,   false,  true,   true  },
    { at_Activity,      wt_Activity,    wt_Activity,    true,   false,  true,   true  },
    { at_Activity,      wt_ForkJoin,    wt_Activity,    true,   false,  true,   true  },
    { at_Activity,      wt_Activity,    wt_ForkJoin,    true,   false,  true,   true  },
    { at_Anchor,        wt_Class,       wt_Note,        false,  false,  false,  false },
    { at_Anchor,        wt_Package,     wt_Note,        false,  false,  false,  false },
    { at_Anchor,        wt_Interface,   wt_Note,        false,  false,  false,  false },
    { at_Anchor,        wt_Datatype,    wt_Note,        false,  false,  false,  false },
    { at_Anchor,        wt_Enum,        wt_Note,        false,  false,  false,  false },
    { at_Anchor,        wt_Object,      wt_Note,        false,  false,  false,  false },
    { at_Anchor,        wt_Actor,       wt_Note,        false,  false,  false,  false },
    { at_Anchor,        wt_UseCase,     wt_Note,        false,  false,  false,  false },
    { at_Anchor,        wt_Message,     wt_Note,        false,  false,  false,  false },
    { at_Anchor,        wt_State,       wt_Note,        false,  false,  false,  false },
    { at_Anchor,        wt_Activity,    wt_Note,        false,  false,  false,  false },
    { at_Relationship,  wt_Entity,      wt_Entity,      true,   true,   true,   true  },
};

int AssocRules::m_nNumRules = sizeof( m_AssocRules ) / sizeof( AssocRules::Assoc_Rule );

