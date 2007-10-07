/***************************************************************************
                          rubywriter.h  -  description
                             -------------------
    begin                : Sat Dec 21 2002
    copyright            : Vincent Decorges
    email                : vincent.decorges@eivd.ch
      (C) 2003-2006  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rubywriter.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include "classifierinfo.h"
#include "../umldoc.h"
#include "../umlattributelist.h"
#include "../association.h"
#include "../attribute.h"
#include "../classifier.h"
#include "../operation.h"
#include "../umlnamespace.h"

RubyWriter::RubyWriter() {
}

RubyWriter::~RubyWriter() {}

void RubyWriter::writeClass(UMLClassifier *c) {
    if(!c) {
        kDebug()<<"Cannot write class of NULL concept!" << endl;
        return;
    }

    QString classname = cleanName(c->getName());

    UMLClassifierList superclasses = c->getSuperClasses();
    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();

    //find an appropriate name for our file
    QString fileName = findFileName(c, ".rb");
    if (fileName.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    QFile fileh;
    if( !openFile(fileh, fileName) ) {
        emit codeGenerated(c, false);
        return;
    }
    QTextStream h(&fileh);

    // preparations
    classifierInfo = new ClassifierInfo(c);
    classifierInfo->fileName = fileName;
    classifierInfo->className = cleanName(c->getName());

    //////////////////////////////
    //Start generating the code!!
    /////////////////////////////


    //try to find a heading file (license, coments, etc)
    QString str;

    str = getHeadingFile(".rb");
    if(!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), fileName);
        str.replace(QRegExp("%filepath%"), fileh.name());
        h<<str<<m_endl;
    }

    if(forceDoc() || !c->getDoc().isEmpty()) {
        QString docStr = c->getDoc();
        docStr.replace(QRegExp("\\n"), "\n# ");
        docStr.replace("@ref ", "");
        docStr.replace("@see", "_See_");
        docStr.replace("@short", "_Summary_");
        docStr.replace("@author", "_Author_");
        h<<"#"<<m_endl;
        h<<"# "<<docStr<<m_endl;
        h<<"#"<<m_endl<<m_endl;
    }

    // write inheritances out
    UMLClassifier *concept;

    h<< "class " << cppToRubyType(classname) << (superclasses.count() > 0 ? " < ":"");

    int i = 0;
    for (concept = superclasses.first(); concept; concept = superclasses.next()) {
        if (i == 0) {
            h << cppToRubyType(concept->getName()) << m_endl;
        } else {
            // Assume ruby modules that can be mixed in, after the first
            // superclass name in the list
            h << m_indentation << "include "<< cppToRubyType(concept->getName()) << m_endl;
        }
        i++;
    }

    h << m_endl;

    // write comment for sub-section IF needed
    if (forceDoc() || classifierInfo->hasAccessorMethods) {
        h << m_indentation << "#" << m_endl;
        h << m_indentation << "# Accessor Methods" << m_endl;
        h << m_indentation << "#" << m_endl << m_endl;

        // Accessor methods for attributes
        writeAttributeMethods(&(classifierInfo->atpub), Uml::Visibility::Public, h);
        writeAttributeMethods(&(classifierInfo->atprot), Uml::Visibility::Protected, h);
        writeAttributeMethods(&(classifierInfo->atpriv), Uml::Visibility::Private, h);
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


////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

QString RubyWriter::cppToRubyType(const QString &typeStr) {
    QString type = cleanName(typeStr);
    type.replace("const ", "");
    type.replace(QRegExp("[*&\\s]"), "");
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

QString RubyWriter::cppToRubyName(const QString &nameStr) {
    QString name = cleanName(nameStr);
    name.replace(QRegExp("^m_"), "");
    name.replace(QRegExp("^[pbn](?=[A-Z])"), "");
    name = name.mid(0, 1).lower() + name.mid(1);
    return name;
}

void RubyWriter::writeOperations(UMLClassifier *c,QTextStream &h) {

    //Lists to store operations  sorted by scope
    UMLOperationList oppub,opprot,oppriv;

    oppub.setAutoDelete(false);
    opprot.setAutoDelete(false);
    oppriv.setAutoDelete(false);

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList opl(c->getOpList());
    for(UMLOperation *op = opl.first(); op ; op = opl.next()) {
        switch(op->getVisibility()) {
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

    QString classname(cleanName(c->getName()));

    //write operations to file
    if(forceSections() || !oppub.isEmpty()) {
        writeOperations(classname, oppub, Uml::Visibility::Public, h);
    }

    if(forceSections() || !opprot.isEmpty()) {
        writeOperations(classname, opprot, Uml::Visibility::Protected, h);
    }

    if(forceSections() || !oppriv.isEmpty()) {
        writeOperations(classname, oppriv, Uml::Visibility::Private, h);
    }

}

void RubyWriter::writeOperations(const QString &classname, UMLOperationList &opList,
                                 Uml::Visibility permitScope, QTextStream &h)
{
    UMLOperation *op;
    UMLAttribute *at;

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

    for (op=opList.first(); op ; op=opList.next()) {
        QString methodName = cleanName(op->getName());
        QStringList commentedParams;

        // Skip destructors, and operator methods which
        // can't be defined in ruby
        if (    methodName.startsWith("~")
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

        methodName.replace("operator ", "");
        methodName = methodName.mid(0, 1).lower() + methodName.mid(1);

        UMLAttributeList atl = op->getParmList();
        //write method doc if we have doc || if at least one of the params has doc
        bool writeDoc = forceDoc() || !op->getDoc().isEmpty();
        // Always write out the docs for ruby as the type of the
        // arguments and return value of the methods is useful
        writeDoc = true;
//        for (at = atl.first(); at; at = atl.next())
//            writeDoc |= !at->getDoc().isEmpty();

        if (writeDoc) {
            h << m_indentation << "#" << m_endl;
            QString docStr = op->getDoc();

            docStr.replace(QRegExp("[\\n\\r]+ *"), m_endl);
            docStr.replace(QRegExp("[\\n\\r]+\\t*"), m_endl);

            docStr.replace(" m_", " ");
            docStr.replace(QRegExp("\\s[npb](?=[A-Z])"), " ");
            QRegExp re_params("@param (\\w)(\\w*)");
            int pos = re_params.search(docStr);
            while (pos != -1) {
                docStr.replace( re_params.cap(0),
                                QString("@param _") + re_params.cap(1).lower() + re_params.cap(2) + '_' );
                commentedParams.append(re_params.cap(1).lower() + re_params.cap(2));

                pos += re_params.matchedLength() + 3;
                pos = re_params.search(docStr, pos);
            }

            docStr.replace("\n", QString("\n") + m_indentation + "# ");

            // Write parameter documentation
            for (at = atl.first(); at ; at = atl.next()) {
                // Only write an individual @param entry if one hasn't been found already
                // in the main doc comment
                if (commentedParams.contains(cppToRubyName(at->getName())) == 0) {
                    docStr += (m_endl + m_indentation + "# @param _" + cppToRubyName(at->getName()) + '_');
                    if (at->getDoc().isEmpty()) {
                        docStr += (' ' + cppToRubyType(at->getTypeName()));
                    } else {
                        docStr += (' ' + at->getDoc().replace(QRegExp("[\\n\\r]+[\\t ]*"), m_endl + "   "));
                    }
                }
            }

            docStr.replace("@ref ", "");
            docStr.replace("@param", "*");
            docStr.replace("@return", "* _returns_");

            // All lines after the first '# *' in the doc comment
            // must be indented correctly. If they aren't a list
            // item starting with '# *', then indent the text with
            // three spaces, '#   ', to line up with the list item.
            pos = docStr.find("# *");
            QRegExp re_linestart("# (?!\\*)");
            pos = re_linestart.search(docStr, pos);
            while (pos > 0) {
                docStr.insert(pos + 1, "  ");

                pos += re_linestart.matchedLength() + 2;
                pos = re_linestart.search(docStr, pos);
            }

            h << m_indentation << "# "<< docStr << m_endl;

            QString typeStr = cppToRubyType(op->getTypeName());
            if (!typeStr.isEmpty() && typeStr != "void" && docStr.contains("_returns_") == 0) {
                h << m_indentation << "# * _returns_ " << typeStr << m_endl;
            }
        }

        h<< m_indentation << "def " + methodName << "(";

        int j=0;
        for (at = atl.first(); at; at = atl.next(), j++) {
            QString nameStr = cppToRubyName(at->getName());
            if (j > 0) {
                h << ", " << nameStr;
            } else {
                h << nameStr;
            }
            h << (!(at->getInitialValue().isEmpty()) ?
                (QString(" = ") + cppToRubyType(at->getInitialValue())) :
                QString(""));
        }

        h <<")" << m_endl;

        h << m_indentation << m_indentation << m_endl;

        h << m_indentation << "end" << m_endl << m_endl;

    }//end for

}

// this is for writing file attribute methods
//
void RubyWriter::writeAttributeMethods(UMLAttributeList *attribs,
                                      Uml::Visibility visibility, QTextStream &stream)
{
    // return now if NO attributes to work on
    if (attribs->count() == 0 || visibility == Uml::Visibility::Private)
        return;

    UMLAttribute *at;
    for(at=attribs->first(); at; at=attribs->next())
    {
        QString varName = cppToRubyName(cleanName(at->getName()));

        writeSingleAttributeAccessorMethods(varName, at->getDoc(), stream);
    }

}

void RubyWriter::writeSingleAttributeAccessorMethods(
        const QString &fieldName,
        const QString &descr,
        QTextStream &h)
{
    QString description = descr;
    description.replace(QRegExp("m_[npb](?=[A-Z])"), "");
    description.replace("m_", "");
    description.replace("\n", QString("\n") + m_indentation + "# ");

    if (!description.isEmpty()) {
        h << m_indentation << "# " << description << m_endl;
    }

    h << m_indentation << "attr_accessor :" << fieldName << m_endl << m_endl;

    return;
}

/**
 * returns "Ruby"
 */
Uml::Programming_Language RubyWriter::getLanguage() {
    return Uml::pl_Ruby;
}

const QStringList RubyWriter::reservedKeywords() const {

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
