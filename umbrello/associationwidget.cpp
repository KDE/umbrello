/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cmath>

#include "activitywidget.h"
#include "umlview.h"
#include "umldoc.h"
#include "umlwidget.h"
#include "messagewidget.h"
#include "umlrole.h"
#include "listpopupmenu.h"
#include "association.h"
#include "associationwidget.h"
#include "assocrules.h"
#include "floatingtext.h"
#include "objectwidget.h"
#include "dialogs/assocpropdlg.h"
#include "dialogs/selectopdlg.h"
#include "inputdialog.h"

#include <kdebug.h>
#include <klocale.h>

// this constructor really only for loading from XMI, otherwise it
// is bad..and shouldnt be allowed as it creates an incomplete
// associationwidget.
AssociationWidget::AssociationWidget(UMLView *view)
	: QObject(view)
{
	init(view);
}

// the preferred constructor
AssociationWidget::AssociationWidget(UMLView *view, UMLWidget* pWidgetA,
				     Association_Type assocType, UMLWidget* pWidgetB )
	: QObject(view)
{

	init(view);

	UMLDoc *umldoc = m_pView->getDocument();

	// set up UMLAssociation obj if assoc is represented and both roles are UML objects
	if (UMLAssociation::assocTypeHasUMLRepresentation(assocType)) {
		UMLObject* umlRoleA = pWidgetA->getUMLObject();
		UMLObject* umlRoleB = pWidgetB->getUMLObject();
		if (umlRoleA != NULL && umlRoleB != NULL) {
			bool swap;
			m_pAssociation = umldoc->findAssociation( assocType, umlRoleA, umlRoleB, &swap );
			if (m_pAssociation == NULL) {
				m_pAssociation = new UMLAssociation( umldoc, assocType, umlRoleA, umlRoleB );
			} else if (swap) {
				UMLWidget *tmp = pWidgetA;
				pWidgetA = pWidgetB;
				pWidgetB = tmp;
			}
			connect(m_pAssociation, SIGNAL(modified()), this,
				SLOT(mergeUMLRepresentationIntoAssociationData()));
			m_pAssociation->nrof_parent_widgets++;
		}
	}

	setWidgetA(pWidgetA);
	setWidgetB(pWidgetB);

	setAssocType(assocType);

	calculateEndingPoints();

	//The AssociationWidget is set to Activated because it already has its side widgets
	setActivated(true);

	// sync UML meta-data to settings here
	mergeAssociationDataIntoUMLRepresentation();

	//collaboration messages need a name label because it's that
	//which handles the right click menu options
	if (getAssocType() == at_Coll_Message) {
		setName("");
		if (m_pAssociation)
			m_pName->setUMLObject( m_pAssociation );
		else
			m_pName->setUMLObject( m_role[B].m_pWidget->getUMLObject() );
	}
}

AssociationWidget::~AssociationWidget() { 
	cleanup();
}

AssociationWidget& AssociationWidget::operator=(AssociationWidget & Other) {
	m_LinePath = Other.m_LinePath;

	m_pView = Other.m_pView;

	if (Other.m_pName) {
		m_pName = new FloatingText(m_pView);
		*m_pName = *(Other.m_pName);
	} else {
		m_pName = NULL;
	}

	for (unsigned r = (unsigned)A; r <= (unsigned)B; r++) {
		WidgetRole& lhs = m_role[r];
		const WidgetRole& rhs = Other.m_role[r];
		lhs.m_nIndex = rhs.m_nIndex;
		lhs.m_nTotalCount = rhs.m_nTotalCount;

		if (rhs.m_pMulti) {
			lhs.m_pMulti = new FloatingText(m_pView);
			*(lhs.m_pMulti) = *(rhs.m_pMulti);
		} else {
			lhs.m_pMulti = NULL;
		}

		if (rhs.m_pRole) {
			lhs.m_pRole = new FloatingText(m_pView);
			*(lhs.m_pRole) = *(rhs.m_pRole);
		} else {
			lhs.m_pRole = NULL;
		}

		if (rhs.m_pChangeWidget) {
			lhs.m_pChangeWidget = new FloatingText(m_pView);
			*(lhs.m_pChangeWidget) = *(rhs.m_pChangeWidget);
		} else {
			lhs.m_pChangeWidget = NULL;
		}

		lhs.m_pWidget = rhs.m_pWidget;
		lhs.m_OldCorner = rhs.m_OldCorner;
		lhs.m_nCornerRegion = rhs.m_nCornerRegion;
		lhs.m_WidgetRegion = rhs.m_WidgetRegion;
	}

	m_bActivated = Other.m_bActivated;
	m_unNameLineSegment = Other.m_unNameLineSegment;
	m_bFocus = Other.m_bFocus;
	m_pMenu = Other.m_pMenu;
	m_pAssociation = Other.m_pAssociation;
	m_bSelected = Other.m_bSelected;
	m_nMovingPoint = Other.m_nMovingPoint;

	return *this;
}

bool AssociationWidget::operator==(AssociationWidget & Other) {
	if( this == &Other )
		return true;

	if( !m_pAssociation || !Other.m_pAssociation ) {
		if( !Other.m_pAssociation && m_pAssociation )
			return false;
		if( Other.m_pAssociation && !m_pAssociation )
			return false;
	} else if( m_pAssociation != Other.m_pAssociation )
		return false;

	if (getAssocType() != Other.getAssocType())
		return false;

	if (getWidgetAID() != Other.getWidgetAID())
		return false;

	if (getWidgetBID() != Other.getWidgetBID())
		return false;

	return true;
}

bool AssociationWidget::operator!=(AssociationWidget & Other) {
	return !(*this == Other);
}

FloatingText* AssociationWidget::getMultiAWidget() {
	return m_role[A].m_pMulti;
}

QString AssociationWidget::getMultiA() const
{
	if (m_role[A].m_pMulti == NULL)
		return "";
	return m_role[A].m_pMulti->getText();
}

FloatingText* AssociationWidget::getMultiBWidget() {
	return m_role[B].m_pMulti;
}

QString AssociationWidget::getMultiB() const {
	if (m_role[B].m_pMulti == NULL)
		return "";
	return m_role[B].m_pMulti->getText();
}

FloatingText* AssociationWidget::getNameWidget()
{
	return m_pName;
}

QString AssociationWidget::getName() const {
	if (m_pName == NULL)
		return "";
	return m_pName->getText();
}

QString AssociationWidget::getDoc() const {
	if (m_pAssociation)
		return m_pAssociation->getDoc();
	return m_Doc;
}

FloatingText* AssociationWidget::getRoleAWidget()
{
	return m_role[A].m_pRole;
}

FloatingText* AssociationWidget::getRoleBWidget()
{
	return m_role[B].m_pRole;
}

FloatingText* AssociationWidget::getChangeWidgetA()
{
	return m_role[A].m_pChangeWidget;
}

FloatingText* AssociationWidget::getChangeWidgetB()
{
	return m_role[B].m_pChangeWidget;
}

QString AssociationWidget::getRoleNameA() const {
	if (m_role[A].m_pRole == NULL)
		return "";
	return m_role[A].m_pRole->getText();
}

QString AssociationWidget::getRoleADoc() const {
	if (m_pAssociation)
		return m_pAssociation->getRoleADoc();
	return "";
}

QString AssociationWidget::getRoleNameB() const {
	if (m_role[B].m_pRole == NULL)
		return "";
	return m_role[B].m_pRole->getText();
}

QString AssociationWidget::getRoleBDoc() const {
	if (m_pAssociation)
		return m_pAssociation->getRoleBDoc();
	return "";
}

void AssociationWidget::setName(QString strName) {
	bool newLabel = false;
        if(!m_pName) {
		newLabel = true;
                m_pName = new FloatingText(m_pView, CalculateNameType(tr_Name), strName);
                m_pName->setAssoc(this);
		m_pName->setUMLObject(m_role[B].m_pWidget->getUMLObject());
        } else {
		if (m_pName->getText() == "") {
			newLabel = true;
		}
                m_pName->setText(strName);
        }
        m_pName->setActivated();

	// set attribute of UMLAssociation associated with this associationwidget
	if (m_pAssociation)
		m_pAssociation->setName(strName);

	if (newLabel) {
		setTextPosition( tr_Name, calculateTextPosition(tr_Name) );
	}

        if(FloatingText::isTextValid(m_pName->getText()))
                m_pName -> show();
        else
                m_pName -> hide();

}

bool AssociationWidget::setMulti(QString strMulti, Role_Type role) {
	bool newLabel = false;
	Text_Role tr = (role == A ? tr_MultiA : tr_MultiB);

	if(!m_role[role].m_pMulti) {
		newLabel = true;
		m_role[role].m_pMulti = new FloatingText(m_pView, tr, strMulti);
		m_role[role].m_pMulti->setAssoc(this);
	} else {
		if (m_role[role].m_pMulti->getText() == "") {
			newLabel = true;
		}
		m_role[role].m_pMulti->setText(strMulti);
	}

	m_role[role].m_pMulti->setActivated();

	if (newLabel) {
		setTextPosition( tr, calculateTextPosition(tr) );
	}

	if(FloatingText::isTextValid(m_role[role].m_pMulti->getText()))
		m_role[role].m_pMulti -> show();
	else
		m_role[role].m_pMulti -> hide();
	return true;
}

bool AssociationWidget::setMultiA(QString strMultiA) {
	Association_Type type = getAssocType();
	//if the association is not supposed to have a Multiplicity FloatingText
	if ( !AssocRules::allowMultiplicity(type, getWidgetA() -> getBaseType()) &&
		type != at_Coll_Message && type != at_Coll_Message_Self) {
		return false;
	}
	bool status = setMulti(strMultiA, A);
	// set attribute of UMLAssociation associated with this associationwidget
	if (m_pAssociation)
		m_pAssociation->setMultiA(strMultiA);
	return status;
}

bool AssociationWidget::setMultiB(QString strMultiB) {
	Association_Type type = getAssocType();
	//if the association is not supposed to have a Multiplicity FloatingText
	if( !AssocRules::allowMultiplicity( type, getWidgetB() -> getBaseType() ) )
		return false;
	bool status = setMulti(strMultiB, B);
	// set attribute of UMLAssociation associated with this associationwidget
	if (m_pAssociation)
		m_pAssociation->setMultiB(strMultiB);
	return status;
}


bool AssociationWidget::setRoleName (QString strRole, Role_Type role) {
	bool newLabel = false;
	Association_Type type = getAssocType();
	//if the association is not supposed to have a Role FloatingText
	if( !AssocRules::allowRole( type ) )  {
		return false;
	}

	Text_Role tr = (role == A ? tr_RoleAName : tr_RoleBName);
	if(!m_role[role].m_pRole) {
		newLabel = true;
		m_role[role].m_pRole = new FloatingText(m_pView, tr, strRole);
		m_role[role].m_pRole->setAssoc(this);
		Scope scope = (role == A ? getVisibilityA() : getVisibilityB());
		m_role[role].m_pRole->setPreText(UMLAssociation::ScopeToString(scope));
	} else {
		if (m_role[role].m_pRole->getText() == "") {
			newLabel = true;
		}
		m_role[role].m_pRole->setText(strRole);
	}

	// set attribute of UMLAssociation associated with this associationwidget
	if (m_pAssociation) {
		if (role == A)
			m_pAssociation->setRoleNameA(strRole);
		else
			m_pAssociation->setRoleNameB(strRole);
	}
	m_role[role].m_RoleName = strRole;

	m_role[role].m_pRole->setActivated();

	if (newLabel) {
		setTextPosition( tr, calculateTextPosition(tr) );
	}

	if(FloatingText::isTextValid(m_role[role].m_pRole->getText()))
		m_role[role].m_pRole -> show();
	else
		m_role[role].m_pRole -> hide();
	return true;
}

bool AssociationWidget::setRoleNameA (QString strRole) {
	return setRoleName(strRole, A);
}

void AssociationWidget::setDoc (QString doc) {
	if (m_pAssociation)
		m_pAssociation->setDoc(doc);
}

void AssociationWidget::setRoleADoc (QString doc) {
	if (m_pAssociation)
		m_pAssociation->setRoleADoc(doc);
	m_role[A].m_RoleDoc = doc;
}

bool AssociationWidget::setRoleNameB(QString strRole) {
	return setRoleName(strRole, B);
}

void AssociationWidget::setRoleBDoc (QString doc) {
	if (m_pAssociation)
		m_pAssociation->setRoleBDoc(doc);
	m_role[B].m_RoleDoc = doc;
}

Scope AssociationWidget::getVisibilityA() const
{
	if (m_pAssociation)
		return m_pAssociation->getVisibilityA();
	return m_role[A].m_Visibility;
}

void AssociationWidget::setVisibility (Scope value, Role_Type role)
{
	m_role[role].m_Visibility = value;
	// update role pre-text attribute as appropriate
	if (m_role[role].m_pRole) {
		QString scopeString = UMLAssociation::ScopeToString(value);
		m_role[role].m_pRole->setPreText(scopeString);
	}
}

void AssociationWidget::setVisibilityA (Scope value)
{
	if (value == getVisibilityA())
		return;
	// update our attribute
	if (m_pAssociation)
		m_pAssociation->setVisibilityA(value);
	setVisibility(value, A);
}

Scope AssociationWidget::getVisibilityB() const
{
	if (m_pAssociation)
		return m_pAssociation->getVisibilityB();
	return m_role[B].m_Visibility;
}

void AssociationWidget::setVisibilityB (Scope value)
{
	if (value == getVisibilityB())
		return;
	// update our attribute
	if (m_pAssociation)
		m_pAssociation->setVisibilityB(value);
	setVisibility(value, B);
}

Changeability_Type AssociationWidget::getChangeabilityA() const
{
	if (m_pAssociation)
		return m_pAssociation->getChangeabilityA();
	return m_role[A].m_Changeability;
}

void AssociationWidget::setChangeabilityA (Changeability_Type value)
{
	Changeability_Type old = getChangeabilityA();
	if (value == old)
		return;

	QString changeString = UMLAssociation::ChangeabilityToString(value);
	// update our attribute
	if (m_pAssociation)
		m_pAssociation->setChangeabilityA(value);
	m_role[A].m_Changeability = value;
	// update our string representation
	setChangeWidget(changeString, A);
}

