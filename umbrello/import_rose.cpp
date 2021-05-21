/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "import_rose.h"

// app includes
#include "uml.h"
#include "umldoc.h"
#include "folder.h"
#include "debug_utils.h"
#include "import_utils.h"
#include "petalnode.h"
#include "petaltree2uml.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QMessageBox>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QTextStream>

namespace Import_Rose {

/**
 * Directory prefix of .mdl file is buffered for possibly finding .cat/.sub
 * controlled units (if no path is given at their definition.)
 */
QString dirPrefix;

/**
 * Set language if encountered in file.
 * The last language encountered wins.
 */
Uml::ProgrammingLanguage::Enum progLang = Uml::ProgrammingLanguage::Reserved;

uint nClosures; // Multiple closing parentheses may appear on a single
                // line. The parsing is done line-by-line and using
                // recursive descent. This means that we can only handle
                // _one_ closing parenthesis at a time, i.e. the closing
                // of the currently parsed node. Since we may see more
                // closing parentheses than we can handle, we need a
                // counter indicating how many additional node closings
                // have been seen.

uint linum;  // line number
QString g_methodName;

void methodName(const QString& m)
{
    g_methodName = m;
}

QString mdlPath()
{
    return dirPrefix;
}

/**
 * Auxiliary function for diagnostics: Return current location.
 */
QString loc()
{
    return QLatin1String("Import_Rose::") + g_methodName +
           QLatin1String(" line ") + QString::number(linum) + QLatin1String(": ");
}

/**
 * Split a line into lexemes.
 */
QStringList scan(const QString& lin)
{
    QStringList result;
    QString line = lin.trimmed();
    if (line.isEmpty())
        return result;  // empty
    QString lexeme;
    const uint len = line.length();
    bool inString = false;
    for (uint i = 0; i < len; ++i) {
        QChar c = line[i];
        if (c == QLatin1Char('"')) {
            lexeme += c;
            if (inString) {
                result.append(lexeme);
                lexeme.clear();
            }
            inString = !inString;
        } else if (inString ||
                   c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('@')) {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                result.append(lexeme);
                lexeme.clear();
            }
            if (! c.isSpace()) {
                result.append(QString(c));
            }
        }
    }
    if (!lexeme.isEmpty())
        result.append(lexeme);
    return result;
}

/**
 * Emulate perl shift().
 */
QString shift(QStringList& l)
{
    QString first = l.first();
    l.pop_front();
    return first;
}

/**
 * Check for closing of one or more scopes.
 */
bool checkClosing(QStringList& tokens)
{
    if (tokens.count() == 0)
        return false;
    if (tokens.last() == QLatin1String(")")) {
        // For a single closing parenthesis, we just return true.
        // But if there are more closing parentheses, we need to increment
        // nClosures for each scope.
        tokens.pop_back();
        while (tokens.count() && tokens.last() == QLatin1String(")")) {
            nClosures++;
            tokens.pop_back();
        }
        return true;
    }
    return false;
}

/**
 * Immediate values are numbers or quoted strings.
 * @return  True if the given text is a natural or negative number
 *          or a quoted string.
 */
bool isImmediateValue(QString s)
{
    return s.contains(QRegExp(QLatin1String("^[\\d\\-\"]")));
}

/**
 * Extract immediate values out of `l'.
 * Examples of immediate value lists:
 *   number list:     (123, 456)
 *   string list:     ("SomeText" 888)
 * Any enclosing parentheses are removed.
 * All extracted items are also removed from `l'.
 * For the example given above the following is returned:
 *   "123 456"
 * or
 *   "\"SomeText\" 888"
 */
QString extractImmediateValues(QStringList& l)
{
    if (l.count() == 0)
        return QString();
    if (l.first() == QLatin1String("("))
        l.pop_front();
    QString result;
    bool start = true;
    while (l.count() && isImmediateValue(l.first())) {
        if (start)
            start = false;
        else
            result += QLatin1Char(' ');
        result += shift(l);
        if (l.first() == QLatin1String(","))
            l.pop_front();
    }
    if (l.first() == QLatin1String(")"))
        l.pop_front();
    while (l.count() && l.first() == QLatin1String(")")) {
        nClosures++;
        l.pop_front();
    }
    return result;
}

