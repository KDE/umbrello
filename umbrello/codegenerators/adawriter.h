/***************************************************************************
                          adawriter.h  -  description
                             -------------------
    Based on javawriter.h by Luis De la Parra Blum
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

#ifndef ADAWRITER_H
#define ADAWRITER_H

#include "../codegenerator.h"
#include <qtextstream.h>

class UMLAssociation;
class UMLOperation;

/**
  * class AdaWriter is a code generator for UMLConcept objects.
  * Create an instance of this class, and feed it a UMLConcept when
  * calling writeClass and it will generate an Ada package spec for
  * that concept
  */
class AdaWriter : public CodeGenerator {
public:

	AdaWriter (QObject* parent = 0, const char* name = 0);
	virtual ~AdaWriter ();

	/**
	 * call this method to generate Ada code for a UMLConcept
	 * @param c the class to generate code for
	 */
	virtual void writeClass (UMLConcept *c);

private:

	/**
	 * write one operation
	 * @param op the class for which we are generating code
	 * @param ada the stream associated with the output file
	 */
	void writeOperation (UMLOperation *op, QTextStream &ada, bool is_comment = false);

	void computeAssocTypeAndRole (UMLAssociation *a, QString& typeName, QString& roleName);

	bool isOOClass (UMLConcept *c);

	QString qualifiedName
	(UMLConcept *c, bool withType = false, bool byValue = false);

	QString adatype (QString umbtype);

	QString toupperfirst (QString s);

	QString spc ();

	static const QString defaultPackageSuffix;

	int indentlevel;
};

#endif // ADAWRITER_H
