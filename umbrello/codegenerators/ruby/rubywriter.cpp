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
#include <QRegularExpression>
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
        logWarn0("RubyWriter::writeClass: Cannot write class of NULL classifier");
        return;
    }

    UMLClassifierList superclasses = c->getSuperClasses();
    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();

    //find an appropriate name for our file
    fileName_ = findFileName(c, QStringLiteral(".rb"));
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

    str = getHeadingFile(QStringLiteral(".rb"));
    if (!str.isEmpty()) {
        str.replace(QRegularExpression(QStringLiteral("%filename%")), fileName_);
        str.replace(QRegularExpression(QStringLiteral("%filepath%")), fileh.fileName());
        h << str << m_endl;
    }

    if (forceDoc() || !c->doc().isEmpty()) {
        QString docStr = c->doc();
        docStr.replace(QRegularExpression(QStringLiteral("\\n")), QStringLiteral("\n# "));
        docStr.remove(QStringLiteral("@ref "));
        docStr.replace(QStringLiteral("@see"), QStringLiteral("_See_"));
        docStr.replace(QStringLiteral("@short"), QStringLiteral("_Summary_"));
        docStr.replace(QStringLiteral("@author"), QStringLiteral("_Author_"));
        h << "#" << m_endl;
        h << "# " << docStr << m_endl;
        h << "#" << m_endl << m_endl;
    }

    // write inheritances out

    h <<  "class " << cppToRubyType(className_) << (superclasses.count() > 0 ? QStringLiteral(" < ") : QString());

    int i = 0;
    for(UMLClassifier *classifier: superclasses) {
        if (i == 0) {
            h << cppToRubyType(classifier->name()) << m_endl;
        } else {
            // Assume ruby modules that can be mixed in, after the first
            // superclass name in the list
            h << m_indentation << "include " <<  cppToRubyType(classifier->name()) << m_endl;
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
 * @param typeStr the C++ type to be converted
 */
QString RubyWriter::cppToRubyType(const QString &typeStr)
{
    QString type = cleanName(typeStr);
    type.remove(QStringLiteral("const "));
    type.remove(QRegularExpression(QStringLiteral("[*&\\s]")));
    type.replace(QRegularExpression(QStringLiteral("[<>]")), QStringLiteral("_"));
    type.replace(QStringLiteral("QStringList"), QStringLiteral("Array"));
    type.replace(QStringLiteral("QString"), QStringLiteral("String"));
    type.replace(QStringLiteral("bool"), QStringLiteral("true|false"));
    type.replace(QRegularExpression(QStringLiteral("^(uint|int|ushort|short|ulong|long)$")), QStringLiteral("Integer"));
    type.replace(QRegularExpression(QStringLiteral("^(float|double)$")), QStringLiteral("Float"));
    type.replace(QRegularExpression(QStringLiteral("^Q(?=[A-Z])")), QStringLiteral("Qt::"));
    type.replace(QRegularExpression(QStringLiteral("^K(?!(DE|Parts|IO)")), QStringLiteral("KDE::"));

    return type;
}

/**
 * Convert C++ names such as 'm_foobar' or pFoobar to
 * just 'foobar' for ruby.
 * @param nameStr the C++ name to be converted
 */
QString RubyWriter::cppToRubyName(const QString &nameStr)
{
    QString name = cleanName(nameStr);
    name.remove(QRegularExpression(QStringLiteral("^m_")));
    name.remove(QRegularExpression(QStringLiteral("^[pbn](?=[A-Z])")));
    name = name.mid(0, 1).toLower() + name.mid(1);
    return name;
}

/**
 * Write all operations for a given class.
 * @param c   the classifier we are generating code for
 * @param h   output stream for the header file
 */
void RubyWriter::writeOperations(UMLClassifier *c, QTextStream &h)
{
    //Lists to store operations  sorted by scope
    UMLOperationList oppub, opprot, oppriv;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList opl(c->getOpList());
    for(UMLOperation *op : opl) {
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

    for(const UMLOperation *op : opList) {
        QString methodName = cleanName(op->name());
        QStringList commentedParams;

        // Skip destructors, and operator methods which
        // can't be defined in ruby
        if (methodName.startsWith(QLatin1Char('~'))
                || methodName == QStringLiteral("operator =")
                || methodName == QStringLiteral("operator --")
                || methodName == QStringLiteral("operator ++")
                || methodName == QStringLiteral("operator !="))
        {
            continue;
        }

        if (methodName == classname) {
            methodName = QStringLiteral("initialize");
        }

        methodName.remove(QStringLiteral("operator "));
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

            docStr.replace(QRegularExpression(QStringLiteral("[\\n\\r]+ *")), m_endl);
            docStr.replace(QRegularExpression(QStringLiteral("[\\n\\r]+\\t*")), m_endl);

            docStr.replace(QStringLiteral(" m_"), QStringLiteral(" "));
            docStr.replace(QRegularExpression(QStringLiteral("\\s[npb](?=[A-Z])")), QStringLiteral(" "));
            QRegularExpression re_params(QStringLiteral("@param (\\w)(\\w*)"));
            QRegularExpressionMatch re_mat = re_params.match(docStr);
            int pos = docStr.indexOf(re_params);
            while (pos != -1) {
                docStr.replace(re_mat.captured(0),
                                QString(QStringLiteral("@param _")) + re_mat.captured(1).toLower() + re_mat.captured(2) + QLatin1Char('_'));
                commentedParams.append(re_mat.captured(1).toLower() + re_mat.captured(2));

                pos += re_mat.capturedLength() + 3;
                pos = docStr.indexOf(docStr, pos);
            }

            docStr.replace(QLatin1Char('\n'), QString(QStringLiteral("\n")) + m_indentation + QStringLiteral("# "));

            // Write parameter documentation
            for(UMLAttribute* at : atl) {
                // Only write an individual @param entry if one hasn't been found already
                // in the main doc comment
                if (commentedParams.contains(cppToRubyName(at->name())) == 0) {
                    docStr += (m_endl + m_indentation + QStringLiteral("# @param _") + cppToRubyName(at->name()) + QLatin1Char('_'));
                    if (at->doc().isEmpty()) {
                        docStr += (QLatin1Char(' ') + cppToRubyType(at->getTypeName()));
                    } else {
                        docStr += (QLatin1Char(' ') + at->doc().replace(QRegularExpression(QStringLiteral("[\\n\\r]+[\\t ]*")), m_endl + QStringLiteral("   ")));
                    }
                }
            }

            docStr.remove(QStringLiteral("@ref "));
            docStr.replace(QStringLiteral("@param"), QStringLiteral("*"));
            docStr.replace(QStringLiteral("@return"), QStringLiteral("* _returns_"));

            // All lines after the first '# *' in the doc comment
            // must be indented correctly. If they aren't a list
            // item starting with '# *', then indent the text with
            // three spaces, '#   ', to line up with the list item.
            pos = docStr.indexOf(QStringLiteral("# *"));
            QRegularExpression re_linestart(QStringLiteral("# (?!\\*)"));
            QRegularExpressionMatch re_ls_mat = re_linestart.match(docStr);
            pos = docStr.indexOf(re_linestart, pos);
            while (pos > 0) {
                docStr.insert(pos + 1, QStringLiteral("  "));

                pos += re_ls_mat.capturedLength() + 2;
                pos = docStr.indexOf(re_linestart, pos);
            }

            h << m_indentation << "# " <<  docStr << m_endl;

            QString typeStr = cppToRubyType(op->getTypeName());
            if (!typeStr.isEmpty() && typeStr != QStringLiteral("void") && docStr.contains(QStringLiteral("_returns_")) == 0) {
                h << m_indentation << "# * _returns_ " << typeStr << m_endl;
            }
        }

        h << m_indentation << "def " << methodName << "(";

        int j=0;
        for(UMLAttribute *at : atl) {
            QString nameStr = cppToRubyName(at->name());
            if (j > 0) {
                h << ", " << nameStr;
            } else {
                h << nameStr;
            }
            h << (!(at->getInitialValue().isEmpty()) ?
                QStringLiteral(" = ") + cppToRubyType(at->getInitialValue()) :
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

    for(UMLAttribute *at:  attribs)
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
    description.remove(QRegularExpression(QStringLiteral("m_[npb](?=[A-Z])")));
    description.remove(QStringLiteral("m_"));
    description.replace(QLatin1Char('\n'), QString(QStringLiteral("\n")) + m_indentation + QStringLiteral("# "));

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
          << QStringLiteral("__FILE__")
          << QStringLiteral("__LINE__")
          << QStringLiteral("BEGIN")
          << QStringLiteral("END")
          << QStringLiteral("alias")
          << QStringLiteral("and")
          << QStringLiteral("begin")
          << QStringLiteral("break")
          << QStringLiteral("case")
          << QStringLiteral("class")
          << QStringLiteral("def")
          << QStringLiteral("defined?")
          << QStringLiteral("do")
          << QStringLiteral("else")
          << QStringLiteral("elsif")
          << QStringLiteral("end")
          << QStringLiteral("ensure")
          << QStringLiteral("false")
          << QStringLiteral("for")
          << QStringLiteral("if")
          << QStringLiteral("in")
          << QStringLiteral("module")
          << QStringLiteral("next")
          << QStringLiteral("nil")
          << QStringLiteral("not")
          << QStringLiteral("or")
          << QStringLiteral("redo")
          << QStringLiteral("rescue")
          << QStringLiteral("retry")
          << QStringLiteral("return")
          << QStringLiteral("self")
          << QStringLiteral("super")
          << QStringLiteral("then")
          << QStringLiteral("true")
          << QStringLiteral("undef")
          << QStringLiteral("unless")
          << QStringLiteral("until")
          << QStringLiteral("when")
          << QStringLiteral("while")
          << QStringLiteral("yield");
    }

    return keywords;
}

