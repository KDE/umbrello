/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003       Alexander Blum  <blum@kewbee.de>             *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef JSWRITER_H
#define JSWRITER_H

#include "simplecodegenerator.h"
#include "umloperationlist.h"
#include "umlassociationlist.h"

class QTextStream;

/**
 * Class JSWriter is a JavaScript code generator for UMLClassifier objects.
 * Just call writeClass and feed it a UMLClassifier.
 */
class JSWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    JSWriter();
    virtual ~JSWriter();

    /**
     * Call this method to generate Actionscript code for a UMLClassifier.
     * @param c   the class you want to generate code for
     */
    virtual void writeClass(UMLClassifier *c);

    /**
     * Returns "JavaScript".
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
     * We do not want to write the comment "Private methods" twice.
     */
    bool bPrivateSectionCommentIsWritten;

    /**
     * Write a list of class operations.
     * @param classname   the name of the class
     * @param opList      the list of operations
     * @param js          output stream for the JS file
     */
    void writeOperations(QString classname, UMLOperationList *opList, QTextStream &js);

    /**
     * Write a list of associations.
     * @param classname   the name of the class
     * @param assocList   the list of associations
     * @param js          output stream for the JS file
     */
    void writeAssociation(QString& classname, UMLAssociationList& assoclist , QTextStream &js);
};

#endif //JSWRITER
