/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSIMPORT_H
#define CLASSIMPORT_H

#include <qptrlist.h>
#include <qstringlist.h>
#include "umlnamespace.h"
#include "umlattributelist.h"

class UMLDoc;
class UMLObject;
class UMLPackage;
class UMLOperation;
class CppDriver;

/**
 * Interfaces classparser library to uml models
 * @author Mikko Pasanen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class ClassImport {
public:
	ClassImport(UMLDoc *parentDoc);
	~ClassImport();

	/**
	 *
	 */
	void importCPP(QStringList headerFiles);

	/**
	 * Find or create a document object.
	 */
	UMLObject* createUMLObject(Uml::UMLObject_Type type,
				   QString name,
				   QString comment = "",
				   UMLPackage *parentPkg = NULL);

	/**
	 * Create a UMLAttribute and insert it into the document.
	 */
	UMLObject* insertAttribute(UMLObject *o, Uml::Scope scope, QString name,
				   QString type, QString comment = "",
				   bool isStatic = false);

	/**
	 * Create a UMLOperation and insert it into the document.
	 */
	UMLOperation* insertMethod(UMLObject *o, Uml::Scope scope, QString name,
				   QString type, bool isStatic, bool isAbstract,
				   QString comment="", UMLAttributeList *parList=NULL);

	/**
	 * Check that a given comment conforms to the Doxygen convention, i.e.
	 * check that it begins with slash-star-star.
	 * Strip the comment lines of leading whitespace and stars.
	 */
	QString doxyComment(QString comment);

private:
	UMLDoc * m_umldoc;  // just a shorthand for UMLApp::app()->getDocument()
	CppDriver * m_driver;
};

#endif
