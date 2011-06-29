/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002    Oliver Kellogg <okellogg@users.sourceforge.net> *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "adawriter.h"

#include "debug_utils.h"
#include "umldoc.h"
#include "uml.h"
#include "classifier.h"
#include "enum.h"
#include "classifierlistitem.h"
#include "umlclassifierlistitemlist.h"
#include "umltemplatelist.h"
#include "folder.h"
#include "association.h"
#include "attribute.h"
#include "operation.h"
#include "template.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>

const QString AdaWriter::defaultPackageSuffix = "_Holder";

/**
 * Basic Constructor
 */
AdaWriter::AdaWriter()
 : SimpleCodeGenerator()
{
}

/**
 * Empty Destructor
 */
AdaWriter::~AdaWriter()
{
}

/**
 * Returns "Ada".
 * @return   the programming language identifier
 */
Uml::ProgrammingLanguage AdaWriter::language() const
{
    return Uml::ProgrammingLanguage::Ada;
}

/**
 * ...
 */
bool AdaWriter::isOOClass(UMLClassifier *c)
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

/**
 * Returns the class name.
 */
QString AdaWriter::className(UMLClassifier *c, bool inOwnScope)
{
    // If the class has an enclosing package then it is assumed that
    // the class name is the type name; if the class does not have an
    // enclosing package then the class name acts as the Ada package
    // name.
    QString retval;
    QString className = cleanName(c->name());
    UMLPackage *umlPkg = c->umlPackage();
    if (umlPkg == UMLApp::app()->document()->rootFolder(Uml::ModelType::Logical)) {
        if (! inOwnScope)
            retval = className + '.';
        retval.append("Object");
    } else {
        if (! inOwnScope)
            retval = umlPkg->fullyQualifiedName(".") + '.';
        retval.append(className);
    }
    return retval;
}

/**
 * Returns the package name.
 */
QString AdaWriter::packageName(UMLPackage *p)
{
    // If the class has an enclosing package then it is assumed that
    // the class name is the type name; if the class does not have an
    // enclosing package then the class name acts as the Ada package
    // name.
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
    }
    return retval;
}

/**
 * Compute the type and role name from the given association.
 *
 * @param c         The UMLClassifier for which code is being generated.
 * @param a         The UMLAssociation to analyze.
 * @param typeName  Return value: type name.
 * @param roleName  Return value: role name.
 */
void AdaWriter::computeAssocTypeAndRole(UMLClassifier *c,
                                        UMLAssociation *a,
                                        QString& typeName, QString& roleName)
{
    UMLClassifier* assocEnd = dynamic_cast<UMLClassifier*>(a->getObject(Uml::B));
    if (assocEnd == NULL)
        return;
    const Uml::AssociationType assocType = a->getAssocType();
    if (assocType != Uml::AssociationType::Aggregation && assocType != Uml::AssociationType::Composition)
        return;
    const QString multi = a->getMulti(Uml::B);
    bool hasNonUnityMultiplicity = (!multi.isEmpty() && multi != "1");
    hasNonUnityMultiplicity &= !multi.contains(QRegExp("^1 *\\.\\. *1$"));
    roleName = cleanName(a->getRoleName(Uml::B));
    if (roleName.isEmpty())
        roleName = cleanName(a->name());
    if (roleName.isEmpty()) {
        QString artificialName = cleanName(assocEnd->name());
        if (hasNonUnityMultiplicity) {
            roleName = artificialName;
            roleName.append("_Vector");
        } else {
            roleName = "M_";
            roleName.append(artificialName);
        }
    }
    typeName = className(assocEnd, (assocEnd == c));
    if (hasNonUnityMultiplicity)
        typeName.append("_Array_Ptr");
    else if (assocType == Uml::AssociationType::Aggregation)
        typeName.append("_Ptr");
}

/**
 * Call this method to generate Ada code for a UMLClassifier.
 * @param c the class to generate code for
 */
void AdaWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        uDebug() << "Cannot write class of NULL concept!";
        return;
    }

    const bool isClass = !c->isInterface();
    QString classname = cleanName(c->name());
    QString fileName = packageName(c).toLower();
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

    QTextStream ada(&file);
    //try to find a heading file(license, comments, etc)
    QString str;
    str = getHeadingFile(".ads");
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), fileName);
        str.replace(QRegExp("%filepath%"), file.fileName());
        ada << str << endl;
    }

    // Import referenced classes.
    UMLPackageList imports;
    findObjectsRelated(c, imports);
    if (imports.count()) {
        foreach (UMLPackage* con, imports ) {
            if (con->baseType() != UMLObject::ot_Datatype)
                ada << "with " << packageName(con) << "; " << m_endl;
        }
        ada << m_endl;
    }

    // Generate generic formals.
    UMLTemplateList template_params = c->getTemplateList();
    if (template_params.count()) {
        ada << indent() << "generic" << m_endl;
        m_indentLevel++;
        foreach (UMLTemplate* t, template_params ) {
            QString formalName = t->name();
            QString typeName = t->getTypeName();
            if (typeName == "class") {
                ada << indent() << "type " << formalName << " is tagged private;"
                << m_endl;
            } else {
                // Check whether it's a data type.
                UMLClassifier *typeObj = t->getType();
                if (typeObj == NULL) {
                    uError() << "template_param " << typeName << ": typeObj is NULL";
                    ada << indent() << "type " << formalName << " is new " << typeName
                    << " with private;  -- CHECK: codegen error"
                    << m_endl;
                } else if (typeObj->baseType() == UMLObject::ot_Datatype) {
                    ada << indent() << formalName << " : " << typeName << ";"
                    << m_endl;
                } else {
                    ada << indent() << "type " << typeName << " is new "
                    << formalName << " with private;" << m_endl;
                }
            }
        }
        m_indentLevel--;
    }

    // Here comes the package proper.
    QString pkg = packageName(c);
    ada << indent() << "package " << pkg << " is" << m_endl << m_endl;
    m_indentLevel++;
    if (c->baseType() == UMLObject::ot_Enum) {
        UMLEnum *ue = static_cast<UMLEnum*>(c);
        UMLClassifierListItemList litList = ue->getFilteredList(UMLObject::ot_EnumLiteral);
        uint i = 0;
        ada << indent() << "type " << classname << " is (" << m_endl;
        m_indentLevel++;
        foreach (UMLClassifierListItem* lit, litList ) {
            QString enumLiteral = cleanName(lit->name());
            ada << indent() << enumLiteral;
            if (++i < ( uint )litList.count())
                ada << "," << m_endl;
        }
        m_indentLevel--;
        ada << ");" << m_endl << m_endl;
        m_indentLevel--;
        ada << indent() << "end " << pkg << ";" << m_endl << m_endl;
        return;
    }
    if (! isOOClass(c)) {
        QString stype = c->stereotype();
        if (stype == "CORBAConstant") {
            ada << indent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if(stype == "CORBAStruct") {
            if (isClass) {
                UMLAttributeList atl = c->getAttributeList();
                ada << indent() << "type " << classname << " is record" << m_endl;
                m_indentLevel++;
                foreach (UMLAttribute* at,  atl ) {
                    QString name = cleanName(at->name());
                    QString typeName = at->getTypeName();
                    ada << indent() << name << " : " << typeName;
                    QString initialVal = at->getInitialValue();
                    if (! initialVal.isEmpty() && ! initialVal.toLatin1().isEmpty())
                        ada << " := " << initialVal;
                    ada << ";" << m_endl;
                }
                m_indentLevel--;
                ada << indent() << "end record;" << m_endl << m_endl;
            }
        } else if(stype == "CORBAUnion") {
            ada << indent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if(stype == "CORBATypedef") {
            ada << indent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else {
            ada << indent() << "-- " << stype << ": Unknown stereotype" << m_endl << m_endl;
        }
        m_indentLevel--;
        ada << indent() << "end " << pkg << ";" << m_endl << m_endl;
        return;
    }

    // Write class Documentation if non-empty or if force option set.
    if (forceDoc() || !c->doc().isEmpty()) {
        ada << "--" << m_endl;
        ada << "-- class " << classname << endl;
        ada << formatDoc(c->doc(), "-- ");
        ada << m_endl;
    }

    UMLClassifierList superclasses = c->getSuperClasses();

    const QString name = className(c);
    ada << indent() << "type " << name << " is ";
    if (c->isAbstract())
        ada << "abstract ";
    if (superclasses.isEmpty()) {
        ada << "tagged ";
    } else {
        // FIXME: Multiple inheritance is not yet supported
        UMLClassifier* parent = superclasses.first();
        ada << "new " << className(parent, false) << " with ";
    }
    ada << "private;" << m_endl << m_endl;
    ada << indent() << "type " << name << "_Ptr is access all " << name << "'Class;" << m_endl << m_endl;
    ada << indent() << "type " << name << "_Array is array (Positive range <>) of " << name << "_Ptr;" << m_endl << m_endl;
    ada << indent() << "type " << name << "_Array_Ptr is access " << name << "_Array;" << m_endl << m_endl;

    // Generate accessors for public attributes.
    UMLAttributeList atl;
    if (isClass) {
        UMLAttributeList atpub;

        atl = c->getAttributeList();

        foreach (UMLAttribute* at, atl ) {
            if (at->visibility() == Uml::Visibility::Public)
                atpub.append(at);
        }
        if (forceSections() || atpub.count())
            ada << indent() << "-- Accessors for public attributes:" << m_endl << m_endl;

        foreach (UMLAttribute* at, atpub ) {
            QString member = cleanName(at->name());
            ada << indent() << "procedure Set_" << member << " (";
            if (! at->isStatic())
                ada << "Self : access " << name << "; ";
            ada << "To : " << at->getTypeName() << ");" << m_endl;
            ada << indent() << "function  Get_" << member;
            if (! at->isStatic())
                ada << " (Self : access " << name << ")";
            ada << " return " << at->getTypeName() << ";" << m_endl
            << m_endl;
        }
    }

    // Generate public operations.
    UMLOperationList opl(c->getOpList());
    UMLOperationList oppub;
    foreach (UMLOperation* op, opl ) {
        if (op->visibility() == Uml::Visibility::Public)
            oppub.append(op);
    }
    if (forceSections() || oppub.count())
        ada << indent() << "-- Public methods:" << m_endl << m_endl;

    foreach (UMLOperation* op, oppub ) {
        writeOperation(op, ada);
    }

    m_indentLevel--;
    ada << indent() << "private" << m_endl << m_endl;
    m_indentLevel++;

    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();

    ada << indent() << "type " << name << " is ";
    if (c->isAbstract())
        ada << "abstract ";
    if (superclasses.isEmpty()) {
        ada << "tagged ";
    } else {
        // FIXME: Multiple inheritance is not yet supported
        UMLClassifier* parent = superclasses.first();
        ada << "new " << className(parent, false) << " with ";
    }
    ada << "record" << m_endl;
    m_indentLevel++;

    if (forceSections() || !aggregations.isEmpty()) {
        ada << indent() << "-- Aggregations:" << m_endl;
        foreach (UMLAssociation *a , aggregations) {
            if (c != a->getObject(Uml::A))
                continue;
            QString typeName, roleName;
            computeAssocTypeAndRole(c, a, typeName, roleName);
            ada << indent() << roleName << " : " << typeName << ";" << m_endl;
        }
        ada << endl;
    }
    if (forceSections() || !compositions.isEmpty()) {
        ada << indent() << "-- Compositions:" << m_endl;
        foreach (UMLAssociation *a , compositions ) {
            if (c != a->getObject(Uml::A))
                continue;
            QString typeName, roleName;
            computeAssocTypeAndRole(c, a, typeName, roleName);
            ada << indent() << roleName << " : " << typeName << ";" << m_endl;
        }
        ada << endl;
    }

    if (isClass && (forceSections() || atl.count())) {
        ada << indent() << "-- Attributes:" << m_endl;
        foreach (UMLAttribute* at, atl ) {
            if (at->isStatic())
                continue;
            ada << indent() << cleanName(at->name()) << " : "
            << at->getTypeName();
            if (at && ! at->getInitialValue().isEmpty() && ! at->getInitialValue().toLatin1().isEmpty())
                ada << " := " << at->getInitialValue();
            ada << ";" << m_endl;
        }
    }
    bool haveAttrs = (isClass && atl.count());
    if (aggregations.isEmpty() && compositions.isEmpty() && !haveAttrs)
        ada << indent() << "null;" << m_endl;
    m_indentLevel--;
    ada << indent() << "end record;" << m_endl << m_endl;
    if (haveAttrs) {
        bool seen_static_attr = false;
        foreach (UMLAttribute* at, atl ) {
            if (! at->isStatic())
                continue;
            if (! seen_static_attr) {
                ada << indent() << "-- Static attributes:" << m_endl;
                seen_static_attr = true;
            }
            ada << indent();
            if (at->visibility() == Uml::Visibility::Private)
                ada << "-- Private:  ";
            ada << cleanName(at->name()) << " : " << at->getTypeName();
            if (at && ! at->getInitialValue().isEmpty() && ! at->getInitialValue().toLatin1().isEmpty() )
                ada << " := " << at->getInitialValue();
            ada << ";" << m_endl;
        }
        if (seen_static_attr)
            ada << m_endl;
    }
    // Generate protected operations.
    UMLOperationList opprot;
    foreach (UMLOperation* op,  opl ) {
        if (op->visibility() == Uml::Visibility::Protected)
            opprot.append(op);
    }
    if (forceSections() || opprot.count())
        ada << indent() << "-- Protected methods:" << m_endl << m_endl;
    foreach (UMLOperation* op, opprot ) {
        writeOperation(op, ada);
    }

    // Generate private operations.
    // These are currently only generated as comments in the private part
    // of the spec.
    // Once umbrello supports the merging of automatically generated and
    // hand written code sections, private operations should be generated
    // into the package body.
    UMLOperationList oppriv;
    foreach (UMLOperation* op, opl ) {
        const Uml::Visibility::Value vis = op->visibility();
        if (vis == Uml::Visibility::Private ||
            vis == Uml::Visibility::Implementation)
        oppriv.append(op);
    }
    if (forceSections() || oppriv.count())
        ada << indent() << "-- Private methods:" << m_endl << m_endl;
    foreach (UMLOperation* op, oppriv ) {
        writeOperation(op, ada, true);
    }

    m_indentLevel--;
    ada << indent() << "end " << pkg << ";" << m_endl << m_endl;
    file.close();
    emit codeGenerated(c, true);
}

/**
 * Write one operation.
 * @param op          the class for which we are generating code
 * @param ada         the stream associated with the output file
 * @param is_comment  flag for a comment
 */
void AdaWriter::writeOperation(UMLOperation *op, QTextStream &ada, bool is_comment)
{
    UMLAttributeList atl = op->getParmList();
    QString rettype = op->getTypeName();
    bool use_procedure = (rettype.isEmpty() || rettype == "void");

    ada << indent();
    if (is_comment)
        ada << "-- ";
    if (use_procedure)
        ada << "procedure ";
    else
        ada << "function ";
    ada << cleanName(op->name()) << " ";
    if (! (op->isStatic() && atl.count() == 0))
        ada << "(";
    UMLClassifier *parentClassifier = static_cast<UMLClassifier*>(op->umlPackage());
    if (! op->isStatic()) {
        ada << "Self : access " << className(parentClassifier);
        if (atl.count())
            ada << ";" << m_endl;
    }
    if (atl.count()) {
        uint i = 0;
        m_indentLevel++;
        foreach (UMLAttribute* at, atl ) {
            ada << indent();
            if (is_comment)
                ada << "-- ";
            ada << cleanName(at->name()) << " : ";
            Uml::Parameter_Direction pk = at->getParmKind();
            if (pk == Uml::pd_Out)
                ada << "out ";
            else if (pk == Uml::pd_InOut)
                ada << "in out ";
            else
                ada << "in ";
            ada << at->getTypeName();
            if (! at->getInitialValue().isEmpty())
                ada << " := " << at->getInitialValue();
            if (++i < ( uint )atl.count()) //FIXME gcc warning
                ada << ";" << m_endl;
        }
        m_indentLevel--;
    }
    if (! (op->isStatic() && atl.count() == 0))
        ada << ")";
    if (! use_procedure)
        ada << " return " << rettype;
    if (op->isAbstract())
        ada << " is abstract";
    ada << ";" << m_endl << m_endl;
}

/**
 * Returns the default datatypes.
 */
QStringList AdaWriter::defaultDatatypes()
{
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
    l.append("Duration");
    l.append("String");
    return l;
}

/**
 * Check whether the given string is a reserved word for the
 * language of this code generator
 *
 * @param rPossiblyReservedKeyword  The string to check.
 */
bool AdaWriter::isReservedKeyword(const QString & rPossiblyReservedKeyword)
{
    const QStringList keywords = reservedKeywords();

    QStringList::ConstIterator it;
    for (it = keywords.begin(); it != keywords.end(); ++it) {
        if ((*it).toLower() == rPossiblyReservedKeyword.toLower()) {
            return true;
        }
    }
    return false;
}

/**
 * Get list of reserved keywords.
 * @return   the list of reserved keywords
 */
QStringList AdaWriter::reservedKeywords() const
{
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

#include "adawriter.moc"
