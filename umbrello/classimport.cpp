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
#include <qmap.h>
#include <qregexp.h>
//#include <kapplication.h>
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
#include "template.h"
#include "association.h"
#include "idlimport.h"
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
	m_putAtGlobalScope = false;
}

ClassImport::~ClassImport() {}

QString ClassImport::formatComment(const QString &comment) {
	QStringList lines = QStringList::split("\n", comment);
	QString& first = lines.first();
	QRegExp wordex("\\w");
	if (first.startsWith("/*")) {
		int wordpos = wordex.search(first);
		if (wordpos != -1)
			first = first.mid(wordpos);  // remove comment start
		else
			lines.pop_front();  // nothing interesting on this line
	}
	QString& last = lines.last();
	int endpos = last.find("*/");
	if (endpos != -1) {
		if (last.contains(wordex))
			last = last.mid(0, endpos - 1);  // remove comment end
		else
			lines.pop_back();  // nothing interesting on this line
	}
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
	UMLObject * o = m_umldoc->findUMLObject(name, type, parentPkg);
	if (o == NULL) {
		// Strip possible adornments and look again.
		int isConst = name.contains(QRegExp("^const "));
		name.remove(QRegExp("^const\\s+"));
		QString typeName(name);
		int isPointer = typeName.contains('*');
		int isRef = typeName.contains('&');
		typeName.remove(QRegExp("[^\\w: ].*$"));
		UMLObject *origType = m_umldoc->findUMLObject(typeName, Uml::ot_UMLObject, parentPkg);
		if (origType == NULL) {
			// Still not found. Create the stripped down type.
			if (m_putAtGlobalScope)
				parentPkg = NULL;
			// Find, or create, the scopes.
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
						i18n("C++ Import Requests Your Help"),
						i18n("Namespace"), i18n("Class"));
					Uml::Object_Type ot = (wantNamespace == KMessageBox::Yes ? Uml::ot_Package : Uml::ot_Class);
					o = m_umldoc->createUMLObject(ot, scopeName, parentPkg);
					parentPkg = static_cast<UMLPackage*>(o);
				}
				// All scope qualified datatypes live in the global scope.
				m_putAtGlobalScope = true;
			}
			Uml::Object_Type t = type;
			if (type == Uml::ot_UMLObject || isConst || isRef || isPointer)
				t = Uml::ot_Class;
			origType = m_umldoc->createUMLObject(t, typeName, parentPkg);
		}
		if (isConst || isPointer || isRef) {
			// Create the full given type (including adornments.)
			if (isConst)
				name.prepend("const ");
			if (m_putAtGlobalScope)
				parentPkg = NULL;
			o = m_umldoc->createUMLObject(Uml::ot_Datatype, name, parentPkg);
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
		} else {
			o = origType;
		}
	} else if (parentPkg && !m_putAtGlobalScope) {
		o->setUMLPackage(parentPkg);
	}
	QString strippedComment = formatComment(comment);
	if (! strippedComment.isEmpty()) {
		o->setDoc(strippedComment);
		UMLApp::app()->getDocWindow()->showDocumentation(o, true);
	}
	if (!stereotype.isEmpty()) {
		o->setStereotype(stereotype);
	}
	return o;
}

UMLOperation* ClassImport::makeOperation(UMLClassifier *parent, const QString &name) {
	UMLOperation *op = new UMLOperation(parent, name);
	return op;
}

UMLObject* ClassImport::insertAttribute(UMLClass *owner, Uml::Scope scope, QString name,
					QString type, QString comment /* ="" */,
					bool isStatic /* =false */) {
	Uml::Object_Type ot = owner->getBaseType();
	if (ot != Uml::ot_Class) {
		kdDebug() << "ClassImport::insertAttribute: Don't know what to do with "
			  << owner->getName() << " (object type " << ot << ")" << endl;
		return NULL;
	}
	UMLObjectList atts = owner->findChildObject(Uml::ot_Attribute, name);
	if (atts.count()) {
		return atts.first();
	}
	UMLObject *attrType = owner->findTemplate(type);
	if (attrType == NULL) {
		m_putAtGlobalScope = true;
		attrType = createUMLObject(Uml::ot_UMLObject, type, owner);
		m_putAtGlobalScope = false;
	}
	UMLAttribute *attr = owner->addAttribute(name, attrType, scope);
	attr->setStatic(isStatic);
	QString strippedComment = formatComment(comment);
	if (! strippedComment.isEmpty()) {
		attr->setDoc(strippedComment);
		UMLApp::app()->getDocWindow()->showDocumentation(attr, true);
	}

	m_umldoc->setModified(true);
	return attr;
}