bool AssociationWidget::setChangeWidget(QString strChangeWidget, Role_Type role) {
	bool newLabel = false;
	Text_Role tr = (role == A ? tr_ChangeA : tr_ChangeB);

        if(!m_role[role].m_pChangeWidget) {
		newLabel = true;
                m_role[role].m_pChangeWidget = new FloatingText(m_pView, tr, strChangeWidget);
                m_role[role].m_pChangeWidget->setAssoc(this);
		m_role[role].m_pChangeWidget->setPreText("{"); // all types have this
		m_role[role].m_pChangeWidget->setPostText("}"); // all types have this
        } else {
		if (m_role[role].m_pChangeWidget->getText() == "") {
			newLabel = true;
		}
                m_role[role].m_pChangeWidget->setText(strChangeWidget);
        }
        m_role[role].m_pChangeWidget->setActivated();

	if (newLabel) {
		setTextPosition( tr, calculateTextPosition(tr) );
	}

        if(FloatingText::isTextValid(m_role[role].m_pChangeWidget->getText()))
                m_role[role].m_pChangeWidget -> show();
        else
                m_role[role].m_pChangeWidget -> hide();

        return true;
}

Changeability_Type AssociationWidget::getChangeabilityB() const
{
	if (m_pAssociation)
		return m_pAssociation->getChangeabilityB();
	return m_role[B].m_Changeability;
}

void AssociationWidget::setChangeabilityB(Changeability_Type value) {
	Changeability_Type old = getChangeabilityB();
	if (value == old)
		return;

	QString changeString = UMLAssociation::ChangeabilityToString(value);
	// update our attribute
	if (m_pAssociation)
		m_pAssociation->setChangeabilityB(value);
	m_role[B].m_Changeability = value;
	// update our string representation
	setChangeWidget(changeString, B);
}

bool AssociationWidget::activate() {
	if(isActivated())
		return true;

	bool status = true;
	Association_Type type = getAssocType();

	if (m_role[A].m_pWidget == NULL)
		setWidgetA(m_pView->findWidget(getWidgetAID()));
	if (m_role[B].m_pWidget == NULL)
		setWidgetB(m_pView->findWidget(getWidgetBID()));

	if(!m_role[A].m_pWidget || !m_role[B].m_pWidget) {
		kdDebug() << "Can't make association" << endl;
		return false;
	}

	calculateEndingPoints();
	m_LinePath.activate();

	/*
	  There used to be calls to to setRole( ), setMultiA, setMultiB here.  But I
	  have removed them and cut and pasted most of that code here - why??
	  There was a call to SynchronizeData in each of them which deleted the
	  data we needed.  The other way is to add a loading variable to miss that call
	  but just as easy to put the code we need here.
	*/

	if (AssocRules::allowRole(type)) {
		for (unsigned r = A; r <= B; r++) {
			WidgetRole& robj = m_role[r];
			if (robj.m_pRole == NULL)
				continue;
			robj.m_pRole->setAssoc(this);
			Text_Role tr = (r == A ? tr_RoleAName : tr_RoleBName);
			robj.m_pRole->setRole(tr);
			Scope scope;
			if (r == A)
				scope = getVisibilityA();
			else
				scope = getVisibilityB();
			robj.m_pRole->setPreText(UMLAssociation::ScopeToString(scope));

			if (FloatingText::isTextValid(robj.m_pRole->getText()))
				robj.m_pRole -> show();
			else
				robj.m_pRole -> hide();
			if (m_pView->getType() == dt_Collaboration)
				robj.m_pRole->setUMLObject(robj.m_pWidget->getUMLObject());
			robj.m_pRole->activate();
		}
	}

        if( m_pName != NULL ) {
		m_pName->setAssoc(this);
                m_pName->setRole( CalculateNameType(tr_Name) );

		if ( FloatingText::isTextValid(m_pName->getText()) ) {
			m_pName-> show();
		} else {
			m_pName-> hide();
		}
                if( m_pView->getType() == dt_Collaboration && m_pName) {
                        m_pName->setUMLObject(m_role[B].m_pWidget->getUMLObject());
                }
		m_pName->activate();
		calculateNameTextSegment();
	}

	for (unsigned r = A; r <= B; r++) {
		WidgetRole& robj = m_role[r];

		FloatingText* pMulti = robj.m_pMulti;
		if (pMulti != NULL &&
		    AssocRules::allowMultiplicity(type, robj.m_pWidget->getBaseType())) {
			pMulti->setAssoc(this);
			Text_Role tr = (r == A ? tr_MultiA : tr_MultiB);
			pMulti->setRole(tr);
			if (FloatingText::isTextValid(pMulti->getText()))
				pMulti -> show();
			else
				pMulti -> hide();
			pMulti->activate();
		}

		FloatingText* pChangeWidget = robj.m_pChangeWidget;
		if (pChangeWidget != NULL ) {
			pChangeWidget->setAssoc(this);
			Text_Role tr = (r == A ? tr_ChangeA : tr_ChangeB);
			pChangeWidget->setRole(tr);
			if (FloatingText::isTextValid(pChangeWidget->getText()))
				pChangeWidget -> show();
			else
				pChangeWidget -> hide ();
			pChangeWidget->activate();
		}
	}

	// CHECK
	// m_LinePath.setAssocType( getAssocType() );
	if(status) {
		m_bActivated = true;
	}
	return status;
}

/** This function calculates which role should be set for the m_pName FloatingText */
Uml::Text_Role AssociationWidget::CalculateNameType(Text_Role defaultRole) {

	Text_Role result = defaultRole;
	if( m_pView -> getType() == dt_Collaboration ) {
		if(m_role[A].m_pWidget == m_role[B].m_pWidget) {
			result = tr_Coll_Message;//for now same as other Coll_Message
		} else {
			result = tr_Coll_Message;
		}
	} else if( m_pView -> getType() == dt_Sequence ) {
		if(m_role[A].m_pWidget == m_role[B].m_pWidget) {
			result = tr_Seq_Message_Self;
		} else {
			result = tr_Seq_Message;
		}
	}

	return result;
}

UMLWidget* AssociationWidget::getWidgetA() {
	return m_role[A].m_pWidget;
}

UMLWidget* AssociationWidget::getWidgetB() {
	return m_role[B].m_pWidget;
}


bool AssociationWidget::setWidgets( UMLWidget* widgetA,
				    Association_Type assocType,
				    UMLWidget* widgetB) {
	//if the association already has a WidgetB or WidgetA associated, then
	//it cannot be changed to other widget, that would require a  deletion
	//of the association and the creation of a new one
	if ((m_role[A].m_pWidget && (m_role[A].m_pWidget != widgetA)) ||
	    (m_role[B].m_pWidget && (m_role[B].m_pWidget != widgetB))) {
		return false;
	}
	setWidgetA(widgetA);
	setAssocType(assocType);
	setWidgetB(widgetB);

	calculateEndingPoints();
	return true;
}

/** Returns true if this association associates WidgetA to WidgetB, otherwise it returns
    false */
bool AssociationWidget::checkAssoc(UMLWidget * widgetA, UMLWidget *widgetB) {
	return (widgetA == m_role[A].m_pWidget && widgetB == m_role[B].m_pWidget);
}

/** CleansUp all the association's data in the related widgets  */
void AssociationWidget::cleanup() {

	//let any other associations know we are going so they can tidy their positions up
	if(m_role[A].m_nTotalCount > 2)
		updateAssociations(m_role[A].m_nTotalCount - 1, m_role[A].m_WidgetRegion, true);
	if(m_role[B].m_nTotalCount > 2)
		updateAssociations(m_role[B].m_nTotalCount - 1, m_role[B].m_WidgetRegion, false);

	for (unsigned r = A; r <= B; r++) {
		WidgetRole& robj = m_role[r];

		if(robj.m_pWidget) {
			robj.m_pWidget->removeAssoc(this);
			robj.m_pWidget = 0;
		}
		if(robj.m_pRole) {
			m_pView->removeWidget(robj.m_pRole);
			robj.m_pRole = 0;
		}
		if(robj.m_pMulti) {
			m_pView->removeWidget(robj.m_pMulti);
			robj.m_pMulti = 0;
		}
		if(robj.m_pChangeWidget) {
			m_pView->removeWidget(robj.m_pChangeWidget);
			robj.m_pChangeWidget = 0;
		}
	}

	if(m_pName) {
		m_pView->removeWidget(m_pName);
		m_pName = 0;
	}

	if (m_pAssociation) {
		/*
		   We do not remove the UMLAssociation from the document.
		   Why? - Well, for example we might be in the middle of
		   a cut/paste. If the UMLAssociation is removed by the cut
		   then upon pasteing we have a problem.
		   This is not quite clean yet - there should be a way to
		   explicitly delete a UMLAssociation.  The Right Thing would
		   be to have a ListView representation for UMLAssociation.
`
			IF we are cut n pasting, why are we handling this association as a pointer?
			We should be using the XMI representation for a cut and paste. This
			allows us to be clean here, AND a choice of recreating the object 
			w/ same id IF its a "cut", or a new object if its a "copy" operation
			(in which case we wouldnt be here, in cleanup()).
		 */
		setUMLAssociation(0);
	}

	m_LinePath.cleanup();
}

void AssociationWidget::setUMLAssociation (UMLAssociation * assoc) 
{

	if(m_pAssociation) {
		m_pAssociation->disconnect(this);
		m_pAssociation->nrof_parent_widgets--;

		// we are the last "owner" of this association, so delete it
		// from the parent UMLDoc, and as a stand-alone
		//DISCUSS: Should we really do this?
		//    It implies that an association's existence is ONLY
		//    governed by its existence on at least one diagram.
		//    OTOH, it might be argued that an association should
		//    further exist even when it's (temporarily) not present
		//    on any diagram. This is exactly what cut and paste
		//    relies on (at least the way it's implemented now)
		if(m_pAssociation->nrof_parent_widgets == 0)
		{
			delete m_pAssociation;
		}
	
		m_pAssociation = 0;
	}

	if(assoc) {
		m_pAssociation = assoc;
		m_pAssociation->nrof_parent_widgets++;
		connect(m_pAssociation, SIGNAL(modified()), this,
                                        SLOT(mergeUMLRepresentationIntoAssociationData()));
	}

}


/** Returns true if the Widget is either at the starting or ending side of the association */
bool AssociationWidget::contains(UMLWidget* widget) {
	return (widget == m_role[A].m_pWidget || widget == m_role[B].m_pWidget);
}

Association_Type AssociationWidget::getAssocType() const {
	if (m_pAssociation)
		return m_pAssociation->getAssocType();
	return m_AssocType;
}

/** Sets the association's type */
void AssociationWidget::setAssocType(Association_Type type) {
	if (m_pAssociation)
		m_pAssociation->setAssocType(type);
	m_AssocType = type;
	m_LinePath.setAssocType(type);
	// If the association new type is not supposed to have Multiplicity
	// FloatingTexts and a Role FloatingText then set the internal
	// floating text pointers to null.
	if( !AssocRules::allowMultiplicity(type, getWidgetA()->getBaseType()) ) {
		if (m_role[A].m_pMulti) {
			delete m_role[A].m_pMulti;
			m_role[A].m_pMulti = NULL;
		}
		if (m_role[B].m_pMulti) {
			delete m_role[B].m_pMulti;
			m_role[B].m_pMulti = NULL;
		}
	}
	if( !AssocRules::allowRole( type ) ) {
		if (m_role[A].m_pRole) {
			delete m_role[A].m_pRole;
			m_role[A].m_pRole = NULL;
		}
		if (m_role[B].m_pRole) {
			delete m_role[B].m_pRole;
			m_role[B].m_pRole = NULL;
		}
		setRoleADoc("");
		setRoleBDoc("");
	}
}

int AssociationWidget::getWidgetID(Role_Type role) const {
	if (m_role[role].m_pWidget == NULL) {
		if (m_pAssociation) {
			int id;
			if (role == A)
				id = m_pAssociation->getRoleAId();
			else
				id = m_pAssociation->getRoleBId();
			return id;
		}
		kdError() << "AssociationWidget::getWidgetID(): m_pWidget is NULL" << endl;
		return -1;
	}
	if (m_role[role].m_pWidget->getBaseType() == Uml::wt_Object)
		return static_cast<ObjectWidget*>(m_role[role].m_pWidget)->getLocalID();
	int id = m_role[role].m_pWidget->getID();
	return id;
}

int AssociationWidget::getWidgetAID() const {
	int id = getWidgetID(A);
	if (id == -1 && m_pAssociation != NULL)      // just defense,
		id = m_pAssociation->getRoleAId();   // not expected to happen
	return id;
}

int AssociationWidget::getWidgetBID() const {
	int id = getWidgetID(B);
	if (id == -1 && m_pAssociation != NULL)      // just defense,
		id = m_pAssociation->getRoleBId();   // not expected to happen
	return id;
}

/** Returns a QString Object representing this AssociationWidget */
QString AssociationWidget::toString() {
	QString string = "";

	if(m_role[A].m_pWidget) {
		string = m_role[A].m_pWidget -> getName();
	}
	string.append(":");

	if(m_role[A].m_pRole) {
		string += m_role[A].m_pRole -> getText();
	}
	string.append(":");
	switch(getAssocType()) {
	case at_Generalization:
		string.append(i18n("Generalization"));
		break;

	case at_Aggregation:
		string.append(i18n("Aggregation"));
		break;

	case at_Dependency:
		string.append(i18n("Dependency"));
		break;

	case at_Association:
		string.append(i18n("Association"));
		break;

	case at_Anchor:
		string.append(i18n("Anchor"));
		break;

	case at_Realization:
		string.append( i18n("Realization") );
		break;

	case at_Composition:
		string.append( i18n("Composition") );
		break;

	case at_UniAssociation:
		string.append( i18n("Uni Association") );
		break;

	case at_Implementation:
		string.append( i18n("Implementation") );
		break;

	case at_State:
		string.append( i18n("State Transition") );
		break;

	default:
		string.append(i18n("Other Type"));
		break;
	}; //end switch
	string.append(":");
	if(m_role[B].m_pWidget) {
		string += m_role[B].m_pWidget -> getName();
	}

	if(m_role[B].m_pRole) {
		string += m_role[B].m_pRole -> getText();
	}

	return string;
}

