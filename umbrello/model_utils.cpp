 /*
  *  copyright (C) 2004
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
#include "model_utils.h"

// qt/kde includes
#include <qregexp.h>
#include <qstringlist.h>
#include <klocale.h>
#include <kdebug.h>

// app includes
#include "umlobject.h"
#include "umlpackagelist.h"
#include "package.h"
#include "classifier.h"
#include "template.h"
#include "association.h"
#include "umlrole.h"
#include "umldoc.h"
#include "uml.h"

namespace Umbrello {

bool isCloneable(Uml::Widget_Type type) {
	switch (type) {
		case Uml::wt_Actor:
		case Uml::wt_UseCase:
		case Uml::wt_Class:
		case Uml::wt_Interface:
		case Uml::wt_Enum:
		case Uml::wt_Datatype:
		case Uml::wt_Package:
		case Uml::wt_Component:
		case Uml::wt_Node:
		case Uml::wt_Artifact:
			return true;
		default:
			return false;
	}
}

UMLObject * findObjectInList(Uml::IDType id, UMLObjectList inList) {
	for (UMLObjectListIt oit(inList); oit.current(); ++oit) {
		UMLObject *obj = oit.current();
		if (obj->getID() == id)
			return obj;
		UMLObject *o;
		Uml::Object_Type t = obj->getBaseType();
		switch (t) {
			case Uml::ot_Package:
				o = ((UMLPackage*)obj)->findObjectById(id);
				if (o)
					return o;
				break;
			case Uml::ot_Interface:
			case Uml::ot_Class:
			case Uml::ot_Enum:
			case Uml::ot_Entity:
				o = ((UMLClassifier*)obj)->findChildObject(id);
				if (o == NULL &&
				    (t == Uml::ot_Interface || t == Uml::ot_Class))
					o = ((UMLPackage*)obj)->findObjectById(id);
				if (o)
					return o;
				break;
			case Uml::ot_Association:
				{
					UMLAssociation *assoc = (UMLAssociation*)obj;
					UMLRole *rA = assoc->getUMLRole(Uml::A);
					if (rA->getID() == id)
						return rA;
					UMLRole *rB = assoc->getUMLRole(Uml::B);
					if (rB->getID() == id)
						return rB;
				}
				break;
			default:
				break;
		}
	}
	return NULL;
}

UMLObject* findUMLObject(UMLObjectList inList, QString name,
			 Uml::Object_Type type /* = ot_UMLObject */,
			 UMLObject *currentObj /* = NULL */) {
	QStringList components;
	if (name.contains("::"))
		components = QStringList::split("::", name);
	else if (name.contains("."))
		components = QStringList::split(".", name);
	QString nameWithoutFirstPrefix;
	if (components.size() > 1) {
		if (name.contains(QRegExp("[^\\w:\\.]"))) {
			// It's obviously a datatype.
			// Scope qualified datatypes live in the global scope.
			for (UMLObjectListIt oit(inList); oit.current(); ++oit) {
				UMLObject *obj = oit.current();
				if (obj->getName() == name)
					return obj;
			}
			return NULL;
		}
		name = components.front();
		components.pop_front();
		nameWithoutFirstPrefix = components.join("::");
	}
	if (currentObj) {
		UMLPackage *pkg = NULL;
		if (dynamic_cast<UMLClassifierListItem*>(currentObj)) {
			pkg = dynamic_cast<UMLPackage*>(currentObj->parent());
		} else {
			pkg = currentObj->getUMLPackage();
		}
		// Remember packages that we've seen - for avoiding cycles.
		UMLPackageList seenPkgs;
		for (; pkg; pkg = currentObj->getUMLPackage()) {
			if (seenPkgs.findRef(pkg) != -1) {
				kdError() << "findUMLObject(" << name << "): "
					  << "breaking out of cycle involving "
					  << pkg->getName() << endl;
				break;
			}
			seenPkgs.append(pkg);
			UMLObjectList objectsInCurrentScope = pkg->containedObjects();
			for (UMLObjectListIt oit(objectsInCurrentScope); oit.current(); ++oit) {
				UMLObject *obj = oit.current();
				if (obj->getName() != name)
					continue;
				Uml::Object_Type foundType = obj->getBaseType();
				if (nameWithoutFirstPrefix.isEmpty()) {
					if (type != Uml::ot_UMLObject && type != foundType) {
						kdDebug() << "findUMLObject: type mismatch for "
							  << name << " (seeking type: "
							  << type << ", found type: "
							  << foundType << ")" << endl;
						continue;
					}
					return obj;
				}
				if (foundType != Uml::ot_Package &&
				    foundType != Uml::ot_Class &&
				    foundType != Uml::ot_Interface) {
					kdDebug() << "findUMLObject: found \"" << name
						  << "\" is not a package (?)" << endl;
					continue;
				}
				UMLPackage *pkg = static_cast<UMLPackage*>(obj);
				return findUMLObject( pkg->containedObjects(),
						      nameWithoutFirstPrefix, type );
			}
			currentObj = pkg;
		}
	}
	for (UMLObjectListIt oit(inList); oit.current(); ++oit) {
		UMLObject *obj = oit.current();
		if (obj->getName() != name)
			continue;
		Uml::Object_Type foundType = obj->getBaseType();
		if (nameWithoutFirstPrefix.isEmpty()) {
			if (type != Uml::ot_UMLObject && type != foundType) {
				kdDebug() << "findUMLObject: type mismatch for "
					  << name << " (seeking type: "
					  << type << ", found type: "
					  << foundType << ")" << endl;
				continue;
			}
			return obj;
		}
		if (foundType != Uml::ot_Package &&
		    foundType != Uml::ot_Class &&
		    foundType != Uml::ot_Interface) {
			kdDebug() << "findUMLObject: found \"" << name
				  << "\" is not a package (?)" << endl;
			continue;
		}
		UMLPackage *pkg = static_cast<UMLPackage*>(obj);
		return findUMLObject( pkg->containedObjects(),
				      nameWithoutFirstPrefix, type );
	}
	return NULL;
}

