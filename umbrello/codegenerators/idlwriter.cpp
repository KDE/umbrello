/***************************************************************************
                          idlwriter.cpp  -  description
                             -------------------
    begin                : Sat Jan 4 2003
    copyright            : (C) 2003 by Oliver Kellogg
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

#include "idlwriter.h"

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

IDLWriter::IDLWriter(UMLDoc *parent, const char *name)
		: SimpleCodeGenerator(parent, name) {
	indentlevel = 0;
	// FIXME: Eventually we should fabricate an Indenter class
	// that can be used by all code generators.
	// NOTE: this has now been done in new code gen class. -b.t.
}

IDLWriter::~IDLWriter() {}

QString IDLWriter::spc() {
	QString s;
	s.fill(' ', indentlevel * 3);
	return s;
}

bool IDLWriter::isOOClass(UMLClassifier *c) {
	QString stype = c->getStereotype();
	if (stype == "CORBAConstant" || stype == "CORBAEnum" ||
	        stype == "CORBAStruct" || stype == "CORBAUnion" ||
	        stype == "CORBASequence" || stype == "CORBAArray" ||
	        stype == "CORBATypedef")
		return false;

	// CORBAValue, CORBAInterface, and all empty/unknown stereotypes are
	// assumed to be OO classes.
	return true;
}

QString IDLWriter::qualifiedName(UMLClassifier *c) {
	UMLPackage *umlPkg = c->getUMLPackage();
	QString className = cleanName(c->getName());
	QString retval;

	if (umlPkg) {
		retval = umlPkg->getFullyQualifiedName();
		retval.append("::");
	}
	retval.append(className);
	return retval;
}

QString IDLWriter::getLanguage() {
        return "IDL";
}

bool IDLWriter::isType (QString & type)
{
   if(type == "IDLWriter")
        return true;
   return false;
}

void IDLWriter::computeAssocTypeAndRole
       (UMLAssociation *a, QString& typeName, QString& roleName) {
	UMLClassifier* c = (UMLClassifier*) m_doc->findUMLObject(a->getRoleAId());
	typeName = cleanName(c->getName());
	QString multiplicity = a->getMultiA();
	if (!multiplicity.isEmpty() && multiplicity != "1")
		typeName.append("Vector");
	roleName = a->getRoleNameA();
	if (roleName.isEmpty()) {
		if (multiplicity.isEmpty() || multiplicity == "1") {
			roleName = "m_";
			roleName.append(typeName);
		} else {
			roleName = typeName;
			roleName.append("Vector");
		}
	}
}

void IDLWriter::writeClass(UMLClassifier *c) {
	if (!c) {
		kdDebug() << "Cannot write class of NULL concept!\n";
		return;
	}

	UMLClass * myClass = dynamic_cast<UMLClass*>(c);
	QString classname = cleanName(c->getName());

	//find an appropriate name for our file
	QString fileName = findFileName(c, ".idl");
	if (!fileName) {
		emit codeGenerated(c, false);
		return;
	}

	QFile file;
	if (!openFile(file, fileName + ".idl")) {
		emit codeGenerated(c, false);
		return;
	}

	// Start generating the code.

	QTextStream idl(&file);
	//try to find a heading file(license, comments, etc)
	QString str;
	str = getHeadingFile(".idl");
	if (!str.isEmpty()) {
		str.replace(QRegExp("%filename%"), fileName);
		str.replace(QRegExp("%filepath%"), file.name());
		idl << str << endl;
	}

	// Write includes.
	UMLClassifierList includes;
	findObjectsRelated(c, includes);
	if (includes.count()) {
		for (UMLClassifier *conc = includes.first(); conc; conc = includes.next()) {
			if (conc->getBaseType() == Uml::ot_Datatype)
				continue;
			QString baseName = findFileName(conc, ".idl");
			if (!baseName.isEmpty())
				idl << "#include \"" << baseName << ".idl\"\n";
		}
		idl << "\n";
	}

	// Generate the module declaration(s) for the package(s) in which
	// we are embedded.
	UMLPackageList pkgList = c->getPackages();
	UMLPackage *pkg;
	for (pkg = pkgList.first(); pkg != NULL; pkg = pkgList.next()) {
		idl << spc() << "module " << pkg->getName() << " {\n\n";
		indentlevel++;
	}

	// Write class Documentation if non-empty or if force option set.
	if (forceDoc() || !c->getDoc().isEmpty()) {
		idl << "//\n";
		idl << "// class " << classname << endl;
		idl << formatDoc(c->getDoc(), "// ");
		idl << "\n";
	}

	if (c->getBaseType() == Uml::ot_Enum) {
		UMLEnum *ue = static_cast<UMLEnum*>(c);
		UMLEnumLiteralList litList = ue->getFilteredEnumLiteralList();
		uint i = 0;
		idl << spc() << "enum " << classname << " {\n";
		indentlevel++;
		for (UMLEnumLiteral *lit = litList.first(); lit; lit = litList.next()) {
			QString enumLiteral = cleanName(lit->getName());
			idl << spc() << enumLiteral;
			if (++i < litList.count())
				idl << ",";
			idl << endl;
		}
		indentlevel--;
		idl << spc() << "};\n\n";
		// Close the modules inside which we might be nested.
		for (pkg = pkgList.first(); pkg != NULL; pkg = pkgList.next()) {
			indentlevel--;
			idl << spc() << "};\n\n";
		}
		return;
	}
	if (! isOOClass(c)) {
		QString stype = c->getStereotype();
		if (stype == "CORBAConstant") {
			kdError() << "The stereotype " << stype << " cannot be applied to "
				  << c->getName() << ", but only to attributes." << endl;
			return;
		}
		if (myClass == NULL) {
			kdError() << "The stereotype " << stype
				  << " cannot be applied to " << c->getName()
				  << ", but only to classes." << endl;
			return;
		}
		if (stype == "CORBAEnum") {
			UMLAttributeList *atl = myClass->getFilteredAttributeList();
			UMLAttribute *at;
			idl << spc() << "enum " << classname << " {\n";
			indentlevel++;
			uint i = 0;
			for (at = atl->first(); at; at = atl->next()) {
				QString enumLiteral = cleanName(at->getName());
				idl << spc() << enumLiteral;
				if (++i < atl->count())
					idl << ",";
				idl << endl;
			}
			indentlevel--;
			idl << spc() << "};\n\n";
		} else if (stype == "CORBAStruct") {
			UMLAttributeList *atl = myClass->getFilteredAttributeList();
			UMLAttribute *at;
			idl << spc() << "struct " << classname << " {\n";
			indentlevel++;
			for (at = atl->first(); at; at = atl->next()) {
				QString name = cleanName(at->getName());
				idl << spc() << at->getTypeName() << " " << name << ";\n";
				// Initial value not possible in IDL.
			}
			indentlevel--;
			idl << spc() << "};\n\n";
		} else if (stype == "CORBAUnion") {
			idl << spc() << "// " << stype << " " << c->getName()
			    << " is Not Yet Implemented\n\n";
		} else if (stype == "CORBATypedef") {
			UMLClassifierList superclasses = c->getSuperClasses();
			UMLClassifier* firstParent = superclasses.first();
			idl << spc() << "typedef " << firstParent->getName() << " "
			    << c->getName() << ";\n\n";
		} else {
			idl << spc() << "// " << stype << ": Unknown stereotype\n\n";
		}
		// Close the modules inside which we might be nested.
		for (pkg = pkgList.first(); pkg != NULL; pkg = pkgList.next()) {
			indentlevel--;
			idl << spc() << "};\n\n";
		}
		return;
	}

	idl << spc();
	if (c->getAbstract())
		idl << "abstract ";
	idl << "interface " << c->getName();
	UMLClassifierList superclasses = c->getSuperClasses();
	if (! superclasses.isEmpty()) {
		idl << " : ";
		UMLClassifier *parent = superclasses.first();
		int n_parents = superclasses.count();
		while (n_parents--) {
			idl << qualifiedName(parent);
			if (n_parents)
				idl << ", ";
	        	parent = superclasses.next();
		}
	}
	idl << " {\n\n";
	indentlevel++;

	// Generate auxiliary declarations for multiplicity of associations
	UMLAssociation *a;
	bool didComment = false;
	UMLAssociationList aggregations = c->getAggregations();
	for (a = aggregations.first(); a; a = aggregations.next()) {
		QString multiplicity = a->getMultiA();
		if (multiplicity.isEmpty() || multiplicity == "1")
			continue;
		if (!didComment) {
			idl << spc() << "// Types for association multiplicities\n\n";
			didComment = true;
		}
		UMLClassifier* other = (UMLClassifier*)m_doc->findUMLObject(a->getRoleAId());
		QString bareName = cleanName(other->getName());
		idl << spc() << "typedef sequence<" << qualifiedName(other) << "> "
		    << bareName << "Vector;\n\n";
	}

	UMLAssociationList compositions = c->getCompositions();
	for (a = compositions.first(); a; a = compositions.next()) {
		QString multiplicity = a->getMultiA();
		if (multiplicity.isEmpty() || multiplicity == "1")
			continue;
		if (!didComment) {
			idl << spc() << "// Types for association multiplicities\n\n";
			didComment = true;
		}
		UMLClassifier* other = (UMLClassifier*)m_doc->findUMLObject(a->getRoleAId());
		QString bareName = cleanName(other->getName());
		idl << spc() << "typedef sequence<" << qualifiedName(other) << "> "
		    << bareName << "Vector;\n\n";
	}

	// Generate public attributes.
	if(myClass) {
		UMLAttributeList *atl = myClass->getFilteredAttributeList();
		UMLAttributeList atpub;
		UMLAttribute *at;
		for (at = atl->first(); at; at = atl->next()) {
			if (at->getScope() == Uml::Public)
				atpub.append(at);
		}
		if (forceSections() || atpub.count()) {
			idl << spc() << "// Public attributes:\n\n";
			for (at = atpub.first(); at; at = atpub.next()) {
				QString attName = cleanName(at->getName());
				idl << spc() << "attribute " << at->getTypeName()
				    << " " << attName << ";\n\n";
			}
		}
	}

	// Generate public operations.
	UMLOperationList opl(c->getFilteredOperationsList());
	UMLOperationList oppub;
	UMLOperation *op;
	for (op = opl.first(); op; op = opl.next()) {
		if (op->getScope() == Uml::Public)
			oppub.append(op);
	}
	if (forceSections() || oppub.count()) {
		idl << spc() << "// Public methods:\n\n";
		for (op = oppub.first(); op; op = oppub.next())
			writeOperation(op, idl);
		idl << "\n";
	}


	if (forceSections() || !aggregations.isEmpty()) {
		idl << spc() << "// Aggregations:\n";
		for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next()) {
			QString typeName, roleName;
			computeAssocTypeAndRole(a, typeName, roleName);
			idl << spc() << "// " << typeName << " " << roleName << ";\n";
		}
		idl << endl;
	}
	if (forceSections() || !compositions.isEmpty()) {
		idl << spc() << "// Compositions:\n";
		for (UMLAssociation *a = compositions.first(); a; a = compositions.next()) {
			QString typeName, roleName;
			computeAssocTypeAndRole(a, typeName, roleName);
			idl << spc() << "// " << typeName << " " << roleName << ";\n";
		}
		idl << endl;
	}

	indentlevel--;
	idl << spc() << "};\n\n";

	// Close the modules inside which we might be nested.
	for (pkg = pkgList.first(); pkg != NULL; pkg = pkgList.next()) {
		indentlevel--;
		idl << spc() << "};\n\n";
	}
	file.close();
	emit codeGenerated(c, true);
}


void IDLWriter::writeOperation(UMLOperation *op, QTextStream &idl, bool is_comment) {
	UMLAttributeList *atl = op->getParmList();
	QString rettype = op->getReturnType();

	idl << spc();
	if (is_comment)
		idl << "// ";
	idl << rettype << " " << cleanName(op->getName()) << " (";
	if (atl->count()) {
		idl << "\n";
		indentlevel++;
		uint i = 0;
		for (UMLAttribute *at = atl->first(); at; at = atl->next()) {
			idl << spc();
			if (is_comment)
				idl << "// ";
			Uml::Parameter_Kind pk = at->getParmKind();
			if (pk == Uml::pk_Out)
				idl << "out ";
			else if (pk == Uml::pk_InOut)
				idl << "inout ";
			else
				idl << "in ";
			idl << at->getTypeName() << " " << cleanName(at->getName());
			if (++i < atl->count())
				idl << ",\n";
		}
		indentlevel--;
	}
	idl << ");\n\n";
}

void IDLWriter::createDefaultDatatypes() {
	m_doc->createDatatype("boolean");
	m_doc->createDatatype("char");
	m_doc->createDatatype("octet");
	m_doc->createDatatype("short");
	m_doc->createDatatype("unsigned short");
	m_doc->createDatatype("long");
	m_doc->createDatatype("unsigned long");
	m_doc->createDatatype("float");
	m_doc->createDatatype("double");
	m_doc->createDatatype("string");
}

