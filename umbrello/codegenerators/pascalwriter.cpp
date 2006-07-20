/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                 *
 ***************************************************************************/

#include "pascalwriter.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtextstream.h>

#include "../umldoc.h"
#include "../classifier.h"
#include "../enum.h"
#include "../classifierlistitem.h"
#include "../umlclassifierlistitemlist.h"
#include "../umltemplatelist.h"
#include "../package.h"
#include "../association.h"
#include "../attribute.h"
#include "../operation.h"
#include "../template.h"
#include "../umlnamespace.h"

const QString PascalWriter::defaultPackageSuffix = "_Holder";

PascalWriter::PascalWriter() {
}

PascalWriter::~PascalWriter() {}

/**
 * returns "Pascal"
 */
Uml::Programming_Language PascalWriter::getLanguage() {
    return Uml::pl_Pascal;
}


bool PascalWriter::isOOClass(UMLClassifier *c) {
    Uml::Object_Type ot = c->getBaseType();
    if (ot == Uml::ot_Interface)
        return true;
    if (ot == Uml::ot_Enum)
        return false;
    if (ot != Uml::ot_Class) {
        kdDebug() << "PascalWriter::isOOClass: unknown object type " << ot << endl;
        return false;
    }
    QString stype = c->getStereotype();
    if (stype == "CORBAConstant" || stype == "CORBATypedef" ||
            stype == "CORBAStruct" || stype == "CORBAUnion")
        return false;
    // CORBAValue, CORBAInterface, and all empty/unknown stereotypes are
    // assumed to be OO classes.
    return true;
}

QString PascalWriter::qualifiedName(UMLClassifier *c, bool withType, bool byValue) {
    UMLPackage *umlPkg = c->getUMLPackage();
    QString className = cleanName(c->getName());
    QString retval;

    if (umlPkg == NULL) {
        retval = className;
        if (! isOOClass(c))
            retval.append(defaultPackageSuffix);
    } else {
        retval = umlPkg->getFullyQualifiedName(".");
        if (isOOClass(c)) {
            retval.append(".");
            retval.append(className);
        }
    }
    if (! withType)
        return retval;
    if (isOOClass(c)) {
        retval.append(".Object");
        if (! byValue)
            retval.append("_Ptr");
    } else {
        retval.append(".");
        retval.append(className);
    }
    return retval;
}

void PascalWriter::computeAssocTypeAndRole
(UMLAssociation *a, QString& typeName, QString& roleName) {
    roleName = a->getRoleName(Uml::A);
    if (roleName.isEmpty()) {
        if (a->getMulti(Uml::A).isEmpty()) {
            roleName = "M_";
            roleName.append(typeName);
        } else {
            roleName = typeName;
            roleName.append("_Vector");
        }
    }
    UMLClassifier* c = dynamic_cast<UMLClassifier*>(a->getObject(Uml::A));
    if (c == NULL)
        return;
    typeName = cleanName(c->getName());
    if (! a->getMulti(Uml::A).isEmpty())
        typeName.append("_Array_Access");
}