void AssociationWidget::mouseDoubleClickEvent(QMouseEvent * me) {
	if(me -> button() != RightButton && me->button() != LeftButton)
		return;
	int i = 0;
	if( ( i = m_LinePath.onLinePath( me -> pos() ) ) == -1 )
	{
		m_LinePath.setSelected(false);
		return;
	}
	if(me->button() == LeftButton) {
		/* if there is no point around the mouse pointer, we insert a new one */
		if (! m_LinePath.isPoint(i, me -> pos(), POINT_DELTA )) {
			m_LinePath.insertPoint( i, me -> pos() );
		} else {
			/* deselect the line path */
			m_LinePath.setSelected( false );

			/* there was a point so we remove the point */
			m_LinePath.removePoint(i, me -> pos(), POINT_DELTA );

			/* select the line path */
			m_LinePath.setSelected( true );
		}

		m_LinePath.update();

		//calculateRoleTextSegment();
		calculateNameTextSegment();
	}
}

void AssociationWidget::moveEvent(QMoveEvent* me) {
	/*to be here a line segment has moved.
	  we need to see if the three text widgets needs to be moved.
	  there are a few things to check first though:

	  1) Do they exist
	  2) does it need to move:
	  2a) for the multi widgets only move if they changed region, otherwise they are close enough
	  2b) for role name move if the segment it is on moves.
	*/
	//first see if either the first or last segments moved, else no need to recalculate their point positions

	QPoint oldNamePoint = calculateTextPosition(tr_Name);
	QPoint oldMultiAPoint = calculateTextPosition(tr_MultiA);
	QPoint oldMultiBPoint = calculateTextPosition(tr_MultiB);
	QPoint oldChangeAPoint = calculateTextPosition(tr_ChangeA);
	QPoint oldChangeBPoint = calculateTextPosition(tr_ChangeB);
	QPoint oldRoleAPoint = calculateTextPosition(tr_RoleAName);
	QPoint oldRoleBPoint = calculateTextPosition(tr_RoleBName);

	m_LinePath.setPoint( m_nMovingPoint, me->pos() );
	int pos = m_LinePath.count() - 1;//set to last point for widget b

	if ( m_nMovingPoint == 1 || (m_nMovingPoint == pos-1) ) {
		calculateEndingPoints();
	}
	if (m_role[A].m_pChangeWidget && (m_nMovingPoint == 1)) {
		setTextPositionRelatively(tr_ChangeA, calculateTextPosition(tr_ChangeA), oldChangeAPoint);
	}
	if (m_role[B].m_pChangeWidget && (m_nMovingPoint == 1)) {
		setTextPositionRelatively(tr_ChangeB, calculateTextPosition(tr_ChangeB), oldChangeBPoint);
	}
	if (m_role[A].m_pMulti && (m_nMovingPoint == 1)) {
		setTextPositionRelatively(tr_MultiA, calculateTextPosition(tr_MultiA), oldMultiAPoint);
	}
	if (m_role[B].m_pMulti && (m_nMovingPoint == pos-1)) {
		setTextPositionRelatively(tr_MultiB, calculateTextPosition(tr_MultiB), oldMultiBPoint);
	}

	if (m_pName) {
		if(m_nMovingPoint == (int)m_unNameLineSegment ||
		   m_nMovingPoint - 1 == (int)m_unNameLineSegment) {
			setTextPositionRelatively(tr_Name, calculateTextPosition(tr_Name), oldNamePoint);
		}
	}

	if (m_role[A].m_pRole) {
		setTextPositionRelatively(tr_RoleAName, calculateTextPosition(tr_RoleAName), oldRoleAPoint);
	}
	if (m_role[B].m_pRole) {
		setTextPositionRelatively(tr_RoleBName, calculateTextPosition(tr_RoleBName), oldRoleBPoint);
	}
}


/** Calculates and sets the first and last point in the Association's LinePath
    Each point is a middle point of its respecting UMLWidget's Bounding rectangle
    or a corner of it
    This method picks which sides to use for the association */
void AssociationWidget::calculateEndingPoints() {
	/*
	 * For each UMLWidget the diagram is divided in four regions by its diagonals
	 * as indicated below
	 *                                         Region 2
	 *                                    \                /
	 *                                      \            /
	 *                                        +--------+
	 *                                        | \    / |
	 *                           Region 1     |   ><   |    Region 3
	 *                                        | /    \ |
	 *                                        +--------+
	 *                                      /            \
	 *                                    /                \
	 *                                         Region 4
	 *
	 * Each diagonal is defined by two corners of the bounding rectangle
	 *
	 * To calculate the first point in the LinePath we have to find out in which
	 * Region (defined by WidgetA's diagonals) is WidgetB's TopLeft corner
	 * (let's call it Region M.) After that the first point will be the middle
	 * point of the rectangle's side contained in Region M.
	 *
	 * To calculate the last point in the LinePath we repeat the above but
	 * in the opposite direction (from widgetB to WidgetA)
	 */


	if(!m_role[A].m_pWidget || !m_role[B].m_pWidget)
		return;
	int xA =  m_role[A].m_pWidget -> getX();
	int yA = m_role[A].m_pWidget -> getY();
	int xB = m_role[B].m_pWidget -> getX();
	int yB = m_role[B].m_pWidget -> getY();
	m_role[A].m_OldCorner.setX(xA);
	m_role[A].m_OldCorner.setY(yA);
	m_role[B].m_OldCorner.setX(xB);
	m_role[B].m_OldCorner.setY(yB);
	uint size = m_LinePath.count();
	uint pos = size - 1;
	if(size < 2)
		m_LinePath.setStartEndPoints( m_role[A].m_OldCorner, m_role[B].m_OldCorner );

	QRect rc(xA, yA, m_role[A].m_pWidget->width(), m_role[A].m_pWidget->height());
	//see if an association to self
	//see if it needs to be set up before we continue
	//if self association/message and doesn't have the minimum
	//4 points then create it.  Make sure no points are out of bounds of viewing area
	//this only happens on first time through that we are worried about.
	if( m_role[A].m_pWidget == m_role[B].m_pWidget  && size < 4 ) {
		const int DISTANCE = 50;
		int x = m_role[A].m_pWidget -> getX();
		int y = m_role[A].m_pWidget -> getY();
		int h = m_role[A].m_pWidget -> getHeight();
		int w = m_role[A].m_pWidget -> getWidth();
		//see if above widget ok to start
		if( y - DISTANCE > 0 ) {
			m_LinePath.setStartEndPoints( QPoint( x + w / 4, y ) , QPoint( x + w * 3 / 4, y ) );
			m_LinePath.insertPoint( 1, QPoint( x + w / 4, y - DISTANCE ) );
			m_LinePath.insertPoint( 2 ,QPoint( x + w * 3 / 4, y - DISTANCE ) );
			m_role[A].m_WidgetRegion = m_role[B].m_WidgetRegion = North;
		} else {
			m_LinePath.setStartEndPoints( QPoint( x + w / 4, y + h ), QPoint( x + w * 3 / 4, y + h ) );
			m_LinePath.insertPoint( 1, QPoint( x + w / 4, y + h + DISTANCE ) );
			m_LinePath.insertPoint( 2, QPoint( x + w * 3 / 4, y + h + DISTANCE ) );
			m_role[A].m_WidgetRegion = m_role[B].m_WidgetRegion = South;
		}
		return;
	}//end a == b
	//if the line has more than one segment change the values to calculate
	//from widget to point 1
	size = m_LinePath.count();
	if( size > 2 ) {
		QPoint p = m_LinePath.getPoint( 1 );
		xB = p.x();
		yB = p.y();
	}
	//find widgetA region
	Region oldRegionA = m_role[A].m_WidgetRegion;
	m_role[A].m_WidgetRegion = findPointRegion( rc, xB, yB );
	doUpdates( m_role[A].m_WidgetRegion, oldRegionA, m_role[A].m_nIndex, m_role[A].m_nTotalCount, true);

	//now do the same for widgetB
	//if the line has more than one segment change the values to calculate
	//from widgetB to point the last point away from it
	xA =  m_role[A].m_pWidget -> getX();
	yA = m_role[A].m_pWidget -> getY();
	xB = m_role[B].m_pWidget -> getX();
	yB = m_role[B].m_pWidget -> getY();
	if (size > 2 ) {
		QPoint p = m_LinePath.getPoint( pos - 1 );
		xA = p.x();
		yA = p.y();
	}
	rc.setX( xB );
	rc.setY( yB );
	rc.setWidth( m_role[B].m_pWidget->width() );
	rc.setHeight( m_role[B].m_pWidget->height() );

	Region oldRegionB = m_role[B].m_WidgetRegion;
	m_role[B].m_WidgetRegion = findPointRegion( rc, xA, yA );
	doUpdates( m_role[B].m_WidgetRegion, oldRegionB, m_role[B].m_nIndex, m_role[B].m_nTotalCount, false);
}

void AssociationWidget::doUpdates(Region& region, Region oldRegion,
				  int index, int totalCount,
				  bool isWidgetA) {
	//move some regions to the standard ones
	switch( region ) {
	case NorthWest:
		region = North;
		break;
	case NorthEast:
		region = East;
		break;
	case SouthEast:
		region = South;
		break;
	case SouthWest:
	case Center:
		region = West;
		break;
	default:
		break;
	}
	int regionCount = getRegionCount( region, isWidgetA ) + 2;//+2 = (1 for this one and one to halve it)
	if( oldRegion != region ) {
		int oldCount = totalCount;
		updateRegionLineCount( regionCount - 1, regionCount, region, isWidgetA );
		updateAssociations( oldCount - 1, oldRegion, isWidgetA );
		updateAssociations( regionCount, region, isWidgetA );
	} else if( totalCount != regionCount ) {
		updateRegionLineCount( regionCount - 1, regionCount, region, isWidgetA );
		updateAssociations( regionCount, region, isWidgetA );
	} else {
		updateRegionLineCount( index, totalCount, region, isWidgetA );
		updateAssociations( totalCount, region, isWidgetA );
	}
}

/** Read property of bool m_bActivated. */
const bool AssociationWidget::isActivated() {
	return m_bActivated;
}

/** Set the m_bActivated flag of a widget but does not perform the Activate method */
void AssociationWidget::setActivated(bool active /*=true*/) {
	m_bActivated = active;
}

// ugly. but its what we are forced into by having the ugly association
// widget data NOT be stored in the UMLAssociation. *sigh*.
//   Yes, but there's a reason for this separation:
//   A single UMLAssociation can be represented in different
//   diagrams. Each graphical representation requires its own
//   AssociationWidget.
//   Besides, it's a good idea to separate the document objects
//   from their graphical representations. That's the way all
//   the rest of umbrello is structured.  --okellogg
void AssociationWidget::mergeUMLRepresentationIntoAssociationData()
{
	UMLAssociation *uml = getAssociation();

	// block signals until finished
	uml->blockSignals(true);

        // floating text widgets
        FloatingText *text = getNameWidget();
        if (text)
                text->setText(uml->getName());

        text = getRoleAWidget();
        if (text)
        {
                text->setText(uml->getRoleNameA());
                // it doesnt make sense to have visibility wi/o Rolename
                // so we only set it when its in here. Probably should have
                // error condition thrown when visb is set but rolename isnt.
                setVisibilityA(uml->getVisibilityA());
        }

        text = getRoleBWidget();
        if (text)
        {
                text->setText(uml->getRoleNameB());
                setVisibilityB(uml->getVisibilityB());
        }

        text = getMultiAWidget();
        if (text)
                text->setText(uml->getMultiA());

        text = getMultiBWidget();
        if (text)
                text->setText(uml->getMultiB());

	uml->blockSignals(false);

}

// this will synchronize UMLAssociation w/ this new Widget
void AssociationWidget::mergeAssociationDataIntoUMLRepresentation()
{

	UMLAssociation *uml = m_pAssociation;
	if (uml == NULL)
		return;

	// block emit modified signal, or we get a horrible loop
	uml->blockSignals(true);

	// would be desirable to do the following
	// so that we can be sure its back to initial state
	// in case we missed something here.
	//uml->init();

	// floating text widgets
	FloatingText *text = getNameWidget();
	if (text)
		uml->setName(text->getText());

	text = getRoleAWidget();
	if (text)
		uml->setRoleNameA(text->getText());

	text = getRoleBWidget();
	if (text)
		uml->setRoleNameB(text->getText());

	text = getMultiAWidget();
	if (text)
		uml->setMultiA(text->getText());

	text = getMultiBWidget();
	if (text)
		uml->setMultiB(text->getText());

	// unblock
	uml->blockSignals(false);
}

