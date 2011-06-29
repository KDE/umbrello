/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "pascalwriter.h"

#include "association.h"
#include "attribute.h"
#include "classifier.h"
#include "classifierlistitem.h"
#include "debug_utils.h"
#include "enum.h"
#include "folder.h"
#include "operation.h"
#include "template.h"
#include "uml.h"
#include "umlclassifierlistitemlist.h"
#include "umldoc.h"
#include "umltemplatelist.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>

const QString PascalWriter::defaultPackageSuffix = "_Holder";

PascalWriter::PascalWriter()
 : SimpleCodeGenerator()
{
}

PascalWriter::~PascalWriter()
{
}

Uml::ProgrammingLanguage PascalWriter::language() const
{
    return Uml::ProgrammingLanguage::Pascal;
}


bool PascalWriter::isOOClass(UMLClassifier *c)
{
    UMLObject::ObjectType ot = c->baseType();
    if (ot == UMLObject::ot_Interface)
        return true;
    if (ot == UMLObject::ot_Enum)
        return false;
    if (ot != UMLObject::ot_Class) {
        uDebug() << "unknown object type " << UMLObject::toString(ot);
        return false;
    }
    QString stype = c->stereotype();
    if (stype == "CORBAConstant" || stype == "CORBATypedef" ||
            stype == "CORBAStruct" || stype == "CORBAUnion")
        return false;
    // CORBAValue, CORBAInterface, and all empty/unknown stereotypes are
    // assumed to be OO classes.
    return true;
}

