/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ast.h"
#include <QStringList>

QString nodeTypeToString(int type)
{
    switch (type) {
    case NodeType_Generic:
        return QLatin1String("Generic");
    case NodeType_TemplateArgumentList:
        return QLatin1String("TemplateArgumentList");
    case NodeType_ClassOrNamespaceName:
        return QLatin1String("ClassOrNamespaceName");
    case NodeType_Name:
        return QLatin1String("Name");
    case NodeType_Declaration:
        return QLatin1String("Declaration");
    case NodeType_TypeSpecifier:
        return QLatin1String("TypeSpecifier");
    case NodeType_BaseSpecifier:
        return QLatin1String("BaseSpecifier");
    case NodeType_BaseClause:
        return QLatin1String("BaseClause");
    case NodeType_ClassSpecifier:
        return QLatin1String("ClassSpecifier");
    case NodeType_Enumerator:
        return QLatin1String("Enumerator");
    case NodeType_EnumSpecifier:
        return QLatin1String("EnumSpecifier");
    case NodeType_ElaboratedTypeSpecifier:
        return QLatin1String("ElaboratedTypeSpecifier");
    case NodeType_LinkageBody:
        return QLatin1String("LinkageBody");
    case NodeType_LinkageSpecification:
        return QLatin1String("LinkageSpecification");
    case NodeType_Namespace:
        return QLatin1String("Namespace");
    case NodeType_NamespaceAlias:
        return QLatin1String("NamespaceAlias");
    case NodeType_Using:
        return QLatin1String("Using");
    case NodeType_UsingDirective:
        return QLatin1String("UsingDirective");
    case NodeType_InitDeclaratorList:
        return QLatin1String("InitDeclaratorList");
    case NodeType_Typedef:
        return QLatin1String("Typedef");
    case NodeType_Declarator:
        return QLatin1String("Declarator");
    case NodeType_InitDeclarator:
        return QLatin1String("InitDeclarator");
    case NodeType_TemplateDeclaration:
        return QLatin1String("TemplateDeclaration");
    case NodeType_SimpleDeclaration:
        return QLatin1String("SimpleDeclaration");
    case NodeType_Statement:
        return QLatin1String("Statement");
    case NodeType_IfStatement:
        return QLatin1String("IfStatement");
    case NodeType_WhileStatement:
        return QLatin1String("WhileStatement");
    case NodeType_DoStatement:
        return QLatin1String("DoStatement");
    case NodeType_ForStatement:
        return QLatin1String("ForStatement");
    case NodeType_ForEachStatement: // qt4 [erbsland]
        return QLatin1String("ForEachStatement");
    case NodeType_SwitchStatement:
        return QLatin1String("SwitchStatement");
    case NodeType_CatchStatement:
        return QLatin1String("CatchStatement");
    case NodeType_CatchStatementList:
        return QLatin1String("CatchStatementList");
    case NodeType_TryBlockStatement:
        return QLatin1String("TryBlockStatement");
    case NodeType_DeclarationStatement:
        return QLatin1String("DeclarationStatement");
    case NodeType_StatementList:
        return QLatin1String("StatementList");
    case NodeType_TranslationUnit:
        return QLatin1String("TranslationUnit");
    case NodeType_FunctionDefinition:
        return QLatin1String("FunctionDefinition");
    case NodeType_ExpressionStatement:
        return QLatin1String("ExpressionStatement");
    case NodeType_ParameterDeclaration:
        return QLatin1String("ParameterDeclaration");
    case NodeType_ParameterDeclarationList:
        return QLatin1String("ParameterDeclarationList");
    case NodeType_ParameterDeclarationClause:
        return QLatin1String("ParameterDeclarationClause");
    case NodeType_Group:
        return QLatin1String("Group");
    case NodeType_AccessDeclaration:
        return QLatin1String("AccessDeclaration");
    case NodeType_TypeParameter:
        return QLatin1String("TypeParameter");
    case NodeType_TemplateParameter:
        return QLatin1String("TemplateParameter");
    case NodeType_TemplateParameterList:
        return QLatin1String("TemplateParameterList");
    case NodeType_Condition:
        return QLatin1String("Condition");
    case NodeType_Custom:
        return QLatin1String("Custom");
    }

    return QString();
}


// ------------------------------------------------------------------------
AST::AST()
    : m_nodeType(NodeType_Generic), m_parent(nullptr),
      m_startLine(0), m_startColumn(0),
      m_endLine(0), m_endColumn(0)
{
}