/** Adjusts the ending point of the association that connects to Widget */
void AssociationWidget::widgetMoved(UMLWidget* widget, int x, int y ) {
	QPoint oldNamePoint = calculateTextPosition(tr_Name);
	QPoint oldMultiAPoint = calculateTextPosition(tr_MultiA);
	QPoint oldMultiBPoint = calculateTextPosition(tr_MultiB);
	QPoint oldChangeAPoint = calculateTextPosition(tr_ChangeA);
	QPoint oldChangeBPoint = calculateTextPosition(tr_ChangeB);
	QPoint oldRoleAPoint = calculateTextPosition(tr_RoleAName);
	QPoint oldRoleBPoint = calculateTextPosition(tr_RoleBName);

	int dx = m_role[A].m_OldCorner.x() - x;
	int dy = m_role[A].m_OldCorner.y() - y;
	uint size = m_LinePath.count();
	uint pos = size - 1;
	calculateEndingPoints();

	if( m_role[A].m_pWidget == m_role[B].m_pWidget ) {
		for( int i=1 ; i < (int)pos ; i++ ) {
			QPoint p = m_LinePath.getPoint( i );
			int newX = p.x() - dx;
			int newY = p.y() - dy;
			// safety. We DONT want to go off the screen
			if(newX < 0)
				newX = 0;
			// safety. We DONT want to go off the screen
			if(newY < 0)
				newY = 0;
			newX = m_pView -> snappedX( newX );
			newY = m_pView -> snappedY( newY );
			p.setX( newX );
			p.setY( newY );
			m_LinePath.setPoint( i, p );
		}

		if ( m_pName && !m_pName->getSelected() ) {
			setTextPositionRelatively(tr_Name, calculateTextPosition(tr_Name), oldNamePoint);
		}

	}//end if widgetA = widgetB
	else if (m_role[A].m_pWidget==widget) {
		if (m_pName && (m_unNameLineSegment == 0) && !m_role[A].m_pWidget->getSelected() ) {
			//only calculate position and move text if the segment it is on is moving
			setTextPositionRelatively(tr_Name, calculateTextPosition(tr_Name),
						  oldNamePoint);
		}
	}//end if widgetA moved
	else if (m_role[B].m_pWidget==widget) {
		if (m_pName && (m_unNameLineSegment == pos-1) && !m_pName->getSelected() ) {
			//only calculate position and move text if the segment it is on is moving
			setTextPositionRelatively(tr_Name, calculateTextPosition(tr_Name),
						  oldNamePoint);
		}
	}//end if widgetB moved

	if ( m_role[A].m_pRole && !m_role[A].m_pRole->getSelected() ) {
		setTextPositionRelatively(tr_RoleAName, calculateTextPosition(tr_RoleAName), oldRoleAPoint);
	}
	if ( m_role[B].m_pRole && !m_role[B].m_pRole->getSelected() ) {
		setTextPositionRelatively(tr_RoleBName, calculateTextPosition(tr_RoleBName), oldRoleBPoint);
	}
	if ( m_role[A].m_pMulti && !m_role[A].m_pMulti->getSelected() ) {
		setTextPositionRelatively(tr_MultiA, calculateTextPosition(tr_MultiA), oldMultiAPoint);
	}
	if ( m_role[B].m_pMulti && !m_role[B].m_pMulti->getSelected() ) {
		setTextPositionRelatively(tr_MultiB, calculateTextPosition(tr_MultiB), oldMultiBPoint);
	}
	if ( m_role[A].m_pChangeWidget && !m_role[A].m_pChangeWidget->getSelected() ) {
		setTextPositionRelatively(tr_ChangeA, calculateTextPosition(tr_ChangeA), oldChangeAPoint);
	}
	if ( m_role[B].m_pChangeWidget && !m_role[B].m_pChangeWidget->getSelected() ) {
		setTextPositionRelatively(tr_ChangeB, calculateTextPosition(tr_ChangeB), oldChangeBPoint);
	}
}//end method widgetMoved

/** Finds out in which region of rectangle Rect contains the Point (PosX, PosY) and returns the region
    number:
    1 = Region 1
    2 = Region 2
    3 = Region 3
    4 = Region 4
    5 = On diagonal 2 between Region 1 and 2
    6 = On diagonal 1 between Region 2 and 3
    7 = On diagonal 2 between Region 3 and 4
    8	= On diagonal 1 between Region4 and 1
    9 = On diagonal 1 and On diagonal 2 (the center)
*/
AssociationWidget::Region AssociationWidget::findPointRegion(QRect Rect, int PosX, int PosY) {
	float w = (float)Rect.width();
	float h = (float)Rect.height();
	float x =  (float)Rect.x();
	float y = (float)Rect.y();
	float Slope2 = w / h;
	float Slope1 = Slope2*(float)(-1);
	float b1 = x + w - ( Slope1* y );
	float b2 = x - ( Slope2* y );

	float eval1 = Slope1 * (float)PosY + b1;
	float eval2 = Slope2  *(float)PosY + b2;

	Region result = Error;
	//if inside region 1
	if(eval1 > PosX && eval2 > PosX) {
		result = West;
	}
	//if inside region 2
	else if (eval1 > PosX && eval2 < PosX) {
		result = North;
	}
	//if inside region 3
	else if (eval1 < PosX && eval2 < PosX) {
		result = East;
	}
	//if inside region 4
	else if (eval1 < PosX && eval2 > PosX) {
		result = South;
	}
	//if inside region 5
	else if (eval1 == PosX && eval2 < PosX) {
		result = NorthWest;
	}
	//if inside region 6
	else if (eval1 < PosX && eval2 == PosX) {
		result = NorthEast;
	}
	//if inside region 7
	else if (eval1 == PosX && eval2 > PosX) {
		result = SouthEast;
	}
	//if inside region 8
	else if (eval1 > PosX && eval2 == PosX) {
		result = SouthWest;
	}
	//if inside region 9
	else if (eval1 == PosX && eval2 == PosX) {
		result = Center;
	}

	return result;
}

QPoint AssociationWidget::findRectIntersectionPoint(UMLWidget* pWidget, QPoint P1, QPoint P2) {
	QPoint result(-1, -1);

	int old_region = -1;
	if(!pWidget) {
		return result;
	}
	int X = -1, Y = -1;
	if(pWidget == m_role[A].m_pWidget)
	{
		X = m_role[B].m_pWidget -> getX();
		Y = m_role[B].m_pWidget -> getY();
		old_region = m_role[A].m_nCornerRegion;
	} else {
		X = m_role[A].m_pWidget -> getX();
		Y = m_role[A].m_pWidget -> getY();
		old_region = m_role[B].m_nCornerRegion;
	}
	QRect rc(pWidget -> getX(), pWidget -> getY(), pWidget->getWidth(), pWidget->getHeight());
	int region = findPointRegion(rc, X, Y);
	if(region == old_region) {
		return QPoint(-1, -1);
	}
	switch(region)
	{
	case West:
		result = findIntersection(rc.topLeft(), rc.bottomLeft(), P1, P2);
		break;
	case North:
		result = findIntersection(rc.topLeft(), rc.topRight(), P1, P2);
		break;
	case East:
		result = findIntersection(rc.topRight(), rc.bottomRight(), P1, P2);
		break;
	case South:
		result = findIntersection(rc.bottomLeft(), rc.bottomRight(), P1, P2);
		break;

	case NorthWest:
		result = rc.topLeft();
		break;
	case NorthEast:
		result  = rc.topRight();
		break;
	case SouthEast:
		result = rc.bottomRight();
		break;

	case SouthWest:
	case Center:
		result = rc.bottomLeft();
		break;
	}
	return result;
}

QPoint AssociationWidget::swapXY(QPoint p) {
	QPoint swapped( p.y(), p.x() );
	return swapped;
}

/** Returns the intersection point between lines P1P2 and P3P4, if the intersection
    point is not contained in the segment P1P2 then it returns (-1, -1)*/
QPoint AssociationWidget::findIntersection(QPoint P1, QPoint P2, QPoint P3, QPoint P4) {
	/*
	 * For the function's internal calculations remember:
	 * QT coordinates start with the point (0,0) as the topleft corner and x-values
	 * increase from left to right and y-values increase from top to bottom; it means
	 * the visible area is quadrant I in the regular XY coordinate system
	 *
	 *                     |
	 *      Quadrant II    |   Quadrant I
	 *    -----------------|-----------------
	 *      Quadrant III   |   Quadrant IV
	 *                     |
	 * In order for the linear function calculations to work in this method we must switch x and y values
	 * (x values become y values and viceversa)
	 */
	int x1 = P1.y();
	int y1 = P1.x();
	int x2 = P2.y();
	int y2 = P2.x();
	int x3 = P3.y();
	int y3 = P3.x();
	int x4 = P4.y();
	int y4 = P4.x();

	//Line 1 is the line between (x1,y1) and (x2,y2)
	//Line 2 is the line between (x3,y3) and (x4, y4)
	bool no_line1 = true; //it is false if Line 1 is a linear function
	bool no_line2 = true; //it is false if Line 2 is a linear function
	float slope1 = 0.0, slope2 = 0.0, b1 = 0.0, b2 = 0.0;
	if(x2 != x1) {
		slope1 = (float)(y2 - y1) / (float)(x2 - x1);
		b1 = (float)y1 - (slope1 * (float)x1);
		no_line1 = false;
	}
	if(x4 != x3) {
		slope2 = (float)(y4 - y3) / (float)(x4 - x3);
		b2 = (float)y3 - (slope2 * (float)x3);
		no_line2 = false;
	}
	QPoint pt;
	//if either line is not a function
	if(no_line2 && no_line1) {
		//if the lines are not the same one
		if(x1 != x3) {
			return QPoint(-1, -1);
		}
		//if the lines are the same one
		if(y3 <= y4) {
			if( y3 <= y1 && y1 <= y4) {
				return QPoint(y1, x1);
			} else {
				return QPoint(y2, x2);
			}
		} else {
			if( y4 <= y1 && y1 <= y3) {
				return QPoint(y1, x1);
			} else {
				return QPoint(y2, x2);
			}
		}
	} else if( no_line1) {
		pt.setX(x1);
		pt.setY((int)((slope2 * (float)x1) + b2));
		if(y1 >= y2) {
			if( !(y2 <= pt.y() && pt.y() <= y1)) {
				pt.setX(-1);
				pt.setY(-1);
			}
		}
		else {
			if( !(y1 <= pt.y() && pt.y() <= y2)) {
				pt.setX(-1);
				pt.setY(-1);
			}
		}
		return swapXY(pt);
	} else if( no_line2) {
		pt.setX(x3);
		pt.setY((int)((slope1 * (float)x3) + b1));
		if(y3 >= y4) {
			if( !(y4 <= pt.y() && pt.y() <= y3)) {
				pt.setX(-1);
				pt.setY(-1);
			}
		} else {
			if( !(y3 <= pt.y() && pt.y() <= y4)) {
				pt.setX(-1);
				pt.setY(-1);
			}
		}
		return swapXY(pt);
	}
	pt.setX((int)((b2 - b1) / (slope1 - slope2)));
	pt.setY((int)((slope1 * (float)pt.x()) + b1));
	//the intersection point must be inside the segment (x1, y1) (x2, y2)
	if(x2 >= x1&& y2 >= y1) {
		if(! ((x1 <= pt.x() && pt.x() <= x2)	&& (y1 <= pt.y() && pt.y() <= y2)) ) {
			pt.setX(-1);
			pt.setY(-1);
		}
	} else if (x2 < x1 && y2 >= y1) {
		if(! ((x2 <= pt.x() && pt.x() <= x1)	&& (y1 <= pt.y() && pt.y() <= y2)) ) {
			pt.setX(-1);
			pt.setY(-1);
		}
	} else if (x2 >= x1 && y2 < y1) {
		if(! ((x1 <= pt.x() && pt.x() <= x2)	&& (y2 <= pt.y() && pt.y() <= y1)) ) {
			pt.setX(-1);
			pt.setY(-1);
		}
	} else {
		if(! ((x2 <= pt.x() && pt.x() <= x1)	&& (y2 <= pt.y() && pt.y() <= y1)) ) {
			pt.setX(-1);
			pt.setY(-1);
		}
	}

	return swapXY(pt);  // Swap X and Y in target to go back to Qt coord.sys.
}

/* Returns the total length of the association's LinePath:
   result = segment_1_length + segment_2_length + ..... + segment_n_length
 */
float AssociationWidget::totalLength() {
	uint size = m_LinePath.count();
	float total_length = 0;

	for(uint i = 0; i < size - 1; i++) {
		QPoint pi = m_LinePath.getPoint( i );
		QPoint pj = m_LinePath.getPoint( i+1 );
		int xi = pi.y();
		int xj = pj.y();
		int yi = pi.x();
		int yj = pj.x();
		total_length +=  sqrt( double(((xj - xi)*(xj - xi)) + ((yj - yi)*(yj - yi))) );
	}

	return total_length;
}


/** Calculates which point of segment P1P2 has a distance equal to Distance from P1,
    Lets say such point is P3,  the distance from P1 to P3 must be equal to Distance
    and if P3 is not a point of the segment P1P2 then the function returns (-1,-1)
*/
QPoint AssociationWidget::calculatePointAtDistance(QPoint P1, QPoint P2, float Distance) {
	/*
	  the distance D between points (x1, y1) and (x3, y3) has the following formula:
	      ---     ------------------------------
	  D =    \   /           2             2
	          \ /   (x3 - x1)  +  (y3 - y1)

	  D, x1 and y1 are known and the point (x3, y3) is inside line (x1,y1)(x2,y2), so if the
	  that line has the formula y = mx + b
	  then y3 = m*x3 + b

	   2             2             2
	  D   = (x3 - x1)  +  (y3 - y1)

	   2       2                 2      2                 2
	  D    = x3    - 2*x3*x1 + x1   + y3   - 2*y3*y1  + y1

	   2       2       2       2                  2
	  D    - x1    - y1    = x3    - 2*x3*x1  + y3   - 2*y3*y1

	   2       2       2       2                          2
	  D    - x1    - y1    = x3    - 2*x3*x1  + (m*x3 + b)  - 2*(m*x3 + b)*y1

	   2       2       2              2       2        2
	  D    - x1    - y1   + 2*b*y1 - b   =  (m  + 1)*x3   + (-2*x1 + 2*m*b -2*m*y1)*x3

	   2      2       2       2
	  C  = - D    + x1    + y1   - 2*b*y1 + b


	   2
	  A  = (m    + 1)

	  B  = (-2*x1 + 2*m*b -2*m*y1)

	  and we have
	   2
	  A * x3 + B * x3 - C = 0

	                         ---------------
	             -B +  ---  /  2
	                      \/  B   - 4*A*C
	  sol_1  = --------------------------------
	                       2*A


	                         ---------------
	             -B -  ---  /  2
	                      \/  B   - 4*A*C
	  sol_2  = --------------------------------
	                       2*A


	  then in the distance formula we have only one variable x3 and that is easy
	  to calculate
	*/
	int x1 = P1.y();
	int y1 = P1.x();
	int x2 = P2.y();
	int y2 = P2.x();

	if(x2 == x1) {
		return QPoint(x1, y1 + (int)Distance);
	}
	float slope = ((float)y2 - (float)y1) / ((float)x2 - (float)x1);
	float b = (y1 - slope*x1);
	float A = (slope * slope) + 1;
	float B = (2*slope*b) - (2*x1)  - (2*slope*y1);
	float C = (b*b) - (Distance*Distance) + (x1*x1) + (y1*y1) - (2*b*y1);
	float t = B*B - 4*A*C;

	if(t < 0) {
		return QPoint(-1, -1);
	}
	float sol_1 = ((-1* B) + sqrt(t) ) / (2*A);
	float sol_2 = ((-1*B) - sqrt(t) ) / (2*A);

	if(sol_1 < 0.0 && sol_2 < 0.0) {
		return QPoint(-1, -1);
	}
	QPoint sol1Point((int)(slope*sol_1 + b), (int)(sol_1));
	QPoint sol2Point((int)(slope*sol_2 + b), (int)(sol_2));
	if(sol_1 < 0 && sol_2 >=0) {
		if(x2 > x1) {
			if(x1 <= sol_2 && sol_2 <= x2)
				return sol2Point;
		} else {
			if(x2 <= sol_2 && sol_2 <= x1)
				return sol2Point;
		}
	} else if(sol_1 >= 0 && sol_2 < 0) {
		if(x2 > x1) {
			if(x1 <= sol_1 && sol_1 <= x2)
				return sol1Point;
		} else {
			if(x2 <= sol_1 && sol_1 <= x1)
				return sol1Point;
		}
	} else {
		if(x2 > x1) {
			if(x1 <= sol_1 && sol_1 <= x2)
				return sol1Point;
			if(x1 <= sol_2 && sol_2 <= x2)
				return sol2Point;
		} else {
			if(x2 <= sol_1 && sol_1 <= x1)
				return sol1Point;
			if(x2 <= sol_2 && sol_2 <= x1)
				return sol2Point;
		}
	}
	return QPoint(-1, -1);
}

