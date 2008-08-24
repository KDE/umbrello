/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003                                                    *
 *   Oliver Kellogg <okellogg@users.sourceforge.net>                       *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef IDLWRITER_H
#define IDLWRITER_H

#include "simplecodegenerator.h"

class UMLAssociation;
class UMLOperation;
class QTextStream;

/**
 * Class IDLWriter is a code generator for UMLClassifier objects.
 * Create an instance of this class, and feed it a UMLClassifier when
 * calling writeClass and it will generate an IDL interface for that
 * concept.
 */
class IDLWriter : public SimpleCodeGenerator
{
public:

    IDLWriter ();
    virtual ~IDLWriter ();

    /**
     * Call this method to generate IDL code for a UMLClassifier.
     * @param c the class to generate code for
     */
    virtual void writeClass (UMLClassifier *c);

    /**
     * Returns "IDL".
     */
    virtual Uml::Programming_Language getLanguage();

    QStringList defaultDatatypes();

    /**
     * Get list of reserved keywords.
     */
    virtual const QStringList reservedKeywords() const;

private:

    /**
     * Write one operation.
     * @param op the class for which we are generating code
     * @param idl the stream associated with the output file
     */
    void writeOperation(UMLOperation* op, QTextStream& idl, bool is_comment = false);

    void computeAssocTypeAndRole(UMLAssociation* a, UMLClassifier *c,
                                 QString& typeName, QString& roleName);

    static bool isOOClass(UMLClassifier* c);

    static bool assocTypeIsMappableToAttribute(Uml::Association_Type at);

};

#endif // IDLWRITER_H
