/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>

#include "assocrules.h"
#include "umlwidget.h"
#include "associationwidget.h"
#include "statewidgetdata.h"
#include "activitywidgetdata.h"

AssocRules::AssocRules() {}

AssocRules::~AssocRules() {}

bool AssocRules::allowAssociation( Association_Type assocType, UMLWidget * widget ) {
	UMLWidget_Type widgetType = widget -> getBaseType();
	bool bValid = false;
	for( int i = 0; i < m_nNumRules; i++ )
		if( assocType == m_AssocRules[ i ].assoc_type )
			if( widgetType == m_AssocRules[ i ].widgetA_type || widgetType == m_AssocRules[ i ].widgetB_type )
				bValid =  true;
	if( !bValid )
		return false;
	AssociationWidgetList list = widget -> getData() -> getAssocList();
	AssociationWidgetListIt it( list );
	AssociationWidget * assoc = 0;
	switch( assocType ) {
		case at_Association:
		case at_UniAssociation:
		case at_Dependency:
		case at_Coll_Message:
		case at_Aggregation:
		case at_Composition:
			return true;//doesn't matter whats already connected to widget
			break;

		case at_Generalization://can have many sub/super types
		case at_Realization:
			return true;
			break;

		case at_Implementation://one connected to widget only (a or b)
			while( ( assoc = it.current() ) ) {
				if( assoc -> getAssocType() == at_Implementation ||
				        assoc -> getAssocType() == at_Realization )
					return false;
				++it;
			}
			return true;
			break;

		case at_State:
			return ( dynamic_cast<StateWidget *>( widget ) -> getStateType() != StateWidget::End );
			break;

		case at_Activity:
			return ( dynamic_cast<ActivityWidget *>( widget ) -> getActivityType() != ActivityWidget::End );
			break;

		case at_Anchor:
			return true;
			break;
		default:
			break;
	}
	return false;
}

bool AssocRules::allowAssociation( Association_Type assocType, UMLWidget * widgetA, UMLWidget * widgetB ) {
	UMLWidget_Type widgetTypeA = widgetA -> getBaseType();
	UMLWidget_Type widgetTypeB = widgetB -> getBaseType();
	bool bValid = false;
	for( int i = 0; i < m_nNumRules; i++ )
		if( assocType == m_AssocRules[ i ].assoc_type )
			if( ( widgetTypeA == m_AssocRules[ i ].widgetA_type && widgetTypeB == m_AssocRules[ i ].widgetB_type ) ||
			        ( widgetTypeB == m_AssocRules[ i ].widgetA_type && widgetTypeA == m_AssocRules[ i ].widgetB_type ) )
				bValid = true;
	if( !bValid )
		return false;
	AssociationWidgetList list = widgetB -> getData() -> getAssocList();
	AssociationWidgetListIt it( list );
	AssociationWidget * assoc = 0;
	switch( assocType ) {
		case at_Association:
		case at_UniAssociation:
		case at_Dependency:
		case at_Coll_Message:
		case at_Aggregation:
		case at_Composition:
			return true;//doesn't matter whats already connected to widget
			break;

		case at_Generalization://can have many sub/super types but can't sup/sub each
			while( ( assoc = it.current() ) ) {
				if( ( widgetA == assoc -> getWidgetA() || widgetA == assoc -> getWidgetB() )
				        && assoc -> getAssocType() == at_Generalization )
					return false;
				++it;
			}
			return true;
			break;

		case at_Realization: // can only connect to abstract (interface) classes
			while( ( assoc = it.current() ) ) {
				if( ( widgetA == assoc -> getWidgetA() || widgetA == assoc -> getWidgetB() )
				        && assoc -> getAssocType() == at_Realization )
					return false;
				++it;
			}
			return widgetB->getUMLObject()->getAbstract();
			break;

		case at_Implementation://one connected to widget only (a or b)
			while( ( assoc = it.current() ) ) {
				if( assoc -> getAssocType() == at_Implementation ||
				        assoc -> getAssocType() == at_Realization )
					return false;
				++it;
			}
			return true;
			break;

		case at_State:
			if( dynamic_cast<StateWidget *>( widgetB ) ->
			        getStateType() == StateWidget::Initial )
				return false;
			if( dynamic_cast<StateWidget *>
			        ( widgetB ) -> getStateType() == StateWidget::End &&
			        dynamic_cast<StateWidget *>( widgetA ) -> getStateType() != StateWidget::Normal )
				return false;
			return true;
			break;

		case at_Activity:
			// no transitions to initial activity allowed
			if( dynamic_cast<ActivityWidget *>
			        ( widgetB ) -> getActivityType() == ActivityWidget::Initial )
				return false;
			// only from a normal, branch or fork activity to the end
			if( dynamic_cast<ActivityWidget *>
			        ( widgetB ) -> getActivityType() == ActivityWidget::End &&
							dynamic_cast<ActivityWidget *>( widgetA ) -> getActivityType() != ActivityWidget::Normal &&
							dynamic_cast<ActivityWidget *>( widgetA ) -> getActivityType() != ActivityWidget::Branch &&
							dynamic_cast<ActivityWidget *>( widgetA ) -> getActivityType() != ActivityWidget::Fork
				)
					return false;
			// only Forks and Branches can have more than one "outgoing" transition
			if( dynamic_cast<ActivityWidget *>
			        ( widgetA ) -> getActivityType() != ActivityWidget::Fork &&
					dynamic_cast<ActivityWidget *>
			        ( widgetA ) -> getActivityType() != ActivityWidget::Branch )
					{
						QList<AssociationWidget> list = widgetA->getData()->getAssocList();
						for (AssociationWidget *assoc = list.first(); assoc; assoc = list.next())
								if( assoc->getWidgetA() == widgetA)
										return false;
					}
			return true;
			break;

		case at_Anchor:
			return true;
			break;

		default:
			break;
	}
	return false;
}

