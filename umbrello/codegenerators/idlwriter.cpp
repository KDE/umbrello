/***************************************************************************
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
#include "../classifierlistitem.h"
#include "../umlclassifierlistitemlist.h"
#include "../package.h"
#include "../association.h"
#include "../attribute.h"
#include "../operation.h"
#include "../umlnamespace.h"

IDLWriter::IDLWriter(UMLDoc *parent, const char *name)
		: SimpleCodeGenerator(parent, name, false) {
	indentlevel = 0;
	// FIXME: Eventually we should fabricate an Indenter class
	// that can be used by all code generators.
	// NOTE: this has now been done in new code gen class. -b.t.
}

IDLWriter::~IDLWriter() {}

QString IDLWriter::spc() {
	QString s;
	if (m_indentation.contains(' '))
		s.fill(' ', indentlevel * m_indentationAmount);
	else
		s.fill('\t', indentlevel * m_indentationAmount);
	return s;
}

bool IDLWriter::isOOClass(UMLClassifier *c) {
	QString stype = c->getStereotype(false);
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

/**
 * returns "IDL"
 */
QString IDLWriter::getLanguage() {
        return "IDL";
}

/**
 * checks whether type is "IDLWriter"
 *
 * @param type
 */
bool IDLWriter::isType (QString & type)
{
   if(type == "IDLWriter")
        return true;
   return false;
}

void IDLWriter::computeAssocTypeAndRole
       (UMLAssociation *a, UMLClassifier *c, QString& typeName, QString& roleName)
{
	// Determine which is the "remote" end of the association:
	bool IAmRoleA = true;
	UMLObject *other = a->getObject(Uml::B);
	if (c->getName() == other->getName()) {
		IAmRoleA = false;
		other = a->getObject(Uml::A);
	}
	// Construct the type name:
	typeName = cleanName(other->getName());
	QString multiplicity;
	if (IAmRoleA)
		multiplicity = a->getMulti(Uml::B);
	else
		multiplicity = a->getMulti(Uml::A);
	if (!multiplicity.isEmpty() && multiplicity != "1")
		typeName.append("Vector");
	// Construct the member name:
	if (IAmRoleA)
		roleName = a->getRoleName(Uml::B);
	else
		roleName = a->getRoleName(Uml::A);
	if (roleName.isEmpty()) {
		roleName = a->getName();
		if (roleName.isEmpty()) {
			roleName = "m_" + typeName;
		}
	}
}

