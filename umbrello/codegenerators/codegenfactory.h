/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                  *
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
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
namespace CodeGenFactory {

    CodeGenerator* createObject(Uml::ProgrammingLanguage::Enum pl);

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

    CodeGenPolicyExt* newCodeGenPolicyExt(Uml::ProgrammingLanguage::Enum pl);
}

#endif //CODEGENFACTORY_H
