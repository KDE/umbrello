/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2017-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "phpimport.h"

// app includes
#include "artifact.h"
#include "association.h"
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "enum.h"
#include "import_utils.h"
#include "object_factory.h"
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

// kdevphp
#include <parser/parsesession.h>
#include <parser/phplexer.h>
#include <parser/phpparser.h>
#include <parser/phpdebugvisitor.h>
#include <parser/phpast.h>
#include <parser/tokenstream.h>
#include <parser/phptokentext.h>

// kdevplatform
#include <tests/autotestshell.h>
#include <language/duchain/duchain.h>
#include <language/duchain/problem.h>
#include <language/codegen/coderepresentation.h>
#include <language/editor/documentrange.h>
#include <tests/testcore.h>

namespace Php {

typedef QMap<QString, QString> VariableMapping;

class PHPIncludeFileVisitor : public DefaultVisitor
{
public:
    PHPIncludeFileVisitor(TokenStream *str, const QString& content = QString())
      : m_str (str),
        m_content(content),
        m_indent(0),
        m_dependencies(0)
    {}
    void setFilePath(const QString &path)
    {
        m_filePath = path;
    }

    void setVariableMapping(VariableMapping &map)
    {
        m_map = map;
    }

    void setDependencies(QStringList &dependencies)
    {
        m_dependencies = &dependencies;
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
            else if (tokenString.startsWith("("))
                tokenString = tokenString.mid(1, tokenString.size() - 2);

            const QString search = "dirname(__FILE__).";
            if (tokenString.contains(search)) {
                tokenString.replace(search, m_filePath);
                tokenString.replace("\"", "");
                tokenString.replace("\'", "");
            } else if(tokenString.startsWith("$")) {
                int i = tokenString.indexOf('.');
                QString key = tokenString.mid(1, i-1);
                if (m_map.contains(key)) {
                    tokenString.replace("$" + key + ".", m_map[key]);
                    tokenString.replace("\"", "");
                    tokenString.replace("\'", "");
                }
            } // generate parse error for unknown variables
            qDebug() << "-------------------include ----- " << tokenString;
            if (!m_dependencies->contains(tokenString))
                m_dependencies->append(tokenString);
        }
    }

