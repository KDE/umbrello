/* This file is part of KDevelop
    Copyright (C) 2002,2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// c++ support
#include "parser.h"
#include "driver.h"
#include "lexer.h"
#include "errors.h"
#include "debug_utils.h"

// qt support
#include <QtCore/QString>
#include <QtCore/QStringList>

// kde support
#include <klocale.h>

bool Parser::advance(int tk, char const* descr)
{
    const Token& token = (*m_tokenIt);
    if (token != tk) {
        reportError(i18n("'%1' expected found '%2'", QString(descr),
                         token.text()));
        return false;
    }
    ++m_tokenIt;
    return true;
}

bool Parser::check(int tk)
{
    const Token& token = (*m_tokenIt);
    if (token != tk) {
        return false;
    }
    ++m_tokenIt;
    return true;
}

//@todo remove me
enum {
    OBJC_CLASS,
    OBJC_PROTOCOL,
    OBJC_ALIAS
};

struct ParserPrivateData {
    ParserPrivateData() {}
};

Parser::Parser(Driver* driver, Lexer* lexer)
        : m_driver(driver),
        lex(lexer)
{
    d = new ParserPrivateData();
    m_maxProblems = 5;
    objcp = false;
}

Parser::~Parser()
{
    delete d;
    d = 0;
}

bool Parser::reportError(const Error& err)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::reportError()";
    if (m_problems < m_maxProblems) {
        ++m_problems;
        const Token& token = (*m_tokenIt);
        Position position = lex->getTokenPosition(token);

        QString s = (*m_tokenIt).text();
        s = s.left(30).trimmed();
        if (s.isEmpty())
            s = i18n("<eof>");

        m_driver->addProblem(m_driver->currentFileName(), Problem(err.text.subs(s).toString(), position));
    }

    return true;
}

bool Parser::reportError(const QString& msg)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::reportError()";
    if (m_problems < m_maxProblems) {
        ++m_problems;
        const Token& token = (*m_tokenIt);
        Position position = lex->getTokenPosition(token);

        m_driver->addProblem(m_driver->currentFileName(), Problem(msg, position));
    }

    return true;
}

void Parser::syntaxError()
{
    (void) reportError(Errors::SyntaxError);
}

bool Parser::skipUntil(int token)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::skipUntil()";
    while (!(*m_tokenIt).isNull()) {
        if ((*m_tokenIt) == token)
            return true;

        ++m_tokenIt;
    }

    return false;
}

bool Parser::skipUntilDeclaration()
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::skipUntilDeclaration()";

    while (!(*m_tokenIt).isNull()) {

        switch ((*m_tokenIt)) {
        case ';':
        case '~':
        case Token_scope:
        case Token_identifier:
        case Token_operator:
        case Token_char:
        case Token_wchar_t:
        case Token_bool:
        case Token_short:
        case Token_int:
        case Token_long:
        case Token_signed:
        case Token_unsigned:
        case Token_float:
        case Token_double:
        case Token_void:
        case Token_extern:
        case Token_namespace:
        case Token_using:
        case Token_typedef:
        case Token_asm:
        case Token_template:
        case Token_export:

        case Token_const:       // cv
        case Token_volatile:    // cv

        case Token_public:
        case Token_protected:
        case Token_private:
        case Token_signals:      // Qt
        case Token_slots:        // Qt
            return true;

        case '}':
            return false;

        default:
            ++m_tokenIt;
        }
    }

    return false;
}

bool Parser::skipUntilStatement()
{
    //uDebug()<< "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::skipUntilStatement() -- token = " << (*m_tokenIt).text();

    while (!(*m_tokenIt).isNull()) {
        switch ((*m_tokenIt)) {
        case ';':
        case '{':
        case '}':
        case Token_const:
        case Token_volatile:
        case Token_identifier:
        case Token_case:
        case Token_default:
        case Token_if:
        case Token_switch:
        case Token_while:
        case Token_do:
        case Token_for:
        case Token_break:
        case Token_continue:
        case Token_return:
        case Token_goto:
        case Token_try:
        case Token_catch:
        case Token_throw:
        case Token_char:
        case Token_wchar_t:
        case Token_bool:
        case Token_short:
        case Token_int:
        case Token_long:
        case Token_signed:
        case Token_unsigned:
        case Token_float:
        case Token_double:
        case Token_void:
        case Token_class:
        case Token_struct:
        case Token_union:
        case Token_enum:
        case Token_scope:
        case Token_template:
        case Token_using:
            return true;

        default:
            ++m_tokenIt;
        }
    }

    return false;
}

bool Parser::skip(int l, int r)
{
    int count = 0;
    while (!(*m_tokenIt).isNull()) {
        int tk = (*m_tokenIt);

        if (tk == l)
            ++count;
        else if (tk == r)
            --count;
        else if (l != '{' && (tk == '{' || tk == '}' || tk == ';'))
            return false;

        if (count == 0)
            return true;

        ++m_tokenIt;
    }

    return false;
}

bool Parser::skipCommaExpression(AST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::skipCommaExpression()";

    TokenIterator start = m_tokenIt;

    AST::Node expr;
    if (!skipExpression(expr))
        return false;

    QString comment;
    while ((*m_tokenIt) == ',') {
        comment.clear();
        advanceAndCheckTrailingComment(comment);

        if (!skipExpression(expr)) {
            reportError(i18n("expression expected"));
            return false;
        }
    }

    AST::Node ast = CreateNode<AST>();
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::skipExpression(AST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::skipExpression()";

    TokenIterator start = m_tokenIt;

    while (!(*m_tokenIt).isNull()) {
        int tk = (*m_tokenIt);

        switch (tk) {
        case '(':
            skip('(', ')');
            ++m_tokenIt;
            break;

        case '[':
            skip('[', ']');
            ++m_tokenIt;
            break;

#if 0
        case Token_identifier:
            ++m_tokenIt;
            if ((*m_tokenIt) == Token_identifier)
                return true;
            break;
#endif

        case ';':
        case ',':
        case ']':
        case ')':
        case '{':
        case '}':
        case Token_case:
        case Token_default:
        case Token_if:
        case Token_while:
        case Token_do:
        case Token_for:
        case Token_break:
        case Token_continue:
        case Token_return:
        case Token_goto: {
            AST::Node ast = CreateNode<AST>();
            update_pos(ast, start, m_tokenIt);
            node = ast;
        }
        return true;

        default:
            ++m_tokenIt;
        }
    }

    return false;
}

bool Parser::parseName(NameAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseName()";

    GroupAST::Node winDeclSpec;
    parseWinDeclSpec(winDeclSpec);

    TokenIterator start = m_tokenIt;

    NameAST::Node ast = CreateNode<NameAST>();

    if ((*m_tokenIt) == Token_scope) {
        ast->setGlobal(true);
        ++m_tokenIt;
    }

    TokenIterator idx = m_tokenIt;

    while (true) {
        ClassOrNamespaceNameAST::Node n;
        if (!parseUnqualifiedName(n)) {
            return false;
        }

        if ((*m_tokenIt) == Token_scope) {
            ++m_tokenIt;
            ast->addClassOrNamespaceName(n);
            if ((*m_tokenIt) == Token_template)
                ++m_tokenIt; /// skip optional template     #### @todo CHECK
        } else {
            ast->setUnqualifiedName(n);
            break;
        }
    }

    if (idx == m_tokenIt)
        return false;

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseTranslationUnit(TranslationUnitAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseTranslationUnit()";

    TokenIterator start = m_tokenIt = lex->tokenBegin();

    m_problems = 0;
    TranslationUnitAST::Node tun = CreateNode<TranslationUnitAST>();
    node = tun;
    while (!(*m_tokenIt).isNull()) {
        DeclarationAST::Node def;
        TokenIterator startDecl = m_tokenIt;
        if (!parseDeclaration(def)) {
            // error recovery
            if (startDecl == m_tokenIt)
                ++m_tokenIt; // skip at least one token
            skipUntilDeclaration();
        }
        node->addDeclaration(def);
    }

    update_pos(node, start, m_tokenIt);

    // force (0,0) as start position
    node->setStartPosition(Position());

    return m_problems == 0;
}

bool Parser::parseDeclaration(DeclarationAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseDeclaration()";

    // catch first comment
    Position ps = m_tokenIt->getStartPosition();
    if (ps.line == 1 && ps.column == 1 && (*m_tokenIt) == Token_comment)
    {
        FileAST::Node ast = CreateNode<FileAST>();
        ast->setFileName(m_driver->currentFileName());
        QString comment = (*m_tokenIt).text();

        if ((*m_tokenIt) == Token_comment) {
            ast->setComment(comment);
            ++m_tokenIt;
        }
        uDebug() << m_driver->currentFileName() << comment;
        node = ast;
        return true;
    }

    QString comment;
    while ((*m_tokenIt) == Token_comment) {
        comment += (*m_tokenIt).text();
        ++m_tokenIt;
    }

    if ((*m_tokenIt).isNull()) {
        // FIXME: add fetched comment to FileAST
        return false;
    }

    TokenIterator start = m_tokenIt;
    bool success = false;

    switch ((*m_tokenIt)) {

    case ';':
        ++m_tokenIt;
        return true;

    case Token_extern:
        success = parseLinkageSpecification(node);
        break;

    case Token_namespace:
        success = parseNamespace(node);
        break;

    case Token_using:
        success = parseUsing(node);
        break;

    case Token_typedef:
        success = parseTypedef(node);
        break;

    case Token_asm:
        success = parseAsmDefinition(node);
        break;

    case Token_template:
    case Token_export:
        success = parseTemplateDeclaration(node);
        break;

    default: {
        // m_tokenIt = start;

        if (objcp && parseObjcDef(node))
            return true;

        m_tokenIt = start;

        GroupAST::Node storageSpec;
        parseStorageClassSpecifier(storageSpec);

        GroupAST::Node cv;
        parseCvQualify(cv);

        TypeSpecifierAST::Node spec;
        AST::Node declarator;
        if (parseEnumSpecifier(spec) || parseClassSpecifier(spec)) {
            spec->setCvQualify(cv);

            GroupAST::Node cv2;
            parseCvQualify(cv2);
            spec->setCv2Qualify(cv2);

            InitDeclaratorListAST::Node declarators;
            parseInitDeclaratorList(declarators);

            if (!advance(';', ";"))
                return false;

            if (!comment.isEmpty()) {
                //uDebug() << "Parser::parseDeclaration(spec): comment is " << comment;
                spec->setComment(comment);
            }

            SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
            ast->setStorageSpecifier(storageSpec);
            ast->setTypeSpec(spec);
            ast->setInitDeclaratorList(declarators);
            update_pos(ast, start, m_tokenIt);
            node = ast;

            return true;
        }

        m_tokenIt = start;
        success = parseDeclarationInternal(node, comment);
    }

    } // end switch

    if (success && !comment.isEmpty()) {
        //uDebug() << "Parser::parseDeclaration(): comment is " << comment;
        node->setComment(comment);
    }
    return success;
}

bool Parser::parseLinkageSpecification(DeclarationAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseLinkageSpecification()";

    TokenIterator start = m_tokenIt;

    if ((*m_tokenIt) != Token_extern) {
        return false;
    }
    ++m_tokenIt;

    LinkageSpecificationAST::Node ast = CreateNode<LinkageSpecificationAST>();

    TokenIterator startExternType = m_tokenIt;
    if ((*m_tokenIt) == Token_string_literal) {
        ++m_tokenIt;
        AST::Node externType = CreateNode<AST>();
        update_pos(externType, startExternType, m_tokenIt);

        ast->setExternType(externType);
    }

    if ((*m_tokenIt) == '{') {
        LinkageBodyAST::Node linkageBody;
        parseLinkageBody(linkageBody);
        ast->setLinkageBody(linkageBody);
    } else {
        DeclarationAST::Node decl;
        if (!parseDeclaration(decl)) {
            reportError(i18n("Declaration syntax error"));
        }
        ast->setDeclaration(decl);
    }

    update_pos(ast, start, m_tokenIt);

    node = ast;

    return true;
}

bool Parser::parseLinkageBody(LinkageBodyAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseLinkageBody()";

    TokenIterator start = m_tokenIt;

    if ((*m_tokenIt) != '{') {
        return false;
    }
    ++m_tokenIt;

    LinkageBodyAST::Node lba = CreateNode<LinkageBodyAST>();
    node = lba;

    while (!(*m_tokenIt).isNull()) {
        int tk = (*m_tokenIt);

        if (tk == '}')
            break;

        DeclarationAST::Node def;
        TokenIterator startDecl = m_tokenIt;
        if (parseDeclaration(def)) {
            node->addDeclaration(def);
        } else {
            // error recovery
            if (startDecl == m_tokenIt)
                ++m_tokenIt; // skip at least one token
            skipUntilDeclaration();
        }
    }

    if ((*m_tokenIt) != '}') {
        reportError(i18n("} expected"));
    } else
        ++m_tokenIt;

    update_pos(node, start, m_tokenIt);
    return true;
}

bool Parser::parseNamespace(DeclarationAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseNamespace()";

    TokenIterator start = m_tokenIt;

    if ((*m_tokenIt) != Token_namespace) {
        return false;
    }
    ++m_tokenIt;

    TokenIterator startNamespaceName = m_tokenIt;
    if ((*m_tokenIt) == Token_identifier) {
        ++m_tokenIt;
    }
    AST::Node namespaceName = CreateNode<AST>();
    update_pos(namespaceName, startNamespaceName, m_tokenIt);

    if ((*m_tokenIt) == '=') {
        // namespace alias
        ++m_tokenIt;

        NameAST::Node name;
        if (parseName(name)) {
            if (!advance(';', ";"))
                return false;

            NamespaceAliasAST::Node ast = CreateNode<NamespaceAliasAST>();
            ast->setNamespaceName(namespaceName);
            ast->setAliasName(name);
            update_pos(ast, start, m_tokenIt);
            node = ast;
            return true;
        } else {
            reportError(i18n("namespace expected"));
            return false;
        }
    } else if ((*m_tokenIt) != '{') {
        reportError(i18n("{ expected"));
        return false;
    }

    NamespaceAST::Node ast = CreateNode<NamespaceAST>();
    ast->setNamespaceName(namespaceName);

    LinkageBodyAST::Node linkageBody;
    parseLinkageBody(linkageBody);

    ast->setLinkageBody(linkageBody);
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseUsing(DeclarationAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseUsing()";

    TokenIterator start = m_tokenIt;

    if ((*m_tokenIt) != Token_using) {
        return false;
    }
    ++m_tokenIt;

    if ((*m_tokenIt) == Token_namespace) {
        if (!parseUsingDirective(node)) {
            return false;
        }
        update_pos(node, start, m_tokenIt);
        return true;
    }

    UsingAST::Node ast = CreateNode<UsingAST>();

    TokenIterator startTypeName = m_tokenIt;
    if ((*m_tokenIt) == Token_typename) {
        ++m_tokenIt;
        AST::Node tn = CreateNode<AST>();
        update_pos(tn, startTypeName, m_tokenIt);
        ast->setTypeName(tn);
    }

    NameAST::Node name;
    if (!parseName(name))
        return false;

    ast->setName(name);

    if (!advance(';', ";"))
        return false;

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseUsingDirective(DeclarationAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseUsingDirective()";

    TokenIterator start = m_tokenIt;

    if ((*m_tokenIt) != Token_namespace) {
        return false;
    }
    ++m_tokenIt;

    NameAST::Node name;
    if (!parseName(name)) {
        reportError(i18n("Namespace name expected"));
        return false;
    }

    if (!advance(';', ";"))
        return false;

    UsingDirectiveAST::Node ast = CreateNode<UsingDirectiveAST>();
    ast->setName(name);
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseOperatorFunctionId(AST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseOperatorFunctionId()";

    TokenIterator start = m_tokenIt;

    if ((*m_tokenIt) != Token_operator) {
        return false;
    }
    ++m_tokenIt;

    AST::Node op;
    if (parseOperator(op)) {
        AST::Node asn = CreateNode<AST>();
        node = asn;
        update_pos(node, start, m_tokenIt);
        return true;
    } else {
        // parse cast operator
        GroupAST::Node cv;
        parseCvQualify(cv);

        TypeSpecifierAST::Node spec;
        if (!parseSimpleTypeSpecifier(spec)) {
            syntaxError();
            return false;
        }
        spec->setCvQualify(cv);

        GroupAST::Node cv2;
        parseCvQualify(cv2);
        spec->setCv2Qualify(cv2);

        AST::Node ptrOp;
        while (parsePtrOperator(ptrOp))
            ;

        AST::Node asn = CreateNode<AST>();
        node = asn;
        update_pos(node, start, m_tokenIt);
        return true;
    }
}

bool Parser::parseTemplateArgumentList(TemplateArgumentListAST::Node& node, bool reportError)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseTemplateArgumentList()";

    TokenIterator start = m_tokenIt;

    TemplateArgumentListAST::Node ast = CreateNode<TemplateArgumentListAST>();

    AST::Node templArg;
    if (!parseTemplateArgument(templArg))
        return false;
    ast->addArgument(templArg);

    QString comment;
    while ((*m_tokenIt) == ',') {
        comment.clear();
        advanceAndCheckTrailingComment(comment);

        if (!parseTemplateArgument(templArg)) {
            if (reportError) {
                syntaxError();
                break;
            } else
                return false;
        }
        if (!comment.isEmpty())
            templArg->setComment(comment);
        ast->addArgument(templArg);
    }

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseTypedef(DeclarationAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseTypedef()";

    TokenIterator start = m_tokenIt;

    if ((*m_tokenIt) != Token_typedef) {
        return false;
    }
    ++m_tokenIt;

    TypeSpecifierAST::Node spec;
    if (!parseTypeSpecifierOrClassSpec(spec)) {
        reportError(i18n("Need a type specifier to declare"));
        return false;
    }

    InitDeclaratorListAST::Node declarators;
    if (!parseInitDeclaratorList(declarators)) {
        //reportError( i18n("Need an identifier to declare") );
        //return false;
    }

    if (!advance(';', ";"))
        return false;

    TypedefAST::Node ast = CreateNode<TypedefAST>();
    ast->setTypeSpec(spec);
    ast->setInitDeclaratorList(declarators);
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseAsmDefinition(DeclarationAST::Node& /*node*/)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseAsmDefinition()";

    if (!advance(Token_asm, "asm"))
        return false;

    GroupAST::Node cv;
    parseCvQualify(cv);

    skip('(', ')');
    if (!advance(')', ")"))
        return false;
    if (!advance(';', ";"))
        return false;

    return true;
}