AST::~AST()
{
#ifndef CPPPARSER_NO_CHILDREN
    if (m_parent)
        m_parent->removeChild(this);
#endif
}

void AST::setStartPosition(int line, int col)
{
    m_startLine = line;
    m_startColumn = col;
}

void AST::getStartPosition(int* line, int* col) const
{
    if (line)
        *line = m_startLine;

    if (col)
        * col = m_startColumn;
}

void AST::setEndPosition(int line, int col)
{
    m_endLine = line;
    m_endColumn = col;
}

void AST::getEndPosition(int* line, int* col) const
{
    if (line)
        *line = m_endLine;

    if (col)
        * col = m_endColumn;
}

void AST::setParent(AST* parent)
{
#ifndef CPPPARSER_NO_CHILDREN
    if (m_parent)
        m_parent->removeChild(this);
#endif

    m_parent = parent;

#ifndef CPPPARSER_NO_CHILDREN
    if (m_parent)
        m_parent->appendChild(this);
#endif
}

#ifndef CPPPARSER_NO_CHILDREN
void AST::appendChild(AST* child)
{
    m_children.append(child);
}

void AST::removeChild(AST* child)
{
    m_children.removeOne(child);
}
#endif

// ------------------------------------------------------------------------
NameAST::NameAST()
    : m_global(false)
{
}

void NameAST::setGlobal(bool b)
{
    m_global = b;
}

void NameAST::setUnqualifiedName(ClassOrNamespaceNameAST::Node& unqualifiedName)
{
    m_unqualifiedName = std::move(unqualifiedName);
    if (m_unqualifiedName.get()) m_unqualifiedName->setParent(this);
}

void NameAST::addClassOrNamespaceName(ClassOrNamespaceNameAST::Node& classOrNamespaceName)
{
    if (!classOrNamespaceName.get())
        return;

    classOrNamespaceName->setParent(this);
    m_classOrNamespaceNameList.append(classOrNamespaceName.release());
}

QString NameAST::text() const
{
    if (!m_unqualifiedName.get())
        return QString();

    QString str;

    if (m_global)
        str += QLatin1String("::");

    for (int i = 0; i < m_classOrNamespaceNameList.size(); ++i) {
        str += m_classOrNamespaceNameList.at(i)->text() + QLatin1String("::");
    }

    if (m_unqualifiedName.get())
        str += m_unqualifiedName->text();

    return str;
}

// ------------------------------------------------------------------------
DeclarationAST::DeclarationAST()
{
}

// ------------------------------------------------------------------------
LinkageBodyAST::LinkageBodyAST()
{
}

void LinkageBodyAST::addDeclaration(DeclarationAST::Node& ast)
{
    if (!ast.get())
        return;

    ast->setParent(this);
    m_declarationList.append(ast.release());
}

// ------------------------------------------------------------------------
LinkageSpecificationAST::LinkageSpecificationAST()
{
}

void LinkageSpecificationAST::setExternType(AST::Node& externType)
{
    m_externType = std::move(externType);
    if (m_externType.get()) m_externType->setParent(this);
}

void LinkageSpecificationAST::setLinkageBody(LinkageBodyAST::Node& linkageBody)
{
    m_linkageBody = std::move(linkageBody);
    if (m_linkageBody.get()) m_linkageBody->setParent(this);
}

void LinkageSpecificationAST::setDeclaration(DeclarationAST::Node& decl)
{
    m_declaration = std::move(decl);
    if (m_declaration.get()) m_declaration->setParent(this);
}

// ------------------------------------------------------------------------
TranslationUnitAST::TranslationUnitAST()
{
}

void TranslationUnitAST::addDeclaration(DeclarationAST::Node& ast)
{
    if (!ast.get())
        return;

    ast->setParent(this);
    m_declarationList.append(ast.release());
}

// ------------------------------------------------------------------------
NamespaceAST::NamespaceAST()
{
}

void NamespaceAST::setNamespaceName(AST::Node& namespaceName)
{
    m_namespaceName = std::move(namespaceName);
    if (m_namespaceName.get()) m_namespaceName->setParent(this);
}

void NamespaceAST::setLinkageBody(LinkageBodyAST::Node& linkageBody)
{
    m_linkageBody = std::move(linkageBody);
    if (m_linkageBody.get()) m_linkageBody->setParent(this);
}


