/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ASSOCIATIONWIDGETDATA_H
#define ASSOCIATIONWIDGETDATA_H

#include "linepath.h"
#include "umlnamespace.h"

class FloatingTextData;

/**
 *	This class holds all the information
 *	from an association
 *	@author Gustavo Madrigal
 */
class AssociationWidgetData {
	friend class AssociationWidget;
	friend class LinePath;
public:
	/**
	 * 	Constructor
	 */
	AssociationWidgetData();

	/**
	* 	Deconstructor
	*/
	virtual ~AssociationWidgetData();


	/**
	* 	Copy Constructor
	*/
	AssociationWidgetData(AssociationWidgetData & Other);

	/**
	* 	Overloaded '=' operator
	*/
	virtual AssociationWidgetData & operator=(AssociationWidgetData & Other);

	/**
	* 	Overloaded '==' operator
	*/
	virtual bool  operator==(AssociationWidgetData & Other);

	/**
	* Write property of FloatingTextData* m_pMultiDataA.
	*/
	virtual void setMultiDataA( FloatingTextData* pMultiData);

	/**
	* Read property of FloatingTextData* m_pMultiDataA.
	*/
	virtual FloatingTextData* getMultiDataA();

	/**
	* Write property of FloatingTextData* m_pMultiDataB.
	*/
	virtual void setMultiDataB( FloatingTextData* pData);

	/**
	* Read property of FloatingTextData* m_pMultiDataB.
	*/
	virtual FloatingTextData* getMultiDataB();

	/**
	 * Read the FloatingTextData * which represents the changeability 
	 * of role A in this association
	 */
	virtual FloatingTextData* getChangeDataA();

	/**
	 * Read the FloatingTextData * which represents the changeability 
	 * of role B in this association
	 */
	virtual FloatingTextData* getChangeDataB();

	/**
	 * Write property of FloatingTextData* m_pChangeDataA
	 */ 
	virtual void setChangeDataA (FloatingTextData *pData);

	/**
	 * Write property of FloatingTextData* m_pChangeDataB
	 */
	virtual void setChangeDataB (FloatingTextData *pData);

	/**
	* Write property of FloatingTextData* m_pRoleData.
	*/
	virtual void setNameData( FloatingTextData* pData);

	/**
	* Read property of FloatingTextData* m_pRoleData.
	*/
	virtual FloatingTextData* getNameData();

	/**
	* Write property of FloatingTextData* m_pRoleData.
	*/
	virtual void setRoleAData( FloatingTextData* pData);
	virtual void setRoleBData( FloatingTextData* pData);

	/**
	* Read property of FloatingTextData* m_pRoleData.
	*/
	virtual FloatingTextData* getRoleAData();
	virtual FloatingTextData* getRoleBData();

	/**
	* Read property of QString m_pRoleADoc.
	*/
	QString getRoleADoc() {
		return m_RoleADoc;
	}

	/**
	 * Write property of QString m_pRoleADoc.
	 */
	void setRoleADoc(QString doc) {
		m_RoleADoc = doc;
	}
		
	/**
	 * Read property of QString m_pRoleBDoc.
	 */
	QString getRoleBDoc() {
		return m_RoleBDoc;
	}

	/**
	 * Write property of QString m_pRoleBDoc.
	 */
	void setRoleBDoc(QString doc) {
		m_RoleBDoc = doc;
	}

	/**
	 * Write property of m_Visibitity[AB]
	 */
	virtual void setVisibilityA ( Scope scope);
	virtual void setVisibilityB ( Scope scope);

	/**
	 * Read property of m_Visibitity[AB]
	 */
	virtual Scope getVisibilityA();
	virtual Scope getVisibilityB();
	
	/**
	 * Write property of m_Changeability[AB]
	 */
	virtual void setChangeabilityA ( Changeability_Type value);
	virtual void setChangeabilityB ( Changeability_Type value);
		  
	/**
	 * Read property of m_Changeability[AB]
	 */
	virtual Changeability_Type getChangeabilityA();
	virtual Changeability_Type getChangeabilityB();

	/**
	* Write property of AssociationType AssocType.
	*/
	virtual void setAssocType( Uml::Association_Type Type);

	/**
	* Read property of AssociationType AssocType.
	*/
	virtual  Uml::Association_Type getAssocType();

