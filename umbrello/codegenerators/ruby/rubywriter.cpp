/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "rubywriter.h"

#include "association.h"
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "operation.h"
#include "umldoc.h"
#include "umlattributelist.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>

RubyWriter::RubyWriter()
{
}

RubyWriter::~RubyWriter()
{
}

void RubyWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        uDebug()<<"Cannot write class of NULL concept!";
        return;
    }

    UMLClassifierList superclasses = c->getSuperClasses();
    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();

    //find an appropriate name for our file
    fileName_ = findFileName(c, ".rb");
    if (fileName_.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    QFile fileh;
    if ( !openFile(fileh, fileName_) ) {
        emit codeGenerated(c, false);
        return;
    }
    QTextStream h(&fileh);

    className_ = cleanName(c->name());

    //////////////////////////////
    //Start generating the code!!
    /////////////////////////////

    //try to find a heading file (license, coments, etc)
    QString str;

    str = getHeadingFile(".rb");
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), fileName_);
        str.replace(QRegExp("%filepath%"), fileh.fileName());
        h<<str<<m_endl;
    }

    if (forceDoc() || !c->doc().isEmpty()) {
        QString docStr = c->doc();
        docStr.replace(QRegExp("\\n"), "\n# ");
        docStr.remove("@ref ");
        docStr.replace("@see", "_See_");
        docStr.replace("@short", "_Summary_");
        docStr.replace("@author", "_Author_");
        h<<"#"<<m_endl;
        h<<"# "<<docStr<<m_endl;
        h<<"#"<<m_endl<<m_endl;
    }

    // write inheritances out
    UMLClassifier *concept;

    h<< "class " << cppToRubyType(className_) << (superclasses.count() > 0 ? " < ":"");

    int i = 0;
    foreach (concept , superclasses ) {
        if (i == 0) {
            h << cppToRubyType(concept->name()) << m_endl;
        } else {
            // Assume ruby modules that can be mixed in, after the first
            // superclass name in the list
            h << m_indentation << "include "<< cppToRubyType(concept->name()) << m_endl;
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

QString RubyWriter::cppToRubyType(const QString &typeStr)
{
    QString type = cleanName(typeStr);
    type.remove("const ");
    type.remove(QRegExp("[*&\\s]"));
    type.replace(QRegExp("[<>]"), "_");
    type.replace("QStringList", "Array");
    type.replace("QString", "String");
    type.replace("bool", "true|false");
    type.replace(QRegExp("^(uint|int|ushort|short|ulong|long)$"), "Integer");
    type.replace(QRegExp("^(float|double)$"), "Float");
    type.replace(QRegExp("^Q(?=[A-Z])"), "Qt::");
    type.replace(QRegExp("^K(?!(DE|Parts|IO)"), "KDE::");

    return type;
}

QString RubyWriter::cppToRubyName(const QString &nameStr)
{
    QString name = cleanName(nameStr);
    name.remove(QRegExp("^m_"));
    name.remove(QRegExp("^[pbn](?=[A-Z])"));
    name = name.mid(0, 1).toLower() + name.mid(1);
    return name;
}

void RubyWriter::writeOperations(UMLClassifier *c,QTextStream &h)
{
    //Lists to store operations  sorted by scope
    UMLOperationList oppub,opprot,oppriv;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList opl(c->getOpList());
    foreach (UMLOperation *op , opl ) {
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

void RubyWriter::writeOperations(const QString &classname, const UMLOperationList &opList,
                                 Uml::Visibility permitScope, QTextStream &h)
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
        if (    methodName.startsWith('~')
                || methodName == "operator ="
                || methodName == "operator --"
                || methodName == "operator ++"
                || methodName == "operator !=" )
        {
            continue;
        }

        if (methodName == classname) {
            methodName = "initialize";
        }

        methodName.remove("operator ");
        methodName = methodName.mid(0, 1).toLower() + methodName.mid(1);

        UMLAttributeList atl = op->getParmList();
        //write method doc if we have doc || if at least one of the params has doc
        bool writeDoc = forceDoc() || !op->doc().isEmpty();
        // Always write out the docs for ruby as the type of the
        // arguments and return value of the methods is useful
        writeDoc = true;
//        for (UMLAttribute& at = atl.first(); at; at = atl.next())
//            writeDoc |= !at->getDoc().isEmpty();

        if (writeDoc) {
            h << m_indentation << "#" << m_endl;
            QString docStr = op->doc();

            docStr.replace(QRegExp("[\\n\\r]+ *"), m_endl);
            docStr.replace(QRegExp("[\\n\\r]+\\t*"), m_endl);

            docStr.replace(" m_", " ");
            docStr.replace(QRegExp("\\s[npb](?=[A-Z])"), " ");
            QRegExp re_params("@param (\\w)(\\w*)");
            int pos = re_params.indexIn(docStr);
            while (pos != -1) {
                docStr.replace( re_params.cap(0),
                                QString("@param _") + re_params.cap(1).toLower() + re_params.cap(2) + '_' );
                commentedParams.append(re_params.cap(1).toLower() + re_params.cap(2));

                pos += re_params.matchedLength() + 3;
                pos = re_params.indexIn(docStr, pos);
            }

            docStr.replace('\n', QString("\n") + m_indentation + "# ");

            // Write parameter documentation
            foreach (const UMLAttribute& at , atl) {
                // Only write an individual @param entry if one hasn't been found already
                // in the main doc comment
                if (commentedParams.contains(cppToRubyName(at.name())) == 0) {
                    docStr += (m_endl + m_indentation + "# @param _" + cppToRubyName(at.name()) + '_');
                    if (at.doc().isEmpty()) {
                        docStr += (' ' + cppToRubyType(at.getTypeName()));
                    } else {
                        docStr += (' ' + at.doc().replace(QRegExp("[\\n\\r]+[\\t ]*"), m_endl + "   "));
                    }
                }
            }

            docStr.remove("@ref ");
            docStr.replace("@param", "*");
            docStr.replace("@return", "* _returns_");

            // All lines after the first '# *' in the doc comment
            // must be indented correctly. If they aren't a list
            // item starting with '# *', then indent the text with
            // three spaces, '#   ', to line up with the list item.
            pos = docStr.indexOf("# *");
            QRegExp re_linestart("# (?!\\*)");
            pos = re_linestart.indexIn(docStr, pos);
            while (pos > 0) {
                docStr.insert(pos + 1, "  ");

                pos += re_linestart.matchedLength() + 2;
                pos = re_linestart.indexIn(docStr, pos);
            }

            h << m_indentation << "# "<< docStr << m_endl;

            QString typeStr = cppToRubyType(op->getTypeName());
            if (!typeStr.isEmpty() && typeStr != "void" && docStr.contains("_returns_") == 0) {
                h << m_indentation << "# * _returns_ " << typeStr << m_endl;
            }
        }

        h<< m_indentation << "def " + methodName << "(";

        int j=0;
        foreach (const UMLAttribute& at , atl) {
            QString nameStr = cppToRubyName(at.name());
            if (j > 0) {
                h << ", " << nameStr;
            } else {
                h << nameStr;
            }
            h << (!(at.getInitialValue().isEmpty()) ?
                (QString(" = ") + cppToRubyType(at.getInitialValue())) :
                QString(""));
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

void RubyWriter::writeAttributeMethods(UMLAttributeList attribs,
                                      Uml::Visibility visibility, QTextStream &stream)
{
    // return now if NO attributes to work on
    if (attribs.count() == 0 || visibility == Uml::Visibility::Private)
        return;

    UMLAttribute *at;
    foreach (at ,  attribs)
    {
        QString varName = cppToRubyName(cleanName(at->name()));

        writeSingleAttributeAccessorMethods(varName, at->doc(), stream);
    }

}

void RubyWriter::writeSingleAttributeAccessorMethods(
        const QString &fieldName,
        const QString &descr,
        QTextStream &h)
{
    QString description = descr;
    description.remove(QRegExp("m_[npb](?=[A-Z])"));
    description.remove("m_");
    description.replace('\n', QString("\n") + m_indentation + "# ");

    if (!description.isEmpty()) {
        h << m_indentation << "# " << description << m_endl;
    }

    h << m_indentation << "attr_accessor :" << fieldName << m_endl << m_endl;

    return;
}

Uml::ProgrammingLanguage RubyWriter::language() const
{
    return Uml::ProgrammingLanguage::Ruby;
}

QStringList RubyWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords << "__FILE__"
        << "__LINE__"
        << "BEGIN"
        << "END"
        << "alias"
        << "and"
        << "begin"
        << "break"
        << "case"
        << "class"
        << "def"
        << "defined?"
        << "do"
        << "else"
        << "elsif"
        << "end"
        << "ensure"
        << "false"
        << "for"
        << "if"
        << "in"
        << "module"
        << "next"
        << "nil"
        << "not"
        << "or"
        << "redo"
        << "rescue"
        << "retry"
        << "return"
        << "self"
        << "super"
        << "then"
        << "true"
        << "undef"
        << "unless"
        << "until"
        << "when"
        << "while"
        << "yield";
    }

    return keywords;
}

#include "rubywriter.moc"
