/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "rubywriter.h"

#include "association.h"
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "operation.h"
#include "umldoc.h"
#include "umlattributelist.h"
#include "uml.h"  // Only needed for log{Warn,Error}

#include <KLocalizedString>
#include <KMessageBox>

#include <QFile>
#include <QRegExp>
#include <QTextStream>

RubyWriter::RubyWriter()
{
}

RubyWriter::~RubyWriter()
{
}

/**
 * Call this method to generate C++ code for a UMLClassifier.
 * @param c   the class you want to generate code for.
 */
void RubyWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        logWarn0("RubyWriter::writeClass: Cannot write class of NULL concept");
        return;
    }

    UMLClassifierList superclasses = c->getSuperClasses();
    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();

    //find an appropriate name for our file
    fileName_ = findFileName(c, QLatin1String(".rb"));
    if (fileName_.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    QFile fileh;
    if (!openFile(fileh, fileName_)) {
        emit codeGenerated(c, false);
        return;
    }
    QTextStream h(&fileh);

    className_ = cleanName(c->name());

    //////////////////////////////
    //Start generating the code!!
    /////////////////////////////

    //try to find a heading file (license, comments, etc)
    QString str;

    str = getHeadingFile(QLatin1String(".rb"));
    if (!str.isEmpty()) {
        str.replace(QRegExp(QLatin1String("%filename%")), fileName_);
        str.replace(QRegExp(QLatin1String("%filepath%")), fileh.fileName());
        h << str << m_endl;
    }

    if (forceDoc() || !c->doc().isEmpty()) {
        QString docStr = c->doc();
        docStr.replace(QRegExp(QLatin1String("\\n")), QLatin1String("\n# "));
        docStr.remove(QLatin1String("@ref "));
        docStr.replace(QLatin1String("@see"), QLatin1String("_See_"));
        docStr.replace(QLatin1String("@short"), QLatin1String("_Summary_"));
        docStr.replace(QLatin1String("@author"), QLatin1String("_Author_"));
        h << "#" << m_endl;
        h << "# " << docStr << m_endl;
        h << "#" << m_endl << m_endl;
    }

    // write inheritances out
    UMLClassifier *concept;

    h <<  "class " << cppToRubyType(className_) << (superclasses.count() > 0 ? QLatin1String(" < ") : QString());

    int i = 0;
    foreach (concept, superclasses) {
        if (i == 0) {
            h << cppToRubyType(concept->name()) << m_endl;
        } else {
            // Assume ruby modules that can be mixed in, after the first
            // superclass name in the list
            h << m_indentation << "include " <<  cppToRubyType(concept->name()) << m_endl;
        }
        i++;
    }

    h << m_endl;

    // write comment for sub-section IF needed
    if (forceDoc() || c->hasAccessorMethods()) {
        h << m_indentation << "#" << m_endl;
        h << m_indentation << "# Accessor Methods" << m_endl;
        h << m_indentation << "#" << m_endl << m_endl;

        // Accessor methods for attributes
        writeAttributeMethods(c->getAttributeList(Uml::Visibility::Public), Uml::Visibility::Public, h);
        writeAttributeMethods(c->getAttributeList(Uml::Visibility::Protected), Uml::Visibility::Protected, h);
        writeAttributeMethods(c->getAttributeList(Uml::Visibility::Private), Uml::Visibility::Private, h);
        h << m_endl;
    }

    //operations
    writeOperations(c, h);

    //finish files
    h << "end" << m_endl << m_endl;

    //close files and notfiy we are done
    fileh.close();
    emit codeGenerated(c, true);
}

/**
 * Convert a C++ type such as 'int' or 'QWidget' to
 * ruby types Integer and Qt::Widget.
 * @param cppType the C++ type to be converted
 */
QString RubyWriter::cppToRubyType(const QString &typeStr)
{
    QString type = cleanName(typeStr);
    type.remove(QLatin1String("const "));
    type.remove(QRegExp(QLatin1String("[*&\\s]")));
    type.replace(QRegExp(QLatin1String("[<>]")), QLatin1String("_"));
    type.replace(QLatin1String("QStringList"), QLatin1String("Array"));
    type.replace(QLatin1String("QString"), QLatin1String("String"));
    type.replace(QLatin1String("bool"), QLatin1String("true|false"));
    type.replace(QRegExp(QLatin1String("^(uint|int|ushort|short|ulong|long)$")), QLatin1String("Integer"));
    type.replace(QRegExp(QLatin1String("^(float|double)$")), QLatin1String("Float"));
    type.replace(QRegExp(QLatin1String("^Q(?=[A-Z])")), QLatin1String("Qt::"));
    type.replace(QRegExp(QLatin1String("^K(?!(DE|Parts|IO)")), QLatin1String("KDE::"));

    return type;
}

/**
 * Convert C++ names such as 'm_foobar' or pFoobar to
 * just 'foobar' for ruby.
 * @param cppName the C++ name to be converted
 */
QString RubyWriter::cppToRubyName(const QString &nameStr)
{
    QString name = cleanName(nameStr);
    name.remove(QRegExp(QLatin1String("^m_")));
    name.remove(QRegExp(QLatin1String("^[pbn](?=[A-Z])")));
    name = name.mid(0, 1).toLower() + name.mid(1);
    return name;
}

