/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODEGENFACTORY_H
#define CODEGENFACTORY_H

#include "../umlnamespace.h"
#include "../codeaccessormethod.h"

// fwd decls
class CodeGenerator;
class ClassifierCodeDocument;
class CodeOperation;
class CodeClassField;
class CodeClassFieldDeclarationBlock;
class UMLClassifier;
class UMLOperation;
class UMLAttribute;
class UMLRole;

/**
 * CodeGenFactory allows creating the available code generators as well
 * as the auxiliary objects required for the advanced code generators.
 *
 * @author Jonathan Riddell
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace CodeGenFactory {

    /**
     * Create a code generator specific to the given language.
     */
    CodeGenerator* createObject(Uml::Programming_Language pl);


    /**
     * Create a CodeOperation belonging to the given ClassifierCodeDocument.
     * Only applies to the advanced generators.
     *
     * @param cd    the parent ClassifierCodeDocument.
     * @param op    the related UMLOperation
     * @return  CodeOperation which is specific to the current language
     */
    CodeOperation *newCodeOperation(ClassifierCodeDocument *cd, UMLOperation * op);

    /**
     * Create an attribute CodeClassField belonging to the given
     * ClassifierCodeDocument.
     * Only applies to the advanced generators.
     *
     * @param cd         the parent ClassifierCodeDocument
     * @param at         attribute which is parent of this class field
     * @return  CodeClassField which is specific to the current language
     */
    CodeClassField * newCodeClassField (ClassifierCodeDocument *cd, UMLAttribute * at);

    /**
     * Create an association role CodeClassField belonging to the given
     * ClassifierCodeDocument.
     * Only applies to the advanced generators.
     *
     * @param cd         the parent ClassifierCodeDocument
     * @param role       association role which is parent of this class field
     * @return  CodeClassField which is specific to the current language
     */
    CodeClassField * newCodeClassField(ClassifierCodeDocument *cd, UMLRole *role);

    /**
     * Create a CodeAccessorMethod object belonging to the given ClassifierCodeDocument.
     * Only applies to the advanced generators.
     *
     * @param cd         the parent ClassifierCodeDocument
     * @param cf         CodeClassField which is parent of this object
     * @param type       CodeAccessorMethod::AccessorType to create
     *
     * @return  CodeAccessorMethod which is specific to the current language
     */
    CodeAccessorMethod * newCodeAccessorMethod(ClassifierCodeDocument *cd,
                                               CodeClassField *cf,
                                               CodeAccessorMethod::AccessorType type);

    /**
     * Create a CodeClassFieldDeclarationBlock object belonging to the given
     * ClassifierCodeDocument.
     * Only applies to the advanced generators.
     *
     * @param cd         the parent ClassifierCodeDocument
     * @param cf         CodeClassField which is parent of this object
     *
     * @return  CodeClassFieldDeclarationBlock which is specific to the current language
     */
    CodeClassFieldDeclarationBlock * newDeclarationCodeBlock (ClassifierCodeDocument *cd,
                                                              CodeClassField * cf);

    /**
     * Create a new CodeComment object belonging to the given CodeDocument.
     * Only applies to the advanced generators.
     *
     * @param cd         the parent CodeDocument
     * @return      CodeBlockWithComments
     */
    CodeComment * newCodeComment (CodeDocument *cd);

    /**
     * Currently unused (for possible future use)
     */
    CodeDocument * newClassifierCodeDocument (UMLClassifier * classifier);
}

#endif //CODEGENFACTORY_H