// ------------------------------------------------------------------------
NamespaceAliasAST::NamespaceAliasAST()
{
}

void NamespaceAliasAST::setNamespaceName(AST::Node& namespaceName)
{
    m_namespaceName = std::move(namespaceName);
    if (m_namespaceName.get()) m_namespaceName->setParent(this);
}

void NamespaceAliasAST::setAliasName(NameAST::Node& name)
{
    m_aliasName = std::move(name);
    if (m_aliasName.get()) m_aliasName->setParent(this);
}

// ------------------------------------------------------------------------
UsingAST::UsingAST()
{
}

void UsingAST::setTypeName(AST::Node& typeName)
{
    m_typeName = std::move(typeName);
    if (m_typeName.get()) m_typeName->setParent(this);
}

void UsingAST::setName(NameAST::Node& name)
{
    m_name = std::move(name);
    if (m_name.get()) m_name->setParent(this);
}

// ------------------------------------------------------------------------
UsingDirectiveAST::UsingDirectiveAST()
{
}

void UsingDirectiveAST::setName(NameAST::Node& name)
{
    m_name = std::move(name);
    if (m_name.get()) m_name->setParent(this);
}

TypedefAST::TypedefAST()
{
}

void TypeSpecifierAST::setName(NameAST::Node& name)
{
    m_name = std::move(name);
    if (m_name.get()) m_name->setParent(this);
}

void TypedefAST::setTypeSpec(TypeSpecifierAST::Node& typeSpec)
{
    m_typeSpec = std::move(typeSpec);
    if (m_typeSpec.get()) m_typeSpec->setParent(this);
}

void TypedefAST::setInitDeclaratorList(InitDeclaratorListAST::Node& initDeclaratorList)
{
    m_initDeclaratorList = std::move(initDeclaratorList);
    if (m_initDeclaratorList.get()) m_initDeclaratorList->setParent(this);
}

// ------------------------------------------------------------------------
TemplateArgumentListAST::TemplateArgumentListAST()
{
}

void TemplateArgumentListAST::addArgument(AST::Node& arg)
{
    if (!arg.get())
        return;

    arg->setParent(this);
    m_argumentList.append(arg.release());
}

QString TemplateArgumentListAST::text() const
{
    QStringList l;

    for (int i = 0; i < m_argumentList.size(); ++i) {
        l.append(m_argumentList.at(i)->text());
    }

    return l.join(QLatin1String(", "));
}

// ------------------------------------------------------------------------
TemplateDeclarationAST::TemplateDeclarationAST()
{
}

void TemplateDeclarationAST::setExported(AST::Node& exported)
{
    m_exported = std::move(exported);
    if (m_exported.get()) m_exported->setParent(this);
}

void TemplateDeclarationAST::setTemplateParameterList(TemplateParameterListAST::Node& templateParameterList)
{
    m_templateParameterList = std::move(templateParameterList);
    if (m_templateParameterList.get()) m_templateParameterList->setParent(this);
}

void TemplateDeclarationAST::setDeclaration(DeclarationAST::Node& declaration)
{
    m_declaration = std::move(declaration);
    if (m_declaration.get()) m_declaration->setParent(this);
}

// ------------------------------------------------------------------------
ClassOrNamespaceNameAST::ClassOrNamespaceNameAST()
{
}

void ClassOrNamespaceNameAST::setName(AST::Node& name)
{
    m_name = std::move(name);
    if (m_name.get()) m_name->setParent(this);
}

void ClassOrNamespaceNameAST::setTemplateArgumentList(TemplateArgumentListAST::Node& templateArgumentList)
{
    m_templateArgumentList = std::move(templateArgumentList);
    if (m_templateArgumentList.get()) m_templateArgumentList->setParent(this);
}

QString ClassOrNamespaceNameAST::text() const
{
    if (!m_name.get())
        return QString();

    QString str = m_name->text();
    if (m_templateArgumentList.get())
        str += QString::fromLatin1("< ") + m_templateArgumentList->text() + QString::fromLatin1(" >");

    return str;
}

// ------------------------------------------------------------------------
TypeSpecifierAST::TypeSpecifierAST()
{
}

void TypeSpecifierAST::setCvQualify(GroupAST::Node& cvQualify)
{
    m_cvQualify = std::move(cvQualify);
    if (m_cvQualify.get()) m_cvQualify->setParent(this);
}

