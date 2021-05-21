/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
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
