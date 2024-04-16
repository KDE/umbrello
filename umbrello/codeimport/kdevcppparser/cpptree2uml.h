/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPTREE2UML_H
#define CPPTREE2UML_H

#include "tree_parser.h"
#include "model_utils.h"

#include <QStringList>

// fwd decls
class CodeImpThread;
class UMLClassifier;
class UMLFolder;
class UMLOperation;
class UMLPackage;

class CppTree2Uml: public TreeParser
{
public:
    explicit CppTree2Uml(const QString& fileName, CodeImpThread *thread = nullptr);
    virtual ~CppTree2Uml();

    void clear();
    void setRootPath(const QString &rootPath);
    //FileDom file() { return m_file; }

    // translation-unit
    virtual void parseTranslationUnit(const ParsedFile &file);

    // declarations
    //virtual void parseDeclaration(DeclarationAST*);  // use parent method
    //virtual void parseLinkageSpecification(LinkageSpecificationAST*);  // use parent method
    virtual void parseNamespace(NamespaceAST*);
    //virtual void parseNamespaceAlias(NamespaceAliasAST*);  // use parent method
    //virtual void parseUsing(UsingAST*);  // use parent method
    //virtual void parseUsingDirective(UsingDirectiveAST*);  // use parent method
    virtual void parseTypedef(TypedefAST*);
    virtual void parseTemplateDeclaration(TemplateDeclarationAST*);
    virtual void parseSimpleDeclaration(SimpleDeclarationAST*);
    virtual void parseFunctionDefinition(FunctionDefinitionAST*);
    //virtual void parseLinkageBody(LinkageBodyAST*);  // use parent method
    virtual void parseAccessDeclaration(AccessDeclarationAST*);

    // type-specifier
    //virtual void parseTypeSpecifier(TypeSpecifierAST*);  // use parent method
    virtual void parseClassSpecifier(ClassSpecifierAST*);
    virtual void parseEnumSpecifier(EnumSpecifierAST*);
    virtual void parseElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST*);

    // non-overriding (locally added) methods

    virtual void parseDeclaration2(GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl);
    virtual void parseFunctionDeclaration(GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl);
    void parseFunctionArguments(DeclaratorAST* declarator, UMLOperation* method);
    virtual void parseBaseClause(BaseClauseAST* baseClause, UMLClassifier* klass);

private:
    //NamespaceDom findOrInsertNamespace(NamespaceAST* ast, const QString& name);

    QString typeOfDeclaration(TypeSpecifierAST* typeSpec, DeclaratorAST* declarator);
    QStringList scopeOfName(NameAST* id, const QStringList& scope);
    QStringList scopeOfDeclarator(DeclaratorAST* d, const QStringList& scope);
    void flushTemplateParams(UMLClassifier *klass);

private:
    //FileDom m_file;
    QString m_fileName;
    QStringList m_currentScope;
    Uml::Visibility::Enum m_currentAccess;
    bool m_inSlots;
    bool m_inSignals;
    int m_anon;
    bool m_inStorageSpec;
    bool m_inTypedef;
    QString m_comment;
    Model_Utils::NameAndType_List m_templateParams;

    DeclaratorAST* m_currentDeclarator;
#   define STACKSIZE 30
    UMLPackage* m_currentNamespace[STACKSIZE+1];  ///< stack
    UMLClassifier* m_currentClass[STACKSIZE+1];  ///< stack
    int m_nsCnt;  ///< stack top for m_currentNamespace
    int m_clsCnt;  ///< stack top for m_currentClass
    CodeImpThread* m_thread;
    QString m_rootPath; ///< root path of project
    UMLFolder *m_rootFolder;
    UMLDoc *m_doc;

private:
    CppTree2Uml(const CppTree2Uml& source);
    void operator = (const CppTree2Uml& source);
};

#endif // CPPTREE2UML