bool Parser::parseTemplateDeclaration(DeclarationAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseTemplateDeclaration()";

    TokenIterator start = m_tokenIt;

    AST::Node exp;

    TokenIterator startExport = m_tokenIt;
    if ((*m_tokenIt) == Token_export) {
        ++m_tokenIt;
        AST::Node n = CreateNode<AST>();
        update_pos(n, startExport, m_tokenIt);
        exp = n;
    }

    if ((*m_tokenIt) != Token_template) {
        return false;
    }
    ++m_tokenIt;

    TemplateParameterListAST::Node params;
    if ((*m_tokenIt) == '<') {
        ++m_tokenIt;
        if ((*m_tokenIt) != '>')
            parseTemplateParameterList(params);

        if (!advance('>', ">"))
            return false;
    }

    DeclarationAST::Node def;
    if (!parseDeclaration(def)) {
        reportError(i18n("expected a declaration"));
    }

    TemplateDeclarationAST::Node ast = CreateNode<TemplateDeclarationAST>();
    ast->setExported(exp);
    ast->setTemplateParameterList(params);
    ast->setDeclaration(def);
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseOperator(AST::Node& /*node*/)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseOperator()";
    QString text = (*m_tokenIt).text();

    switch ((*m_tokenIt)) {
    case Token_new:
    case Token_delete:
        ++m_tokenIt;
        if ((*m_tokenIt) == '[' && lex->lookAhead(m_tokenIt, 1) == ']') {
            ++m_tokenIt;
            ++m_tokenIt;
            text += "[]";
        }
        return true;

    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '^':
    case '&':
    case '|':
    case '~':
    case '!':
    case '=':
    case '<':
    case '>':
    case ',':
    case Token_assign:
    case Token_shift:
    case Token_eq:
    case Token_not_eq:
    case Token_leq:
    case Token_geq:
    case Token_and:
    case Token_or:
    case Token_incr:
    case Token_decr:
    case Token_ptrmem:
    case Token_arrow:
        ++m_tokenIt;
        return true;

    default:
        if ((*m_tokenIt) == '(' && lex->lookAhead(m_tokenIt, 1) == ')') {
            ++m_tokenIt;
            ++m_tokenIt;
            return true;
        } else if ((*m_tokenIt) == '[' && lex->lookAhead(m_tokenIt, 1) == ']') {
            ++m_tokenIt;
            ++m_tokenIt;
            return true;
        }
    }

    return false;
}

bool Parser::parseCvQualify(GroupAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseCvQualify()";

    TokenIterator start = m_tokenIt;

    GroupAST::Node ast = CreateNode<GroupAST>();

    int n = 0;
    while (!(*m_tokenIt).isNull()) {
        int tk = (*m_tokenIt);
        if (tk == Token_const || tk == Token_volatile) {
            ++n;
            TokenIterator startWord = m_tokenIt;
            ++m_tokenIt;
            AST::Node word = CreateNode<AST>();
            update_pos(word, startWord, m_tokenIt);
            ast->addNode(word);
        } else
            break;
    }

    if (n == 0)
        return false;


    //uDebug() << "-----------------> token = " << (*m_tokenIt).text();
    update_pos(ast, start, m_tokenIt);

    node = ast;
    return true;
}

bool Parser::parseSimpleTypeSpecifier(TypeSpecifierAST::Node& node)
{
    TokenIterator start = m_tokenIt;
    bool isIntegral = false;
    bool done = false;

    while (!done) {

        switch ((*m_tokenIt)) {
        case Token_char:
        case Token_wchar_t:
        case Token_bool:
        case Token_short:
        case Token_int:
        case Token_long:
        case Token_signed:
        case Token_unsigned:
        case Token_float:
        case Token_double:
        case Token_void:
            isIntegral = true;
            ++m_tokenIt;
            break;

        default:
            done = true;
        }
    }

    TypeSpecifierAST::Node ast = CreateNode<TypeSpecifierAST>();
    if (isIntegral) {
        ClassOrNamespaceNameAST::Node cl = CreateNode<ClassOrNamespaceNameAST>();

        AST::Node n = CreateNode<AST>();
        update_pos(n, start, m_tokenIt);
        cl->setName(n);
        update_pos(cl, start, m_tokenIt);

        NameAST::Node name = CreateNode<NameAST>();
        name->setUnqualifiedName(cl);
        update_pos(name, start, m_tokenIt);
        ast->setName(name);

    } else {
        NameAST::Node name;
        if (!parseName(name)) {
            m_tokenIt = start;
            return false;
        }
        ast->setName(name);
    }

    update_pos(ast, start, m_tokenIt);
    node = ast;
    return true;
}