void PascalWriter::writeClass(UMLClassifier *c) {
    if (!c) {
        kdDebug() << "Cannot write class of NULL concept!" << endl;
        return;
    }

    const bool isClass = !c->isInterface();
    QString classname = cleanName(c->getName());
    QString fileName = qualifiedName(c).lower();
    fileName.replace('.', '-');

    //find an appropriate name for our file
    fileName = overwritableName(c, fileName, ".pas");
    if (fileName.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    QFile file;
    if (!openFile(file, fileName)) {
        emit codeGenerated(c, false);
        return;
    }

    // Start generating the code.

    QTextStream pas(&file);
    //try to find a heading file(license, comments, etc)
    QString str;
    str = getHeadingFile(".pas");
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), fileName);
        str.replace(QRegExp("%filepath%"), file.name());
        pas << str << endl;
    }

    QString unit = qualifiedName(c);
    pas << "unit " << unit << ";" << m_endl << m_endl;
    pas << "INTERFACE" << m_endl << m_endl;
    /* Use referenced classes.
    UMLClassifierList imports;
    findObjectsRelated(c, imports);
    if (imports.count()) {
        pas << "uses" << m_endl;
        bool first = true;
        for (UMLClassifier *con = imports.first(); con; con = imports.next()) {
            if (con->getBaseType() != Uml::ot_Datatype) {
                if (first)
                    first = false;
                else
                    pas << "," << m_endl;
                pas << "  " << qualifiedName(con);
            }
        }
        pas << ";" << m_endl << m_endl;
    }
     */

    /* Generate generic formals.
    UMLTemplateList template_params = c->getTemplateList();
    if (template_params.count()) {
        pas << getIndent() << "generic" << m_endl;
        m_indentLevel++;
        for (UMLTemplate *t = template_params.first(); t; t = template_params.next()) {
            QString formalName = t->getName();
            QString typeName = t->getTypeName();
            if (typeName == "class") {
                pas << getIndent() << "type " << formalName << " is tagged private;"
                << m_endl;
            } else {
                // Check whether it's a data type.
                UMLClassifier *typeObj = t->getType();
                if (typeObj == NULL) {
                    kdError() << "PascalWriter::writeClass(template_param "
                    << typeName << "): typeObj is NULL" << endl;
                    pas << getIndent() << "type " << formalName << " is new " << typeName
                    << " with private;  // CHECK: codegen error"
                    << m_endl;
                } else if (typeObj->getBaseType() == Uml::ot_Datatype) {
                    pas << getIndent() << formalName << " : " << typeName << ";"
                    << m_endl;
                } else {
                    pas << getIndent() << "type " << typeName << " is new "
                    << formalName << " with private;" << m_endl;
                }
            }
        }
        m_indentLevel--;
    }
     */

    pas << "type" << m_endl;
    m_indentLevel++;
    if (c->getBaseType() == Uml::ot_Enum) {
        UMLEnum *ue = static_cast<UMLEnum*>(c);
        UMLClassifierListItemList litList = ue->getFilteredList(Uml::ot_EnumLiteral);
        uint i = 0;
        pas << getIndent() << classname << " = (" << m_endl;
        m_indentLevel++;
        for (UMLClassifierListItem *lit = litList.first(); lit; lit = litList.next()) {
            QString enumLiteral = cleanName(lit->getName());
            pas << getIndent() << enumLiteral;
            if (++i < litList.count())
                pas << "," << m_endl;
        }
        m_indentLevel--;
        pas << ");" << m_endl << m_endl;
        m_indentLevel--;
        pas << "end." << m_endl << m_endl;
        return;
    }
    UMLAttributeList atl = c->getAttributeList();
    if (! isOOClass(c)) {
        QString stype = c->getStereotype();
        if (stype == "CORBAConstant") {
            pas << getIndent() << "// " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if(stype == "CORBAStruct") {
            if (isClass) {
                UMLAttribute *at;
                pas << getIndent() << classname << " = record" << m_endl;
                m_indentLevel++;
                for (at = atl.first(); at; at = atl.next()) {
                    QString name = cleanName(at->getName());
                    QString typeName = at->getTypeName();
                    pas << getIndent() << name << " : " << typeName;
                    QString initialVal = at->getInitialValue();
                    if (initialVal.latin1() && ! initialVal.isEmpty())
                        pas << " := " << initialVal;
                    pas << ";" << m_endl;
                }
                m_indentLevel--;
                pas << "end;" << m_endl << m_endl;
            }
        } else if(stype == "CORBAUnion") {
            pas << getIndent() << "// " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if(stype == "CORBATypedef") {
            pas << getIndent() << "// " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else {
            pas << getIndent() << "// " << stype << ": Unknown stereotype" << m_endl << m_endl;
        }
        m_indentLevel--;
        pas << getIndent() << "end." << m_endl << m_endl;
        return;
    }

    // Write class Documentation if non-empty or if force option set.
    if (forceDoc() || !c->getDoc().isEmpty()) {
        pas << "//" << m_endl;
        pas << "// class " << classname << endl;
        pas << formatDoc(c->getDoc(), "// ");
        pas << m_endl;
    }

    UMLClassifierList superclasses = c->getSuperClasses();

    pas << getIndent() << "TObject = object";
    if (!superclasses.isEmpty()) {
        // FIXME: Multiple inheritance is not yet supported
        UMLClassifier* parent = superclasses.first();
        pas << "(" << qualifiedName(parent) << ")";
    }
    pas << m_endl;

    if (isClass && (forceSections() || atl.count())) {
        pas << getIndent() << "// Attributes:" << m_endl;
        UMLAttribute *at;
        for (at = atl.first(); at; at = atl.next()) {
            // if (at->getStatic())
            //     continue;
            pas << getIndent() << cleanName(at->getName()) << " : "
                << at->getTypeName();
            if (at && at->getInitialValue().latin1() && ! at->getInitialValue().isEmpty())
                pas << " := " << at->getInitialValue();
            pas << ";" << m_endl;
        }
    }
    bool haveAttrs = (isClass && atl.count());

    // Generate public operations.
    UMLOperationList opl(c->getOpList());
    UMLOperationList oppub;
    oppub.setAutoDelete(false);
    UMLOperation *op;
    for (op = opl.first(); op; op = opl.next()) {
         if (op->getVisibility() == Uml::Visibility::Public)
            oppub.append(op);
    }
    if (forceSections() || oppub.count())
        pas << getIndent() << "// Public methods:" << m_endl << m_endl;
    for (op = oppub.first(); op; op = oppub.next())
        writeOperation(op, pas);

    pas << getIndent() << "end;" << m_endl << m_endl;

    pas << getIndent() << "PObject = ^TObject;" << m_endl << m_endl;

    m_indentLevel--;
    pas << "end;" << m_endl << m_endl;
    file.close();
    emit codeGenerated(c, true);
}


void PascalWriter::writeOperation(UMLOperation *op, QTextStream &pas, bool is_comment) {
    if (op->getStatic()) {
        pas << "// TODO: generate status method " << op->getName() << m_endl;
        return;
    }
    UMLAttributeList *atl = op->getParmList();
    QString rettype = op->getTypeName();
    bool use_procedure = (rettype.isEmpty() || rettype == "void");

    pas << getIndent();
    if (is_comment)
        pas << "// ";
    if (use_procedure)
        pas << "procedure ";
    else
        pas << "function ";
    pas << cleanName(op->getName()) << " ";
    if (atl->count()) {
        pas << "(" << m_endl;
        uint i = 0;
        m_indentLevel++;
        for (UMLAttribute *at = atl->first(); at; at = atl->next()) {
            pas << getIndent();
            if (is_comment)
                pas << "// ";
            pas << cleanName(at->getName()) << " : ";
            Uml::Parameter_Direction pk = at->getParmKind();
            if (pk != Uml::pd_In)
                pas << "var ";
            pas << at->getTypeName();
            if (! at->getInitialValue().isEmpty())
                pas << " := " << at->getInitialValue();
            if (++i < (uint)atl->count())
                pas << ";" << m_endl;
        }
        m_indentLevel--;
    }
    pas << ")";
    if (! use_procedure)
        pas << " : " << rettype;
    pas << "; virtual; abstract;" << m_endl << m_endl;
    // TBH, we make the methods abstract here because we don't have the means
    // for generating meaningful implementations.
}

QStringList PascalWriter::defaultDatatypes() {
    QStringList l;
    l.append("Boolean");
    l.append("Character");
    l.append("Cardinal");
    l.append("Integer");
    l.append("Shortint");
    l.append("Smallint");
    l.append("Longint");
    l.append("Byte");
    l.append("Word");
    l.append("Longword");
    l.append("Real");
    l.append("Single");
    l.append("Double");
    l.append("Extended");
    l.append("Currency");
    l.append("String");
    l.append("ShortString");
    l.append("AnsiString");
    l.append("WideString");
    return l;
}

/**
 * Check whether the given string is a reserved word for the
 * language of this code generator
 *
 * @param rPossiblyReservedKeyword  The string to check.
 */
bool PascalWriter::isReservedKeyword(const QString & rPossiblyReservedKeyword) {

    const QStringList keywords = reservedKeywords();

    QStringList::ConstIterator it;
    for (it = keywords.begin(); it != keywords.end(); ++it)
        if ((*it).lower() == rPossiblyReservedKeyword.lower())
            return true;

    return false;
}

/**
 * get list of reserved keywords
 */
const QStringList PascalWriter::reservedKeywords() const {

    static QStringList keywords;

    if ( keywords.isEmpty() ) {
        keywords.append( "abs" );
        keywords.append( "abstract" );
        keywords.append( "and" );
        keywords.append( "array" );
        keywords.append( "begin" );
        keywords.append( "Boolean" );
        keywords.append( "case" );
        keywords.append( "Character" );
        keywords.append( "declare" );
        keywords.append( "do" );
        keywords.append( "else" );
        keywords.append( "elsif" );
        keywords.append( "end" );
        keywords.append( "exit" );
        keywords.append( "false" );
        keywords.append( "for" );
        keywords.append( "function" );
        keywords.append( "goto" );
        keywords.append( "if" );
        keywords.append( "Integer" );
        keywords.append( "mod" );
        keywords.append( "new" );
        keywords.append( "not" );
        keywords.append( "null" );
        keywords.append( "object" );
        keywords.append( "or" );
        keywords.append( "otherwise" );
        keywords.append( "procedure" );
        keywords.append( "protected" );
        keywords.append( "record" );
        keywords.append( "rem" );
        keywords.append( "return" );
        keywords.append( "String" );
        keywords.append( "then" );
        keywords.append( "true" );
        keywords.append( "type" );
        keywords.append( "unit" );
        keywords.append( "use" );
        keywords.append( "while" );
        keywords.append( "xor" );
    }

    return keywords;
}

#include "pascalwriter.moc"
