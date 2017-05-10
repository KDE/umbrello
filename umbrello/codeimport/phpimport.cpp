/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2017                                                     *
 *  Umbrello UML Modeller Authors <umbrello-devel@kde.org>                 *
 ***************************************************************************/

// own header
#include "phpimport.h"

// app includes
#include "association.h"
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "enum.h"
#include "import_utils.h"
#include "operation.h"
#include "optionstate.h"
#include "package.h"
#include "template.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"

// KDE includes
#include <KLocalizedString>

// qt includes
#include <QListWidget>
#include <QMap>

QTextStream qout(stdout);
QTextStream qerr(stderr);
QTextStream qin(stdin);

#include "parsesession.h"
#include "phplexer.h"
#include "phpparser.h"
#include "phpdebugvisitor.h"
#include "phpast.h"
#include "tokenstream.h"
#include "phptokentext.h"

#include <tests/autotestshell.h>
#include <language/duchain/duchain.h>
#include <language/duchain/problem.h>
#include <language/codegen/coderepresentation.h>
#include <tests/testcore.h>

namespace Php {

class PHPIncludeFileVisitor : public DefaultVisitor
{
public:
    PHPIncludeFileVisitor(TokenStream *str, const QString& content = QString())
      : m_str (str),
        m_content(content)
    {}

    void visitStart(StartAst *ast, QStringList &dependencies)
    {
        m_dependencies = &dependencies;
        DefaultVisitor::visitStart(ast);
    }

    virtual void visitUnaryExpression(UnaryExpressionAst *node)
    {
        if (node->includeExpression) {
            visitIncludeExpression(node->includeExpression);
        }
        DefaultVisitor::visitUnaryExpression(node);
    }

    void visitIncludeExpression(UnaryExpressionAst *node)
    {
        QString tokenString;
        if (!m_content.isEmpty())
        {
            TokenStream::Token startToken = m_str->at(node->startToken);
            TokenStream::Token endToken = m_str->at(node->endToken);
            int begin = startToken.begin;
            int end = endToken.end;
            tokenString = m_content.mid(begin, end-begin+1);
            if (tokenString.startsWith("(\"") || tokenString.startsWith("('"))
                tokenString = tokenString.mid(2, tokenString.size() - 4);
            qDebug() << "-------------------include ----- " << tokenString;
            if (!m_dependencies->contains(tokenString))
                m_dependencies->append(tokenString);
        }
    }

    TokenStream *m_str;
    QString m_content;
    int m_indent;
    QStringList *m_dependencies;
};

const int NamespaceSize = 100;

/**
 * Derived visitor class
 */
class PHPImportVisitor : public DefaultVisitor
{
public:
    PHPImportVisitor(TokenStream *str, const QString& content = QString())
      : m_str (str),
        m_content(content),
        m_indent(0),
        m_nsCnt(0)
    {
        m_currentNamespace.fill(0, NamespaceSize);
        m_usingNamespaces.fill(0, NamespaceSize);
    }

    QString tokenValue(AstNode *node)
    {
        TokenStream::Token startToken = m_str->at(node->startToken);
        TokenStream::Token endToken = m_str->at(node->endToken);
        int begin = startToken.begin;
        int end = endToken.end;
        return m_content.mid(begin, end-begin+1);
    }

    void visitSimpleNamespaceDeclarationStatement(NamespaceDeclarationStatementAst *node)
    {
        QStringList nsNames;
        if (node->namespaceNameSequence)
        {
            const KDevPG::ListNode<IdentifierAst*> *it = node->namespaceNameSequence->front(), *end = it;
            do {
                nsNames.append(tokenValue(it->element));
                visitNode(it->element);
                it = it->next;
            } while (it != end);
        }

        m_nsCnt = 0;
        foreach(const QString &nsName, nsNames) {
            UMLPackage *parentPackage = m_currentNamespace[m_nsCnt];
            UMLObject *o = UMLApp::app()->document()->findUMLObject(nsName, UMLObject::ot_Package, parentPackage);
            if (!o)
                o = Import_Utils::createUMLObject(UMLObject::ot_Package, nsName, parentPackage);
            if (++m_nsCnt > NamespaceSize) {
                uError() << "excessive namespace nesting";
                m_nsCnt = NamespaceSize;
            }
            UMLPackage *ns = o->asUMLPackage();
            m_currentScope.push_back(nsName);
            m_currentNamespace[m_nsCnt] = ns;
        }
    }

