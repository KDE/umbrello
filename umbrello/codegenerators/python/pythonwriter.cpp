/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002      Vincent Decorges  <vincent.decorges@eivd.ch>  *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "pythonwriter.h"

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

static const char *reserved_words[] = {
    "abs",
    "and",
    "apply",
    "ArithmeticError",
    "assert",
    "AssertionError",
    "AttributeError",
    "break",
    "buffer",
    "callable",
    "chr",
    "class",
    "classmethod",
    "cmp",
    "coerce",
    "compile",
    "complex",
    "continue",
    "def",
    "del",
    "delattr",
    "DeprecationWarning",
    "dict",
    "dir",
    "divmod",
    "elif",
    "Ellipsis",
    "else",
    "EnvironmentError",
    "EOFError",
    "eval",
    "except",
    "Exception",
    "exec",
    "execfile",
    "file",
    "filter",
    "finally",
    "float",
    "FloatingPointError",
    "for",
    "from",
    "getattr",
    "global",
    "globals",
    "hasattr",
    "hash",
    "hex",
    "id",
    "if",
    "import",
    "__import__",
    "ImportError",
    "in",
    "IndentationError",
    "IndexError",
    "input",
    "int",
    "intern",
    "IOError",
    "is",
    "isinstance",
    "issubclass",
    "iter",
    "KeyboardInterrupt",
    "KeyError",
    "lambda",
    "len",
    "list",
    "locals",
    "long",
    "LookupError",
    "map",
    "max",
    "MemoryError",
    "min",
    "NameError",
    "None",
    "not",
    "NotImplemented",
    "NotImplementedError",
    "object",
    "oct",
    "open",
    "or",
    "ord",
    "OSError",
    "OverflowError",
    "OverflowWarning",
    "pass",
    "pow",
    "print",
    "property",
    "raise",
    "range",
    "raw_input",
    "reduce",
    "ReferenceError",
    "reload",
    "repr",
    "return",
    "round",
    "RuntimeError",
    "RuntimeWarning",
    "setattr",
    "slice",
    "StandardError",
    "staticmethod",
    "StopIteration",
    "str",
    "super",
    "SyntaxError",
    "SyntaxWarning",
    "SystemError",
    "SystemExit",
    "TabError",
    "try",
    "tuple",
    "type",
    "TypeError",
    "UnboundLocalError",
    "unichr",
    "unicode",
    "UnicodeError",
    "UserWarning",
    "ValueError",
    "vars",
    "Warning",
    "while",
    "WindowsError",
    "xrange",
    "yield",
    "ZeroDivisionError",
    "zip",
    0
};

PythonWriter::PythonWriter() : m_bNeedPass(true)
{
}

PythonWriter::~PythonWriter()
{
}

void PythonWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        uDebug() << "Cannot write class of NULL concept!";
        return;
    }

    QString classname = cleanName(c->name());

    UMLClassifierList superclasses = c->getSuperClasses();
    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();

    m_bNeedPass = true;

    //find an appropriate name for our file
    QString fileName = findFileName(c, ".py");
    if (fileName.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    QChar first = fileName.at(0);
    //Replace the first letter of the filename because
    //python class begin with an upper caracter (convention)
    first = first.toUpper();
    fileName = fileName.replace(0, 1, first);

    QFile fileh;
    if ( !openFile(fileh, fileName) ) {
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
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), fileName);
        str.replace(QRegExp("%filepath%"), fileh.fileName());
        h<<str<<m_endl;
    }

    // generate import statement for superclasses and take packages into account
    str = cleanName(c->name());
    QString pkg = cleanName(c->package());
    if (!pkg.isEmpty())
        str.prepend(pkg + '.');
    QStringList includesList  = QStringList(str); //save imported classes
    int i = superclasses.count();
    foreach (UMLClassifier *classifier ,  superclasses ) {
        str = cleanName(classifier->name());
        pkg = cleanName(classifier->package());
        if (!pkg.isEmpty())
            str.prepend(pkg + '.');
        includesList.append(str);
        h << "from " + str + " import *" << m_endl;
        i--;
    }

    //write includes and take namespaces into account
    UMLPackageList includes;
    findObjectsRelated(c,includes);

    foreach(UMLPackage* conc, includes ) {
        QString headerName = findFileName(conc, ".py");
        if ( !headerName.isEmpty() ) {
            headerName.remove(QRegExp(".py$"));
            first = headerName.at(0);
            first = first.toUpper();
            headerName = headerName.replace(0, 1, first);
            str = headerName.replace(QChar('/'),QChar('.'));
            if (includesList.indexOf(str) < 0)  // not yet imported
                h << "from " << str << " import *" << m_endl;
        }
    }
    h << m_endl;

    h << "class " << classname << (superclasses.count() > 0 ? " (" : "(object)");
    i = superclasses.count();

    foreach (UMLClassifier *obj , superclasses ) {

        h << cleanName(obj->name()) << (i>1?", ":"");
        i--;
    }

    h<<(superclasses.count() > 0 ? ")":"")<<":"<<m_endl<<m_endl;

    if (forceDoc() || !c->doc().isEmpty()) {
        h << m_indentation << "\"\"\"" << m_endl;
        h << formatDoc(c->doc(), m_indentation + ' ') << m_endl;
        h << m_indentation << ":version:" << m_endl;
        h << m_indentation << ":author:" << m_endl;
        h << m_indentation << "\"\"\"" << m_endl << m_endl;
        m_bNeedPass = false;
    }

    // attributes
    writeAttributes(c->getAttributeList(), h);

    //operations
    writeOperations(c,h);

    if (m_bNeedPass)
        h << m_indentation << "pass" << m_endl;

    //finish files
    h << m_endl << m_endl;

    //close files and notfiy we are done
    fileh.close();
    emit codeGenerated(c, true);
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

