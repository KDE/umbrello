/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "import_rose.h"

// qt includes
#include <qstring.h>
#include <qtextstream.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
// app includes
#include "petalnode.h"

namespace Import_Rose {

typedef QPtrList<PetalNode> PetalNodeList;

uint nClosures; // Multiple closing parentheses may appear on a single
                // line. The parsing is done line-by-line and using
                // recursive descent. This means that we can only handle
                // _one_ closing parenthesis at a time, i.e. the closing
                // of the currently parsed node. Since we may see more
                // closing parentheses than we can handle, we need a
                // counter indicating how many additional node closings
                // have been seen.

QString comment;  // comment buffer

uint linum;  // line number

/**
 * Split a line into lexemes.
 */
QStringList scan(QString line) {
    QStringList result;
    line = line.stripWhiteSpace();
    if (line.isEmpty())
        return result;  // empty
    QString lexeme;
    const uint len = line.length();
    bool inString = false;
    for (uint i = 0; i < len; i++) {
        QChar c = line[i];
        if (c == '"') {
            lexeme += c;
            if (inString) {
                result.append(lexeme);
                lexeme = QString::null;
            }
            inString = !inString;
        } else if (inString ||
                   c.isLetterOrNumber() || c == '_' || c == '@') {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                result.append(lexeme);
                lexeme = QString::null;
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
QString shift(QStringList& l) {
    QString first = l.first();
    l.pop_front();
    return first;
}

/**
 * Check for closing of one or more scopes.
 */
bool checkClosing(QStringList& tokens) {
    if (tokens.count() == 0)
        return false;
    if (tokens.last() == ")") {
        // For a single closing parenthesis, we just return true.
        // But if there are more closing parentheses, we need to set the
        // seenClosure for each corresponding scope to true.
        tokens.pop_back();
        while (tokens.count() && tokens.last() == ")") {
            nClosures++;
            tokens.pop_back();
        }
        /*
        if (tokens.last() == ")") {
            kdError() << "Import_Rose::checkClosing:" << linum
                << " too many ')' in file " << endl;
        }  */
        return true;
    }
    return false;
}

/**
 * Immediate values are numbers or quoted strings.
 * @return  True if the given text is a natural or negative number
 *          or a quoted string.
 */
bool isImmediateValue(QString s) {
    return s.contains(QRegExp("^[\\d\\-\"]"));
}

/**
 * Extract immediate values out of `l'.
 * Examples of immediate value lists:
 *   number list:     ( 123 , 456 )
 *   string list:     ( "SomeText" 888 )
 * Any enclosing parentheses are removed.
 * All extracted items are also removed from `l'.
 * For the example given above the following is returned:
 *   "123 456"
 * or
 *   "\"SomeText\" 888"
 */
QString extractImmediateValues(QStringList& l) {
    if (l.count() == 0)
        return QString::null;
    if (l.first() == "(")
        l.pop_front();
    QString result;
    bool start = true;
    while (l.count() && isImmediateValue(l.first())) {
        if (start)
            start = false;
        else
            result += " ";
        result += shift(l);
        if (l.first() == ",")
            l.pop_front();
    }
    if (l.first() == ")")
        l.pop_front();
    while (l.count() && l.first() == ")") {
        nClosures++;
        l.pop_front();
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
 *         )
 * (The '|' character is supposed to be in the first column of the file)
 * In this case the two lines are extracted without the leading '|'.
 * The line ending '\n' of each line is preserved.
 */
QString extractValue(QStringList& l, QTextStream& stream) {
    if (l.count() == 0)
        return QString::null;
    if (l.first() == "(")
        l.pop_front();
    if (l.first() != "value")
        return QString::null;
    l.pop_front();  // remove "value"
    l.pop_front();  // remove the value type: could be e.g. "Text" or "cardinality"
    QString result;
    if (l.count() == 0) {  // expect verbatim text to follow on subsequent lines
        const QRegExp closingParenth("^\\s*\\)");
        QString line;
        while ((line = stream.readLine()) != QString::null) {
            linum++;
            if (line.isEmpty())
                continue;
            if (line.contains(closingParenth))
                break;
            if (line[0] != '|') {
                kdError() << "Import_Rose::extractValue " << linum
                    << ": expecting '|' at start of verbatim text" << endl;
                break;
            } else {
                result += line.mid(1) + "\n";
            }
        }
        if (line == QString::null)
            return QString::null;  // error - premature EOF
        line.remove(closingParenth);
        if (! line.isEmpty()) {
            l = QStringList::split(QRegExp("\\b"), line);
        }
    } else {
        result = shift(l);
        if (l.first() != ")") {
            kdError() << "Import_Rose::extractValue " << linum
                 << ": expecting closing parenthesis" << endl;
            return result;
        }
        l.pop_front();
    }
    while (l.count() && l.first() == ")") {
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
PetalNode *readAttributes(QStringList initialArgs, QTextStream& stream) {
    if (initialArgs.count() == 0) {
        kdError() << "Import_Rose::readAttributes " << linum
            << ": firstLine is empty" << endl;
        return false;
    }
    PetalNode::NodeType nt;
    QString type = shift(initialArgs);
    if (type == "object")
        nt = PetalNode::nt_object;
    else if (type == "list")
        nt = PetalNode::nt_list;
    else {
        kdError() << "Import_Rose::readAttributes " << linum
            << ": unknown node type " << type << endl;
        return NULL;
    }
    PetalNode *node = new PetalNode(nt);
    bool seenClosing = checkClosing(initialArgs);
    node->setInitialArgs(initialArgs);
    if (seenClosing)
        return node;
    PetalNode::NameValueList attrs;
    QString line;
    while ((line = stream.readLine()) != QString::null) {
        linum++;
        line = line.simplifyWhiteSpace();
        if (line.isEmpty())
            continue;
        QStringList tokens = scan(line);
        if (tokens.count() == 0)
            return NULL;
        QString stringOrNodeOpener = shift(tokens);
        QString name;
        if (nt == PetalNode::nt_object && !stringOrNodeOpener.contains(QRegExp("^[A-Za-z]")) ||
            nt == PetalNode::nt_list && stringOrNodeOpener != "(") {
            kdError() << "Import_Rose::readAttributes " << linum
                << ": unexpected line " << line << endl;
            return NULL;
        }
        if (nt == PetalNode::nt_object) {
            name = stringOrNodeOpener;
            stringOrNodeOpener = shift(tokens);
        }
        PetalNode::StringOrNode value;
        if (stringOrNodeOpener == "(") {
            if (isImmediateValue(tokens.first())) {
                value.string = extractImmediateValues(tokens);
            } else if (tokens.first() == "value" || tokens.first() == "\"") {
                value.string = extractValue(tokens, stream);
            } else {
                value.node = readAttributes(tokens, stream);
                if (value.node == NULL)
                    return NULL;
            }
            PetalNode::NameValue attr(name, value);
            attrs.append(attr);
            if (nClosures) {  // && value.node
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
            if (seenClosing) {
                break;
            }
        }
    }
    node->setAttributes(attrs);
    return node;
}

bool loadFromMDL(QIODevice& file) {
    QTextStream stream(&file);
    QString line;
    PetalNode *root = NULL;
    while ((line = stream.readLine()) != QString::null) {
        linum++;
        if (line.contains( QRegExp("^\\s*\\(object Petal") )) {
            while ((line = stream.readLine()) != QString::null && !line.contains(')')) {
                linum++; // CHECK: do we need petal version info?
            }
            if (line == QString::null)
                return false;
        } else {
            QRegExp objectRx("^\\s*\\(object ");
            if (line.contains(objectRx)) {
                nClosures = 0;
                QStringList initialArgs = scan(line);
                initialArgs.pop_front();  // remove opening parenthesis
                root = readAttributes(initialArgs, stream);
            }
        }
    }
    file.close();
    if (root -= NULL)
        return false;
    // @todo traverse the PetalNode tree and create Umbrello model objects
    //
    return true;
}

}

