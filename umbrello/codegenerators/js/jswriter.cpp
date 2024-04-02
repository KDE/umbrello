/*
    SPDX-FileCopyrightText: 2003 Alexander Blum <blum@kewbee.de>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "jswriter.h"

#include "association.h"
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "operation.h"
#include "umldoc.h"
#include "uml.h"  // Only needed for log{Warn,Error}

#include <QRegularExpression>
#include <QTextStream>

JSWriter::JSWriter()
{
}

JSWriter::~JSWriter()
{
}

/**
 * Call this method to generate Actionscript code for a UMLClassifier.
 * @param c   the class you want to generate code for
 */
void JSWriter::writeClass(UMLClassifier *c)
{
    if (!c)
    {
        logWarn0("JSWriter::writeClass: Cannot write class of NULL classifier");
        return;
    }

    QString classname = cleanName(c->name());
    QString fileName = c->name().toLower();

    //find an appropriate name for our file
    fileName = findFileName(c, QStringLiteral(".js"));
    if (fileName.isEmpty())
    {
        emit codeGenerated(c, false);
        return;
    }

    QFile filejs;
    if (!openFile(filejs, fileName))
    {
        emit codeGenerated(c, false);
        return;
    }
    QTextStream js(&filejs);

    //////////////////////////////
    //Start generating the code!!
    /////////////////////////////

    //try to find a heading file (license, comments, etc)
    QString str;
    str = getHeadingFile(QStringLiteral(".js"));
    if (!str.isEmpty())
    {
        str.replace(QRegularExpression(QStringLiteral("%filename%")), fileName);
        str.replace(QRegularExpression(QStringLiteral("%filepath%")), filejs.fileName());
        js << str << m_endl;
    }

    //write includes
    UMLPackageList includes;
    findObjectsRelated(c, includes);
    Q_FOREACH(UMLPackage* conc,  includes) {
        QString headerName = findFileName(conc, QStringLiteral(".js"));
        if (!headerName.isEmpty())
        {
            js << "#include \"" << headerName << "\"" << m_endl;
        }
    }
    js << m_endl;

    //Write class Documentation if there is something or if force option
    if (forceDoc() || !c->doc().isEmpty())
    {
        js << m_endl << "/**" << m_endl;
        js << "  * class " << classname << m_endl;
        js << formatDoc(c->doc(), QStringLiteral("  * "));
        js << "  */" << m_endl << m_endl;
    }

    //check if class is abstract and / or has abstract methods
    if (c->isAbstract() && !hasAbstractOps(c))
        js << "/******************************* Abstract Class ****************************" << m_endl << "  "
           << classname << " does not have any pure virtual methods, but its author" << m_endl
           << "  defined it as an abstract class, so you should not use it directly." << m_endl
           << "  Inherit from it instead and create only objects from the derived classes" << m_endl
           << "*****************************************************************************/" << m_endl << m_endl;

    js << classname << " = function ()" << m_endl;
    js << "{" << m_endl;
    js << m_indentation << "this._init ();" << m_endl;
    js << "}" << m_endl;
    js << m_endl;

    UMLClassifierList superclasses = c->getSuperClasses();
    Q_FOREACH(UMLClassifier *obj, superclasses) {
        js << classname << ".prototype = new " << cleanName(obj->name()) << " ();" << m_endl;
    }

    js << m_endl;

    if (! c->isInterface()) {
        UMLAttributeList atl = c->getAttributeList();

        js << "/**" << m_endl;
        QString temp = QStringLiteral("_init sets all ") + classname + QStringLiteral(" attributes to their default value.")
                     + QStringLiteral(" Make sure to call this method within your class constructor");
        js << formatDoc(temp, QStringLiteral(" * "));
        js << " */" << m_endl;
        js << classname << ".prototype._init = function ()" << m_endl;
        js << "{" << m_endl;
        Q_FOREACH(UMLAttribute *at, atl) {
            if (forceDoc() || !at->doc().isEmpty())
            {
                js << m_indentation << "/**" << m_endl
                   << formatDoc(at->doc(), m_indentation + QStringLiteral(" * "))
                   << m_indentation << " */" << m_endl;
            }
            if (!at->getInitialValue().isEmpty())
            {
                js << m_indentation << "this.m_" << cleanName(at->name()) << " = " << at->getInitialValue() << ";" << m_endl;
            }
            else
            {
                js << m_indentation << "this.m_" << cleanName(at->name()) << " = \"\";" << m_endl;
            }
        }
    }

    //associations
    UMLAssociationList aggregations = c->getAggregations();
    if (forceSections() || !aggregations.isEmpty())
    {
        js << m_endl << m_indentation << "/**Aggregations: */" << m_endl;
        writeAssociation(classname, aggregations, js);

    }
    UMLAssociationList compositions = c->getCompositions();
    if (forceSections() || !compositions.isEmpty())
    {
        js << m_endl << m_indentation << "/**Compositions: */" << m_endl;
        writeAssociation(classname, compositions, js);

    }
    js << m_endl;
    js << "}" << m_endl;
    js << m_endl;

    //operations
    UMLOperationList ops(c->getOpList());
    writeOperations(classname, &ops, js);

    js << m_endl;

    //finish file

    //close files and notfiy we are done
    filejs.close();
    emit codeGenerated(c, true);
    emit showGeneratedFile(filejs.fileName());
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

/**
 * Write a list of associations.
 * @param classname   the name of the class
 * @param assocList   the list of associations
 * @param js          output stream for the JS file
 */
void JSWriter::writeAssociation(QString& classname, UMLAssociationList& assocList, QTextStream &js)
{
    Q_FOREACH(UMLAssociation *a, assocList) {
        // association side
        Uml::RoleType::Enum role = (a->getObject(Uml::RoleType::A)->name() == classname ? Uml::RoleType::B : Uml::RoleType::A);

        QString roleName(cleanName(a->getRoleName(role)));

        if (!roleName.isEmpty()) {

            // association doc
            if (forceDoc() || !a->doc().isEmpty())
            {
                js << m_indentation << "/**" << m_endl
                   << formatDoc(a->doc(), m_indentation + QStringLiteral(" * "))
                   << m_indentation << " */" << m_endl;
            }

            // role doc
            if (forceDoc() || !a->getRoleDoc(role).isEmpty())
            {
                js << m_indentation << "/**" << m_endl
                   << formatDoc(a->getRoleDoc(role), m_indentation + QStringLiteral(" * "))
                   << m_indentation << " */" << m_endl;
            }

            bool okCvt;
            int nMulti = a->getMultiplicity(role).toInt(&okCvt, 10);
            bool isNotMulti = a->getMultiplicity(role).isEmpty() || (okCvt && nMulti == 1);

            QString typeName(cleanName(a->getObject(role)->name()));

            if (isNotMulti)
                js << m_indentation << "this.m_" << roleName << " = new " << typeName << "();" << m_endl;
            else
                js << m_indentation << "this.m_" << roleName << " = new Array();" << m_endl;

            // role visibility
        }
    }
}

/**
 * Write a list of class operations.
 * @param classname   the name of the class
 * @param opList      the list of operations
 * @param js          output stream for the JS file
 */
void JSWriter::writeOperations(QString classname, UMLOperationList *opList, QTextStream &js)
{
    Q_FOREACH(UMLOperation* op,  *opList)
    {
        UMLAttributeList atl = op->getParmList();
        //write method doc if we have doc || if at least one of the params has doc
        bool writeDoc = forceDoc() || !op->doc().isEmpty();
        Q_FOREACH(UMLAttribute* at, atl)
            writeDoc |= !at->doc().isEmpty();

        if (writeDoc)  //write method documentation
        {
            js << "/**" << m_endl << formatDoc(op->doc(), QStringLiteral(" * "));

            Q_FOREACH(UMLAttribute* at, atl)  //write parameter documentation
            {
                if (forceDoc() || !at->doc().isEmpty())
                {
                    js << " * @param " << cleanName(at->name()) << m_endl;
                    js << formatDoc(at->doc(), QStringLiteral("    *      "));
                }
            }//end for : write parameter documentation
            js << " */" << m_endl;
        }//end if : write method documentation

        js << classname << ".prototype." << cleanName(op->name()) << " = function " << "(";

        int i = atl.count();
        int j=0;
        Q_FOREACH(UMLAttribute* at, atl) {
            js << cleanName(at->name())
               << (!(at->getInitialValue().isEmpty()) ? QStringLiteral(" = ") + at->getInitialValue() : QString())
               << ((j < i-1) ? QStringLiteral(", ") : QString());
            j++;
        }
        js << ")" << m_endl << "{" << m_endl;
        QString sourceCode = op->getSourceCode();
        if (sourceCode.isEmpty()) {
            js << m_indentation << m_endl;
        }
        else {
            js << formatSourceCode(sourceCode, m_indentation);
        }
        js << "}" << m_endl;
        js << m_endl << m_endl;
    }//end for
}

/**
 * Returns "JavaScript".
 * @return   the programming language identifier
 */
Uml::ProgrammingLanguage::Enum JSWriter::language() const
{
    return Uml::ProgrammingLanguage::JavaScript;
}

/**
 * Get list of reserved keywords.
 * @return   the list of reserved keywords
 */
QStringList JSWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords
          << QStringLiteral("break")
          << QStringLiteral("case")
          << QStringLiteral("const")
          << QStringLiteral("continue")
          << QStringLiteral("default")
          << QStringLiteral("else")
          << QStringLiteral("false")
          << QStringLiteral("for")
          << QStringLiteral("function")
          << QStringLiteral("if")
          << QStringLiteral("in")
          << QStringLiteral("new")
          << QStringLiteral("return")
          << QStringLiteral("switch")
          << QStringLiteral("this")
          << QStringLiteral("true")
          << QStringLiteral("var")
          << QStringLiteral("while")
          << QStringLiteral("with");
    }

    return keywords;
}

