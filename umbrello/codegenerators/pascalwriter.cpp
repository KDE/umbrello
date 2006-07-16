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
    fileName = overwritableName(c, fileName, ".ads");
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
    str = getHeadingFile(".ads");
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), fileName);
        str.replace(QRegExp("%filepath%"), file.name());
        pas << str << endl;
    }

    // Import referenced classes.
    UMLClassifierList imports;
    findObjectsRelated(c, imports);
    if (imports.count()) {
        for (UMLClassifier *con = imports.first(); con; con = imports.next()) {
            if (con->getBaseType() != Uml::ot_Datatype)
                pas << "with " << qualifiedName(con) << "; " << m_endl;
        }
        pas << m_endl;
    }

    // Generate generic formals.
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
                    << " with private;  -- CHECK: codegen error"
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

    // Here comes the package proper.
    QString pkg = qualifiedName(c);
    pas << getIndent() << "package " << pkg << " is" << m_endl << m_endl;
    m_indentLevel++;
    if (c->getBaseType() == Uml::ot_Enum) {
        UMLEnum *ue = static_cast<UMLEnum*>(c);
        UMLClassifierListItemList litList = ue->getFilteredList(Uml::ot_EnumLiteral);
        uint i = 0;
        pas << getIndent() << "type " << classname << " is (" << m_endl;
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
        pas << getIndent() << "end " << pkg << ";" << m_endl << m_endl;
        return;
    }
    if (! isOOClass(c)) {
        QString stype = c->getStereotype();
        if (stype == "CORBAConstant") {
            pas << getIndent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if(stype == "CORBAStruct") {
            if (isClass) {
                UMLAttributeList atl = c->getAttributeList();
                UMLAttribute *at;
                pas << getIndent() << "type " << classname << " is record" << m_endl;
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
                pas << getIndent() << "end record;" << m_endl << m_endl;
            }
        } else if(stype == "CORBAUnion") {
            pas << getIndent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if(stype == "CORBATypedef") {
            pas << getIndent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else {
            pas << getIndent() << "-- " << stype << ": Unknown stereotype" << m_endl << m_endl;
        }
        m_indentLevel--;
        pas << getIndent() << "end " << pkg << ";" << m_endl << m_endl;
        return;
    }

    // Write class Documentation if non-empty or if force option set.
    if (forceDoc() || !c->getDoc().isEmpty()) {
        pas << "--" << m_endl;
        pas << "-- class " << classname << endl;
        pas << formatDoc(c->getDoc(), "-- ");
        pas << m_endl;
    }

    UMLClassifierList superclasses = c->getSuperClasses();

    pas << getIndent() << "type Object is ";
    if (c->getAbstract())
        pas << "abstract ";
    if (superclasses.isEmpty()) {
        pas << "tagged ";
    } else {
        // FIXME: Multiple inheritance is not yet supported
        UMLClassifier* parent = superclasses.first();
        pas << "new " << qualifiedName(parent) << ".Object with ";
    }
    pas << "private;" << m_endl << m_endl;
    pas << getIndent() << "type Object_Ptr is access all Object'Class;" << m_endl << m_endl;

    // Generate accessors for public attributes.
    UMLAttributeList atl;
    if (isClass) {
        UMLAttributeList atpub;
        atpub.setAutoDelete(false);

        atl = c->getAttributeList();

        UMLAttribute *at;
        for (at = atl.first(); at; at = atl.next()) {
              if (at->getVisibility() == Uml::Visibility::Public)
                atpub.append(at);
        }
        if (forceSections() || atpub.count())
            pas << getIndent() << "-- Accessors for public attributes:" << m_endl << m_endl;
        for (at = atpub.first(); at; at = atpub.next()) {
            QString member = cleanName(at->getName());
            pas << getIndent() << "procedure Set_" << member << " (";
            if (! at->getStatic())
                pas << "Self : access Object; ";
            pas << "To : " << at->getTypeName() << ");" << m_endl;
            pas << getIndent() << "function  Get_" << member;
            if (! at->getStatic())
                pas << " (Self : access Object)";
            pas << " return " << at->getTypeName() << ";" << m_endl
            << m_endl;
        }
    }

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
        pas << getIndent() << "-- Public methods:" << m_endl << m_endl;
    for (op = oppub.first(); op; op = oppub.next())
        writeOperation(op, pas);

    m_indentLevel--;
    pas << getIndent() << "private" << m_endl << m_endl;
    m_indentLevel++;

    // Generate auxiliary declarations for multiplicity of associations
    UMLAssociationList aggregations = c->getAggregations();
    if (!aggregations.isEmpty()) {
        for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next()) {
            if (a->getMulti(Uml::A).isEmpty())
                continue;
            UMLClassifier* other = (UMLClassifier*)a->getObject(Uml::A);
            QString member = cleanName(other->getName());
            // Handling of packages is missing here
            // A test and error action is missing here for !isOOClass()
            pas << getIndent() << "type " << member << "_Array is array"
            << " (Positive range <>) of " << member << ".Object_Ptr;" << m_endl;
            pas << getIndent() << "type " << member << "_Array_Access is access "
            << member << "_array;" << m_endl << m_endl;
        }
    }
    UMLAssociationList compositions = c->getCompositions();
    if (!compositions.isEmpty()) {
        for (UMLAssociation *a = compositions.first(); a; a = compositions.next()) {
            if (a->getMulti(Uml::A).isEmpty())
                continue;
            UMLObject *other = a->getObject(Uml::A);
            QString member = cleanName(other->getName());
            // Handling of packages is missing here
            // Treatment of !isOOClass() is missing here
            pas << getIndent() << "type " << member << "_Array is array"
            << " (Positive range <>) of " << member << ".Object;" << m_endl;
            pas << getIndent() << "type " << member << "_Array_Access is access "
            << member << "_array;" << m_endl << m_endl;
        }
    }

    pas << getIndent() << "type Object is ";
    if (c->getAbstract())
        pas << "abstract ";
    if (superclasses.isEmpty()) {
        pas << "tagged ";
    } else {
        // FIXME: Multiple inheritance is not yet supported
        UMLClassifier* parent = superclasses.first();
        pas << "new " << qualifiedName(parent) << ".Object with ";
    }
    pas << "record" << m_endl;
    m_indentLevel++;

    if (forceSections() || !aggregations.isEmpty()) {
        pas << getIndent() << "-- Aggregations:" << m_endl;
        for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next()) {
            QString typeName, roleName;
            computeAssocTypeAndRole(a, typeName, roleName);
            pas << getIndent() << roleName << " : " << typeName << ";" << m_endl;
        }
        pas << endl;
    }
    if (forceSections() || !compositions.isEmpty()) {
        pas << getIndent() << "-- Compositions:" << m_endl;
        for (UMLAssociation *a = compositions.first(); a; a = compositions.next()) {
            QString typeName, roleName;
            computeAssocTypeAndRole(a, typeName, roleName);
            pas << getIndent() << roleName << " : " << typeName << ";" << m_endl;
        }
        pas << endl;
    }

    if (isClass && (forceSections() || atl.count())) {
        pas << getIndent() << "-- Attributes:" << m_endl;
        UMLAttribute *at;
        for (at = atl.first(); at; at = atl.next()) {
            if (at->getStatic())
                continue;
            pas << getIndent() << cleanName(at->getName()) << " : "
            << at->getTypeName();
            if (at && at->getInitialValue().latin1() && ! at->getInitialValue().isEmpty())
                pas << " := " << at->getInitialValue();
            pas << ";" << m_endl;
        }
    }
    bool haveAttrs = (isClass && atl.count());
    if (aggregations.isEmpty() && compositions.isEmpty() && !haveAttrs)
        pas << getIndent() << "null;" << m_endl;
    m_indentLevel--;
    pas << getIndent() << "end record;" << m_endl << m_endl;
    if (haveAttrs) {
        bool seen_static_attr = false;
        for (UMLAttribute *at = atl.first(); at; at = atl.next()) {
            if (! at->getStatic())
                continue;
            if (! seen_static_attr) {
                pas << getIndent() << "-- Static attributes:" << m_endl;
                seen_static_attr = true;
            }
            pas << getIndent();
            if (at->getVisibility() == Uml::Visibility::Private)
                pas << "-- Private:  ";
            pas << cleanName(at->getName()) << " : " << at->getTypeName();
            if (at && at->getInitialValue().latin1() && ! at->getInitialValue().isEmpty())
                pas << " := " << at->getInitialValue();
            pas << ";" << m_endl;
        }
        if (seen_static_attr)
            pas << m_endl;
    }
    // Generate protected operations.
    UMLOperationList opprot;
    opprot.setAutoDelete(false);
    for (op = opl.first(); op; op = opl.next()) {
      if (op->getVisibility() == Uml::Visibility::Protected)
            opprot.append(op);
    }
    if (forceSections() || opprot.count())
        pas << getIndent() << "-- Protected methods:" << m_endl << m_endl;
    for (op = opprot.first(); op; op = opprot.next())
        writeOperation(op, pas);

    // Generate private operations.
    // These are currently only generated as comments in the private part
    // of the spec.
    // Once umbrello supports the merging of automatically generated and
    // hand written code sections, private operations should be generated
    // into the package body.
    UMLOperationList oppriv;
    oppriv.setAutoDelete(false);
    for (op = opl.first(); op; op = opl.next()) {
          const Uml::Visibility::Value vis = op->getVisibility();
          if (vis == Uml::Visibility::Private ||
              vis == Uml::Visibility::Implementation)
            oppriv.append(op);
    }
    if (forceSections() || oppriv.count())
        pas << getIndent() << "-- Private methods:" << m_endl << m_endl;
    for (op = oppriv.first(); op; op = oppriv.next())
        writeOperation(op, pas, true);

    m_indentLevel--;
    pas << getIndent() << "end " << pkg << ";" << m_endl << m_endl;
    file.close();
    emit codeGenerated(c, true);
}