bool isCommonXMIAttribute( const QString &tag ) {
	bool retval = (Uml::tagEq(tag, "name") ||
		       Uml::tagEq(tag, "visibility") ||
		       Uml::tagEq(tag, "isRoot") ||
		       Uml::tagEq(tag, "isLeaf") ||
		       Uml::tagEq(tag, "isAbstract") ||
		       Uml::tagEq(tag, "isSpecification") ||
		       Uml::tagEq(tag, "isActive") ||
		       Uml::tagEq(tag, "namespace") ||
		       Uml::tagEq(tag, "ownerScope") ||
		       Uml::tagEq(tag, "GeneralizableElement.generalization") ||
		       Uml::tagEq(tag, "specialization") ||   //NYI
		       Uml::tagEq(tag, "clientDependency") || //NYI
		       Uml::tagEq(tag, "supplierDependency")  //NYI
		      );
	return retval;
}

QString scopeToString(Uml::Scope scope, bool mnemonic) {
	switch (scope) {
	case Uml::Protected:
		return (mnemonic ? "#" : "protected");
		break;
	case Uml::Private:
		return (mnemonic ? "-" : "private");
		break;
	case Uml::Public:
	default:
		return (mnemonic ? "+" : "public");
		break;
	}
}

int stringToDirection(QString input, Uml::Parameter_Direction & result) {
	QRegExp dirx("^(in|out|inout)");
	int pos = dirx.search(input);
	if (pos == -1)
		return 0;
	const QString& dirStr = dirx.capturedTexts().first();
	uint dirLen = dirStr.length();
	if (input.length() > dirLen && !input[dirLen].isSpace())
		return 0;	// no match after all.
	if (dirStr == "out")
		result = Uml::pd_Out;
	else if (dirStr == "inout")
		result = Uml::pd_InOut;
	else
		result = Uml::pd_In;
	return dirLen;
}

