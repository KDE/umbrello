/***************************************************************************
                          pythonwriter.h  -  description
                             -------------------
    begin                : Sat Dec 21 2002
    copyright            : Vincent Decorges
    email                : vincent.decorges@eivd.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pythonwriter.h"


#include <kdebug.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include "../umldoc.h"
#include "../umlattributelist.h"
#include "../association.h"
#include "../attribute.h"
#include "../classifier.h"
#include "../operation.h"
#include "../umlnamespace.h"

PythonWriter::PythonWriter( UMLDoc *parent, const char *name )
  : SimpleCodeGenerator(parent, name), m_bNeedPass(true) {
}

PythonWriter::~PythonWriter() {}

void PythonWriter::writeClass(UMLClassifier *c) {
    if(!c) {
        kdDebug()<<"Cannot write class of NULL concept!" << endl;
        return;
    }

    QString classname = cleanName(c->getName());
    QString fileName = c->getName();

    UMLClassifierList superclasses = c->getSuperClasses();
    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();

    m_bNeedPass = true;

    //find an appropriate name for our file
    fileName = findFileName(c,".py");
    if (!fileName) {
        emit codeGenerated(c, false);
        return;
    }

    QChar first = fileName.at(0);
    //Replace the first letter of the filename because
    //python class begin with an upper caracter (convention)
    first = first.upper();
    fileName = fileName.replace(0, 1, first);

    QFile fileh;
    if( !openFile(fileh,fileName+".py") ) {
        emit codeGenerated(c, false);
        return;
    }
    QTextStream h(&fileh);

    //////////////////////////////
    //Start generating the code!!
    /////////////////////////////


    //try to find a heading file (license, coments, etc)
    QString str;

    str = getHeadingFile(".py");
    if(!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), fileName+".py");
        str.replace(QRegExp("%filepath%"), fileh.name());
        h<<str<<m_endl;
    }


    //write includes and take namespaces into account
    UMLClassifierList includes;
    findObjectsRelated(c,includes);
    UMLClassifier* conc;
    for(conc = includes.first(); conc ;conc = includes.next()) {
        QString headerName = findFileName(conc, ".py");
        if ( !headerName.isEmpty() ) {
            first = headerName.at(0);
            first = first.upper();
            headerName = headerName.replace(0, 1, first);
            if (headerName.find('/') > 0)
                h<<"from "<<headerName.replace(QChar('/'),QChar('.'))<<" import *"<<m_endl;
            else
                h<<"from "<<headerName<<" import *"<<m_endl;
        }
    }
    h<<m_endl;

    h<<"class "<<classname<<(superclasses.count() > 0 ? " (":"");
    int i = superclasses.count();

    for (UMLClassifier *obj = superclasses.first();
            obj && i; obj = superclasses.next(), i--) {

        h<<cleanName(obj->getName())<<(i>1?", ":"");
    }


    h<<(superclasses.count() > 0 ? ")":"")<<":"<<m_endl<<m_endl;

    if(forceDoc() || !c->getDoc().isEmpty()) {
        h<<m_indentation<<"\"\"\""<<m_endl;
        h<<m_indentation<<c->getDoc()<<m_endl;
        h<<m_indentation<<":version:"<<m_endl;
        h<<m_indentation<<":author:"<<m_endl;
        h<<m_indentation<<"\"\"\""<<m_endl<<m_endl;
        m_bNeedPass = false;
    }

    //operations
    writeOperations(c,h);

    if (m_bNeedPass)
        h << m_indentation << "pass" << m_endl;

    //finish files
    h<<m_endl<<m_endl;

    //close files and notfiy we are done
    fileh.close();
    emit codeGenerated(c, true);
}


////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

void PythonWriter::writeOperations(UMLClassifier *c,QTextStream &h) {

    //Lists to store operations  sorted by scope
    UMLOperationList oppub,opprot,oppriv;

    oppub.setAutoDelete(false);
    opprot.setAutoDelete(false);
    oppriv.setAutoDelete(false);

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList opl(c->getOpList());
    for(UMLOperation *op = opl.first(); op ; op = opl.next()) {
        switch(op->getScope()) {
        case Uml::Public:
            oppub.append(op);
            break;
        case Uml::Protected:
            opprot.append(op);
            break;
        case Uml::Private:
            oppriv.append(op);
            break;
        }
    }

    QString classname(cleanName(c->getName()));

    //write operations to file
    if(forceSections() || !oppub.isEmpty()) {
        writeOperations(classname,oppub,h,PUBLIC);
    }

    if(forceSections() || !opprot.isEmpty()) {
        writeOperations(classname,opprot,h,PROTECTED);
    }

    if(forceSections() || !oppriv.isEmpty()) {
        writeOperations(classname,oppriv,h,PRIVATE);
    }

}

void PythonWriter::writeOperations(QString /*classname*/, UMLOperationList &opList,
                                   QTextStream &h, Access access) {
    UMLOperation *op;
    UMLAttributeList *atl;
    UMLAttribute *at;

    QString sAccess;

    switch (access) {

    case PUBLIC:
        sAccess = QString("");
        break;
    case PRIVATE:
        sAccess = QString("__");
        break;
    case PROTECTED:
        sAccess = QString("_");
        break;
    }


    for(op=opList.first(); op ; op=opList.next()) {
        atl = op -> getParmList();
        //write method doc if we have doc || if at least one of the params has doc
        bool writeDoc = forceDoc() || !op->getDoc().isEmpty();
        for(at = atl->first(); at ; at = atl -> next())
            writeDoc |= !at->getDoc().isEmpty();

        h<< m_indentation << "def "<< sAccess + cleanName(op->getName()) << "(self";

        int j=0;
        for( at = atl->first(); at ;at = atl->next(),j++) {
            h << ", " << cleanName(at->getName())
            << (!(at->getInitialValue().isEmpty()) ?
                (QString(" = ")+at->getInitialValue()) :
                QString(""));
        }

        h<<"):"<<m_endl;

        if( writeDoc )  //write method documentation
        {
            h<<m_indentation<<m_indentation<<"\"\"\""<<m_endl;
            h<<m_indentation<<m_indentation<<op->getDoc()<<m_endl<<m_endl;

            for(at = atl->first(); at ; at = atl -> next())  //write parameter documentation
            {
                if(forceDoc() || !at->getDoc().isEmpty()) {
                    h<<m_indentation<<m_indentation<<"@param "<<at->getTypeName()<<
                    " " << cleanName(at->getName());
                    h<<" : "<<at->getDoc()<<m_endl;
                }
            }//end for : write parameter documentation
            h<<m_indentation<<m_indentation<<"@return " + op->getTypeName()<<" :"<<m_endl;
            h<<m_indentation<<m_indentation<<"@since"<<m_endl;
            h<<m_indentation<<m_indentation<<"@author"<<m_endl;
            h<<m_indentation<<m_indentation<<"\"\"\""<<m_endl;
        }
        h<<m_indentation<<m_indentation<<"pass"<<m_endl<<m_endl;
        m_bNeedPass = false;
    }//end for
}

