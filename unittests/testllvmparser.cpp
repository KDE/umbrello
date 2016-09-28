/*
    Copyright 2016  Ralf Habacker <ralf.habacker@freenet.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
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
            FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getLocStart());
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

