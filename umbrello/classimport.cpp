/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classimport.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "docwindow.h"
#include "package.h"
#include "enum.h"
#include "class.h"
#include "operation.h"
#include "attribute.h"
#include "association.h"
#include "classparser/lexer.h"
#include "classparser/driver.h"
#include "classparser/cpptree2uml.h"
#include <kdebug.h>
#include <qregexp.h>

class CppDriver : public Driver {
public:
	void setupLexer(Lexer* lexer) {
		Driver::setupLexer(lexer);
	/**** Activate this code when ast.h has a NodeType_Comment:
		lexer->setRecordComments(true);
         ****/
	}
};

ClassImport::ClassImport(UMLDoc * parentDoc) {
	m_umldoc = parentDoc;
	m_driver = new CppDriver();
}

ClassImport::~ClassImport() {}

QString ClassImport::doxyComment(QString comment) {
	QStringList lines = QStringList::split("\n", comment);
	if (lines.first() != "/**")
		return "";
	lines.pop_front();  // remove comment start
	lines.pop_back();   // remove comment end
	if (! lines.count())
		return "";
	for (QStringList::Iterator lit = lines.begin(); lit != lines.end(); ++lit) {
		(*lit).remove(QRegExp("^\\s+"));
		(*lit).remove(QRegExp("^\\*+\\s?"));
	}
	return lines.join("\n");
}

UMLObject *ClassImport::createUMLObject(Uml::UMLObject_Type type,
					QString name,
					QString comment,
					UMLPackage *parentPkg) {
	UMLObject * o = m_umldoc->findUMLObject(name);
	if (o == NULL)
		o = m_umldoc->createUMLObject(type, name, parentPkg);
	else
		o->setUMLPackage(parentPkg);
	QString strippedComment = doxyComment(comment);
	if (! strippedComment.isEmpty()) {
		o->setDoc(strippedComment);
		m_umldoc->getDocWindow()->showDocumentation(o, true);
	}
	return o;
}

UMLObject* ClassImport::insertAttribute(UMLObject *o, Uml::Scope scope, QString name,
					   QString type, QString comment /* ="" */,
					   bool isStatic /* =false */) {
	QString strippedComment = doxyComment(comment);
	QString typeName(type);
	int isPointer = typeName.contains('*');
	typeName.replace(QRegExp("^const\\s+"), "");
	typeName.replace(QRegExp("[^:\\w].*$"), "");
	UMLObject *newObj = NULL;
	UMLObject *other = m_umldoc->findUMLObject(typeName);
	if (other != NULL && other->getBaseType() != Uml::ot_Datatype) {
		kdDebug() << "ClassImport::insertAttribute: creating assoc for "
			  << name << endl;
		Uml::Association_Type assocType;
		if (isPointer)
			assocType = Uml::at_Aggregation;
		else
			assocType = Uml::at_Composition;
		UMLAssociation *assoc = new UMLAssociation(m_umldoc, assocType, o, other);
		assoc->setRoleNameB(name);
		assoc->setVisibilityB(scope);
		m_umldoc->addAssociation(assoc);
		newObj = assoc;
	} else {
		Uml::UMLObject_Type ot = o->getBaseType();
		if (ot != Uml::ot_Class) {
			kdDebug() << "ClassImport::insertAttribute: Don't know what to do with "
				  << o->getName() << " (object type " << ot << ")" << endl;
			return NULL;
		}
		int attID = m_umldoc->getUniqueID();
		UMLAttribute *attr = ((UMLClass*)o)->addAttribute(name , attID);
		attr->setTypeName(type);
		attr->setScope(scope);
		attr->setStatic(isStatic);
		newObj = attr;
	}
	if (! strippedComment.isEmpty()) {
		newObj->setDoc(strippedComment);
		m_umldoc->getDocWindow()->showDocumentation(newObj, true);
	}

	m_umldoc->setModified(true);
	return newObj;
}

UMLOperation * ClassImport::insertMethod(UMLObject *o, Uml::Scope scope, QString name,
					 QString type, bool isStatic, bool isAbstract,
					 QString comment /* = "" */,
					 UMLAttributeList *parList /*= NULL*/) {
	if (o->getBaseType() != Uml::ot_Class)
	{
		kdWarning() << "ClassImport::insertMethod called for a non-class: "
			<< o->getName() << "(type " << o->getBaseType() << ")" << endl;
		return NULL;
	}
	UMLClass *klass = static_cast<UMLClass*>(o);
	if(!klass)
	{
		kdWarning()<<"ClassImport::insertMethod() cannot cast object to UMLClass"<<endl;
		return NULL;
	}
	UMLOperation *op = m_umldoc->createOperation( klass, name, parList );
	if(!op)
	{
		kdError()<<"Could not create operation with name "<<name<<endl;
		return NULL;
	}
	op->setScope(scope);
	op->setReturnType(type);
	op->setStatic(isStatic);
	op->setAbstract(isAbstract);
	QString strippedComment = doxyComment(comment);
	if (! strippedComment.isEmpty()) {
		op->setDoc(strippedComment);
		m_umldoc->getDocWindow()->showDocumentation(op, true);
	}
	//setModified(true);
	return op;
}

void ClassImport::importCPP(QStringList headerFileList) {
	for (QStringList::Iterator fileIT = headerFileList.begin();
				   fileIT != headerFileList.end(); ++fileIT) {
		QString fileName = (*fileIT);
		m_driver->parseFile( fileName );
		TranslationUnitAST *ast = m_driver->translationUnit( fileName );
		CppTree2Uml modelFeeder( fileName, this );
		kdDebug() << "Now calling modelFeeder.parseTranslationUnit for file "
			  << fileName << endl;
		modelFeeder.parseTranslationUnit( ast );
	}
}