void IDLWriter::writeClass(UMLClassifier *c) {
	if (!c) {
		kdDebug() << "Cannot write class of NULL concept!" << endl;
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
				idl << "#include \"" << baseName << ".idl\"" << m_newLineEndingChars;
		}
		idl << m_newLineEndingChars;
	}

	// Generate the module declaration(s) for the package(s) in which
	// we are embedded.
	UMLPackageList pkgList = c->getPackages();
	UMLPackage *pkg;
	for (pkg = pkgList.first(); pkg != NULL; pkg = pkgList.next()) {
		idl << spc() << "module " << pkg->getName() << " {" << m_newLineEndingChars << m_newLineEndingChars;
		indentlevel++;
	}

	// Write class Documentation if non-empty or if force option set.
	if (forceDoc() || !c->getDoc().isEmpty()) {
		idl << "//" << m_newLineEndingChars;
		idl << "// class " << classname << endl;
		idl << formatDoc(c->getDoc(), "// ");
		idl << m_newLineEndingChars;
	}

	if (c->getBaseType() == Uml::ot_Enum) {
		UMLEnum *ue = static_cast<UMLEnum*>(c);
		UMLClassifierListItemList litList = ue->getFilteredList(Uml::ot_EnumLiteral);
		uint i = 0;
		idl << spc() << "enum " << classname << " {" << m_newLineEndingChars;
		indentlevel++;
		for (UMLClassifierListItem *lit = litList.first(); lit; lit = litList.next()) {
			QString enumLiteral = cleanName(lit->getName());
			idl << spc() << enumLiteral;
			if (++i < litList.count())
				idl << ",";
			idl << endl;
		}
		indentlevel--;
		idl << spc() << "};" << m_newLineEndingChars << m_newLineEndingChars;
		// Close the modules inside which we might be nested.
		for (pkg = pkgList.first(); pkg != NULL; pkg = pkgList.next()) {
			indentlevel--;
			idl << spc() << "};" << m_newLineEndingChars << m_newLineEndingChars;
		}
		return;
	}
	if (! isOOClass(c)) {
		QString stype = c->getStereotype(false);
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
			UMLAttributeList atl = myClass->getFilteredAttributeList();
			UMLAttribute *at;
			idl << spc() << "enum " << classname << " {" << m_newLineEndingChars;
			indentlevel++;
			uint i = 0;
			for (at = atl.first(); at; at = atl.next()) {
				QString enumLiteral = cleanName(at->getName());
				idl << spc() << enumLiteral;
				if (++i < atl.count())
					idl << ",";
				idl << endl;
			}
			indentlevel--;
			idl << spc() << "};" << m_newLineEndingChars << m_newLineEndingChars;
		} else if (stype == "CORBAStruct") {
			UMLAttributeList atl = myClass->getFilteredAttributeList();
			UMLAttribute *at;
			idl << spc() << "struct " << classname << " {" << m_newLineEndingChars;
			indentlevel++;
			for (at = atl.first(); at; at = atl.next()) {
				QString name = cleanName(at->getName());
				idl << spc() << at->getTypeName() << " " << name << ";" << m_newLineEndingChars;
				// Initial value not possible in IDL.
			}
			UMLAssociationList compositions = c->getCompositions();
			if (!compositions.isEmpty()) {
				idl << spc() << "// Compositions." << m_newLineEndingChars;
				for (UMLAssociation *a = compositions.first(); a; a = compositions.next()) {
					QString memberType, memberName;
					computeAssocTypeAndRole(a, c, memberType, memberName);
					idl << spc() << memberType << " " << memberName << ";" << m_newLineEndingChars;
				}
			}
			UMLAssociationList aggregations = c->getAggregations();
			if (!aggregations.isEmpty()) {
				idl << spc() << "// Aggregations." << m_newLineEndingChars;
				for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next()) {
					QString memberType, memberName;
					computeAssocTypeAndRole(a, c, memberType, memberName);
					idl << spc() << memberType << " " << memberName << ";" << m_newLineEndingChars;
				}
			}
			indentlevel--;
			idl << spc() << "};" << m_newLineEndingChars << m_newLineEndingChars;
		} else if (stype == "CORBAUnion") {
			idl << spc() << "// " << stype << " " << c->getName()
			    << " is Not Yet Implemented" << m_newLineEndingChars << m_newLineEndingChars;
		} else if (stype == "CORBATypedef") {
			UMLClassifierList superclasses = c->getSuperClasses();
			UMLClassifier* firstParent = superclasses.first();
			idl << spc() << "typedef " << firstParent->getName() << " "
			    << c->getName() << ";" << m_newLineEndingChars << m_newLineEndingChars;
		} else {
			idl << spc() << "// " << stype << ": Unknown stereotype" << m_newLineEndingChars << m_newLineEndingChars;
		}
		// Close the modules inside which we might be nested.
		for (pkg = pkgList.first(); pkg != NULL; pkg = pkgList.next()) {
			indentlevel--;
			idl << spc() << "};" << m_newLineEndingChars << m_newLineEndingChars;
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
	idl << " {" << m_newLineEndingChars << m_newLineEndingChars;
	indentlevel++;

	// Generate auxiliary declarations for multiplicity of associations
	UMLAssociation *a;
	bool didComment = false;
	UMLAssociationList aggregations = c->getAggregations();
	for (a = aggregations.first(); a; a = aggregations.next()) {
		QString multiplicity = a->getMulti(Uml::A);
		if (multiplicity.isEmpty() || multiplicity == "1")
			continue;
		if (!didComment) {
			idl << spc() << "// Types for association multiplicities" << m_newLineEndingChars << m_newLineEndingChars;
			didComment = true;
		}
		UMLClassifier* other = (UMLClassifier*)m_doc->findObjectById(a->getRoleId(Uml::A));
		QString bareName = cleanName(other->getName());
		idl << spc() << "typedef sequence<" << qualifiedName(other) << "> "
		    << bareName << "Vector;" << m_newLineEndingChars << m_newLineEndingChars;
	}

	UMLAssociationList compositions = c->getCompositions();
	for (a = compositions.first(); a; a = compositions.next()) {
		QString multiplicity = a->getMulti(Uml::A);
		if (multiplicity.isEmpty() || multiplicity == "1")
			continue;
		if (!didComment) {
			idl << spc() << "// Types for association multiplicities" << m_newLineEndingChars << m_newLineEndingChars;
			didComment = true;
		}
		UMLClassifier* other = (UMLClassifier*)m_doc->findObjectById(a->getRoleId(Uml::A));
		QString bareName = cleanName(other->getName());
		idl << spc() << "typedef sequence<" << qualifiedName(other) << "> "
		    << bareName << "Vector;" << m_newLineEndingChars << m_newLineEndingChars;
	}

	// Generate public attributes.
	if(myClass) {
		UMLAttributeList atl = myClass->getFilteredAttributeList();
		UMLAttributeList atpub;
		UMLAttribute *at;
		for (at = atl.first(); at; at = atl.next()) {
			if (at->getScope() == Uml::Public)
				atpub.append(at);
		}
		if (forceSections() || atpub.count()) {
			idl << spc() << "// Public attributes:" << m_newLineEndingChars << m_newLineEndingChars;
			for (at = atpub.first(); at; at = atpub.next()) {
				QString attName = cleanName(at->getName());
				idl << spc() << "attribute " << at->getTypeName()
				    << " " << attName << ";" << m_newLineEndingChars << m_newLineEndingChars;
			}
		}
	}

	// Generate public operations.
	UMLOperationList opl(c->getOpList());
	UMLOperationList oppub;
	UMLOperation *op;
	for (op = opl.first(); op; op = opl.next()) {
		if (op->getScope() == Uml::Public)
			oppub.append(op);
	}
	if (forceSections() || oppub.count()) {
		idl << spc() << "// Public methods:" << m_newLineEndingChars << m_newLineEndingChars;
		for (op = oppub.first(); op; op = oppub.next())
			writeOperation(op, idl);
		idl << m_newLineEndingChars;
	}


	if (forceSections() || !aggregations.isEmpty()) {
		idl << spc() << "// Aggregations:" << m_newLineEndingChars;
		for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next()) {
			QString typeName, roleName;
			computeAssocTypeAndRole(a, c, typeName, roleName);
			idl << spc() << "// " << typeName << " " << roleName << ";" << m_newLineEndingChars;
		}
		idl << endl;
	}
	if (forceSections() || !compositions.isEmpty()) {
		idl << spc() << "// Compositions:" << m_newLineEndingChars;
		for (UMLAssociation *a = compositions.first(); a; a = compositions.next()) {
			QString typeName, roleName;
			computeAssocTypeAndRole(a, c, typeName, roleName);
			idl << spc() << "// " << typeName << " " << roleName << ";" << m_newLineEndingChars;
		}
		idl << endl;
	}

	indentlevel--;
	idl << spc() << "};" << m_newLineEndingChars << m_newLineEndingChars;

	// Close the modules inside which we might be nested.
	for (pkg = pkgList.first(); pkg != NULL; pkg = pkgList.next()) {
		indentlevel--;
		idl << spc() << "};" << m_newLineEndingChars << m_newLineEndingChars;
	}
	file.close();
	emit codeGenerated(c, true);
}