void TypeSpecifierAST::setCv2Qualify(GroupAST::Node& cv2Qualify)
{
    m_cv2Qualify = std::move(cv2Qualify);
    if (m_cv2Qualify.get()) m_cv2Qualify->setParent(this);
}

QString TypeSpecifierAST::text() const
{
    QString str;

    if (m_cvQualify.get())
        str += m_cvQualify->text() + QLatin1Char(' ');

    if (m_name.get())
        str += m_name->text();

    if (m_cv2Qualify.get())
        str += QString::fromLatin1(" ") + m_cv2Qualify->text();

    return str;
}

// ------------------------------------------------------------------------
ClassSpecifierAST::ClassSpecifierAST()
{
}

void ClassSpecifierAST::setClassKey(AST::Node& classKey)
{
    m_classKey = std::move(classKey);
    if (m_classKey.get()) m_classKey->setParent(this);
}

void ClassSpecifierAST::addDeclaration(DeclarationAST::Node& declaration)
{
    if (!declaration.get())
        return;

    declaration->setParent(this);
    m_declarationList.append(declaration.release());
}

void ClassSpecifierAST::setBaseClause(BaseClauseAST::Node& baseClause)
{
    m_baseClause = std::move(baseClause);
    if (m_baseClause.get()) m_baseClause->setParent(this);
}

// ------------------------------------------------------------------------
EnumSpecifierAST::EnumSpecifierAST()
{
    m_isClass = false;
}

void EnumSpecifierAST::setClass(bool b)
{
    m_isClass = b;
}

void EnumSpecifierAST::setEnumBase(TypeSpecifierAST::Node& enumBase)
{
    m_enumBase = std::move(enumBase);
    if (m_enumBase.get()) m_enumBase->setParent(this);
}

void EnumSpecifierAST::addEnumerator(EnumeratorAST::Node& enumerator)
{
    if (!enumerator.get())
        return;

    enumerator->setParent(this);
    m_enumeratorList.append(enumerator.release());
}


// ------------------------------------------------------------------------
ElaboratedTypeSpecifierAST::ElaboratedTypeSpecifierAST()
{
}

void ElaboratedTypeSpecifierAST::setKind(AST::Node& kind)
{
    m_kind = std::move(kind);
    if (m_kind.get()) m_kind->setParent(this);
}

QString ElaboratedTypeSpecifierAST::text() const
{
    if (m_kind.get())
        return m_kind->text() + QLatin1Char(' ') + TypeSpecifierAST::text();

    return TypeSpecifierAST::text();
}

// ------------------------------------------------------------------------
StatementAST::StatementAST()
{
}

// ------------------------------------------------------------------------
EnumeratorAST::EnumeratorAST()
{
}

void EnumeratorAST::setId(AST::Node& id)
{
    m_id = std::move(id);
    if (m_id.get()) m_id->setParent(this);
}

void EnumeratorAST::setExpr(AST::Node& expr)
{
    m_expr = std::move(expr);
    if (m_expr.get()) m_expr->setParent(this);
}

// ------------------------------------------------------------------------
BaseClauseAST::BaseClauseAST()
{
}

void BaseClauseAST::addBaseSpecifier(BaseSpecifierAST::Node& baseSpecifier)
{
    if (!baseSpecifier.get())
        return;

    baseSpecifier->setParent(this);
    m_baseSpecifierList.append(baseSpecifier.release());
}

// ------------------------------------------------------------------------
BaseSpecifierAST::BaseSpecifierAST()
{
}

void BaseSpecifierAST::setIsVirtual(AST::Node& isVirtual)
{
    m_isVirtual = std::move(isVirtual);
    if (m_isVirtual.get()) m_isVirtual->setParent(this);
}

void BaseSpecifierAST::setAccess(AST::Node& access)
{
    m_access = std::move(access);
    if (m_access.get()) m_access->setParent(this);
}

void BaseSpecifierAST::setName(NameAST::Node& name)
{
    m_name = std::move(name);
    if (m_name.get()) m_name->setParent(this);
}

// ------------------------------------------------------------------------
SimpleDeclarationAST::SimpleDeclarationAST()
{
}

void SimpleDeclarationAST::setFunctionSpecifier(GroupAST::Node& functionSpecifier)
{
    m_functionSpecifier = std::move(functionSpecifier);
    if (m_functionSpecifier.get()) m_functionSpecifier->setParent(this);
}

