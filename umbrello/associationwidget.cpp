 /*
  *  copyright (C) 2002-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "associationwidget.h"
// system includes
#include <cstdlib>
#include <cmath>
// qt/kde includes
#include <qcanvas.h>
#include <kdebug.h>
#include <klocale.h>
// app includes
#include "activitywidget.h"
#include "uml.h"
#include "umlview.h"
#include "umldoc.h"
#include "umlwidget.h"
#include "messagewidget.h"
#include "umlrole.h"
#include "listpopupmenu.h"
#include "classwidget.h"
#include "class.h"
#include "attribute.h"
#include "operation.h"
#include "association.h"
#include "assocrules.h"
#include "floatingtext.h"
#include "objectwidget.h"
#include "model_utils.h"
#include "widget_utils.h"
#include "dialogs/assocpropdlg.h"
#include "inputdialog.h"

using namespace Uml;

// this constructor really only for loading from XMI, otherwise it
// is bad..and shouldnt be allowed as it creates an incomplete
// associationwidget.
AssociationWidget::AssociationWidget(UMLView *view)
	: WidgetBase(view)
{
	init(view);
}

// the preferred constructor
AssociationWidget::AssociationWidget(UMLView *view, UMLWidget* pWidgetA,
				     Association_Type assocType, UMLWidget* pWidgetB,
				     UMLAssociation *umlassoc /* = NULL */)
	: WidgetBase(view)
{
	init(view);
	if (umlassoc)
		setUMLAssociation(umlassoc);
	else
	// set up UMLAssociation obj if assoc is represented and both roles are UML objects
	     if (UMLAssociation::assocTypeHasUMLRepresentation(assocType)) {
		UMLObject* umlRoleA = pWidgetA->getUMLObject();
		UMLObject* umlRoleB = pWidgetB->getUMLObject();
		if (umlRoleA != NULL && umlRoleB != NULL) {
			bool swap;

			// THis isnt correct. We could very easily have more than one
			// of the same type of association between the same two objects.
			// Just create the association. This search should have been
			// done BEFORE creation of the widget, if it mattered to the code.
			// But lets leave check in here for the time being so that debugging
			// output is shown, in case there is a collision with code elsewhere.
			UMLAssociation * myAssoc = m_umldoc->findAssociation( assocType, umlRoleA, umlRoleB, &swap );
			if (myAssoc != NULL) {
				if (assocType == at_Generalization) {
					kdDebug() << " Ignoring second construction of same generalization"
						  << endl;
				} else {
					kdDebug() << " constructing a similar or exact same assoc " <<
					"as an already existing assoc (swap=" << swap << ")" << endl;
					// now, just create a new association anyways
					myAssoc = NULL;
				}
			}
			if (myAssoc == NULL)
				myAssoc = new UMLAssociation( assocType, umlRoleA, umlRoleB );
			setUMLAssociation(myAssoc);
		}
	}

	setWidget(pWidgetA, A);
	setWidget(pWidgetB, B);

	setAssocType(assocType);

	calculateEndingPoints();

	//The AssociationWidget is set to Activated because it already has its side widgets
	setActivated(true);

	// sync UML meta-data to settings here
	mergeAssociationDataIntoUMLRepresentation();

	// Collaboration messages need a name label because it's that
	// which lets operator== distinguish them, which in turn
	// permits us to have more than one message between two objects.
	if (getAssocType() == at_Coll_Message) {
		// Create a temporary name to bring on setName()
		int collabID = m_pView->generateCollaborationId();
		setName("m" + QString::number(collabID));
		if (m_pObject) {
			m_pName->setUMLObject( m_pObject );
		} else {
			ObjectWidget *ow = static_cast<ObjectWidget*>(m_role[B].m_pWidget);
			m_pName->setUMLObject( ow->getUMLObject() );
		}
	}
}

AssociationWidget::~AssociationWidget() {
	cleanup();
	if (m_pAssocClassLine)
		delete m_pAssocClassLine;
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
		lhs.m_WidgetRegion = rhs.m_WidgetRegion;
	}

	m_bActivated = Other.m_bActivated;
	m_unNameLineSegment = Other.m_unNameLineSegment;
	m_bFocus = Other.m_bFocus;
	m_pMenu = Other.m_pMenu;
	setUMLAssociation(Other.getAssociation());
	m_bSelected = Other.m_bSelected;
	m_nMovingPoint = Other.m_nMovingPoint;

	return *this;
}

bool AssociationWidget::operator==(AssociationWidget & Other) {
	if( this == &Other )
		return true;

	if( !m_pObject || !Other.m_pObject ) {
		if( !Other.m_pObject && m_pObject )
			return false;
		if( Other.m_pObject && !m_pObject )
			return false;
	} else if( m_pObject != Other.m_pObject )
		return false;

	if (getAssocType() != Other.getAssocType())
		return false;

	if (getWidgetID(A) != Other.getWidgetID(A))
		return false;

	if (getWidgetID(B) != Other.getWidgetID(B))
		return false;

	if (getWidget(A)->getBaseType() == Uml::wt_Object &&
	    Other.getWidget(A)->getBaseType() == Uml::wt_Object) {
		ObjectWidget *ownA = static_cast<ObjectWidget*>(getWidget(A));
		ObjectWidget *otherA = static_cast<ObjectWidget*>(Other.getWidget(A));
		if (ownA->getLocalID() != otherA->getLocalID())
			return false;
	}

	if (getWidget(B)->getBaseType() == Uml::wt_Object &&
	    Other.getWidget(B)->getBaseType() == Uml::wt_Object) {
		ObjectWidget *ownB = static_cast<ObjectWidget*>(getWidget(B));
		ObjectWidget *otherB = static_cast<ObjectWidget*>(Other.getWidget(B));
		if (ownB->getLocalID() != otherB->getLocalID())
			return false;
	}

	// Two objects in a collaboration can have multiple messages between each other.
	// Here we depend on the messages having names, and the names must be different.
	// That is the reason why collaboration messages have strange initial names like
	// "m29997" or similar.
	return (getName() == Other.getName());
}

bool AssociationWidget::operator!=(AssociationWidget & Other) {
	return !(*this == Other);
}

UMLAssociation * AssociationWidget::getAssociation () {
	if (m_pObject == NULL || m_pObject->getBaseType() != ot_Association)
		return NULL;
	return static_cast<UMLAssociation*>(m_pObject);
}

UMLAttribute * AssociationWidget::getAttribute () {
	if (m_pObject == NULL || m_pObject->getBaseType() != ot_Attribute)
		return NULL;
	return static_cast<UMLAttribute*>(m_pObject);
}

FloatingText* AssociationWidget::getMultiWidget(Role_Type role) {
	return m_role[role].m_pMulti;
}

QString AssociationWidget::getMulti(Role_Type role) const
{
	if (m_role[role].m_pMulti == NULL)
		return "";
	return m_role[role].m_pMulti->getText();
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
	if (m_pObject)
		return m_pObject->getDoc();
	return m_Doc;
}

FloatingText* AssociationWidget::getRoleWidget(Role_Type role) {
	return m_role[role].m_pRole;
}

FloatingText* AssociationWidget::getChangeWidget(Role_Type role) {
	return m_role[role].m_pChangeWidget;
}

QString AssociationWidget::getRoleName(Role_Type role) const {
	if (m_role[role].m_pRole == NULL)
		return "";
	return m_role[role].m_pRole->getText();
}

QString AssociationWidget::getRoleDoc(Role_Type role) const {
	if (m_pObject == NULL || m_pObject->getBaseType() != ot_Association)
		return "";
	UMLAssociation *umla = static_cast<UMLAssociation*>(m_pObject);
	return umla->getRoleDoc(role);
}

void AssociationWidget::setName(const QString &strName) {
	bool newLabel = false;
	if(!m_pName) {
		// Don't construct the FloatingText if the string is empty.
		if (strName.isEmpty())
			return;

		newLabel = true;
		m_pName = new FloatingText(m_pView, CalculateNameType(tr_Name), strName);
		m_pName->setLink(this);
		if (m_role[B].m_pWidget)
			m_pName->setUMLObject(m_role[B].m_pWidget->getUMLObject());
	} else {
		m_pName->setText(strName);
	}

	// set attribute of UMLAssociation associated with this associationwidget
	if (m_pObject)
		m_pObject->setName(strName);

	setTextPosition( tr_Name );
	if (newLabel) {
		m_pName->setActivated();
		m_pView->addWidget(m_pName);
	}

	if(FloatingText::isTextValid(m_pName->getText()))
		m_pName -> show();
	else
		m_pName -> hide();

}

void AssociationWidget::setMulti(const QString &strMulti, Role_Type role) {
	bool newLabel = false;
	Text_Role tr = (role == A ? tr_MultiA : tr_MultiB);

	if(!m_role[role].m_pMulti) {
		// Don't construct the FloatingText if the string is empty.
		if (strMulti.isEmpty())
			return;

		newLabel = true;
		m_role[role].m_pMulti = new FloatingText(m_pView, tr, strMulti);
		m_role[role].m_pMulti->setLink(this);
		m_pView->addWidget(m_role[role].m_pMulti);
	} else {
		if (m_role[role].m_pMulti->getText().isEmpty()) {
			newLabel = true;
		}
		m_role[role].m_pMulti->setText(strMulti);
	}

	m_role[role].m_pMulti->setActivated();

	if (newLabel) {
		setTextPosition( tr );
	}

	if(FloatingText::isTextValid(m_role[role].m_pMulti->getText()))
		m_role[role].m_pMulti -> show();
	else
		m_role[role].m_pMulti -> hide();

	if (m_pObject && m_pObject->getBaseType() == ot_Association)
		getAssociation()->setMulti(strMulti, role);
}

