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
#include "../classifierlistitem.h"
#include "../umlclassifierlistitemlist.h"
#include "../umltemplatelist.h"
#include "../package.h"
#include "../association.h"
#include "../attribute.h"
#include "../operation.h"
#include "../template.h"
#include "../umlnamespace.h"

const QString AdaWriter::defaultPackageSuffix = "_Holder";

AdaWriter::AdaWriter(UMLDoc *parent, const char *name)
		: SimpleCodeGenerator(parent, name) {
}

AdaWriter::~AdaWriter() {}

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
	Uml::Object_Type ot = c->getBaseType();
	if (ot == Uml::ot_Interface)
		return true;
	if (ot == Uml::ot_Enum)
		return false;
	if (ot != Uml::ot_Class) {
		kdDebug() << "AdaWriter::isOOClass: unknown object type " << ot << endl;
		return false;
	}
	QString stype = c->getStereotype(false);
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
	UMLClassifier* c = (UMLClassifier*)a->getObject(Uml::A);
	typeName = cleanName(c->getName());
	if (! a->getMulti(Uml::A).isEmpty())
		typeName.append("_Array_Access");
	roleName = a->getRoleName(Uml::A);
	if (roleName.isEmpty()) {
		if (a->getMulti(Uml::A).isEmpty()) {
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
				ada << "with " << qualifiedName(con) << "; " << m_endl;
		}
		ada << m_endl;
	}

	// Generate generic formals.
	UMLTemplateList template_params = c->getTemplateList();
	if (template_params.count()) {
		ada << getIndent() << "generic" << m_endl;
		m_indentLevel++;
		for (UMLTemplate *t = template_params.first(); t; t = template_params.next()) {
			QString formalName = t->getName();
			QString typeName = t->getTypeName();
			if (typeName == "class") {
				ada << getIndent() << "type " << formalName << " is tagged private;"
				    << m_endl;
			} else {
				// Check whether it's a data type.
				UMLClassifier *typeObj = t->getType();
				if (typeObj == NULL) {
					kdError() << "AdaWriter::writeClass(template_param "
						  << typeName << "): typeObj is NULL" << endl;
					ada << getIndent() << "type " << formalName << " is new " << typeName
					    << " with private;  -- CHECK: codegen error"
					    << m_endl;
				} else if (typeObj->getBaseType() == Uml::ot_Datatype) {
					ada << getIndent() << formalName << " : " << typeName << ";"
					    << m_endl;
				} else {
					ada << getIndent() << "type " << typeName << " is new "
					    << formalName << " with private;" << m_endl;
				}
			}
		}
		m_indentLevel--;
	}

	// Here comes the package proper.
	QString pkg = qualifiedName(c);
	ada << getIndent() << "package " << pkg << " is" << m_endl << m_endl;
	m_indentLevel++;
	if (c->getBaseType() == Uml::ot_Enum) {
		UMLEnum *ue = static_cast<UMLEnum*>(c);
		UMLClassifierListItemList litList = ue->getFilteredList(Uml::ot_EnumLiteral);
		uint i = 0;
		ada << getIndent() << "type " << classname << " is (" << m_endl;
		m_indentLevel++;
		for (UMLClassifierListItem *lit = litList.first(); lit; lit = litList.next()) {
			QString enumLiteral = cleanName(lit->getName());
			ada << getIndent() << enumLiteral;
			if (++i < litList.count())
				ada << "," << m_endl;
		}
		m_indentLevel--;
		ada << ");" << m_endl << m_endl;
		m_indentLevel--;
		ada << getIndent() << "end " << pkg << ";" << m_endl << m_endl;
		return;
	}
	if (! isOOClass(c)) {
		QString stype = c->getStereotype(false);
		if (stype == "CORBAConstant") {
			ada << getIndent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
		} else if (myClass && myClass->isEnumeration()) {
			UMLAttributeList atl = myClass->getAttributeList();
			UMLAttribute *at;
			ada << getIndent() << "type " << classname << " is (" << m_endl;
			m_indentLevel++;
			uint i = 0;
			for (at = atl.first(); at; at = atl.next()) {
				QString enumLiteral = cleanName(at->getName());
				ada << getIndent() << enumLiteral;
				if (++i < atl.count())
					ada << "," << m_endl;
			}
			m_indentLevel--;
			ada << ");" << m_endl << m_endl;
		} else if(stype == "CORBAStruct") {
			if(myClass) {
				UMLAttributeList atl = myClass->getAttributeList();
				UMLAttribute *at;
				ada << getIndent() << "type " << classname << " is record" << m_endl;
				m_indentLevel++;
				for (at = atl.first(); at; at = atl.next()) {
					QString name = cleanName(at->getName());
					QString typeName = at->getTypeName();
					ada << getIndent() << name << " : " << typeName;
					QString initialVal = at->getInitialValue();
					if (initialVal.latin1() && ! initialVal.isEmpty())
						ada << " := " << initialVal;
					ada << ";" << m_endl;
				}
				m_indentLevel--;
				ada << getIndent() << "end record;" << m_endl << m_endl;
			}
		} else if(stype == "CORBAUnion") {
			ada << getIndent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
		} else if(stype == "CORBATypedef") {
			ada << getIndent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
		} else {
			ada << getIndent() << "-- " << stype << ": Unknown stereotype" << m_endl << m_endl;
		}
		m_indentLevel--;
		ada << getIndent() << "end " << pkg << ";" << m_endl << m_endl;
		return;
	}

	// Write class Documentation if non-empty or if force option set.
	if (forceDoc() || !c->getDoc().isEmpty()) {
		ada << "--" << m_endl;
		ada << "-- class " << classname << endl;
		ada << formatDoc(c->getDoc(), "-- ");
		ada << m_endl;
	}

	UMLClassifierList superclasses = c->getSuperClasses();

	ada << getIndent() << "type Object is ";
	if (c->getAbstract())
		ada << "abstract ";
	if (superclasses.isEmpty()) {
		ada << "tagged ";
	} else {
		// FIXME: Multiple inheritance is not yet supported
		UMLClassifier* parent = superclasses.first();
		ada << "new " << qualifiedName(parent) << ".Object with ";
	}
	ada << "private;" << m_endl << m_endl;
	ada << getIndent() << "type Object_Ptr is access all Object'Class;" << m_endl << m_endl;

	// Generate accessors for public attributes.
	UMLAttributeList atl;
	if(myClass) {
		UMLAttributeList atpub;
		atpub.setAutoDelete(false);

		atl = myClass->getAttributeList();

		UMLAttribute *at;
		for (at = atl.first(); at; at = atl.next()) {
			if (at->getScope() == Uml::Public)
				atpub.append(at);
		}
		if (forceSections() || atpub.count())
			ada << getIndent() << "-- Accessors for public attributes:" << m_endl << m_endl;
		for (at = atpub.first(); at; at = atpub.next()) {
			QString member = cleanName(at->getName());
			ada << getIndent() << "procedure Set_" << member << " (";
			if (! at->getStatic())
				ada << "Self : access Object; ";
			ada << "To : " << at->getTypeName() << ");" << m_endl;
			ada << getIndent() << "function  Get_" << member;
			if (! at->getStatic())
				ada << " (Self : access Object)";
			ada << " return " << at->getTypeName() << ";" << m_endl
<< m_endl;
		}
	}

	// Generate public operations.
	UMLOperationList opl(c->getOpList());
	UMLOperationList oppub;
	oppub.setAutoDelete(false);
	UMLOperation *op;
	for (op = opl.first(); op; op = opl.next()) {
		if (op->getScope() == Uml::Public)
			oppub.append(op);
	}
	if (forceSections() || oppub.count())
		ada << getIndent() << "-- Public methods:" << m_endl << m_endl;
	for (op = oppub.first(); op; op = oppub.next())
		writeOperation(op, ada);

	m_indentLevel--;
	ada << getIndent() << "private" << m_endl << m_endl;
	m_indentLevel++;

	// Generate auxiliary declarations for multiplicity of associations
	UMLAssociationList aggregations = c->getAggregations();
	if (!aggregations.isEmpty()) {
		for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next()) {
			if (a->getMulti(Uml::A).isEmpty())
				continue;
			UMLClassifier* other = (UMLClassifier*)a->getObject(Uml::A);
			QString member = cleanName(other->getName());
			// Handling of packages is missing here
			// A test and error action is missing here for !isOOClass()
			ada << getIndent() << "type " << member << "_Array is array"
			    << " (Positive range <>) of " << member << ".Object_Ptr;" << m_endl;
			ada << getIndent() << "type " << member << "_Array_Access is access "
			    << member << "_array;" << m_endl << m_endl;
		}
	}
	UMLAssociationList compositions = c->getCompositions();
	if (!compositions.isEmpty()) {
		for (UMLAssociation *a = compositions.first(); a; a = compositions.next()) {
			if (a->getMulti(Uml::A).isEmpty())
				continue;
			UMLObject *other = a->getObject(Uml::A);
			QString member = cleanName(other->getName());
			// Handling of packages is missing here
			// Treatment of !isOOClass() is missing here
			ada << getIndent() << "type " << member << "_Array is array"
			    << " (Positive range <>) of " << member << ".Object;" << m_endl;
			ada << getIndent() << "type " << member << "_Array_Access is access "
			    << member << "_array;" << m_endl << m_endl;
		}
	}

	ada << getIndent() << "type Object is ";
	if (c->getAbstract())
		ada << "abstract ";
	if (superclasses.isEmpty()) {
		ada << "tagged ";
	} else {
		// FIXME: Multiple inheritance is not yet supported
		UMLClassifier* parent = superclasses.first();
		ada << "new " << qualifiedName(parent) << ".Object with ";
	}
	ada << "record" << m_endl;
	m_indentLevel++;

	if (forceSections() || !aggregations.isEmpty()) {
		ada << getIndent() << "-- Aggregations:" << m_endl;
		for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next()) {
			QString typeName, roleName;
			computeAssocTypeAndRole(a, typeName, roleName);
			ada << getIndent() << roleName << " : " << typeName << ";" << m_endl;
		}
		ada << endl;
	}
	if (forceSections() || !compositions.isEmpty()) {
		ada << getIndent() << "-- Compositions:" << m_endl;
		for (UMLAssociation *a = compositions.first(); a; a = compositions.next()) {
			QString typeName, roleName;
			computeAssocTypeAndRole(a, typeName, roleName);
			ada << getIndent() << roleName << " : " << typeName << ";" << m_endl;
		}
		ada << endl;
	}

	if (myClass && (forceSections() || atl.count())) {
		ada << getIndent() << "-- Attributes:" << m_endl;
		UMLAttribute *at;
		for (at = atl.first(); at; at = atl.next()) {
			if (at->getStatic())
				continue;
			ada << getIndent() << cleanName(at->getName()) << " : "
			    << at->getTypeName();
			if (at && at->getInitialValue().latin1() && ! at->getInitialValue().isEmpty())
				ada << " := " << at->getInitialValue();
			ada << ";" << m_endl;
		}
	}
	bool haveAttrs = (myClass && atl.count());
	if (aggregations.isEmpty() && compositions.isEmpty() && !haveAttrs)
		ada << getIndent() << "null;" << m_endl;
	m_indentLevel--;
	ada << getIndent() << "end record;" << m_endl << m_endl;
	if (haveAttrs) {
		bool seen_static_attr = false;
		for (UMLAttribute *at = atl.first(); at; at = atl.next()) {
			if (! at->getStatic())
				continue;
			if (! seen_static_attr) {
				ada << getIndent() << "-- Static attributes:" << m_endl;
				seen_static_attr = true;
			}
			ada << getIndent();
			if (at->getScope() == Uml::Private)
				ada << "-- Private:  ";
			ada << cleanName(at->getName()) << " : " << at->getTypeName();
			if (at && at->getInitialValue().latin1() && ! at->getInitialValue().isEmpty())
				ada << " := " << at->getInitialValue();
			ada << ";" << m_endl;
		}
		if (seen_static_attr)
			ada << m_endl;
	}
	// Generate protected operations.
	UMLOperationList opprot;
	opprot.setAutoDelete(false);
	for (op = opl.first(); op; op = opl.next()) {
		if (op->getScope() == Uml::Protected)
			opprot.append(op);
	}
	if (forceSections() || opprot.count())
		ada << getIndent() << "-- Protected methods:" << m_endl << m_endl;
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
		ada << getIndent() << "-- Private methods:" << m_endl << m_endl;
	for (op = oppriv.first(); op; op = oppriv.next())
		writeOperation(op, ada, true);

	m_indentLevel--;
	ada << getIndent() << "end " << pkg << ";" << m_endl << m_endl;
	file.close();
	emit codeGenerated(c, true);
}