void SimpleDeclarationAST::setStorageSpecifier(GroupAST::Node& storageSpecifier)
{
    m_storageSpecifier = std::move(storageSpecifier);
    if (m_storageSpecifier.get()) m_storageSpecifier->setParent(this);
}

void SimpleDeclarationAST::setTypeSpec(TypeSpecifierAST::Node& typeSpec)
{
    m_typeSpec = std::move(typeSpec);
    if (m_typeSpec.get()) m_typeSpec->setParent(this);
}

void SimpleDeclarationAST::setInitDeclaratorList(InitDeclaratorListAST::Node& initDeclaratorList)
{
    m_initDeclaratorList = std::move(initDeclaratorList);
    if (m_initDeclaratorList.get()) m_initDeclaratorList->setParent(this);
}

void SimpleDeclarationAST::setWinDeclSpec(GroupAST::Node& winDeclSpec)
{
    m_winDeclSpec = std::move(winDeclSpec);
    if (m_winDeclSpec.get()) m_winDeclSpec->setParent(this);
}


// ------------------------------------------------------------------------
InitDeclaratorListAST::InitDeclaratorListAST()
{
}

void InitDeclaratorListAST::addInitDeclarator(InitDeclaratorAST::Node& decl)
{
    if (!decl.get())
        return;

    decl->setParent(this);
    m_initDeclaratorList.append(decl.release());
}

// ------------------------------------------------------------------------
DeclaratorAST::DeclaratorAST()
{
}

void DeclaratorAST::setSubDeclarator(DeclaratorAST::Node& subDeclarator)
{
    m_subDeclarator = std::move(subDeclarator);
    if (m_subDeclarator.get()) m_subDeclarator->setParent(this);
}

void DeclaratorAST::setDeclaratorId(NameAST::Node& declaratorId)
{
    m_declaratorId = std::move(declaratorId);
    if (m_declaratorId.get()) m_declaratorId->setParent(this);
}

void DeclaratorAST::setBitfieldInitialization(AST::Node& bitfieldInitialization)
{
    m_bitfieldInitialization = std::move(bitfieldInitialization);
    if (m_bitfieldInitialization.get()) m_bitfieldInitialization->setParent(this);
}

void DeclaratorAST::addArrayDimension(AST::Node& arrayDimension)
{
    if (!arrayDimension.get())
        return;

    arrayDimension->setParent(this);
    m_arrayDimensionList.append(arrayDimension.release());
}

void DeclaratorAST::setParameterDeclarationClause(AUTO_PTR<class ParameterDeclarationClauseAST>& parameterDeclarationClause)
{
    m_parameterDeclarationClause = std::move(parameterDeclarationClause);
    if (m_parameterDeclarationClause.get()) m_parameterDeclarationClause->setParent(this);
}

void DeclaratorAST::setConstant(AST::Node& constant)
{
    m_constant = std::move(constant);
    if (m_constant.get()) m_constant->setParent(this);
}

void DeclaratorAST::setOverride(AST::Node& override_)
{
    m_override = std::move(override_);
    if (m_override.get()) m_override->setParent(this);
}

void DeclaratorAST::setFinal(AST::Node& final_)
{
    m_final = std::move(final_);
    if (m_final.get()) m_final->setParent(this);
}

void DeclaratorAST::setExceptionSpecification(GroupAST::Node& exceptionSpecification)
{
    m_exceptionSpecification = std::move(exceptionSpecification);
    if (m_exceptionSpecification.get()) m_exceptionSpecification->setParent(this);
}

void DeclaratorAST::addPtrOp(AST::Node& ptrOp)
{
    if (!ptrOp.get())
        return;

    ptrOp->setParent(this);
    m_ptrOpList.append(ptrOp.release());
}

// --------------------------------------------------------------------------
InitDeclaratorAST::InitDeclaratorAST()
{
}

void InitDeclaratorAST::setDeclarator(DeclaratorAST::Node& declarator)
{
    m_declarator = std::move(declarator);
    if (m_declarator.get()) m_declarator->setParent(this);
}

void InitDeclaratorAST::setInitializer(AST::Node& initializer)
{
    m_initializer = std::move(initializer);
    if (m_initializer.get()) m_initializer->setParent(this);
}

// --------------------------------------------------------------------------
FunctionDefinitionAST::FunctionDefinitionAST()
{
}

void FunctionDefinitionAST::setFunctionSpecifier(GroupAST::Node& functionSpecifier)
{
    m_functionSpecifier = std::move(functionSpecifier);
    if (m_functionSpecifier.get()) m_functionSpecifier->setParent(this);
}