QString collectVerbatimText(QTextStream& stream)
{
    QString result;
    QString line;
    methodName(QLatin1String("collectVerbatimText"));
    while (!(line = stream.readLine()).isNull()) {
        linum++;
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char(')')))
            break;
        if (line[0] != QLatin1Char('|')) {
            uError() << loc() << "expecting '|' at start of verbatim text";
            return QString();
        } else {
            result += line.mid(1) + QLatin1Char('\n');
        }
    }
    if (line.isNull()) {
        uError() << loc() << "premature EOF";
        return QString();
    }
    if (! line.isEmpty()) {
        for (int i = 0; i < line.length(); ++i) {
            const QChar& clParenth = line[i];
            if (clParenth != QLatin1Char(')')) {
                uError() << loc() << "expected ')', found: " << clParenth;
                return QString();
            }
            nClosures++;
        }
    }
    return result;
}

/**
 * Extract the stripped down value from a (value ...) element.
 * Example: for the input
 *            (value Text "This is some text")
 *          the following is extracted:
 *            "This is some text"
 * Extracted elements and syntactic sugar of the value element are
 * removed from the input list.
 * The stream is passed into this method because it may be necessary
 * to read new lines - in the case of verbatim text.
 * The format of verbatim text in petal files is as follows:
 *
 *         (value Text
 * |This is the first line of verbatim text.
 * |This is another line of verbatim text.
 *        )
 * (The '|' character is supposed to be in the first column of the line)
 * In this case the two lines are extracted without the leading '|'.
 * The line ending '\n' of each line is preserved.
 */
QString extractValue(QStringList& l, QTextStream& stream)
{
    methodName(QLatin1String("extractValue"));
    if (l.count() == 0)
        return QString();
    if (l.first() == QLatin1String("("))
        l.pop_front();
    if (l.first() != QLatin1String("value"))
        return QString();
    l.pop_front();  // remove "value"
    l.pop_front();  // remove the value type: could be e.g. "Text" or "cardinality"
    QString result;
    if (l.count() == 0) {  // expect verbatim text to follow on subsequent lines
        QString text = collectVerbatimText(stream);
        nClosures--;  // expect own closure
        return text;
    } else {
        result = shift(l);
        if (l.first() != QLatin1String(")")) {
            uError() << loc() << "expecting closing parenthesis";
            return result;
        }
        l.pop_front();
    }
    while (l.count() && l.first() == QLatin1String(")")) {
        nClosures++;
        l.pop_front();
    }
    return result;
}

/**
 * Read attributes of a node.
 * @param initialArgs  Tokens on the line of the opening "(" of the node
 *                   but with leading whitespace and the opening "(" removed.
 * @param stream     The QTextStream from which to read following lines.
 * @return           Pointer to the created PetalNode or NULL on error.
 */