bool Parser::parsePtrOperator(AST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parsePtrOperator()";

    TokenIterator start = m_tokenIt;

    if ((*m_tokenIt) == '&') {
        ++m_tokenIt;
    } else if ((*m_tokenIt) == '*') {
        ++m_tokenIt;
    } else {
        TokenIterator index = m_tokenIt;
        AST::Node memPtr;
        if (!parsePtrToMember(memPtr)) {
            m_tokenIt = index;
            return false;
        }
    }

    GroupAST::Node cv;
    parseCvQualify(cv);

    AST::Node ast = CreateNode<AST>();
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseTemplateArgument(AST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseTemplateArgument()";

    TokenIterator start = m_tokenIt;
    if (parseTypeId(node)) {
        if ((*m_tokenIt) == ',' || (*m_tokenIt) == '>')
            return true;
    }

    m_tokenIt = start;
    if (!parseLogicalOrExpression(node, true)) {
        return false;
    }

    return true;
}

bool Parser::parseTypeSpecifier(TypeSpecifierAST::Node& spec)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseTypeSpecifier()";

    GroupAST::Node cv;
    parseCvQualify(cv);

    if (parseElaboratedTypeSpecifier(spec) || parseSimpleTypeSpecifier(spec)) {
        spec->setCvQualify(cv);

        GroupAST::Node cv2;
        parseCvQualify(cv2);
        spec->setCv2Qualify(cv2);

        return true;
    }

    return false;
}