/** Calculates which point of a perpendicular line to segment P1P2 that contains P2
    has a distance equal to Distance from P2,
    Lets say such point is P3,  the distance from P2 to P3 must be equal to Distance
*/
QPoint AssociationWidget::calculatePointAtDistanceOnPerpendicular(QPoint P1, QPoint P2, float Distance) {
	/*
	  the distance D between points (x2, y2) and (x3, y3) has the following formula:

	       ---     ------------------------------
	  D =     \   /           2             2
	           \ /   (x3 - x2)  +  (y3 - y2)

	  D, x2 and y2 are known and line P2P3 is perpendicular to line (x1,y1)(x2,y2), so if the
	  line P1P2 has the formula y = m*x + b,
	  then          (x1 - x2)
	          m =  -----------    , because it is perpendicular to line P1P2
	                (y2 - y1)

	  also y2 = m*x2 + b
	  => b = y2 - m*x2

	  then P3 = (x3, m*x3 + b)

	   2             2            2
	  D  = (x3 - x2)  + (y3 - y2)

	   2     2                 2      2                 2
	  D  = x3    - 2*x3*x2 + x2   + y3   - 2*y3*y2  + y2

	   2       2       2       2                  2
	  D    - x2    - y2    = x3    - 2*x3*x2  + y3   - 2*y3*y2



	   2       2       2       2                          2
	  D    - x2    - y2    = x3    - 2*x3*x2  + (m*x3 + b)  - 2*(m*x3 + b)*y2

	   2       2       2                2       2        2
	  D    - x2    - y2     + 2*b*y2 - b   =  (m  + 1)*x3     + (-2*x2 + 2*m*b -2*m*y2)*x3

	   2      2       2       2
	  C  = - D    + x2    + y2   - 2*b*y2 + b

	   2
	  A  = (m    + 1)

	  B  = (-2*x2 + 2*m*b -2*m*y2)

	  and we have
	   2
	  A * x3 + B * x3 - C = 0


	                         ---------------
	             -B +  ---  /  2
	                      \/  B   - 4*A*C
	  sol_1  = --------------------------------
	                       2*A


	                         ---------------
	  sol_2  =   -B -  ---  /  2
	                      \/  B   - 4*A*C
	           --------------------------------
	                       2*A

	  then in the distance formula we have only one variable x3 and that is easy
	  to calculate
	*/
	int x1 = P1.y();
	int y1 = P1.x();
	int x2 = P2.y();
	int y2 = P2.x();

	if(x2 == x1) {
		return QPoint((int)(x2+ Distance), y2);
	}
	float slope = ((float)x1 - (float)x2) / ((float)y2 - (float)y1);
	float b = (y2 - slope*x2);
	float A = (slope * slope) + 1;
	float B = (2*slope*b) - (2*x2)  - (2*slope*y2);
	float C = (b*b) - (Distance*Distance) + (x2*x2) + (y2*y2) - (2*b*y2);
	float t = B*B - 4*A*C;
	if(t < 0) {
		return QPoint(-1, -1);
	}
	float sol_1 = ((-1* B) + sqrt(t) ) / (2*A);

	float sol_2 = ((-1*B) - sqrt(t) ) / (2*A);

	if(sol_1 < 0 && sol_2 < 0) {
		return QPoint(-1, -1);
	}
	QPoint sol1Point((int)(slope*sol_1 + b), (int)sol_1);
	QPoint sol2Point((int)(slope*sol_2 + b), (int)sol_2);
	if(sol_1 < 0 && sol_2 >=0) {
		return sol2Point;
	} else if(sol_1 >= 0 && sol_2 < 0) {
		return sol1Point;
	} else {	// Choose one solution , either will work fine
		if(slope >= 0) {
			if(sol_1 <= sol_2)
				return sol2Point;
			else
				return sol1Point;
		} else {
			if(sol_1 <= sol_2)
				return sol1Point;
			else
				return sol2Point;
		}

	}
	return QPoint(-1, -1);  // never reached, just keep compilers happy
}

/** Calculates the intersection (PS) between line P1P2 and a perpendicular line containing
    P3, the result is returned in ResultingPoint. and result value represents the distance
    between ResultingPoint and P3; if this value is negative an error ocurred. */
float AssociationWidget::perpendicularProjection(QPoint P1, QPoint P2, QPoint P3,
						 QPoint& ResultingPoint) {
	//line P1P2 is Line 1 = y=slope1*x + b1

	//line P3PS is Line 1 = y=slope2*x + b2

	float slope2 = 0;
	float slope1 = 0;
	float sx = 0, sy = 0;
	int y2 = P2.x();
	int y1 = P1.x();
	int x2 = P2.y();
	int x1 = P1.y();
	int y3 = P3.x();
	int x3 = P3.y();
	float distance = 0;
	float b1 = 0;

	float b2 = 0;

	if(x2 == x1) {
		sx = x2;
		sy = y3;
	} else if(y2 == y1) {
		sy = y2;
		sx = x3;
	} else {
		slope1 = (y2 - y1)/ (x2 - x1);
		slope2 = (x1 - x2)/ (y2 - y1);
		b1 = y2 - (slope1 * x2);
		b2 = y3 - (slope2 * x3);
		sx = (b2 - b1) / (slope1 - slope2);
		sy = slope1*sx + b1;
	}
	distance = (int)( sqrt( ((x3 - sx)*(x3 - sx)) + ((y3 - sy)*(y3 - sy)) ) );

	ResultingPoint.setX( (int)sy );
	ResultingPoint.setY( (int)sx );

	return distance;
}

QPoint AssociationWidget::calculateTextPosition(Text_Role role) {
	QPoint p( -1, -1 ), q( -1, -1 );
	uint size = m_LinePath.count();
	uint pos = size - 1;
	int x = 0, y = 0;
	int textW = 0, textH = 0;
	int slope = 0, divisor = 1;
	const int SPACE = 2;
	FloatingText const * text = 0;

	if(role == tr_MultiA) {
		text = getMultiAWidget();
		if( text ) {
			textW = text -> width();
			textH = text -> height();
		}
		p = m_LinePath.getPoint( 0 );
		q = m_LinePath.getPoint( 1 );
		divisor = (p.x()-q.x());
		if (divisor != 0)
			slope = (p.y()-q.y())/divisor;
		else
			slope = 10000;


		if( p.y() > q.y() )
			if(slope == 0)
				y = p.y() + SPACE;
			else
				y = p.y() - SPACE - textH;
		else
			if(slope == 0)
				y = p.y() - SPACE - textH;
			else
				y = p.y() + SPACE;

		if( p.x() < q.x() )
			if(slope == 0)
				x = p.x() + SPACE;
			else
				x = p.x() - SPACE - textW;
		else
			if(slope == 0)
				x = p.x() - SPACE - textW;
			else
				x = p.x() + SPACE;

	} else if(role == tr_MultiB) {
		text = getMultiBWidget();
		if( text ) {
			textW = text -> width();
			textH = text -> height();
		}
		p = m_LinePath.getPoint( pos );
		q = m_LinePath.getPoint( pos - 1 );
		divisor = (p.x()-q.x());
		if (divisor != 0)
			slope = (p.y()-q.y())/divisor;
		else
			slope = 10000000;

		if( p.y() > q.y() )
		{
			if(slope == 0)
				y = p.y() - SPACE;
			else
				y = p.y() - textH - SPACE;
		} else
			if(slope == 0)
				y = p.y() - textH - SPACE;
			else
				y = p.y() + SPACE;


		if( p.x() < q.x() )
			if(slope == 0)
				x = p.x() + SPACE;
			else
				x = p.x() - textW + SPACE;
		else
			if(slope == 0)
				x = p.x() - textW + SPACE;
			else
				x = p.x() + SPACE;

	} else if(role == tr_Name) {

		x = (int)( ( m_LinePath.getPoint(m_unNameLineSegment).x() +
			     m_LinePath.getPoint(m_unNameLineSegment + 1).x() ) / 2 );

		y = (int)( ( m_LinePath.getPoint(m_unNameLineSegment).y() +
			     m_LinePath.getPoint(m_unNameLineSegment + 1).y() ) / 2 );

	} else if(role == tr_ChangeA) {

		text = getChangeWidgetA();

		if( text ) {
			textW = text -> width();
			textH = text -> height();
		}
		p = m_LinePath.getPoint( 0 );
		q = m_LinePath.getPoint( 1 );

		if( p.y() > q.y() )
			y = p.y() - SPACE - (textH *2);
		else
			y = p.y() + SPACE + textH;

		if( p.x() < q.x() )
			x = p.x() + SPACE;
		else
			x = p.x() - SPACE - textW;

	} else if(role == tr_ChangeB) {

		text = getChangeWidgetB();
		if( text ) {
			textW = text -> width();
			textH = text -> height();
		}

		p = m_LinePath.getPoint( pos );
		q = m_LinePath.getPoint( pos - 1 );

		if( p.y() > q.y() )
			y = p.y() - (textH*2) - SPACE;
		else
			y = p.y() + textH + SPACE;

		if( p.x() < q.x() )
			x = p.x() + SPACE;
		else
			x = p.x() - textW - SPACE;

	} else if(role == tr_RoleAName) {

		text = getRoleAWidget();
		if( text ) {
			textW = text -> width();
			textH = text -> height();
		}
		p = m_LinePath.getPoint( 0 );
		q = m_LinePath.getPoint( 1 );

		if( p.y() > q.y() )
			y = p.y() - SPACE - textH;
		else
			y = p.y() + SPACE;

		if( p.x() < q.x() )
			x = p.x() + SPACE;
		else
			x = p.x() - SPACE - textW;
	}
	else if(role == tr_RoleBName)
	{
		text = getRoleBWidget();
		if( text ) {
			textW = text -> width();
			textH = text -> height();
		}

		p = m_LinePath.getPoint( pos );
		q = m_LinePath.getPoint( pos - 1 );
		if( p.y() > q.y() )
			y = p.y() - textH - SPACE;
		else
			y = p.y() + SPACE;

		if( p.x() < q.x() )
			x = p.x() + SPACE;
		else
			x = p.x() - textW - SPACE;
	}
	p = QPoint( x, y );
	return p;
}

void AssociationWidget::calculateNameTextSegment() {
	if(!m_pName) {
		return;
	}
	//changed to use the middle of the text
	//i think this will give a better result.
	//never know what sort of lines people come up with
	//and text could be long to give a false reading
	int xt = m_pName -> getX();
	int yt = m_pName -> getY();
	xt += m_pName -> getWidth() / 2;
	yt += m_pName -> getHeight() / 2;
	uint size = m_LinePath.count();
	int xi = 0, xj = 0, yi = 0, yj = 0;
	//sum of length(PTP1) and length(PTP2)
	float total_length = 0;
	float smallest_length = 0;
	for(uint i = 0; i < size - 1; i++) {
		QPoint pi = m_LinePath.getPoint( i );
		QPoint pj = m_LinePath.getPoint( i+1 );
		xi = pi.x();
		xj = pj.x();
		yi = pi.y();
		yj = pj.y();
		total_length =  sqrt( pow( double(xt - xi), 2.0 ) + pow( double(yt - yi), 2. ) ) + sqrt( pow( double(xt - xj), 2.0) + pow( double(yt - yj), 2.0) );
		//this gives the closest point
		if( total_length < smallest_length || i == 0) {
			smallest_length = total_length;
			m_unNameLineSegment = i;
		}
	}
}

void AssociationWidget::setTextPosition(Text_Role role, QPoint pos) {
	bool startMove = false;
	if( m_role[A].m_pMulti && m_role[A].m_pMulti->getStartMove() )
		startMove = true;
	else if( m_role[B].m_pMulti && m_role[B].m_pMulti->getStartMove() )
		startMove = true;
	else if( m_role[A].m_pChangeWidget && m_role[A].m_pChangeWidget->getStartMove() )
		startMove = true;
	else if( m_role[B].m_pChangeWidget && m_role[B].m_pChangeWidget->getStartMove() )
		startMove = true;
	else if( m_role[A].m_pRole  && m_role[A].m_pRole->getStartMove() )
		startMove = true;
	else if( m_role[B].m_pRole  && m_role[B].m_pRole->getStartMove() )
		startMove = true;
	else if( m_pName && m_pName->getStartMove() )
		startMove = true;

	if (startMove) {
		return;
	}

	switch(role) {
		case tr_MultiA:
			m_role[A].m_pMulti->setLinePos( pos.x(), pos.y() );
			break;
		case tr_MultiB:
			m_role[B].m_pMulti->setLinePos( pos.x(), pos.y() );
			break;
		case tr_Name:
		case tr_Coll_Message:
			m_pName->setLinePos( pos.x(), pos.y() );
			break;
		case tr_RoleAName:
			m_role[A].m_pRole->setLinePos( pos.x(), pos.y() );
			break;
		case tr_RoleBName:
			m_role[B].m_pRole->setLinePos( pos.x(), pos.y() );
			break;
		case tr_ChangeA:
			m_role[A].m_pChangeWidget->setLinePos( pos.x(), pos.y() );
			break;
		case tr_ChangeB:
			m_role[B].m_pChangeWidget->setLinePos( pos.x(), pos.y() );
			break;
		default:
			break;
	}
}