void FunctionDefinitionAST::setStorageSpecifier(GroupAST::Node& storageSpecifier)
{
    m_storageSpecifier = std::move(storageSpecifier);
    if (m_storageSpecifier.get()) m_storageSpecifier->setParent(this);
}

void FunctionDefinitionAST::setTypeSpec(TypeSpecifierAST::Node& typeSpec)
{
    m_typeSpec = std::move(typeSpec);
    if (m_typeSpec.get()) m_typeSpec->setParent(this);
}

void FunctionDefinitionAST::setInitDeclarator(InitDeclaratorAST::Node& initDeclarator)
{
    m_initDeclarator = std::move(initDeclarator);
    if (m_initDeclarator.get()) m_initDeclarator->setParent(this);
}

void FunctionDefinitionAST::setFunctionBody(StatementListAST::Node& functionBody)
{
    m_functionBody = std::move(functionBody);
    if (m_functionBody.get()) m_functionBody->setParent(this);
}

void FunctionDefinitionAST::setWinDeclSpec(GroupAST::Node& winDeclSpec)
{
    m_winDeclSpec = std::move(winDeclSpec);
    if (m_winDeclSpec.get()) m_winDeclSpec->setParent(this);
}

// --------------------------------------------------------------------------
StatementListAST::StatementListAST()
{
}

void StatementListAST::addStatement(StatementAST::Node& statement)
{
    if (!statement.get())
        return;

    statement->setParent(this);
    m_statementList.append(statement.release());
}

// --------------------------------------------------------------------------
IfStatementAST::IfStatementAST()
{
}

void IfStatementAST::setCondition(ConditionAST::Node& condition)
{
    m_condition = std::move(condition);
    if (m_condition.get()) m_condition->setParent(this);
}

void IfStatementAST::setStatement(StatementAST::Node& statement)
{
    m_statement = std::move(statement);
    if (m_statement.get()) m_statement->setParent(this);
}

void IfStatementAST::setElseStatement(StatementAST::Node& elseStatement)
{
    m_elseStatement = std::move(elseStatement);
    if (m_elseStatement.get()) m_elseStatement->setParent(this);
}

// --------------------------------------------------------------------------
WhileStatementAST::WhileStatementAST()
{
}

void WhileStatementAST::setCondition(ConditionAST::Node& condition)
{
    m_condition = std::move(condition);
    if (m_condition.get()) m_condition->setParent(this);
}

void WhileStatementAST::setStatement(StatementAST::Node& statement)
{
    m_statement = std::move(statement);
    if (m_statement.get()) m_statement->setParent(this);
}

// --------------------------------------------------------------------------
DoStatementAST::DoStatementAST()
{
}

void DoStatementAST::setCondition(ConditionAST::Node& condition)
{
    m_condition = std::move(condition);
    if (m_condition.get()) m_condition->setParent(this);
}

void DoStatementAST::setStatement(StatementAST::Node& statement)
{
    m_statement = std::move(statement);
    if (m_statement.get()) m_statement->setParent(this);
}

// --------------------------------------------------------------------------
ForStatementAST::ForStatementAST()
{
}

void ForStatementAST::setCondition(ConditionAST::Node& condition)
{
    m_condition = std::move(condition);
    if (m_condition.get()) m_condition->setParent(this);
}

void ForStatementAST::setExpression(AST::Node& expression)
{
    m_expression = std::move(expression);
    if (m_expression.get()) m_expression->setParent(this);
}

void ForStatementAST::setStatement(StatementAST::Node& statement)
{
    m_statement = std::move(statement);
    if (m_statement.get()) m_statement->setParent(this);
}

void ForStatementAST::setInitStatement(StatementAST::Node& initStatement)
{
    m_initStatement = std::move(initStatement);
    if (m_initStatement.get()) m_initStatement->setParent(this);
}

// --------------------------------------------------------------------------
ForEachStatementAST::ForEachStatementAST()
{
}

void ForEachStatementAST::setExpression(AST::Node& expression)
{
    m_expression = std::move(expression);
    if (m_expression.get()) m_expression->setParent(this);
}

void ForEachStatementAST::setStatement(StatementAST::Node& statement)
{
    m_statement = std::move(statement);
    if (m_statement.get()) m_statement->setParent(this);
}