PetalNode *readAttributes(QStringList initialArgs, QTextStream& stream)
{
    methodName(QLatin1String("readAttributes"));
    if (initialArgs.count() == 0) {
        uError() << loc() << "initialArgs is empty";
        return 0;
    }
    PetalNode::NodeType nt;
    QString type = shift(initialArgs);
    if (type == QLatin1String("object"))
        nt = PetalNode::nt_object;
    else if (type == QLatin1String("list"))
        nt = PetalNode::nt_list;
    else {
        uError() << loc() << "unknown node type " << type;
        return 0;
    }
    PetalNode *node = new PetalNode(nt);
    bool seenClosing = checkClosing(initialArgs);
    node->setInitialArgs(initialArgs);
    if (seenClosing)
        return node;
    PetalNode::NameValueList attrs;
    QString line;
    while (!(line = stream.readLine()).isNull()) {
        linum++;
        line = line.trimmed();
        if (line.isEmpty())
            continue;
        QStringList tokens = scan(line);
        QString stringOrNodeOpener = shift(tokens);
        QString name;
        if (nt == PetalNode::nt_object && !stringOrNodeOpener.contains(QRegExp(QLatin1String("^[A-Za-z]")))) {
            uError() << loc() << "unexpected line " << line;
            delete node;
            return 0;
        }
        PetalNode::StringOrNode value;
        if (nt == PetalNode::nt_object) {
            name = stringOrNodeOpener;
            if (tokens.count() == 0) {  // expect verbatim text to follow on subsequent lines
                value.string = collectVerbatimText(stream);
                PetalNode::NameValue attr(name, value);
                attrs.append(attr);
                if (nClosures) {
                    // Decrement nClosures exactly once, namely for the own scope.
                    // Each recursion of readAttributes() is only responsible for
                    // its own scope. I.e. each further scope closing is handled by
                    // an outer recursion in case of multiple closing parentheses.
                    nClosures--;
                    break;
                }
                continue;
            }
            stringOrNodeOpener = shift(tokens);
        } else if (stringOrNodeOpener != QLatin1String("(")) {
            value.string = stringOrNodeOpener;
            PetalNode::NameValue attr;
            attr.second = value;
            attrs.append(attr);
            if (tokens.count() && tokens.first() != QLatin1String(")")) {
                uDebug() << loc()
                    << "NYI - immediate list entry with more than one item";
            }
            if (checkClosing(tokens))
                break;
            continue;
        }
        if (stringOrNodeOpener == QLatin1String("(")) {
            QString nxt = tokens.first();
            if (isImmediateValue(nxt)) {
                value.string = extractImmediateValues(tokens);
            } else if (nxt == QLatin1String("value") || nxt.startsWith(QLatin1Char('"'))) {
                value.string = extractValue(tokens, stream);
            } else {
                value.node = readAttributes(tokens, stream);
                if (value.node == 0) {
                    delete node;
                    return 0;
                }
            }
            PetalNode::NameValue attr(name, value);
            attrs.append(attr);
            if (nClosures) {
                // Decrement nClosures exactly once, namely for the own scope.
                // Each recursion of readAttributes() is only responsible for
                // its own scope. I.e. each further scope closing is handled by
                // an outer recursion in case of multiple closing parentheses.
                nClosures--;
                break;
            }
        } else {
            value.string = stringOrNodeOpener;
            bool seenClosing = checkClosing(tokens);
            PetalNode::NameValue attr(name, value);
            attrs.append(attr);
            if (name == QLatin1String("language")) {
                QString language(value.string);
                language.remove(QLatin1Char('\"'));
                if (language == QLatin1String("Analysis"))
                    progLang = Uml::ProgrammingLanguage::Reserved;
                else if (language == QLatin1String("CORBA"))
                    progLang = Uml::ProgrammingLanguage::IDL;
                else if (language == QLatin1String("C++") || language == QLatin1String("VC++"))
                    progLang = Uml::ProgrammingLanguage::Cpp;
                else if (language == QLatin1String("Java"))
                    progLang = Uml::ProgrammingLanguage::Java;
                else if (language == QLatin1String("Ada"))
                    progLang = Uml::ProgrammingLanguage::Ada;
            }
            if (seenClosing) {
                break;
            }
        }
    }
    node->setAttributes(attrs);
    return node;
}

#define SETCODEC(str)  stream.setCodec(str); break

/**
 * Parse a file into the PetalNode internal tree representation
 * and then create Umbrello objects by traversing the tree.
 *
 * @return  In case of error: NULL
 *          In case of success with non NULL parentPkg: pointer to UMLPackage created for controlled unit
 *          In case of success with NULL parentPkg: pointer to root folder of Logical View
 */