/**
 * returns "Python"
 */
QString PythonWriter::getLanguage() {
    return "Python";
}

/**
 * checks whether type is "PythonWriter"
 *
 * @param type
 */
bool PythonWriter::isType (QString & type)
{
    if(type == "PythonWriter")
        return true;
    return false;
}

const QStringList PythonWriter::reservedKeywords() const {

    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords << "abs"
        << "and"
        << "apply"
        << "ArithmeticError"
        << "assert"
        << "AssertionError"
        << "AttributeError"
        << "break"
        << "buffer"
        << "callable"
        << "chr"
        << "class"
        << "classmethod"
        << "cmp"
        << "coerce"
        << "compile"
        << "complex"
        << "continue"
        << "def"
        << "del"
        << "delattr"
        << "DeprecationWarning"
        << "dict"
        << "dir"
        << "divmod"
        << "elif"
        << "Ellipsis"
        << "else"
        << "EnvironmentError"
        << "EOFError"
        << "eval"
        << "except"
        << "Exception"
        << "exec"
        << "execfile"
        << "file"
        << "filter"
        << "finally"
        << "float"
        << "FloatingPointError"
        << "for"
        << "from"
        << "getattr"
        << "global"
        << "globals"
        << "hasattr"
        << "hash"
        << "hex"
        << "id"
        << "if"
        << "import"
        << "__import__"
        << "ImportError"
        << "in"
        << "IndentationError"
        << "IndexError"
        << "input"
        << "int"
        << "intern"
        << "IOError"
        << "is"
        << "isinstance"
        << "issubclass"
        << "iter"
        << "KeyboardInterrupt"
        << "KeyError"
        << "lambda"
        << "len"
        << "list"
        << "locals"
        << "long"
        << "LookupError"
        << "map"
        << "max"
        << "MemoryError"
        << "min"
        << "NameError"
        << "None"
        << "not"
        << "NotImplemented"
        << "NotImplementedError"
        << "object"
        << "oct"
        << "open"
        << "or"
        << "ord"
        << "OSError"
        << "OverflowError"
        << "OverflowWarning"
        << "pass"
        << "pow"
        << "print"
        << "property"
        << "raise"
        << "range"
        << "raw_input"
        << "reduce"
        << "ReferenceError"
        << "reload"
        << "repr"
        << "return"
        << "round"
        << "RuntimeError"
        << "RuntimeWarning"
        << "setattr"
        << "slice"
        << "StandardError"
        << "staticmethod"
        << "StopIteration"
        << "str"
        << "super"
        << "SyntaxError"
        << "SyntaxWarning"
        << "SystemError"
        << "SystemExit"
        << "TabError"
        << "try"
        << "tuple"
        << "type"
        << "TypeError"
        << "UnboundLocalError"
        << "unichr"
        << "unicode"
        << "UnicodeError"
        << "UserWarning"
        << "ValueError"
        << "vars"
        << "Warning"
        << "while"
        << "WindowsError"
        << "xrange"
        << "yield"
        << "ZeroDivisionError"
        << "zip";
    }

    return keywords;
}

#include "pythonwriter.moc"
