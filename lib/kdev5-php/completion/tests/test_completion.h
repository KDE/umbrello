/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef TESTCOMPLETION_H
#define TESTCOMPLETION_H

#include <language/codecompletion/codecompletionitem.h>
#include <language/duchain/classdeclaration.h>

#include "../../duchain/tests/duchaintestbase.h"

namespace Php
{

class TestCompletion : public DUChainTestBase
{
    Q_OBJECT

public:
    TestCompletion();

private:
    void dumpCompletionItems(QList<KDevelop::CompletionTreeItemPointer> items);
    void verifyExtendsOrImplements(const QString &code, const QString &actionStr,
                                   KDevelop::ClassDeclarationData::ClassType,
                                   const KDevelop::CursorInRevision& cursor,
                                   QStringList forbiddenIdentifiers);

    Q_SLOT void publicObjectCompletion();
    Q_SLOT void publicStaticObjectCompletion();
    Q_SLOT void protectedObjectCompletion();
    Q_SLOT void protectedStaticObjectCompletion();
    Q_SLOT void privateObjectCompletion();
    Q_SLOT void privateStaticObjectCompletion();
    Q_SLOT void methodCall();
    Q_SLOT void functionCall();
    Q_SLOT void nestedFunctionCall_data();
    Q_SLOT void nestedFunctionCall();
    Q_SLOT void newObjectFromOtherFile();
    Q_SLOT void constantFromOtherFile();
    Q_SLOT void baseClass();
    Q_SLOT void extendsFromOtherFile();
    Q_SLOT void globalClassFromOtherFile();
    Q_SLOT void codeModel();
    Q_SLOT void projectFileClass();
    Q_SLOT void variable();
    Q_SLOT void nameNormalVariable();
    Q_SLOT void nameClassMember();
    Q_SLOT void exceptions();
    Q_SLOT void exceptionOtherFile();
    Q_SLOT void abstractMethods();
    Q_SLOT void interfaceMethods();
    Q_SLOT void interfaceMethods2();
    Q_SLOT void overrideMethods();
    Q_SLOT void overrideVars();
    Q_SLOT void implementMethods();
    Q_SLOT void inArray();
    Q_SLOT void newExtends();
    Q_SLOT void updateExtends();
    Q_SLOT void newImplements();
    Q_SLOT void updateImplements();
    Q_SLOT void avoidCircularInheritance();
    Q_SLOT void unsureType();
    Q_SLOT void completionAfterComments();
    Q_SLOT void completionInComments();
    Q_SLOT void phpStartTag();
    Q_SLOT void outsidePhpContext();
    Q_SLOT void nonGlobalInFunction();
    Q_SLOT void fileCompletion();
    Q_SLOT void instanceof();
    Q_SLOT void afterFunctionArg();
    Q_SLOT void functionBeforeDeclaration();
    Q_SLOT void classBeforeDeclaration();
    Q_SLOT void constantBeforeDeclaration();
    Q_SLOT void variableBeforeDeclaration();
    Q_SLOT void functionArguments();
    Q_SLOT void referencedClass();
    Q_SLOT void ctorCall();
    Q_SLOT void chainedCalling();
    Q_SLOT void funcCallInConditional();
    Q_SLOT void namespaces();
    Q_SLOT void inNamespace();
    Q_SLOT void closures();
};

}

#endif // TESTCOMPLETION_H