/**
 * Write all operations for a given class.
 * @param c   the concept we are generating code for
 * @param h   output stream for the header file
 */
void RubyWriter::writeOperations(UMLClassifier *c, QTextStream &h)
{
    //Lists to store operations  sorted by scope
    UMLOperationList oppub, opprot, oppriv;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList opl(c->getOpList());
    foreach (UMLOperation *op, opl) {
        switch(op->visibility()) {
        case Uml::Visibility::Public:
            oppub.append(op);
            break;
        case Uml::Visibility::Protected:
            opprot.append(op);
            break;
        case Uml::Visibility::Private:
            oppriv.append(op);
            break;
        default:
            break;
        }
    }

    QString classname(cleanName(c->name()));

    //write operations to file
    if (forceSections() || !oppub.isEmpty()) {
        writeOperations(classname, oppub, Uml::Visibility::Public, h);
    }

    if (forceSections() || !opprot.isEmpty()) {
        writeOperations(classname, opprot, Uml::Visibility::Protected, h);
    }

    if (forceSections() || !oppriv.isEmpty()) {
        writeOperations(classname, oppriv, Uml::Visibility::Private, h);
    }
}

/**
 * Write a list of class operations.
 * @param classname   the name of the class
 * @param opList      the list of operations
 * @param permitScope the visibility enum
 * @param h           output stream for the header file
 */
void RubyWriter::writeOperations(const QString &classname, const UMLOperationList &opList,
                                 Uml::Visibility::Enum permitScope, QTextStream &h)
{
//    UMLOperation *op;
//    UMLAttribute *at;

    switch (permitScope) {
    case Uml::Visibility::Public:
        h << m_indentation << "public" << m_endl << m_endl;
        break;
    case Uml::Visibility::Protected:
        h << m_indentation << "protected" << m_endl << m_endl;
        break;
    case Uml::Visibility::Private:
        h << m_indentation << "private" << m_endl << m_endl;
        break;
    default:
        break;
    }

    foreach (const UMLOperation* op, opList) {
        QString methodName = cleanName(op->name());
        QStringList commentedParams;

        // Skip destructors, and operator methods which
        // can't be defined in ruby
        if (methodName.startsWith(QLatin1Char('~'))
                || methodName == QLatin1String("operator =")
                || methodName == QLatin1String("operator --")
                || methodName == QLatin1String("operator ++")
                || methodName == QLatin1String("operator !="))
        {
            continue;
        }

        if (methodName == classname) {
            methodName = QLatin1String("initialize");
        }

        methodName.remove(QLatin1String("operator "));
        methodName = methodName.mid(0, 1).toLower() + methodName.mid(1);

        UMLAttributeList atl = op->getParmList();
        // Always write out the docs for ruby as the type of the
        // arguments and return value of the methods is useful
        bool writeDoc = true;
//        for (UMLAttribute& at = atl.first(); at; at = atl.next())
//            writeDoc |= !at->getDoc().isEmpty();

        if (writeDoc) {
            h << m_indentation << "#" << m_endl;
            QString docStr = op->doc();

            docStr.replace(QRegExp(QLatin1String("[\\n\\r]+ *")), m_endl);
            docStr.replace(QRegExp(QLatin1String("[\\n\\r]+\\t*")), m_endl);

            docStr.replace(QLatin1String(" m_"), QLatin1String(" "));
            docStr.replace(QRegExp(QLatin1String("\\s[npb](?=[A-Z])")), QLatin1String(" "));
            QRegExp re_params(QLatin1String("@param (\\w)(\\w*)"));
            int pos = re_params.indexIn(docStr);
            while (pos != -1) {
                docStr.replace(re_params.cap(0),
                                QString(QLatin1String("@param _")) + re_params.cap(1).toLower() + re_params.cap(2) + QLatin1Char('_'));
                commentedParams.append(re_params.cap(1).toLower() + re_params.cap(2));

                pos += re_params.matchedLength() + 3;
                pos = re_params.indexIn(docStr, pos);
            }

            docStr.replace(QLatin1Char('\n'), QString(QLatin1String("\n")) + m_indentation + QLatin1String("# "));

            // Write parameter documentation
            foreach (UMLAttribute* at, atl) {
                // Only write an individual @param entry if one hasn't been found already
                // in the main doc comment
                if (commentedParams.contains(cppToRubyName(at->name())) == 0) {
                    docStr += (m_endl + m_indentation + QLatin1String("# @param _") + cppToRubyName(at->name()) + QLatin1Char('_'));
                    if (at->doc().isEmpty()) {
                        docStr += (QLatin1Char(' ') + cppToRubyType(at->getTypeName()));
                    } else {
                        docStr += (QLatin1Char(' ') + at->doc().replace(QRegExp(QLatin1String("[\\n\\r]+[\\t ]*")), m_endl + QLatin1String("   ")));
                    }
                }
            }

            docStr.remove(QLatin1String("@ref "));
            docStr.replace(QLatin1String("@param"), QLatin1String("*"));
            docStr.replace(QLatin1String("@return"), QLatin1String("* _returns_"));

            // All lines after the first '# *' in the doc comment
            // must be indented correctly. If they aren't a list
            // item starting with '# *', then indent the text with
            // three spaces, '#   ', to line up with the list item.
            pos = docStr.indexOf(QLatin1String("# *"));
            QRegExp re_linestart(QLatin1String("# (?!\\*)"));
            pos = re_linestart.indexIn(docStr, pos);
            while (pos > 0) {
                docStr.insert(pos + 1, QLatin1String("  "));

                pos += re_linestart.matchedLength() + 2;
                pos = re_linestart.indexIn(docStr, pos);
            }

            h << m_indentation << "# " <<  docStr << m_endl;

            QString typeStr = cppToRubyType(op->getTypeName());
            if (!typeStr.isEmpty() && typeStr != QLatin1String("void") && docStr.contains(QLatin1String("_returns_")) == 0) {
                h << m_indentation << "# * _returns_ " << typeStr << m_endl;
            }
        }

        h << m_indentation << "def " << methodName << "(";

        int j=0;
        foreach (UMLAttribute* at, atl) {
            QString nameStr = cppToRubyName(at->name());
            if (j > 0) {
                h << ", " << nameStr;
            } else {
                h << nameStr;
            }
            h << (!(at->getInitialValue().isEmpty()) ?
                QLatin1String(" = ") + cppToRubyType(at->getInitialValue()) :
                QString());
            j++;
        }

        h <<")" << m_endl;

        // write body
        QString sourceCode = op->getSourceCode();
        if (sourceCode.isEmpty()) {  // empty method body
            h << m_indentation << m_indentation << m_endl;
        }
        else {
            h << formatSourceCode(sourceCode, m_indentation + m_indentation);
        }

        h << m_indentation << "end" << m_endl << m_endl;

    }//end for
}