void AssociationWidget::setTextPositionRelatively(Text_Role role, QPoint pos, QPoint oldPosition) {
	bool startMove = false;
	if( m_role[A].m_pMulti && m_role[A].m_pMulti->getStartMove() )
		startMove = true;
	else if( m_role[B].m_pMulti && m_role[B].m_pMulti->getStartMove() )
		startMove = true;
	else if( m_role[A].m_pChangeWidget && m_role[A].m_pChangeWidget->getStartMove() )
		startMove = true;
	else if( m_role[B].m_pChangeWidget && m_role[B].m_pChangeWidget->getStartMove() )
		startMove = true;
	else if( m_role[A].m_pRole  && m_role[A].m_pRole->getStartMove() )
		startMove = true;
	else if( m_role[B].m_pRole  && m_role[B].m_pRole->getStartMove() )
		startMove = true;
	else if( m_pName && m_pName->getStartMove() )
		startMove = true;

	if (startMove) {
		return;
	}

	switch(role) {
		case tr_MultiA:
			m_role[A].m_pMulti->setLinePositionRelatively( pos.x(), pos.y(),
							      oldPosition.x(), oldPosition.y() );
			break;

		case tr_MultiB:
			m_role[B].m_pMulti->setLinePositionRelatively( pos.x(), pos.y(),
							      oldPosition.x(), oldPosition.y() );
			break;

		case tr_Name:
		case tr_Coll_Message:
			m_pName->setLinePositionRelatively( pos.x(), pos.y(),
							    oldPosition.x(), oldPosition.y() );
			break;
		case tr_RoleAName:
			m_role[A].m_pRole->setLinePositionRelatively( pos.x(), pos.y(),
							     oldPosition.x(), oldPosition.y() );
			break;
		case tr_RoleBName:
			m_role[B].m_pRole->setLinePositionRelatively( pos.x(), pos.y(),
							     oldPosition.x(), oldPosition.y() );
			break;
		case tr_ChangeA:
			m_role[A].m_pChangeWidget->setLinePositionRelatively( pos.x(), pos.y(),
								     oldPosition.x(), oldPosition.y() );
			break;
		case tr_ChangeB:
			m_role[B].m_pChangeWidget->setLinePositionRelatively( pos.x(), pos.y(),
								     oldPosition.x(), oldPosition.y() );
			break;
		default:
			break;
	}
}

void AssociationWidget::mousePressEvent(QMouseEvent * me) {
	m_nMovingPoint = -1;
	//make sure we should be here depending on the button
	if(me -> button() != RightButton && me->button() != LeftButton)
		return;
	QPoint mep = me->pos();
	// See if the user has clicked on a point to start moving the line segment
	// from that point
	checkPoints(mep);
	if( me -> state() != ShiftButton )
		m_pView -> clearSelected();
	setSelected( !m_bSelected );
}

void AssociationWidget::mouseReleaseEvent(QMouseEvent * me) {
	if(me -> button() != RightButton && me->button() != LeftButton) {
		setSelected( false );
		return;
	}
	m_nMovingPoint = -1;
	if(me->button() != RightButton) {
		return;
	}
	// right button action:
	//work out the type of menu we want
	//work out if the association allows rolenames, multiplicity, etc
	//also must be within a certain distance to be a multiplicity menu
	QPoint p = me -> pos();
	ListPopupMenu::Menu_Type menuType = ListPopupMenu::mt_Undefined;
	int pos = m_LinePath.count() - 1;
	int DISTANCE = 40;//must be within this many pixels for it to be a multi menu
	float lengthMAP = sqrt( pow( double(m_LinePath.getPoint(0).x() - p.x()), 2.0) +
				pow( double(m_LinePath.getPoint(0).y() - p.y()), 2.0) );
	float lengthMBP = sqrt( pow( double(m_LinePath.getPoint(pos).x() - p.x()), 2.0) +
				pow( double(m_LinePath.getPoint(pos).y() - p.y()), 2.0) );
	Association_Type type = getAssocType();
	//allow multiplicity
	if( AssocRules::allowMultiplicity( type, getWidgetA() -> getBaseType() ) ) {
		if(lengthMAP < DISTANCE)
			menuType =  ListPopupMenu::mt_MultiA;
		else if(lengthMBP < DISTANCE)
			menuType = ListPopupMenu::mt_MultiB;
	}
	if( menuType == ListPopupMenu::mt_Undefined ) {
		if( type == at_Anchor )
			menuType = ListPopupMenu::mt_Anchor;
		else if( AssocRules::allowRole( type ) )
			menuType = ListPopupMenu::mt_RoleName;
		else
			menuType = ListPopupMenu::mt_Association_Selected;
		if(type == at_Coll_Message) {
			menuType = ListPopupMenu::mt_Collaboration_Message;
		}
	}
	m_pMenu = new ListPopupMenu(m_pView, menuType);
	m_pMenu->popup(me -> globalPos());
	connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuSelection(int)));
	setSelected();
}//end method mouseReleaseEvent

void AssociationWidget::slotMenuSelection(int sel) {
	QString oldText, newText;
	bool done = false;
	QFont font;
	Uml::Association_Type atype = getAssocType();

	//if it's a collaboration message we now just use the code in floatingtextwidget
	//this means there's some redundant code below but that's better than duplicated code
	if (atype == at_Coll_Message && sel != ListPopupMenu::mt_Delete) {
		m_pName->slotMenuSelection(sel);
		return;
	}

	switch(sel) {
	case ListPopupMenu::mt_Properties:
		if(atype == at_Seq_Message || atype == at_Seq_Message_Self) {
			// show op dlg for seq. diagram here
			// don't worry about here, I don't think it can get here as
			// line is widget on seq. diagram
			// here just in case - remove later after testing

		} else {  //standard assoc dialog
			m_pView -> updateDocumentation( false );
			AssocPropDlg dlg(static_cast<QWidget*>(m_pView), this );
			int result = dlg.exec();
			QString name = dlg.getName();
			QString doc = dlg.getDoc(), roleADoc = dlg.getRoleADoc(), roleBDoc = dlg.getRoleBDoc();
			QString rnA = dlg.getRoleAName(), rnB = dlg.getRoleBName(),
				 ma = dlg.getMultiA(), mb = dlg.getMultiB();
			Scope vA = dlg.getVisibilityA(), vB = dlg.getVisibilityB();
			Changeability_Type cA = dlg.getChangeabilityA(), cB = dlg.getChangeabilityB();
			if(result) {
				//rules built into these functions to stop updating incorrect values
				setName(name);
				setRoleNameA(rnA);
				setRoleNameB(rnB);
				setDoc(doc);
				setRoleADoc(roleADoc);
				setRoleBDoc(roleBDoc);
				setMultiA(ma);
				setMultiB(mb);
				setVisibilityA(vA);
				setVisibilityB(vB);
				setChangeabilityA(cA);
				setChangeabilityB(cB);
				m_pView -> showDocumentation( this, true );
			}
		}
		done = true;
		break;

	case ListPopupMenu::mt_Delete://for anchor
	case ListPopupMenu::mt_Delete_Association:
	case ListPopupMenu::mt_Delete_Message:
		m_pView->removeAssocInViewAndDoc(this);
		done = true;
		break;

	case ListPopupMenu::mt_Rename_MultiA:
		if(m_role[A].m_pMulti)
			oldText = m_role[A].m_pMulti -> getText();
		else
			oldText = "";
		newText = KInputDialog::getText(i18n("Multiplicity"), i18n("Enter multiplicity:") , oldText, 0, m_pView);
		if ( newText != oldText && FloatingText::isTextValid(newText) ) {
			setMultiA(newText);
		}
		break;


	case ListPopupMenu::mt_Rename_MultiB:
		if(m_role[B].m_pMulti)
			oldText = m_role[B].m_pMulti -> getText();
		else
			oldText = "";
		newText = KInputDialog::getText(i18n("Multiplicity"), i18n("Enter multiplicity:"), oldText, 0, m_pView);
		if ( newText != oldText && FloatingText::isTextValid(newText) ) {
			setMultiB(newText);
		}
		break;

	case ListPopupMenu::mt_Rename_Name:
		if(m_pName)
			oldText = m_pName-> getText();
		else
			oldText = "";

		newText = KInputDialog::getText(i18n("Association Name"), i18n("Enter association name:"), oldText, 0, m_pView);
		if ( newText != oldText && FloatingText::isTextValid(newText) )
			setName(newText);

		break;

	case ListPopupMenu::mt_Rename_RoleAName:
		if(m_role[A].m_pRole)
			oldText = m_role[A].m_pRole -> getText();
		else
			oldText = "";
		newText = KInputDialog::getText(i18n("Role Name"), i18n("Enter role name:"), oldText, 0, m_pView);
		if ( newText != oldText && FloatingText::isTextValid(newText) ) {
			setRoleNameA(newText);
		}
		break;

	case ListPopupMenu::mt_Rename_RoleBName:
		if(m_role[B].m_pRole)
			oldText = m_role[B].m_pRole -> getText();
		else
			oldText = "";
		newText = KInputDialog::getText(i18n("Role Name"), i18n("Enter role name:"), oldText, 0, m_pView);
		if ( newText != oldText && FloatingText::isTextValid(newText) ) {
			setRoleNameB(newText);
		}
		break;

	case ListPopupMenu::mt_Change_Font:
		font = getFont();
		if( KFontDialog::getFont( font, false, m_pView ) )
			setFont(font);
		break;

	case ListPopupMenu::mt_Change_Font_Selection:
		font = getFont();
		if( KFontDialog::getFont( font, false, m_pView ) )
			m_pView -> selectionSetFont( font );
		break;

	case ListPopupMenu::mt_Cut:
		m_pView->setStartedCut();
		m_pView->getDocument()->editCut();
		break;

	case ListPopupMenu::mt_Copy:
		m_pView->getDocument()->editCopy();
		break;

	case ListPopupMenu::mt_Paste:
		m_pView->getDocument()->editPaste();
		break;

	case ListPopupMenu::mt_Reset_Label_Positions:
		resetTextPositions();
		break;
	}//end switch
}


// utility method
void AssociationWidget::setFont (QFont font) { changeFont(font); }

// find a general font for the association
QFont AssociationWidget::getFont() const {
	QFont font;

	if( m_role[A].m_pRole )
		font = m_role[A].m_pRole -> getFont( );
	else    if( m_role[B].m_pRole)
		font = m_role[B].m_pRole -> getFont( );
	else    if( m_role[A].m_pMulti )
		font = m_role[A].m_pMulti -> getFont( );
	else    if( m_role[B].m_pMulti )
		font = m_role[B].m_pMulti -> getFont( );
	else    if( m_role[A].m_pChangeWidget)
		font = m_role[A].m_pChangeWidget-> getFont( );
	else    if( m_role[B].m_pChangeWidget)
		font = m_role[B].m_pChangeWidget-> getFont( );
	else    if( m_pName)
		font = m_pName-> getFont( );
	else
		font = m_role[A].m_pWidget -> getFont();

	return font;
}

void AssociationWidget::checkPoints(QPoint p) {
	m_nMovingPoint = -1;
	//only check if more than the two endpoints
	int size = m_LinePath.count();
	if( size <= 2 )
		return;
	//check all points except the end points to se if we clicked on one of them
	QPoint tempPoint;
	int x, y;
	const int BOUNDRY = 4; //echeck for pixels around the point
	for(int i=1;i<size-1;i++) {
		tempPoint = m_LinePath.getPoint( i );
		x = tempPoint.x();
		y = tempPoint.y();
		if( x - BOUNDRY <= p.x() && x + BOUNDRY >= p.x() &&
		    y - BOUNDRY <= p.y() && y + BOUNDRY >= p.y() ) {
			m_nMovingPoint = i;
			i = size; //no need to check the rest
		}//end if
	}//end for
}

void AssociationWidget::mouseMoveEvent(QMouseEvent* me) {
	//make sure we have a moving point
	//I don't think there is another reason for being here
	if( m_nMovingPoint == -1 || me->state() != LeftButton) {
		return;
	}
	setSelected();
	//new position for point
	QPoint p = me->pos();
	QPoint oldp = m_LinePath.getPoint(m_nMovingPoint);

	if( m_pView -> getSnapToGrid() ) {
		int newX = m_pView->snappedX( p.x() );
		int newY = m_pView->snappedY( p.y() );
		p.setX(newX);
		p.setY(newY);
	}

	//move event called now
	QMoveEvent m(p, oldp);
	moveEvent(&m);
	m_pView->resizeCanvasToItems();
}

AssociationWidget::Region AssociationWidget::getWidgetRegion(AssociationWidget * widget) const {
	if(widget -> getWidgetA() == m_role[A].m_pWidget)
		return m_role[A].m_WidgetRegion;
	if(widget -> getWidgetB() == m_role[B].m_pWidget)
		return m_role[B].m_WidgetRegion;
	return Error;
}

int AssociationWidget::getRegionCount(AssociationWidget::Region region, bool widgetA) {
	if(region == Error)
		return 0;
	int widgetCount = 0;
	AssociationWidgetList list = m_pView -> getAssociationList();
	AssociationWidgetListIt assoc_it(list);
	AssociationWidget* assocwidget = 0;
	while((assocwidget = assoc_it.current())) {
		++assoc_it;
		//don't count this association
		if (assocwidget == this)
			continue;
		UMLWidget *a = assocwidget -> getWidgetA();
		UMLWidget *b = assocwidget -> getWidgetB();
		/*
		//don't count associations to self if both of their end points are on the same region
		//they are different and placement won't interfere with them
		if( a == b && assocwidget -> m_role[A].m_WidgetRegion == assocwidget -> m_role[B].m_WidgetRegion )
			continue;
		 */
		if (widgetA) {	// counting for widgetA
			if (m_role[A].m_pWidget == a && region == assocwidget->m_role[A].m_WidgetRegion)
				widgetCount++;
			else if (m_role[A].m_pWidget == b && region == assocwidget->m_role[B].m_WidgetRegion)
				widgetCount++;
		} else {	// counting for widgetB
			if (m_role[B].m_pWidget == a && region == assocwidget->m_role[A].m_WidgetRegion)
				widgetCount++;
			else if (m_role[B].m_pWidget == b && region == assocwidget->m_role[B].m_WidgetRegion)
				widgetCount++;
		}
	}//end while
	return widgetCount;
}