    void visitStapledNamespaceDeclarationStatement(NamespaceDeclarationStatementAst *node)
    {
        QStringList nsNames;
        if (node->namespaceNameSequence)
        {
            const KDevPG::ListNode<IdentifierAst*> *it = node->namespaceNameSequence->front(), *end = it;
            do {
                nsNames.append(tokenValue(it->element));
                visitNode(it->element);
                it = it->next;
            } while (it != end);
        }

        m_nsCnt = 0;
        foreach(const QString &nsName, nsNames) {
            UMLPackage *parentPackage = m_currentNamespace[m_nsCnt];
            UMLObject *o = UMLApp::app()->document()->findUMLObject(nsName, UMLObject::ot_Package, parentPackage);
            if (!o)
                o = Import_Utils::createUMLObject(UMLObject::ot_Package, nsName, parentPackage);
            if (++m_nsCnt > NamespaceSize) {
                uError() << "excessive namespace nesting";
                m_nsCnt = NamespaceSize;
            }
            UMLPackage *ns = o->asUMLPackage();
            m_currentScope.push_back(nsName);
            m_currentNamespace[m_nsCnt] = ns;
        }
        visitNode(node->body);
        m_nsCnt = 0;
        m_currentScope.clear();
    }

    void visitNamespaceDeclarationStatement(NamespaceDeclarationStatementAst *node)
    {
        if (!node->body)
            visitSimpleNamespaceDeclarationStatement(node);
        else
            visitStapledNamespaceDeclarationStatement(node);
    }

    void visitUseNamespace(UseNamespaceAst *node)
    {
        QStringList nsNames;
        if (node->identifier->namespaceNameSequence)
        {
            const KDevPG::ListNode<IdentifierAst*> *it = node->identifier->namespaceNameSequence->front(), *end = it;
            do {
                nsNames.append(tokenValue(it->element));
                visitNode(it->element);
                it = it->next;
            } while (it != end);
        }
        DefaultVisitor::visitUseNamespace(node);
        UMLPackage *parent = 0;
        UMLPackage *p = 0;
        foreach(const QString &nsName, nsNames) {
            UMLObject *o = UMLApp::app()->document()->findUMLObject(nsName, UMLObject::ot_Package, parent);
            if (!o)
                o = Import_Utils::createUMLObject(UMLObject::ot_Package, nsName, parent);
            p = o->asUMLPackage();
            parent = p;
        }
        if (p) {
            m_usingNamespaces.append(p);
            uDebug() << "adding namespace to use" << nsNames.join("::");
        }
        // TODO add searching referenced objects in that namespace
    }

    void visitClassVariable(ClassVariableAst *node)
    {
        DefaultVisitor::visitClassVariable(node);
        _printToken(node->variable, "VariableIdentifierAst", "ClassVariableAst");
    }

    void visitClassStatement(ClassStatementAst *node)
    {
        if (!node || !node->methodName)
            return;
        QString methodName = tokenValue(node->methodName);
        Uml::Visibility::Enum m_currentAccess = Uml::Visibility::Public;
        QString returnType = "void";
        bool isStatic = false;
        bool isAbstract = false;
        bool isFriend = false;
        bool isConstructor = false;
        bool isDestructor = false;
        QString m_comment;
        UMLPackage *parentPackage = m_currentNamespace[m_nsCnt];
        UMLClassifier *c = parentPackage->asUMLClassifier();
        if (c) {
            UMLOperation *m = Import_Utils::makeOperation(c, methodName);
            Import_Utils::insertMethod(c, m, m_currentAccess, returnType,
                                       isStatic, isAbstract, isFriend, isConstructor,
                                       isDestructor, m_comment);
        }  else {
            uError() << "no parent class found for method" << methodName;
        }
        DefaultVisitor::visitClassStatement(node);
        //_printToken(node->methodName, "IdentifierAst", "ClassStatement");
    }

