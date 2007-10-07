/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "pascalwriter.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtextstream.h>

#include "../umldoc.h"
#include "../uml.h"
#include "../classifier.h"
#include "../enum.h"
#include "../classifierlistitem.h"
#include "../umlclassifierlistitemlist.h"
#include "../umltemplatelist.h"
#include "../folder.h"
#include "../association.h"
#include "../attribute.h"
#include "../operation.h"
#include "../template.h"
#include "../umlnamespace.h"
#include "classifierinfo.h"

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
        kDebug() << "PascalWriter::isOOClass: unknown object type " << ot << endl;
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

QString PascalWriter::qualifiedName(UMLPackage *p, bool withType, bool byValue) {
    UMLPackage *umlPkg = p->getUMLPackage();
    QString className = cleanName(p->getName());
    QString retval;

    if (umlPkg == UMLApp::app()->getDocument()->getRootFolder(Uml::mt_Logical))
        umlPkg = NULL;

    UMLClassifier *c = dynamic_cast<UMLClassifier*>(p);
    if (umlPkg == NULL) {
        retval = className;
        if (c == NULL || !isOOClass(c))
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
    if (c && isOOClass(c)) {
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
        kDebug() << "Cannot write class of NULL concept!" << endl;
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
    // Use referenced classes.
    UMLPackageList imports;
    findObjectsRelated(c, imports);
    if (imports.count()) {
        pas << "uses" << m_endl;
        bool first = true;
        for (UMLPackage *con = imports.first(); con; con = imports.next()) {
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

    pas << getIndent() << classname << " = object";
    if (!superclasses.isEmpty()) {
        // FIXME: Multiple inheritance is not yet supported
        UMLClassifier* parent = superclasses.first();
        pas << "(" << qualifiedName(parent) << ")";
    }
    pas << m_endl;

    ClassifierInfo info(c);
    UMLAttributeList atpub = info.atpub;
    if (isClass && (forceSections() || atpub.count())) {
        pas << getIndent() << "// Public attributes:" << m_endl;
        UMLAttribute *at;
        for (at = atpub.first(); at; at = atpub.next()) {
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

    if (info.atprot.count()) {
        pas << "protected" << m_endl << m_endl;
        UMLAttribute *at;
        UMLAttributeList atprot = info.atprot;
        for (at = atprot.first(); at; at = atprot.next()) {
            // if (at->getStatic())
            //     continue;
            pas << getIndent() << cleanName(at->getName()) << " : "
                << at->getTypeName();
            if (at && at->getInitialValue().latin1() && ! at->getInitialValue().isEmpty())
                pas << " := " << at->getInitialValue();
            pas << ";" << m_endl;
        }
        pas << m_endl;
    }
    if (info.atpriv.count()) {
        pas << "private" << m_endl << m_endl;
        UMLAttribute *at;
        UMLAttributeList atpriv = info.atpriv;
        for (at = atpriv.first(); at; at = atpriv.next()) {
            // if (at->getStatic())
            //     continue;
            pas << getIndent() << cleanName(at->getName()) << " : "
                << at->getTypeName();
            if (at && at->getInitialValue().latin1() && ! at->getInitialValue().isEmpty())
                pas << " := " << at->getInitialValue();
            pas << ";" << m_endl;
        }
        pas << m_endl;
    }
    pas << getIndent() << "end;" << m_endl << m_endl;

    pas << getIndent() << "P" << classname << " = ^" << classname <<";" << m_endl << m_endl;

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
    UMLAttributeList atl = op->getParmList();
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
    if (atl.count()) {
        pas << "(" << m_endl;
        uint i = 0;
        m_indentLevel++;
        for (UMLAttribute *at = atl.first(); at; at = atl.next()) {
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
            if (++i < (uint)atl.count())
                pas << ";" << m_endl;
        }
        m_indentLevel--;
        pas << ")";
    }
    if (! use_procedure)
        pas << " : " << rettype;
    pas << "; virtual; abstract;" << m_endl << m_endl;
    // TBH, we make the methods abstract here because we don't have the means
    // for generating meaningful implementations.
}

QStringList PascalWriter::defaultDatatypes() {
    QStringList l;
    l.append("AnsiString");
    l.append("Boolean");
    l.append("Byte");
    l.append("ByteBool");
    l.append("Cardinal");
    l.append("Character");
    l.append("Currency");
    l.append("Double");
    l.append("Extended");
    l.append("Int64");
    l.append("Integer");
    l.append("Longint");
    l.append("LongBool");
    l.append("Longword");
    l.append("QWord");
    l.append("Real");
    l.append("Shortint");
    l.append("ShortString");
    l.append("Single");
    l.append("Smallint");
    l.append("String");
    l.append("WideString");
    l.append("Word");
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
        keywords.append( "absolute" );
        keywords.append( "abstract" );
        keywords.append( "and" );
        keywords.append( "array" );
        keywords.append( "as" );
        keywords.append( "asm" );
        keywords.append( "assembler" );
        keywords.append( "automated" );
        keywords.append( "begin" );
        keywords.append( "case" );
        keywords.append( "cdecl" );
        keywords.append( "class" );
        keywords.append( "const" );
        keywords.append( "constructor" );
        keywords.append( "contains" );
        keywords.append( "default" );
        keywords.append( "deprecated" );
        keywords.append( "destructor" );
        keywords.append( "dispid" );
        keywords.append( "dispinterface" );
        keywords.append( "div" );
        keywords.append( "do" );
        keywords.append( "downto" );
        keywords.append( "dynamic" );
        keywords.append( "else" );
        keywords.append( "end" );
        keywords.append( "except" );
        keywords.append( "export" );
        keywords.append( "exports" );
        keywords.append( "external" );
        keywords.append( "far" );
        keywords.append( "file" );
        keywords.append( "final" );
        keywords.append( "finalization" );
        keywords.append( "finally" );
        keywords.append( "for" );
        keywords.append( "forward" );
        keywords.append( "function" );
        keywords.append( "goto" );
        keywords.append( "if" );
        keywords.append( "implementation" );
        keywords.append( "implements" );
        keywords.append( "in" );
        keywords.append( "index" );
        keywords.append( "inherited" );
        keywords.append( "initialization" );
        keywords.append( "inline" );
        keywords.append( "inline" );
        keywords.append( "interface" );
        keywords.append( "is" );
        keywords.append( "label" );
        keywords.append( "library" );
        keywords.append( "library" );
        keywords.append( "local" );
        keywords.append( "message" );
        keywords.append( "mod" );
        keywords.append( "name" );
        keywords.append( "near" );
        keywords.append( "nil" );
        keywords.append( "nodefault" );
        keywords.append( "not" );
        keywords.append( "object" );
        keywords.append( "of" );
        keywords.append( "or" );
        keywords.append( "out" );
        keywords.append( "overload" );
        keywords.append( "override" );
        keywords.append( "package" );
        keywords.append( "packed" );
        keywords.append( "pascal" );
        keywords.append( "platform" );
        keywords.append( "private" );
        keywords.append( "procedure" );
        keywords.append( "program" );
        keywords.append( "property" );
        keywords.append( "protected" );
        keywords.append( "public" );
        keywords.append( "published" );
        keywords.append( "raise" );
        keywords.append( "read" );
        keywords.append( "readonly" );
        keywords.append( "record" );
        keywords.append( "register" );
        keywords.append( "reintroduce" );
        keywords.append( "repeat" );
        keywords.append( "requires" );
        keywords.append( "resident" );
        keywords.append( "resourcestring" );
        keywords.append( "safecall" );
        keywords.append( "sealed" );
        keywords.append( "set" );
        keywords.append( "shl" );
        keywords.append( "shr" );
        keywords.append( "static" );
        keywords.append( "stdcall" );
        keywords.append( "stored" );
        keywords.append( "string" );
        keywords.append( "then" );
        keywords.append( "threadvar" );
        keywords.append( "to" );
        keywords.append( "try" );
        keywords.append( "type" );
        keywords.append( "unit" );
        keywords.append( "unsafe" );
        keywords.append( "until" );
        keywords.append( "uses" );
        keywords.append( "var" );
        keywords.append( "varargs" );
        keywords.append( "virtual" );
        keywords.append( "while" );
        keywords.append( "with" );
        keywords.append( "write" );
        keywords.append( "writeonly" );
        keywords.append( "xor" );
    }

    return keywords;
}

#include "pascalwriter.moc"