void AdaWriter::writeOperation(UMLOperation *op, QTextStream &ada, bool is_comment) {
	UMLAttributeList *atl = op->getParmList();
	QString rettype = op->getTypeName();
	bool use_procedure = (rettype.isEmpty() || rettype == "void");

	ada << getIndent();
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
			ada << ";" << m_endl;
	}
	if (atl->count()) {
		uint i = 0;
		m_indentLevel++;
		for (UMLAttribute *at = atl->first(); at; at = atl->next()) {
			ada << getIndent();
			if (is_comment)
				ada << "-- ";
			ada << cleanName(at->getName()) << " : ";
			Uml::Parameter_Direction pk = at->getParmKind();
			if (pk == Uml::pd_Out)
				ada << "out ";
			else if (pk == Uml::pd_InOut)
				ada << "in out ";
			else
				ada << "in ";
			ada << at->getTypeName();
			if (! at->getInitialValue().isEmpty())
				ada << " := " << at->getInitialValue();
			if (++i < atl->count()) //FIXME gcc warning
				ada << ";" << m_endl;
		}
		m_indentLevel--;
	}
	if (! (op->getStatic() && atl->count() == 0))
		ada << ")";
	if (! use_procedure)
		ada << " return " << rettype;
	ada << " is abstract;" << m_endl << m_endl;
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
 * Check whether the given string is a reserved word for the
 * language of this code generator
 *
 * @param rPossiblyReservedKeyword  The string to check.
 */