    void visitClassDeclarationStatement(ClassDeclarationStatementAst *node)
    {
        _printToken(node->className, "IdentifierAst", "ClassDeclarationStatement");
        QString className = tokenValue(node->className);

        UMLObject *o = UMLApp::app()->document()->findUMLObject(className, UMLObject::ot_Class, m_currentNamespace[m_nsCnt]);
        if (!o)
            o = UMLApp::app()->document()->findUMLObject(className, UMLObject::ot_Datatype, m_currentNamespace[m_nsCnt]);
        if (!o)
            o = Import_Utils::createUMLObject(UMLObject::ot_Class, className,
                                              m_currentNamespace[m_nsCnt],
                                              QString()/*ast->comment()*/, QString(), true);
        m_currentScope.push_back(className);
        if (++m_nsCnt > NamespaceSize) {
            uError() << "excessive namespace nesting";
            m_nsCnt = NamespaceSize;
        }
        UMLPackage *ns = o->asUMLPackage();
        m_currentNamespace[m_nsCnt] = ns;
        // UMLClassifier *klass = o->asUMLClassifier();
        // handle modifier, extents, implements
        DefaultVisitor::visitClassDeclarationStatement(node);
        --m_nsCnt;
        m_currentScope.pop_back();
    }

    void visitFunctionDeclarationStatement(FunctionDeclarationStatementAst *node)
    {
        DefaultVisitor::visitFunctionDeclarationStatement(node);
        _printToken(node->functionName, "IdentifierAst", "FunctionDeclarationStatementAst");
    }

    void _printToken(AstNode *node, const QString &mType, const QString &mName = QString())
    {
        if (!node)
            return;
        QString tokenString;
        if (!m_content.isEmpty())
        {
            TokenStream::Token startToken = m_str->at(node->startToken);
            TokenStream::Token endToken = m_str->at(node->endToken);
            int begin = startToken.begin;
            int end = endToken.end;
            if (end-begin > 30)
            {
                tokenString = m_content.mid(begin, 10);
                tokenString += " ...";
                tokenString += QString("%1 more").arg(end-begin-20);
                tokenString += "... ";
                tokenString += m_content.mid(end-10, 10);
            }
            else
            {
                tokenString = m_content.mid(begin, end-begin+1);
            }
            tokenString = tokenString.replace('\n', "\\n");
            tokenString = tokenString.replace('\r', "\\r");
        }
        qint64 beginLine,endLine,beginCol,endCol;
        m_str->startPosition(node->startToken, &beginLine, &beginCol);
        m_str->endPosition(node->endToken, &endLine, &endCol);
        qDebug() << "++++" << QString().fill(' ', m_indent) + mName + (!mName.isEmpty() ? "->" : "") + mType + "[" << m_str->at( node->startToken ).begin << "," << beginLine << "," << beginCol << "] --- [" << m_str->at( node->endToken ).end << "," << endLine << "," << endCol << "] " << tokenString;
    }

    TokenStream *m_str;
    QString m_content;
    int m_indent;
    QVector<QPointer<UMLPackage>> m_currentNamespace;
    QVector<QPointer<UMLPackage>> m_usingNamespaces;
    QStringList m_currentScope;
    int m_nsCnt;
};

} // namespace

