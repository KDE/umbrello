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
	if (m_indentation == " ")
		s.fill(' ', indentlevel * m_indentationAmount);
	else
		s.fill('\t', indentlevel * m_indentationAmount);
	return s;
}

/**
 * returns "Ada"
 */
QString AdaWriter::getLanguage() {
        return "Ada";
}

/**
 * checks whether type is "AdaWriter"
 *
 * @param type
 */
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
		kdDebug() << "AdaWriter::isOOClass: unknown object type " << ot << endl;
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
	UMLClassifier* c = (UMLClassifier*) m_doc->findUMLObject(a->getRoleId(A));
	typeName = cleanName(c->getName());
	if (! a->getMulti(A).isEmpty())
		typeName.append("_Array_Access");
	roleName = a->getRoleName(A);
	if (roleName.isEmpty()) {
		if (a->getMulti(A).isEmpty()) {
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
		kdDebug() << "Cannot write class of NULL concept!" << endl;
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
		for (UMLClassifier *con = imports.first(); con; con = imports.next()) {
			if (con->getBaseType() != Uml::ot_Datatype)
				ada << "with " << qualifiedName(con) << "; " << m_newLineEndingChars;
		}
		ada << m_newLineEndingChars;
	}

	QString pkg = qualifiedName(c);
	ada << spc() << "package " << pkg << " is" << m_newLineEndingChars << m_newLineEndingChars;
	indentlevel++;
	if (c->getBaseType() == Uml::ot_Enum) {
		UMLEnum *ue = static_cast<UMLEnum*>(c);
		UMLEnumLiteralList litList = ue->getFilteredEnumLiteralList();
		uint i = 0;
		ada << spc() << "type " << classname << " is (" << m_newLineEndingChars;
		indentlevel++;
		for (UMLEnumLiteral *lit = litList.first(); lit; lit = litList.next()) {
			QString enumLiteral = cleanName(lit->getName());
			ada << spc() << enumLiteral;
			if (++i < litList.count())
				ada << "," << m_newLineEndingChars;
		}
		indentlevel--;
		ada << ");" << m_newLineEndingChars << m_newLineEndingChars;
		indentlevel--;
		ada << spc() << "end " << pkg << ";" << m_newLineEndingChars << m_newLineEndingChars;
		return;
	}
	if (! isOOClass(c)) {
		QString stype = c->getStereotype();
		if (stype == "CORBAConstant") {
			ada << spc() << "-- " << stype << " is Not Yet Implemented" << m_newLineEndingChars << m_newLineEndingChars;
		} else if (myClass && myClass->isEnumeration()) {
			UMLAttributeList *atl = myClass->getFilteredAttributeList();
			UMLAttribute *at;
			ada << spc() << "type " << classname << " is (" << m_newLineEndingChars;
			indentlevel++;
			uint i = 0;
			for (at = atl->first(); at; at = atl->next()) {
				QString enumLiteral = cleanName(at->getName());
				ada << spc() << enumLiteral;
				if (++i < atl->count())
					ada << "," << m_newLineEndingChars;
			}
			indentlevel--;
			ada << ");" << m_newLineEndingChars << m_newLineEndingChars;
		} else if(stype == "CORBAStruct") {
			if(myClass) {
				UMLAttributeList *atl = myClass->getFilteredAttributeList();
				UMLAttribute *at;
				ada << spc() << "type " << classname << " is record" << m_newLineEndingChars;
				indentlevel++;
				for (at = atl->first(); at; at = atl->next()) {
					QString name = cleanName(at->getName());
					QString typeName = at->getTypeName();
					ada << spc() << name << " : " << typeName;
					QString initialVal = at->getInitialValue();
					if (initialVal.latin1() && ! initialVal.isEmpty())
						ada << " := " << initialVal;
					ada << ";" << m_newLineEndingChars;
				}
				indentlevel--;
				ada << spc() << "end record;" << m_newLineEndingChars << m_newLineEndingChars;
			}
		} else if(stype == "CORBAUnion") {
			ada << spc() << "-- " << stype << " is Not Yet Implemented" << m_newLineEndingChars << m_newLineEndingChars;
		} else if(stype == "CORBATypedef") {
			ada << spc() << "-- " << stype << " is Not Yet Implemented" << m_newLineEndingChars << m_newLineEndingChars;
		} else {
			ada << spc() << "-- " << stype << ": Unknown stereotype" << m_newLineEndingChars << m_newLineEndingChars;
		}
		indentlevel--;
		ada << spc() << "end " << pkg << ";" << m_newLineEndingChars << m_newLineEndingChars;
		return;
	}

	// Write class Documentation if non-empty or if force option set.
	if (forceDoc() || !c->getDoc().isEmpty()) {
		ada << "--" << m_newLineEndingChars;
		ada << "-- class " << classname << endl;
		ada << formatDoc(c->getDoc(), "-- ");
		ada << m_newLineEndingChars;
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
	ada << "private;" << m_newLineEndingChars << m_newLineEndingChars;
	ada << spc() << "type Object_Ptr is access all Object'Class;" << m_newLineEndingChars << m_newLineEndingChars;

	// Generate accessors for public attributes.
	UMLAttributeList *atl = NULL;
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
			ada << spc() << "-- Accessors for public attributes:" << m_newLineEndingChars << m_newLineEndingChars;
		for (at = atpub.first(); at; at = atpub.next()) {
			QString member = cleanName(at->getName());
			ada << spc() << "procedure Set_" << member << " (";
			if (! at->getStatic())
				ada << "Self : access Object; ";
			ada << "To : " << at->getTypeName() << ");" << m_newLineEndingChars;
			ada << spc() << "function  Get_" << member;
			if (! at->getStatic())
				ada << " (Self : access Object)";
			ada << " return " << at->getTypeName() << ";" << m_newLineEndingChars
<< m_newLineEndingChars;
		}
	}

	// Generate public operations.
	UMLOperationList opl(c->getFilteredOperationsList());
	UMLOperationList oppub;
	oppub.setAutoDelete(false);
	UMLOperation *op;
	for (op = opl.first(); op; op = opl.next()) {
		if (op->getScope() == Uml::Public)
			oppub.append(op);
	}
	if (forceSections() || oppub.count())
		ada << spc() << "-- Public methods:" << m_newLineEndingChars << m_newLineEndingChars;
	for (op = oppub.first(); op; op = oppub.next())
		writeOperation(op, ada);

	indentlevel--;
	ada << spc() << "private" << m_newLineEndingChars << m_newLineEndingChars;
	indentlevel++;

	// Generate auxiliary declarations for multiplicity of associations
	UMLAssociationList aggregations = c->getAggregations();
	if (!aggregations.isEmpty()) {
		for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next()) {
			if (a->getMulti(A).isEmpty())
				continue;
			UMLClassifier* other = (UMLClassifier*) m_doc->findUMLObject(a->getRoleId(A));
			QString member = cleanName(other->getName());
			// Handling of packages is missing here
			// A test and error action is missing here for !isOOClass()
			ada << spc() << "type " << member << "_Array is array"
			    << " (Positive range <>) of " << member << ".Object_Ptr;" << m_newLineEndingChars;
			ada << spc() << "type " << member << "_Array_Access is access "
			    << member << "_array;" << m_newLineEndingChars << m_newLineEndingChars;
		}
	}
	UMLAssociationList compositions = c->getCompositions();
	if (!compositions.isEmpty()) {
		for (UMLAssociation *a = compositions.first(); a; a = compositions.next()) {
			if (a->getMulti(A).isEmpty())
				continue;
			UMLObject *other = m_doc->findUMLObject(a->getRoleId(A));
			QString member = cleanName(other->getName());
			// Handling of packages is missing here
			// Treatment of !isOOClass() is missing here
			ada << spc() << "type " << member << "_Array is array"
			    << " (Positive range <>) of " << member << ".Object;" << m_newLineEndingChars;
			ada << spc() << "type " << member << "_Array_Access is access "
			    << member << "_array;" << m_newLineEndingChars << m_newLineEndingChars;
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
	ada << "record" << m_newLineEndingChars;
	indentlevel++;

	if (forceSections() || !aggregations.isEmpty()) {
		ada << spc() << "-- Aggregations:" << m_newLineEndingChars;
		for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next()) {
			QString typeName, roleName;
			computeAssocTypeAndRole(a, typeName, roleName);
			ada << spc() << roleName << " : " << typeName << ";" << m_newLineEndingChars;
		}
		ada << endl;
	}
	if (forceSections() || !compositions.isEmpty()) {
		ada << spc() << "-- Compositions:" << m_newLineEndingChars;
		for (UMLAssociation *a = compositions.first(); a; a = compositions.next()) {
			QString typeName, roleName;
			computeAssocTypeAndRole(a, typeName, roleName);
			ada << spc() << roleName << " : " << typeName << ";" << m_newLineEndingChars;
		}
		ada << endl;
	}

	if (myClass && (forceSections() || atl->count())) {
		ada << spc() << "-- Attributes:" << m_newLineEndingChars;
		UMLAttribute *at;
		for (at = atl->first(); at; at = atl->next()) {
			if (at->getStatic())
				continue;
			ada << spc() << cleanName(at->getName()) << " : "
			    << at->getTypeName();
			if (at && at->getInitialValue().latin1() && ! at->getInitialValue().isEmpty())
				ada << " := " << at->getInitialValue();
			ada << ";" << m_newLineEndingChars;
		}
	}
	bool haveAttrs = (myClass && atl->count());
	if (aggregations.isEmpty() && compositions.isEmpty() && !haveAttrs)
		ada << spc() << "null;" << m_newLineEndingChars;
	indentlevel--;
	ada << spc() << "end record;" << m_newLineEndingChars << m_newLineEndingChars;
	if (haveAttrs) {
		bool seen_static_attr = false;
		for (UMLAttribute *at = atl->first(); at; at = atl->next()) {
			if (! at->getStatic())
				continue;
			if (! seen_static_attr) {
				ada << spc() << "-- Static attributes:" << m_newLineEndingChars;
				seen_static_attr = true;
			}
			ada << spc();
			if (at->getScope() == Uml::Private)
				ada << "-- Private:  ";
			ada << cleanName(at->getName()) << " : " << at->getTypeName();
			if (at && at->getInitialValue().latin1() && ! at->getInitialValue().isEmpty())
				ada << " := " << at->getInitialValue();
			ada << ";" << m_newLineEndingChars;
		}
		if (seen_static_attr)
			ada << m_newLineEndingChars;
	}
	// Generate protected operations.
	UMLOperationList opprot;
	opprot.setAutoDelete(false);
	for (op = opl.first(); op; op = opl.next()) {
		if (op->getScope() == Uml::Protected)
			opprot.append(op);
	}
	if (forceSections() || opprot.count())
		ada << spc() << "-- Protected methods:" << m_newLineEndingChars << m_newLineEndingChars;
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
	for (op = opl.first(); op; op = opl.next()) {
		if (op->getScope() == Uml::Private)
			oppriv.append(op);
	}
	if (forceSections() || oppriv.count())
		ada << spc() << "-- Private methods:" << m_newLineEndingChars << m_newLineEndingChars;
	for (op = oppriv.first(); op; op = oppriv.next())
		writeOperation(op, ada, true);

	indentlevel--;
	ada << spc() << "end " << pkg << ";" << m_newLineEndingChars << m_newLineEndingChars;
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
			ada << ";" << m_newLineEndingChars;
	}
	if (atl->count()) {
		uint i = 0;
		indentlevel++;
		for (UMLAttribute *at = atl->first(); at; at = atl->next()) {
			ada << spc();
			if (is_comment)
				ada << "-- ";
			ada << cleanName(at->getName()) << " : ";
			Uml::Parameter_Kind pk = at->getParmKind();
			if (pk == Uml::pk_Out)
				ada << "out ";
			else if (pk == Uml::pk_InOut)
				ada << "in out ";
			else
				ada << "in ";
			ada << at->getTypeName();
			if (! at->getInitialValue().isEmpty())
				ada << " := " << at->getInitialValue();
			if (++i < atl->count()) //FIXME gcc warning
				ada << ";" << m_newLineEndingChars;
		}
		indentlevel--;
	}
	if (! (op->getStatic() && atl->count() == 0))
		ada << ")";
	if (! use_procedure)
		ada << " return " << rettype;
	ada << " is abstract;" << m_newLineEndingChars << m_newLineEndingChars;
	// TBH, we make the methods abstract here because we don't have the means
	// for generating meaningful implementations.
}

