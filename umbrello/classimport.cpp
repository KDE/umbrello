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
					QString stereotype,
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
	o->setStereotype(stereotype);
	return o;
}

UMLObject* ClassImport::insertAttribute(UMLClass *o, Uml::Scope scope, QString name,
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
		UMLAssociation *assoc = new UMLAssociation(assocType, o, other);
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
		UMLAttribute *attr = ((UMLClass*)o)->addAttribute(name);
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

UMLOperation * ClassImport::insertMethod(UMLClass *klass, Uml::Scope scope, QString name,
					 QString type, bool isStatic, bool isAbstract,
					 QString comment /* = "" */,
					 UMLAttributeList *parList /*= NULL*/) {
	if (klass == NULL) {
		kdWarning() << "ClassImport::insertMethod: NULL class pointer for "
			    << name << endl;
		return NULL;
	}
	if (parList == NULL) {
		parList = new UMLAttributeList();
		// This prevents UMLDoc::createOperation() from entering into
		// interactive mode.
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

UMLAttribute* ClassImport::addMethodParameter(UMLOperation *method,
					      QString type, QString name,
					      QString initialValue, QString doc,
					      Uml::Parameter_Kind kind) {
	// We don't necessarily expose the full declaration of UMLOperation
	// to clients of ClassImport.  I.e. if clients only see a pointer
	// to the forward declaration of UMLOperation, they can't call any
	// methods on that pointer.
	// That's the raison d'etre for these thin wrappers.
	return method->addParm(type, name, initialValue, doc, kind);
}

void ClassImport::addEnumLiteral(UMLEnum *enumType, QString literal) {
	// Why an extra wrapper? See comment at addMethodParameter()
	enumType->addEnumLiteral( literal, m_umldoc->getUniqueID() );
}

void ClassImport::createGeneralization(UMLClass *child, QString parentName) {
	UMLObject *parent = m_umldoc->findUMLObject( parentName, Uml::ot_Class );
	if (parent == NULL) {
	    kdDebug() << "ClassImport::createGeneralization: Could not find UML object for "
		      << parentName << endl;
	    return;
	}
	UMLAssociation *assoc = new UMLAssociation( Uml::at_Generalization,
						    child, parent );
	m_umldoc->addAssociation(assoc);
}

void ClassImport::importCPP(QStringList headerFileList) {
	// Reset the driver
	m_driver->reset();
	// Add some standard include paths
	m_driver->addIncludePath( "/usr/include" );
	m_driver->addIncludePath( "/usr/include/c++" );
	m_driver->addIncludePath( "/usr/include/g++" );
	m_driver->addIncludePath( "/usr/local/include" );
	// FIXME: The following hack is to be replaced by a config menu in umbrello
	char *umbrello_incpath = getenv( "UMBRELLO_INCPATH" );
	if (umbrello_incpath) {
		QStringList includes = QStringList::split( ':', umbrello_incpath );
		for (QStringList::Iterator i = includes.begin();
					   i != includes.end(); i++)
			m_driver->addIncludePath( *i );
	}
	for (QStringList::Iterator fileIT = headerFileList.begin();
				   fileIT != headerFileList.end(); ++fileIT) {
		QString fileName = (*fileIT);
		m_umldoc->writeToStatusBar("Importing file: " + fileName);
		m_driver->parseFile( fileName );
		TranslationUnitAST *ast = m_driver->translationUnit( fileName );
		if (ast == NULL)
			continue;
		CppTree2Uml modelFeeder( fileName, this );
		kdDebug() << "Now calling modelFeeder.parseTranslationUnit for file "
			  << fileName << endl;
		modelFeeder.parseTranslationUnit( ast );
	}
	m_umldoc->writeToStatusBar("Ready.");
}

