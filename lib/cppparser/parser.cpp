/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// c++ support
#include "parser.h"
#include "driver.h"
#include "lexer.h"
#include "errors.h"
#define DBG_SRC  QLatin1String("Parser")
#include "debug_utils.h"
#include "uml.h"

// qt
#include <QString>
#include <QStringList>

#if QT_VERSION >= 0x050000
#else
#include <kdebug.h>
#endif

#include <KLocalizedString>

using namespace std;

DEBUG_REGISTER_DISABLED(Parser)

#define ADVANCE(tk, descr) \
{ \
  const Token& token = m_lexer->lookAhead(0); \
  if(token != tk){ \
      reportError(i18n("'%1' expected found '%2'").arg(QLatin1String(descr)).arg(token.text())); \
      return false; \
  } \
  nextToken(); \
}

#define ADVANCE_NR(tk, descr) \
{ \
  const Token& token = m_lexer->lookAhead(0); \
  if(token != tk){ \
      reportError(i18n("'%1' expected found '%2'").arg(QLatin1String(descr)).arg(token.text())); \
  } \
  else \
      nextToken(); \
}

#define CHECK(tk, descr) \
{ \
  const Token& token = m_lexer->lookAhead(0); \
  if(token != tk){ \
      return false; \
  } \
  nextToken(); \
}

#define MATCH(tk, descr) \
{ \
  const Token& token = m_lexer->lookAhead(0); \
  if(token != tk){ \
      reportError(Errors::SyntaxError); \
      return false; \
  } \
}



#define UPDATE_POS(node, start, end) \
{ \
   int line, col; \
   const Token &a = m_lexer->tokenAt(start); \
   const Token &b = m_lexer->tokenAt(end!=start ? end-1 : end); \
   a.getStartPosition(&line, &col); \
   (node)->setStartPosition(line, col); \
   b.getEndPosition(&line, &col); \
   (node)->setEndPosition(line, col); \
   if((node)->nodeType() == NodeType_Generic) { \
       if ((start) == (end) || (end) == (start)+1) \
           (node)->setSlice(m_lexer->source(), a.position(), a.length()); \
       else \
           (node)->setText(toString((start),(end))); \
   } \
}

#define AST_FROM_TOKEN(node, tk) \
    AST::Node node = CreateNode<AST>(); \
    UPDATE_POS(node, (tk), (tk)+1);


//@todo remove me
enum {
    OBJC_CLASS,
    OBJC_PROTOCOL,
    OBJC_ALIAS
};

struct ParserPrivateData {
    ParserPrivateData()
    {}
};

Parser::Parser(Driver* driver, Lexer* lexer)
  : m_driver(driver),
    m_lexer(lexer),
    m_problems(0)
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
    PARSER_DEBUG_METHOD;
    if (m_problems < m_maxProblems) {
        ++m_problems;
        int line=0, col=0;
        const Token& token = m_lexer->lookAhead(0);
        m_lexer->getTokenPosition(token, &line, &col);

        QString s = m_lexer->lookAhead(0).text();
        s = s.left(30).trimmed();
        if (s.isEmpty())
            s = i18n("<eof>");

        m_driver->addProblem(m_driver->currentFileName(), Problem(err.text.arg(s), line, col));
    }

    return true;
}

bool Parser::reportError(const QString& msg)
{
    PARSER_DEBUG_METHOD;
    if (m_problems < m_maxProblems) {
        ++m_problems;
        int line=0, col=0;
        const Token& token = m_lexer->lookAhead(0);
        m_lexer->getTokenPosition(token, &line, &col);

        m_driver->addProblem(m_driver->currentFileName(), Problem(msg, line, col));
    }

    return true;
}

void Parser::syntaxError()
{
    (void) reportError(Errors::SyntaxError);
}

bool Parser::skipUntil(int token)
{
    PARSER_DEBUG_METHOD;
    while (!m_lexer->lookAhead(0).isNull()) {
        if (m_lexer->lookAhead(0) == token)
            return true;

        nextToken();
    }

    return false;
}

bool Parser::skipUntilDeclaration()
{
    PARSER_DEBUG_METHOD;
    clearComment();

    while (!m_lexer->lookAhead(0).isNull()) {

        switch (m_lexer->lookAhead(0)) {
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
        case Token_noexcept:
        case Token_const_expr:  // cv
        case Token_volatile:    // cv
        case Token_mutable:     // cv

        case Token_public:
        case Token_protected:
        case Token_private:
        case Token_signals:      // Qt
        case Token_slots:        // Qt
            return true;

        default:
            nextToken();
        }
    }

    return false;
}

bool Parser::skipUntilStatement()
{
    PARSER_DEBUG_METHOD;

    while (!m_lexer->lookAhead(0).isNull()) {
        switch (m_lexer->lookAhead(0)) {
        case ';':
        case '{':
        case '}':
        case Token_const:
        case Token_const_expr:
        case Token_volatile:
        case Token_mutable:
        case Token_noexcept:
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
            nextToken();
        }
    }

    return false;
}

bool Parser::skip(int l, int r)
{
    int count = 0;
    while (!m_lexer->lookAhead(0).isNull()) {
        int tk = m_lexer->lookAhead(0);

        if (tk == l)
            ++count;
        else if (tk == r)
            --count;
        else if (l != '{' && (tk == '{' || tk == '}' || tk == ';'))
            return false;

        if (count == 0)
            return true;

        nextToken();
    }

    return false;
}

bool Parser::skipCommaExpression(AST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    AST::Node expr;
    if (!skipExpression(expr))
        return false;

    while (m_lexer->lookAhead(0) == ',') {
        nextToken();

        if (!skipExpression(expr)) {
            reportError(i18n("expression expected"));
            return false;
        }
    }

    AST::Node ast = CreateNode<AST>();
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::skipExpression(AST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    while (!m_lexer->lookAhead(0).isNull()) {
        int tk = m_lexer->lookAhead(0);

        switch (tk) {
        case '(':
            skip('(', ')');
            nextToken();
            break;

        case '[':
            skip('[', ']');
            nextToken();
            break;

#if 0
        case Token_identifier:
            nextToken();
            if (m_lexer->lookAhead(0) == Token_identifier)
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
            UPDATE_POS(ast, start, m_lexer->index());
            node = std::move(ast);
        }
        return true;

        default:
            nextToken();
        }
    }

    return false;
}