void ForEachStatementAST::setInitStatement(StatementAST::Node& initStatement)
{
    m_initStatement = std::move(initStatement);
    if (m_initStatement.get()) m_initStatement->setParent(this);
}

// --------------------------------------------------------------------------
SwitchStatementAST::SwitchStatementAST()
{
}

void SwitchStatementAST::setCondition(ConditionAST::Node& condition)
{
    m_condition = std::move(condition);
    if (m_condition.get()) m_condition->setParent(this);
}

void SwitchStatementAST::setStatement(StatementAST::Node& statement)
{
    m_statement = std::move(statement);
    if (m_statement.get()) m_statement->setParent(this);
}

// --------------------------------------------------------------------------
CatchStatementListAST::CatchStatementListAST()
{
}

void CatchStatementListAST::addStatement(CatchStatementAST::Node& statement)
{
    if (!statement.get())
        return;

    statement->setParent(this);
    m_statementList.append(statement.release());
}

// --------------------------------------------------------------------------
CatchStatementAST::CatchStatementAST()
{
}

void CatchStatementAST::setCondition(ConditionAST::Node& condition)
{
    m_condition = std::move(condition);
    if (m_condition.get()) m_condition->setParent(this);
}

void CatchStatementAST::setStatement(StatementAST::Node& statement)
{
    m_statement = std::move(statement);
    if (m_statement.get()) m_statement->setParent(this);
}

// --------------------------------------------------------------------------
TryBlockStatementAST::TryBlockStatementAST()
{
}

void TryBlockStatementAST::setStatement(StatementAST::Node& statement)
{
    m_statement = std::move(statement);
    if (m_statement.get()) m_statement->setParent(this);
}

void TryBlockStatementAST::setCatchStatementList(CatchStatementListAST::Node& statementList)
{
    m_catchStatementList = std::move(statementList);
    if (m_catchStatementList.get()) m_catchStatementList->setParent(this);
}

// --------------------------------------------------------------------------
DeclarationStatementAST::DeclarationStatementAST()
{
}

void DeclarationStatementAST::setDeclaration(DeclarationAST::Node& declaration)
{
    m_declaration = std::move(declaration);
    if (m_declaration.get()) m_declaration->setParent(this);
}

// --------------------------------------------------------------------------
ExpressionStatementAST::ExpressionStatementAST()
{
}

void ExpressionStatementAST::setExpression(AST::Node& expression)
{
    m_expression = std::move(expression);
    if (m_expression.get()) m_expression->setParent(this);
}


// --------------------------------------------------------------------------
ParameterDeclarationAST::ParameterDeclarationAST()
{
}

void ParameterDeclarationAST::setTypeSpec(TypeSpecifierAST::Node& typeSpec)
{
    m_typeSpec = std::move(typeSpec);
    if (m_typeSpec.get()) m_typeSpec->setParent(this);
}

void ParameterDeclarationAST::setDeclarator(DeclaratorAST::Node& declarator)
{
    m_declarator = std::move(declarator);
    if (m_declarator.get()) m_declarator->setParent(this);
}

void ParameterDeclarationAST::setExpression(AST::Node& expression)
{
    m_expression = std::move(expression);
    if (m_expression.get()) m_expression->setParent(this);
}

QString ParameterDeclarationAST::text() const
{
    QString str;
    if (m_typeSpec.get())
        str += m_typeSpec->text() + QLatin1Char(' ');

    if (m_declarator.get())
        str += m_declarator->text();

    if (m_expression.get())
        str += QString::fromLatin1(" = ") + m_expression->text();

    return str;
}

// --------------------------------------------------------------------------
ParameterDeclarationListAST::ParameterDeclarationListAST()
{
}

void ParameterDeclarationListAST::addParameter(ParameterDeclarationAST::Node& parameter)
{
    if (!parameter.get())
        return;

    parameter->setParent(this);
    m_parameterList.append(parameter.release());
}

QString ParameterDeclarationListAST::text() const
{
    QStringList l;
    for (int i = 0; i  < m_parameterList.size(); ++i) {
        l.append(m_parameterList.at(i)->text());
    }
    return l.join(QLatin1String(", "));
}


// --------------------------------------------------------------------------
ParameterDeclarationClauseAST::ParameterDeclarationClauseAST()
{
}

void ParameterDeclarationClauseAST::setParameterDeclarationList(ParameterDeclarationListAST::Node& parameterDeclarationList)
{
    m_parameterDeclarationList = std::move(parameterDeclarationList);
    if (m_parameterDeclarationList.get()) m_parameterDeclarationList->setParent(this);
}

