/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ASSOCRULES_H
#define ASSOCRULES_H

#include "umlnamespace.h"
namespace std
{ class type_info; }

class UMLWidget;

using namespace Uml;

/**
 * Used to determine rules for associations.
 *
 *	@author Paul Hensgen
 */
class AssocRules {
public:
	/**
	* 	Constructor
	*/
	AssocRules();

	/**
	*  	Deconstructor
	*/
	~AssocRules();

	/**
	* 	Returns whether an association is going to be allowed for the given values.
	*	This method is used to test if you can start an association.
	*/
	static bool allowAssociation( Association_Type assocType, UMLWidget * widget );
	
	static bool allowAssociation( Association_Type assocType, const std::type_info & );

	/**
	* 	Returns whether an association is valid with the given variables.
	*	This method is used to finish an association
	*/
	static bool allowAssociation( Association_Type assocType, UMLWidget * widgetA, UMLWidget * widgetB );

	/**
	*	Returns whether to allow a role text for the given association type.
	*/
	static bool allowRole( Association_Type assocType );

	/**
	*	Returns whether to allow a multiplicity text for the given association and widget type.
	*/
	static bool allowMultiplicity( Association_Type assocType, UMLWidget_Type widgetType );

	/**
	*	Returns whether to allow an association to self for given variables.
	*/
	static bool allowSelf( Association_Type assocType, UMLWidget_Type widgetType );

	/**
	 *	Returns whether an implements association should be a Realisation or a Generalisation
	 *	as defined in m_AssocRules.
	 */
	static Association_Type isGeneralisationOrRealisation(UMLWidget* widgetA, UMLWidget* widgetB);
private:

	/**
	* 	Structure to help determine association rules.
	*/
	struct Assoc_Rule {
		Association_Type assoc_type;
		UMLWidget_Type widgetA_type;
		UMLWidget_Type widgetB_type;
		bool role;                  //role text
		bool multiplicity;          //multipliciy text on association
		bool directional;           //can have an association of same type going between widget each way
		bool self;                  //association to self
	};

	/**
	* 	Container that holds all the rules.
	*/
	static Assoc_Rule m_AssocRules[];

	/**
	* 	The number of rules known about.
	*/
	static int m_nNumRules;
};

#endif
