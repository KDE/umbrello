/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef TESTCOMPLETION_H
#define TESTCOMPLETION_H

#include "duchain/tests/duchaintestbase.h"
#include <language/codecompletion/codecompletionitem.h>
#include <language/duchain/classdeclaration.h>

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

private slots:
    void publicObjectCompletion();
    void publicStaticObjectCompletion();
    void protectedObjectCompletion();
    void protectedStaticObjectCompletion();
    void privateObjectCompletion();
    void privateStaticObjectCompletion();
    void methodCall();
    void functionCall();
    void nestedFunctionCall_data();
    void nestedFunctionCall();
    void newObjectFromOtherFile();
    void constantFromOtherFile();
    void baseClass();
    void extendsFromOtherFile();
    void globalClassFromOtherFile();
    void codeModel();
    void projectFileClass();
    void variable();
    void nameNormalVariable();
    void nameClassMember();
    void exceptions();
    void exceptionOtherFile();
    void abstractMethods();
    void interfaceMethods();
    void interfaceMethods2();
    void overrideMethods();
    void overrideVars();
    void implementMethods();
    void inArray();
    void newExtends();
    void updateExtends();
    void newImplements();
    void updateImplements();
    void avoidCircularInheritance();
    void unsureType();
    void completionAfterComments();
    void completionInComments();
    void phpStartTag();
    void outsidePhpContext();
    void nonGlobalInFunction();
    void fileCompletion();
    void instanceof();
    void afterFunctionArg();
    void functionBeforeDeclaration();
    void classBeforeDeclaration();
    void constantBeforeDeclaration();
    void variableBeforeDeclaration();
    void functionArguments();
    void referencedClass();
    void ctorCall();
    void chainedCalling();
    void funcCallInConditional();
    void namespaces();
    void inNamespace();
    void closures();
};

}

#endif // TESTCOMPLETION_H