bool AdaWriter::isReservedKeyword(const QString & rPossiblyReservedKeyword) {

	const QStringList keywords = reservedKeywords();

	QStringList::ConstIterator it;
	for (it = keywords.begin(); it != keywords.end(); ++it)
		if ((*it).lower() == rPossiblyReservedKeyword.lower())
			return true;

	return false;
}

/**
 * get list of reserved keywords
 */
const QStringList AdaWriter::reservedKeywords() const {

  static QStringList keywords;

  if ( keywords.isEmpty() ) {
    keywords.append( "abort" );
    keywords.append( "abs" );
    keywords.append( "abstract" );
    keywords.append( "accept" );
    keywords.append( "access" );
    keywords.append( "aliased" );
    keywords.append( "all" );
    keywords.append( "and" );
    keywords.append( "Argument_Error" );
    keywords.append( "array" );
    keywords.append( "Assert_Failure" );
    keywords.append( "at" );
    keywords.append( "begin" );
    keywords.append( "body" );
    keywords.append( "Boolean" );
    keywords.append( "case" );
    keywords.append( "Character" );
    keywords.append( "constant" );
    keywords.append( "Constraint_Error" );
    keywords.append( "Conversion_Error" );
    keywords.append( "Data_Error" );
    keywords.append( "declare" );
    keywords.append( "delay" );
    keywords.append( "delta" );
    keywords.append( "Dereference_Error" );
    keywords.append( "Device_Error" );
    keywords.append( "digits" );
    keywords.append( "do" );
    keywords.append( "Duration" );
    keywords.append( "else" );
    keywords.append( "elsif" );
    keywords.append( "end" );
    keywords.append( "End_Error" );
    keywords.append( "entry" );
    keywords.append( "exception" );
    keywords.append( "exit" );
    keywords.append( "false" );
    keywords.append( "Float" );
    keywords.append( "for" );
    keywords.append( "function" );
    keywords.append( "generic" );
    keywords.append( "goto" );
    keywords.append( "if" );
    keywords.append( "in" );
    keywords.append( "Index_Error" );
    keywords.append( "Integer" );
    keywords.append( "is" );
    keywords.append( "Layout_Error" );
    keywords.append( "Length_Error" );
    keywords.append( "limited" );
    keywords.append( "Long_Float" );
    keywords.append( "Long_Integer" );
    keywords.append( "Long_Long_Float" );
    keywords.append( "Long_Long_Integer" );
    keywords.append( "loop" );
    keywords.append( "mod" );
    keywords.append( "Mode_Error" );
    keywords.append( "Name_Error" );
    keywords.append( "Natural" );
    keywords.append( "new" );
    keywords.append( "not" );
    keywords.append( "null" );
    keywords.append( "of" );
    keywords.append( "or" );
    keywords.append( "others" );
    keywords.append( "out" );
    keywords.append( "package" );
    keywords.append( "Pattern_Error" );
    keywords.append( "Picture_Error" );
    keywords.append( "Pointer_Error" );
    keywords.append( "Positive" );
    keywords.append( "pragma" );
    keywords.append( "private" );
    keywords.append( "procedure" );
    keywords.append( "Program_Error" );
    keywords.append( "protected" );
    keywords.append( "raise" );
    keywords.append( "range" );
    keywords.append( "record" );
    keywords.append( "rem" );
    keywords.append( "renames" );
    keywords.append( "requeue" );
    keywords.append( "return" );
    keywords.append( "reverse" );
    keywords.append( "select" );
    keywords.append( "separate" );
    keywords.append( "Short_Float" );
    keywords.append( "Short_Integer" );
    keywords.append( "Short_Short_Float" );
    keywords.append( "Short_Short_Integer" );
    keywords.append( "Status_Error" );
    keywords.append( "Storage_Error" );
    keywords.append( "String" );
    keywords.append( "subtype" );
    keywords.append( "Tag_Error" );
    keywords.append( "tagged" );
    keywords.append( "task" );
    keywords.append( "Tasking_Error" );
    keywords.append( "terminate" );
    keywords.append( "Terminator_Error" );
    keywords.append( "then" );
    keywords.append( "Time_Error" );
    keywords.append( "Translation_Error" );
    keywords.append( "true" );
    keywords.append( "type" );
    keywords.append( "until" );
    keywords.append( "Update_Error" );
    keywords.append( "use" );
    keywords.append( "Use_Error" );
    keywords.append( "when" );
    keywords.append( "while" );
    keywords.append( "Wide_Character" );
    keywords.append( "Wide_String" );
    keywords.append( "with" );
    keywords.append( "xor" );
  }

  return keywords;
}

#include "adawriter.moc"