void AssociationWidget::updateAssociations(int totalCount,
					   AssociationWidget::Region region, bool widgetA)
{
	if( region == Error )
		return;
	AssociationWidgetList list = m_pView -> getAssociationList();
	AssociationWidgetListIt assoc_it(list);
	AssociationWidget* assocwidget = 0;
	AssociationWidget* assocwidget2 = 0;

	AssociationWidgetList ordered;

	// we order the AssociationWidget list by region and x/y value
	while ( (assocwidget = assoc_it.current()) ) {
		++assoc_it;
		// widgetA is given as function parameter!
		if (widgetA) {
			// so we have to look at m_role[A].m_pWidget
			// now we must find out with which end assocwidget connects to
			// m_role[A].m_pWidget
			bool inWidgetARegion = ( m_role[A].m_pWidget == assocwidget -> getWidgetA() &&
						 region == assocwidget -> m_role[A].m_WidgetRegion );
			bool inWidgetBRegion = ( m_role[A].m_pWidget == assocwidget -> getWidgetB() &&
						 region == assocwidget -> m_role[B].m_WidgetRegion);
			if ( !inWidgetARegion && !inWidgetBRegion )
				continue;
			uint counter = 0;
			bool out = false;
			// now we go through all already known associations and insert
			// assocwidget at the right position so that the lines don't cross
			for (assocwidget2 = ordered.first(); assocwidget2;
			     assocwidget2 = ordered.next()) {
				switch (region) {
				case North:
				case South:
					if ( (inWidgetARegion &&
					      assocwidget2->getWidgetB()->x() >
					      assocwidget->getWidgetB()->x()) ||
					     (inWidgetBRegion &&
					      assocwidget2->getWidgetB()->x() >
					      assocwidget->getWidgetA()->x()) ) {
						ordered.insert(counter, assocwidget);
						out = true;
					}
					break;
				case East:
				case West:
					if ( (inWidgetARegion &&
					      assocwidget2->getWidgetB()->y() >
					      assocwidget->getWidgetB()->y()) ||
					     (inWidgetBRegion &&
					      assocwidget2->getWidgetB()->y() >
					      assocwidget->getWidgetA()->y()) ) {
						ordered.insert(counter, assocwidget);
						out = true;
					}
					break;
				default:
					break;
				} // switch (region)
				if (out)
					break;
				counter++;
			} // for (assocwidget2 = ordered.first(); assocwidget2; ...
			if (out == false)
				ordered.append(assocwidget);
		} else {
			// so we have to look at m_role[B].m_pWidget
			// now we must find out with which end assocwidget connects to
			// m_role[B].m_pWidget
			bool inWidgetARegion = ( m_role[B].m_pWidget == assocwidget -> getWidgetA() &&
						 region == assocwidget -> m_role[A].m_WidgetRegion );
			bool inWidgetBRegion = ( m_role[B].m_pWidget == assocwidget -> getWidgetB() &&
						 region == assocwidget -> m_role[B].m_WidgetRegion );
			if ( !inWidgetARegion && !inWidgetBRegion )
				continue;
			uint counter = 0;
			bool out = false;

			// now we go through all already known associations and insert
			// assocwidget at the right position so that the lines don't cross
			for (assocwidget2 = ordered.first(); assocwidget2;
			     assocwidget2 = ordered.next()) {
				switch (region) {
				case North:
				case South:
					if ( (inWidgetARegion &&
					      assocwidget2->getWidgetA()->x() >
					      assocwidget->getWidgetB()->x()) ||
					     (inWidgetBRegion &&
					      assocwidget2->getWidgetA()->x() >
					      assocwidget->getWidgetA()->x()) ) {
						ordered.insert(counter, assocwidget);
						out = true;
					}
					break;
				case East:
				case West:
					if ( (inWidgetARegion &&
					      assocwidget2->getWidgetA()->y() >
					      assocwidget->getWidgetB()->y()) ||
					     (inWidgetBRegion &&
					      assocwidget2->getWidgetA()->y() >
					      assocwidget->getWidgetA()->y()) ) {
						ordered.insert(counter, assocwidget);
						out = true;
					}
					break;
				default:
					break;
				} // switch (region)
				if (out)
					break;
				counter++;
			} // for (assocwidget2 = ordered.first(); assocwidget2; ...
			if (out == false)
				ordered.append(assocwidget);
		} // if (widgetA)
	} // while ( (assocwidget = assoc_it.current()) )

	// we now have an ordered list and we only have to call updateRegionLineCount
	int index = 1;
	for (assocwidget = ordered.first(); assocwidget; assocwidget = ordered.next()) {
		if(widgetA) {
			if( m_role[A].m_pWidget == assocwidget -> getWidgetA() ) {
				assocwidget -> updateRegionLineCount(index++, totalCount, region, true);
			} else if( m_role[A].m_pWidget == assocwidget -> getWidgetB() ) {
				assocwidget -> updateRegionLineCount(index++, totalCount, region, false);
			}
		} else { //end widgetA
			if(m_role[B].m_pWidget == assocwidget -> getWidgetA() ) {
				assocwidget -> updateRegionLineCount(index++, totalCount, region, true);
			} else if(m_role[B].m_pWidget == assocwidget -> getWidgetB() ) {
				assocwidget -> updateRegionLineCount(index++, totalCount, region, false);
			}
		}
	} // for (assocwidget = ordered.first(); ...)
}

void AssociationWidget::updateRegionLineCount(int index, int totalCount, AssociationWidget::Region region , bool widgetA) {
	if( region == Error )
		return;
	QPoint pt;
	int x, y, cw, ch, ww, wh;
	UMLWidget * pWidget = m_role[A].m_pWidget;
	if( !widgetA )
		pWidget = m_role[B].m_pWidget;
	//if a fork (ActivityWidget) for widget B then all associations should meet in the middle
	if( pWidget -> getBaseType() == Uml::wt_Activity) {
		if( static_cast<ActivityWidget*>(pWidget)->getActivityType() == ActivityWidget::Fork ) {
			totalCount = 2;
			index = 1;
		}
	}
	//if the association is to self and the line ends are on the same region  use a different calculation
	if(m_role[A].m_pWidget == m_role[B].m_pWidget && m_role[A].m_WidgetRegion == m_role[B].m_WidgetRegion) {
		x = m_role[A].m_pWidget -> getX();
		y = m_role[A].m_pWidget -> getY();
		wh = m_role[A].m_pWidget -> height();
		ww = m_role[A].m_pWidget -> width();
		int size = m_LinePath.count();
		//see if above widget ok to place assoc.
		switch( m_role[A].m_WidgetRegion ) {
		case North:
			m_LinePath.setPoint( 0, QPoint( x + ( ww / 4 ), y ) );
			m_LinePath.setPoint( size - 1, QPoint(x + ( ww * 3 / 4 ), y ) );
			break;

		case South:
			m_LinePath.setPoint( 0, QPoint( x + ( ww / 4 ), y + wh ) );
			m_LinePath.setPoint( size - 1, QPoint( x + ( ww * 3 / 4 ), y + wh ) );
			break;

		case East:
			m_LinePath.setPoint( 0, QPoint( x + ww, y + ( wh / 4 ) ) );
			m_LinePath.setPoint( size - 1, QPoint( x + ww, y + ( wh * 3 / 4 ) ) );
			break;

		case West:
			m_LinePath.setPoint( 0, QPoint( x, y + ( wh / 4 ) ) );
			m_LinePath.setPoint( size - 1, QPoint( x, y + ( wh * 3 / 4 ) ) );
			break;
		default:
			break;
		}//end switch
		m_role[A].m_OldCorner.setX( x );
		m_role[A].m_OldCorner.setY( y );
		m_role[B].m_OldCorner.setX( x );
		m_role[B].m_OldCorner.setY( y );

		return;
	}
	else if(widgetA) {
		m_role[A].m_nIndex = index;
		m_role[A].m_nTotalCount = totalCount;
		x =  m_role[A].m_pWidget -> getX();
		y = m_role[A].m_pWidget -> getY();
		ww = m_role[A].m_pWidget->width();
		wh = m_role[A].m_pWidget->height();
		ch = wh * index / totalCount;
		cw = ww * index / totalCount;
		m_role[A].m_OldCorner.setX(x);
		m_role[A].m_OldCorner.setY(y);
	} else {
		m_role[B].m_nIndex = index;
		m_role[B].m_nTotalCount = totalCount;
		x = m_role[B].m_pWidget -> getX();
		y = m_role[B].m_pWidget -> getY();
		ww = m_role[B].m_pWidget->getWidth();
		wh = m_role[B].m_pWidget->getHeight();
		ch = wh * index / totalCount;
		cw = ww * index / totalCount;
		m_role[B].m_OldCorner.setX(x);
		m_role[B].m_OldCorner.setY(y);
	}

	switch(region) {
	case West:
		pt.setX(x);
		pt.setY(y + ch);
		break;
	case North:
		pt.setX(x + cw);
		pt.setY(y);
		break;
	case East:
		pt.setX(x + ww);
		pt.setY(y + ch);
		break;
	case South:
	case Center:
		pt.setX(x + cw);
		pt.setY(y + wh);
		break;
	default:
		break;
	}
	if(widgetA)
		m_LinePath.setPoint( 0, pt );
	else
		m_LinePath.setPoint( m_LinePath.count() - 1, pt );
}

void AssociationWidget::setSelected(bool _select /* = true */) {
	m_bSelected = _select;
	m_LinePath.setSelected( _select );
	if( m_pName)
		m_pName-> setSelected( _select );
	if( m_role[A].m_pRole )
		m_role[A].m_pRole -> setSelected( _select );
	if( m_role[B].m_pRole )
		m_role[B].m_pRole -> setSelected( _select );
	if( m_role[A].m_pMulti )
		m_role[A].m_pMulti -> setSelected( _select );
	if( m_role[B].m_pMulti )
		m_role[B].m_pMulti -> setSelected( _select );
	if( m_role[A].m_pChangeWidget)
		m_role[A].m_pChangeWidget-> setSelected( _select );
	if( m_role[B].m_pChangeWidget)
		m_role[B].m_pChangeWidget-> setSelected( _select );
	// Update the docwindow for this association.
	// This is done last because each of the above setSelected calls
	// overwrites the docwindow, but we want the main association doc
	// to win.
	if( _select ) {
		if( m_pView -> getSelectCount() == 0 )
			m_pView -> showDocumentation( this, false );
	} else
		m_pView -> updateDocumentation( true );
}

bool AssociationWidget::onAssociation(const QPoint & point) {
	return ( m_LinePath.onLinePath( point ) != -1 );
}

