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

#include "attribute.h"
#include "umldoc.h"

/**
 * Interfaces classparser library to uml models
 * @author Mikko Pasanen
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
	void insertMethod(UMLObject *o, Uml::Scope scope, QString name, QString type, QList<UMLAttribute> *parList = NULL);

	/**
	 *
	 */
	bool createClass(QString className, UMLObject *uObject);
};

#endif