bool AssocRules::allowRole( Association_Type assocType ) {
	for( int i = 0; i < m_nNumRules; i++ )
		if( assocType == m_AssocRules[ i ].assoc_type )
			return m_AssocRules[ i ].role;
	return false;
}

bool AssocRules::allowMultiplicity( Association_Type assocType, UMLWidget_Type widgetType ) {
	for( int i = 0; i < m_nNumRules; i++ )
		if( assocType == m_AssocRules[ i ].assoc_type )
			if( widgetType == m_AssocRules[ i ].widgetA_type || widgetType == m_AssocRules[ i ].widgetB_type )
				return m_AssocRules[ i ].multiplicity;
	return false;
}

bool AssocRules::allowSelf( Association_Type assocType, UMLWidget_Type widgetType ) {
	for( int i = 0; i < m_nNumRules; i++ )
		if( assocType == m_AssocRules[ i ].assoc_type )
			if( widgetType == m_AssocRules[ i ].widgetA_type || widgetType == m_AssocRules[ i ].widgetB_type )
				return m_AssocRules[ i ].self;

	return false;
}

AssocRules::Assoc_Rule AssocRules::m_AssocRules []= {
            //		Association					widgetA				widgetB				role	multiplicity	directional	self
            { at_Association,				wt_Class,			wt_Class,			true,		true,			true,			true },
            { at_Association,				wt_Actor,			wt_UseCase,		true,		false,		false,		false },
            { at_Association,				wt_UseCase,		wt_UseCase,		true,		false,		false,		false },
            { at_Association,				wt_Actor,			wt_Actor,			true,		false,		false,		false },
            { at_Association,				wt_Actor,			wt_UseCase,		true,		false,		false,		false },
            { at_UniAssociation,		wt_Class,			wt_Class,			true,		true,			true,			true },
            { at_UniAssociation,		wt_Actor,			wt_Actor,			true,		false,		false,		false },
            { at_UniAssociation,		wt_UseCase,		wt_UseCase,		true,		false,		false,		false },
            { at_UniAssociation,		wt_UseCase,		wt_Actor,			true,		false,		false,		false },
            { at_Generalization,		wt_Class,			wt_Class,			false,	false,		false,		false },
            { at_Generalization,		wt_UseCase,		wt_UseCase,		false,	false,		false,		false },
            { at_Generalization,		wt_Actor,			wt_Actor,			false,	false,		false,		false },
            {	at_Aggregation,				wt_Class,			wt_Class,			true,		true,			false,		false },
            { at_Dependency,				wt_Class,			wt_Class,			false,	false,		false,		false },
            { at_Dependency,				wt_UseCase,		wt_UseCase,		false,	false,		false,		false },
            { at_Dependency,				wt_Actor,			wt_Actor,			false,	false,		false,		false },
            { at_Dependency,				wt_Actor,			wt_UseCase,		false,	false,		false,		false },
            { at_Realization,				wt_Class,			wt_Class,			false,	false,		false,		false },
            { at_Composition,				wt_Class,			wt_Class,			true,		true,			false,		false },
            { at_Implementation,		wt_Class,			wt_Class,			false,	false,		false,		false },
            { at_Coll_Message,			wt_Object,		wt_Object,		true,		false,		true,			true },
            { at_State,							wt_State,			wt_State,			true,		false,		true,			true },
            { at_Activity,					wt_Activity,	wt_Activity,	true,		false,		true,			true },
            { at_Anchor,						wt_Class,			wt_Note,			false,	false,		false,		false},
            { at_Anchor,						wt_Object,		wt_Note,			false,	false,		false,		false},
            { at_Anchor,						wt_Actor,			wt_Note,			false,	false,		false,		false},
            { at_Anchor,						wt_UseCase,		wt_Note,			false,	false,		false,		false},
            { at_Anchor,						wt_Message,		wt_Note,			false,	false,		false,		false},
            { at_Anchor,						wt_State,			wt_Note,			false,	false,		false,		false},
            { at_Anchor,						wt_Activity,	wt_Note,			false,	false,		false,		false},
        };

int AssocRules::m_nNumRules = sizeof( m_AssocRules ) / sizeof( AssocRules::Assoc_Rule );