bool AssociationWidget::setRoleName (const QString &strRole, Role_Type role) {
	bool newLabel = false;
	Association_Type type = getAssocType();
	//if the association is not supposed to have a Role FloatingText
	if( !AssocRules::allowRole( type ) )  {
		return false;
	}

	Text_Role tr = (role == A ? tr_RoleAName : tr_RoleBName);
	if(!m_role[role].m_pRole) {
		// Don't construct the FloatingText if the string is empty.
		if (strRole.isEmpty())
			return true;

		newLabel = true;
		m_role[role].m_pRole = new FloatingText(m_pView, tr, strRole);
		m_role[role].m_pRole->setLink(this);
		m_pView->addWidget(m_role[role].m_pRole);
		Scope scope = getVisibility(role);
		m_role[role].m_pRole->setPreText(Umbrello::scopeToString(scope));
	} else {
		if (m_role[role].m_pRole->getText().isEmpty()) {
			newLabel = true;
		}
		m_role[role].m_pRole->setText(strRole);
	}

	// set attribute of UMLAssociation associated with this associationwidget
	if (m_pObject && m_pObject->getBaseType() == ot_Association)
		getAssociation()->setRoleName(strRole, role);
	m_role[role].m_RoleName = strRole;

	m_role[role].m_pRole->setActivated();

	if (newLabel) {
		setTextPosition( tr );
	}

	if(FloatingText::isTextValid(m_role[role].m_pRole->getText()))
		m_role[role].m_pRole -> show();
	else
		m_role[role].m_pRole -> hide();
	return true;
}

void AssociationWidget::setDoc (const QString &doc) {
	if (m_pObject)
		m_pObject->setDoc(doc);
	else
		m_Doc = doc;
}

void AssociationWidget::setRoleDoc (const QString &doc, Role_Type role) {
	if (m_pObject && m_pObject->getBaseType() == ot_Association)
		getAssociation()->setRoleDoc(doc, role);
	else
		m_role[role].m_RoleDoc = doc;
}

void AssociationWidget::setMessageText(FloatingText *ft) {
	QString message;
	if (getAssocType() == at_Coll_Message) {
		if (m_pObject != NULL) {
			message = getMulti(A) + ": " + getOperationText(m_pView);
		} else {
			message = getMulti(A) + ": " + getName();
		}
	} else {
		message = getName();
	}
	ft->setText(message);
}

Scope AssociationWidget::getVisibility(Role_Type role) const {
	if (m_pObject == NULL || m_pObject->getBaseType() != ot_Association)
		return m_role[role].m_Visibility;
	UMLAssociation *umla = static_cast<UMLAssociation*>(m_pObject);
	return umla->getVisibility(role);
}

void AssociationWidget::setVisibility (Scope value, Role_Type role)
{
	if (value == getVisibility(role))
		return;
	if (m_pObject && m_pObject->getBaseType() == ot_Association)  // update our model object
		getAssociation()->setVisibility(value, role);
	m_role[role].m_Visibility = value;
	// update role pre-text attribute as appropriate
	if (m_role[role].m_pRole) {
		QString scopeString = Umbrello::scopeToString(value);
		m_role[role].m_pRole->setPreText(scopeString);
	}
}

Changeability_Type AssociationWidget::getChangeability(Role_Type role) const
{
	if (m_pObject == NULL || m_pObject->getBaseType() != ot_Association)
		return m_role[role].m_Changeability;
	UMLAssociation *umla = static_cast<UMLAssociation*>(m_pObject);
	return umla->getChangeability(role);
}

void AssociationWidget::setChangeability (Changeability_Type value, Role_Type role)
{
	if (value == getChangeability(role))
		return;
	QString changeString = UMLAssociation::ChangeabilityToString(value);
	if (m_pObject && m_pObject->getBaseType() == ot_Association)  // update our model object
		getAssociation()->setChangeability(value, role);
	m_role[role].m_Changeability = value;
	// update our string representation
	setChangeWidget(changeString, role);
}

void AssociationWidget::setChangeWidget(const QString &strChangeWidget, Role_Type role) {
	bool newLabel = false;
	Text_Role tr = (role == A ? tr_ChangeA : tr_ChangeB);

	if(!m_role[role].m_pChangeWidget) {
		// Don't construct the FloatingText if the string is empty.
		if (strChangeWidget.isEmpty())
			return;

		newLabel = true;
		m_role[role].m_pChangeWidget = new FloatingText(m_pView, tr, strChangeWidget);
		m_role[role].m_pChangeWidget->setLink(this);
		m_pView->addWidget(m_role[role].m_pChangeWidget);
		m_role[role].m_pChangeWidget->setPreText("{"); // all types have this
		m_role[role].m_pChangeWidget->setPostText("}"); // all types have this
	} else {
		if (m_role[role].m_pChangeWidget->getText().isEmpty()) {
			newLabel = true;
		}
		m_role[role].m_pChangeWidget->setText(strChangeWidget);
	}
	m_role[role].m_pChangeWidget->setActivated();

	if (newLabel) {
		setTextPosition( tr );
	}

	if(FloatingText::isTextValid(m_role[role].m_pChangeWidget->getText()))
		m_role[role].m_pChangeWidget -> show();
	else
		m_role[role].m_pChangeWidget -> hide();
}

bool AssociationWidget::linePathStartsAt(const UMLWidget* widget) {
	QPoint lpStart = m_LinePath.getPoint(0);
	int startX = lpStart.x();
	int startY = lpStart.y();
	int wX = widget->getX();
	int wY = widget->getY();
	int wWidth = widget->getWidth();
	int wHeight = widget->getHeight();
	bool result = (startX >= wX && startX <= wX + wWidth &&
			startY >= wY && startY <= wY + wHeight);
	return result;
}

void AssociationWidget::setText(FloatingText *ft, const QString &text) {
	Uml::Text_Role role = ft->getRole();
	switch (role) {
		case tr_Name:
			setName(text);
			break;
		case tr_RoleAName:
			setRoleName(text, A);
			break;
		case tr_RoleBName:
			setRoleName(text, B);
			break;
		case tr_MultiA:
			setMulti(text, A);
			break;
		case tr_MultiB:
			setMulti(text, B);
			break;
		default:
			break;
	}
}