void AssociationWidget::slotRemovePopupMenu()
{
	if(m_pMenu) {
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuSelection(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
}

void AssociationWidget::slotClearAllSelected() {
	setSelected( false );
}

void AssociationWidget::moveMidPointsBy( int x, int y ) {
	int pos = m_LinePath.count() - 1;
	for( int i=1 ; i < (int)pos ; i++ ) {
		QPoint p = m_LinePath.getPoint( i );
		int newX = p.x() + x;
		int newY = p.y() + y;
		newX = m_pView -> snappedX( newX );
		newY = m_pView -> snappedY( newY );
		p.setX( newX );
		p.setY( newY );
		m_LinePath.setPoint( i, p );
	}
}

void AssociationWidget::moveEntireAssoc( int x, int y ) {
	//TODO: ADD SUPPORT FOR ASSOC. ON SEQ. DIAGRAMS WHEN NOTES BACK IN.
	moveMidPointsBy( x, y );
	calculateEndingPoints();
	calculateNameTextSegment();
	resetTextPositions();
}

void AssociationWidget::changeFont(QFont font) {
	if( m_pName) {
		m_pName->setFont( font );
	}
	if( m_role[A].m_pRole ) {
		m_role[A].m_pRole->setFont( font );
	}
	if( m_role[B].m_pRole ) {
		m_role[B].m_pRole->setFont( font );
	}
	if( m_role[A].m_pMulti ) {
		m_role[A].m_pMulti->setFont( font );
	}
	if( m_role[B].m_pMulti ) {
		m_role[B].m_pMulti->setFont( font );
	}
	if( m_role[A].m_pChangeWidget)
		m_role[A].m_pChangeWidget->setFont( font );
	if( m_role[B].m_pChangeWidget)
		m_role[B].m_pChangeWidget->setFont( font );
}

QRect AssociationWidget::getAssocLineRectangle()
{
	QRect rectangle;
	QPoint p;
	uint pen_width;

	/* we also want the end points connected to the other widget */
	int pos = m_LinePath.count();

	/* go through all points on the linepath */
	for( int i=0 ; i < (int) pos; i++ )
	{
		p = m_LinePath.getPoint( i );

		/* the first point is our starting point */
		if (i == 0) {
			rectangle.setRect(p.x(), p.y(), 0, 0);
			continue;
		}

		/* the lines have the width of the pen */
		pen_width = m_LinePath.getPen().width();
		if (pen_width == 0)
			pen_width = 1; // width must be at least 1

		if (p.x() < rectangle.x())
			rectangle.setX(p.x());
		if (p.y() < rectangle.y())
			rectangle.setY(p.y());
		if (p.x() > rectangle.x() + rectangle.width())
			rectangle.setWidth(abs((int)(p.x() - rectangle.x() + pen_width)));
		if (p.y() > rectangle.y() + rectangle.height())
			rectangle.setHeight(abs((int)(p.y() - rectangle.y() + pen_width)));
	}
	return rectangle;
}


void AssociationWidget::init (UMLView *view)
{
	// pointer to parent viewwidget object
	m_pView = view;

	m_pAssociation = NULL;   // Must be set to something useful later.

	// pointers to floating text widgets objects owned by this association
	m_pName = 0;
	m_role[A].m_pChangeWidget = 0;
	m_role[B].m_pChangeWidget = 0;
	m_role[A].m_pMulti = 0;
	m_role[B].m_pMulti = 0;
	m_role[A].m_pRole = 0;
	m_role[B].m_pRole = 0;
	m_role[A].m_pWidget = 0;
	m_role[B].m_pWidget = 0;

	// associationwidget attributes
	m_role[A].m_nCornerRegion = -1;
	m_role[B].m_nCornerRegion = -1;
	m_role[A].m_WidgetRegion = Error;
	m_role[B].m_WidgetRegion = Error;
	m_bActivated = false;
	m_unNameLineSegment = 0;
	m_pMenu = 0;
	m_bSelected = false;
	m_nMovingPoint = -1;

	// Initialize local members.
	// These are only used if we don't have a UMLAssociation attached.
	m_role[A].m_Visibility = Public;
	m_role[B].m_Visibility = Public;
	m_role[A].m_Changeability = chg_Changeable;
	m_role[B].m_Changeability = chg_Changeable;
	m_AssocType = Uml::at_Association;

	m_LinePath.setAssociation( this );

	connect(m_pView, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu()));
	connect(m_pView, SIGNAL( sigClearAllSelected() ), this, SLOT( slotClearAllSelected() ) );
}

void AssociationWidget::resetTextPositions() {
	if (m_role[A].m_pMulti) {
		setTextPosition( tr_MultiA, calculateTextPosition(tr_MultiA) );
	}
	if (m_role[B].m_pMulti) {
		setTextPosition( tr_MultiB, calculateTextPosition(tr_MultiB) );
	}
	if (m_role[A].m_pChangeWidget) {
		setTextPosition( tr_ChangeA, calculateTextPosition(tr_ChangeA) );
	}
	if (m_role[B].m_pChangeWidget) {
		setTextPosition( tr_ChangeB, calculateTextPosition(tr_ChangeB) );
	}
	if (m_pName) {
		setTextPosition( tr_Name, calculateTextPosition(tr_Name) );
	}
	if (m_role[A].m_pRole) {
		setTextPosition( tr_RoleAName, calculateTextPosition(tr_RoleAName) );
	}
	if (m_role[B].m_pRole) {
		setTextPosition( tr_RoleBName, calculateTextPosition(tr_RoleBName) );
	}
}

void AssociationWidget::setIndexA(int index) {
	m_role[A].m_nIndex = index;
}

int AssociationWidget::getIndexA() const {
	return m_role[A].m_nIndex;
}

void AssociationWidget::setIndexB(int index) {
	m_role[B].m_nIndex = index;
}

int AssociationWidget::getIndexB() const {
	return m_role[B].m_nIndex;
}

void AssociationWidget::setTotalCountA(int count) {
	m_role[A].m_nTotalCount = count;
}

int AssociationWidget::getTotalCountA() const {
	return  m_role[A].m_nTotalCount;
}

void AssociationWidget::setTotalCountB(int count) {
	m_role[B].m_nTotalCount = count;
}

int AssociationWidget::getTotalCountB() const {
	return m_role[B].m_nTotalCount;
}

void AssociationWidget::setWidgetA( UMLWidget* widgetA) {
	m_role[A].m_pWidget = widgetA;
	if (widgetA) {
		m_role[A].m_pWidget->addAssoc(this);
		if(m_pAssociation)
			m_pAssociation->setObjectA(widgetA->getUMLObject());
	}
}

void AssociationWidget::setWidgetB( UMLWidget* widgetB) {
	m_role[B].m_pWidget = widgetB;
	if (widgetB) {
		m_role[B].m_pWidget->addAssoc(this);
		if(m_pAssociation)
			m_pAssociation->setObjectB(widgetB->getUMLObject());
	}
}

UMLWidget* AssociationWidget::findWidget( int id, const UMLWidgetList& widgets,
						  const MessageWidgetList* pMessages /* = NULL */)
{
	UMLWidgetListIt it( widgets );
	UMLWidget * obj = NULL;
	while ( (obj = it.current()) != NULL ) {
		++it;
		if (obj->getBaseType() == wt_Object) {
			if (static_cast<ObjectWidget *>(obj)->getLocalID() == id)
				return obj;
		} else if (obj->getID() == id) {
			return obj;
		}
	}

	if (pMessages == NULL)
		return NULL;

	MessageWidgetListIt mit( *pMessages );
	while ( (obj = (UMLWidget*)mit.current()) != NULL ) {
		++mit;
		if( obj -> getID() == id )
			return obj;
	}
	return NULL;
}

bool AssociationWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement assocElement = qDoc.createElement( "UML:AssocWidget" );
	bool status = true;

	if (m_pAssociation) {
		assocElement.setAttribute( "xmi.id", m_pAssociation->getID() );
	} else {
		assocElement.setAttribute( "type", m_AssocType );
		assocElement.setAttribute( "visibilityA", m_role[A].m_Visibility);
		assocElement.setAttribute( "visibilityB", m_role[B].m_Visibility);
		assocElement.setAttribute( "changeabilityA", m_role[A].m_Changeability);
		assocElement.setAttribute( "changeabilityB", m_role[B].m_Changeability);
		assocElement.setAttribute( "roleAdoc", m_role[A].m_RoleDoc);
		assocElement.setAttribute( "roleBdoc", m_role[B].m_RoleDoc);
		// assocElement.setAttribute( "documentation", m_Doc );
	}
	assocElement.setAttribute( "widgetaid", getWidgetAID() );
	assocElement.setAttribute( "widgetbid", getWidgetBID() );
	assocElement.setAttribute( "indexa", m_role[A].m_nIndex );
	assocElement.setAttribute( "indexb", m_role[B].m_nIndex );
	assocElement.setAttribute( "totalcounta", m_role[A].m_nTotalCount );
	assocElement.setAttribute( "totalcountb", m_role[B].m_nTotalCount );
	m_LinePath.saveToXMI( qDoc, assocElement );

	if( m_pName )
		status =  m_pName -> saveToXMI( qDoc, assocElement );

	if( m_role[A].m_pMulti )
		status =  m_role[A].m_pMulti -> saveToXMI( qDoc, assocElement );

	if( m_role[B].m_pMulti )
		status = m_role[B].m_pMulti -> saveToXMI( qDoc, assocElement );

	if( m_role[A].m_pRole )
		status = m_role[A].m_pRole -> saveToXMI( qDoc, assocElement );

	if( m_role[B].m_pRole )
		status = m_role[B].m_pRole -> saveToXMI( qDoc, assocElement );

	if( m_role[A].m_pChangeWidget )
		status =  m_role[A].m_pChangeWidget -> saveToXMI( qDoc, assocElement );

	if( m_role[B].m_pChangeWidget )
		status =  m_role[B].m_pChangeWidget -> saveToXMI( qDoc, assocElement );

	qElement.appendChild( assocElement );
	return status;
}

bool AssociationWidget::loadFromXMI( QDomElement & qElement,
				     const UMLWidgetList& widgets,
				     const MessageWidgetList* pMessages )
{

	// load child widgets first
        QString widgetaid = qElement.attribute( "widgetaid", "-1" );
        QString widgetbid = qElement.attribute( "widgetbid", "-1" );
        int aId = widgetaid.toInt();
        int bId = widgetbid.toInt();
        UMLWidget *pWidgetA = findWidget( aId, widgets, pMessages );
        if (!pWidgetA) {
		kdError() << "AssociationWidget::loadFromXMI(): "
			  << "cannot find widget for roleA id " << aId << endl;
		return false;
        }
        UMLWidget *pWidgetB = findWidget( bId, widgets, pMessages );
        if (!pWidgetB) {
		kdError() << "AssociationWidget::loadFromXMI(): "
			  << "cannot find widget for roleB id " << bId << endl;
		return false;
        }
        setWidgetA(pWidgetA);
        setWidgetB(pWidgetB);

	QString id = qElement.attribute( "xmi.id", "-1" );
	bool oldStyleLoad = false;
	int nId = id.toInt();
	if (nId == -1) {

		// xmi.id not present, ergo either a pure widget association,
		// or old (pre-1.2) style:
		// Everything is loaded from the AssociationWidget.
		// UMLAssociation may or may not be saved - if it is, it's a dummy.
		// Create the UMLAssociation if both roles are UML objects;
		// else load the info locally.

                QString type = qElement.attribute( "type", "-1" );
		Uml::Association_Type aType = (Uml::Association_Type) type.toInt();
		if (UMLAssociation::assocTypeHasUMLRepresentation(aType)) {
			// lack of an association in our widget AND presence of
			// both uml objects for each role clearly identifies this
			// as reading in an old-school file. Note it as such, and
			// create, and add, the UMLAssociation for this widget.
       		 	// Remove this special code when backwards compatibility
			// with older files isn't important anymore. -b.t.
			UMLObject* umlRoleA = pWidgetA->getUMLObject();
			UMLObject* umlRoleB = pWidgetB->getUMLObject();
			if (!m_pAssociation && umlRoleA && umlRoleB)
			{
				oldStyleLoad = true; // flag for further special config below
				m_pAssociation = m_pView->getDocument()->createUMLAssociation(
							umlRoleA, umlRoleB, aType);
				connect(m_pAssociation, SIGNAL(modified()), this,
					SLOT(mergeUMLRepresentationIntoAssociationData()));
				m_pAssociation->nrof_parent_widgets++;
			}
		}

		setDoc( qElement.attribute("documentation", "") );
		setRoleADoc( qElement.attribute("roleAdoc", "") );
		setRoleBDoc( qElement.attribute("roleBdoc", "") );

                setAssocType(aType);

		// visibilty defaults to Public if it cant set it here..
		QString visibilityA = qElement.attribute( "visibilityA", "0");
		if (visibilityA.toInt() > 0)
			setVisibilityA( (Scope) visibilityA.toInt());

		QString visibilityB = qElement.attribute( "visibilityB", "0");
		if (visibilityB.toInt() > 0)
			setVisibilityB( (Scope) visibilityB.toInt());

		// Changeability defaults to "Changeable" if it cant set it here..
		QString changeabilityA = qElement.attribute( "changeabilityA", "0");
		if (changeabilityA.toInt() > 0)
			setChangeabilityA ( (Changeability_Type) changeabilityA.toInt());

		QString changeabilityB = qElement.attribute( "changeabilityB", "0");
		if (changeabilityB.toInt() > 0)
			setChangeabilityB ( (Changeability_Type) changeabilityB.toInt());

	} else {

		// we should disconnect any prior association (can this happen??)
		if(m_pAssociation)
		{
			m_pAssociation->disconnect(this);
			m_pAssociation->nrof_parent_widgets--;
		}

		// New style: The xmi.id is a reference to the UMLAssociation.
		UMLDoc* umldoc = m_pView->getDocument();
		m_pAssociation = (UMLAssociation*)umldoc->findUMLObject(nId);
		if (m_pAssociation == NULL) {
			kdError() << " AssociationWidget cannot find UML:Association " << nId << " for loadFromXMI"<< endl;
			return false;
		} else
		{
			connect(m_pAssociation, SIGNAL(modified()), this,
				SLOT(mergeUMLRepresentationIntoAssociationData()));
			m_pAssociation->nrof_parent_widgets++;
		}

		m_LinePath.setAssocType( m_pAssociation->getAssocType() );

	}

	QString indexa = qElement.attribute( "indexa", "0" );
	QString indexb = qElement.attribute( "indexb", "0" );
	QString totalcounta = qElement.attribute( "totalcounta", "0" );
	QString totalcountb = qElement.attribute( "totalcountb", "0" );
	m_role[A].m_nIndex = indexa.toInt();
	m_role[B].m_nIndex = indexb.toInt();
	m_role[A].m_nTotalCount = totalcounta.toInt();
	m_role[B].m_nTotalCount = totalcountb.toInt();

	//now load child elements
	QDomNode node = qElement.firstChild();
	QDomElement element = node.toElement();
	while( !element.isNull() ) {
		QString tag = element.tagName();
		if( tag == "linepath" ) {
			if( !m_LinePath.loadFromXMI( element ) )
				return false;
			else {
				// set up 'old' corner from first point in line
				// as IF this ISNT done, then the subsequent call to
				// widgetMoved will inadvertantly think we have made a
				// big move in the position of the association when we haven't.
				QPoint p = m_LinePath.getPoint(0);
				m_role[A].m_OldCorner.setX(p.x());
				m_role[A].m_OldCorner.setY(p.y());
			}
		} else if( tag == "UML:FloatingTextWidget" ) {
			QString r = element.attribute( "role", "-1");
			if( r == "-1" )
				return false;
			Uml::Text_Role role = (Uml::Text_Role)r.toInt();
			FloatingText *ft = new FloatingText(m_pView, role);
			if( ! ft->loadFromXMI(element) )
				return false;

			// always need this
			ft->setAssoc(this);

			switch( role ) {
				case Uml::tr_MultiA:
					m_role[A].m_pMulti = ft;
					if(oldStyleLoad)
						setMultiA(m_role[A].m_pMulti->getText());
					break;

				case Uml::tr_MultiB:
					m_role[B].m_pMulti = ft;
					if(oldStyleLoad)
						setMultiB(m_role[B].m_pMulti->getText());
					break;

				case Uml::tr_ChangeA:
					m_role[A].m_pChangeWidget = ft;
					break;

				case Uml::tr_ChangeB:
					m_role[B].m_pChangeWidget = ft;
					break;

				case Uml::tr_Name:
					m_pName = ft;
					if(oldStyleLoad)
						setName(m_pName->getText());
					break;

				case Uml::tr_Coll_Message:
					m_pName = ft;
					ft->setAssoc(this);
					ft->setActivated();
					setTextPosition( tr_Name, calculateTextPosition(tr_Name) );
        				if(FloatingText::isTextValid(ft->getText()))
						ft -> show();
					else
						ft -> hide();
					break;

				case Uml::tr_RoleAName:
					m_role[A].m_pRole = ft;
					if(oldStyleLoad)
						if( AssocRules::allowRole( getAssocType()) )
							setRoleNameA(m_role[A].m_pRole->getText());
						else {
							m_pView->removeWidget(m_role[A].m_pRole);
							m_role[A].m_pRole = 0;
						}
					break;
				case Uml::tr_RoleBName:
					m_role[B].m_pRole = ft;
					if(oldStyleLoad)
						if( AssocRules::allowRole( getAssocType()) )
							setRoleNameB(m_role[B].m_pRole->getText());
						else {
							m_pView->removeWidget(m_role[B].m_pRole);
							m_role[B].m_pRole = 0;
						}
					break;
				default:
					kdDebug() << "AssociationWidget::loadFromXMI(): "
						<< "unexpected FloatingText (textrole "
						<< role << ")" << endl;
					delete ft;
					break;
			}
		}
		node = element.nextSibling();
		element = node.toElement();
	}

	return true;
}

bool AssociationWidget::loadFromXMI( QDomElement & qElement ) {
	const MessageWidgetList& messages = m_pView->getMessageList();
	return loadFromXMI( qElement, m_pView->getWidgetList(), &messages );
}

#include "associationwidget.moc"