bool Parser::parseDeclarator(DeclaratorAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseDeclarator()";

    TokenIterator start = m_tokenIt;

    DeclaratorAST::Node ast = CreateNode<DeclaratorAST>();

    DeclaratorAST::Node decl;
    NameAST::Node declId;

    AST::Node ptrOp;
    while (parsePtrOperator(ptrOp)) {
        ast->addPtrOp(ptrOp);
    }

    if ((*m_tokenIt) == '(') {
        ++m_tokenIt;

        if (!parseDeclarator(decl)) {
            return false;
        }
        ast->setSubDeclarator(decl);

        if ((*m_tokenIt) != ')') {
            return false;
        }
        ++m_tokenIt;
    } else {

        if ((*m_tokenIt) == ':') {
            // unnamed bitfield
        } else if (parseDeclaratorId(declId)) {
            ast->setDeclaratorId(declId);
        } else {
            m_tokenIt = start;
            return false;
        }

        if ((*m_tokenIt) == ':') {
            ++m_tokenIt;
            AST::Node expr;
            if (!parseConstantExpression(expr)) {
                reportError(i18n("Constant expression expected"));
            }
            goto update_pos;
        }
    }

    {
        bool isVector = true;

        while ((*m_tokenIt) == '[') {
            TokenIterator startArray = m_tokenIt;
            ++m_tokenIt;
            AST::Node expr;
            parseCommaExpression(expr);

            if (!advance(']', "]"))
                return false;
            AST::Node array = CreateNode<AST>();
            update_pos(array, startArray, m_tokenIt);
            ast->addArrayDimension(array);
            isVector = true;
        }

        bool skipParen = false;
        if ((*m_tokenIt) == Token_identifier && lex->lookAhead(m_tokenIt, 1) == '(' && lex->lookAhead(m_tokenIt, 2) == '(') {
            ++m_tokenIt;
            ++m_tokenIt;
            skipParen = true;
        }

        if (ast->subDeclarator() && (!isVector || (*m_tokenIt) != '(')) {
            m_tokenIt = start;
            return false;
        }

        TokenIterator index = m_tokenIt;
        if ((*m_tokenIt) == '(') {
            ++m_tokenIt;

            ParameterDeclarationClauseAST::Node params;
            if (!parseParameterDeclarationClause(params)) {
                //uDebug() << "----------------------> not a parameter declaration, maybe an initializer!?";
                m_tokenIt = index;
                goto update_pos;
            }
            ast->setParameterDeclarationClause(params);

            if ((*m_tokenIt) != ')') {
                m_tokenIt = index;
                goto update_pos;
            }

            ++m_tokenIt;  // skip ')'

            TokenIterator startConstant = m_tokenIt;
            if ((*m_tokenIt) == Token_const) {
                ++m_tokenIt;
                AST::Node constant = CreateNode<AST>();
                update_pos(constant, startConstant, m_tokenIt);
                ast->setConstant(constant);
            }

            GroupAST::Node except;
            if (parseExceptionSpecification(except)) {
                ast->setExceptionSpecification(except);
            }
        }

        if (skipParen) {
            if ((*m_tokenIt) != ')') {
                reportError(i18n("')' expected"));
            } else
                ++m_tokenIt;
        }
    }

update_pos:
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseAbstractDeclarator(DeclaratorAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseDeclarator()";
    TokenIterator start = m_tokenIt;

    DeclaratorAST::Node ast = CreateNode<DeclaratorAST>();

    DeclaratorAST::Node decl;
    NameAST::Node declId;

    AST::Node ptrOp;
    while (parsePtrOperator(ptrOp)) {
        ast->addPtrOp(ptrOp);
    }

    if ((*m_tokenIt) == '(') {
        ++m_tokenIt;

        if (!parseAbstractDeclarator(decl)) {
            return false;
        }
        ast->setSubDeclarator(decl);

        if ((*m_tokenIt) != ')') {
            return false;
        }
        ++m_tokenIt;
    }

    {
        while ((*m_tokenIt) == '[') {
            TokenIterator startArray = m_tokenIt;
            ++m_tokenIt;
            AST::Node expr;
            skipCommaExpression(expr);

            if (!advance(']', "]"))
                return false;
            AST::Node array = CreateNode<AST>();
            update_pos(array, startArray, m_tokenIt);
            ast->addArrayDimension(array);
        }

        bool skipParen = false;
        if ((*m_tokenIt) == Token_identifier && lex->lookAhead(m_tokenIt, 1) == '(' && lex->lookAhead(m_tokenIt, 2) == '(') {
            ++m_tokenIt;
            ++m_tokenIt;
            skipParen = true;
        }

        TokenIterator index = m_tokenIt;
        if ((*m_tokenIt) == '(') {
            ++m_tokenIt;

            ParameterDeclarationClauseAST::Node params;
            if (!parseParameterDeclarationClause(params)) {
                m_tokenIt = index;
                goto update_pos;
            }
            ast->setParameterDeclarationClause(params);

            if ((*m_tokenIt) != ')') {
                m_tokenIt = index;
                goto update_pos;
            } else
                ++m_tokenIt;

            TokenIterator startConstant = m_tokenIt;
            if ((*m_tokenIt) == Token_const) {
                ++m_tokenIt;
                AST::Node constant = CreateNode<AST>();
                update_pos(constant, startConstant, m_tokenIt);
                ast->setConstant(constant);
            }

            GroupAST::Node except;
            if (parseExceptionSpecification(except)) {
                ast->setExceptionSpecification(except);
            }
        }

        if (skipParen) {
            if ((*m_tokenIt) != ')') {
                reportError(i18n("')' expected"));
            } else
                ++m_tokenIt;
        }

    }

update_pos:
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseEnumSpecifier(TypeSpecifierAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseEnumSpecifier()";

    QString comment;
    while ((*m_tokenIt) == Token_comment) {
        comment += (*m_tokenIt).text();
        ++m_tokenIt;
    }
    if ((*m_tokenIt).isNull())
        return false;

    TokenIterator start = m_tokenIt;

    if ((*m_tokenIt) != Token_enum) {
        return false;
    }

    ++m_tokenIt;

    NameAST::Node name;
    parseName(name);

    if ((*m_tokenIt) != '{') {
        m_tokenIt = start;
        return false;
    }
    ++m_tokenIt;

    EnumSpecifierAST::Node ast = CreateNode<EnumSpecifierAST>();
    ast->setName(name);

    EnumeratorAST::Node enumerator;
    if (parseEnumerator(enumerator)) {
        ast->addEnumerator(enumerator);

        QString comment;
        while ((*m_tokenIt) == ',') {
            comment = "";
            advanceAndCheckTrailingComment(comment);
            if (!comment.isEmpty()) {
                EnumeratorAST *lastLit = ast->enumeratorList().last();
                if (lastLit)
                    lastLit->setComment(comment);
            }

            if (!parseEnumerator(enumerator)) {
                //reportError( i18n("Enumerator expected") );
                break;
            }

            ast->addEnumerator(enumerator);
        }
    }

    if ((*m_tokenIt) == Token_comment)
        ++m_tokenIt;
    if ((*m_tokenIt) != '}')
        reportError(i18n("} missing"));
    else
        ++m_tokenIt;

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseTemplateParameterList(TemplateParameterListAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseTemplateParameterList()";

    TokenIterator start = m_tokenIt;

    TemplateParameterListAST::Node ast = CreateNode<TemplateParameterListAST>();

    TemplateParameterAST::Node param;
    if (!parseTemplateParameter(param)) {
        return false;
    }
    ast->addTemplateParameter(param);

    QString comment;
    while ((*m_tokenIt) == ',') {
        comment.clear();
        advanceAndCheckTrailingComment(comment);

        if (!parseTemplateParameter(param)) {
            syntaxError();
            break;
        } else {
            if (!comment.isEmpty())
                param->setComment(comment);
            ast->addTemplateParameter(param);
        }
    }

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseTemplateParameter(TemplateParameterAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseTemplateParameter()";

    TokenIterator start = m_tokenIt;
    TemplateParameterAST::Node ast = CreateNode<TemplateParameterAST>();

    TypeParameterAST::Node typeParameter;
    ParameterDeclarationAST::Node param;

    int tk = (*m_tokenIt);

    if ((tk == Token_class || tk == Token_typename || tk == Token_template) && parseTypeParameter(typeParameter)) {
        ast->setTypeParameter(typeParameter);
        goto ok;
    }

    if (!parseParameterDeclaration(param))
        return false;
    ast->setTypeValueParameter(param);

ok:
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseTypeParameter(TypeParameterAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseTypeParameter()";

    TokenIterator start = m_tokenIt;
    TypeParameterAST::Node ast = CreateNode<TypeParameterAST>();

    AST::Node kind = ast_from_token(m_tokenIt);
    ast->setKind(kind);

    switch ((*m_tokenIt)) {

    case Token_class:
    case Token_typename: {
        ++m_tokenIt; // skip class

        // parse optional name
        NameAST::Node name;
        if (parseName(name)) {
            ast->setName(name);
            if ((*m_tokenIt) == '=') {
                ++m_tokenIt;

                AST::Node typeId;
                if (!parseTypeId(typeId)) {
                    syntaxError();
                    return false;
                }
                ast->setTypeId(typeId);
            }
        }
    }
    break;

    case Token_template: {
        ++m_tokenIt; // skip template
        if (!advance('<', "<"))
            return false;

        TemplateParameterListAST::Node params;
        if (!parseTemplateParameterList(params)) {
            return false;
        }
        ast->setTemplateParameterList(params);

        if (!advance('>', ">"))
            return false;

        if ((*m_tokenIt) == Token_class)
            ++m_tokenIt;

        // parse optional name
        NameAST::Node name;
        if (parseName(name)) {
            ast->setName(name);
            if ((*m_tokenIt) == '=') {
                ++m_tokenIt;

                AST::Node typeId;
                if (!parseTypeId(typeId)) {
                    syntaxError();
                    return false;
                }
                ast->setTypeId(typeId);
            }
        }

        if ((*m_tokenIt) == '=') {
            ++m_tokenIt;

            NameAST::Node templ_name;
            parseName(templ_name);
        }
    }
    break;

    default:
        return false;

    } // end switch


    update_pos(ast, start, m_tokenIt);
    node = ast;
    return true;
}

bool Parser::parseStorageClassSpecifier(GroupAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseStorageClassSpecifier()";

    TokenIterator start = m_tokenIt;
    GroupAST::Node ast = CreateNode<GroupAST>();

    while (!(*m_tokenIt).isNull()) {
        int tk = (*m_tokenIt);
        if (tk == Token_friend || tk == Token_auto || tk == Token_register || tk == Token_static ||
                tk == Token_extern || tk == Token_mutable) {
            TokenIterator startNode = m_tokenIt;
            ++m_tokenIt;

            AST::Node n = CreateNode<AST>();
            update_pos(n, startNode, m_tokenIt);
            ast->addNode(n);
        } else
            break;
    }

    if (ast->nodeList().count() == 0)
        return false;

    update_pos(ast, start, m_tokenIt);
    node = ast;
    return true;
}

bool Parser::parseFunctionSpecifier(GroupAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseFunctionSpecifier()";

    TokenIterator start = m_tokenIt;
    GroupAST::Node ast = CreateNode<GroupAST>();

    while (!(*m_tokenIt).isNull()) {
        int tk = (*m_tokenIt);
        if (tk == Token_inline || tk == Token_virtual || tk == Token_explicit) {
            TokenIterator startNode = m_tokenIt;
            ++m_tokenIt;

            AST::Node n = CreateNode<AST>();
            update_pos(n, startNode, m_tokenIt);
            ast->addNode(n);
        } else {
            break;
        }
    }

    if (ast->nodeList().count() == 0)
        return false;

    update_pos(ast, start, m_tokenIt);
    node = ast;
    return true;
}

bool Parser::parseTypeId(AST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseTypeId()";

    /// @todo implement the AST for typeId
    TokenIterator start = m_tokenIt;
    AST::Node ast = CreateNode<AST>();

    TypeSpecifierAST::Node spec;
    if (!parseTypeSpecifier(spec)) {
        return false;
    }

    DeclaratorAST::Node decl;
    parseAbstractDeclarator(decl);

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseInitDeclaratorList(InitDeclaratorListAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseInitDeclaratorList()";

    TokenIterator start = m_tokenIt;

    InitDeclaratorListAST::Node ast = CreateNode<InitDeclaratorListAST>();
    InitDeclaratorAST::Node decl;

    if (!parseInitDeclarator(decl)) {
        return false;
    }
    ast->addInitDeclarator(decl);

    QString comment;
    while ((*m_tokenIt) == ',') {
        comment = "";
        advanceAndCheckTrailingComment(comment);

        if (!parseInitDeclarator(decl)) {
            syntaxError();
            break;
        }
        if (!comment.isEmpty())
            decl->setComment(comment);
        ast->addInitDeclarator(decl);
    }
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseInitDeclaratorList() -- end";

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseParameterDeclarationClause(ParameterDeclarationClauseAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseParameterDeclarationClause()";

    TokenIterator start = m_tokenIt;

    ParameterDeclarationClauseAST::Node ast = CreateNode<ParameterDeclarationClauseAST>();

    ParameterDeclarationListAST::Node params;
    if (!parseParameterDeclarationList(params)) {

        if ((*m_tokenIt) == ')')
            goto good;

        if ((*m_tokenIt) == Token_ellipsis && lex->lookAhead(m_tokenIt, 1) == ')') {
            AST::Node ellipsis = ast_from_token(m_tokenIt);
            ast->setEllipsis(ellipsis);
            ++m_tokenIt;
            goto good;
        }
        return false;
    }

    if ((*m_tokenIt) == Token_ellipsis) {
        AST::Node ellipsis = ast_from_token(m_tokenIt);
        ast->setEllipsis(ellipsis);
        ++m_tokenIt;
    }

good:
    ast->setParameterDeclarationList(params);

    /// @todo add ellipsis
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseParameterDeclarationList(ParameterDeclarationListAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseParameterDeclarationList()";

    TokenIterator start = m_tokenIt;

    ParameterDeclarationListAST::Node ast = CreateNode<ParameterDeclarationListAST>();

    ParameterDeclarationAST::Node param;
    if (!parseParameterDeclaration(param)) {
        m_tokenIt = start;
        return false;
    }
    ast->addParameter(param);

    QString comment;
    while ((*m_tokenIt) == ',') {
        comment.clear();
        advanceAndCheckTrailingComment(comment);

        if ((*m_tokenIt) == Token_ellipsis)
            break;

        if (!parseParameterDeclaration(param)) {
            m_tokenIt = start;
            return false;
        }
        if (!comment.isEmpty())
            param->setComment(comment);
        ast->addParameter(param);
    }

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseParameterDeclaration(ParameterDeclarationAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseParameterDeclaration()";

    TokenIterator start = m_tokenIt;

    // parse decl spec
    TypeSpecifierAST::Node spec;
    if (!parseTypeSpecifier(spec)) {
        m_tokenIt = start;
        return false;
    }

    TokenIterator index = m_tokenIt;

    DeclaratorAST::Node decl;
    if (!parseDeclarator(decl)) {
        m_tokenIt = index;

        // try with abstract declarator
        if (!parseAbstractDeclarator(decl))
            return false;
    }

    AST::Node expr;
    if ((*m_tokenIt) == '=') {
        ++m_tokenIt;
        if (!parseLogicalOrExpression(expr, true)) {
            //reportError( i18n("Expression expected") );
        }
    }

    ParameterDeclarationAST::Node ast = CreateNode<ParameterDeclarationAST>();
    ast->setTypeSpec(spec);
    ast->setDeclarator(decl);
    ast->setExpression(expr);

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseClassSpecifier(TypeSpecifierAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseClassSpecifier()";

    TokenIterator start = m_tokenIt;

    AST::Node classKey;
    TokenIterator classKeyStart = m_tokenIt;

    int kind = (*m_tokenIt);
    if (kind == Token_class || kind == Token_struct || kind == Token_union) {
        AST::Node asn = CreateNode<AST>();
        classKey = asn;
        ++m_tokenIt;
        update_pos(classKey, classKeyStart, m_tokenIt);
    } else {
        return false;
    }

    GroupAST::Node winDeclSpec;
    parseWinDeclSpec(winDeclSpec);

    while ((*m_tokenIt) == Token_identifier && lex->lookAhead(m_tokenIt, 1) == Token_identifier)
        ++m_tokenIt;

    NameAST::Node name;
    parseName(name);

    BaseClauseAST::Node bases;
    if ((*m_tokenIt) == ':') {
        if (!parseBaseClause(bases)) {
            skipUntil('{');
        }
    }

    QString comment;
    while ((*m_tokenIt) == Token_comment) {
        comment += (*m_tokenIt).text();
        ++m_tokenIt;
    }
    if ((*m_tokenIt) != '{') {
        m_tokenIt = start;
        return false;
    }

    if (!advance('{', "{"))
        return false;

    ClassSpecifierAST::Node ast = CreateNode<ClassSpecifierAST>();
    ast->setWinDeclSpec(winDeclSpec);
    ast->setClassKey(classKey);
    ast->setName(name);
    ast->setBaseClause(bases);

    while (!(*m_tokenIt).isNull()) {
        if ((*m_tokenIt) == '}')
            break;

        DeclarationAST::Node memSpec = CreateNode<DeclarationAST>();
        TokenIterator startDecl = m_tokenIt;
        if (!parseMemberSpecification(memSpec)) {
            if (startDecl == m_tokenIt)
                ++m_tokenIt; // skip at least one token
            skipUntilDeclaration();
        } else
            ast->addDeclaration(memSpec);
    }

    if ((*m_tokenIt) != '}') {
        reportError(i18n("} missing"));
    } else
        ++m_tokenIt;

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseAccessSpecifier(AST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseAccessSpecifier()";

    TokenIterator start = m_tokenIt;

    switch ((*m_tokenIt)) {
    case Token_public:
    case Token_protected:
    case Token_private: {
        AST::Node asn = CreateNode<AST>();
        node = asn;
        ++m_tokenIt;
        update_pos(node, start, m_tokenIt);
        return true;
    }
    }

    return false;
}

void Parser::advanceAndCheckTrailingComment(QString& comment)
{
    Token t = *m_tokenIt;
    int previousTokenEndLine = t.getEndPosition().line;
    ++m_tokenIt;
    if ((*m_tokenIt) != Token_comment)
        return;
    t = *m_tokenIt;
    int commentStartLine = t.getStartPosition().line;
    if (commentStartLine != previousTokenEndLine)
        return;
    comment += (*m_tokenIt).text();
    ++m_tokenIt;
}

void Parser::advance_nr(int tk, char const* descr)
{
    const Token& token = (*m_tokenIt);
    if (token != tk) {
        reportError(i18n("'%1' expected found '%2'", QString(descr),
                         token.text()));
    } else
        ++m_tokenIt;
}

AST::Node Parser::ast_from_token(TokenIterator tk) const
{
    AST::Node node = CreateNode<AST>();
    TokenIterator l_tmp = tk;
    ++l_tmp;
    update_pos(node, tk, l_tmp);
    return node;
}

bool Parser::parseMemberSpecification(DeclarationAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseMemberSpecification()";

    QString comment;
    while ((*m_tokenIt) == Token_comment) {
        comment += (*m_tokenIt).text();
        ++m_tokenIt;
    }
    if ((*m_tokenIt).isNull())
        return false;

    TokenIterator start = m_tokenIt;

    AST::Node access;

    if ((*m_tokenIt) == ';') {
        advanceAndCheckTrailingComment(comment);
        if (!comment.isEmpty())
            node->setComment(comment);
        return true;
    } else if ((*m_tokenIt) == Token_Q_OBJECT || (*m_tokenIt) == Token_K_DCOP) {
        ++m_tokenIt;
        return true;
    } else if ((*m_tokenIt) == Token_signals || (*m_tokenIt) == Token_k_dcop || (*m_tokenIt) == Token_k_dcop_signals) {
        AccessDeclarationAST::Node ast = CreateNode<AccessDeclarationAST>();
        ++m_tokenIt;
        AST::Node n = CreateNode<AST>();
        update_pos(n, start, m_tokenIt);
        ast->addAccess(n);
        if (!advance(':', ":"))
            return false;
        update_pos(ast, start, m_tokenIt);
        node = ast;
        return true;
    } else if (parseTypedef(node)) {
        return true;
    } else if (parseUsing(node)) {
        return true;
    } else if (parseTemplateDeclaration(node)) {
        return true;
    } else if (parseAccessSpecifier(access)) {
        AccessDeclarationAST::Node ast = CreateNode<AccessDeclarationAST>();
        ast->addAccess(access);

        TokenIterator startSlot = m_tokenIt;
        if ((*m_tokenIt) == Token_slots) {
            ++m_tokenIt;
            AST::Node sl = CreateNode<AST>();
            update_pos(sl, startSlot, m_tokenIt);
            ast->addAccess(sl);
        }
        if (!advance(':', ":"))
            return false;
        update_pos(ast, start, m_tokenIt);
        node = ast;
        return true;
    }

    m_tokenIt = start;

    GroupAST::Node storageSpec;
    parseStorageClassSpecifier(storageSpec);

    GroupAST::Node cv;
    parseCvQualify(cv);

    TypeSpecifierAST::Node spec;
    if (parseEnumSpecifier(spec) || parseClassSpecifier(spec)) {
        spec->setCvQualify(cv);

        GroupAST::Node cv2;
        parseCvQualify(cv2);
        spec->setCv2Qualify(cv2);

        InitDeclaratorListAST::Node declarators;
        parseInitDeclaratorList(declarators);
        if (!advance(';', ";"))
            return false;

        if (!comment.isEmpty()) {
            //uDebug() << "Parser::parseMemberSpecification(spec): comment is " << comment;
            spec->setComment(comment);
        }

        SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
        ast->setTypeSpec(spec);
        ast->setInitDeclaratorList(declarators);
        update_pos(ast, start, m_tokenIt);
        node = ast;

        return true;
    }

    m_tokenIt = start;

    bool success = parseDeclarationInternal(node, comment);
    if (success && !comment.isEmpty()) {
        node->setComment(comment);
        //uDebug() << "Parser::parseMemberSpecification(): comment is " << comment;
    }
    return success;
}

bool Parser::parseCtorInitializer(AST::Node& /*node*/)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseCtorInitializer()";

    if ((*m_tokenIt) != ':') {
        return false;
    }
    ++m_tokenIt;

    AST::Node inits;
    if (!parseMemInitializerList(inits)) {
        reportError(i18n("Member initializers expected"));
    }

    return true;
}

bool Parser::parseElaboratedTypeSpecifier(TypeSpecifierAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseElaboratedTypeSpecifier()";

    TokenIterator start = m_tokenIt;

    int tk = (*m_tokenIt);
    if (tk == Token_class  ||
            tk == Token_struct ||
            tk == Token_union  ||
            tk == Token_enum   ||
            tk == Token_typename) {
        AST::Node kind = CreateNode<AST>();
        ++m_tokenIt;
        update_pos(kind, start, m_tokenIt);

        NameAST::Node name;

        if (parseName(name)) {
            ElaboratedTypeSpecifierAST::Node ast = CreateNode<ElaboratedTypeSpecifierAST>();
            ast->setKind(kind);
            ast->setName(name);
            update_pos(ast, start, m_tokenIt);
            node = ast;

            return true;
        }
    }

    m_tokenIt = start;
    return false;
}

bool Parser::parseDeclaratorId(NameAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseDeclaratorId()";
    return parseName(node);
}

bool Parser::parseExceptionSpecification(GroupAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseExceptionSpecification()";

    if ((*m_tokenIt) != Token_throw) {
        return false;
    }
    ++m_tokenIt;

    if (!advance('(', "("))
        return false;
    if ((*m_tokenIt) == Token_ellipsis) {
        // extension found in MSVC++ 7.x headers
        TokenIterator start = m_tokenIt;
        GroupAST::Node ast = CreateNode<GroupAST>();
        AST::Node ellipsis = ast_from_token(m_tokenIt);
        ast->addNode(ellipsis);
        ++m_tokenIt;
        update_pos(ast, start, m_tokenIt);
        node = ast;
    } else {
        parseTypeIdList(node);
    }
    if (!advance(')', ")"))
        return false;

    return true;
}

bool Parser::parseEnumerator(EnumeratorAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseEnumerator()";

    QString comment;
    while ((*m_tokenIt) == Token_comment) {
        comment += (*m_tokenIt).text();
        ++m_tokenIt;
    }
    if ((*m_tokenIt).isNull())
        return false;

    TokenIterator start = m_tokenIt;

    if ((*m_tokenIt) != Token_identifier) {
        return false;
    }
    ++m_tokenIt;

    EnumeratorAST::Node ena = CreateNode<EnumeratorAST>();
    node = ena;

    AST::Node id = CreateNode<AST>();
    update_pos(id, start, m_tokenIt);
    node->setId(id);

    if ((*m_tokenIt) == '=') {
        ++m_tokenIt;

        AST::Node expr;
        if (!parseConstantExpression(expr)) {
            reportError(i18n("Constant expression expected"));
        }
        node->setExpr(expr);
    }

    update_pos(node, start, m_tokenIt);

    return true;
}

bool Parser::parseInitDeclarator(InitDeclaratorAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseInitDeclarator()";

    TokenIterator start = m_tokenIt;

    DeclaratorAST::Node decl;
    AST::Node init;
    if (!parseDeclarator(decl)) {
        return false;
    }

    parseInitializer(init);

    InitDeclaratorAST::Node ast = CreateNode<InitDeclaratorAST>();
    ast->setDeclarator(decl);
    ast->setInitializer(init);
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}



bool Parser::parseBaseClause(BaseClauseAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseBaseClause()";

    TokenIterator start = m_tokenIt;
    if ((*m_tokenIt) != ':') {
        return false;
    }
    ++m_tokenIt;

    BaseClauseAST::Node bca = CreateNode<BaseClauseAST>();

    BaseSpecifierAST::Node baseSpec;
    if (parseBaseSpecifier(baseSpec)) {
        bca->addBaseSpecifier(baseSpec);

        QString comment;
        while ((*m_tokenIt) == ',') {
            comment.clear();
            advanceAndCheckTrailingComment(comment);

            if (!parseBaseSpecifier(baseSpec)) {
                reportError(i18n("Base class specifier expected"));
                return false;
            }
            if (!comment.isEmpty())
                baseSpec->setComment(comment);
            bca->addBaseSpecifier(baseSpec);
        }
    } else
        return false;

    update_pos(bca, start, m_tokenIt);
    node = bca;

    return true;
}

bool Parser::parseInitializer(AST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseInitializer()";

    if ((*m_tokenIt) == '=') {
        ++m_tokenIt;

        AST::Node init;
        if (!parseInitializerClause(node)) {
            reportError(i18n("Initializer clause expected"));
            return false;
        }
    } else if ((*m_tokenIt) == '(') {
        ++m_tokenIt;
        AST::Node expr;
        skipCommaExpression(expr);

        if (!advance(')', ")"))
            return false;
    }

    return false;
}

bool Parser::parseMemInitializerList(AST::Node& /*node*/)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseMemInitializerList()";

    AST::Node init;
    if (!parseMemInitializer(init)) {
        return false;
    }

    QString comment;
    while ((*m_tokenIt) == ',') {
        comment.clear();
        advanceAndCheckTrailingComment(comment);

        if (parseMemInitializer(init)) {
        } else {
            break;
        }
    }

    return true;
}

bool Parser::parseMemInitializer(AST::Node& /*node*/)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseMemInitializer()";

    NameAST::Node initId;
    if (!parseMemInitializerId(initId)) {
        reportError(i18n("Identifier expected"));
        return false;
    }
    if (!advance('(', "("))
        return false;
    AST::Node expr;
    skipCommaExpression(expr);
    if (!advance(')', ")"))
        return false;

    return true;
}

bool Parser::parseTypeIdList(GroupAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseTypeIdList()";

    TokenIterator start = m_tokenIt;

    AST::Node typeId;
    if (!parseTypeId(typeId)) {
        return false;
    }

    GroupAST::Node ast = CreateNode<GroupAST>();
    ast->addNode(typeId);

    QString comment;
    while ((*m_tokenIt) == ',') {
        comment.clear();
        advanceAndCheckTrailingComment(comment);
        if (parseTypeId(typeId)) {
            if (!comment.isEmpty())
                typeId->setComment(comment);
            ast->addNode(typeId);
        } else {
            reportError(i18n("Type id expected"));
            break;
        }
    }

    update_pos(ast, start, m_tokenIt);
    node = ast;
    return true;
}

bool Parser::parseBaseSpecifier(BaseSpecifierAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseBaseSpecifier()";

    TokenIterator start = m_tokenIt;
    BaseSpecifierAST::Node ast = CreateNode<BaseSpecifierAST>();

    AST::Node access;
    if ((*m_tokenIt) == Token_virtual) {
        AST::Node virt = ast_from_token(m_tokenIt);
        ast->setIsVirtual(virt);

        ++m_tokenIt;

        parseAccessSpecifier(access);
    } else {
        parseAccessSpecifier(access);

        if ((*m_tokenIt) == Token_virtual) {
            AST::Node virt = ast_from_token(m_tokenIt);
            ast->setIsVirtual(virt);
            ++m_tokenIt;
        }
    }

    NameAST::Node name;
    if (!parseName(name)) {
        reportError(i18n("Class name expected"));
    }

    ast->setAccess(access);
    ast->setName(name);
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}


bool Parser::parseInitializerClause(AST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseInitializerClause()";

    if ((*m_tokenIt) == '{') {
        if (!skip('{', '}')) {
            reportError(i18n("} missing"));
        } else
            ++m_tokenIt;
    } else {
        if (!parseAssignmentExpression(node)) {
            //reportError( i18n("Expression expected") );
        }
    }

    return true;
}

bool Parser::parseMemInitializerId(NameAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseMemInitializerId()";

    return parseName(node);
}

bool Parser::parsePtrToMember(AST::Node& /*node*/)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parsePtrToMember()";

    if ((*m_tokenIt) == Token_scope) {
        ++m_tokenIt;
    }

    while ((*m_tokenIt) == Token_identifier) {
        ++m_tokenIt;

        if ((*m_tokenIt) == Token_scope && lex->lookAhead(m_tokenIt, 1) == '*') {
            ++m_tokenIt; // skip ::
            ++m_tokenIt; // skip *
            return true;
        } else
            break;
    }

    return false;
}

bool Parser::parseUnqualifiedName(ClassOrNamespaceNameAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseUnqualifiedName()";

    TokenIterator start = m_tokenIt;
    bool isDestructor = false;

    ClassOrNamespaceNameAST::Node ast = CreateNode<ClassOrNamespaceNameAST>();

    if ((*m_tokenIt) == Token_identifier) {
        TokenIterator startName = m_tokenIt;
        AST::Node n = CreateNode<AST>();
        ++m_tokenIt;
        update_pos(n, startName, m_tokenIt);
        ast->setName(n);
    } else if ((*m_tokenIt) == '~' && lex->lookAhead(m_tokenIt, 1) == Token_identifier) {
        TokenIterator startName = m_tokenIt;
        AST::Node n = CreateNode<AST>();
        ++m_tokenIt; // skip ~
        ++m_tokenIt; // skip classname
        update_pos(n, startName, m_tokenIt);
        ast->setName(n);
        isDestructor = true;
    } else if ((*m_tokenIt) == Token_operator) {
        AST::Node n;
        if (!parseOperatorFunctionId(n))
            return false;
        ast->setName(n);
    } else {
        return false;
    }

    if (!isDestructor) {

        TokenIterator index = m_tokenIt;

        if ((*m_tokenIt) == '<') {
            ++m_tokenIt;

            // optional template arguments
            TemplateArgumentListAST::Node args;
            parseTemplateArgumentList(args);

            if ((*m_tokenIt) != '>') {
                m_tokenIt = index;
            } else {
                ++m_tokenIt;
                ast->setTemplateArgumentList(args);
            }
        }
    }

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseStringLiteral(AST::Node& /*node*/)
{
    while (!(*m_tokenIt).isNull()) {
        if ((*m_tokenIt) == Token_identifier &&
                (*m_tokenIt).text() == "L" && lex->lookAhead(m_tokenIt, 1) == Token_string_literal) {

            ++m_tokenIt;
            ++m_tokenIt;
        } else if ((*m_tokenIt) == Token_string_literal) {
            ++m_tokenIt;
        } else
            return false;
    }
    return true;
}

bool Parser::skipExpressionStatement(StatementAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::skipExpressionStatement()";

    TokenIterator start = m_tokenIt;

    AST::Node expr;
    skipCommaExpression(expr);

    if (!advance(';', ";"))
        return false;

    ExpressionStatementAST::Node ast = CreateNode<ExpressionStatementAST>();
    ast->setExpression(expr);
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseStatement(StatementAST::Node& node)   // thanks to fiore@8080.it ;)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseStatement()";
    switch ((*m_tokenIt)) {

    case Token_while:
        return parseWhileStatement(node);

    case Token_do:
        return parseDoStatement(node);

    case Token_for:
        return parseForStatement(node);

    case Token_if:
        return parseIfStatement(node);

    case Token_switch:
        return parseSwitchStatement(node);

    case Token_try:
        return parseTryBlockStatement(node);

    case Token_case:
    case Token_default:
        return parseLabeledStatement(node);

    case Token_break:
    case Token_continue:
        ++m_tokenIt;
        if (!advance(';', ";"))
            return false;
        return true;

    case Token_goto:
        ++m_tokenIt;
        if (!advance(Token_identifier, "identifier"))
            return false;
        if (!advance(';', ";"))
            return false;
        return true;

    case Token_return: {
        ++m_tokenIt;
        AST::Node expr;
        skipCommaExpression(expr);
        if (!advance(';', ";"))
            return false;
    }
    return true;

    case '{':
        return parseCompoundStatement(node);

    case Token_identifier:
        if (parseLabeledStatement(node))
            return true;
        break;
    }

    //uDebug() << "------------> try with declaration statement";
    if (parseDeclarationStatement(node))
        return true;

    return skipExpressionStatement(node);
}

bool Parser::parseCondition(ConditionAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseCondition()";

    TokenIterator start = m_tokenIt;

    ConditionAST::Node ast = CreateNode<ConditionAST>();

    TypeSpecifierAST::Node spec;
    if (parseTypeSpecifier(spec)) {
        DeclaratorAST::Node decl;
        if (parseDeclarator(decl) && (*m_tokenIt) == '=') {
            ++m_tokenIt;

            AST::Node expr;
            if (skipExpression(expr)) {
                ast->setTypeSpec(spec);
                ast->setDeclarator(decl);
                ast->setExpression(expr);

                update_pos(ast, start, m_tokenIt);
                node = ast;

                return true;
            }
        }
    }

    m_tokenIt = start;

    AST::Node expr;
    if (!skipCommaExpression(expr))
        return false;

    ast->setExpression(expr);
    update_pos(ast, start, m_tokenIt);
    node = ast;
    return true;
}


bool Parser::parseWhileStatement(StatementAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseWhileStatement()";
    TokenIterator start = m_tokenIt;

    if (!advance(Token_while, "while"))
        return false;
    if (!advance('(' , "("))
        return false;

    ConditionAST::Node cond;
    if (!parseCondition(cond)) {
        reportError(i18n("condition expected"));
        return false;
    }
    if (!advance(')', ")"))
        return false;

    StatementAST::Node body;
    if (!parseStatement(body)) {
        reportError(i18n("statement expected"));
        return false;
    }

    WhileStatementAST::Node ast = CreateNode<WhileStatementAST>();
    ast->setCondition(cond);
    ast->setStatement(body);
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseDoStatement(StatementAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseDoStatement()";
    TokenIterator start = m_tokenIt;

    if (!advance(Token_do, "do"))
        return false;

    StatementAST::Node body;
    if (!parseStatement(body)) {
        reportError(i18n("statement expected"));
        //return false;
    }

    advance_nr(Token_while, "while");
    advance_nr('(' , "(");

    AST::Node expr;
    if (!skipCommaExpression(expr)) {
        reportError(i18n("expression expected"));
        //return false;
    }

    advance_nr(')', ")");
    advance_nr(';', ";");

    DoStatementAST::Node ast = CreateNode<DoStatementAST>();
    ast->setStatement(body);
    //ast->setCondition( condition );
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseForStatement(StatementAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseForStatement()";
    TokenIterator start = m_tokenIt;

    if (!advance(Token_for, "for"))
        return false;
    if (!advance('(', "("))
        return false;

    StatementAST::Node init;
    if (!parseForInitStatement(init)) {
        reportError(i18n("for initialization expected"));
        return false;
    }

    ConditionAST::Node cond;
    parseCondition(cond);
    if (!advance(';', ";"))
        return false;

    AST::Node expr;
    skipCommaExpression(expr);
    if (!advance(')', ")"))
        return false;

    StatementAST::Node body;
    if (!parseStatement(body))
        return false;

    ForStatementAST::Node ast = CreateNode<ForStatementAST>();
    ast->setInitStatement(init);
    ast->setCondition(cond);
    // ast->setExpression( expression );
    ast->setStatement(body);
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseForInitStatement(StatementAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseForInitStatement()";

    if (parseDeclarationStatement(node))
        return true;

    return skipExpressionStatement(node);
}

bool Parser::parseCompoundStatement(StatementAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseCompoundStatement()";
    TokenIterator start = m_tokenIt;

    if ((*m_tokenIt) != '{') {
        return false;
    }
    ++m_tokenIt;

    StatementListAST::Node ast = CreateNode<StatementListAST>();

    while (!(*m_tokenIt).isNull()) {
        if ((*m_tokenIt) == '}')
            break;

        StatementAST::Node stmt;
        TokenIterator startStmt = m_tokenIt;
        if (!parseStatement(stmt)) {
            if (startStmt == m_tokenIt)
                ++m_tokenIt;
            skipUntilStatement();
        } else {
            ast->addStatement(stmt);
        }
    }

    if ((*m_tokenIt) != '}') {
        reportError(i18n("} expected"));
    } else {
        ++m_tokenIt;
    }

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseIfStatement(StatementAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseIfStatement()";

    TokenIterator start = m_tokenIt;

    if (!advance(Token_if, "if"))
        return false;

    if (!advance('(' , "("))
        return false;

    IfStatementAST::Node ast = CreateNode<IfStatementAST>();

    ConditionAST::Node cond;
    if (!parseCondition(cond)) {
        reportError(i18n("condition expected"));
        return false;
    }
    if (!advance(')', ")"))
        return false;

    StatementAST::Node stmt;
    if (!parseStatement(stmt)) {
        reportError(i18n("statement expected"));
        return false;
    }

    ast->setCondition(cond);
    ast->setStatement(stmt);

    if ((*m_tokenIt) == Token_else) {
        ++m_tokenIt;
        StatementAST::Node elseStmt;
        if (!parseStatement(elseStmt)) {
            reportError(i18n("statement expected"));
            return false;
        }
        ast->setElseStatement(elseStmt);
    }

    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseSwitchStatement(StatementAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseSwitchStatement()";
    TokenIterator start = m_tokenIt;
    if (!advance(Token_switch, "switch"))
        return false;

    if (!advance('(' , "("))
        return false;

    ConditionAST::Node cond;
    if (!parseCondition(cond)) {
        reportError(i18n("condition expected"));
        return false;
    }
    if (!advance(')', ")"))
        return false;

    StatementAST::Node stmt;
    if (!parseCompoundStatement(stmt)) {
        syntaxError();
        return false;
    }

    SwitchStatementAST::Node ast = CreateNode<SwitchStatementAST>();
    ast->setCondition(cond);
    ast->setStatement(stmt);
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseLabeledStatement(StatementAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseLabeledStatement()";
    switch ((*m_tokenIt)) {
    case Token_identifier:
    case Token_default:
        if (lex->lookAhead(m_tokenIt, 1) == ':') {
            ++m_tokenIt;
            ++m_tokenIt;

            StatementAST::Node stmt;
            if (parseStatement(stmt)) {
                node = stmt;
                return true;
            }
        }
        break;

    case Token_case: {
        ++m_tokenIt;
        AST::Node expr;
        if (!parseConstantExpression(expr)) {
            reportError(i18n("expression expected"));
        } else if ((*m_tokenIt) == Token_ellipsis) {
            ++m_tokenIt;

            AST::Node expr2;
            if (!parseConstantExpression(expr2)) {
                reportError(i18n("expression expected"));
            }
        }
        if (!advance(':', ":"))
            return false;

        StatementAST::Node stmt;
        if (parseStatement(stmt)) {
            node = stmt;
            return true;
        }
    }
    break;

    }

    return false;
}

bool Parser::parseBlockDeclaration(DeclarationAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseBlockDeclaration()";
    switch ((*m_tokenIt)) {
    case Token_typedef:
        return parseTypedef(node);
    case Token_using:
        return parseUsing(node);
    case Token_asm:
        return parseAsmDefinition(node);
    case Token_namespace:
        return parseNamespaceAliasDefinition(node);
    }

    TokenIterator start = m_tokenIt;

    GroupAST::Node storageSpec;
    parseStorageClassSpecifier(storageSpec);

    GroupAST::Node cv;
    parseCvQualify(cv);

    TypeSpecifierAST::Node spec;
    if (!parseTypeSpecifierOrClassSpec(spec)) {   // replace with simpleTypeSpecifier?!?!
        m_tokenIt = start;
        return false;
    }
    spec->setCvQualify(cv);

    GroupAST::Node cv2;
    parseCvQualify(cv2);
    spec->setCv2Qualify(cv2);

    InitDeclaratorListAST::Node declarators;
    parseInitDeclaratorList(declarators);

    if ((*m_tokenIt) != ';') {
        m_tokenIt = start;
        return false;
    }
    ++m_tokenIt;

    SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
    ast->setTypeSpec(spec);
    ast->setInitDeclaratorList(declarators);
    update_pos(ast, start, m_tokenIt);
    node = ast;

    return true;
}

bool Parser::parseNamespaceAliasDefinition(DeclarationAST::Node& /*node*/)
{
    if ((*m_tokenIt) != Token_namespace) {
        return false;
    }
    ++m_tokenIt;

    if (!advance(Token_identifier,  "identifier"))
        return false;
    if (!advance('=', "="))
        return false;

    NameAST::Node name;
    if (!parseName(name)) {
        reportError(i18n("Namespace name expected"));
    }

    if (!advance(';', ";"))
        return false;

    return true;

}

bool Parser::parseDeclarationStatement(StatementAST::Node& node)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseDeclarationStatement()";

    TokenIterator start = m_tokenIt;

    DeclarationAST::Node decl;
    if (!parseBlockDeclaration(decl)) {
        return false;
    }

    DeclarationStatementAST::Node ast = CreateNode<DeclarationStatementAST>();
    ast->setDeclaration(decl);
    update_pos(ast, start, m_tokenIt);
    node = ast;

    //uDebug() << "---------------------> found a block declaration";
    return true;
}

bool Parser::parseDeclarationInternal(DeclarationAST::Node& node, QString& comment)
{
    //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseDeclarationInternal()";

    TokenIterator start = m_tokenIt;

    // that is for the case '__declspec(dllexport) int ...' or
    // '__declspec(dllexport) inline int ...', etc.
    GroupAST::Node winDeclSpec;
    parseWinDeclSpec(winDeclSpec);

    GroupAST::Node funSpec;
    bool hasFunSpec = parseFunctionSpecifier(funSpec);

    GroupAST::Node storageSpec;
    bool hasStorageSpec = parseStorageClassSpecifier(storageSpec);

    if (hasStorageSpec && !hasFunSpec)
        hasFunSpec = parseFunctionSpecifier(funSpec);

    // that is for the case 'friend __declspec(dllexport) ....'
    GroupAST::Node winDeclSpec2;
    parseWinDeclSpec(winDeclSpec2);

    GroupAST::Node cv;
    parseCvQualify(cv);

    TokenIterator index = m_tokenIt;
    NameAST::Node name;
    if (parseName(name) && (*m_tokenIt) == '(') {
        // no type specifier, maybe a constructor or a cast operator??

        m_tokenIt = index;

        InitDeclaratorAST::Node declarator;
        if (parseInitDeclarator(declarator)) {
            TokenIterator endSignature = m_tokenIt;

            switch ((*m_tokenIt)) {
            case ';': {
                ++m_tokenIt;

                InitDeclaratorListAST::Node declarators = CreateNode<InitDeclaratorListAST>();

                // update declarators position
                if (declarator.get()) {
                    declarators->setStartPosition(declarator->getStartPosition());
                    declarators->setEndPosition(declarator->getEndPosition());
                }
                declarators->addInitDeclarator(declarator);

                SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
                ast->setInitDeclaratorList(declarators);
                ast->setText(toString(start, endSignature));
                node = ast;
                update_pos(node, start, m_tokenIt);
                return true;

            }
            break;

            case ':': {
                    AST::Node ctorInit;
                    StatementListAST::Node funBody;
                    if (parseCtorInitializer(ctorInit) && parseFunctionBody(funBody)) {
                        FunctionDefinitionAST::Node ast = CreateNode<FunctionDefinitionAST>();
                        ast->setStorageSpecifier(storageSpec);
                        ast->setFunctionSpecifier(funSpec);
                        ast->setInitDeclarator(declarator);
                        ast->setFunctionBody(funBody);
                        ast->setText(toString(start, endSignature));
                        node = ast;
                        update_pos(node, start, m_tokenIt);
                        return true;
                    }
                }
                break;

        case '{': {
                StatementListAST::Node funBody;
                if (parseFunctionBody(funBody)) {
                    FunctionDefinitionAST::Node ast = CreateNode<FunctionDefinitionAST>();
                    ast->setStorageSpecifier(storageSpec);
                    ast->setFunctionSpecifier(funSpec);
                    ast->setInitDeclarator(declarator);
                    ast->setText(toString(start, endSignature));
                    ast->setFunctionBody(funBody);
                    node = ast;
                    update_pos(node, start, m_tokenIt);
                    return true;
                }
            }
            break;

        case '(':
        case '[':
            // ops!! it seems a declarator
            goto start_decl;
            break;
        }

    }

    syntaxError();
    return false;
}

start_decl:
m_tokenIt = index;

if ((*m_tokenIt) == Token_const && lex->lookAhead(m_tokenIt, 1) == Token_identifier && lex->lookAhead(m_tokenIt, 2) == '=') {
    // constant definition
    ++m_tokenIt;
    InitDeclaratorListAST::Node declarators;
    if (parseInitDeclaratorList(declarators)) {
        if (!advance(';', ";"))
            return false;
        DeclarationAST::Node ast = CreateNode<DeclarationAST>();
        node = ast;
        update_pos(node, start, m_tokenIt);
        return true;
    }
    syntaxError();
    return false;
}

TypeSpecifierAST::Node spec;
if (parseTypeSpecifier(spec)) {
    if (!hasFunSpec)
        parseFunctionSpecifier(funSpec);           // e.g. "void inline"
    spec->setCvQualify(cv);

    InitDeclaratorListAST::Node declarators;

    InitDeclaratorAST::Node decl;
    TokenIterator startDeclarator = m_tokenIt;
    bool maybeFunctionDefinition = false;

    if ((*m_tokenIt) != ';') {
        if (parseInitDeclarator(decl) && (*m_tokenIt) == '{') {
            // function definition
            maybeFunctionDefinition = true;
        } else {
            m_tokenIt = startDeclarator;
            if (!parseInitDeclaratorList(declarators)) {
                syntaxError();
                return false;
            }
        }
    }

    TokenIterator endSignature = m_tokenIt;
    switch ((*m_tokenIt)) {
    case ';': {
            advanceAndCheckTrailingComment(comment);
            SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
            ast->setStorageSpecifier(storageSpec);
            ast->setFunctionSpecifier(funSpec);
            ast->setText(toString(start, endSignature));
            ast->setTypeSpec(spec);
            ast->setWinDeclSpec(winDeclSpec);
            ast->setInitDeclaratorList(declarators);
            node = ast;
            update_pos(node, start, m_tokenIt);
        }
        return true;

    case '{': {
            if (!maybeFunctionDefinition) {
                syntaxError();
                return false;
            }
            StatementListAST::Node funBody;
            if (parseFunctionBody(funBody)) {
                FunctionDefinitionAST::Node ast = CreateNode<FunctionDefinitionAST>();
                ast->setWinDeclSpec(winDeclSpec);
                ast->setStorageSpecifier(storageSpec);
                ast->setFunctionSpecifier(funSpec);
                ast->setText(toString(start, endSignature));
                ast->setTypeSpec(spec);
                ast->setFunctionBody(funBody);
                ast->setInitDeclarator(decl);
                node = ast;
                update_pos(node, start, m_tokenIt);
                return true;
            }
        }
        break;

    }
}

syntaxError();
return false;
}

bool Parser::parseFunctionBody(StatementListAST::Node& node)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseFunctionBody()";

TokenIterator start = m_tokenIt;
if ((*m_tokenIt) != '{') {
    return false;
}
++m_tokenIt;

StatementListAST::Node ast = CreateNode<StatementListAST>();

while (!(*m_tokenIt).isNull()) {
    if ((*m_tokenIt) == '}')
        break;

    StatementAST::Node stmt;
    TokenIterator startStmt = m_tokenIt;
    if (!parseStatement(stmt)) {
        if (startStmt == m_tokenIt)
            ++m_tokenIt;
        skipUntilStatement();
    } else
        ast->addStatement(stmt);
}

if ((*m_tokenIt) != '}') {
    reportError(i18n("} expected"));
} else
    ++m_tokenIt;

update_pos(ast, start, m_tokenIt);
node = ast;

return true;
}

QString Parser::toString(TokenIterator start, TokenIterator end,
                     const QString& sep) const
{
QStringList l;

for (; start != end; ++start) {
    l << (*start).text();
}

return l.join(sep).trimmed();
}

bool Parser::parseTypeSpecifierOrClassSpec(TypeSpecifierAST::Node& node)
{
if (parseClassSpecifier(node))
    return true;
else if (parseEnumSpecifier(node))
    return true;
else if (parseTypeSpecifier(node))
    return true;

return false;
}

bool Parser::parseTryBlockStatement(StatementAST::Node& node)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseTryBlockStatement()";

if ((*m_tokenIt) != Token_try) {
    return false;
}
++m_tokenIt;

StatementAST::Node stmt;
if (!parseCompoundStatement(stmt)) {
    syntaxError();
    return false;
}

if ((*m_tokenIt) != Token_catch) {
    reportError(i18n("catch expected"));
    return false;
}

while ((*m_tokenIt) == Token_catch) {
    ++m_tokenIt;
    if (!advance('(', "("))
        return false;
    ConditionAST::Node cond;
    if (!parseCondition(cond)) {
        reportError(i18n("condition expected"));
        return false;
    }
    if (!advance(')', ")"))
        return false;

    StatementAST::Node body;
    if (!parseCompoundStatement(body)) {
        syntaxError();
        return false;
    }
}

node = stmt;
return true;
}

bool Parser::parsePrimaryExpression(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parsePrimarExpression()";


switch ((*m_tokenIt)) {
case Token_string_literal: {
        AST::Node lit;
        parseStringLiteral(lit);
    }
    return true;

case Token_number_literal:
case Token_char_literal:
case Token_true:
case Token_false:
    ++m_tokenIt;
    return true;

case Token_this:
    ++m_tokenIt;
    return true;

case Token_dynamic_cast:
case Token_static_cast:
case Token_reinterpret_cast:
case Token_const_cast: {
        ++m_tokenIt;

        if (! check('<'))
            return false;
        AST::Node typeId;
        parseTypeId(typeId);
        if (! check('>'))
            return false;

        if (! check('('))
            return false;
        AST::Node expr;
        parseCommaExpression(expr);
        if (! check(')'))
            return false;
    }
    return true;

case Token_typeid: {
        ++m_tokenIt;
        if (! check('('))
            return false;
        AST::Node expr;
        parseCommaExpression(expr);
        if (! check(')'))
            return false;
    }
    return true;

case '(': {
        ++m_tokenIt;
        //uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "token = " << (*m_tokenIt).text();
        AST::Node expr;
        if (!parseExpression(expr)) {
            return false;
        }
        if (! check(')'))
            return false;
    }
    return true;

default: {
        TokenIterator start = m_tokenIt;
        TypeSpecifierAST::Node typeSpec;
        if (parseSimpleTypeSpecifier(typeSpec) && (*m_tokenIt) == '(') {
            ++m_tokenIt;
            AST::Node expr;
            parseCommaExpression(expr);
            if (! check(')'))
                return false;
            return true;
        }

        m_tokenIt = start;
        NameAST::Node name;
        if (parseName(name))
            return true;
    }
}

return false;
}

bool Parser::parsePostfixExpression(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parsePostfixExpression()";

AST::Node expr;
if (!parsePrimaryExpression(expr))
    return false;

while (true) {
    switch ((*m_tokenIt)) {
    case '[': {
            ++m_tokenIt;
            AST::Node e;
            parseCommaExpression(e);
            if (! check(']'))
                return false;
        }
        break;

    case '(': {
            ++m_tokenIt;
            AST::Node funArgs;
            parseCommaExpression(funArgs);
            if (! check(')'))
                return false;
        }
        break;

    case Token_incr:
    case Token_decr:
        ++m_tokenIt;
        break;

    case '.':
    case Token_arrow: {
            ++m_tokenIt;
            if ((*m_tokenIt) == Token_template)
                ++m_tokenIt;

            NameAST::Node name;
            if (!parseName(name)) {
                return false;
            }
        }
        break;

    case Token_typename: {
            ++m_tokenIt;

            NameAST::Node name;
            if (!parseName(name)) {
                return false;
            }

            if (! check('('))
                return false;
            AST::Node expr;
            parseCommaExpression(expr);
            if (! check(')'))
                return false;
        }
        return true;

    default:
        return true;

    } // end switch

} // end while

return true;
}

bool Parser::parseUnaryExpression(AST::Node& node)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseUnaryExpression()";

switch ((*m_tokenIt)) {
case Token_incr:
case Token_decr:
case '*':
case '&':
case '+':
case '-':
case '!':
case '~': {
        ++m_tokenIt;
        AST::Node expr;
        return parseCastExpression(expr);
    }

case Token_sizeof: {
        ++m_tokenIt;
        TokenIterator index = m_tokenIt;
        if ((*m_tokenIt) == '(') {
            ++m_tokenIt;
            AST::Node typeId;
            if (parseTypeId(typeId) && (*m_tokenIt) == ')') {
                ++m_tokenIt;
                return true;
            }
            m_tokenIt = index;
        }
        AST::Node expr;
        return parseUnaryExpression(expr);
    }

case Token_new:
    return parseNewExpression(node);

case Token_delete:
    return parseDeleteExpression(node);
}

return parsePostfixExpression(node);
}

bool Parser::parseNewExpression(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseNewExpression()";
if ((*m_tokenIt) == Token_scope && lex->lookAhead(m_tokenIt, 1) == Token_new)
    ++m_tokenIt;

if (! check(Token_new))
    return false;

if ((*m_tokenIt) == '(') {
    ++m_tokenIt;
    AST::Node expr;
    parseCommaExpression(expr);
    if (! check(')'))
        return false;
}

if ((*m_tokenIt) == '(') {
    ++m_tokenIt;
    AST::Node typeId;
    parseTypeId(typeId);
    if (! check(')'))
        return false;
} else {
    AST::Node typeId;
    parseNewTypeId(typeId);
}

AST::Node init;
parseNewInitializer(init);
return true;
}

bool Parser::parseNewTypeId(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseNewTypeId()";
TypeSpecifierAST::Node typeSpec;
if (parseTypeSpecifier(typeSpec)) {
    AST::Node declarator;
    parseNewDeclarator(declarator);
    return true;
}

return false;
}

bool Parser::parseNewDeclarator(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseNewDeclarator()";
AST::Node ptrOp;
if (parsePtrOperator(ptrOp)) {
    AST::Node declarator;
    parseNewDeclarator(declarator);
    return true;
}

if ((*m_tokenIt) == '[') {
    while ((*m_tokenIt) == '[') {
        ++m_tokenIt;
        AST::Node expr;
        parseExpression(expr);
        if (!advance(']', "]"))
            return false;
    }
    return true;
}

return false;
}

bool Parser::parseNewInitializer(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseNewInitializer()";
if ((*m_tokenIt) != '(')
    return false;

++m_tokenIt;
AST::Node expr;
parseCommaExpression(expr);
if (! check(')'))
    return false;

return true;
}

bool Parser::parseDeleteExpression(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseDeleteExpression()";
if ((*m_tokenIt) == Token_scope && lex->lookAhead(m_tokenIt, 1) == Token_delete)
    ++m_tokenIt;

if (! check(Token_delete))
    return false;

if ((*m_tokenIt) == '[') {
    ++m_tokenIt;
    if (! check(']'))
        return false;
}

AST::Node expr;
return parseCastExpression(expr);
}

bool Parser::parseCastExpression(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseCastExpression()";

TokenIterator index = m_tokenIt;

if ((*m_tokenIt) == '(') {
    ++m_tokenIt;
    AST::Node typeId;
    if (parseTypeId(typeId)) {
        if ((*m_tokenIt) == ')') {
            ++m_tokenIt;
            AST::Node expr;
            if (parseCastExpression(expr))
                return true;
        }
    }
}

m_tokenIt = index;

AST::Node expr;
return parseUnaryExpression(expr);
}

bool Parser::parsePmExpression(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser:parsePmExpression()";
AST::Node expr;
if (!parseCastExpression(expr))
    return false;

while ((*m_tokenIt) == Token_ptrmem) {
    ++m_tokenIt;

    if (!parseCastExpression(expr))
        return false;
}

return true;
}

bool Parser::parseMultiplicativeExpression(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseMultiplicativeExpression()";
AST::Node expr;
if (!parsePmExpression(expr))
    return false;

while ((*m_tokenIt) == '*' || (*m_tokenIt) == '/' || (*m_tokenIt) == '%') {
    ++m_tokenIt;

    if (!parsePmExpression(expr))
        return false;
}

return true;
}


bool Parser::parseAdditiveExpression(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseAdditiveExpression()";
AST::Node expr;
if (!parseMultiplicativeExpression(expr))
    return false;

while ((*m_tokenIt) == '+' || (*m_tokenIt) == '-') {
    ++m_tokenIt;

    if (!parseMultiplicativeExpression(expr))
        return false;
}

return true;
}

bool Parser::parseShiftExpression(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseShiftExpression()";
AST::Node expr;
if (!parseAdditiveExpression(expr))
    return false;

while ((*m_tokenIt) == Token_shift) {
    ++m_tokenIt;

    if (!parseAdditiveExpression(expr))
        return false;
}

return true;
}

bool Parser::parseRelationalExpression(AST::Node& /*node*/, bool templArgs)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseRelationalExpression()";
AST::Node expr;
if (!parseShiftExpression(expr))
    return false;

while ((*m_tokenIt) == '<' || ((*m_tokenIt) == '>' && !templArgs) ||
        (*m_tokenIt) == Token_leq || (*m_tokenIt) == Token_geq) {
    ++m_tokenIt;

    if (!parseShiftExpression(expr))
        return false;
}

return true;
}

bool Parser::parseEqualityExpression(AST::Node& /*node*/, bool templArgs)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseEqualityExpression()";
AST::Node expr;
if (!parseRelationalExpression(expr, templArgs))
    return false;

while ((*m_tokenIt) == Token_eq || (*m_tokenIt) == Token_not_eq) {
    ++m_tokenIt;

    if (!parseRelationalExpression(expr, templArgs))
        return false;
}

return true;
}

bool Parser::parseAndExpression(AST::Node& /*node*/, bool templArgs)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseAndExpression()";
AST::Node expr;
if (!parseEqualityExpression(expr, templArgs))
    return false;

while ((*m_tokenIt) == '&') {
    ++m_tokenIt;

    if (!parseEqualityExpression(expr, templArgs))
        return false;
}

return true;
}

bool Parser::parseExclusiveOrExpression(AST::Node& /*node*/, bool templArgs)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseExclusiveOrExpression()";
AST::Node expr;
if (!parseAndExpression(expr, templArgs))
    return false;

while ((*m_tokenIt) == '^') {
    ++m_tokenIt;

    if (!parseAndExpression(expr, templArgs))
        return false;
}

return true;
}

bool Parser::parseInclusiveOrExpression(AST::Node& /*node*/, bool templArgs)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseInclusiveOrExpression()";
AST::Node expr;
if (!parseExclusiveOrExpression(expr, templArgs))
    return false;

while ((*m_tokenIt) == '|') {
    ++m_tokenIt;

    if (!parseExclusiveOrExpression(expr, templArgs))
        return false;
}

return true;
}

bool Parser::parseLogicalAndExpression(AST::Node& /*node*/, bool templArgs)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseLogicalAndExpression()";

AST::Node expr;
if (!parseInclusiveOrExpression(expr, templArgs))
    return false;

while ((*m_tokenIt) == Token_and) {
    ++m_tokenIt;

    if (!parseInclusiveOrExpression(expr, templArgs))
        return false;
}

return true;
}

bool Parser::parseLogicalOrExpression(AST::Node& node, bool templArgs)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseLogicalOrExpression()";

TokenIterator start = m_tokenIt;

AST::Node expr;
if (!parseLogicalAndExpression(expr, templArgs))
    return false;

while ((*m_tokenIt) == Token_or) {
    ++m_tokenIt;

    if (!parseLogicalAndExpression(expr, templArgs))
        return false;
}

AST::Node ast = CreateNode<AST>();
update_pos(ast, start, m_tokenIt);
node = ast;
return true;
}

bool Parser::parseConditionalExpression(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseConditionalExpression()";
AST::Node expr;
if (!parseLogicalOrExpression(expr))
    return false;

if ((*m_tokenIt) == '?') {
    ++m_tokenIt;

    if (!parseExpression(expr))
        return false;

    if (! check(':'))
        return false;

    if (!parseAssignmentExpression(expr))
        return false;
}

return true;
}

bool Parser::parseAssignmentExpression(AST::Node& node)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseAssignmentExpression()";
TokenIterator start = m_tokenIt;
AST::Node expr;
if ((*m_tokenIt) == Token_throw && !parseThrowExpression(expr))
    return false;
else if (!parseConditionalExpression(expr))
    return false;

while ((*m_tokenIt) == Token_assign || (*m_tokenIt) == '=') {
    ++m_tokenIt;

    if (!parseConditionalExpression(expr))
        return false;
}

AST::Node ast = CreateNode<AST>();
update_pos(ast, start, m_tokenIt);
node = ast;
return true;
}

bool Parser::parseConstantExpression(AST::Node& node)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseConstantExpression()";
TokenIterator start = m_tokenIt;
if (parseConditionalExpression(node)) {
    AST::Node ast = CreateNode<AST>();
    update_pos(ast, start, m_tokenIt);
    node = ast;
    return true;
}
return false;
}

bool Parser::parseExpression(AST::Node& node)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseExpression()";

TokenIterator start = m_tokenIt;

if (!parseCommaExpression(node))
    return false;

AST::Node ast = CreateNode<AST>();
update_pos(ast, start, m_tokenIt);
node = ast;
return true;
}

