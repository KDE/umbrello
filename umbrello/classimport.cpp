/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "classimport.h"
// qt/kde includes
#include <qregexp.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
// app includes
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "docwindow.h"
#include "package.h"
#include "enum.h"
#include "datatype.h"
#include "class.h"
#include "operation.h"
#include "attribute.h"
#include "association.h"
#include "classparser/lexer.h"
#include "classparser/driver.h"
#include "classparser/cpptree2uml.h"

class CppDriver : public Driver {
public:
	void setupLexer(Lexer* lexer) {
		Driver::setupLexer(lexer);
		lexer->setRecordComments(true);
	}
};

ClassImport::ClassImport(UMLDoc * parentDoc) {
	m_umldoc = parentDoc;
	m_driver = new CppDriver();
}

ClassImport::~ClassImport() {}

QString ClassImport::doxyComment(const QString &comment) {
	QStringList lines = QStringList::split("\n", comment);
	if (lines.first() != "/**")
		return "";
	lines.pop_front();  // remove comment start
	lines.pop_back();   // remove comment end
	if (! lines.count())
		return "";

	QStringList::Iterator end(lines.end());
	for (QStringList::Iterator lit(lines.begin()); lit != end; ++lit) {
		(*lit).remove(QRegExp("^\\s+"));
		(*lit).remove(QRegExp("^\\*+\\s?"));
	}
	return lines.join("\n");
}

/*
UMLObject* ClassImport::findUMLObject(QString name,
				      Uml::Object_Type type) {
	// Why an extra wrapper? See comment at addMethodParameter()
	UMLObject * o = m_umldoc->findUMLObject(name, type);
	return o;
}
 */

UMLObject *ClassImport::createUMLObject(Uml::Object_Type type,
					QString name,
					UMLPackage *parentPkg,
					QString comment,
					QString stereotype) {
	UMLObject * o = m_umldoc->findUMLObject(name);
	if (o == NULL) {
		int isConst = name.contains(QRegExp("^const "));
		name.remove(QRegExp("^const\\s+"));
		QString typeName(name);
		int isPointer = typeName.contains('*');
		int isRef = typeName.contains('&');
		typeName.remove(QRegExp("[^:\\w].*$"));
		o = m_umldoc->findUMLObject(typeName,
					    Uml::ot_UMLObject,
					    parentPkg);
		if (o == NULL) {
			if (m_putAtGlobalScope)
				parentPkg = NULL;
			if (typeName.contains("::")) {
				QStringList components = QStringList::split("::", typeName);
				typeName = components.back();
				components.pop_back();
				while ( components.count() ) {
					QString scopeName = components.front();
					components.pop_front();
					o = m_umldoc->findUMLObject(scopeName, Uml::ot_UMLObject, parentPkg);
					if (o) {
						parentPkg = static_cast<UMLPackage*>(o);
						continue;
					}
					int wantNamespace = KMessageBox::questionYesNo(NULL,
						i18n("Is the scope %1 a namespace or a class?").arg(scopeName),
						i18n("C++ import requests your help"),
						i18n("namespace"), i18n("class"));
					Uml::Object_Type ot = (wantNamespace == KMessageBox::Yes ? Uml::ot_Package : Uml::ot_Class);
					o = m_umldoc->createUMLObject(ot, scopeName, parentPkg);
					parentPkg = static_cast<UMLPackage*>(o);
				}
				name.remove(QRegExp("^.*::"));  // may also zap "const "
			}
			UMLObject *origType = NULL;
			if (isPointer || isRef) {
				origType = m_umldoc->createUMLObject(Uml::ot_Class, typeName, parentPkg);
				type = Uml::ot_Datatype;
			} else if (type == Uml::ot_UMLObject)
				type = Uml::ot_Class;
			if (isConst)
				name.prepend("const ");
			o = m_umldoc->createUMLObject(type, name, parentPkg);
			if (origType) {
				UMLDatatype *dt = static_cast<UMLDatatype*>(o);
				UMLClassifier *c = dynamic_cast<UMLClassifier*>(origType);
				if (c)
					dt->setOriginType(c);
				else
					kdError() << "ClassImport::createUMLObject(" << name << "): "
						  << "origType " << typeName << " is not a UMLClassifier"
						  << endl;
				dt->setIsReference();
				/*
				if (isPointer) {
					UMLObject *pointerDecl = m_umldoc->createUMLObject(Uml::ot_Datatype, type);
					UMLDatatype *dt = static_cast<UMLDatatype*>(pointerDecl);
					dt->setOriginType(classifier);
					dt->setIsReference();
					classifier = dt;
				}  */
			}
		}
	} else if (parentPkg) {
		o->setUMLPackage(parentPkg);
	}
	QString strippedComment = doxyComment(comment);
	if (! strippedComment.isEmpty()) {
		o->setDoc(strippedComment);
		UMLApp::app()->getDocWindow()->showDocumentation(o, true);
	}
	if (!stereotype.isEmpty()) {
		o->setStereotype(stereotype);
	}
	return o;
}

UMLOperation* ClassImport::makeOperation(UMLClass *parent, const QString &name) {
	UMLOperation *op = new UMLOperation(parent, name);
	return op;
}