void AdaWriter::createDefaultDatatypes() {
	m_doc->createDatatype("Boolean");
	m_doc->createDatatype("Character");
	m_doc->createDatatype("Positive");
	m_doc->createDatatype("Natural");
	m_doc->createDatatype("Integer");
	m_doc->createDatatype("Short_Integer");
	m_doc->createDatatype("Long_Integer");
	m_doc->createDatatype("Float");
	m_doc->createDatatype("Long_Float");
	m_doc->createDatatype("String");
}

/**
 * List of reserved keywords for this code generator.
 *
 * Just add new keywords, then mark all lines and
 * pipe it through the external 'sort' program.
 */
static const char *ReservedWords[] = {
  "abort",
  "abs",
  "abstract",
  "accept",
  "access",
  "aliased",
  "all",
  "and",
  "Argument_Error",
  "array",
  "Assert_Failure",
  "at",
  "begin",
  "body",
  "Boolean",
  "case",
  "Character",
  "constant",
  "Constraint_Error",
  "Conversion_Error",
  "Data_Error",
  "declare",
  "delay",
  "delta",
  "Dereference_Error",
  "Device_Error",
  "digits",
  "do",
  "Duration",
  "else",
  "elsif",
  "end",
  "End_Error",
  "entry",
  "exception",
  "exit",
  "false",
  "Float",
  "for",
  "function",
  "generic",
  "goto",
  "if",
  "in",
  "Index_Error",
  "Integer",
  "is",
  "Layout_Error",
  "Length_Error",
  "limited",
  "Long_Float",
  "Long_Integer",
  "Long_Long_Float",
  "Long_Long_Integer",
  "loop",
  "mod",
  "Mode_Error",
  "Name_Error",
  "Natural",
  "new",
  "not",
  "null",
  "of",
  "or",
  "others",
  "out",
  "package",
  "Pattern_Error",
  "Picture_Error",
  "Pointer_Error",
  "Positive",
  "pragma",
  "private",
  "procedure",
  "Program_Error",
  "protected",
  "raise",
  "range",
  "record",
  "rem",
  "renames",
  "requeue",
  "return",
  "reverse",
  "select",
  "separate",
  "Short_Float",
  "Short_Integer",
  "Short_Short_Float",
  "Short_Short_Integer",
  "Status_Error",
  "Storage_Error",
  "String",
  "subtype",
  "Tag_Error",
  "tagged",
  "task",
  "Tasking_Error",
  "terminate",
  "Terminator_Error",
  "then",
  "Time_Error",
  "Translation_Error",
  "true",
  "type",
  "until",
  "Update_Error",
  "use",
  "Use_Error",
  "when",
  "while",
  "Wide_Character",
  "Wide_String",
  "with",
  "xor",
  NULL
};

/**
 * Check whether the given string is a reserved word for the
 * language of this code generator
 *
 * @param rPossiblyReservedKeyword  The string to check.
 */
bool AdaWriter::isReservedKeyword(const QString & rPossiblyReservedKeyword) {
  const char **tmpReservedWords = getReservedKeywords();

  if (tmpReservedWords == NULL)
  {
    return false;
  }

  while (tmpReservedWords[0] != NULL) {
		QString keyword(tmpReservedWords[0]);

		if (keyword.lower() == rPossiblyReservedKeyword.lower()) {
			return true;
		}

    tmpReservedWords++;
	}

	return false;
}

/**
 * get list of reserved keywords
 */
const char **
AdaWriter::getReservedKeywords() {
  return ReservedWords;
}

#include "adawriter.moc"