void PythonWriter::writeAttributes(UMLAttributeList atList, QTextStream &py)
{
    if (!forceDoc() || atList.count() == 0)
        return;
    py << m_indentation << "\"\"\" ATTRIBUTES" << m_endl << m_endl;
    foreach (UMLAttribute *at , atList ) {
        py << formatDoc(at->doc(), m_indentation + ' ') << m_endl;
        Uml::Visibility vis = at->visibility();
        py << m_indentation << cleanName(at->name()) << "  ("
            << vis.toString() << ")" << m_endl << m_endl ;
    } // end for
    py << m_indentation << "\"\"\"" << m_endl << m_endl;
}

void PythonWriter::writeOperations(UMLClassifier *c, QTextStream &h)
{
    //Lists to store operations  sorted by scope
    UMLOperationList oppub,opprot,oppriv;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList opl(c->getOpList());
    foreach(UMLOperation *op , opl ) {
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
    if(forceSections() || !oppub.isEmpty()) {
        writeOperations(classname, oppub, h, Uml::Visibility::Public);
    }

    if(forceSections() || !opprot.isEmpty()) {
        writeOperations(classname, opprot, h, Uml::Visibility::Protected);
    }

    if(forceSections() || !oppriv.isEmpty()) {
        writeOperations(classname, oppriv, h, Uml::Visibility::Private);
    }

}

void PythonWriter::writeOperations(const QString& classname, UMLOperationList &opList,
                                   QTextStream &h, Uml::Visibility access)
{
    Q_UNUSED(classname);
    QString sAccess;

    switch (access)
    {
    case Uml::Visibility::Public:
        sAccess = QString("");
        break;
    case Uml::Visibility::Private:
        sAccess = QString("__");
        break;
    case Uml::Visibility::Protected:
        sAccess = QString("_");
        break;
    default:
        break;
    }

    foreach (UMLOperation* op  ,  opList ) {
        UMLAttributeList atl = op->getParmList();
        //write method doc if we have doc || if at least one of the params has doc
        bool writeDoc = forceDoc() || !op->doc().isEmpty();
        foreach (UMLAttribute* at , atl )
            writeDoc |= !at->doc().isEmpty();

        h << m_indentation << "def "<< sAccess + cleanName(op->name()) << "(self";

        int j=0;
        foreach (UMLAttribute* at , atl ) {
            h << ", " << cleanName(at->name())
            << (!(at->getInitialValue().isEmpty()) ?
                (QString(" = ")+at->getInitialValue()) :
                QString(""));
            j++;
        }

        h << "):" << m_endl;

        if ( writeDoc )  //write method documentation
        {
            h << m_indentation << m_indentation << "\"\"\"" << m_endl;
            h << formatDoc(op->doc(), m_indentation + m_indentation + ' ') << m_endl;

            foreach (UMLAttribute* at , atl )  //write parameter documentation
            {
                if(forceDoc() || !at->doc().isEmpty()) {
                    h<<m_indentation<<m_indentation<<"@param "<<at->getTypeName()<<
                    " " << cleanName(at->name());
                    h<<" : "<<at->doc()<<m_endl;
                }
            }//end for : write parameter documentation
            h << m_indentation << m_indentation << "@return " + op->getTypeName() << " :" << m_endl;
            h << m_indentation << m_indentation << "@author" << m_endl;
            h << m_indentation << m_indentation << "\"\"\"" << m_endl;
        }
        QString sourceCode = op->getSourceCode();
        if (sourceCode.isEmpty()) {
            h << m_indentation << m_indentation << "pass" << m_endl << m_endl;
        }
        else {
            h << formatSourceCode(sourceCode, m_indentation + m_indentation) << m_endl << m_endl;
        }
        m_bNeedPass = false;
    }//end for
}

Uml::ProgrammingLanguage PythonWriter::language() const
{
    return Uml::ProgrammingLanguage::Python;
}

QStringList PythonWriter::defaultDatatypes()
{
    QStringList l;
    l.append("array");
    l.append("bool");
    l.append("tuple");
    l.append("float");
    l.append("int");
    l.append("long");
    l.append("dict");
    l.append("object");
    l.append("set");
    l.append("string");
    return l;
}

QStringList PythonWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        for (int i = 0; reserved_words[i]; ++i) {
            keywords.append(reserved_words[i]);
        }
    }

    return keywords;
}

#include "pythonwriter.moc"