UMLObject* ClassImport::insertAttribute(UMLClass *o, Uml::Scope scope, QString name,
					QString type, QString comment /* ="" */,
					bool isStatic /* =false */,
					UMLPackage *parentPkg /* = NULL */) {
	Uml::Object_Type ot = o->getBaseType();
	if (ot != Uml::ot_Class) {
		kdDebug() << "ClassImport::insertAttribute: Don't know what to do with "
			  << o->getName() << " (object type " << ot << ")" << endl;
		return NULL;
	}
	QString strippedComment = doxyComment(comment);
	UMLAttribute *attr = ((UMLClass*)o)->addAttribute(name);
	attr->setScope(scope);
	attr->setStatic(isStatic);
	m_putAtGlobalScope = true;
	UMLObject *obj = createUMLObject(Uml::ot_UMLObject, type, parentPkg);
	m_putAtGlobalScope = false;
	UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(obj);
	attr->setType(classifier);
	if (! strippedComment.isEmpty()) {
		attr->setDoc(strippedComment);
		UMLApp::app()->getDocWindow()->showDocumentation(attr, true);
	}

	m_umldoc->setModified(true);
	return attr;
}

void ClassImport::insertMethod(UMLClass *klass, UMLOperation *op,
					 Uml::Scope scope, QString type,
					 bool isStatic, bool isAbstract,
					 QString comment,
					 UMLPackage *parentPkg) {
	op->setScope(scope);
	if (!type.isEmpty()) {  // return type may be missing (constructor/destructor)
		m_putAtGlobalScope = true;
		UMLObject *typeObj = createUMLObject(Uml::ot_UMLObject, type, parentPkg);
		m_putAtGlobalScope = false;
		op->setType(dynamic_cast<UMLClassifier*>(typeObj));
	}
	op->setStatic(isStatic);
	op->setAbstract(isAbstract);
	klass->addOperation(op);
	//m_umldoc->signalUMLObjectCreated(op);
	QString strippedComment = doxyComment(comment);
	if (! strippedComment.isEmpty()) {
		op->setDoc(strippedComment);
		UMLApp::app()->getDocWindow()->showDocumentation(op, true);
	}
	//setModified(true);
}

UMLAttribute* ClassImport::addMethodParameter(UMLOperation *method,
					      QString type, QString name,
					      UMLPackage *parentPkg) {
	m_putAtGlobalScope = true;
	UMLObject *typeObj = createUMLObject(Uml::ot_UMLObject, type, parentPkg);
	m_putAtGlobalScope = false;
	UMLAttribute *attr = new UMLAttribute(method, name);
	attr->setType(dynamic_cast<UMLClassifier*>(typeObj));
	//attr->setInitialValue(initialValue);
	//attr->setDoc(doc);
	//attr->setParmKind(kind);
	method->addParm(attr);
	return attr;
}

void ClassImport::addEnumLiteral(UMLEnum *enumType, const QString &literal) {
	enumType->addEnumLiteral( literal );
}

void ClassImport::createGeneralization(UMLClass *child, const QString &parentName) {
	UMLObject *parentObj = createUMLObject( Uml::ot_Class, parentName );
	UMLClass *parent = static_cast<UMLClass*>(parentObj);
	UMLAssociation *assoc = new UMLAssociation( Uml::at_Generalization,
						    child, parent );
	m_umldoc->addAssociation(assoc);
}

void ClassImport::importCPP(QStringList headerFileList) {
	// Reset the driver
	m_driver->reset();
	// The driver shall attempt to parse included files.
	// TODO: Although the include files do get parsed, they do not get
	//       generated into UML.
	//       We need to look into the Driver's mechanism for handling that.
	m_driver->setResolveDependencesEnabled( true );
	// Add some standard include paths
	m_driver->addIncludePath( "/usr/include" );
	m_driver->addIncludePath( "/usr/include/c++" );
	m_driver->addIncludePath( "/usr/include/g++" );
	m_driver->addIncludePath( "/usr/local/include" );
	// FIXME: The following hack is to be replaced by a config menu in umbrello
	char *umbrello_incpath = getenv( "UMBRELLO_INCPATH" );

	if (umbrello_incpath) {

		QStringList includes = QStringList::split( ':', umbrello_incpath );

		QStringList::Iterator end(includes.end());
		for (QStringList::Iterator i(includes.begin()); i != end; ++i) {
			m_driver->addIncludePath( *i );
                }

	}
	for (QStringList::Iterator fileIT = headerFileList.begin();
				   fileIT != headerFileList.end(); ++fileIT) {
		QString fileName = (*fileIT);
		m_umldoc->writeToStatusBar(i18n("Importing file: %1").arg(fileName));
		kapp->processEvents();
		m_driver->parseFile( fileName );
		TranslationUnitAST *ast = m_driver->translationUnit( fileName );
		if (ast == NULL)
			continue;
		CppTree2Uml modelFeeder( fileName, this );
		// kdDebug() << "Now calling modelFeeder.parseTranslationUnit for file "
		//	   << fileName << endl;
		modelFeeder.parseTranslationUnit( ast );
	}
	m_umldoc->writeToStatusBar("Ready.");
}

