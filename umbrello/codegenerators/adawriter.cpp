/***************************************************************************
                          adawriter.cpp  -  description
                             -------------------
    Based on javawriter.cpp by Luis De la Parra Blum
    begin                : Sat Dec 14 2002
    copyright            : (C) 2002 by Oliver Kellogg
    email                : okellogg@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "adawriter.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qptrlist.h>
#include <qregexp.h>

#include "../umldoc.h"
#include "../class.h"
#include "../enum.h"
#include "../enumliteral.h"
#include "../umlenumliterallist.h"
#include "../package.h"
#include "../association.h"
#include "../attribute.h"
#include "../operation.h"
#include "../umlnamespace.h"

const QString AdaWriter::defaultPackageSuffix = "_Holder";

AdaWriter::AdaWriter(UMLDoc *parent, const char *name)
		: SimpleCodeGenerator(parent, name) {
	indentlevel = 0;
	// FIXME: Eventually we should fabricate an Indenter class
	// that can be used by all code generators.
	// NOTE: this now exists under new code gen system. Dont do here! -b.t.
}

AdaWriter::~AdaWriter() {}

QString AdaWriter::spc() {
	QString s;
	s.fill(' ', indentlevel * 3);
	return s;
}

QString AdaWriter::toupperfirst(QString s) {
	QChar first = s.at(0);
	QString retval = first.upper() + s.right(s.length() - 1);
	return retval;
}

QString AdaWriter::adatype(QString umbtype) {
	QString retval;
	if (umbtype == "int")
		retval = "Integer";
	else if(umbtype == "long")
		retval = "Long_Integer";
	else if(umbtype == "bool")
		retval = "Boolean";
	else if(umbtype == "string")
		retval = "String";
	else if(umbtype == "double")
		retval = "Long_Float";
	else if(umbtype == "float")
		retval = "Float";
	else
		retval = umbtype;
	return retval;
}

QString AdaWriter::getLanguage() {
        return "Ada";
}

bool AdaWriter::isType (QString & type)
{
   if(type == "AdaWriter")
        return true;
   return false;
}


bool AdaWriter::isOOClass(UMLClassifier *c) {
	Uml::UMLObject_Type ot = c->getBaseType();
	if (ot == Uml::ot_Interface)
		return true;
	if (ot == Uml::ot_Enum)
		return false;
	if (ot != Uml::ot_Class) {
		kdWarning() << "AdaWriter::isOOClass: unknown object type " << ot << endl;
		return false;
	}
	QString stype = c->getStereotype();
	if (stype == "CORBAConstant" || stype == "CORBATypedef" ||
	    stype == "CORBAStruct" || stype == "CORBAUnion")
		return false;
	UMLClass *cl = static_cast<UMLClass *>(c);
	if (cl->isEnumeration())
		return false;

	// CORBAValue, CORBAInterface, and all empty/unknown stereotypes are
	// assumed to be OO classes.
	return true;
}

QString AdaWriter::qualifiedName(UMLClassifier *c, bool withType, bool byValue) {
	UMLPackage *umlPkg = c->getUMLPackage();
	QString className = cleanName(c->getName());
	QString retval;

	if (umlPkg == NULL) {
		retval = className;
		if (! isOOClass(c))
			retval.append(defaultPackageSuffix);
	} else {
		retval = umlPkg->getFullyQualifiedName(".");
		if (isOOClass(c)) {
			retval.append(".");
			retval.append(className);
		}
	}
	if (! withType)
		return retval;
	if (isOOClass(c)) {
		retval.append(".Object");
		if (! byValue)
			retval.append("_Ptr");
	} else {
		retval.append(".");
		retval.append(className);
	}
	return retval;
}

void AdaWriter::computeAssocTypeAndRole
       (UMLAssociation *a, QString& typeName, QString& roleName) {
	UMLClassifier* c = (UMLClassifier*) m_doc->findUMLObject(a->getRoleAId());
	typeName = cleanName(c->getName());
	if (! a->getMultiA().isEmpty())
		typeName.append("_Array_Access");
	roleName = a->getRoleNameA();
	if (roleName.isEmpty()) {
		if (a->getMultiA().isEmpty()) {
			roleName = "M_";
			roleName.append(typeName);
		} else {
			roleName = typeName;
			roleName.append("_Vector");
		}
	}
}

void AdaWriter::writeClass(UMLClassifier *c) {
	if (!c) {
		kdDebug() << "Cannot write class of NULL concept!\n";
		return;
	}

	UMLClass * myClass = dynamic_cast<UMLClass*>(c);
	QString classname = cleanName(c->getName());
	QString fileName = qualifiedName(c).lower();
	fileName.replace('.', '-');

	//find an appropriate name for our file
	fileName = overwritableName(c, fileName, ".ads");
	if (!fileName) {
		emit codeGenerated(c, false);
		return;
	}

	QFile file;
	if (!openFile(file, fileName + ".ads")) {
		emit codeGenerated(c, false);
		return;
	}

	// Start generating the code.

	QTextStream ada(&file);
	//try to find a heading file(license, comments, etc)
	QString str;
	str = getHeadingFile(".ads");
	if (!str.isEmpty()) {
		str.replace(QRegExp("%filename%"), fileName);
		str.replace(QRegExp("%filepath%"), file.name());
		ada << str << endl;
	}

	// Import referenced classes.
	UMLClassifierList imports;
	findObjectsRelated(c, imports);
	if (imports.count()) {
		for (UMLClassifier *con = imports.first(); con; con = imports.next())
			ada << "with " << qualifiedName(con) << "; \n";
		ada << "\n";
	}

	QString pkg = qualifiedName(c);
	ada << spc() << "package " << pkg << " is\n\n";
	indentlevel++;
	if (c->getBaseType() == Uml::ot_Enum) {
		UMLEnum *ue = static_cast<UMLEnum*>(c);
		UMLEnumLiteralList litList = ue->getFilteredEnumLiteralList();
		uint i = 0;
		ada << spc() << "type " << classname << " is (\n";
		indentlevel++;
		for (UMLEnumLiteral *lit = litList.first(); lit; lit = litList.next()) {
			QString enumLiteral = cleanName(lit->getName());
			ada << spc() << enumLiteral;
			if (++i < litList.count())
				ada << ",\n";
		}
		indentlevel--;
		ada << ");\n\n";
		indentlevel--;
		ada << spc() << "end " << pkg << ";\n\n";
		return;
	}
	if (! isOOClass(c)) {
		QString stype = c->getStereotype();
		if (stype == "CORBAConstant") {
			ada << spc() << "-- " << stype << " is Not Yet Implemented\n\n";
		} else if (myClass && myClass->isEnumeration()) {
			UMLAttributeList *atl = myClass->getFilteredAttributeList();
			UMLAttribute *at;
			ada << spc() << "type " << classname << " is (\n";
			indentlevel++;
			uint i = 0;
			for (at = atl->first(); at; at = atl->next()) {
				QString enumLiteral = cleanName(at->getName());
				ada << spc() << enumLiteral;
				if (++i < atl->count())
					ada << ",\n";
			}
			indentlevel--;
			ada << ");\n\n";
		} else if(stype == "CORBAStruct") {
			if(myClass) {
				UMLAttributeList *atl = myClass->getFilteredAttributeList();
				UMLAttribute *at;
				ada << spc() << "type " << classname << " is record\n";
				indentlevel++;
				for (at = atl->first(); at; at = atl->next()) {
					QString name = cleanName(at->getName());
					QString typeName = adatype(at->getTypeName());
					ada << spc() << name << " : " << typeName;
					QString initialVal = at->getInitialValue();
					if (initialVal.latin1() && ! initialVal.isEmpty())
						ada << " := " << initialVal;
					ada << ";\n";
				}
				indentlevel--;
				ada << spc() << "end record;\n\n";
			}
		} else if(stype == "CORBAUnion") {
			ada << spc() << "-- " << stype << " is Not Yet Implemented\n\n";
		} else if(stype == "CORBATypedef") {
			ada << spc() << "-- " << stype << " is Not Yet Implemented\n\n";
		} else {
			ada << spc() << "-- " << stype << ": Unknown stereotype\n\n";
		}
		indentlevel--;
		ada << spc() << "end " << pkg << ";\n\n";
		return;
	}

	// Write class Documentation if non-empty or if force option set.
	if (forceDoc() || !c->getDoc().isEmpty()) {
		ada << "--\n";
		ada << "-- class " << classname << endl;
		ada << formatDoc(c->getDoc(), "-- ");
		ada << "\n";
	}

	UMLClassifierList superclasses = c->getSuperClasses();

	ada << spc() << "type Object is ";
	if (c->getAbstract())
		ada << "abstract ";
	if (superclasses.isEmpty()) {
		ada << "tagged ";
	} else {
		// FIXME: Multiple inheritance is not yet supported
		UMLClassifier* parent = superclasses.first();
		ada << "new " << qualifiedName(parent) << ".Object with ";
	}
	ada << "private;\n\n";
	ada << spc() << "type Object_Ptr is access all Object'Class;\n\n";

	// Generate accessors for public attributes.
	UMLAttributeList *atl;
        if(myClass) {
		UMLAttributeList atpub;
		atpub.setAutoDelete(false);

		atl = myClass->getFilteredAttributeList();

		UMLAttribute *at;
		for (at = atl->first(); at; at = atl->next()) {
			if (at->getScope() == Uml::Public)
				atpub.append(at);
		}
		if (forceSections() || atpub.count())
			ada << spc() << "-- Accessors for public attributes:\n\n";
		for (at = atpub.first(); at; at = atpub.next()) {
			QString member = cleanName(at->getName());
			ada << spc() << "procedure Set_" << member << " (";
			if (! at->getStatic())
				ada << "Self : access Object; ";
			ada << "To : " << adatype(at->getTypeName()) << ");\n";
			ada << spc() << "function  Get_" << member;
			if (! at->getStatic())
				ada << " (Self : access Object)";
			ada << " return " << adatype(at->getTypeName()) << ";\n\n";
		}
	} else {
		kdWarning() << "atl not initialised in writeClass()" << endl;
		atl = 0;
	}

	// Generate public operations.
	UMLOperationList *opl = c->getFilteredOperationsList();
	UMLOperationList oppub;
	oppub.setAutoDelete(false);
	UMLOperation *op;
	for (op = opl->first(); op; op = opl->next()) {
		if (op->getScope() == Uml::Public)
			oppub.append(op);
	}
	if (forceSections() || oppub.count())
		ada << spc() << "-- Public methods:\n\n";
	for (op = oppub.first(); op; op = oppub.next())
		writeOperation(op, ada);

	indentlevel--;
	ada << spc() << "private\n\n";
	indentlevel++;

	// Generate auxiliary declarations for multiplicity of associations
	UMLAssociationList aggregations = c->getAggregations();
	if (!aggregations.isEmpty()) {
		for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next()) {
			if (a->getMultiA().isEmpty())
				continue;
			UMLClassifier* other = (UMLClassifier*) m_doc->findUMLObject(a->getRoleAId());
			QString member = cleanName(other->getName());
			// Handling of packages is missing here
			// A test and error action is missing here for !isOOClass()
			ada << spc() << "type " << member << "_Array is array"
			    << " (Positive range <>) of " << member << ".Object_Ptr;\n";
			ada << spc() << "type " << member << "_Array_Access is access "
			    << member << "_array;\n\n";
		}
	}
	UMLAssociationList compositions = c->getCompositions();
	if (!compositions.isEmpty()) {
		for (UMLAssociation *a = compositions.first(); a; a = compositions.next()) {
			if (a->getMultiA().isEmpty())
				continue;
			UMLObject *other = m_doc->findUMLObject(a->getRoleAId());
			QString member = cleanName(other->getName());
			// Handling of packages is missing here
			// Treatment of !isOOClass() is missing here
			ada << spc() << "type " << member << "_Array is array"
			    << " (Positive range <>) of " << member << ".Object;\n";
			ada << spc() << "type " << member << "_Array_Access is access "
			    << member << "_array;\n\n";
		}
	}

	ada << spc() << "type Object is ";
	if (c->getAbstract())
		ada << "abstract ";
	if (superclasses.isEmpty()) {
		ada << "tagged ";
	} else {
		// FIXME: Multiple inheritance is not yet supported
		UMLClassifier* parent = superclasses.first();
		ada << "new " << qualifiedName(parent) << ".Object with ";
	}
	ada << "record\n";
	indentlevel++;

	if (forceSections() || !aggregations.isEmpty()) {
		ada << spc() << "-- Aggregations:\n";
		for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next()) {
			QString typeName, roleName;
			computeAssocTypeAndRole(a, typeName, roleName);
			ada << spc() << roleName << " : " << typeName << ";\n";
		}
		ada << endl;
	}
	if (forceSections() || !compositions.isEmpty()) {
		ada << spc() << "-- Compositions:\n";
		for (UMLAssociation *a = compositions.first(); a; a = compositions.next()) {
			QString typeName, roleName;
			computeAssocTypeAndRole(a, typeName, roleName);
			ada << spc() << roleName << " : " << typeName << ";\n";
		}
		ada << endl;
	}

	if (forceSections() || atl->count()) {
		ada << spc() << "-- Attributes:\n";
		UMLAttribute *at;
		for (at = atl->first(); at; at = atl->next()) {
			if (at->getStatic())
				continue;
			ada << spc() << cleanName(at->getName()) << " : "
			    << adatype(at->getTypeName());
			if (at && at->getInitialValue().latin1() && ! at->getInitialValue().isEmpty())
				ada << " := " << at->getInitialValue();
			ada << ";\n";
		}
	}
	if (aggregations.isEmpty() && compositions.isEmpty() && !atl->count())
		ada << spc() << "null;\n";
	indentlevel--;
	ada << spc() << "end record;\n\n";
	bool seen_static_attr = false;
	UMLAttribute *at;
	for (at = atl->first(); at; at = atl->next()) {
		if (! at->getStatic())
			continue;
		if (! seen_static_attr) {
			ada << spc() << "-- Static attributes:\n";
			seen_static_attr = true;
		}
		ada << spc();
		if (at->getScope() == Uml::Private)
			ada << "-- Private:  ";
		ada << cleanName(at->getName()) << " : "
		    << adatype(at->getTypeName());
		if (at && at->getInitialValue().latin1() && ! at->getInitialValue().isEmpty())
			ada << " := " << at->getInitialValue();
		ada << ";\n";
	}
	if (seen_static_attr)
		ada << "\n";

	// Generate protected operations.
	UMLOperationList opprot;
	opprot.setAutoDelete(false);
	for (op = opl->first(); op; op = opl->next()) {
		if (op->getScope() == Uml::Protected)
			opprot.append(op);
	}
	if (forceSections() || opprot.count())
		ada << spc() << "-- Protected methods:\n\n";
	for (op = opprot.first(); op; op = opprot.next())
		writeOperation(op, ada);

	// Generate private operations.
	// These are currently only generated as comments in the private part
	// of the spec.
	// Once umbrello supports the merging of automatically generated and
	// hand written code sections, private operations should be generated
	// into the package body.
	UMLOperationList oppriv;
	oppriv.setAutoDelete(false);
	for (op = opl->first(); op; op = opl->next()) {
		if (op->getScope() == Uml::Private)
			oppriv.append(op);
	}
	if (forceSections() || oppriv.count())
		ada << spc() << "-- Private methods:\n\n";
	for (op = oppriv.first(); op; op = oppriv.next())
		writeOperation(op, ada, true);

	indentlevel--;
	ada << spc() << "end " << pkg << ";\n\n";
	file.close();
	emit codeGenerated(c, true);
}


void AdaWriter::writeOperation(UMLOperation *op, QTextStream &ada, bool is_comment) {
	UMLAttributeList *atl = op->getParmList();
	QString rettype = op->getReturnType();
	bool use_procedure = (rettype.isEmpty() || rettype == "void");

	ada << spc();
	if (is_comment)
		ada << "-- ";
	if (use_procedure)
		ada << "procedure ";
	else
		ada << "function ";
	ada << cleanName(op->getName()) << " ";
	if (! (op->getStatic() && atl->count() == 0))
		ada << "(";
	if (! op->getStatic()) {
		ada << "Self : access Object";
		if (atl->count())
			ada << ";\n";
	}
	if (atl->count()) {
		uint i = 0;
		indentlevel++;
		for (UMLAttribute *at = atl->first(); at; at = atl->next()) {
			ada << spc();
			if (is_comment)
				ada << "-- ";
			ada << cleanName(at->getName()) << " : "
			    << adatype(at->getTypeName());
			if (! at->getInitialValue().isEmpty())
				ada << " := " << at->getInitialValue();
			if (++i < atl->count()) //FIXME gcc warning
				ada << ";\n";
		}
		indentlevel--;
	}
	if (! (op->getStatic() && atl->count() == 0))
		ada << ")";
	if (! use_procedure)
		ada << " return " << adatype(rettype);
	ada << " is abstract;\n\n";
	// TBH, we make the methods abstract here because we don't have the means
	// for generating meaningful implementations.
}



#include "adawriter.moc"
