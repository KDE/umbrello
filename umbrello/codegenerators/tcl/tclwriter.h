/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005      Rene Meyer <rene.meyer@sturmit.de>            *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef TCLWRITER_H
#define TCLWRITER_H

#include "simplecodegenerator.h"
#include "umloperationlist.h"
#include "umlattributelist.h"
#include "umlassociationlist.h"

#include <QtCore/QStringList>

class QFile;
class QTextStream;

/**
 * Class TclWriter is a code generator for UMLClassifier objects.
 * Create an instance of this class, and feed it a UMLClassifier when
 * calling writeClass and it will generate both a header (.h) and
 * source (.tcl) file for that classifier.
 */
class TclWriter : public SimpleCodeGenerator
{
public:

    /**
     * Constructor, initialises a couple of variables.
     */
    TclWriter();

    /**
     * Destructor, empty.
     */
    virtual ~TclWriter();

    /**
     * Call this method to generate tcl code for a UMLClassifier.
     * @param c   the class to generate code for
     */
    virtual void writeClass(UMLClassifier * c);

    /**
     * Returns "Tcl".
     * @return   the programming language identifier
     */
    virtual Uml::ProgrammingLanguage language() const;

    /**
     * Get list of reserved keywords.
     * @return   the list of reserved keywords
     */
    virtual QStringList reservedKeywords() const;

private:
    /**
     * Current output stream.
     */
    QTextStream * mStream;

    /**
     * Write the header file for this classifier.
     */
    void writeHeaderFile(UMLClassifier * c, QFile & file);

    /**
     * Write the source code body file for this classifier.
     */
    void writeSourceFile(UMLClassifier * c, QFile & file);

    /**
     * Write the source codei text.
     */
    void writeCode(const QString &text);

    /**
     * Write comment text.
     */
    void writeComm(const QString &text);

    /**
     * Write documentation text.
     */
    void writeDocu(const QString &text);

    void writeConstructorDecl();

    void writeDestructorDecl();

    /**
     * Summary information about current classifier.
     */
    QString mNamespace;
    QString mClassGlobal;

    /**
     * Writes the Attribute declarations
     * @param visibility the visibility of the attribs to print out
     * @param writeStatic whether to write static or non-static attributes out
     * @param stream text stream
     */
    void writeAttributeDecl(UMLClassifier * c, Uml::Visibility visibility, bool writeStatic);

    void writeAssociationIncl(UMLAssociationList list,
                              Uml::IDType myId, const QString &type);
    /**
     * Searches a list of associations for appropriate ones to write out as attributes.
     */
    void writeAssociationDecl(UMLAssociationList associations,
                              Uml::Visibility permit, Uml::IDType id,
                              const QString &type);

    /**
     * Writes out an association as an attribute using Vector.
     */
    void writeAssociationRoleDecl(const QString &fieldClassName,
                                  const QString &roleName, const QString &multi,
                                  const QString &doc, const QString &docname);

    /**
     * If needed, write out the declaration for the method to initialize attributes of our class.
     */
    void writeInitAttributeHeader(UMLClassifier * c);
    void writeInitAttributeSource(UMLClassifier* c);

    void writeConstructorHeader();
    void writeConstructorSource(UMLClassifier * c);
    void writeDestructorHeader();
    void writeDestructorSource();
    void writeOperationHeader(UMLClassifier * c,
                              Uml::Visibility permitScope);
    void writeOperationSource(UMLClassifier * c,
                              Uml::Visibility permitScope);
    void writeAttributeSource(UMLClassifier * c);
    void writeAssociationSource(UMLAssociationList associations,
                                Uml::IDType id);
    void writeAssociationRoleSource(const QString &fieldClassName,
                                    const QString &roleName,
                                    const QString &multi);
    void writeUse(UMLClassifier * c);

    /**
     * Returns the name of the given object (if it exists).
     */
    QString getUMLObjectName(UMLObject * obj);

    /**
     * Replaces `string' with STRING_TYPENAME.
     */
    QString fixTypeName(const QString &string);

    QStringList ObjectFieldVariables;
    QStringList VectorFieldVariables;

};

#endif // TCLWRITER_H
