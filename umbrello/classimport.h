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
#include "umldoc.h"
#include "umlattributelist.h"

class UMLPackage;
class CClassStore;

/**
 * Interfaces classparser library to uml models
 * @author Mikko Pasanen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class ClassImport : public UMLDoc {
public:
	ClassImport(QWidget *parent, const char *name);
	~ClassImport();

	/**
	 *
	 */
	void importCPP(QStringList headerFiles);

	/**
	 * Find or create a document object.
	 */
	UMLObject *createUMLObject(Uml::UMLObject_Type type,
				   QString name,
				   QString comment = "",
				   UMLPackage *parentPkg = NULL);

	/**
	 * Create a UMLAttribute and insert it into the document.
	 */
	void insertAttribute(CClassStore& store,
			     UMLObject *o, Uml::Scope scope, QString name,
			     QString type, QString comment = "");

	/**
	 * Create a UMLOperation and insert it into the document.
	 */
	void insertMethod(UMLObject *o, Uml::Scope scope, QString name, QString type,
			  QString comment = "", UMLAttributeList *parList = NULL);

	QString doxyComment(QString comment);
};

#endif