QString PascalWriter::qualifiedName(UMLPackage *p, bool withType, bool byValue)
{
    UMLPackage *umlPkg = p->umlPackage();
    QString className = cleanName(p->name());
    QString retval;

    if (umlPkg == UMLApp::app()->document()->rootFolder(Uml::ModelType::Logical))
        umlPkg = NULL;

    UMLClassifier *c = dynamic_cast<UMLClassifier*>(p);
    if (umlPkg == NULL) {
        retval = className;
        if (c == NULL || !isOOClass(c))
            retval.append(defaultPackageSuffix);
    } else {
        retval = umlPkg->fullyQualifiedName(".");
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
(UMLAssociation *a, QString& typeName, QString& roleName)
{
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
    typeName = cleanName(c->name());
    if (! a->getMulti(Uml::A).isEmpty())
        typeName.append("_Array_Access");
}

void PascalWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        uDebug() << "Cannot write class of NULL concept!";
        return;
    }

    const bool isClass = !c->isInterface();
    QString classname = cleanName(c->name());
    QString fileName = qualifiedName(c).toLower();
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
        str.replace(QRegExp("%filepath%"), file.fileName());
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
        foreach (UMLPackage* con, imports ) {
            if (con->baseType() != UMLObject::ot_Datatype) {
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
    if (c->baseType() == UMLObject::ot_Enum) {
        UMLEnum *ue = static_cast<UMLEnum*>(c);
        UMLClassifierListItemList litList = ue->getFilteredList(UMLObject::ot_EnumLiteral);
        uint i = 0;
        pas << indent() << classname << " = (" << m_endl;
        m_indentLevel++;
        foreach (UMLClassifierListItem *lit , litList ) {
            QString enumLiteral = cleanName(lit->name());
            pas << indent() << enumLiteral;
            if (++i < ( uint )litList.count())
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
        QString stype = c->stereotype();
        if (stype == "CORBAConstant") {
            pas << indent() << "// " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if(stype == "CORBAStruct") {
            if (isClass) {

                pas << indent() << classname << " = record" << m_endl;
                m_indentLevel++;
                foreach (UMLAttribute* at , atl ) {
                    QString name = cleanName(at->name());
                    QString typeName = at->getTypeName();
                    pas << indent() << name << " : " << typeName;
                    QString initialVal = at->getInitialValue();
                    if (!initialVal.isEmpty())
                        pas << " := " << initialVal;
                    pas << ";" << m_endl;
                }
                m_indentLevel--;
                pas << "end;" << m_endl << m_endl;
            }
        } else if(stype == "CORBAUnion") {
            pas << indent() << "// " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if(stype == "CORBATypedef") {
            pas << indent() << "// " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else {
            pas << indent() << "// " << stype << ": Unknown stereotype" << m_endl << m_endl;
        }
        m_indentLevel--;
        pas << indent() << "end." << m_endl << m_endl;
        return;
    }

    // Write class Documentation if non-empty or if force option set.
    if (forceDoc() || !c->doc().isEmpty()) {
        pas << "//" << m_endl;
        pas << "// class " << classname << endl;
        pas << formatDoc(c->doc(), "// ");
        pas << m_endl;
    }

    UMLClassifierList superclasses = c->getSuperClasses();

    pas << indent() << classname << " = object";
    if (!superclasses.isEmpty()) {
        // FIXME: Multiple inheritance is not yet supported
        UMLClassifier* parent = superclasses.first();
        pas << "(" << qualifiedName(parent) << ")";
    }
    pas << m_endl;

    UMLAttributeList atpub = c->getAttributeList(Uml::Visibility::Public);
    if (isClass && (forceSections() || atpub.count())) {
        pas << indent() << "// Public attributes:" << m_endl;

        foreach ( UMLAttribute* at  , atpub ) {
            // if (at->getStatic())
            //     continue;
            pas << indent() << cleanName(at->name()) << " : "
                << at->getTypeName();
            if (at && !at->getInitialValue().isEmpty())
                pas << " := " << at->getInitialValue();
            pas << ";" << m_endl;
        }
    }
    //bool haveAttrs = (isClass && atl.count());

    // Generate public operations.
    UMLOperationList opl(c->getOpList());
    UMLOperationList oppub;

    foreach (UMLOperation* op , opl ) {
         if (op->visibility() == Uml::Visibility::Public)
            oppub.append(op);
    }
    if (forceSections() || oppub.count())
        pas << indent() << "// Public methods:" << m_endl << m_endl;
    foreach (UMLOperation* op , oppub )
        writeOperation(op, pas);

    UMLAttributeList atprot = c->getAttributeList(Uml::Visibility::Protected);
    if (atprot.count()) {
        pas << "protected" << m_endl << m_endl;

        foreach (UMLAttribute*  at , atprot ) {
            // if (at->getStatic())
            //     continue;
            pas << indent() << cleanName(at->name()) << " : "
                << at->getTypeName();
            if (at && !at->getInitialValue().isEmpty())
                pas << " := " << at->getInitialValue();
            pas << ";" << m_endl;
        }
        pas << m_endl;
    }

    UMLAttributeList atpriv = c->getAttributeList(Uml::Visibility::Private);
    if (atpriv.count()) {
        pas << "private" << m_endl << m_endl;

        foreach (UMLAttribute* at , atpriv ) {
            // if (at->getStatic())
            //     continue;
            pas << indent() << cleanName(at->name()) << " : "
                << at->getTypeName();
            if (at && !at->getInitialValue().isEmpty())
                pas << " := " << at->getInitialValue();
            pas << ";" << m_endl;
        }
        pas << m_endl;
    }
    pas << indent() << "end;" << m_endl << m_endl;

    pas << indent() << "P" << classname << " = ^" << classname <<";" << m_endl << m_endl;

    m_indentLevel--;
    pas << "end;" << m_endl << m_endl;
    file.close();
    emit codeGenerated(c, true);
}

void PascalWriter::writeOperation(UMLOperation *op, QTextStream &pas, bool is_comment)
{
    if (op->isStatic()) {
        pas << "// TODO: generate status method " << op->name() << m_endl;
        return;
    }
    UMLAttributeList atl = op->getParmList();
    QString rettype = op->getTypeName();
    bool use_procedure = (rettype.isEmpty() || rettype == "void");

    pas << indent();
    if (is_comment)
        pas << "// ";
    if (use_procedure)
        pas << "procedure ";
    else
        pas << "function ";
    pas << cleanName(op->name()) << " ";
    if (atl.count()) {
        pas << "(" << m_endl;
        uint i = 0;
        m_indentLevel++;
        foreach (UMLAttribute *at , atl ) {
            pas << indent();
            if (is_comment)
                pas << "// ";
            pas << cleanName(at->name()) << " : ";
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
        pas << " : " << rettype << ";";

    QString sourceCode = op->getSourceCode();
    if (sourceCode.isEmpty()) {
        pas << " virtual; abstract;" << m_endl << m_endl;
        // TBH, we make the methods abstract here because we don't have the means
        // for generating meaningful implementations.
    }
    else {
        pas << m_endl;
        pas << indent() << "begin" << m_endl;
        m_indentLevel++;
        pas << formatSourceCode(sourceCode, indent());
        m_indentLevel--;
        pas << indent() << "end;" << m_endl << m_endl;
    }
}

QStringList PascalWriter::defaultDatatypes()
{
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
 * language of this code generator.
 *
 * @param rPossiblyReservedKeyword  The string to check.
 */
bool PascalWriter::isReservedKeyword(const QString & rPossiblyReservedKeyword)
{
    const QStringList keywords = reservedKeywords();

    QStringList::ConstIterator it;
    for (it = keywords.begin(); it != keywords.end(); ++it)
        if ((*it).toLower() == rPossiblyReservedKeyword.toLower())
            return true;

    return false;
}

/**
 * Get list of reserved keywords.
 */
QStringList PascalWriter::reservedKeywords() const
{
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