void PascalWriter::writeOperation(UMLOperation *op, QTextStream &pas, bool is_comment) {
    UMLAttributeList *atl = op->getParmList();
    QString rettype = op->getTypeName();
    bool use_procedure = (rettype.isEmpty() || rettype == "void");

    pas << getIndent();
    if (is_comment)
        pas << "-- ";
    if (use_procedure)
        pas << "procedure ";
    else
        pas << "function ";
    pas << cleanName(op->getName()) << " ";
    if (! (op->getStatic() && atl->count() == 0))
        pas << "(";
    if (! op->getStatic()) {
        pas << "Self : access Object";
        if (atl->count())
            pas << ";" << m_endl;
    }
    if (atl->count()) {
        uint i = 0;
        m_indentLevel++;
        for (UMLAttribute *at = atl->first(); at; at = atl->next()) {
            pas << getIndent();
            if (is_comment)
                pas << "-- ";
            pas << cleanName(at->getName()) << " : ";
            Uml::Parameter_Direction pk = at->getParmKind();
            if (pk == Uml::pd_Out)
                pas << "out ";
            else if (pk == Uml::pd_InOut)
                pas << "in out ";
            else
                pas << "in ";
            pas << at->getTypeName();
            if (! at->getInitialValue().isEmpty())
                pas << " := " << at->getInitialValue();
            if (++i < atl->count()) //FIXME gcc warning
                pas << ";" << m_endl;
        }
        m_indentLevel--;
    }
    if (! (op->getStatic() && atl->count() == 0))
        pas << ")";
    if (! use_procedure)
        pas << " return " << rettype;
    pas << " is abstract;" << m_endl << m_endl;
    // TBH, we make the methods abstract here because we don't have the means
    // for generating meaningful implementations.
}