/**
 * Calls @ref writeSingleAttributeAccessorMethods() on each of the attributes in attribs list.
 * @param attribs      the attribute
 * @param visibility   the visibility of the attribute
 * @param stream       output stream to the generated file
 */
void RubyWriter::writeAttributeMethods(UMLAttributeList attribs,
                                      Uml::Visibility::Enum visibility, QTextStream &stream)
{
    // return now if NO attributes to work on
    if (attribs.count() == 0 || visibility == Uml::Visibility::Private)
        return;

    UMLAttribute *at;
    foreach (at,  attribs)
    {
        QString varName = cppToRubyName(cleanName(at->name()));

        writeSingleAttributeAccessorMethods(varName, at->doc(), stream);
    }

}

/**
 * Write all method declarations, for attributes and associations
 * for the given permitted scope.
 * @param fieldName   the field name
 * @param descr       the description
 * @param h           output stream to the generated file
 */
void RubyWriter::writeSingleAttributeAccessorMethods(
        const QString &fieldName,
        const QString &descr,
        QTextStream &h)
{
    QString description = descr;
    description.remove(QRegExp(QLatin1String("m_[npb](?=[A-Z])")));
    description.remove(QLatin1String("m_"));
    description.replace(QLatin1Char('\n'), QString(QLatin1String("\n")) + m_indentation + QLatin1String("# "));

    if (!description.isEmpty()) {
        h << m_indentation << "# " << description << m_endl;
    }

    h << m_indentation << "attr_accessor :" << fieldName << m_endl << m_endl;

    return;
}

/**
 * Returns "Ruby".
 * @return   the programming language identifier
 */
Uml::ProgrammingLanguage::Enum RubyWriter::language() const
{
    return Uml::ProgrammingLanguage::Ruby;
}

/**
 * Get list of reserved keywords.
 * @return   the list of reserved keywords
 */
QStringList RubyWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords
          << QLatin1String("__FILE__")
          << QLatin1String("__LINE__")
          << QLatin1String("BEGIN")
          << QLatin1String("END")
          << QLatin1String("alias")
          << QLatin1String("and")
          << QLatin1String("begin")
          << QLatin1String("break")
          << QLatin1String("case")
          << QLatin1String("class")
          << QLatin1String("def")
          << QLatin1String("defined?")
          << QLatin1String("do")
          << QLatin1String("else")
          << QLatin1String("elsif")
          << QLatin1String("end")
          << QLatin1String("ensure")
          << QLatin1String("false")
          << QLatin1String("for")
          << QLatin1String("if")
          << QLatin1String("in")
          << QLatin1String("module")
          << QLatin1String("next")
          << QLatin1String("nil")
          << QLatin1String("not")
          << QLatin1String("or")
          << QLatin1String("redo")
          << QLatin1String("rescue")
          << QLatin1String("retry")
          << QLatin1String("return")
          << QLatin1String("self")
          << QLatin1String("super")
          << QLatin1String("then")
          << QLatin1String("true")
          << QLatin1String("undef")
          << QLatin1String("unless")
          << QLatin1String("until")
          << QLatin1String("when")
          << QLatin1String("while")
          << QLatin1String("yield");
    }

    return keywords;
}