Parse_Status parseTemplate(QString t, NameAndType& nmTp, UMLClassifier *owningScope) {

	UMLDoc *pDoc = UMLApp::app()->getDocument();

	t = t.stripWhiteSpace();
	if (t.isEmpty())
		return PS_Empty;

	QStringList nameAndType = QStringList::split( QRegExp("\\s*:\\s*"), t);
	if (nameAndType.count() == 2) {
		UMLObject *pType = NULL;
		if (nameAndType[1] != "class") {
			pType = pDoc->findUMLObject(nameAndType[1], Uml::ot_UMLObject, owningScope);
			if (pType == NULL)
				return PS_Unknown_ArgType;
		}
		nmTp = NameAndType(nameAndType[0], pType);
	} else {
		nmTp = NameAndType(t, NULL);
	}
	return PS_OK;
}

Parse_Status parseAttribute(QString a, NameAndType& nmTp, UMLClassifier *owningScope) {
	UMLDoc *pDoc = UMLApp::app()->getDocument();

	a = a.stripWhiteSpace();
	if (a.isEmpty())
		return PS_Empty;

	QStringList nameAndType = QStringList::split( QRegExp("\\s*:\\s*"), a);
	const QString &name = nameAndType[0];
	UMLObject *pType = NULL;
	QString initialValue;
	if (nameAndType.count() == 2) {
		QStringList typeAndInitialValue = QStringList::split( QRegExp("\\s*=\\s*"), nameAndType[1] );
		const QString &type = typeAndInitialValue[0];
       		pType = pDoc->findUMLObject(type, Uml::ot_UMLObject, owningScope);
       		if (pType == NULL)
       			return PS_Unknown_ArgType;
		if (typeAndInitialValue.count() == 2) {
			initialValue = typeAndInitialValue[1];
		}
	}
	nmTp = NameAndType(name, pType, initialValue);
	return PS_OK;
}

Parse_Status parseOperation(QString m, OpDescriptor& desc, UMLClassifier *owningScope) {
	UMLDoc *pDoc = UMLApp::app()->getDocument();

	m = m.stripWhiteSpace();
	if (m.isEmpty())
		return PS_Empty;
	QRegExp pat( "^(\\w+)" );
	int pos = pat.search(m);
	if (pos == -1)
		return PS_Illegal_MethodName;
	desc.m_name = pat.cap(1);
	desc.m_pReturnType = NULL;
	pat = QRegExp( ":\\s*(\\w[\\w\\. ]*)$" );
	pos = pat.search(m);
	if (pos != -1) {  // return type is optional
		QString retType = pat.cap(1);
		if (retType != "void") {
			UMLObject *pRetType = owningScope->findTemplate(retType);
			if (pRetType == NULL) {
				pRetType = pDoc->findUMLObject(retType, Uml::ot_UMLObject, owningScope);
				if (pRetType == NULL)
					return PS_Unknown_ReturnType;
			}
			desc.m_pReturnType = pRetType;
		}
	}
	desc.m_args.clear();
	pat = QRegExp( "\\((.*)\\)" );
	pos = pat.search(m);
	if (pos == -1)  // argument list is optional
		return PS_OK;
	QString arglist = pat.cap(1);
	arglist = arglist.stripWhiteSpace();
	if (arglist.isEmpty())
		return PS_OK;
	QStringList args = QStringList::split( QRegExp("\\s*,\\s*"), arglist);
	for (QStringList::Iterator lit = args.begin(); lit != args.end(); ++lit) {
		NameAndType nmTp;
		Parse_Status ps = parseAttribute(*lit, nmTp, owningScope);
		if (ps)
			return ps;
		desc.m_args.append(nmTp);
	}
	return PS_OK;
}

QString psText(Parse_Status value) {
	const QString text[] = {
		i18n("OK"), i18n("Empty"), i18n("Malformed argument"),
		i18n("Unknown argument type"), i18n("Illegal method name"),
		i18n("Unknown return type"), i18n("Unspecified error")
	};
	return text[(unsigned) value];
}

}  // namespace Umbrello