bool AssociationWidget::activate() {
	if(isActivated())
		return true;

	bool status = true;
	Association_Type type = getAssocType();

	if (m_role[A].m_pWidget == NULL)
		setWidget(m_pView->findWidget(getWidgetID(A)), A);
	if (m_role[B].m_pWidget == NULL)
		setWidget(m_pView->findWidget(getWidgetID(B)), B);

	if(!m_role[A].m_pWidget || !m_role[B].m_pWidget) {
		kdDebug() << "Can't make association" << endl;
		return false;
	}

	calculateEndingPoints();
	m_LinePath.activate();

	if (AssocRules::allowRole(type)) {
		for (unsigned r = A; r <= B; r++) {
			WidgetRole& robj = m_role[r];
			if (robj.m_pRole == NULL)
				continue;
			robj.m_pRole->setLink(this);
			Text_Role tr = (r == A ? tr_RoleAName : tr_RoleBName);
			robj.m_pRole->setRole(tr);
			Scope scope = getVisibility((Role_Type)r);
			robj.m_pRole->setPreText(Umbrello::scopeToString(scope));

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
		m_pName->setLink(this);
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
			pMulti->setLink(this);
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
			pChangeWidget->setLink(this);
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

UMLWidget* AssociationWidget::getWidget(Role_Type role) {
	return m_role[role].m_pWidget;
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
	setWidget(widgetA, A);
	setAssocType(assocType);
	setWidget(widgetB, B);

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
		updateAssociations(m_role[A].m_nTotalCount - 1, m_role[A].m_WidgetRegion, A);
	if(m_role[B].m_nTotalCount > 2)
		updateAssociations(m_role[B].m_nTotalCount - 1, m_role[B].m_WidgetRegion, B);

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

	if (m_pObject && m_pObject->getBaseType() == ot_Association) {
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
	if (m_pObject && m_pObject->getBaseType() == ot_Association) {
		UMLAssociation *umla = getAssociation();

		// safety check. Did some num-nuts try to set the existing
		// association again? If so, just bail here
		if (assoc && umla == assoc)
			return;

		umla->disconnect(this);
		umla->nrof_parent_widgets--;

		// we are the last "owner" of this association, so delete it
		// from the parent UMLDoc, and as a stand-alone
		//DISCUSS: Should we really do this?
		//    It implies that an association's existence is ONLY
		//    governed by its existence on at least one diagram.
		//    OTOH, it might be argued that an association should
		//    further exist even when it's (temporarily) not present
		//    on any diagram. This is exactly what cut and paste
		//    relies on (at least the way it's implemented now)
		// ANSWER: yes, we *should* do this.
		//   This only implies that IF an association once 'belonged'
 		//   to one or more parent associationwidgets, then it must 'die' when the
		//   last widget does. UMLAssociations which never had a parent
		//   in the first place wont be affected by this code, and can happily
		//   live on without a parent.
		//DISCUSS: Sorry Brian, but this breaks cut/paste.
		//    In particular, cut/paste means that the UMLAssociation _does_
		//    have the assocwidget parent - the only means of doing a cut/paste
		//    on the diagram is via the widgets. I.e. in practice there is no
		//    such thing as an "orphan" UMLAssociation.
		//    BTW, IMHO the concept of a widget being the parent of a UML object
		//    is fundamentally flawed. Widgets are pure presentation - they can
		//    come and go at a whim. If at all, the widgets could be considered
		//    children of the corresponding UML object.
		//
		// ANSWER: This is the wrong treatment of cut and paste. Associations that
		// are being cut/n pasted should be serialized to XMI, then reconstituted
		// (IF a paste operation) rather than passing around object pointers. Its
		// just too hard otherwise to prevent problems in the code. Bottom line: we need to
		// delete orphaned associations or we well get code crashes and memory leaks.
		if (umla->nrof_parent_widgets == 0) {
			//umla->deleteLater();
		}

		m_pObject = NULL;
	}

	if(assoc) {
		m_pObject = assoc;

		// move counter to "0" from "-1" (which means, no assocwidgets)
		if(assoc->nrof_parent_widgets < 0)
			assoc->nrof_parent_widgets = 0;

		assoc->nrof_parent_widgets++;
		connect(assoc, SIGNAL(modified()), this, SLOT(syncToModel()));
	}

}


/** Returns true if the Widget is either at the starting or ending side of the association */
bool AssociationWidget::contains(UMLWidget* widget) {
	return (widget == m_role[A].m_pWidget || widget == m_role[B].m_pWidget);
}

Association_Type AssociationWidget::getAssocType() const {
	if (m_pObject == NULL || m_pObject->getBaseType() != ot_Association)
		return m_AssocType;
	UMLAssociation *umla = static_cast<UMLAssociation*>(m_pObject);
	return umla->getAssocType();
}

/** Sets the association's type */
void AssociationWidget::setAssocType(Association_Type type) {
	if (m_pObject && m_pObject->getBaseType() == ot_Association)
		getAssociation()->setAssocType(type);
	m_AssocType = type;
	m_LinePath.setAssocType(type);
	// If the association new type is not supposed to have Multiplicity
	// FloatingTexts and a Role FloatingText then set the internal
	// floating text pointers to null.
	if( !AssocRules::allowMultiplicity(type, getWidget(A)->getBaseType()) ) {
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
		setRoleDoc("", A);
		setRoleDoc("", B);
	}
}

Uml::IDType AssociationWidget::getWidgetID(Role_Type role) const {
	if (m_role[role].m_pWidget == NULL) {
		if (m_pObject && m_pObject->getBaseType() == ot_Association) {
			UMLAssociation *umla = static_cast<UMLAssociation*>(m_pObject);
			return umla->getObjectId(role);
		}
		kdError() << "AssociationWidget::getWidgetID(): m_pWidget is NULL" << endl;
		return Uml::id_None;
	}
	if (m_role[role].m_pWidget->getBaseType() == Uml::wt_Object)
		return static_cast<ObjectWidget*>(m_role[role].m_pWidget)->getLocalID();
	Uml::IDType id = m_role[role].m_pWidget->getID();
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
	string.append( UMLAssociation::typeAsString(getAssocType()) );
	string.append(":");
	if(m_role[B].m_pWidget) {
		string += m_role[B].m_pWidget -> getName();
	}

	string.append(":");
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
			m_LinePath.insertPoint( i + 1, me -> pos() );
		} else {
			/* deselect the line path */
			m_LinePath.setSelected( false );

			/* there was a point so we remove the point */
			m_LinePath.removePoint(i, me -> pos(), POINT_DELTA );

			/* select the line path */
			m_LinePath.setSelected( true );
		}

		m_LinePath.update();

		calculateNameTextSegment();
		m_umldoc->setModified(true);
	}
}

void AssociationWidget::moveEvent(QMoveEvent* me) {
	// 2004-04-30: Achim Spangler
	// Simple Approach to block moveEvent during load of
	// XMI
	/// @todo avoid trigger of this event during load
	if ( m_umldoc->loading() ) {
		// hmmh - change of position during load of XMI
		// -> there is something wrong
		// -> avoid movement during opening
		// -> print warn and stay at old position
		kdWarning() << "AssociationWidget::moveEvent() called during load of XMI for ViewType: " << m_pView -> getType()
			<< ", and BaseType: " << getBaseType()
			<< endl;
		return;
	}
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
		setTextPositionRelatively(tr_ChangeA, oldChangeAPoint);
	}
	if (m_role[B].m_pChangeWidget && (m_nMovingPoint == 1)) {
		setTextPositionRelatively(tr_ChangeB, oldChangeBPoint);
	}
	if (m_role[A].m_pMulti && (m_nMovingPoint == 1)) {
		setTextPositionRelatively(tr_MultiA, oldMultiAPoint);
	}
	if (m_role[B].m_pMulti && (m_nMovingPoint == pos-1)) {
		setTextPositionRelatively(tr_MultiB, oldMultiBPoint);
	}

	if (m_pName) {
		if(m_nMovingPoint == (int)m_unNameLineSegment ||
		   m_nMovingPoint - 1 == (int)m_unNameLineSegment) {
			setTextPositionRelatively(tr_Name, oldNamePoint);
		}
	}

	if (m_role[A].m_pRole) {
		setTextPositionRelatively(tr_RoleAName, oldRoleAPoint);
	}
	if (m_role[B].m_pRole) {
		setTextPositionRelatively(tr_RoleBName, oldRoleBPoint);
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
	 *                              Region 2
	 *                         \                /
	 *                           \            /
	 *                             +--------+
	 *                             | \    / |
	 *                Region 1     |   ><   |    Region 3
	 *                             | /    \ |
	 *                             +--------+
	 *                           /            \
	 *                         /                \
	 *                              Region 4
	 *
	 * Each diagonal is defined by two corners of the bounding rectangle
	 *
	 * To calculate the first point in the LinePath we have to find out in which
	 * Region (defined by WidgetA's diagonals) is WidgetB's center
	 * (let's call it Region M.) After that the first point will be the middle
	 * point of the rectangle's side contained in Region M.
	 *
	 * To calculate the last point in the LinePath we repeat the above but
	 * in the opposite direction (from widgetB to WidgetA)
	 */

	UMLWidget *pWidgetA = m_role[A].m_pWidget;
	UMLWidget *pWidgetB = m_role[B].m_pWidget;
	if (!pWidgetA || !pWidgetB)
		return;
	m_role[A].m_OldCorner.setX( pWidgetA->getX() );
	m_role[A].m_OldCorner.setY( pWidgetA->getY() );
	m_role[B].m_OldCorner.setX( pWidgetB->getX() );
	m_role[B].m_OldCorner.setY( pWidgetB->getY() );
	uint size = m_LinePath.count();
	if(size < 2)
		m_LinePath.setStartEndPoints( m_role[A].m_OldCorner, m_role[B].m_OldCorner );

	// See if an association to self.
	// See if it needs to be set up before we continue:
	// If self association/message and doesn't have the minimum 4 points
	// then create it.  Make sure no points are out of bounds of viewing area.
	// This only happens on first time through that we are worried about.
	if (pWidgetA == pWidgetB && size < 4) {
		const int DISTANCE = 50;
		int x = pWidgetA -> getX();
		int y = pWidgetA -> getY();
		int h = pWidgetA -> getHeight();
		int w = pWidgetA -> getWidth();
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

	// If the line has more than one segment change the values to calculate
	// from widget to point 1.
	int xB = pWidgetB->getX() + pWidgetB->getWidth() / 2;
	int yB = pWidgetB->getY() + pWidgetB->getHeight() / 2;
	if( size > 2 ) {
		QPoint p = m_LinePath.getPoint( 1 );
		xB = p.x();
		yB = p.y();
	}
	doUpdates(xB, yB, A);

	// Now do the same for widgetB.
	// If the line has more than one segment change the values to calculate
	// from widgetB to the last point away from it.
	int xA = pWidgetA->getX() + pWidgetA->getWidth() / 2;
	int yA = pWidgetA->getY() + pWidgetA->getHeight() / 2;
	if (size > 2 ) {
		QPoint p = m_LinePath.getPoint( size - 2 );
		xA = p.x();
		yA = p.y();
	}
	doUpdates( xA, yA, B );

	computeAssocClassLine();
}

void AssociationWidget::doUpdates(int otherX, int otherY, Role_Type role) {
	// Find widget region.
	Region oldRegion = m_role[role].m_WidgetRegion;
	UMLWidget *pWidget = m_role[role].m_pWidget;
	QRect rc(pWidget->getX(), pWidget->getY(),
		 pWidget->getWidth(), pWidget->getHeight());
	Region& region = m_role[role].m_WidgetRegion;  // alias for brevity
	region = findPointRegion( rc, otherX, otherY);
	// Move some regions to the standard ones.
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
	int regionCount = getRegionCount(region, role) + 2;//+2 = (1 for this one and one to halve it)
	int totalCount = m_role[role].m_nTotalCount;
	if( oldRegion != region ) {
		updateRegionLineCount( regionCount - 1, regionCount, region, role );
		updateAssociations( totalCount - 1, oldRegion, role );
	} else if( totalCount != regionCount ) {
		updateRegionLineCount( regionCount - 1, regionCount, region, role );
	} else {
		updateRegionLineCount( m_role[role].m_nIndex, totalCount, region, role );
	}
	updateAssociations( regionCount, region, role );
}

/** Read property of bool m_bActivated. */
const bool AssociationWidget::isActivated() {
	return m_bActivated;
}

/** Set the m_bActivated flag of a widget but does not perform the Activate method */
void AssociationWidget::setActivated(bool active /*=true*/) {
	m_bActivated = active;
}

void AssociationWidget::syncToModel()
{
	UMLAssociation *uml = getAssociation();

	if (uml == NULL) {
		return;
	}
	// block signals until finished
	uml->blockSignals(true);

	setName(uml->getName());
	setRoleName(uml->getRoleName(A), A);
	setRoleName(uml->getRoleName(B), B);
	setVisibility(uml->getVisibility(A), A);
	setVisibility(uml->getVisibility(B), B);
	setChangeability(uml->getChangeability(A), A);
	setChangeability(uml->getChangeability(B), B);
	setMulti(uml->getMulti(A), A);
	setMulti(uml->getMulti(B), B);

	uml->blockSignals(false);
}

// this will synchronize UMLAssociation w/ this new Widget
void AssociationWidget::mergeAssociationDataIntoUMLRepresentation()
{

	UMLAssociation *uml = getAssociation();
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

	text = getRoleWidget(A);
	if (text)
		uml->setRoleName(text->getText(), A);

	text = getRoleWidget(B);
	if (text)
		uml->setRoleName(text->getText(), B);

	text = getMultiWidget(A);
	if (text)
		uml->setMulti(text->getText(), A);

	text = getMultiWidget(B);
	if (text)
		uml->setMulti(text->getText(), B);

	// unblock
	uml->blockSignals(false);
}

/** Adjusts the ending point of the association that connects to Widget */
void AssociationWidget::widgetMoved(UMLWidget* widget, int x, int y ) {
	// 2004-04-30: Achim Spangler
	// Simple Approach to block moveEvent during load of
	// XMI
	/// @todo avoid trigger of this event during load
	if ( m_umldoc->loading() ) {
		// hmmh - change of position during load of XMI
		// -> there is something wrong
		// -> avoid movement during opening
		// -> print warn and stay at old position
		kdDebug() << "AssociationWidget::widgetMoved() called during load of XMI for ViewType: " << m_pView -> getType()
			<< ", and BaseType: " << getBaseType()
			<< endl;
		return;
	}
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

	// Assoc to self - move all points:
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
			setTextPositionRelatively(tr_Name, oldNamePoint);
		}

	}//end if widgetA = widgetB
	else if (m_role[A].m_pWidget==widget) {
		if (m_pName && m_unNameLineSegment == 0 && !m_pName->getSelected() ) {
			//only calculate position and move text if the segment it is on is moving
			setTextPositionRelatively(tr_Name, oldNamePoint);
		}
	}//end if widgetA moved
	else if (m_role[B].m_pWidget==widget) {
		if (m_pName && (m_unNameLineSegment == pos-1) && !m_pName->getSelected() ) {
			//only calculate position and move text if the segment it is on is moving
			setTextPositionRelatively(tr_Name, oldNamePoint);
		}
	}//end if widgetB moved

	if ( m_role[A].m_pRole && !m_role[A].m_pRole->getSelected() ) {
		setTextPositionRelatively(tr_RoleAName, oldRoleAPoint);
	}
	if ( m_role[B].m_pRole && !m_role[B].m_pRole->getSelected() ) {
		setTextPositionRelatively(tr_RoleBName, oldRoleBPoint);
	}
	if ( m_role[A].m_pMulti && !m_role[A].m_pMulti->getSelected() ) {
		setTextPositionRelatively(tr_MultiA, oldMultiAPoint);
	}
	if ( m_role[B].m_pMulti && !m_role[B].m_pMulti->getSelected() ) {
		setTextPositionRelatively(tr_MultiB, oldMultiBPoint);
	}
	if ( m_role[A].m_pChangeWidget && !m_role[A].m_pChangeWidget->getSelected() ) {
		setTextPositionRelatively(tr_ChangeA, oldChangeAPoint);
	}
	if ( m_role[B].m_pChangeWidget && !m_role[B].m_pChangeWidget->getSelected() ) {
		setTextPositionRelatively(tr_ChangeB, oldChangeBPoint);
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
    8 = On diagonal 1 between Region 4 and 1
    9 = On diagonal 1 and On diagonal 2 (the center)
*/
AssociationWidget::Region AssociationWidget::findPointRegion(QRect Rect, int PosX, int PosY) {
	float w = (float)Rect.width();
	float h = (float)Rect.height();
	float x = (float)Rect.x();
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

QPoint AssociationWidget::swapXY(const QPoint &p) {
	QPoint swapped( p.y(), p.x() );
	return swapped;
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
QPoint AssociationWidget::calculatePointAtDistance(const QPoint &P1, const QPoint &P2, float Distance) {
	/*
	  the distance D between points (x1, y1) and (x3, y3) has the following formula:
	      ---     ------------------------------
	  D =    \   /	   2	     2
		  \ /   (x3 - x1)  +  (y3 - y1)

	  D, x1 and y1 are known and the point (x3, y3) is inside line (x1,y1)(x2,y2), so if the
	  that line has the formula y = mx + b
	  then y3 = m*x3 + b

	   2	     2	     2
	  D   = (x3 - x1)  +  (y3 - y1)

	   2       2		 2      2		 2
	  D    = x3    - 2*x3*x1 + x1   + y3   - 2*y3*y1  + y1

	   2       2       2       2		  2
	  D    - x1    - y1    = x3    - 2*x3*x1  + y3   - 2*y3*y1

	   2       2       2       2			  2
	  D    - x1    - y1    = x3    - 2*x3*x1  + (m*x3 + b)  - 2*(m*x3 + b)*y1

	   2       2       2	      2       2	2
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
QPoint AssociationWidget::calculatePointAtDistanceOnPerpendicular(const QPoint &P1, const QPoint &P2, float Distance) {
	/*
	  the distance D between points (x2, y2) and (x3, y3) has the following formula:

	       ---     ------------------------------
	  D =     \   /	   2	     2
		   \ /   (x3 - x2)  +  (y3 - y2)

	  D, x2 and y2 are known and line P2P3 is perpendicular to line (x1,y1)(x2,y2), so if the
	  line P1P2 has the formula y = m*x + b,
	  then	  (x1 - x2)
		  m =  -----------    , because it is perpendicular to line P1P2
			(y2 - y1)

	  also y2 = m*x2 + b
	  => b = y2 - m*x2

	  then P3 = (x3, m*x3 + b)

	   2	     2	    2
	  D  = (x3 - x2)  + (y3 - y2)

	   2     2		 2      2		 2
	  D  = x3    - 2*x3*x2 + x2   + y3   - 2*y3*y2  + y2

	   2       2       2       2		  2
	  D    - x2    - y2    = x3    - 2*x3*x2  + y3   - 2*y3*y2



	   2       2       2       2			  2
	  D    - x2    - y2    = x3    - 2*x3*x2  + (m*x3 + b)  - 2*(m*x3 + b)*y2

	   2       2       2		2       2	2
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
		text = getMultiWidget(A);
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
		text = getMultiWidget(B);
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

		text = getChangeWidget(A);

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

		text = getChangeWidget(B);
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

		text = getRoleWidget(A);
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
		text = getRoleWidget(B);
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
	//sum of length(PTP1) and length(PTP2)
	float total_length = 0;
	float smallest_length = 0;
	for(uint i = 0; i < size - 1; i++) {
		QPoint pi = m_LinePath.getPoint( i );
		QPoint pj = m_LinePath.getPoint( i+1 );
		int xtiDiff = xt - pi.x();
		int xtjDiff = xt - pj.x();
		int ytiDiff = yt - pi.y();
		int ytjDiff = yt - pj.y();
		total_length =  sqrt( double(xtiDiff * xtiDiff + ytiDiff * ytiDiff) )
			      + sqrt( double(xtjDiff * xtjDiff + ytjDiff * ytjDiff) );
		//this gives the closest point
		if( total_length < smallest_length || i == 0) {
			smallest_length = total_length;
			m_unNameLineSegment = i;
		}
	}
}

FloatingText* AssociationWidget::floatingText(Text_Role role) {
	FloatingText *ft = NULL;
	switch(role) {
		case tr_MultiA:
			ft = m_role[A].m_pMulti;
			break;
		case tr_MultiB:
			ft = m_role[B].m_pMulti;
			break;
		case tr_Name:
		case tr_Coll_Message:
			ft = m_pName;
			break;
		case tr_RoleAName:
			ft = m_role[A].m_pRole;
			break;
		case tr_RoleBName:
			ft = m_role[B].m_pRole;
			break;
		case tr_ChangeA:
			ft = m_role[A].m_pChangeWidget;
			break;
		case tr_ChangeB:
			ft = m_role[B].m_pChangeWidget;
			break;
		default:
			break;
	}
	return ft;
}

void AssociationWidget::setTextPosition(Text_Role role) {
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
	FloatingText *ft = floatingText(role);
	if (ft == NULL)
		return;
	QPoint pos = calculateTextPosition(role);
	int x = pos.x();
	int y = pos.y();
	if ( (x < 0 || x > FloatingText::restrictPositionMax) ||
	     (y < 0 || y > FloatingText::restrictPositionMax) ) {
		kdDebug() << "AssociationWidget::setTextPosition( " << x << " , " << y << " ) "
			<< "- was blocked because at least one value is out of bounds: ["
			<< "0 ... " << FloatingText::restrictPositionMax << "]"
			<< endl;
		return;
	}
	ft->setX( x );
	ft->setY( y );
}

void AssociationWidget::setTextPositionRelatively(Text_Role role, const QPoint &oldPosition) {
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
	FloatingText *ft = floatingText(role);
	if (ft == NULL)
		return;
	int ftX = ft->getX();
	int ftY = ft->getY();
	if ( (ftX < 0 || ftX > FloatingText::restrictPositionMax) ||
	     (ftY < 0 || ftY > FloatingText::restrictPositionMax) ) {
		kdDebug() << "AssociationWidget::setTextPositionRelatively: "
			<< "blocked because the FloatingText original position ("
			<< ftX << "," << ftY << " is out of bounds: [0 ... "
			<< FloatingText::restrictPositionMax << "]" << endl;
		return;
	}
	QPoint pos = calculateTextPosition(role);
	int relX = pos.x() - oldPosition.x();
	int relY = pos.y() - oldPosition.y();
	int ftNewX = ftX + relX;
	int ftNewY = ftY + relY;
	if ( (ftNewX < 0 || ftNewX > FloatingText::restrictPositionMax) ||
	     (ftNewY < 0 || ftNewY > FloatingText::restrictPositionMax) ) {
		kdDebug() << "AssociationWidget::setTextPositionRelatively: "
			<< "blocked because the FloatingText new position ("
			<< ftNewX << "," << ftNewY << " is out of bounds: [0 ... "
			<< FloatingText::restrictPositionMax << "]" << endl;
		return;
	}
	bool oldIgnoreSnapToGrid = ft->getIgnoreSnapToGrid();
	ft->setIgnoreSnapToGrid( true );
	ft->setX( ftNewX );
	ft->setY( ftNewY );
	ft->setIgnoreSnapToGrid( oldIgnoreSnapToGrid );
}

void AssociationWidget::computeAssocClassLine() {
	if (m_pAssocClassWidget == NULL || m_pAssocClassLine == NULL)
		return;
	if (m_nLinePathSegmentIndex < 0) {
		kdError() << "AssociationWidget::computeAssocClassLine: "
			  << "m_nLinePathSegmentIndex is not set" << endl;
		return;
	}
	QPoint segStart = m_LinePath.getPoint(m_nLinePathSegmentIndex);
	QPoint segEnd = m_LinePath.getPoint(m_nLinePathSegmentIndex + 1);
	const int midSegX = segStart.x() + (segEnd.x() - segStart.x()) / 2;
	const int midSegY = segStart.y() + (segEnd.y() - segStart.y()) / 2;

	QPoint segmentMidPoint(midSegX, midSegY);
	QRect classRectangle = m_pAssocClassWidget->rect();
	QPoint cwEdgePoint = findIntercept(classRectangle, segmentMidPoint);
	int acwMinX = cwEdgePoint.x();
	int acwMinY = cwEdgePoint.y();

	m_pAssocClassLine->setPoints(midSegX, midSegY, acwMinX, acwMinY);
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
	const QPoint p = me->pos();
	if (me->button() == LeftButton) {
		UMLWidget *otherWidget = m_pView->getFirstSelectedWidget();
		if (otherWidget == NULL || otherWidget->getBaseType() != Uml::wt_Class)
			return;
		m_nLinePathSegmentIndex = m_LinePath.onLinePath(p);
		if (m_nLinePathSegmentIndex < 0)
			return;
		m_pAssocClassWidget = static_cast<ClassWidget*>(otherWidget);
		m_pAssocClassWidget->setClassAssocWidget(this);
		if (m_pAssocClassLine)
			delete m_pAssocClassLine;
		m_pAssocClassLine = new QCanvasLine(m_pView->canvas());
		computeAssocClassLine();
		QPen pen(m_pView->getLineColor(), m_pView->getLineWidth(), DashLine);
		m_pAssocClassLine->setPen(pen);
		m_pAssocClassLine->setVisible(true);
		return;
	}
	// right button action:
	//work out the type of menu we want
	//work out if the association allows rolenames, multiplicity, etc
	//also must be within a certain distance to be a multiplicity menu
	ListPopupMenu::Menu_Type menuType = ListPopupMenu::mt_Undefined;
	int pos = m_LinePath.count() - 1;
	int DISTANCE = 40;//must be within this many pixels for it to be a multi menu
	QPoint lpStart = m_LinePath.getPoint(0);
	QPoint lpEnd = m_LinePath.getPoint(pos);
	int startXDiff = lpStart.x() - p.x();
	int startYDiff = lpStart.y() - p.y();
	int endXDiff = lpEnd.x() - p.x();
	int endYDiff = lpEnd.y() - p.y();
	float lengthMAP = sqrt( double(startXDiff * startXDiff + startYDiff * startYDiff) );
	float lengthMBP = sqrt( double(endXDiff * endXDiff + endYDiff * endYDiff) );
	Association_Type type = getAssocType();
	//allow multiplicity
	if( AssocRules::allowMultiplicity( type, getWidget(A) -> getBaseType() ) ) {
		if(lengthMAP < DISTANCE)
			menuType =  ListPopupMenu::mt_MultiA;
		else if(lengthMBP < DISTANCE)
			menuType = ListPopupMenu::mt_MultiB;
	}
	if( menuType == ListPopupMenu::mt_Undefined ) {
		if( type == at_Anchor )
			menuType = ListPopupMenu::mt_Anchor;
		else if( type == at_Coll_Message )
			menuType = ListPopupMenu::mt_Collaboration_Message;
		else if( AssocRules::allowRole( type ) )
			menuType = ListPopupMenu::mt_FullAssociation;
		else
			menuType = ListPopupMenu::mt_Association_Selected;
	}
	m_pMenu = new ListPopupMenu(m_pView, menuType);
	m_pMenu->popup(me -> globalPos());
	connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuSelection(int)));
	setSelected();
}//end method mouseReleaseEvent

bool AssociationWidget::showDialog() {
	AssocPropDlg dlg(static_cast<QWidget*>(m_pView), this );
	if (! dlg.exec())
		return false;
	QString name = getName();
	QString doc = getDoc();
	QString roleADoc = getRoleDoc(A), roleBDoc = getRoleDoc(B);
	QString rnA = getRoleName(A), rnB = getRoleName(B);
	QString ma = getMulti(A), mb = getMulti(B);
	Scope vA = getVisibility(A), vB = getVisibility(B);
	Changeability_Type cA = getChangeability(A), cB = getChangeability(B);
	//rules built into these functions to stop updating incorrect values
	setName(name);
	setRoleName(rnA, A);
	setRoleName(rnB, B);
	setDoc(doc);
	setRoleDoc(roleADoc, A);
	setRoleDoc(roleBDoc, B);
	setMulti(ma, A);
	setMulti(mb, B);
	setVisibility(vA, A);
	setVisibility(vB, B);
	setChangeability(cA, A);
	setChangeability(cB, B);
	m_pView -> showDocumentation( this, true );
	return true;
}

void AssociationWidget::slotMenuSelection(int sel) {
	QString oldText, newText;
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
			kdDebug() << "AssociationWidget::slotMenuSelection(mt_Properties): "
				  << "assoctype is " << atype << endl;
		} else {  //standard assoc dialog
			m_pView -> updateDocumentation( false );
			showDialog();
		}
		break;

	case ListPopupMenu::mt_Delete:
		m_pView->removeAssocInViewAndDoc(this);
		break;

	case ListPopupMenu::mt_Rename_MultiA:
		if(m_role[A].m_pMulti)
			oldText = m_role[A].m_pMulti -> getText();
		else
			oldText = "";
		newText = KInputDialog::getText(i18n("Multiplicity"), i18n("Enter multiplicity:") , oldText, 0, m_pView);
		if ( newText != oldText && FloatingText::isTextValid(newText) ) {
			setMulti(newText, A);
		}
		break;


	case ListPopupMenu::mt_Rename_MultiB:
		if(m_role[B].m_pMulti)
			oldText = m_role[B].m_pMulti -> getText();
		else
			oldText = "";
		newText = KInputDialog::getText(i18n("Multiplicity"), i18n("Enter multiplicity:"), oldText, 0, m_pView);
		if ( newText != oldText && FloatingText::isTextValid(newText) ) {
			setMulti(newText, B);
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
			setRoleName(newText, A);
		}
		break;

	case ListPopupMenu::mt_Rename_RoleBName:
		if(m_role[B].m_pRole)
			oldText = m_role[B].m_pRole -> getText();
		else
			oldText = "";
		newText = KInputDialog::getText(i18n("Role Name"), i18n("Enter role name:"), oldText, 0, m_pView);
		if ( newText != oldText && FloatingText::isTextValid(newText) ) {
			setRoleName(newText, B);
		}
		break;

	case ListPopupMenu::mt_Change_Font:
		font = getFont();
		if( KFontDialog::getFont( font, false, m_pView ) )
			lwSetFont(font);
		break;

	case ListPopupMenu::mt_Change_Font_Selection:
		font = getFont();
		if( KFontDialog::getFont( font, false, m_pView ) ) {
			m_pView -> selectionSetFont( font );
			m_umldoc->setModified(true);
		}
		break;

	case ListPopupMenu::mt_Cut:
		m_pView->setStartedCut();
		UMLApp::app()->slotEditCut();
		break;

	case ListPopupMenu::mt_Copy:
		UMLApp::app()->slotEditCopy();
		break;

	case ListPopupMenu::mt_Paste:
		UMLApp::app()->slotEditPaste();
		break;

	case ListPopupMenu::mt_Reset_Label_Positions:
		resetTextPositions();
		break;
	}//end switch
}


void AssociationWidget::lwSetFont (QFont font) {
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

void AssociationWidget::checkPoints(const QPoint &p) {
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
	if(widget -> getWidget(A) == m_role[A].m_pWidget)
		return m_role[A].m_WidgetRegion;
	if(widget -> getWidget(B) == m_role[B].m_pWidget)
		return m_role[B].m_WidgetRegion;
	return Error;
}

int AssociationWidget::getRegionCount(AssociationWidget::Region region, Role_Type role) {
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
		const WidgetRole& otherA = assocwidget->m_role[A];
		const WidgetRole& otherB = assocwidget->m_role[B];
		const UMLWidget *a = otherA.m_pWidget;
		const UMLWidget *b = otherB.m_pWidget;
		/*
		//don't count associations to self if both of their end points are on the same region
		//they are different and placement won't interfere with them
		if( a == b && otherA.m_WidgetRegion == otherB.m_WidgetRegion )
			continue;
		 */
		if (m_role[role].m_pWidget == a && region == otherA.m_WidgetRegion)
			widgetCount++;
		else if (m_role[role].m_pWidget == b && region == otherB.m_WidgetRegion)
			widgetCount++;
	}//end while
	return widgetCount;
}

QPoint AssociationWidget::findIntercept(const QRect &rect, const QPoint &point) {
	Region region = findPointRegion(rect, point.x(), point.y());
	/*
	const char *regionStr[] = { "Error",
		"West", "North", "East", "South",
		"NorthWest", "NorthEast", "SouthEast", "SouthWest",
		"Center"
	};
	kdDebug() << "findPointRegion(rect(" << rect.x() << "," << rect.y()
		  << "," << rect.width() << "," << rect.height() << "), p("
		  << point.x() << "," << point.y() << ")) = " << regionStr[region]
		  << endl;
	 */
	// Move some regions to the standard ones.
	switch (region) {
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
	// The Qt coordinate system has (0,0) in the top left corner.
	// In order to go to the regular XY coordinate system with (0,0)
	// in the bottom left corner, we swap the X and Y axis.
	// That's why the following assignments look twisted.
	const int rectHalfWidth = rect.height() / 2;
	const int rectHalfHeight = rect.width() / 2;
	const int rectMidX = rect.y() + rectHalfWidth;
	const int rectMidY = rect.x() + rectHalfHeight;
	const int pX = point.y();
	const int pY = point.x();
	const int dX = rectMidX - pX;
	const int dY = rectMidY - pY;
	switch (region) {
		case West:
			region = South;
			break;
		case North:
			region = East;
			break;
		case East:
			region = North;
			break;
		case South:
			region = West;
			break;
		default:
			break;
	}
	// Now we have regular coordinates with the point (0,0) in the
	// bottom left corner.
	if (region == North || region == South) {
		int yoff = rectHalfHeight;
		if (region == North)
			yoff = -yoff;
		if (dX == 0) {
			return QPoint(rectMidY + yoff, rectMidX);  // swap back X and Y
		}
		if (dY == 0) {
			kdError() << "AssociationWidget::findIntercept usage error: "
				  << "North/South (dY == 0)" << endl;
			return QPoint(0,0);
		}
		const float m = (float)dY / (float)dX;
		const float b = (float)pY - m * pX;
		const int inputY = rectMidY + yoff;
		const float outputX = ((float)inputY - b) / m;
		return QPoint(inputY, (int)outputX);  // swap back X and Y
	} else {
		int xoff = rectHalfWidth;
		if (region == East)
			xoff = -xoff;
		if (dY == 0)
			return QPoint(rectMidY, rectMidX + xoff);  // swap back X and Y
		if (dX == 0) {
			kdError() << "AssociationWidget::findIntercept usage error: "
				  << "East/West (dX == 0)" << endl;
			return QPoint(0,0);
		}
		const float m = (float)dY / (float)dX;
		const float b = (float)pY - m * pX;
		const int inputX = rectMidX + xoff;
		const float outputY = m * (float)inputX + b;
		return QPoint((int)outputY, inputX);  // swap back X and Y
	}
}

int AssociationWidget::findInterceptOnEdge(const QRect &rect,
					   AssociationWidget::Region region,
					   const QPoint &point)
{
	// The Qt coordinate system has (0,0) in the top left corner.
	// In order to go to the regular XY coordinate system with (0,0)
	// in the bottom left corner, we swap the X and Y axis.
	// That's why the following assignments look twisted.
	const int rectHalfWidth = rect.height() / 2;
	const int rectHalfHeight = rect.width() / 2;
	const int rectMidX = rect.y() + rectHalfWidth;
	const int rectMidY = rect.x() + rectHalfHeight;
	const int dX = rectMidX - point.y();
	const int dY = rectMidY - point.x();
	switch (region) {
		case West:
			region = South;
			break;
		case North:
			region = West;
			break;
		case East:
			region = North;
			break;
		case South:
			region = East;
			break;
		default:
			break;
	}
	// Now we have regular coordinates with the point (0,0) in the
	// bottom left corner.
	if (region == North || region == South) {
		if (dX == 0)
			return rectMidY;
			// should be rectMidX, but we go back to Qt coord.sys.
		if (dY == 0) {
			kdError() << "AssociationWidget::findInterceptOnEdge usage error: "
				  << "North/South (dY == 0)" << endl;
			return -1;
		}
		const float m = (float)dY / (float)dX;
		float relativeX;
		if (region == North)
			relativeX = (float)rectHalfHeight / m;
		else
			relativeX = -(float)rectHalfHeight / m;
		return (rectMidY + (int)relativeX);
		// should be rectMidX, but we go back to Qt coord.sys.
	} else {
		if (dY == 0)
			return rectMidX;
			// should be rectMidY, but we go back to Qt coord.sys.
		if (dX == 0) {
			kdError() << "AssociationWidget::findInterceptOnEdge usage error: "
				  << "East/West (dX == 0)" << endl;
			return -1;
		}
		const float m = (float)dY / (float)dX;
		float relativeY = m * (float)rectHalfWidth;
		if (region == West)
			relativeY = -relativeY;
		return (rectMidX + (int)relativeY);
		// should be rectMidY, but we go back to Qt coord.sys.
	}
}

void AssociationWidget::insertIntoLists(int position, const AssociationWidget* assoc)
{
	bool did_insertion = false;
	for (int index = 0; index < m_positions_len; index++) {
		if (position < m_positions[index]) {
			for (int moveback = m_positions_len; moveback > index; moveback--)
				m_positions[moveback] = m_positions[moveback - 1];
			m_positions[index] = position;
			m_ordered.insert(index, assoc);
			did_insertion = true;
			break;
		}
	}
	if (! did_insertion) {
		m_positions[m_positions_len] = position;
		m_ordered.append(assoc);
	}
	m_positions_len++;
}

void AssociationWidget::updateAssociations(int totalCount,
					   AssociationWidget::Region region,
					   Role_Type role)
{
	if( region == Error )
		return;
	AssociationWidgetList list = m_pView -> getAssociationList();
	AssociationWidgetListIt assoc_it(list);
	AssociationWidget* assocwidget = 0;
	UMLWidget *ownWidget = m_role[role].m_pWidget;
	m_positions_len = 0;
	m_ordered.clear();
	// we order the AssociationWidget list by region and x/y value
	while ( (assocwidget = assoc_it.current()) ) {
		++assoc_it;
		WidgetRole *roleA = &assocwidget->m_role[A];
		WidgetRole *roleB = &assocwidget->m_role[B];
		UMLWidget *wA = roleA->m_pWidget;
		UMLWidget *wB = roleB->m_pWidget;
		// Skip self associations.
		if (wA == wB)
			continue;
		// Now we must find out with which end the assocwidget connects
		// to the input widget (ownWidget).
		bool inWidgetARegion = ( ownWidget == wA &&
					 region == roleA->m_WidgetRegion );
		bool inWidgetBRegion = ( ownWidget == wB &&
					 region == roleB->m_WidgetRegion);
		if ( !inWidgetARegion && !inWidgetBRegion )
			continue;
		// Determine intercept position on the edge indicated by `region'.
		UMLWidget * otherWidget = (inWidgetARegion ? wB : wA);
		LinePath *linepath = assocwidget->getLinePath();
		QPoint refpoint;
		if (assocwidget->linePathStartsAt(otherWidget))
			refpoint = linepath->getPoint(linepath->count() - 2);
		else
			refpoint = linepath->getPoint(1);
		// The point is authoritative if we're called for the second time
		// (i.e. role==B) or it is a waypoint on the line path.
		bool pointIsAuthoritative = (role == B || linepath->count() > 2);
		if (! pointIsAuthoritative) {
			// If the point is not authoritative then we use the other
			// widget's center.
			refpoint.setX(otherWidget->getX() + otherWidget->getWidth() / 2);
			refpoint.setY(otherWidget->getY() + otherWidget->getHeight() / 2);
		}
		int intercept = findInterceptOnEdge(ownWidget->rect(), region, refpoint);
		if (intercept < 0) {
			kdDebug() << "updateAssociations: error from findInterceptOnEdge for"
				  << " assocType=" << assocwidget->getAssocType()
				  << " ownWidget=" << ownWidget->getName()
				  << " otherWidget=" << otherWidget->getName() << endl;
			continue;
		}
		insertIntoLists(intercept, assocwidget);
	} // while ( (assocwidget = assoc_it.current()) )

	// we now have an ordered list and we only have to call updateRegionLineCount
	int index = 1;
	for (assocwidget = m_ordered.first(); assocwidget; assocwidget = m_ordered.next()) {
		if (ownWidget == assocwidget->getWidget(A)) {
			assocwidget->updateRegionLineCount(index++, totalCount, region, A);
		} else if (ownWidget == assocwidget->getWidget(B)) {
			assocwidget->updateRegionLineCount(index++, totalCount, region, B);
		}
	} // for (assocwidget = ordered.first(); ...)
}

void AssociationWidget::updateRegionLineCount(int index, int totalCount,
					      AssociationWidget::Region region,
					      Role_Type role) {
	if( region == Error )
		return;
	// If the association is to self and the line ends are on the same region then
	// use a different calculation.
	if (m_role[A].m_pWidget == m_role[B].m_pWidget &&
	    m_role[A].m_WidgetRegion == m_role[B].m_WidgetRegion) {
		UMLWidget * pWidget = m_role[A].m_pWidget;
		int x = pWidget -> getX();
		int y = pWidget -> getY();
		int wh = pWidget -> height();
		int ww = pWidget -> width();
		int size = m_LinePath.count();
		// See if above widget ok to place assoc.
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

	WidgetRole& robj = m_role[role];
	UMLWidget * pWidget = robj.m_pWidget;

	// If a fork (ActivityWidget) for widget B then all associations should
	// meet in the middle.
	if (pWidget->getBaseType() == Uml::wt_Activity) {
		ActivityWidget *act = static_cast<ActivityWidget*>(pWidget);
		ActivityWidget::ActivityType atype = act->getActivityType();
		if (atype == ActivityWidget::Fork) {
			totalCount = 2;
			index = 1;
		} else if (atype == ActivityWidget::Initial ||
			   atype == ActivityWidget::End) {
			region = Center;
			totalCount = 2;
			index = 1;
		}
	}
	robj.m_nIndex = index;
	robj.m_nTotalCount = totalCount;
	int x = pWidget->getX();
	int y = pWidget->getY();
	robj.m_OldCorner.setX(x);
	robj.m_OldCorner.setY(y);
	int ww = pWidget->getWidth();
	int wh = pWidget->getHeight();
	int ch = wh * index / totalCount;
	int cw = ww * index / totalCount;
	int snapX = m_pView->snappedX(x + cw);
	int snapY = m_pView->snappedY(y + ch);

	QPoint pt;

	switch(region) {
	case West:
		pt.setX(x);
		pt.setY(snapY);
		break;
	case North:
		pt.setX(snapX);
		pt.setY(y);
		break;
	case East:
		pt.setX(x + ww);
		pt.setY(snapY);
		break;
	case South:
		pt.setX(snapX);
		pt.setY(y + wh);
		break;
	case Center:
		pt.setX(x + ww / 2);
		pt.setY(y + wh / 2);
		break;
	default:
		break;
	}
	if (role == A)
		m_LinePath.setPoint( 0, pt );
	else {
		m_LinePath.setPoint( m_LinePath.count() - 1, pt );
		LinePath::Region r = ( region == South || region == North ) ?
			LinePath::TopBottom : LinePath::LeftRight;
		m_LinePath.setDockRegion( r );
	}
}

void AssociationWidget::setSelected(bool _select /* = true */) {
	m_bSelected = _select;
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
	/**
	 * @fixme MYSTERY:
	 *        If the following code is activated and the m_LinePath.setSelected()
	 *        precedes it then the blue selection markers sometimes disappear
	 *        (notably on messages in collaboration diagrams.)
	//Update the docwindow for this association.
	// This is done last because each of the above setSelected calls
	// overwrites the docwindow, but we want the main association doc
	// to win.
	if( _select ) {
		if( m_pView -> getSelectCount() == 0 )
			m_pView -> showDocumentation( this, false );
	} else
		m_pView -> updateDocumentation( true );
	 */
	/** @fixme Had to pull the m_LinePath.setSelected() call down here
	 *         because otherwise the blue selection markers would not show
	 *         when selecting collaboration messages.
	 */
	m_LinePath.setSelected( _select );
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
		if (p.x() > rectangle.x() + rectangle.width()) {
			int newX = p.x() - rectangle.x() + pen_width;
			rectangle.setWidth(abs(newX));
		}
		if (p.y() > rectangle.y() + rectangle.height()) {
			int newY = p.y() - rectangle.y() + pen_width;
			rectangle.setHeight(abs(newY));
		}
	}
	return rectangle;
}


void AssociationWidget::init (UMLView *view)
{
	m_pView = view;  // pointer to parent viewwidget object
	m_Type = wt_Association;

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
	m_role[A].m_WidgetRegion = Error;
	m_role[B].m_WidgetRegion = Error;
	m_role[A].m_nIndex = 0;
	m_role[B].m_nIndex = 0;
	m_role[A].m_nTotalCount = 0;
	m_role[B].m_nTotalCount = 0;
	m_role[A].m_Visibility = Uml::Public;
	m_role[B].m_Visibility = Uml::Public;
	m_role[A].m_Changeability = Uml::chg_Changeable;
	m_role[B].m_Changeability = Uml::chg_Changeable;
	m_bActivated = false;
	m_unNameLineSegment = 0;
	m_pMenu = 0;
	m_bSelected = false;
	m_nMovingPoint = -1;
	m_nLinePathSegmentIndex = -1;
	m_pAssocClassLine = NULL;
	m_pAssocClassWidget = NULL;

	// Initialize local members.
	// These are only used if we don't have a UMLAssociation attached.
	m_role[A].m_Visibility = Public;
	m_role[B].m_Visibility = Public;
	m_role[A].m_Changeability = chg_Changeable;
	m_role[B].m_Changeability = chg_Changeable;
	m_AssocType = Uml::at_Association;
	m_umldoc = UMLApp::app()->getDocument();
	m_LinePath.setAssociation( this );

	connect(m_pView, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu()));
	connect(m_pView, SIGNAL( sigClearAllSelected() ), this, SLOT( slotClearAllSelected() ) );
}

void AssociationWidget::resetTextPositions() {
	if (m_role[A].m_pMulti) {
		setTextPosition( tr_MultiA );
	}
	if (m_role[B].m_pMulti) {
		setTextPosition( tr_MultiB );
	}
	if (m_role[A].m_pChangeWidget) {
		setTextPosition( tr_ChangeA );
	}
	if (m_role[B].m_pChangeWidget) {
		setTextPosition( tr_ChangeB );
	}
	if (m_pName) {
		setTextPosition( tr_Name );
	}
	if (m_role[A].m_pRole) {
		setTextPosition( tr_RoleAName );
	}
	if (m_role[B].m_pRole) {
		setTextPosition( tr_RoleBName );
	}
}

void AssociationWidget::setIndex(int index, Role_Type role) {
	m_role[role].m_nIndex = index;
}

int AssociationWidget::getIndex(Role_Type role) const {
	return m_role[role].m_nIndex;
}

void AssociationWidget::setTotalCount(int count, Role_Type role) {
	m_role[role].m_nTotalCount = count;
}

int AssociationWidget::getTotalCount(Role_Type role) const {
	return  m_role[role].m_nTotalCount;
}

UMLOperation *AssociationWidget::getOperation() {
	return dynamic_cast<UMLOperation*>(m_pObject);
}

void AssociationWidget::setOperation(UMLOperation *op) {
	if (m_pObject)
		disconnect(m_pObject, SIGNAL(modified()), m_pName, SLOT(setMessageText()));
	m_pObject = op;
	if (m_pObject)
		connect(m_pObject, SIGNAL(modified()), m_pName, SLOT(setMessageText()));
}

UMLClassifier *AssociationWidget::getOperationOwner() {
	Association_Type atype = getAssocType();
	Role_Type role = (atype == at_Coll_Message ? B : A);
	UMLObject *o = getWidget(role)->getUMLObject();
	if (o == NULL)
		return NULL;
	UMLClassifier *c = dynamic_cast<UMLClassifier*>(o);
	if (c == NULL)
		kdError() << "AssociationWidget::getOperationOwner: "
			  << "getWidget(" << role << ") is not a classifier"
			  << endl;
	return c;
}

void AssociationWidget::setSeqNumAndOp(const QString &seqNum, const QString &op) {
	if (! op.isEmpty())
		setName(op);
	setMulti(seqNum, A);
}

UMLClassifier *AssociationWidget::getSeqNumAndOp(QString& seqNum, QString& op) {
	seqNum = getMulti(A);
	op = getName();
	UMLObject *o = getWidget(B)->getUMLObject();
	UMLClassifier *c = dynamic_cast<UMLClassifier*>(o);
	return c;
}

void AssociationWidget::setCustomOpText(const QString &opText) {
	setName(opText);
}

QString AssociationWidget::getCustomOpText() {
	return getName();
}

void AssociationWidget::setWidget( UMLWidget* widget, Role_Type role) {
	m_role[role].m_pWidget = widget;
	if (widget) {
		m_role[role].m_pWidget->addAssoc(this);
		if (m_pObject && m_pObject->getBaseType() == ot_Association)
			getAssociation()->setObject(widget->getUMLObject(), role);
	}
}

void AssociationWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement assocElement = qDoc.createElement( "assocwidget" );

	if (m_pObject) {
		assocElement.setAttribute( "xmi.id", ID2STR(m_pObject->getID()) );
	}
	if (getAssociation() == NULL) {
		assocElement.setAttribute( "type", m_AssocType );
		assocElement.setAttribute( "visibilityA", m_role[A].m_Visibility);
		assocElement.setAttribute( "visibilityB", m_role[B].m_Visibility);
		assocElement.setAttribute( "changeabilityA", m_role[A].m_Changeability);
		assocElement.setAttribute( "changeabilityB", m_role[B].m_Changeability);
		if (m_pObject == NULL) {
			assocElement.setAttribute( "roleAdoc", m_role[A].m_RoleDoc);
			assocElement.setAttribute( "roleBdoc", m_role[B].m_RoleDoc);
			assocElement.setAttribute( "documentation", m_Doc );
		}
	}
	assocElement.setAttribute( "widgetaid", ID2STR(getWidgetID(A)) );
	assocElement.setAttribute( "widgetbid", ID2STR(getWidgetID(B)) );
	assocElement.setAttribute( "indexa", m_role[A].m_nIndex );
	assocElement.setAttribute( "indexb", m_role[B].m_nIndex );
	assocElement.setAttribute( "totalcounta", m_role[A].m_nTotalCount );
	assocElement.setAttribute( "totalcountb", m_role[B].m_nTotalCount );
	m_LinePath.saveToXMI( qDoc, assocElement );

	if( m_pName )
		m_pName -> saveToXMI( qDoc, assocElement );

	if( m_role[A].m_pMulti )
		m_role[A].m_pMulti -> saveToXMI( qDoc, assocElement );

	if( m_role[B].m_pMulti )
		m_role[B].m_pMulti -> saveToXMI( qDoc, assocElement );

	if( m_role[A].m_pRole )
		m_role[A].m_pRole -> saveToXMI( qDoc, assocElement );

	if( m_role[B].m_pRole )
		m_role[B].m_pRole -> saveToXMI( qDoc, assocElement );

	if( m_role[A].m_pChangeWidget )
		m_role[A].m_pChangeWidget -> saveToXMI( qDoc, assocElement );

	if( m_role[B].m_pChangeWidget )
		m_role[B].m_pChangeWidget -> saveToXMI( qDoc, assocElement );

	qElement.appendChild( assocElement );
}

bool AssociationWidget::loadFromXMI( QDomElement & qElement,
				     const UMLWidgetList& widgets,
				     const MessageWidgetList* pMessages )
{

	// load child widgets first
	QString widgetaid = qElement.attribute( "widgetaid", "-1" );
	QString widgetbid = qElement.attribute( "widgetbid", "-1" );
	Uml::IDType aId = STR2ID(widgetaid);
	Uml::IDType bId = STR2ID(widgetbid);
	UMLWidget *pWidgetA = Umbrello::findWidget( aId, widgets, pMessages );
	if (!pWidgetA) {
		kdError() << "AssociationWidget::loadFromXMI(): "
			  << "cannot find widget for roleA id " << ID2STR(aId) << endl;
		return false;
	}
	UMLWidget *pWidgetB = Umbrello::findWidget( bId, widgets, pMessages );
	if (!pWidgetB) {
		kdError() << "AssociationWidget::loadFromXMI(): "
			  << "cannot find widget for roleB id " << ID2STR(bId) << endl;
		return false;
	}
	setWidget(pWidgetA, A);
	setWidget(pWidgetB, B);

	QString id = qElement.attribute( "xmi.id", "-1" );
	bool oldStyleLoad = false;
	if (id == "-1") {
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
			if (!m_pObject && umlRoleA && umlRoleB)
			{
				oldStyleLoad = true; // flag for further special config below
				if (aType == at_Aggregation || aType == at_Composition) {
					kdWarning()<<" Old Style save file? swapping roles on association widget"<<this<<endl;
					// We have to swap the A and B widgets to compensate
					// for the long standing bug in LinePath of drawing
					// the diamond at the wrong end which was fixed
					// just before the 1.2 release.
					// The logic here is that the user has understood
					// that the diamond belongs at the SOURCE end of the
					// the association (i.e. at the container, not at the
					// contained), and has compensated for this anomaly
					// by drawing the aggregations/compositions from
					// target to source.
					UMLWidget *tmpWidget = pWidgetA;
					pWidgetA = pWidgetB;
					pWidgetB = tmpWidget;
					setWidget(pWidgetA, A);
					setWidget(pWidgetB, B);
					umlRoleA = pWidgetA->getUMLObject();
					umlRoleB = pWidgetB->getUMLObject();
				}

				setUMLAssociation(m_umldoc->createUMLAssociation(umlRoleA, umlRoleB, aType));
			}
		}

		setDoc( qElement.attribute("documentation", "") );
		setRoleDoc( qElement.attribute("roleAdoc", ""), A );
		setRoleDoc( qElement.attribute("roleBdoc", ""), B );

		setAssocType(aType);

		// visibilty defaults to Public if it cant set it here..
		QString visibilityA = qElement.attribute( "visibilityA", "0");
		if (visibilityA.toInt() > 0)
			setVisibility( (Scope)visibilityA.toInt(), A);

		QString visibilityB = qElement.attribute( "visibilityB", "0");
		if (visibilityB.toInt() > 0)
			setVisibility( (Scope)visibilityB.toInt(), B);

		// Changeability defaults to "Changeable" if it cant set it here..
		QString changeabilityA = qElement.attribute( "changeabilityA", "0");
		if (changeabilityA.toInt() > 0)
			setChangeability( (Changeability_Type)changeabilityA.toInt(), A);

		QString changeabilityB = qElement.attribute( "changeabilityB", "0");
		if (changeabilityB.toInt() > 0)
			setChangeability( (Changeability_Type)changeabilityB.toInt(), B);

	} else {

		// we should disconnect any prior association (can this happen??)
		if (m_pObject && m_pObject->getBaseType() == ot_Association)
		{
			UMLAssociation *umla = getAssociation();
			umla->disconnect(this);
			umla->nrof_parent_widgets--;
		}

		// New style: The xmi.id is a reference to the UMLAssociation.
		Uml::IDType nId = STR2ID(id);
		UMLObject *myObj = m_umldoc->findObjectById(nId);
		if (myObj == NULL) {
			kdError() << "AssociationWidget::loadFromXMI: cannot find UMLObject "
				  << ID2STR(nId) << endl;
			return false;
		} else {
			const Uml::Object_Type ot = myObj->getBaseType();
			if (ot == ot_Attribute || ot == ot_Operation) {
				m_pObject = myObj;
				QString type = qElement.attribute( "type", "-1" );
				Uml::Association_Type aType = (Uml::Association_Type) type.toInt();
				setAssocType(aType);
			} else {
				UMLAssociation * myAssoc = (UMLAssociation*)myObj;
				setUMLAssociation(myAssoc);
				m_LinePath.setAssocType( myAssoc->getAssocType() );
			}
		}
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
		} else if (tag == "floatingtext" ||
			   tag == "UML:FloatingTextWidget") {  // for bkwd compatibility
			QString r = element.attribute( "role", "-1");
			if( r == "-1" )
				return false;
			Uml::Text_Role role = (Uml::Text_Role)r.toInt();
			FloatingText *ft = new FloatingText(m_pView, role, "", Uml::id_Reserved);
			if( ! ft->loadFromXMI(element) ) {
				// Most likely cause: The FloatingText is empty.
				delete ft;
				node = element.nextSibling();
				element = node.toElement();
				continue;
			}
			// always need this
			ft->setLink(this);

			switch( role ) {
				case Uml::tr_MultiA:
					m_role[A].m_pMulti = ft;
					if(oldStyleLoad)
						setMulti(m_role[A].m_pMulti->getText(), A);
					break;

				case Uml::tr_MultiB:
					m_role[B].m_pMulti = ft;
					if(oldStyleLoad)
						setMulti(m_role[B].m_pMulti->getText(), B);
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
					ft->setLink(this);
					ft->setActivated();
					if(FloatingText::isTextValid(ft->getText()))
						ft -> show();
					else
						ft -> hide();
					break;

				case Uml::tr_RoleAName:
					m_role[A].m_pRole = ft;
					setRoleName( ft->getText(), A );
					break;
				case Uml::tr_RoleBName:
					m_role[B].m_pRole = ft;
					setRoleName( ft->getText(), B );
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