bool Parser::parseName(NameAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    GroupAST::Node winDeclSpec;
    parseWinDeclSpec(winDeclSpec);

    int start = m_lexer->index();

    NameAST::Node ast = CreateNode<NameAST>();

    if (m_lexer->lookAhead(0) == Token_scope) {
        ast->setGlobal(true);
        nextToken();
    }

    int idx = m_lexer->index();

    while (true) {
        ClassOrNamespaceNameAST::Node n;
        if (!parseUnqualifiedName(n)) {
            return false;
        }

        if (m_lexer->lookAhead(0) == Token_scope) {
            nextToken();
            ast->addClassOrNamespaceName(n);
            if (m_lexer->lookAhead(0) == Token_template)
                nextToken(); /// skip optional template     #### @todo CHECK
        } else {
            ast->setUnqualifiedName(n);
            break;
        }
    }

    if (idx == m_lexer->index())
        return false;

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseTranslationUnit(TranslationUnitAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    m_problems = 0;
    TranslationUnitAST::Node tun = CreateNode<TranslationUnitAST>();
    node = tun;
    // only setup file comment if present at first line, and first column
    if (m_lexer->lookAhead(0) == Token_comment) {
        processComment();
        if (m_lexer->lookAhead(0).position() == 0) {
            if (comment()) {
                tun->setComment(comment());
                clearComment();
            }
        }
        nextToken();
    }
    else if (m_lexer->lookAhead(0) == '<' &&
            m_lexer->lookAhead(1) == '?' &&
            m_lexer->lookAhead(2) == Token_identifier &&
            m_lexer->lookAhead(2).text() == QLatin1String("php") ) {
        uDebug() << "found php tag";
        nextToken();
        nextToken();
        nextToken();
    }

    while (!m_lexer->lookAhead(0).isNull()) {
        DeclarationAST::Node def;
        int startDecl = m_lexer->index();
        if (!parseDeclaration(def)) {
            // error recovery
            if (startDecl == m_lexer->index())
                nextToken(); // skip at least one token
            skipUntilDeclaration();
        }
        node->addDeclaration(def);
    }

    UPDATE_POS(node, start, m_lexer->index());

    // force (0,0) as start position
    node->setStartPosition(0, 0);

    return m_problems == 0;
}

bool Parser::parseDeclaration(DeclarationAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    switch (m_lexer->lookAhead(0)) {

    case ';':
        nextToken();
        return true;

    case Token_extern:
        return parseLinkageSpecification(node);

    case Token_namespace:
        return parseNamespace(node);

    case Token_using:
        return parseUsing(node);

    case Token_typedef:
        return parseTypedef(node);

    case Token_asm:
        return parseAsmDefinition(node);

    case Token_template:
    case Token_export:
        return parseTemplateDeclaration(node);

    default: {
        // m_lexer->setIndex(start);

        if (objcp && parseObjcDef(node))
            return true;

        m_lexer->setIndex(start);

        GroupAST::Node storageSpec;
        parseStorageClassSpecifier(storageSpec);

        GroupAST::Node cv;
        parseCvQualify(cv);

        TypeSpecifierAST::Node spec;
        AST::Node declarator;
        if (parseEnumSpecifier(spec) || parseClassSpecifier(spec)) {
            int line, c;
            spec->getEndPosition(&line, &c);
            spec->setCvQualify(cv);

            GroupAST::Node cv2;
            parseCvQualify(cv2);
            spec->setCv2Qualify(cv2);

            InitDeclaratorListAST::Node declarators;
            parseInitDeclaratorList(declarators);

            SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();

            ADVANCE(';', ";");

            preparseLineComments(line);

            ast->setComment(m_commentStore.getCommentInRange(line));

            ast->setStorageSpecifier(storageSpec);
            ast->setTypeSpec(spec);
            ast->setInitDeclaratorList(declarators);
            UPDATE_POS(ast, start, m_lexer->index());
            node = std::move(ast);

            return true;
        }

        m_lexer->setIndex(start);
        return parseDeclarationInternal(node);
    }

    } // end switch
}

bool Parser::parseLinkageSpecification(DeclarationAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    if (m_lexer->lookAhead(0) != Token_extern) {
        return false;
    }
    nextToken();

    LinkageSpecificationAST::Node ast = CreateNode<LinkageSpecificationAST>();

    int startExternType = m_lexer->index();
    if (m_lexer->lookAhead(0) == Token_string_literal) {
        nextToken();
        AST::Node externType = CreateNode<AST>();
        UPDATE_POS(externType, startExternType, m_lexer->index());

        ast->setExternType(externType);
    }

    if (m_lexer->lookAhead(0) == '{') {
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

    UPDATE_POS(ast, start, m_lexer->index());

    node = std::move(ast);

    return true;
}

bool Parser::parseLinkageBody(LinkageBodyAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    if (m_lexer->lookAhead(0) != '{') {
        return false;
    }
    nextToken();

    LinkageBodyAST::Node lba = CreateNode<LinkageBodyAST>();
    node = std::move(lba);

    while (!m_lexer->lookAhead(0).isNull()) {
        int tk = m_lexer->lookAhead(0);

        if (tk == '}')
            break;

        DeclarationAST::Node def;
        int startDecl = m_lexer->index();
        if (parseDeclaration(def)) {
            node->addDeclaration(def);
        } else {
            // error recovery
            if (startDecl == m_lexer->index())
                nextToken(); // skip at least one token
            skipUntilDeclaration();
        }
    }

    clearComment();

    if (m_lexer->lookAhead(0) != '}') {
        reportError(i18n("} expected"));
    } else
        nextToken();

    UPDATE_POS(node, start, m_lexer->index());
    return true;
}

bool Parser::parseNamespace(DeclarationAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    if (m_lexer->lookAhead(0) != Token_namespace) {
        return false;
    }
    nextToken();

    int startNamespaceName = m_lexer->index();
    if (m_lexer->lookAhead(0) == Token_identifier) {
        nextToken();
    }
    AST::Node namespaceName = CreateNode<AST>();
    UPDATE_POS(namespaceName, startNamespaceName, m_lexer->index());

    if (m_lexer->lookAhead(0) == '=') {
        // namespace alias
        nextToken();

        NameAST::Node name;
        if (parseName(name)) {
            ADVANCE(';', ";");

            NamespaceAliasAST::Node ast = CreateNode<NamespaceAliasAST>();
            ast->setNamespaceName(namespaceName);
            ast->setAliasName(name);
            UPDATE_POS(ast, start, m_lexer->index());
            node = std::move(ast);
            return true;
        } else {
            reportError(i18n("namespace expected"));
            return false;
        }
    } else if (m_lexer->lookAhead(0) == '\\') {
        // php namespace
        while (m_lexer->lookAhead(0) == '\\' || m_lexer->lookAhead(0) == Token_identifier)
        // namespace alias
            nextToken();
    } else if (m_lexer->lookAhead(0) != '{') {
        reportError(i18n("{ expected"));
        return false;
    }

    NamespaceAST::Node ast = CreateNode<NamespaceAST>();
    ast->setNamespaceName(namespaceName);

    LinkageBodyAST::Node linkageBody;
    parseLinkageBody(linkageBody);

    ast->setLinkageBody(linkageBody);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseUsing(DeclarationAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    if (m_lexer->lookAhead(0) != Token_using) {
        return false;
    }
    nextToken();

    if (m_lexer->lookAhead(0) == Token_namespace) {
        if (!parseUsingDirective(node)) {
            return false;
        }
        UPDATE_POS(node, start, m_lexer->index());
        return true;
    }

    UsingAST::Node ast = CreateNode<UsingAST>();

    int startTypeName = m_lexer->index();
    if (m_lexer->lookAhead(0) == Token_typename) {
        nextToken();
        AST::Node tn = CreateNode<AST>();
        UPDATE_POS(tn, startTypeName, m_lexer->index());
        ast->setTypeName(tn);
    }

    NameAST::Node name;
    if (!parseName(name))
        return false;

    ast->setName(name);

    ADVANCE(';', ";");

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseUsingDirective(DeclarationAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    if (m_lexer->lookAhead(0) != Token_namespace) {
        return false;
    }
    nextToken();

    NameAST::Node name;
    if (!parseName(name)) {
        reportError(i18n("Namespace name expected"));
        return false;
    }

    ADVANCE(';', ";");

    UsingDirectiveAST::Node ast = CreateNode<UsingDirectiveAST>();
    ast->setName(name);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}


bool Parser::parseOperatorFunctionId(AST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    if (m_lexer->lookAhead(0) != Token_operator) {
        return false;
    }
    nextToken();

    AST::Node op;
    if(parseOperator(op)){
        AST::Node asn = CreateNode<AST>();
        node = std::move(asn);
        UPDATE_POS(node, start, m_lexer->index());
        if (node->text() == QLatin1String("operator > >"))
            node->setText(QLatin1String("operator >>"));
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
        node = std::move(asn);
        UPDATE_POS(node, start, m_lexer->index());
        return true;
    }
}

bool Parser::parseTemplateArgumentList(TemplateArgumentListAST::Node& node, bool reportError)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    TemplateArgumentListAST::Node ast = CreateNode<TemplateArgumentListAST>();

    AST::Node templArg;
    if (!parseTemplateArgument(templArg))
        return false;
    ast->addArgument(templArg);

    while (m_lexer->lookAhead(0) == ',') {
        nextToken();

        if (!parseTemplateArgument(templArg)) {
            if (reportError) {
                syntaxError();
                break;
            } else
                return false;
        }
        ast->addArgument(templArg);
    }

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseTypedef(DeclarationAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    if (m_lexer->lookAhead(0) != Token_typedef) {
        return false;
    }
    nextToken();

    TypeSpecifierAST::Node spec;
    if (!parseTypeSpecifierOrClassSpec(spec)) {
        reportError(i18n("Need a type specifier to declare"));
        return false;
    }

    InitDeclaratorListAST::Node declarators;
    if (!parseInitDeclaratorList(declarators)) {
        //reportError(i18n("Need an identifier to declare"));
        //return false;
    }

    TypedefAST::Node ast = CreateNode<TypedefAST>();


    if (comment()) {
        ast->setComment(comment());
        clearComment();

        preparseLineComments(currentLine());

        if (comment()) {
            ast->addComment(comment());
            clearComment();
        }
    }

    ADVANCE(';', ";");

    ast->setTypeSpec(spec);
    ast->setInitDeclaratorList(declarators);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseAsmDefinition(DeclarationAST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;

    ADVANCE(Token_asm, "asm");

    GroupAST::Node cv;
    parseCvQualify(cv);

    skip('(', ')');
    ADVANCE(')', ")");
    ADVANCE(';', ";");

    return true;
}

bool Parser::parseTemplateDeclaration(DeclarationAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    AST::Node exp;

    int startExport = m_lexer->index();
    if (m_lexer->lookAhead(0) == Token_export) {
        nextToken();
        AST::Node n = CreateNode<AST>();
        UPDATE_POS(n, startExport, m_lexer->index());
        exp = std::move(n);
    }

    if (m_lexer->lookAhead(0) != Token_template) {
        return false;
    }
    nextToken();

    TemplateParameterListAST::Node params;
    if (m_lexer->lookAhead(0) == '<') {
        nextToken();
        parseTemplateParameterList(params);

        ADVANCE('>', ">");
    }

    DeclarationAST::Node def;
    if (!parseDeclaration(def)) {
        reportError(i18n("expected a declaration"));
    }

    TemplateDeclarationAST::Node ast = CreateNode<TemplateDeclarationAST>();
    ast->setExported(exp);
    ast->setTemplateParameterList(params);
    ast->setDeclaration(def);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseOperator(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;
    QString text = m_lexer->lookAhead(0).text();

    switch (m_lexer->lookAhead(0)) {
    case Token_new:
    case Token_delete:
        nextToken();
        if (m_lexer->lookAhead(0) == '[' && m_lexer->lookAhead(1) == ']') {
            nextToken();
            nextToken();
            text += QLatin1String("[]");
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
        nextToken();
        if (m_lexer->lookAhead(0) == '>')
            nextToken();
        return true;

    default:
        if (m_lexer->lookAhead(0) == '(' && m_lexer->lookAhead(1) == ')') {
            nextToken();
            nextToken();
            return true;
        } else if (m_lexer->lookAhead(0) == '[' && m_lexer->lookAhead(1) == ']') {
            nextToken();
            nextToken();
            return true;
        }
    }

    return false;
}

bool Parser::parseCvQualify(GroupAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    GroupAST::Node ast = CreateNode<GroupAST>();

    int n = 0;
    while (!m_lexer->lookAhead(0).isNull()) {
        int tk = m_lexer->lookAhead(0);
        if (tk == Token_const || tk == Token_volatile || tk == Token_mutable) {
            ++n;
            int startWord = m_lexer->index();
            nextToken();
            AST::Node word = CreateNode<AST>();
            UPDATE_POS(word, startWord, m_lexer->index());
            ast->addNode(word);
        } else
            break;
    }

    if (n == 0)
        return false;


    PARSER_DEBUG_METHOD;
    UPDATE_POS(ast, start, m_lexer->index());

    node = std::move(ast);
    return true;
}

bool Parser::parseSimpleTypeSpecifier(TypeSpecifierAST::Node& node)
{
    int start = m_lexer->index();
    bool isIntegral = false;
    bool done = false;

    while (!done) {

        switch (m_lexer->lookAhead(0)) {
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
            nextToken();
            break;

        default:
            done = true;
        }
    }

    TypeSpecifierAST::Node ast = CreateNode<TypeSpecifierAST>();
    if (isIntegral) {
        ClassOrNamespaceNameAST::Node cl = CreateNode<ClassOrNamespaceNameAST>();

        AST::Node n = CreateNode<AST>();
        UPDATE_POS(n, start, m_lexer->index());
        cl->setName(n);
        UPDATE_POS(cl, start, m_lexer->index());

        NameAST::Node name = CreateNode<NameAST>();
        name->setUnqualifiedName(cl);
        UPDATE_POS(name, start, m_lexer->index());
        ast->setName(name);

    } else {
        NameAST::Node name;
        if (!parseName(name)) {
            m_lexer->setIndex(start);
            return false;
        }
        ast->setName(name);
    }

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);
    return true;
}

bool Parser::parsePtrOperator(AST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    if (m_lexer->lookAhead(0) == '&') {
        nextToken();
    } else if (m_lexer->lookAhead(0) == '*') {
        nextToken();
    } else {
        int index = m_lexer->index();
        AST::Node memPtr;
        if (!parsePtrToMember(memPtr)) {
            m_lexer->setIndex(index);
            return false;
        }
    }

    GroupAST::Node cv;
    parseCvQualify(cv);

    AST::Node ast = CreateNode<AST>();
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}


bool Parser::parseTemplateArgument(AST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();
    if (parseTypeId(node)) {
        if (m_lexer->lookAhead(0) == ',' || m_lexer->lookAhead(0) == '>')
            return true;
    }

    m_lexer->setIndex(start);
    if (!parseLogicalOrExpression(node, true)) {
        return false;
    }

    return true;
}

bool Parser::parseTypeSpecifier(TypeSpecifierAST::Node& spec)
{
    PARSER_DEBUG_METHOD;

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
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    DeclaratorAST::Node ast = CreateNode<DeclaratorAST>();

    DeclaratorAST::Node decl;
    NameAST::Node declId;

    AST::Node ptrOp;
    while (parsePtrOperator(ptrOp)) {
        ast->addPtrOp(ptrOp);
    }

    if (m_lexer->lookAhead(0) == '(') {
        nextToken();

        if (!parseDeclarator(decl)) {
            return false;
        }
        ast->setSubDeclarator(decl);

        if (m_lexer->lookAhead(0) != ')') {
            return false;
        }
        nextToken();
    } else {

        if (m_lexer->lookAhead(0) == ':') {
            // unnamed bitfield
        } else if (parseDeclaratorId(declId)) {
            ast->setDeclaratorId(declId);
        } else {
            m_lexer->setIndex(start);
            return false;
        }

        if (m_lexer->lookAhead(0) == ':') {
            nextToken();
            AST::Node expr;
            if (!parseConstantExpression(expr)) {
                reportError(i18n("Constant expression expected"));
            }
            goto update_pos;
        }
    }

    {
        bool isVector = true;

        while (m_lexer->lookAhead(0) == '[') {
            int startArray = m_lexer->index();
            nextToken();
            AST::Node expr;
            parseCommaExpression(expr);

            ADVANCE(']', "]");
            AST::Node array = CreateNode<AST>();
            UPDATE_POS(array, startArray, m_lexer->index());
            ast->addArrayDimension(array);
            isVector = true;
        }

        bool skipParen = false;
        if (m_lexer->lookAhead(0) == Token_identifier && m_lexer->lookAhead(1) == '(' && m_lexer->lookAhead(2) == '(') {
            nextToken();
            nextToken();
            skipParen = true;
        }

        if (ast->subDeclarator() && (!isVector || m_lexer->lookAhead(0) != '(')) {
            m_lexer->setIndex(start);
            return false;
        }

        int index = m_lexer->index();
        if (m_lexer->lookAhead(0) == '(') {
            nextToken();

            ParameterDeclarationClauseAST::Node params;
            if (!parseParameterDeclarationClause(params)) {
                PARSER_DEBUG_METHOD;
                m_lexer->setIndex(index);
                goto update_pos;
            }
            ast->setParameterDeclarationClause(params);

            if (m_lexer->lookAhead(0) != ')') {
                m_lexer->setIndex(index);
                goto update_pos;
            }

            nextToken();  // skip ')'

            int tk;
            while ((tk = m_lexer->lookAhead(0)) == Token_const
                   || tk == Token_override || tk == Token_final) {
                int start = m_lexer->index();
                nextToken();
                AST::Node n = CreateNode<AST>();
                UPDATE_POS(n, start, m_lexer->index());
                if (tk == Token_const)
                    ast->setConstant(n);
                else if (tk == Token_override)
                    ast->setOverride(n);
                else
                    ast->setFinal(n);
            }

            GroupAST::Node except;
            if (parseExceptionSpecification(except)) {
                ast->setExceptionSpecification(except);
            }
        }

        if (skipParen) {
            if (m_lexer->lookAhead(0) != ')') {
                reportError(i18n("')' expected"));
            } else
                nextToken();
        }

    }

update_pos:
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseAbstractDeclarator(DeclaratorAST::Node& node)
{
    PARSER_DEBUG_METHOD;
    int start = m_lexer->index();

    DeclaratorAST::Node ast = CreateNode<DeclaratorAST>();

    DeclaratorAST::Node decl;
    NameAST::Node declId;

    AST::Node ptrOp;
    while (parsePtrOperator(ptrOp)) {
        ast->addPtrOp(ptrOp);
    }

    if (m_lexer->lookAhead(0) == '(') {
        nextToken();

        if (!parseAbstractDeclarator(decl)) {
            return false;
        }
        ast->setSubDeclarator(decl);

        if (m_lexer->lookAhead(0) != ')') {
            return false;
        }
        nextToken();
    }

    {

        while (m_lexer->lookAhead(0) == '[') {
            int startArray = m_lexer->index();
            nextToken();
            AST::Node expr;
            skipCommaExpression(expr);

            ADVANCE(']', "]");
            AST::Node array = CreateNode<AST>();
            UPDATE_POS(array, startArray, m_lexer->index());
            ast->addArrayDimension(array);
        }

        bool skipParen = false;
        if (m_lexer->lookAhead(0) == Token_identifier && m_lexer->lookAhead(1) == '(' && m_lexer->lookAhead(2) == '(') {
            nextToken();
            nextToken();
            skipParen = true;
        }

        int index = m_lexer->index();
        if (m_lexer->lookAhead(0) == '(') {
            nextToken();

            ParameterDeclarationClauseAST::Node params;
            if (!parseParameterDeclarationClause(params)) {
                m_lexer->setIndex(index);
                goto UPDATE_POS;
            }
            ast->setParameterDeclarationClause(params);

            if (m_lexer->lookAhead(0) != ')') {
                m_lexer->setIndex(index);
                goto UPDATE_POS;
            } else
                nextToken();

            int tk;
            while ((tk = m_lexer->lookAhead(0)) == Token_const
                   || tk == Token_override || tk == Token_final) {
                int start = m_lexer->index();
                nextToken();
                AST::Node n = CreateNode<AST>();
                UPDATE_POS(n, start, m_lexer->index());
                if (tk == Token_const)
                    ast->setConstant(n);
                else if (tk == Token_override)
                    ast->setOverride(n);
                else
                    ast->setFinal(n);
            }

            GroupAST::Node except;
            if (parseExceptionSpecification(except)) {
                ast->setExceptionSpecification(except);
            }
        }

        if (skipParen) {
            if (m_lexer->lookAhead(0) != ')') {
                reportError(i18n("')' expected"));
            } else
                nextToken();
        }

    }

UPDATE_POS:
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}


bool Parser::parseEnumSpecifier(TypeSpecifierAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    bool isClass = false;
    int start = m_lexer->index();

    if (m_lexer->lookAhead(0) != Token_enum) {
        return false;
    }

    nextToken();

    int tk = m_lexer->lookAhead(0);
    if (tk == Token_class || tk == Token_struct) {
        isClass = true;
        nextToken();
    }

    Comment c = comment();
    clearComment();
    NameAST::Node name;
    parseName(name);

    TypeSpecifierAST::Node enumBase;
    if (m_lexer->lookAhead(0) == ':') {
        nextToken();
        if (!parseSimpleTypeSpecifier(enumBase)) {
            syntaxError();
            return false;
        }
    }

    tk = m_lexer->lookAhead(0);
    if (tk != ';' && tk != '{') {
        m_lexer->setIndex(start);
        return false;
    }

    EnumSpecifierAST::Node ast = CreateNode<EnumSpecifierAST>();
    ast->setClass(isClass);
    ast->setName(name);
    ast->setComment(c);
    ast->setEnumBase(enumBase);

    if (tk == '{') {
        nextToken();

        EnumeratorAST::Node enumerator;
        if (parseEnumerator(enumerator)) {
            ast->addEnumerator(enumerator);

            while (m_lexer->lookAhead(0) == ',') {
                nextToken();

                if (!parseEnumerator(enumerator)) {
                    //reportError(i18n("Enumerator expected"));
                    break;
                }

                ast->addEnumerator(enumerator);
            }
        }

        clearComment();

        if (m_lexer->lookAhead(0) != '}')
            reportError(i18n("} missing"));
        else
            nextToken();
    }

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseTemplateParameterList(TemplateParameterListAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    TemplateParameterListAST::Node ast = CreateNode<TemplateParameterListAST>();

    TemplateParameterAST::Node param;
    if (!parseTemplateParameter(param)) {
        return false;
    }
    ast->addTemplateParameter(param);

    while (m_lexer->lookAhead(0) == ',') {
        nextToken();

        if (!parseTemplateParameter(param)) {
            syntaxError();
            break;
        } else {
            ast->addTemplateParameter(param);
        }
    }

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseTemplateParameter(TemplateParameterAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();
    TemplateParameterAST::Node ast = CreateNode<TemplateParameterAST>();

    TypeParameterAST::Node typeParameter;
    ParameterDeclarationAST::Node param;

    int tk = m_lexer->lookAhead(0);

    if ((tk == Token_class || tk == Token_typename || tk == Token_template) && parseTypeParameter(typeParameter)) {
        ast->setTypeParameter(typeParameter);
        goto ok;
    }

    if (!parseParameterDeclaration(param))
        return false;
    ast->setTypeValueParameter(param);

ok:
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseTypeParameter(TypeParameterAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();
    TypeParameterAST::Node ast = CreateNode<TypeParameterAST>();

    AST_FROM_TOKEN(kind, m_lexer->index());
    ast->setKind(kind);

    switch (m_lexer->lookAhead(0)) {

    case Token_class:
    case Token_typename: {
        nextToken(); // skip class

        // parse optional name
        NameAST::Node name;
        if (parseName(name)) {
            ast->setName(name);
            if (m_lexer->lookAhead(0) == '=') {
                nextToken();

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
        nextToken(); // skip template
        ADVANCE('<', "<");

        TemplateParameterListAST::Node params;
        if (!parseTemplateParameterList(params)) {
            return false;
        }
        ast->setTemplateParameterList(params);

        ADVANCE('>', ">");

        if (m_lexer->lookAhead(0) == Token_class)
            nextToken();

        // parse optional name
        NameAST::Node name;
        if (parseName(name)) {
            ast->setName(name);
            if (m_lexer->lookAhead(0) == '=') {
                nextToken();

                AST::Node typeId;
                if (!parseTypeId(typeId)) {
                    syntaxError();
                    return false;
                }
                ast->setTypeId(typeId);
            }
        }

        if (m_lexer->lookAhead(0) == '=') {
            nextToken();

            NameAST::Node templ_name;
            parseName(templ_name);
        }
    }
    break;

    default:
        return false;

    } // end switch


    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);
    return true;
}

bool Parser::parseStorageClassSpecifier(GroupAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();
    GroupAST::Node ast = CreateNode<GroupAST>();

    while (!m_lexer->lookAhead(0).isNull()) {
        int tk = m_lexer->lookAhead(0);
        if (tk == Token_friend || tk == Token_auto || tk == Token_register || tk == Token_static ||
            tk == Token_extern || tk == Token_mutable || tk == Token_const_expr ) {
            int startNode = m_lexer->index();
            nextToken();

            AST::Node n = CreateNode<AST>();
            UPDATE_POS(n, startNode, m_lexer->index());
            ast->addNode(n);
        } else
            break;
    }

    if (ast->nodeList().count() == 0)
        return false;

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);
    return true;
}

bool Parser::parseFunctionSpecifier(GroupAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();
    GroupAST::Node ast = CreateNode<GroupAST>();

    while (!m_lexer->lookAhead(0).isNull()) {
        int tk = m_lexer->lookAhead(0);
        if (tk == Token_inline || tk == Token_virtual || tk == Token_explicit) {
            int startNode = m_lexer->index();
            nextToken();

            AST::Node n = CreateNode<AST>();
            UPDATE_POS(n, startNode, m_lexer->index());
            ast->addNode(n);
        } else {
            break;
        }
    }

    if (ast->nodeList().count() == 0)
        return false;

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);
    return true;
}

bool Parser::parseTypeId(AST::Node& node)
{
    PARSER_DEBUG_METHOD;

    /// @todo implement the AST for typeId
    int start = m_lexer->index();
    AST::Node ast = CreateNode<AST>();

    TypeSpecifierAST::Node spec;
    if (!parseTypeSpecifier(spec)) {
        return false;
    }

    DeclaratorAST::Node decl;
    parseAbstractDeclarator(decl);

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseInitDeclaratorList(InitDeclaratorListAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    InitDeclaratorListAST::Node ast = CreateNode<InitDeclaratorListAST>();
    InitDeclaratorAST::Node decl;

    if (!parseInitDeclarator(decl)) {
        return false;
    }
    ast->addInitDeclarator(decl);

    while (m_lexer->lookAhead(0) == ',') {
        nextToken();

        if (!parseInitDeclarator(decl)) {
            syntaxError();
            break;
        }
        ast->addInitDeclarator(decl);
    }
    PARSER_DEBUG_METHOD;

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseParameterDeclarationClause(ParameterDeclarationClauseAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    ParameterDeclarationClauseAST::Node ast = CreateNode<ParameterDeclarationClauseAST>();

    ParameterDeclarationListAST::Node params;
    if (!parseParameterDeclarationList(params)) {

        if (m_lexer->lookAhead(0) == ')')
            goto good;

        if (m_lexer->lookAhead(0) == Token_ellipsis && m_lexer->lookAhead(1) == ')') {
            AST_FROM_TOKEN(ellipsis, m_lexer->index());
            ast->setEllipsis(ellipsis);
            nextToken();
            goto good;
        }
        return false;
    }

    if (m_lexer->lookAhead(0) == Token_ellipsis) {
        AST_FROM_TOKEN(ellipsis, m_lexer->index());
        ast->setEllipsis(ellipsis);
        nextToken();
    }

good:
    ast->setParameterDeclarationList(params);

    /// @todo add ellipsis
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}


void Parser::nextToken(bool skipComm)
{
    m_lexer->nextToken();
    if (skipComm) {
        if (m_lexer->lookAhead(0) == Token_comment) {
            processComment();
            nextToken();
        }
    }
}


Comment Parser::comment()
{
    return m_commentStore.latestComment();
}


void Parser::preparseLineComments(int l)
{
    for (int a = 0; a < 40; a++) {
        if (m_lexer->lookAhead(a).isNull()) break;
        int line, col;
        m_lexer->lookAhead(a).getStartPosition(&line, &col);
        if (line < l) {
            continue;
        } else if (line == l) {
            if (m_lexer->lookAhead(a) == Token_comment) {
                processComment(a);
            }
        } else {
            break;
        }
    }
}


void Parser::processComment(int offset)
{
    int line, col;
    m_lexer->lookAhead(offset).getStartPosition(&line, &col);
    m_commentStore.addComment(Comment(m_lexer->lookAhead(offset).text(), line));
}

template<class Type>
void Parser::eventuallyTakeComment(int startLn, int endLn, Type& ast)
{
    if (comment().line() >= startLn && comment().line() <= endLn) {
        if (&(*ast)) {
            if (comment()) {
                ast->setComment(comment());
            }
        }

        clearComment();
    }
}

template<class Type>
void Parser::eventuallyTakeComment(Type& ast)
{
    int line = currentLine();
    Comment c = m_commentStore.getCommentsInRange(line, true);

    if (&(*ast) && c) {
        ast->setComment(c);
    }
}

void Parser::clearComment()
{

    m_commentStore.clear();

}

int Parser::currentLine()
{
    int ln, col;
    m_lexer->lookAhead(0).getStartPosition(&ln, &col);
    return ln;
}

bool Parser::parseParameterDeclarationList(ParameterDeclarationListAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    ParameterDeclarationListAST::Node ast = CreateNode<ParameterDeclarationListAST>();

    ParameterDeclarationAST::Node param;
    if (!parseParameterDeclaration(param)) {
        m_lexer->setIndex(start);
        return false;
    }
    ast->addParameter(param);

    while (m_lexer->lookAhead(0) == ',') {
        nextToken();

        if (m_lexer->lookAhead(0) == Token_ellipsis)
            break;

        if (!parseParameterDeclaration(param)) {
            m_lexer->setIndex(start);
            return false;
        }
        ast->addParameter(param);
    }

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseParameterDeclaration(ParameterDeclarationAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    // parse decl spec
    TypeSpecifierAST::Node spec;
    if (!parseTypeSpecifier(spec)) {
        m_lexer->setIndex(start);
        return false;
    }

    int index = m_lexer->index();

    DeclaratorAST::Node decl;
    if (!parseDeclarator(decl)) {
        m_lexer->setIndex(index);

        // try with abstract declarator
        if (!parseAbstractDeclarator(decl))
            return false;
    }

    AST::Node expr;
    if (m_lexer->lookAhead(0) == '=') {
        nextToken();
        if (!parseLogicalOrExpression(expr,true)) {
            //reportError(i18n("Expression expected"));
        }
    }

    ParameterDeclarationAST::Node ast = CreateNode<ParameterDeclarationAST>();
    ast->setTypeSpec(spec);
    ast->setDeclarator(decl);
    ast->setExpression(expr);

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}


bool Parser::parseClassSpecifier(TypeSpecifierAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    AST::Node classKey;
    int classKeyStart = m_lexer->index();

    int kind = m_lexer->lookAhead(0);
    if (kind == Token_class || kind == Token_struct || kind == Token_union) {
        AST::Node asn = CreateNode<AST>();
        classKey = std::move(asn);
        nextToken();
        UPDATE_POS(classKey, classKeyStart, m_lexer->index());
    } else {
        return false;
    }

    GroupAST::Node winDeclSpec;
    parseWinDeclSpec(winDeclSpec);

    while (m_lexer->lookAhead(0) == Token_identifier && m_lexer->lookAhead(1) == Token_identifier)
        nextToken();

    NameAST::Node name;
    parseName(name);

    AST::Node final_;
    int finalStart = m_lexer->index();
    if (m_lexer->lookAhead(0) == Token_final) {
        final_ = std::move(CreateNode<AST>());
        nextToken();
        UPDATE_POS(final_, finalStart, m_lexer->index());
    }

    BaseClauseAST::Node bases;
    if (m_lexer->lookAhead(0) == ':') {
        if (!parseBaseClause(bases)) {
            skipUntil('{');
        }
    }

    if (m_lexer->lookAhead(0) != '{') {
        m_lexer->setIndex(start);
        return false;
    }

    ClassSpecifierAST::Node ast = CreateNode<ClassSpecifierAST>();

    eventuallyTakeComment(ast);

    ADVANCE('{', "{");

    ast->setWinDeclSpec(winDeclSpec);
    ast->setClassKey(classKey);
    ast->setName(name);
    ast->setBaseClause(bases);
    ast->setFinal(final_);

    while (!m_lexer->lookAhead(0).isNull()) {
        if (m_lexer->lookAhead(0) == '}')
            break;

        DeclarationAST::Node memSpec;
        int startDecl = m_lexer->index();
        if (!parseMemberSpecification(memSpec)) {
            if (startDecl == m_lexer->index())
                nextToken(); // skip at least one token
            skipUntilDeclaration();
        } else
            ast->addDeclaration(memSpec);
    }

    clearComment();

    if (m_lexer->lookAhead(0) != '}') {
        reportError(i18n("} missing"));
    } else
        nextToken();

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseAccessSpecifier(AST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    switch (m_lexer->lookAhead(0)) {
    case Token_public:
    case Token_protected:
    case Token_private: {
        AST::Node asn = CreateNode<AST>();
        node = std::move(asn);
        nextToken();
        UPDATE_POS(node, start, m_lexer->index());
        return true;
    }
    }

    return false;
}

bool Parser::parseMemberSpecification(DeclarationAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    AST::Node access;

    if (m_lexer->lookAhead(0) == ';') {
        nextToken();
        return true;
    } else if (m_lexer->lookAhead(0) == Token_Q_OBJECT || m_lexer->lookAhead(0) == Token_K_DCOP) {
        nextToken();
        return true;
    } else if (m_lexer->lookAhead(0) == Token_signals || m_lexer->lookAhead(0) == Token_k_dcop || m_lexer->lookAhead(0) == Token_k_dcop_signals) {
        AccessDeclarationAST::Node ast = CreateNode<AccessDeclarationAST>();
        nextToken();
        AST::Node n = CreateNode<AST>();
        UPDATE_POS(n, start, m_lexer->index());
        ast->addAccess(n);
        ADVANCE(':', ":");
        UPDATE_POS(ast, start, m_lexer->index());
        node = std::move(ast);
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

        int startSlot = m_lexer->index();
        if (m_lexer->lookAhead(0) == Token_slots) {
            nextToken();
            AST::Node sl = CreateNode<AST>();
            UPDATE_POS(sl, startSlot, m_lexer->index());
            ast->addAccess(sl);
        }
        ADVANCE(':', ":");
        UPDATE_POS(ast, start, m_lexer->index());
        node = std::move(ast);
        return true;
    }

    m_lexer->setIndex(start);

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
        ADVANCE(';', ";");

        SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
        ast->setTypeSpec(spec);
        ast->setInitDeclaratorList(declarators);
        UPDATE_POS(ast, start, m_lexer->index());
        node = std::move(ast);

        return true;
    }

    m_lexer->setIndex(start);
    return parseDeclarationInternal(node);
}

bool Parser::parseCtorInitializer(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;

    if (m_lexer->lookAhead(0) != ':') {
        return false;
    }
    nextToken();

    AST::Node inits;
    if (!parseMemInitializerList(inits)) {
        reportError(i18n("Member initializers expected"));
    }

    return true;
}

bool Parser::parseElaboratedTypeSpecifier(TypeSpecifierAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    int tk = m_lexer->lookAhead(0);
    if (tk == Token_class  ||
        tk == Token_struct ||
        tk == Token_union  ||
        tk == Token_enum   ||
        tk == Token_typename) {
        AST::Node kind = CreateNode<AST>();
        nextToken();
        UPDATE_POS(kind, start, m_lexer->index());

        NameAST::Node name;

        if (parseName(name)) {
            ElaboratedTypeSpecifierAST::Node ast = CreateNode<ElaboratedTypeSpecifierAST>();
            ast->setKind(kind);
            ast->setName(name);
            UPDATE_POS(ast, start, m_lexer->index());
            node = std::move(ast);

            return true;
        }
    }

    m_lexer->setIndex(start);
    return false;
}

bool Parser::parseDeclaratorId(NameAST::Node& node)
{
    PARSER_DEBUG_METHOD;
    return parseName(node);
}

bool Parser::parseExceptionSpecification(GroupAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    if (m_lexer->lookAhead(0) == Token_noexcept) {
        GroupAST::Node ast = CreateNode<GroupAST>();
        int start = m_lexer->index();
        nextToken();
        AST::Node word = CreateNode<AST>();
        UPDATE_POS(word, start, m_lexer->index());
        ast->addNode(word);
        node = std::move(ast);
        return true;
    }

    if (m_lexer->lookAhead(0) != Token_throw) {
        return false;
    }
    nextToken();

    ADVANCE('(', "(");
    if (m_lexer->lookAhead(0) == Token_ellipsis) {
        // extension found in MSVC++ 7.x headers
        int start = m_lexer->index();
        GroupAST::Node ast = CreateNode<GroupAST>();
        AST_FROM_TOKEN(ellipsis, m_lexer->index());
        ast->addNode(ellipsis);
        nextToken();
        UPDATE_POS(ast, start, m_lexer->index());
        node = std::move(ast);
    } else if (m_lexer->lookAhead(0) == ')') {
        node = CreateNode<GroupAST>();
    } else {
        parseTypeIdList(node);
    }
    ADVANCE(')', ")");

    return true;
}

bool Parser::parseEnumerator(EnumeratorAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    if (m_lexer->lookAhead(0) != Token_identifier) {
        return false;
    }

    nextToken();


    EnumeratorAST::Node ena = CreateNode<EnumeratorAST>();
    node = std::move(ena);

    AST::Node id = CreateNode<AST>();
    UPDATE_POS(id, start, m_lexer->index());
    node->setId(id);
    int line = currentLine();

    if (m_lexer->lookAhead(0) == '=') {
        nextToken();

        AST::Node expr;
        line = currentLine();
        if (!parseConstantExpression(expr)) {
            reportError(i18n("Constant expression expected"));
        }
        node->setExpr(expr);
    }

    UPDATE_POS(node, start, m_lexer->index());

    preparseLineComments(line);

    node->setComment(m_commentStore.getCommentInRange(line));

    return true;
}

bool Parser::parseInitDeclarator(InitDeclaratorAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    DeclaratorAST::Node decl;
    AST::Node init;
    if (!parseDeclarator(decl)) {
        return false;
    }

    parseInitializer(init);

    InitDeclaratorAST::Node ast = CreateNode<InitDeclaratorAST>();
    ast->setDeclarator(decl);
    ast->setInitializer(init);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}



bool Parser::parseBaseClause(BaseClauseAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();
    if (m_lexer->lookAhead(0) != ':') {
        return false;
    }
    nextToken();

    BaseClauseAST::Node bca = CreateNode<BaseClauseAST>();

    BaseSpecifierAST::Node baseSpec;
    if (parseBaseSpecifier(baseSpec)) {
        bca->addBaseSpecifier(baseSpec);

        while (m_lexer->lookAhead(0) == ',') {
            nextToken();

            if (!parseBaseSpecifier(baseSpec)) {
                reportError(i18n("Base class specifier expected"));
                return false;
            }
            bca->addBaseSpecifier(baseSpec);
        }
    } else
        return false;

    UPDATE_POS(bca, start, m_lexer->index());
    node = std::move(bca);

    return true;
}

bool Parser::parseInitializer(AST::Node& node)
{
    PARSER_DEBUG_METHOD;

    if (m_lexer->lookAhead(0) == '=') {
        nextToken();

        AST::Node init;
        if (!parseInitializerClause(node)) {
            reportError(i18n("Initializer clause expected"));
            return false;
        }
    } else if (m_lexer->lookAhead(0) == '(') {
        nextToken();
        AST::Node expr;
        skipCommaExpression(expr);

        ADVANCE(')', ")");
    }

    return false;
}

bool Parser::parseMemInitializerList(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;

    AST::Node init;
    if (!parseMemInitializer(init)) {
        return false;
    }

    while (m_lexer->lookAhead(0) == ',') {
        nextToken();

        if (parseMemInitializer(init)) {
        } else {
            break;
        }
    }

    return true;
}

bool Parser::parseMemInitializer(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;

    NameAST::Node initId;
    if (!parseMemInitializerId(initId)) {
        reportError(i18n("Identifier expected"));
        return false;
    }
    ADVANCE('(', "(");
    AST::Node expr;
    skipCommaExpression(expr);
    ADVANCE(')', ")");

    return true;
}

bool Parser::parseTypeIdList(GroupAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    AST::Node typeId;
    if (!parseTypeId(typeId)) {
        return false;
    }

    GroupAST::Node ast = CreateNode<GroupAST>();
    ast->addNode(typeId);

    while (m_lexer->lookAhead(0) == ',') {
        nextToken();
        if (parseTypeId(typeId)) {
            ast->addNode(typeId);
        } else {
            reportError(i18n("Type id expected"));
            break;
        }
    }

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);
    return true;
}

bool Parser::parseBaseSpecifier(BaseSpecifierAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();
    BaseSpecifierAST::Node ast = CreateNode<BaseSpecifierAST>();

    AST::Node access;
    if (m_lexer->lookAhead(0) == Token_virtual) {
        AST_FROM_TOKEN(virt, m_lexer->index());
        ast->setIsVirtual(virt);

        nextToken();

        parseAccessSpecifier(access);
    } else {
        parseAccessSpecifier(access);

        if (m_lexer->lookAhead(0) == Token_virtual) {
            AST_FROM_TOKEN(virt, m_lexer->index());
            ast->setIsVirtual(virt);
            nextToken();
        }
    }

    NameAST::Node name;
    if (!parseName(name)) {
        reportError(i18n("Class name expected"));
    }

    ast->setAccess(access);
    ast->setName(name);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}


bool Parser::parseInitializerClause(AST::Node& node)
{
    PARSER_DEBUG_METHOD;

    if (m_lexer->lookAhead(0) == '{') {
        if (!skip('{','}')) {
            reportError(i18n("} missing"));
        } else {
            clearComment();
            nextToken();
        }
    } else {
        if (!parseAssignmentExpression(node)) {
            //reportError(i18n("Expression expected"));
        }
    }

    return true;
}

bool Parser::parseMemInitializerId(NameAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    return parseName(node);
}

bool Parser::parsePtrToMember(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;

    if (m_lexer->lookAhead(0) == Token_scope) {
        nextToken();
    }

    while (m_lexer->lookAhead(0) == Token_identifier) {
        nextToken();

        if (m_lexer->lookAhead(0) == Token_scope && m_lexer->lookAhead(1) == '*') {
            nextToken(); // skip ::
            nextToken(); // skip *
            return true;
        } else
            break;
    }

    return false;
}

bool Parser::parseUnqualifiedName(ClassOrNamespaceNameAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();
    bool isDestructor = false;

    ClassOrNamespaceNameAST::Node ast = CreateNode<ClassOrNamespaceNameAST>();

    if (m_lexer->lookAhead(0) == Token_identifier) {
        int startName = m_lexer->index();
        AST::Node n = CreateNode<AST>();
        nextToken();
        UPDATE_POS(n, startName, m_lexer->index());
        ast->setName(n);
    } else if (m_lexer->lookAhead(0) == '~' && m_lexer->lookAhead(1) == Token_identifier) {
        int startName = m_lexer->index();
        AST::Node n = CreateNode<AST>();
        nextToken(); // skip ~
        nextToken(); // skip classname
        UPDATE_POS(n, startName, m_lexer->index());
        ast->setName(n);
        isDestructor = true;
    } else if (m_lexer->lookAhead(0) == Token_operator) {
        AST::Node n;
        if (!parseOperatorFunctionId(n))
            return false;
        ast->setName(n);
    } else {
        return false;
    }

    if (!isDestructor) {

        int index = m_lexer->index();

        if (m_lexer->lookAhead(0) == '<') {
            nextToken();

            // optional template arguments
            TemplateArgumentListAST::Node args;
            parseTemplateArgumentList(args);

            if (m_lexer->lookAhead(0) != '>') {
                m_lexer->setIndex(index);
            } else {
                nextToken();
                ast->setTemplateArgumentList(args);
            }
        }
    }

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseStringLiteral(AST::Node& /*node*/)
{
    while (!m_lexer->lookAhead(0).isNull()) {
        if (m_lexer->lookAhead(0) == Token_identifier &&
                m_lexer->lookAhead(0).text() == QLatin1String("L") &&
                m_lexer->lookAhead(1) == Token_string_literal) {
            nextToken();
            nextToken();
        } else if (m_lexer->lookAhead(0) == Token_string_literal) {
            nextToken();
        } else
            return false;
    }
    return true;
}

bool Parser::skipExpressionStatement(StatementAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    AST::Node expr;
    skipCommaExpression(expr);

    ADVANCE(';', ";");

    ExpressionStatementAST::Node ast = CreateNode<ExpressionStatementAST>();
    ast->setExpression(expr);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseStatement(StatementAST::Node& node) // thanks to fiore@8080.it ;)
{
    PARSER_DEBUG_METHOD;
    switch (m_lexer->lookAhead(0)) {

    case Token_while:
        return parseWhileStatement(node);

    case Token_do:
        return parseDoStatement(node);

    case Token_for:
        return parseForStatement(node);

    case Token_foreach:
        return parseForEachStatement(node);

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
        nextToken();
        ADVANCE(';', ";");
        return true;

    case Token_goto:
        nextToken();
        ADVANCE(Token_identifier, "identifier");
        ADVANCE(';', ";");
        return true;

    case Token_return: {
        nextToken();
        AST::Node expr;
        skipCommaExpression(expr);
        ADVANCE(';', ";");
    }
    return true;

    case '{':
        return parseCompoundStatement(node);

    case Token_identifier:
        if (parseLabeledStatement(node))
            return true;
        break;
    }

    PARSER_DEBUG_METHOD;
    if (parseDeclarationStatement(node))
        return true;

    return skipExpressionStatement(node);
}

bool Parser::parseCondition(ConditionAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    ConditionAST::Node ast = CreateNode<ConditionAST>();

    TypeSpecifierAST::Node spec;
    if (parseTypeSpecifier(spec)) {
        DeclaratorAST::Node decl;
        if (parseDeclarator(decl)) {
            if (m_lexer->lookAhead(0) == '=') {
                nextToken();

                AST::Node expr;
                if (skipExpression(expr)) {
                    ast->setTypeSpec(spec);
                    ast->setDeclarator(decl);
                    ast->setExpression(expr);

                    UPDATE_POS(ast, start, m_lexer->index());
                    node = std::move(ast);

                    return true;
                }
            } else {
                ast->setTypeSpec(spec);
                ast->setDeclarator(decl);

                UPDATE_POS(ast, start, m_lexer->index());
                node = std::move(ast);

                return true;
            }
        }
    }

    m_lexer->setIndex(start);

    AST::Node expr;
    if (!skipCommaExpression(expr))
        return false;

    ast->setExpression(expr);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);
    return true;
}


bool Parser::parseWhileStatement(StatementAST::Node& node)
{
    PARSER_DEBUG_METHOD;
    int start = m_lexer->index();

    ADVANCE(Token_while, "while");
    ADVANCE('(' , "(");

    ConditionAST::Node cond;
    if (!parseCondition(cond)) {
        reportError(i18n("condition expected"));
        return false;
    }
    ADVANCE(')', ")");

    StatementAST::Node body;
    if (!parseStatement(body)) {
        reportError(i18n("statement expected"));
    }

    WhileStatementAST::Node ast = CreateNode<WhileStatementAST>();
    ast->setCondition(cond);
    ast->setStatement(body);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseDoStatement(StatementAST::Node& node)
{
    PARSER_DEBUG_METHOD;
    int start = m_lexer->index();

    ADVANCE(Token_do, "do");

    StatementAST::Node body;
    if (!parseStatement(body)) {
        reportError(i18n("statement expected"));
        //return false;
    }

    ADVANCE_NR(Token_while, "while");
    ADVANCE_NR('(' , "(");

    AST::Node expr;
    if (!skipCommaExpression(expr)) {
        reportError(i18n("expression expected"));
        //return false;
    }

    ADVANCE_NR(')', ")");
    ADVANCE_NR(';', ";");

    DoStatementAST::Node ast = CreateNode<DoStatementAST>();
    ast->setStatement(body);
    //ast->setCondition(condition);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseForStatement(StatementAST::Node& node)
{
    PARSER_DEBUG_METHOD;
    int start = m_lexer->index();

    ADVANCE(Token_for, "for");
    ADVANCE('(', "(");

    StatementAST::Node init;
    if (!parseForInitStatement(init)) {
        reportError(i18n("for initialization expected"));
        return false;
    }

    ConditionAST::Node cond;
    parseCondition(cond);
    ADVANCE(';', ";");

    AST::Node expr;
    skipCommaExpression(expr);
    ADVANCE(')', ")");

    StatementAST::Node body;
    if (!parseStatement(body)) {
        reportError(i18n("statement expected"));
    }

    ForStatementAST::Node ast = CreateNode<ForStatementAST>();
    ast->setInitStatement(init);
    ast->setCondition(cond);
    // ast->setExpression(expression);
    ast->setStatement(body);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

// qt4 [erbsland]
///@todo add the right parsing for the foreach statement
bool Parser::parseForEachStatement(StatementAST::Node& node)
{
    int start = m_lexer->index();

    ADVANCE(Token_foreach, "foreach");
    ADVANCE('(', "(");

    AST::Node expr;
    // replace with the right parsing
    skipCommaExpression(expr);
    ADVANCE(')', ")");

    StatementAST::Node body;
    if (!parseStatement(body)) {
        reportError(i18n("statement expected"));
    }

    ForEachStatementAST::Node ast = CreateNode<ForEachStatementAST>();
    // add here the parser results
    ast->setStatement(body);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseForInitStatement(StatementAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    if (parseDeclarationStatement(node))
        return true;

    return skipExpressionStatement(node);
}

bool Parser::parseCompoundStatement(StatementAST::Node& node)
{
    PARSER_DEBUG_METHOD;
    int start = m_lexer->index();

    if (m_lexer->lookAhead(0) != '{') {
        return false;
    }
    nextToken();

    StatementListAST::Node ast = CreateNode<StatementListAST>();

    while (!m_lexer->lookAhead(0).isNull()) {
        if (m_lexer->lookAhead(0) == '}')
            break;

        StatementAST::Node stmt;
        int startStmt = m_lexer->index();
        if (!parseStatement(stmt)) {
            if (startStmt == m_lexer->index())
                nextToken();
            skipUntilStatement();
        } else {
            ast->addStatement(stmt);
        }
    }

    clearComment();

    if (m_lexer->lookAhead(0) != '}') {
        reportError(i18n("} expected"));
    } else {
        nextToken();
    }

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseIfStatement(StatementAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    ADVANCE(Token_if, "if");

    ADVANCE('(' , "(");

    IfStatementAST::Node ast = CreateNode<IfStatementAST>();

    ConditionAST::Node cond;
    if (!parseCondition(cond)) {
        reportError(i18n("condition expected"));
        return false;
    }
    ADVANCE(')', ")");

    StatementAST::Node stmt;
    if (!parseStatement(stmt)) {
        reportError(i18n("statement expected"));
    }

    ast->setCondition(cond);
    ast->setStatement(stmt);

    if (m_lexer->lookAhead(0) == Token_else) {
        nextToken();
        StatementAST::Node elseStmt;
        if (!parseStatement(elseStmt)) {
            reportError(i18n("statement expected"));
        }
        ast->setElseStatement(elseStmt);
    }

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseSwitchStatement(StatementAST::Node& node)
{
    PARSER_DEBUG_METHOD;
    int start = m_lexer->index();
    ADVANCE(Token_switch, "switch");

    ADVANCE('(' , "(");

    ConditionAST::Node cond;
    if (!parseCondition(cond)) {
        reportError(i18n("condition expected"));
        return false;
    }
    ADVANCE(')', ")");

    StatementAST::Node stmt;
    if (!parseCompoundStatement(stmt)) {
        syntaxError();
        return false;
    }

    SwitchStatementAST::Node ast = CreateNode<SwitchStatementAST>();
    ast->setCondition(cond);
    ast->setStatement(stmt);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseLabeledStatement(StatementAST::Node& node)
{
    PARSER_DEBUG_METHOD;
    switch (m_lexer->lookAhead(0)) {
    case Token_identifier:
    case Token_default:
        if (m_lexer->lookAhead(1) == ':') {
            nextToken();
            nextToken();

            StatementAST::Node stmt;
            if (parseStatement(stmt)) {
                node = std::move(stmt);
                return true;
            }
        }
        break;

    case Token_case: {
        nextToken();
        AST::Node expr;
        if (!parseConstantExpression(expr)) {
            reportError(i18n("expression expected"));
        } else if (m_lexer->lookAhead(0) == Token_ellipsis) {
            nextToken();

            AST::Node expr2;
            if (!parseConstantExpression(expr2)) {
                reportError(i18n("expression expected"));
            }
        }
        ADVANCE(':', ":");

        StatementAST::Node stmt;
        if (parseStatement(stmt)) {
            node = std::move(stmt);
            return true;
        }
    }
    break;

    }

    return false;
}

bool Parser::parseBlockDeclaration(DeclarationAST::Node& node)
{
    PARSER_DEBUG_METHOD;
    switch (m_lexer->lookAhead(0)) {
    case Token_typedef:
        return parseTypedef(node);
    case Token_using:
        return parseUsing(node);
    case Token_asm:
        return parseAsmDefinition(node);
    case Token_namespace:
        return parseNamespaceAliasDefinition(node);
    }

    int start = m_lexer->index();

    GroupAST::Node storageSpec;
    parseStorageClassSpecifier(storageSpec);

    GroupAST::Node cv;
    parseCvQualify(cv);

    TypeSpecifierAST::Node spec;
    if (!parseTypeSpecifierOrClassSpec(spec)) { // replace with simpleTypeSpecifier?!?!
        m_lexer->setIndex(start);
        return false;
    }
    spec->setCvQualify(cv);

    GroupAST::Node cv2;
    parseCvQualify(cv2);
    spec->setCv2Qualify(cv2);

    InitDeclaratorListAST::Node declarators;
    parseInitDeclaratorList(declarators);

    if (m_lexer->lookAhead(0) != ';') {
        m_lexer->setIndex(start);
        return false;
    }
    nextToken();

    SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
    ast->setTypeSpec(spec);
    ast->setInitDeclaratorList(declarators);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parseNamespaceAliasDefinition(DeclarationAST::Node& /*node*/)
{
    if (m_lexer->lookAhead(0) != Token_namespace) {
        return false;
    }
    nextToken();

    ADVANCE(Token_identifier,  "identifier");
    ADVANCE('=', "=");

    NameAST::Node name;
    if (!parseName(name)) {
        reportError(i18n("Namespace name expected"));
    }

    ADVANCE(';', ";");

    return true;

}

bool Parser::parseDeclarationStatement(StatementAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    DeclarationAST::Node decl;
    if (!parseBlockDeclaration(decl)) {
        return false;
    }

    DeclarationStatementAST::Node ast = CreateNode<DeclarationStatementAST>();
    ast->setDeclaration(decl);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    PARSER_DEBUG_METHOD;
    return true;
}

bool Parser::parseDeclarationInternal(DeclarationAST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

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

    int index = m_lexer->index();
    NameAST::Node name;
    if (parseName(name) && m_lexer->lookAhead(0) == '(') {
        // no type specifier, maybe a constructor or a cast operator??

        m_lexer->setIndex(index);

        InitDeclaratorAST::Node declarator;
        if (parseInitDeclarator(declarator)) {
            int endSignature = m_lexer->index();

            Comment mcomment;
            if (&(*declarator)) {
                int endLine, endColumn;
                declarator->getEndPosition(&endLine, &endColumn);
                mcomment = m_commentStore.getCommentsInRange(endLine);
            }
            else {
                mcomment = comment();
            }
            clearComment();

            switch (m_lexer->lookAhead(0)) {
            case ';': {
                nextToken();

                InitDeclaratorListAST::Node declarators = CreateNode<InitDeclaratorListAST>();

                SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();

                // update declarators position
                int startLine, startColumn, endLine, endColumn;
                if (declarator.get()) {
                    declarator->getStartPosition(&startLine, &startColumn);
                    declarator->getEndPosition(&endLine, &endColumn);
                    declarators->setStartPosition(startLine, startColumn);
                    declarators->setEndPosition(endLine, endColumn);

                    ast->setComment(mcomment);
                    preparseLineComments(endLine);
                    Comment c = m_commentStore.getCommentInRange(endLine);
                    if (c) {
                        ast->addComment(c);
                    }
                }
                declarators->addInitDeclarator(declarator);

                ast->setInitDeclaratorList(declarators);
                if (hasFunSpec)
                    ast->setFunctionSpecifier(funSpec);
                ast->setText(toString(start, endSignature));
                node = std::move(ast);
                UPDATE_POS(node, start, m_lexer->index());
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
                    node = std::move(ast);
                    UPDATE_POS(node, start, m_lexer->index());
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
                    node = std::move(ast);
                    UPDATE_POS(node, start, m_lexer->index());
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
    m_lexer->setIndex(index);

    if (m_lexer->lookAhead(0) == Token_const && m_lexer->lookAhead(1) == Token_identifier && m_lexer->lookAhead(2) == '=') {
        // constant definition
        nextToken();
        InitDeclaratorListAST::Node declarators;
        if (parseInitDeclaratorList(declarators)) {
            ADVANCE(';', ";");
            DeclarationAST::Node ast = CreateNode<DeclarationAST>();
            node = std::move(ast);
            UPDATE_POS(node, start, m_lexer->index());
            return true;
        }
        syntaxError();
        return false;
    }

    TypeSpecifierAST::Node spec;
    if (parseTypeSpecifier(spec)) {
        if (!hasFunSpec)
            parseFunctionSpecifier(funSpec);    // e.g. "void inline"
        spec->setCvQualify(cv);

        InitDeclaratorListAST::Node declarators;

        InitDeclaratorAST::Node decl;
        int startDeclarator = m_lexer->index();
        bool maybeFunctionDefinition = false;

        if (m_lexer->lookAhead(0) != ';') {
            if (parseInitDeclarator(decl) && m_lexer->lookAhead(0) == '{') {
                // function definition
                maybeFunctionDefinition = true;
            } else {
                m_lexer->setIndex(startDeclarator);
                if (!parseInitDeclaratorList(declarators)) {
                    syntaxError();
                    return false;
                }
            }
        }

        Comment mcomment;
        if (&(*decl)) {
            int line, col;
            decl->getEndPosition(&line, &col);
            mcomment = m_commentStore.getCommentsInRange(line);
        }
        else {
            mcomment = comment();
        }
        clearComment();

        int endSignature = m_lexer->index();
        switch (m_lexer->lookAhead(0)) {
        case ';': {
            nextToken();
            SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
            int line, col;
            ast->setComment(mcomment);
            if (&(*decl)) {
                decl->getEndPosition(&line, &col);

                preparseLineComments(line);
                Comment c = m_commentStore.getCommentInRange(line);
                if (c) {
                    ast->addComment(c);
                }
            }

            ast->setStorageSpecifier(storageSpec);
            ast->setFunctionSpecifier(funSpec);
            ast->setText(toString(start, endSignature));
            ast->setTypeSpec(spec);
            ast->setWinDeclSpec(winDeclSpec);
            ast->setInitDeclaratorList(declarators);

            node = std::move(ast);
            UPDATE_POS(node, start, m_lexer->index());
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

                ast->setComment(mcomment);
                if (&(*decl)) {
                    int line, col;
                    decl->getEndPosition(&line, &col);

                    preparseLineComments(line);
                    Comment c = m_commentStore.getCommentInRange(line);
                    if (c) {
                        ast->addComment(c);
                    }
                }

                ast->setWinDeclSpec(winDeclSpec);
                ast->setStorageSpecifier(storageSpec);
                ast->setFunctionSpecifier(funSpec);
                ast->setText(toString(start, endSignature));
                ast->setTypeSpec(spec);
                ast->setFunctionBody(funBody);
                ast->setInitDeclarator(decl);
                node = std::move(ast);
                UPDATE_POS(node, start, m_lexer->index());
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
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();
    if (m_lexer->lookAhead(0) != '{') {
        return false;
    }
    nextToken();

    StatementListAST::Node ast = CreateNode<StatementListAST>();

    while (!m_lexer->lookAhead(0).isNull()) {
        if (m_lexer->lookAhead(0) == '}')
            break;

        StatementAST::Node stmt;
        int startStmt = m_lexer->index();
        if (!parseStatement(stmt)) {
            if (startStmt == m_lexer->index())
                nextToken();
            skipUntilStatement();
        } else
            ast->addStatement(stmt);
    }

    clearComment();

    if (m_lexer->lookAhead(0) != '}') {
        reportError(i18n("} expected"));
    } else
        nextToken();

    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

QString Parser::toString(int start, int end, const QString& sep) const
{
    QStringList l;

    for (int i=start; i<end; ++i) {
        const Token& t = m_lexer->tokenAt(i);
        if (t != Token_comment)
            l << t.text();
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
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();
    if (m_lexer->lookAhead(0) != Token_try) {
        return false;
    }
    nextToken();

    StatementAST::Node stmt;
    if (!parseCompoundStatement(stmt)) {
        syntaxError();
    }

    if (m_lexer->lookAhead(0) != Token_catch) {
        reportError(i18n("catch expected"));
    }

    CatchStatementListAST::Node list = CreateNode<CatchStatementListAST>();

    while (m_lexer->lookAhead(0) == Token_catch) {

        nextToken();
        ADVANCE('(', "(");
        ConditionAST::Node cond;
        if (!parseCondition(cond)) {
            reportError(i18n("condition expected"));
            return false;
        }
        ADVANCE(')', ")");

        StatementAST::Node body;
        if (!parseCompoundStatement(body)) {
            syntaxError();
        }

        CatchStatementAST::Node cstmt = CreateNode<CatchStatementAST>();
        cstmt->setCondition(cond);
        cstmt->setStatement(body);
        int l=0, c=0;
        if (cond.get())
            cond->getStartPosition(&l, &c);
        else if (body.get())
            body->getStartPosition(&l, &c);

        cstmt->setStartPosition(l, c);
        if (body.get())
            body->getEndPosition(&l, &c);

        cstmt->setEndPosition(l, c);
        list->addStatement(cstmt);
    }

    TryBlockStatementAST::Node ast = CreateNode<TryBlockStatementAST>();
    ast->setStatement(stmt);
    ast->setCatchStatementList(list);
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);

    return true;
}

bool Parser::parsePrimaryExpression(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;


    switch (m_lexer->lookAhead(0)) {
    case Token_string_literal: {
        AST::Node lit;
        parseStringLiteral(lit);
    }
    return true;

    case Token_number_literal:
    case Token_char_literal:
    case Token_true:
    case Token_false:
        nextToken();
        return true;

    case Token_this:
        nextToken();
        return true;

    case Token_dynamic_cast:
    case Token_static_cast:
    case Token_reinterpret_cast:
    case Token_const_cast: {
        nextToken();

        CHECK('<', "<");
        AST::Node typeId;
        parseTypeId(typeId);
        CHECK('>', ">");

        CHECK('(', "(");
        AST::Node expr;
        parseCommaExpression(expr);
        CHECK(')', ")");
    }
    return true;

    case Token_typeid: {
        nextToken();
        CHECK('(', "(");
        AST::Node expr;
        parseCommaExpression(expr);
        CHECK(')', ")");
    }
    return true;

    case '(': {
        nextToken();
        PARSER_DEBUG_METHOD;
        AST::Node expr;
        if (!parseExpression(expr)) {
            return false;
        }
        CHECK(')', ")");
    }
    return true;

    default: {
        int start = m_lexer->index();
        TypeSpecifierAST::Node typeSpec;
        if (parseSimpleTypeSpecifier(typeSpec) && m_lexer->lookAhead(0) == '(') {
            nextToken();
            AST::Node expr;
            parseCommaExpression(expr);
            CHECK(')', ")");
            return true;
        }

        m_lexer->setIndex(start);
        NameAST::Node name;
        if (parseName(name))
            return true;
    }
    }

    return false;
}

bool Parser::parsePostfixExpression(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;

    AST::Node expr;
    if (!parsePrimaryExpression(expr))
        return false;

    while (true) {
        switch (m_lexer->lookAhead(0)) {
        case '[': {
            nextToken();
            AST::Node e;
            parseCommaExpression(e);
            CHECK(']', "]");
        }
        break;

        case '(': {
            nextToken();
            AST::Node funArgs;
            parseCommaExpression(funArgs);
            CHECK(')', ")");
        }
        break;

        case Token_incr:
        case Token_decr:
            nextToken();
            break;

        case '.':
        case Token_arrow: {
            nextToken();
            if (m_lexer->lookAhead(0) == Token_template)
                nextToken();

            NameAST::Node name;
            if (!parseName(name)) {
                return false;
            }
        }
        break;

        case Token_typename: {
            nextToken();

            NameAST::Node name;
            if (!parseName(name)) {
                return false;
            }

            CHECK('(', "(");
            AST::Node expr;
            parseCommaExpression(expr);
            CHECK(')', ")");
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
    PARSER_DEBUG_METHOD;

    switch (m_lexer->lookAhead(0)) {
    case Token_incr:
    case Token_decr:
    case '*':
    case '&':
    case '+':
    case '-':
    case '!':
    case '~': {
        nextToken();
        AST::Node expr;
        return parseCastExpression(expr);
    }

    case Token_sizeof: {
        nextToken();
        int index = m_lexer->index();
        if (m_lexer->lookAhead(0) == '(') {
            nextToken();
            AST::Node typeId;
            if (parseTypeId(typeId) && m_lexer->lookAhead(0) == ')') {
                nextToken();
                return true;
            }
            m_lexer->setIndex(index);
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
    PARSER_DEBUG_METHOD;
    if (m_lexer->lookAhead(0) == Token_scope && m_lexer->lookAhead(1) == Token_new)
        nextToken();

    CHECK(Token_new, "new");

    if (m_lexer->lookAhead(0) == '(') {
        nextToken();
        AST::Node expr;
        parseCommaExpression(expr);
        CHECK(')', ")");
    }

    if (m_lexer->lookAhead(0) == '(') {
        nextToken();
        AST::Node typeId;
        parseTypeId(typeId);
        CHECK(')', ")");
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
    PARSER_DEBUG_METHOD;
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
    PARSER_DEBUG_METHOD;
    AST::Node ptrOp;
    if (parsePtrOperator(ptrOp)) {
        AST::Node declarator;
        parseNewDeclarator(declarator);
        return true;
    }

    if (m_lexer->lookAhead(0) == '[') {
        while (m_lexer->lookAhead(0) == '[') {
            nextToken();
            AST::Node expr;
            parseExpression(expr);
            ADVANCE(']', "]");
        }
        return true;
    }

    return false;
}

bool Parser::parseNewInitializer(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;
    if (m_lexer->lookAhead(0) != '(')
        return false;

    nextToken();
    AST::Node expr;
    parseCommaExpression(expr);
    CHECK(')', ")");

    return true;
}

bool Parser::parseDeleteExpression(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;
    if (m_lexer->lookAhead(0) == Token_scope && m_lexer->lookAhead(1) == Token_delete)
        nextToken();

    CHECK(Token_delete, "delete");

    if (m_lexer->lookAhead(0) == '[') {
        nextToken();
        CHECK(']', "]");
    }

    AST::Node expr;
    return parseCastExpression(expr);
}

bool Parser::parseCastExpression(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;

    int index = m_lexer->index();

    if (m_lexer->lookAhead(0) == '(') {
        nextToken();
        AST::Node typeId;
        if (parseTypeId(typeId)) {
            if (m_lexer->lookAhead(0) == ')') {
                nextToken();
                AST::Node expr;
                if (parseCastExpression(expr))
                    return true;
            }
        }
    }

    m_lexer->setIndex(index);

    AST::Node expr;
    return parseUnaryExpression(expr);
}

bool Parser::parsePmExpression(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;
    AST::Node expr;
    if (!parseCastExpression(expr))
        return false;

    while (m_lexer->lookAhead(0) == Token_ptrmem) {
        nextToken();

        if (!parseCastExpression(expr))
            return false;
    }

    return true;
}

bool Parser::parseMultiplicativeExpression(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;
    AST::Node expr;
    if (!parsePmExpression(expr))
        return false;

    while (m_lexer->lookAhead(0) == '*' || m_lexer->lookAhead(0) == '/' || m_lexer->lookAhead(0) == '%') {
        nextToken();

        if (!parsePmExpression(expr))
            return false;
    }

    return true;
}


bool Parser::parseAdditiveExpression(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;
    AST::Node expr;
    if (!parseMultiplicativeExpression(expr))
        return false;

    while (m_lexer->lookAhead(0) == '+' || m_lexer->lookAhead(0) == '-') {
        nextToken();

        if (!parseMultiplicativeExpression(expr))
            return false;
    }

    return true;
}

bool Parser::parseShiftExpression(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;
    AST::Node expr;
    if (!parseAdditiveExpression(expr))
        return false;

    while (m_lexer->lookAhead(0) == Token_shift || (m_lexer->lookAhead(0) == '>' && m_lexer->lookAhead(1) == '>')) {
        nextToken();

        if (!parseAdditiveExpression(expr))
            return false;
    }

    return true;
}

bool Parser::parseRelationalExpression(AST::Node& /*node*/, bool templArgs)
{
    PARSER_DEBUG_METHOD;
    AST::Node expr;
    if (!parseShiftExpression(expr))
        return false;

    while (m_lexer->lookAhead(0) == '<' || (m_lexer->lookAhead(0) == '>' && !templArgs) ||
           m_lexer->lookAhead(0) == Token_leq || m_lexer->lookAhead(0) == Token_geq) {
        nextToken();

        if (!parseShiftExpression(expr))
            return false;
    }

    return true;
}

bool Parser::parseEqualityExpression(AST::Node& /*node*/, bool templArgs)
{
    PARSER_DEBUG_METHOD;
    AST::Node expr;
    if (!parseRelationalExpression(expr, templArgs))
        return false;

    while (m_lexer->lookAhead(0) == Token_eq || m_lexer->lookAhead(0) == Token_not_eq) {
        nextToken();

        if (!parseRelationalExpression(expr, templArgs))
            return false;
    }

    return true;
}

bool Parser::parseAndExpression(AST::Node& /*node*/, bool templArgs)
{
    PARSER_DEBUG_METHOD;
    AST::Node expr;
    if (!parseEqualityExpression(expr, templArgs))
        return false;

    while (m_lexer->lookAhead(0) == '&') {
        nextToken();

        if (!parseEqualityExpression(expr, templArgs))
            return false;
    }

    return true;
}

bool Parser::parseExclusiveOrExpression(AST::Node& /*node*/, bool templArgs)
{
    PARSER_DEBUG_METHOD;
    AST::Node expr;
    if (!parseAndExpression(expr, templArgs))
        return false;

    while (m_lexer->lookAhead(0) == '^') {
        nextToken();

        if (!parseAndExpression(expr, templArgs))
            return false;
    }

    return true;
}

bool Parser::parseInclusiveOrExpression(AST::Node& /*node*/, bool templArgs)
{
    PARSER_DEBUG_METHOD;
    AST::Node expr;
    if (!parseExclusiveOrExpression(expr, templArgs))
        return false;

    while (m_lexer->lookAhead(0) == '|') {
        nextToken();

        if (!parseExclusiveOrExpression(expr, templArgs))
            return false;
    }

    return true;
}

bool Parser::parseLogicalAndExpression(AST::Node& /*node*/, bool templArgs)
{
    PARSER_DEBUG_METHOD;

    AST::Node expr;
    if (!parseInclusiveOrExpression(expr, templArgs))
        return false;

    while (m_lexer->lookAhead(0) == Token_and) {
        nextToken();

        if (!parseInclusiveOrExpression(expr, templArgs))
            return false;
    }

    return true;
}

bool Parser::parseLogicalOrExpression(AST::Node& node, bool templArgs)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    AST::Node expr;
    if (!parseLogicalAndExpression(expr, templArgs))
        return false;

    while (m_lexer->lookAhead(0) == Token_or) {
        nextToken();

        if (!parseLogicalAndExpression(expr, templArgs))
            return false;
    }

    AST::Node ast = CreateNode<AST>();
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);
    return true;
}

bool Parser::parseConditionalExpression(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;
    AST::Node expr;
    if (!parseLogicalOrExpression(expr))
        return false;

    if (m_lexer->lookAhead(0) == '?') {
        nextToken();

        if (!parseExpression(expr))
            return false;

        CHECK(':', ":");

        if (!parseAssignmentExpression(expr))
            return false;
    }

    return true;
}

bool Parser::parseAssignmentExpression(AST::Node& node)
{
    PARSER_DEBUG_METHOD;
    int start = m_lexer->index();
    AST::Node expr;
    if (m_lexer->lookAhead(0) == Token_throw && !parseThrowExpression(expr))
        return false;
    else if (!parseConditionalExpression(expr))
        return false;

    while (m_lexer->lookAhead(0) == Token_assign || m_lexer->lookAhead(0) == '=') {
        nextToken();

        if (!parseConditionalExpression(expr))
            return false;
    }

    AST::Node ast = CreateNode<AST>();
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);
    return true;
}

bool Parser::parseConstantExpression(AST::Node& node)
{
    PARSER_DEBUG_METHOD;
    int start = m_lexer->index();
    if (parseConditionalExpression(node)) {
        AST::Node ast = CreateNode<AST>();
        UPDATE_POS(ast, start, m_lexer->index());
        node = std::move(ast);
        return true;
    }
    return false;
}

bool Parser::parseExpression(AST::Node& node)
{
    PARSER_DEBUG_METHOD;

    int start = m_lexer->index();

    if (!parseCommaExpression(node))
        return false;

    AST::Node ast = CreateNode<AST>();
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);
    return true;
}

bool Parser::parseCommaExpression(AST::Node& node)
{
    PARSER_DEBUG_METHOD;
    int start = m_lexer->index();

    AST::Node expr;
    if (!parseAssignmentExpression(expr))
        return false;

    while (m_lexer->lookAhead(0) == ',') {
        nextToken();

        if (!parseAssignmentExpression(expr))
            return false;
    }

    AST::Node ast = CreateNode<AST>();
    UPDATE_POS(ast, start, m_lexer->index());
    node = std::move(ast);
    return true;
}

bool Parser::parseThrowExpression(AST::Node& /*node*/)
{
    PARSER_DEBUG_METHOD;
    if (m_lexer->lookAhead(0) != Token_throw)
        return false;

    CHECK(Token_throw, "throw");
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
    int start = m_lexer->index();

    if (m_lexer->lookAhead(0) != Token_identifier)
        return false;

    GroupAST::Node ast = CreateNode<GroupAST>();

    AST_FROM_TOKEN(tk, m_lexer->index());
    ast->addNode(tk);
    nextToken();

    while (m_lexer->lookAhead(0) == ',') {
        nextToken();
        if (m_lexer->lookAhead(0) == Token_identifier) {
            AST_FROM_TOKEN(tk, m_lexer->index());
            ast->addNode(tk);
//      nextToken();
        }
        ADVANCE(Token_identifier, "identifier");
    }

    node = std::move(ast);
    UPDATE_POS(node, start, m_lexer->index());
    return true;
}

bool Parser::parseIdentifierColon(AST::Node & node)
{
    Q_UNUSED(node);

    if (m_lexer->lookAhead(0) == Token_identifier && m_lexer->lookAhead(1) == ':') {
        nextToken();
        nextToken();
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

    ADVANCE(OBJC_CLASS, "@class");

    GroupAST::Node idList;
    if (!parseIdentifierList(idList))
        return false;

    ADVANCE(';', ";");

    return true;
}

bool Parser::parseObjcProtocolDecl(DeclarationAST::Node & node)
{
    Q_UNUSED(node);

    ADVANCE(OBJC_PROTOCOL, "@protocol");

    GroupAST::Node idList;
    if (!parseIdentifierList(idList))
        return false;

    ADVANCE(';', ";");

    return true;
}

bool Parser::parseObjcAliasDecl(DeclarationAST::Node & node)
{
    Q_UNUSED(node);

    ADVANCE(OBJC_ALIAS, "@alias");

    GroupAST::Node idList;
    if (!parseIdentifierList(idList))
        return false;

    ADVANCE(';', ";");

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
    if (m_lexer->lookAhead(0) == Token_identifier &&
            m_lexer->lookAhead(0).text() == QLatin1String("__declspec") &&
            m_lexer->lookAhead(1) == '(' &&
            m_lexer->lookAhead(2) != ')') {
        int start = m_lexer->index();
        nextToken();
        nextToken(); // skip '('

        if (!parseIdentifierList(node))
            return false;

        ADVANCE(')', ")");

        UPDATE_POS(node, start, m_lexer->index());
        return true;
    }

    return false;
}