	/**
	* Write property of int m_nWidgetAID.
	*/
	virtual void setWidgetAID( int AID);

	/**
	* Read property of int m_nWidgetAID.
	*/
	virtual int getWidgetAID();

	/**
	* Write property of int m_nWidgetBID.
	*/
	virtual void setWidgetBID( int BID);

	/**
	* Read property of int m_nWidgetBID.
	*/
	virtual int getWidgetBID();

	/**
	* Prints the association's data to stderr
	*/
	void print2cerr();

	/**
	*  Sets the Association line index for widgetA
	*/
	void setIndexA(int index) {
		m_nIndexA = index;
	}

	/**
	*  Returns the Association line index for widgetA
	*/
	int getIndexA() {
		return m_nIndexA;
	}

	/**
	*  Sets the Association line index for widgetB
	*/
	void setIndexB(int index) {
		m_nIndexB = index;
	}

	/**
	*  Returns the Association line index for widgetB
	*/
	int getIndexB() {
		return m_nIndexB;
	}

	/**
	*  Sets the total count on the Association region for widgetA
	*/
	void setTotalCountA(int count) {
		m_nTotalCountA = count;
	}

	/**
	*  Returns the total count on the Association region for widgetA
	*/
	int getTotalCountA() {
		return m_nTotalCountA;
	}

	/**
	*  Sets the total count on the Association region for widgetB
	*/
	void setTotalCountB(int count) {
		m_nTotalCountB = count;
	}

	/**
	*  Region the total count on the Association region for widgetB
	*/
	int getTotalCountB() {
		return m_nTotalCountB;
	}

	/**
	*  	Returns the associations documentation.
	*/
	QString getDoc() {
		return m_Doc;
	}

	/**
	*	Sets the associations documentation.
	*/
	void setDoc( QString doc) {
		m_Doc = doc;
	}

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );

	void cleanup();

protected:

	/**
	* The Data from the Name Text widget 
	*/
	FloatingTextData* m_pNameData;

	/**
	* The Data from the Multiplicity Text widget at the starting side of the association
	*/
	FloatingTextData* m_pMultiDataA;

	/**
	* The Data from the Multiplicity Text widget at the ending side of the association
	*/
	FloatingTextData* m_pMultiDataB;

	/**
	* The Data from the Association's RoleA FloatingText widget
	*/
	FloatingTextData* m_pRoleAData;

	/**
	 * The Data from the Association's RoleB FloatingText widget
	 */
	FloatingTextData* m_pRoleBData;

	/**
	 * The widget which displays Changeability for role A
	 */
	FloatingTextData* m_pChangeDataA;

	/**
	 * The widget which displays Changeability for role B
	 */
	FloatingTextData* m_pChangeDataB;

	/**
	* This is the type of the association
	*/
	Uml::Association_Type m_AssocType;

	/**
	* The definition points for the association line
	*/
	LinePath m_LinePath;

	/**
	 * the definition of Rolename A visibility
	 */
	Scope m_VisibilityA;

	/**
	 * the definition of Rolename B visibility
	 */
	Scope m_VisibilityB;

	/**
	 * the definition of Rolename A changeability
	 */
	Changeability_Type m_ChangeabilityA;

	/**
	 * the definition of Rolename B changeability
	 */
	Changeability_Type m_ChangeabilityB;

	/*
	 * the Role A documentation.
	 */
	QString m_RoleADoc;

	/**
	 * the Role B documentation.
	 */
	QString m_RoleBDoc;

	/**
	* This member holds the identification number the the UMLWidget at the starting side
	*	of the association
	*/
	int m_nWidgetAID;

	/**
	* This member holds the identification number the the UMLWidget at the ending side
	*	of the association
	*/
	int m_nWidgetBID;

	/**
	* 	The index of where the line is on the region for widget a
	*/
	int m_nIndexA;

	/**
	*   	The index of where the line is on the region for widget b
	*/
	int m_nIndexB;

	/**
	* 	The total amount of associations on the region widget a line is on.
	*/
	int m_nTotalCountA;

	/**
	*	The total amount of associations on the region widget b line is on.
	*/
	int m_nTotalCountB;

	/**
	* 	The documentation for the association.
	*/
	QString	m_Doc;

};

#endif
