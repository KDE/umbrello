/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2002 Vincent Decorges <vincent.decorges@eivd.ch>
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "pythonwriter.h"

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
#include <QTextCodec>
#include <QTextStream>

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

/**
 * Call this method to generate C++ code for a UMLClassifier.
 * @param c   the class you want to generate code for
 */
void PythonWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        logWarn0("PythonWriter::writeClass: Cannot write class of NULL concept!");
        return;
    }

    QString classname = cleanName(c->name());

    UMLClassifierList superclasses = c->getSuperClasses();
    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();

    m_bNeedPass = true;

    //find an appropriate name for our file
    QString fileName = findFileName(c, QLatin1String(".py"));

    // Do not generate files for classes that has a container
    if (hasContainer(fileName)) {
        emit codeGenerated(c, CodeGenerator::Skipped);
        return;
    }

    if (fileName.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    QFile fileh;
    if (!openFile(fileh, fileName)) {
        emit codeGenerated(c, false);
        return;
    }
    QTextStream h(&fileh);
    h.setCodec("UTF-8");

    //////////////////////////////
    //Start generating the code!!
    /////////////////////////////

    //try to find a heading file (license, comments, etc)
    QString str;

    str = getHeadingFile(QLatin1String(".py"));
    if (!str.isEmpty()) {
        str.replace(QRegExp(QLatin1String("%filename%")), fileName);
        str.replace(QRegExp(QLatin1String("%filepath%")), fileh.fileName());
        h<<str<<m_endl;
    }

    // generate import statement for superclasses and take packages into account
    str = cleanName(c->name());
    QString pkg = cleanName(c->package());
    if (!pkg.isEmpty())
        str.prepend(pkg + QLatin1Char('.'));
    QStringList includesList  = QStringList(str); //save imported classes
    int i = superclasses.count();
    foreach (UMLClassifier *classifier,  superclasses) {
        str = cleanName(classifier->name());
        pkg = cleanName(classifier->package());
        if (!pkg.isEmpty())
            str.prepend(pkg + QLatin1Char('.'));
        includesList.append(str);
        h << "from " << str << " import *" << m_endl;
        i--;
    }

    //write includes and take namespaces into account
    UMLPackageList includes;
    findObjectsRelated(c, includes);

    foreach(UMLPackage* conc, includes) {
        QString headerName = findFileName(conc, QLatin1String(".py"));
        if (!headerName.isEmpty()) {
            headerName.remove(QRegExp(QLatin1String(".py$")));
            str = findIncludeFromType(headerName.replace(QLatin1Char('/'), QLatin1Char('.')));
            // not yet imported
            if (includesList.indexOf(str) < 0)  {
                includesList.append(str);
                h << "from " << str << " import *" << m_endl;
            }
        }
    }
    h << m_endl;

    h << "class " << classname;
    if (superclasses.count()) {
        h << QLatin1String("(");
        h << cleanName(superclasses.front()->name());
        for (auto superclass = std::next(std::begin(superclasses)); superclass != std::end(superclasses); superclass++) {
            h << QLatin1String(", ") << cleanName((*superclass)->name());
        }
        h << QLatin1String(")");
    }

    h << ":" << m_endl << m_endl;

    if (forceDoc() || !c->doc().isEmpty()) {
        h << m_indentation << "\"\"\"" << m_endl;
        if (!c->doc().isEmpty()) {
            h << formatDoc(c->doc(), m_indentation + QLatin1Char(' ')) << m_endl;
            h << m_endl;
        }
        h << m_indentation << ":version:" << m_endl;
        h << m_indentation << ":author:" << m_endl;
        h << m_indentation << "\"\"\"" << m_endl << m_endl;
        m_bNeedPass = false;
    }

    // attributes
    writeAttributes(c->getAttributeList(), h);

    //operations
    writeOperations(c, h);

    if (m_bNeedPass)
        h << m_indentation << "pass" << m_endl;

    //finish files
    h << m_endl << m_endl;

    //close files and notfiy we are done
    fileh.close();
    emit codeGenerated(c, true);
    emit showGeneratedFile(fileh.fileName());
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

/**
 * Write all attributes for a given class.
 * @param atList  the attribute list we are generating code for
 * @param py   output stream for the header file
 */
void PythonWriter::writeAttributes(UMLAttributeList atList, QTextStream &py)
{
    if (!forceDoc() || atList.count() == 0)
        return;
    py << m_indentation << "\"\"\" ATTRIBUTES" << m_endl << m_endl;
    foreach (UMLAttribute *at, atList) {
        if (!at->doc().isEmpty()) {
            py << formatDoc(at->doc(), m_indentation + QLatin1Char(' ')) << m_endl;
            py << m_endl;
        }
        Uml::Visibility::Enum vis = at->visibility();
        py << m_indentation << cleanName(at->name()) << "  ("
            << Uml::Visibility::toString(vis) << ")" << m_endl << m_endl ;
    } // end for
    py << m_indentation << "\"\"\"" << m_endl << m_endl;
}

/**
 * Write all operations for a given class.
 * @param c   the concept we are generating code for
 * @param h   output stream for the header file
 */
void PythonWriter::writeOperations(UMLClassifier *c, QTextStream &h)
{
    //Lists to store operations  sorted by scope
    UMLOperationList oppub, opprot, oppriv;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList opl(c->getOpList());
    foreach(UMLOperation *op, opl) {
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

/**
 * Write a list of class operations.
 * @param classname   the name of the class
 * @param opList      the list of operations
 * @param h           output stream for the header file
 * @param access      visibility identifier
 */
void PythonWriter::writeOperations(const QString& classname, UMLOperationList &opList,
                                   QTextStream &h, Uml::Visibility::Enum access)
{
    Q_UNUSED(classname);
    QString sAccess;

    switch (access)
    {
    case Uml::Visibility::Public:
        sAccess = QString();
        break;
    case Uml::Visibility::Private:
        sAccess = QLatin1String("__");
        break;
    case Uml::Visibility::Protected:
        sAccess = QLatin1String("_");
        break;
    default:
        break;
    }

    foreach (UMLOperation* op,  opList) {
        UMLAttributeList atl = op->getParmList();
        //write method doc if we have doc || if at least one of the params has doc
        bool writeDoc = forceDoc() || !op->doc().isEmpty();
        foreach (UMLAttribute* at, atl)
            writeDoc |= !at->doc().isEmpty();

        h << m_indentation << "def "<< sAccess + cleanName(op->name()) << "(self";

        int j=0;
        foreach (UMLAttribute* at, atl) {
            h << ", " << cleanName(at->name()) << ": " << PythonWriter::fixTypeName(at->getTypeName())
              << (!(at->getInitialValue().isEmpty()) ?
                  (QLatin1String(" = ") + at->getInitialValue()) : QString());
            j++;
        }

        h << "):" << m_endl;

        if (writeDoc)  //write method documentation
        {
            h << m_indentation << m_indentation << "\"\"\"" << m_endl;
            h << formatDoc(op->doc(), m_indentation + m_indentation + QLatin1Char(' ')) << m_endl;

            foreach (UMLAttribute* at, atl)  //write parameter documentation
            {
                if(forceDoc() || !at->doc().isEmpty()) {
                    h<<m_indentation<<m_indentation<<"@param "<<at->getTypeName()<<
                    " " << cleanName(at->name());
                    h<<" : "<<at->doc()<<m_endl;
                }
            }//end for : write parameter documentation
            h << m_indentation << m_indentation << "@return " << op->getTypeName() << " :" << m_endl;
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

/**
 * Check if type is a container
 * @param string      type that will be used
 * @return            true if is a container
 */
bool PythonWriter::hasContainer(const QString &string)
{
    return string.contains(QLatin1String("<")) && string.contains(QLatin1String(">"));
}

/**
 * Fix types to be compatible with Python
 * @param string      type as defined in model
 * @return            fixed type
 */
QString PythonWriter::fixTypeName(const QString &string)
{
    if (string == QLatin1String("string")) {
        return QLatin1String("str");
    }
    QRegExp re(QLatin1String("^vector<(.*)>$"));
    if (re.indexIn(string) >= 0) {
        const QString listOf(QLatin1String("List[%1]"));
        return listOf.arg(fixTypeName(re.cap(1)));
    }
    return string;
}

QString PythonWriter::findIncludeFromType(const QString &string)
{
    const QString fixedTypeName = fixTypeName(string);
    QRegExp re(QLatin1String("^(Any|Dict|List|Tuple)\\["));
    if (re.indexIn(fixedTypeName) >= 0) {
        return QLatin1String("typing");
    }
    return string;
}

/**
 * Return the programming language identifier.
 * @return   programming language id
 */
Uml::ProgrammingLanguage::Enum PythonWriter::language() const
{
    return Uml::ProgrammingLanguage::Python;
}

/**
 * Reimplementation of method from class CodeGenerator
 */
QStringList PythonWriter::defaultDatatypes() const
{
    QStringList l;
    l.append(QLatin1String("array"));
    l.append(QLatin1String("bool"));
    l.append(QLatin1String("tuple"));
    l.append(QLatin1String("float"));
    l.append(QLatin1String("int"));
    l.append(QLatin1String("list"));
    l.append(QLatin1String("long"));
    l.append(QLatin1String("dict"));
    l.append(QLatin1String("object"));
    l.append(QLatin1String("set"));
    l.append(QLatin1String("str"));
    return l;
}

/**
 * Get list of reserved keywords.
 * @return   the list of reserved keywords
 */
QStringList PythonWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        for (int i = 0; reserved_words[i]; ++i) {
            keywords.append(QLatin1String(reserved_words[i]));
        }
    }

    return keywords;
}