/**
 * This class is a pure helper to use for binaries that you can
 * run on short snippets of test code or whole files and let
 * it print the generated tokens or AST.
 *
 * It should work fine for any KDevelop-PG-Qt based parser.
 *
 *
 * @param SessionT the parse session for your language.
 * @param TokenStreamT the token stream for your language, based on KDevPG::TokenStreamBase.
 * @param TokenT the token class for your language, based on KDevPG::Token.
 * @param LexerT the Lexer for your language.
 * @param StartAstT the AST node that is returned from @c SessionT::parse().
 * @param DebugVisitorT the debug visitor for your language.
 * @param TokenToTextT function pointer to the function that returns a string representation for an integral token.
 */
typedef QString (*TokenTextFunc)(int);
template<class SessionT, class TokenStreamT, class TokenT, class LexerT,
         class StartAstT, class DebugVisitorT, TokenTextFunc TokenTextT>
class DebugLanguageParserHelper {
public:
    DebugLanguageParserHelper(const bool printAst, const bool printTokens)
        : m_printAst(printAst), m_printTokens(printTokens)
    {
        m_session.setDebug(printAst);
    }

    /// parse contents of a file
    bool parseFile( const QString &fileName )
    {
        if (!m_session.readFile(fileName, "utf-8")) {
            qerr << "Can't open file " << fileName << endl;
            std::exit(255);
        } else {
            qout << "Parsing file " << fileName << endl;
        }
        return runSession();
    }

    /// parse code directly
    bool parseCode( const QString &code )
    {
        m_session.setContents(code);

        qout << "Parsing input" << endl;
        return runSession();
    }

    Php::TokenStream *tokenStream()
    {
        return m_session.tokenStream();
    }

    QString contents() const
    {
        return m_session.contents();
    }

    QStringList dependencies() const
    {
        return m_dependencies;
    }

    StartAstT *ast() const
    {
        return m_ast;
    }
private:
    /**
     * actually run the parse session
     */
    bool runSession()
    {
        bool result = true;

        if (m_printTokens) {
            TokenStreamT tokenStream;
            LexerT lexer(&tokenStream, m_session.contents());
            int token;
            while ((token = lexer.nextTokenKind())) {
                TokenT &t = tokenStream.push();
                t.begin = lexer.tokenBegin();
                t.end = lexer.tokenEnd();
                t.kind = token;
                printToken(token, lexer);
            }
            printToken(token, lexer);
            if ( tokenStream.size() > 0 ) {
                qint64 line;
                qint64 column;
                tokenStream.endPosition(tokenStream.size() - 1, &line, &column);
                qDebug() << "last token endPosition: line" << line << "column" << column;
            } else {
                qDebug() << "empty token stream";
            }
        }
        m_ast = 0;
        if (!m_session.parse(&m_ast)) {
            qerr << "no AST tree could be generated" << endl;
            result = false;
        } else {
            qout << "AST tree successfully generated" << endl;
            if (m_printAst) {
                DebugVisitorT debugVisitor(m_session.tokenStream(), m_session.contents());
                debugVisitor.visitStart(m_ast);
            }
            Php::PHPIncludeFileVisitor includeFileVisitor(m_session.tokenStream(), m_session.contents());
            includeFileVisitor.visitStart(m_ast, m_dependencies);
        }
        if (!m_session.problems().isEmpty()) {
            qout << endl << "problems encountered during parsing:" << endl;
            foreach(KDevelop::ProblemPointer p, m_session.problems()) {
                qout << p->description() << endl;
            }
        } else {
            qout << "no problems encountered during parsing" << endl;
        }
        return result;
    }

    void printToken(int token, const LexerT& lexer) const
    {
        int begin = lexer.tokenBegin();
        int end = lexer.tokenEnd();
        qout << m_session.contents().mid(begin, end - begin + 1).replace('\n', "\\n")
             << ' ' << TokenTextT(token) << endl;
    }

    SessionT m_session;
    const bool m_printAst;
    const bool m_printTokens;
    StartAstT* m_ast;
    QStringList m_dependencies;
};