void ClassImport::insertMethod(UMLClassifier *klass, UMLOperation *op,
					 Uml::Scope scope, QString type,
					 bool isStatic, bool isAbstract,
					 QString comment) {
	op->setScope(scope);
	if (!type.isEmpty()) {  // return type may be missing (constructor/destructor)
		if (type == klass->getName()) {
			op->setType(klass);
		} else {
			UMLObject *typeObj = klass->findTemplate(type);
			if (typeObj == NULL) {
				m_putAtGlobalScope = true;
				typeObj = createUMLObject(Uml::ot_UMLObject, type, klass);
				m_putAtGlobalScope = false;
				op->setType(typeObj);
			}
		}
	}
	op->setStatic(isStatic);
	op->setAbstract(isAbstract);
	klass->addOperation(op);
	//m_umldoc->signalUMLObjectCreated(op);
	QString strippedComment = formatComment(comment);
	if (! strippedComment.isEmpty()) {
		op->setDoc(strippedComment);
		UMLApp::app()->getDocWindow()->showDocumentation(op, true);
	}
	//setModified(true);
}

UMLAttribute* ClassImport::addMethodParameter(UMLOperation *method,
					      QString type, QString name) {
	UMLClassifier *owner = static_cast<UMLClassifier*>(method->parent());
	UMLObject *typeObj = owner->findTemplate(type);
	if (typeObj == NULL) {
		m_putAtGlobalScope = true;
		typeObj = createUMLObject(Uml::ot_UMLObject, type, owner);
		m_putAtGlobalScope = false;
	}
	UMLAttribute *attr = new UMLAttribute(method, name);
	attr->setType(typeObj);
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

void ClassImport::feedTheModel(QString fileName) {
	QMap<QString, Dependence> deps = m_driver->dependences(fileName);
	if (! deps.empty()) {
		QMap<QString, Dependence>::Iterator it;
		for (it = deps.begin(); it != deps.end(); ++it) {
			if (it.data().second == Dep_Global)  // don't want these
				continue;
			QString includeFile = it.key();
			if (includeFile.isEmpty()) {
				kdError() << fileName << ": " << it.data().first
					  << " not found" << endl;
				continue;
			}
			kdDebug() << fileName << ": " << includeFile << " => " << it.data().first << endl;
			if (m_seenFiles.find(includeFile) == m_seenFiles.end())
				feedTheModel(includeFile);
		}
	}
	if (m_seenFiles.find(fileName) != m_seenFiles.end())
		return;
	m_seenFiles.append(fileName);
	TranslationUnitAST *ast = m_driver->translationUnit( fileName );
	if (ast == NULL) {
		kdError() << "ClassImport::feedTheModel: " << fileName << " not found" << endl;
		return;
	}
	CppTree2Uml modelFeeder( fileName, this );
	m_umldoc->writeToStatusBar(i18n("Importing file: %1").arg(fileName));
	modelFeeder.parseTranslationUnit( ast );
}

void ClassImport::importIDL(QStringList idlFileList) {
	char *umbrello_incpath = getenv( "UMBRELLO_INCPATH" );
	if (umbrello_incpath) {
		m_includePathList = QStringList::split( ':', umbrello_incpath );
	}
	for (QStringList::Iterator fileIT = idlFileList.begin();
				   fileIT != idlFileList.end(); ++fileIT) {
		QString fileName = (*fileIT);
		m_umldoc->writeToStatusBar(i18n("Importing file: %1").arg(fileName));
		IDLImport::parseFile(fileName);
	}
}

void ClassImport::importCPP(QStringList headerFileList) {
	// Reset the driver
	m_driver->reset();
	// The driver shall attempt to parse included files.
	m_driver->setResolveDependencesEnabled( true );
	// Add some standard include paths
	m_driver->addIncludePath( "/usr/include" );
	m_driver->addIncludePath( "/usr/include/c++" );
	m_driver->addIncludePath( "/usr/include/g++" );
	m_driver->addIncludePath( "/usr/local/include" );
	// FIXME: The following hack is to be replaced by a config menu in umbrello
	char *umbrello_incpath = getenv( "UMBRELLO_INCPATH" );

	if (umbrello_incpath) {

		m_includePathList = QStringList::split( ':', umbrello_incpath );

		QStringList::Iterator end(m_includePathList.end());
		for (QStringList::Iterator i(m_includePathList.begin()); i != end; ++i) {
			m_driver->addIncludePath( *i );
                }

	}
	m_seenFiles.clear();
	for (QStringList::Iterator fileIT = headerFileList.begin();
				   fileIT != headerFileList.end(); ++fileIT) {
		QString fileName = (*fileIT);
		m_umldoc->writeToStatusBar(i18n("Importing file: %1").arg(fileName));

		/// kapp->processEvents();
		// Invoking kapp->processEvents() improves responsiveness when importing
		// many large header files but slows down import because the list view is
		// intermittently updated.

		if (m_seenFiles.find(fileName) != m_seenFiles.end())
			continue;
		m_driver->parseFile( fileName );
		feedTheModel(fileName);
	}
	m_umldoc->writeToStatusBar("Ready.");
}