UMLPackage* loadFromMDL(QFile& file, UMLPackage *parentPkg /* = 0 */) 
{
    if (parentPkg == 0) {
        QString fName = file.fileName();
        int lastSlash = fName.lastIndexOf(QLatin1Char('/'));
        if (lastSlash > 0) {
            dirPrefix = fName.left(lastSlash + 1);
        }
    }
    QTextStream stream(&file);
    stream.setCodec("ISO 8859-1");
    QString line;
    PetalNode *root = 0;
    uint nClosures_sav = nClosures;
    uint linum_sav = linum;
    nClosures = 0;
    linum = 0;
    while (!(line = stream.readLine()).isNull()) {
        linum++;
        if (line.contains(QRegExp(QLatin1String("^\\s*\\(object Petal")))) {
            bool finish = false;
            // Nested loop determines character set to use
            while (!(line = stream.readLine()).isNull()) {
                linum++; // CHECK: do we need petal version info?
                if (line.contains(QLatin1Char(')'))) {
                    finish = true;
                    line = line.replace(QLatin1String(QLatin1String(")")), QString());
                }
                QStringList a = line.trimmed().split(QRegExp(QLatin1String("\\s+")));
                if (a.size() == 2 && a[0] == QLatin1String("charSet")) {
                    const QString& charSet = a[1];
                    if (!charSet.contains(QRegExp(QLatin1String("^\\d+$")))) {
                        uWarning() << "Unimplemented charSet " << charSet;
                        if (finish)
                            break;
                        continue;
                    }
                    const int charSetNum = charSet.toInt();
                    switch (charSetNum) {
                        case 0:         // ASCII
                            ;
                        case 1:    // Default
                            SETCODEC("System");
                        case 2:    // Symbol
                            ; // @todo     SETCODEC("what");
                        case 77:   // Mac
                            SETCODEC("macintosh");
                        case 128:  // ShiftJIS (Japanese)
                            SETCODEC("Shift_JIS");
                        case 129:  // Hangul (Korean)
                            SETCODEC("EUC-KR");
                        case 130:  // Johab (Korean)
                            SETCODEC("EUC-KR");
                        case 134:  // GB2312 (Chinese)
                            SETCODEC("GB18030");  // "Don't use GB2312 here" (Ralf H.)
                        case 136:  // ChineseBig5
                            SETCODEC("Big5");
                        case 161:  // Greek
                            SETCODEC("windows-1253");
                        case 162:  // Turkish
                            SETCODEC("windows-1254");
                        case 163:  // Vietnamese
                            SETCODEC("windows-1258");
                        case 177:  // Hebrew
                            SETCODEC("windows-1255");
                        case 178:  // Arabic
                            SETCODEC("windows-1256");
                        case 186:  // Baltic
                            SETCODEC("windows-1257");
                        case 204:  // Russian
                            SETCODEC("windows-1251");
                        case 222:  // Thai
                            SETCODEC("TIS-620");
                        case 238:  // EastEurope
                            SETCODEC("windows-1250");
                        case 255:  // OEM (extended ASCII)
                            SETCODEC("windows-1252");
                        default:
                            uWarning() << "Unimplemented charSet number" << charSetNum;
                    }
                }
                if (finish)
                     break;
            }
            if (line.isNull())
                break;
        } else {
            QRegExp objectRx(QLatin1String("^\\s*\\(object "));
            if (line.contains(objectRx)) {
                nClosures = 0;
                QStringList initialArgs = scan(line);
                initialArgs.pop_front();  // remove opening parenthesis
                root = readAttributes(initialArgs, stream);
                break;
            }
        }
    }
    file.close();
    nClosures = nClosures_sav;
    linum = linum_sav;
    if (root == 0)
        return 0;

    if (progLang == Uml::ProgrammingLanguage::Reserved &&
                    UMLApp::app()->generator() == nullptr) {
        UMLApp::app()->setGenerator(Uml::ProgrammingLanguage::Cpp);
    } else if (progLang != UMLApp::app()->activeLanguage()) {
        uDebug() << "loadFromMDL: Setting active language to "
                 << Uml::ProgrammingLanguage::toString(progLang);
        UMLApp::app()->setGenerator(progLang);
    }

    if (parentPkg) {
        UMLPackage *child = petalTree2Uml(root, parentPkg);
        delete root;
        return child;
    }

    if (root->name() != QLatin1String("Design")) {
        uError() << "expecting root name Design";
        delete root;
        return 0;
    }
    Import_Utils::assignUniqueIdOnCreation(false);
    UMLDoc *umldoc = UMLApp::app()->document();

    //*************************** import Logical View *********************************
    umldoc->setCurrentRoot(Uml::ModelType::Logical);
    UMLPackage *logicalView = umldoc->rootFolder(Uml::ModelType::Logical);
    importView(root, logicalView,
               QLatin1String("root_category"), QLatin1String("logical_models"),
               QLatin1String("Class_Category"), QLatin1String("logical_presentations"));

    //*************************** import Use Case View ********************************
    umldoc->setCurrentRoot(Uml::ModelType::UseCase);
    UMLPackage *useCaseView = umldoc->rootFolder(Uml::ModelType::UseCase);
    importView(root, useCaseView,
               QLatin1String("root_usecase_package"), QLatin1String("logical_models"),
               QLatin1String("Class_Category"), QLatin1String("logical_presentations"));

    //*************************** import Component View *******************************
    umldoc->setCurrentRoot(Uml::ModelType::Component);
    UMLPackage *componentView = umldoc->rootFolder(Uml::ModelType::Component);
    importView(root, componentView,
               QLatin1String("root_subsystem"), QLatin1String("physical_models"),
               QLatin1String("SubSystem"), QLatin1String("physical_presentations"));

    //*************************** import Deployment View ******************************
    umldoc->setCurrentRoot(Uml::ModelType::Deployment);
    UMLPackage *deploymentView = umldoc->rootFolder(Uml::ModelType::Deployment);
    importView(root, deploymentView, QLatin1String("process_structure"),
                                     QLatin1String("ProcsNDevs"), QLatin1String("Processes"));

    //***************************       wrap up        ********************************
    delete root;
    umldoc->setCurrentRoot(Uml::ModelType::Logical);
    Import_Utils::assignUniqueIdOnCreation(true);
    umldoc->resolveTypes();
    return logicalView;
}

#undef SETCODEC

}