    TokenStream *m_str;
    QString m_content;
    int m_indent;
    QStringList *m_dependencies;
    QString m_filePath;
    VariableMapping m_map;
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
        Import_Utils::createUMLObject(UMLObject::ot_Datatype, "auto", 0);
    }

    void setFileName(const QString &fileName)
    {
        m_fileName = fileName;
    }

    QString tokenValue(AstNode *node)
    {
        TokenStream::Token startToken = m_str->at(node->startToken);
        TokenStream::Token endToken = m_str->at(node->endToken);
        int begin = startToken.begin;
        int end = endToken.end;
        return m_content.mid(begin, end-begin+1);
    }

    QString tokenValue(const KDevPG::ListNode<Php::NamespacedIdentifierAst*> *node)
    {
        QStringList names;
        const KDevPG::ListNode<NamespacedIdentifierAst*> *__it = node->front(), *__end = __it;
        do
        {
            names.append(tokenValue(__it->element));
            __it = __it->next;
        }
        while (__it != __end);
        return names.join("::");
    }

    void visitStart(StartAst *node)
    {
        if (Settings::optionState().codeImportState.createArtifacts) {
            QFileInfo fi(m_fileName);
            UMLObject *o = Import_Utils::createArtifactFolder(fi.canonicalPath(), 0, QString());
            UMLPackage *p = o->asUMLPackage();
            QString fileName = fi.fileName();
            o = UMLApp::app()->document()->findUMLObject(fileName, UMLObject::ot_Artifact, p);
            if (!o)
                o = Object_Factory::createNewUMLObject(UMLObject::ot_Artifact, fileName, p, true);
            UMLArtifact *a = o->asUMLArtifact();
            if (a)
                a->setDrawAsType(UMLArtifact::file);
            else
                uError() << "could not add artifact" << m_fileName;
            //a->setDoc(comment);
        }
        DefaultVisitor::visitStart(node);
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
        UMLPackage *parent = 0;
        QString names = nsNames.join("::");
        UMLObject *o = UMLApp::app()->document()->findUMLObject(names, UMLObject::ot_Class, parent);
        if (!o)
            o = Import_Utils::createUMLObject(UMLObject::ot_Class, names, parent);
        if (o) {
            m_usingClasses.append(o->asUMLClassifier());
            uDebug() << "using class" << names;
        }
        DefaultVisitor::visitUseNamespace(node);
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
        QString returnType = "auto";
        bool isStatic = false;
        bool isAbstract = false;
        bool isFriend = false;
        bool isConstructor = false;
        bool isDestructor = false;
        QString m_comment;
        UMLPackage *parentPackage = m_currentNamespace[m_nsCnt];
        UMLClassifier *c = parentPackage->asUMLClassifier();
        UMLOperation *m = 0;
        if (c) {
            m = Import_Utils::makeOperation(c, methodName);
        } else {
            uError() << "no parent class found for method" << methodName;
        }
        if (m) {
            if (node->parameters && node->parameters->parametersSequence) {
                const KDevPG::ListNode<ParameterAst*> *__it = node->parameters->parametersSequence->front(), *__end = __it;
                do {
                    QString type = "auto";
                    QString name = tokenValue(__it->element->variable).mid(1);
                    Import_Utils::addMethodParameter(m, type, name);
                    __it = __it->next;
                } while (__it != __end);
            }
            Import_Utils::insertMethod(c, m, m_currentAccess, returnType,
                                       isStatic, isAbstract, isFriend, isConstructor,
                                       isDestructor, m_comment);
        }
        DefaultVisitor::visitClassStatement(node);
    }

    void visitClassExtends(ClassExtendsAst *node)
    {
        if (node->identifier) {
            QString baseName = tokenValue(node->identifier);
            UMLClassifier *a = m_currentNamespace[m_nsCnt]->asUMLClassifier();
            foreach(UMLObject *uc, m_usingClasses) {
                if (uc->name() == baseName) {
                    Import_Utils::createGeneralization(a, uc->asUMLClassifier());
                }
            }
        }
        DefaultVisitor::visitClassExtends(node);
    }

    void visitClassImplements(ClassImplementsAst *node)
    {
        if (node->implementsSequence) {
            QString baseName = tokenValue(node->implementsSequence);
            UMLClassifier *a = m_currentNamespace[m_nsCnt]->asUMLClassifier();
            bool found = false;
            foreach(UMLObject *uc, m_usingClasses) {
                if (uc->name() == baseName) {
                    Import_Utils::createGeneralization(a, uc->asUMLClassifier());
                    found = true;
                }
            }
            if (!found) {
                UMLObject *o = UMLApp::app()->document()->findUMLObject(baseName, UMLObject::ot_Interface,
                                                                        m_currentNamespace[m_nsCnt-1]);
                if (!o)
                    o = Import_Utils::createUMLObject(UMLObject::ot_Interface, baseName,
                                                      m_currentNamespace[m_nsCnt-1],
                                                      QString()/*ast->comment()*/, QString(), true);
                Import_Utils::createGeneralization(a, o->asUMLClassifier());
            }
        }
        DefaultVisitor::visitClassImplements(node);
    }

    void visitInterfaceDeclarationStatement(InterfaceDeclarationStatementAst *node)
    {
        QString interfaceName = tokenValue(node->interfaceName);
        UMLObject *o = UMLApp::app()->document()->findUMLObject(interfaceName, UMLObject::ot_Interface, m_currentNamespace[m_nsCnt]);
        if (!o)
            o = Import_Utils::createUMLObject(UMLObject::ot_Interface, interfaceName,
                                              m_currentNamespace[m_nsCnt],
                                              QString()/*ast->comment()*/, QString(), true);
        m_currentScope.push_back(interfaceName);
        if (++m_nsCnt > NamespaceSize) {
            uError() << "excessive namespace nesting";
            m_nsCnt = NamespaceSize;
        }
        UMLPackage *ns = o->asUMLPackage();
        m_currentNamespace[m_nsCnt] = ns;
        // handle modifier
        DefaultVisitor::visitInterfaceDeclarationStatement(node);
        --m_nsCnt;
        m_currentScope.pop_back();
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
        if (!node || !node->functionName)
            return;
        QString methodName = tokenValue(node->functionName);
        Uml::Visibility::Enum m_currentAccess = Uml::Visibility::Public;
        QString returnType = "auto";
        bool isStatic = false;
        bool isAbstract = false;
        bool isFriend = false;
        bool isConstructor = false;
        bool isDestructor = false;
        QString m_comment;
        UMLPackage *parentPackage = m_currentNamespace[m_nsCnt];
        UMLClassifier *c = parentPackage->asUMLClassifier();
        UMLOperation *m = 0;
        if (c) {
            m = Import_Utils::makeOperation(c, methodName);
        }else {
            uError() << "no parent class found for method" << methodName;
        }

        if (m) {
            if (node->parameters && node->parameters->parametersSequence) {
                const KDevPG::ListNode<ParameterAst*> *__it = node->parameters->parametersSequence->front(), *__end = __it;
                do {
                    QString type = "auto";
                    QString name = tokenValue(__it->element->variable).mid(1);
                    Import_Utils::addMethodParameter(m, type, name);
                    __it = __it->next;
                } while (__it != __end);
            }
            Import_Utils::insertMethod(c, m, m_currentAccess, returnType,
                                       isStatic, isAbstract, isFriend, isConstructor,
                                       isDestructor, m_comment);
        }
        DefaultVisitor::visitFunctionDeclarationStatement(node);
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
    QList<QPointer<UMLClassifier>> m_usingClasses;
    QStringList m_currentScope;
    QString m_fileName;
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
        : m_printAst(printAst),
          m_printTokens(printTokens),
          m_ast(0),
          m_isFed(false)
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
        return runSession(fileName);
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

    void setFed(bool state)
    {
        m_isFed = state;
    }

    bool wasFed()
    {
        return m_isFed;
    }

private:
    /**
     * actually run the parse session
     */
    bool runSession(const QString &fileName=QString())
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
            Php::VariableMapping map;
            QString filePath = QFileInfo(fileName).canonicalPath();
            map["afw_root"] = filePath;
            includeFileVisitor.setFilePath(filePath);
            includeFileVisitor.setDependencies(m_dependencies);
            includeFileVisitor.setVariableMapping(map);
            includeFileVisitor.visitStart(m_ast);
        }
        if (!m_session.problems().isEmpty()) {
            qout << endl << "problems encountered during parsing:" << endl;
            foreach(KDevelop::ProblemPointer p, m_session.problems()) {
                QString item = QString::fromLatin1("%1:%2:%3: %4: %5")
#if QT_VERSION > 0x050000
                        .arg(fileName).arg(p->finalLocation().start().line()+1)
                        .arg(p->finalLocation().start().column())
#else
                        .arg(fileName).arg(p->finalLocation().start.line+1)
                        .arg(p->finalLocation().start.column)
#endif
                        .arg(p->severityString()).arg(p->description());
                UMLApp::app()->logWindow()->addItem(item);
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
    bool m_isFed;
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
        qDebug() << m_parsers.size();
        //qDebug() << m_parsers;
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
        visitor.setFileName(file);
        if (p->ast() && !p->wasFed()) {
            uDebug() << "feeding" << file;
            visitor.visitStart(p->ast());
            p->setFed(true);
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
 * Implement abstract operation from ClassImport for PHP
 */
void PHPImport::initPerFile()
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