void IDLWriter::writeOperation(UMLOperation *op, QTextStream &idl, bool is_comment) {
	UMLAttributeList *atl = op->getParmList();
	QString rettype = op->getTypeName();

	idl << spc();
	if (is_comment)
		idl << "// ";
	idl << rettype << " " << cleanName(op->getName()) << " (";
	if (atl->count()) {
		idl << m_newLineEndingChars;
		indentlevel++;
		uint i = 0;
		for (UMLAttribute *at = atl->first(); at; at = atl->next()) {
			idl << spc();
			if (is_comment)
				idl << "// ";
			Uml::Parameter_Direction pk = at->getParmKind();
			if (pk == Uml::pd_Out)
				idl << "out ";
			else if (pk == Uml::pd_InOut)
				idl << "inout ";
			else
				idl << "in ";
			idl << at->getTypeName() << " " << cleanName(at->getName());
			if (++i < atl->count())
				idl << "," << m_newLineEndingChars;
		}
		indentlevel--;
	}
	idl << ");" << m_newLineEndingChars << m_newLineEndingChars;
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

const QStringList IDLWriter::reservedKeywords() const {

  static QStringList keywords;

  if (keywords.isEmpty()) {
    keywords << "any"
             << "attribute"
             << "boolean"
             << "case"
             << "char"
             << "const"
             << "context"
             << "default"
             << "double"
             << "enum"
             << "exception"
             << "FALSE"
             << "float"
             << "in"
             << "inout"
             << "interface"
             << "long"
             << "module"
             << "octet"
             << "oneway"
             << "out"
             << "raises"
             << "readonly"
             << "sequence"
             << "short"
             << "string"
             << "struct"
             << "switch"
             << "TRUE"
             << "typedef"
             << "union"
             << "unsigned"
             << "void";
  }

  return keywords;
}