typedef DebugLanguageParserHelper<Php::ParseSession, Php::TokenStream, Php::Parser::Token, Php::Lexer, Php::StartAst, Php::DebugVisitor, Php::tokenText> PhpParser;

class PHPImportPrivate {
public:
    PHPImportPrivate()
      : m_printAst(false),
        m_printTokens(false)
    {
        KDevelop::AutoTestShell::init();
        KDevelop::TestCore::initialize(KDevelop::Core::NoUi);

        KDevelop::DUChain::self()->disablePersistentStorage();
        KDevelop::CodeRepresentation::setDiskChangesForbidden(true);
    }

    ~PHPImportPrivate()
    {
        qDeleteAll(m_parsers);
        m_parsers.clear();
        KDevelop::TestCore::shutdown();
    }

    bool parseFile(const QStringList &files)
    {
        foreach(const QString &fileName, files) {
            PhpParser *parser = new PhpParser(m_printAst, m_printTokens);
            QFileInfo fi(fileName);
            parser->parseFile(fi.canonicalFilePath());
            m_parsers[fi.canonicalFilePath()] = parser;
            foreach(const QString dependency, parser->dependencies()) {
                QFileInfo di(dependency);
                QFileInfo ei(di.isAbsolute() ? dependency : fi.canonicalPath() + "/" + dependency);
                QString usePath = ei.canonicalFilePath();
                if (usePath.isEmpty()) {
                    uError() << "could not parse empty file path for dependency" << dependency;
                    continue;
                }
                if (!m_parsers.contains(usePath)) {
                    PhpParser *parser = new PhpParser(m_printAst, m_printTokens);
                    parser->parseFile(usePath);
                    m_parsers[usePath] = parser;
                }
            }
        }
        qDebug() << m_parsers;
        return true;
    }

    QStringList getParsedFiles(const QString &fileName)
    {
        QStringList files;
        QFileInfo fi(fileName);
        QString f = fi.canonicalFilePath();
        if (!m_parsers.contains(f))
            return files;
        files.append(f);
        foreach(const QString dependency, m_parsers[f]->dependencies()) {
            QFileInfo di(dependency);
            QFileInfo ei(di.isAbsolute() ? dependency : fi.canonicalPath() + "/" + dependency);
            QString path = ei.canonicalFilePath();
            if (m_parsers.contains(path))
                files.append(path);
        }
        return files;
    }

    bool m_printAst;
    bool m_printTokens;
    QMap<QString, PhpParser*> m_parsers;
};

/**
 * Constructor.
 */
PHPImport::PHPImport(CodeImpThread* thread)
  : ClassImport(thread),
    m_d(new PHPImportPrivate)
{
}

/**
 * Destructor.
 */
PHPImport::~PHPImport()
{
    delete m_d;
}

/**
 * Auxiliary method for recursively traversing the #include dependencies
 * in order to feed innermost includes to the model before dependent
 * includes.  It is important that includefiles are fed to the model
 * in proper order so that references between UML objects are created
 * properly.
 * @param fileName   the file to import
 */
void PHPImport::feedTheModel(const QString& fileName)
{
    foreach(const QString &file, m_d->getParsedFiles(fileName)) {
        PhpParser *p = m_d->m_parsers[file];
        Php::PHPImportVisitor visitor(p->tokenStream(), p->contents());
        if (p->ast()) {
            qDebug() << file;
            visitor.visitStart(p->ast());
        }
    }
}

/**
 * Implement abstract operation from ClassImport for PHP
 */
void PHPImport::initialize()
{
}

/**
 * Import a single file.
 * @param fileName  The file to import.
 */
bool PHPImport::parseFile(const QString& fileName)
{
    bool result = false;
    QFileInfo fi(fileName);
    QString f = fi.canonicalFilePath();
    if (!m_d->m_parsers.contains(f))
        result  = m_d->parseFile(QStringList() << f);
    if (!result)
        return false;
    feedTheModel(f);
    return true;
}
