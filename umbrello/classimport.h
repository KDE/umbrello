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

// forward declarations
class UMLAttribute;

/**
 * Interfaces classparser library to uml models
 * @author Mikko Pasanen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class ClassImport : private UMLDoc {
public:
	ClassImport(QWidget *parent, const char *name);
	~ClassImport();

	/**
	 *
	 */
	void importCPP(QStringList headerFiles);

	/**
	 *
	 */
	UMLObject *createUMLObject(QString className, Uml::UMLObject_Type type);

	/**
	 *
	 */
	void insertAttribute(UMLObject *o, Uml::Scope scope, QString name, QString type, QString value = "");

	/**
	 *
	 */
	void insertMethod(UMLObject *o, Uml::Scope scope, QString name, QString type, QPtrList<UMLAttribute> *parList = NULL);

	/**
	 *
	 */
	bool createClass(QString className, UMLObject *uObject);
};

#endif
