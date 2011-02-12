/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODEGENFACTORY_H
#define CODEGENFACTORY_H

#include "basictypes.h"
#include "codeaccessormethod.h"

// fwd decls
class CodeGenerator;
class ClassifierCodeDocument;
class CodeOperation;
class CodeClassField;
class CodeClassFieldDeclarationBlock;
class CodeGenPolicyExt;
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

    CodeGenerator* createObject(Uml::ProgrammingLanguage pl);


    CodeOperation *newCodeOperation(ClassifierCodeDocument *cd, UMLOperation * op);

    CodeClassField *newCodeClassField(ClassifierCodeDocument *cd, UMLAttribute * at);

    CodeClassField *newCodeClassField(ClassifierCodeDocument *cd, UMLRole *role);

    CodeAccessorMethod *newCodeAccessorMethod(ClassifierCodeDocument *cd,
                                              CodeClassField *cf,
                                              CodeAccessorMethod::AccessorType type);

    CodeClassFieldDeclarationBlock *newDeclarationCodeBlock(ClassifierCodeDocument *cd,
                                                            CodeClassField * cf);

    CodeComment *newCodeComment(CodeDocument *cd);

    CodeDocument *newClassifierCodeDocument(UMLClassifier *classifier);

    CodeGenPolicyExt* newCodeGenPolicyExt(Uml::ProgrammingLanguage pl);
}

#endif //CODEGENFACTORY_H