QStringList PascalWriter::defaultDatatypes() {
    QStringList l;
    l.append("Boolean");
    l.append("Character");
    l.append("Positive");
    l.append("Natural");
    l.append("Integer");
    l.append("Short_Integer");
    l.append("Long_Integer");
    l.append("Float");
    l.append("Long_Float");
    l.append("String");
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
        keywords.append( "abort" );
        keywords.append( "abs" );
        keywords.append( "abstract" );
        keywords.append( "accept" );
        keywords.append( "access" );
        keywords.append( "aliased" );
        keywords.append( "all" );
        keywords.append( "and" );
        keywords.append( "Argument_Error" );
        keywords.append( "array" );
        keywords.append( "Assert_Failure" );
        keywords.append( "at" );
        keywords.append( "begin" );
        keywords.append( "body" );
        keywords.append( "Boolean" );
        keywords.append( "case" );
        keywords.append( "Character" );
        keywords.append( "constant" );
        keywords.append( "Constraint_Error" );
        keywords.append( "Conversion_Error" );
        keywords.append( "Data_Error" );
        keywords.append( "declare" );
        keywords.append( "delay" );
        keywords.append( "delta" );
        keywords.append( "Dereference_Error" );
        keywords.append( "Device_Error" );
        keywords.append( "digits" );
        keywords.append( "do" );
        keywords.append( "Duration" );
        keywords.append( "else" );
        keywords.append( "elsif" );
        keywords.append( "end" );
        keywords.append( "End_Error" );
        keywords.append( "entry" );
        keywords.append( "exception" );
        keywords.append( "exit" );
        keywords.append( "false" );
        keywords.append( "Float" );
        keywords.append( "for" );
        keywords.append( "function" );
        keywords.append( "generic" );
        keywords.append( "goto" );
        keywords.append( "if" );
        keywords.append( "in" );
        keywords.append( "Index_Error" );
        keywords.append( "Integer" );
        keywords.append( "is" );
        keywords.append( "Layout_Error" );
        keywords.append( "Length_Error" );
        keywords.append( "limited" );
        keywords.append( "Long_Float" );
        keywords.append( "Long_Integer" );
        keywords.append( "Long_Long_Float" );
        keywords.append( "Long_Long_Integer" );
        keywords.append( "loop" );
        keywords.append( "mod" );
        keywords.append( "Mode_Error" );
        keywords.append( "Name_Error" );
        keywords.append( "Natural" );
        keywords.append( "new" );
        keywords.append( "not" );
        keywords.append( "null" );
        keywords.append( "of" );
        keywords.append( "or" );
        keywords.append( "others" );
        keywords.append( "out" );
        keywords.append( "package" );
        keywords.append( "Pattern_Error" );
        keywords.append( "Picture_Error" );
        keywords.append( "Pointer_Error" );
        keywords.append( "Positive" );
        keywords.append( "pragma" );
        keywords.append( "private" );
        keywords.append( "procedure" );
        keywords.append( "Program_Error" );
        keywords.append( "protected" );
        keywords.append( "raise" );
        keywords.append( "range" );
        keywords.append( "record" );
        keywords.append( "rem" );
        keywords.append( "renames" );
        keywords.append( "requeue" );
        keywords.append( "return" );
        keywords.append( "reverse" );
        keywords.append( "select" );
        keywords.append( "separate" );
        keywords.append( "Short_Float" );
        keywords.append( "Short_Integer" );
        keywords.append( "Short_Short_Float" );
        keywords.append( "Short_Short_Integer" );
        keywords.append( "Status_Error" );
        keywords.append( "Storage_Error" );
        keywords.append( "String" );
        keywords.append( "subtype" );
        keywords.append( "Tag_Error" );
        keywords.append( "tagged" );
        keywords.append( "task" );
        keywords.append( "Tasking_Error" );
        keywords.append( "terminate" );
        keywords.append( "Terminator_Error" );
        keywords.append( "then" );
        keywords.append( "Time_Error" );
        keywords.append( "Translation_Error" );
        keywords.append( "true" );
        keywords.append( "type" );
        keywords.append( "until" );
        keywords.append( "Update_Error" );
        keywords.append( "use" );
        keywords.append( "Use_Error" );
        keywords.append( "when" );
        keywords.append( "while" );
        keywords.append( "Wide_Character" );
        keywords.append( "Wide_String" );
        keywords.append( "with" );
        keywords.append( "xor" );
    }

    return keywords;
}

#include "pascalwriter.moc"