bool Parser::parseCommaExpression(AST::Node& node)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseCommaExpression()";
TokenIterator start = m_tokenIt;

AST::Node expr;
if (!parseAssignmentExpression(expr))
    return false;

QString comment;
while ((*m_tokenIt) == ',') {
    comment.clear();
    advanceAndCheckTrailingComment(comment);

    if (!parseAssignmentExpression(expr))
        return false;
    if (!comment.isEmpty())
        expr->setComment(comment);
}

AST::Node ast = CreateNode<AST>();
update_pos(ast, start, m_tokenIt);
node = ast;
return true;
}

bool Parser::parseThrowExpression(AST::Node& /*node*/)
{
//uDebug() << "--- tok = " << (*m_tokenIt).text() << " -- "  << "Parser::parseThrowExpression()";
if ((*m_tokenIt) != Token_throw)
    return false;

if (! check(Token_throw))
    return false;
AST::Node expr;
if (!parseAssignmentExpression(expr))
    return false;

return true;
}

bool Parser::parseIvarDeclList(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseIvarDecls(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseIvarDecl(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseIvars(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseIvarDeclarator(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseMethodDecl(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseUnarySelector(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseKeywordSelector(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseSelector(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseKeywordDecl(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseReceiver(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseObjcMessageExpr(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseMessageArgs(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseKeywordExpr(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseKeywordArgList(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseKeywordArg(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseReservedWord(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseMyParms(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseMyParm(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseOptParmList(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseObjcSelectorExpr(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseSelectorArg(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseKeywordNameList(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseKeywordName(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseObjcEncodeExpr(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseObjcString(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseProtocolRefs(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseIdentifierList(GroupAST::Node & node)
{
TokenIterator start = m_tokenIt;

if ((*m_tokenIt) != Token_identifier)
    return false;

GroupAST::Node ast = CreateNode<GroupAST>();

AST::Node tk = ast_from_token(m_tokenIt);
ast->addNode(tk);
++m_tokenIt;

QString comment;
while ((*m_tokenIt) == ',') {
    comment.clear();
    advanceAndCheckTrailingComment(comment);
    if ((*m_tokenIt) == Token_identifier) {
        AST::Node tk = ast_from_token(m_tokenIt);
        ast->addNode(tk);
        ++m_tokenIt;
    }
    if (!advance(Token_identifier, "identifier"))
        return false;
}

node = ast;
update_pos(node, start, m_tokenIt);
return true;
}

bool Parser::parseIdentifierColon(AST::Node & node)
{
Q_UNUSED(node);

if ((*m_tokenIt) == Token_identifier && lex->lookAhead(m_tokenIt, 1) == ':') {
    ++m_tokenIt;
    ++m_tokenIt;
    return true;
} // ### else if PTYPENAME -> return true ;

return false;
}

bool Parser::parseObjcProtocolExpr(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseObjcOpenBracketExpr(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseObjcCloseBracket(AST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseObjcDef(DeclarationAST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseObjcClassDef(DeclarationAST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseObjcClassDecl(DeclarationAST::Node & node)
{
Q_UNUSED(node);

if (!advance(OBJC_CLASS, "@class"))
    return false;

GroupAST::Node idList;
parseIdentifierList(idList);
if (!advance(';', ";"))
    return false;

return true;
}

bool Parser::parseObjcProtocolDecl(DeclarationAST::Node & node)
{
Q_UNUSED(node);

if (!advance(OBJC_PROTOCOL, "@protocol"))
    return false;

GroupAST::Node idList;
parseIdentifierList(idList);
if (!advance(';', ";"))
    return false;

return true;
}

bool Parser::parseObjcAliasDecl(DeclarationAST::Node & node)
{
Q_UNUSED(node);

if (!advance(OBJC_ALIAS, "@alias"))
    return false;

GroupAST::Node idList;
parseIdentifierList(idList);
if (!advance(';', ";"))
    return false;

return true;
}

bool Parser::parseObjcProtocolDef(DeclarationAST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseObjcMethodDef(DeclarationAST::Node & node)
{
Q_UNUSED(node);
return false;
}

bool Parser::parseWinDeclSpec(GroupAST::Node & node)
{
if ((*m_tokenIt) == Token_identifier && (*m_tokenIt).text() == "__declspec" && lex->lookAhead(m_tokenIt, 1) == '(') {
    TokenIterator start = m_tokenIt;
    ++m_tokenIt;
    ++m_tokenIt; // skip '('

    parseIdentifierList(node);
    if (!advance(')', ")"))
        return false;

    update_pos(node, start, m_tokenIt);
    return true;
}

return false;
}

