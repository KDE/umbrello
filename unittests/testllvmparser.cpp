/*
    SPDX-FileCopyrightText: 2016 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include <clang/Basic/Version.h>
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include <QFile>
#include <QString>

using namespace clang;

class FindNamedClassVisitor : public RecursiveASTVisitor<FindNamedClassVisitor>
{
public:
    explicit FindNamedClassVisitor(ASTContext *Context)
    : Context(Context)
    {
    }

    bool VisitStmt(Stmt *s)
    {
        s->dump();
        return true;
    }

    bool VisitCXXRecordDecl(CXXRecordDecl *Declaration)
    {
        if (Declaration->getQualifiedNameAsString() == "n::m::C") {
#if CLANG_VERSION_MAJOR >= 8
            FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getBeginLoc());
#else
            FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getLocStart());
#endif
            if (FullLocation.isValid())
                llvm::outs() << "Found declaration at "
                             << FullLocation.getSpellingLineNumber() << ":"
                             << FullLocation.getSpellingColumnNumber() << "\n";
        }
        return true;
    }

private:
    ASTContext *Context;
};

class FindNamedClassConsumer : public clang::ASTConsumer
{
public:
    explicit FindNamedClassConsumer(ASTContext *Context)
      : Visitor(Context)
    {
    }

    virtual void HandleTranslationUnit(clang::ASTContext &Context)
    {
        if (!Visitor.TraverseDecl(Context.getTranslationUnitDecl()))
            llvm::errs() << "could not parse file";
    }

private:
    FindNamedClassVisitor Visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction
{
public:
    virtual std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile)
    {
        Q_UNUSED(InFile);
        return std::unique_ptr<clang::ASTConsumer>(
            new FindNamedClassConsumer(&Compiler.getASTContext()));
    }
};

int main(int argc, char **argv)
{
    QByteArray defaultSource = "namespace n {\n namespace m {\n class C {}; \n} \n}";
    if (argc > 1) {
        QFile source(argv[1]);
        if (!source.open(QFile::ReadOnly))
            return 1;
        defaultSource = source.readAll();
    }
    return clang::tooling::runToolOnCode(new FindNamedClassAction, defaultSource.data()) ? 0 : 2;
}