void ParameterDeclarationClauseAST::setEllipsis(AST::Node& ellipsis)
{
    m_ellipsis = std::move(ellipsis);
    if (m_ellipsis.get()) m_ellipsis->setParent(this);
}

QString ParameterDeclarationClauseAST::text() const
{
    QString str;

    if (m_parameterDeclarationList.get())
        str += m_parameterDeclarationList->text();

    if (m_ellipsis.get())
        str += QLatin1String(" ...");

    return str;
}


// --------------------------------------------------------------------------
GroupAST::GroupAST()
{
}

void GroupAST::addNode(AST::Node& node)
{
    if (!node.get())
        return;

    node->setParent(this);
    m_nodeList.append(node.release());
}

QString GroupAST::text() const
{
    QStringList l;
    for (int i = 0; i < m_nodeList.size();  ++i) {
        l.append(m_nodeList.at(i)->text());
    }
    return l.join(QLatin1String(" "));
}

// --------------------------------------------------------------------------
AccessDeclarationAST::AccessDeclarationAST()
{
}

void AccessDeclarationAST::addAccess(AST::Node& access)
{
    if (!access.get())
        return;

    access->setParent(this);
    m_accessList.append(access.release());
}

QString AccessDeclarationAST::text() const
{
    QStringList l;
    for (int i = 0; i < m_accessList.size(); ++i) {
        l.append(m_accessList.at(i)->text());
    }
    return l.join(QLatin1String(" "));
}

// --------------------------------------------------------------------------
TypeParameterAST::TypeParameterAST()
{
}

void TypeParameterAST::setKind(AST::Node& kind)
{
    m_kind = std::move(kind);
    if (m_kind.get()) m_kind->setParent(this);
}

void TypeParameterAST::setTemplateParameterList(AUTO_PTR<class TemplateParameterListAST>& templateParameterList)
{
    m_templateParameterList = std::move(templateParameterList);
    if (m_templateParameterList.get()) m_templateParameterList->setParent(this);
}

void TypeParameterAST::setName(NameAST::Node& name)
{
    m_name = std::move(name);
    if (m_name.get()) m_name->setParent(this);
}

void TypeParameterAST::setTypeId(AST::Node& typeId)
{
    m_typeId = std::move(typeId);
    if (m_typeId.get()) m_typeId->setParent(this);
}

// --------------------------------------------------------------------------
TemplateParameterAST::TemplateParameterAST()
{
}

void TemplateParameterAST::setTypeParameter(TypeParameterAST::Node& typeParameter)
{
    m_typeParameter = std::move(typeParameter);
    if (m_typeParameter.get()) m_typeParameter->setParent(this);
}

void TemplateParameterAST::setTypeValueParameter(ParameterDeclarationAST::Node& typeValueParameter)
{
    m_typeValueParameter = std::move(typeValueParameter);
    if (m_typeValueParameter.get()) m_typeValueParameter->setParent(this);
}

// --------------------------------------------------------------------------
TemplateParameterListAST::TemplateParameterListAST()
{
}

void TemplateParameterListAST::addTemplateParameter(TemplateParameterAST::Node& templateParameter)
{
    if (!templateParameter.get())
        return;

    templateParameter->setParent(this);
    m_templateParameterList.append(templateParameter.release());
}

// --------------------------------------------------------------------------
ConditionAST::ConditionAST()
{
}

void ConditionAST::setTypeSpec(TypeSpecifierAST::Node& typeSpec)
{
    m_typeSpec = std::move(typeSpec);
    if (m_typeSpec.get()) m_typeSpec->setParent(this);
}

void ConditionAST::setDeclarator(DeclaratorAST::Node& declarator)
{
    m_declarator = std::move(declarator);
    if (m_declarator.get()) m_declarator->setParent(this);
}

void ConditionAST::setExpression(AST::Node& expression)
{
    m_expression = std::move(expression);
    if (m_expression.get()) m_expression->setParent(this);
}

void ClassSpecifierAST::setWinDeclSpec(GroupAST::Node & winDeclSpec)
{
    m_winDeclSpec = std::move(winDeclSpec);
    if (m_winDeclSpec.get()) m_winDeclSpec->setParent(this);
}

void ClassSpecifierAST::setFinal(AST::Node& final_)
{
    m_final = std::move(final_);
    if (m_final.get()) m_final->setParent(this);
}

