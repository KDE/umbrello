/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "idlwriter.h"

#include "umldoc.h"
#include "classifier.h"
#include "debug_utils.h"
#include "enum.h"
#include "classifierlistitem.h"
#include "umlclassifierlistitemlist.h"
#include "package.h"
#include "association.h"
#include "attribute.h"
#include "operation.h"

#include <kmessagebox.h>

#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>

IDLWriter::IDLWriter() : SimpleCodeGenerator(false)
{
}

IDLWriter::~IDLWriter()
{
}

bool IDLWriter::isOOClass(UMLClassifier *c)
{
    QString stype = c->stereotype();
    if (stype == "CORBAConstant" || stype == "CORBAEnum" ||
            stype == "CORBAStruct" || stype == "CORBAUnion" ||
            stype == "CORBASequence" || stype == "CORBAArray" ||
            stype == "CORBATypedef")
        return false;

    // CORBAValue, CORBAInterface, and all empty/unknown stereotypes are
    // assumed to be OO classes.
    return true;
}

bool IDLWriter::assocTypeIsMappableToAttribute(Uml::AssociationType at)
{
    return (at == Uml::AssociationType::Aggregation || at == Uml::AssociationType::Association ||
            at == Uml::AssociationType::Composition || at == Uml::AssociationType::UniAssociation);
}

/**
 * Returns "IDL".
 */
Uml::ProgrammingLanguage IDLWriter::language() const
{
    return Uml::ProgrammingLanguage::IDL;
}

void IDLWriter::computeAssocTypeAndRole(UMLAssociation *a, UMLClassifier *c,
                                        QString& typeName, QString& roleName)
{
    // Determine which is the "remote" end of the association:
    bool IAmRoleA = true;
    UMLObject *other = a->getObject(Uml::B);
    Uml::AssociationType at = a->getAssocType();
    if (c->name() == other->name()) {
        if (at == Uml::AssociationType::Aggregation || at == Uml::AssociationType::Composition ||
            at == Uml::AssociationType::UniAssociation) {
            // Assuming unidirectional association, and we are
            // at the "wrong" side.
            // Returning roleName = QString() tells caller to
            // skip this association at this side.
            roleName.clear();
            return;
        }
        IAmRoleA = false;
        other = a->getObject(Uml::A);
    }
    // Construct the type name:
    typeName = cleanName(other->name());
    QString multiplicity;
    if (IAmRoleA)
        multiplicity = a->getMulti(Uml::B);
    else
        multiplicity = a->getMulti(Uml::A);
    if (!multiplicity.isEmpty() && multiplicity != "1")
        typeName.append("Vector");
    // Construct the member name:
    if (IAmRoleA)
        roleName = a->getRoleName(Uml::B);
    else
        roleName = a->getRoleName(Uml::A);
    if (roleName.isEmpty()) {
        roleName = a->name();
        if (roleName.isEmpty()) {
            roleName = "m_" + typeName;
        }
    }
}

/**
 * Call this method to generate IDL code for a UMLClassifier.
 * @param c the class to generate code for
 */
void IDLWriter::writeClass(UMLClassifier *c) 
{
    if (!c) {
        uDebug() << "Cannot write class of NULL concept!";
        return;
    }

    const bool isClass = !c->isInterface();
    QString classname = cleanName(c->name());

    //find an appropriate name for our file
    QString fileName = findFileName(c, ".idl");
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

    QTextStream idl(&file);
    //try to find a heading file(license, comments, etc)
    QString str;
    str = getHeadingFile(".idl");
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), fileName);
        str.replace(QRegExp("%filepath%"), file.fileName());
        idl << str << m_endl;
    }

    // Write includes.
    UMLPackageList includes;
    findObjectsRelated(c, includes);
    if (includes.count()) {
        foreach (UMLPackage* conc, includes ) {
            if (conc->baseType() == UMLObject::ot_Datatype)
                continue;
            QString incName = findFileName(conc, ".idl");
            if (!incName.isEmpty())
                idl << "#include \"" << incName << "\"" << m_endl;
        }
        idl << m_endl;
    }

    // Generate the module declaration(s) for the package(s) in which
    // we are embedded.
    UMLPackageList pkgList = c->packages();

    foreach ( UMLPackage* pkg,  pkgList ) {
        idl << indent() << "module " << pkg->name() << " {" << m_endl << m_endl;
        m_indentLevel++;
    }

    // Write class Documentation if non-empty or if force option set.
    if (forceDoc() || !c->doc().isEmpty()) {
        idl << "//" << m_endl;
        idl << "// class " << classname << m_endl;
        idl << formatDoc(c->doc(), "// ");
        idl << m_endl;
    }

    if (c->baseType() == UMLObject::ot_Enum) {
        UMLClassifierListItemList litList = c->getFilteredList(UMLObject::ot_EnumLiteral);
        uint i = 0;
        idl << indent() << "enum " << classname << " {" << m_endl;
        m_indentLevel++;
        foreach (UMLClassifierListItem *lit , litList ) {
            QString enumLiteral = cleanName(lit->name());
            idl << indent() << enumLiteral;
            if (++i < ( uint )litList.count())
                idl << ",";
            idl << m_endl;
        }
        m_indentLevel--;
        idl << indent() << "};" << m_endl << m_endl;
        // Close the modules inside which we might be nested.
        for (int i = 0; i < pkgList.count(); ++i) {
            m_indentLevel--;
            idl << indent() << "};" << m_endl << m_endl;
        }
        return;
    }
    if (! isOOClass(c)) {
        QString stype = c->stereotype();
        if (stype == "CORBAConstant") {
            uError() << "The stereotype " << stype << " cannot be applied to "
            << c->name() << ", but only to attributes.";
            return;
        }
        if (!isClass) {
            uError() << "The stereotype " << stype
            << " cannot be applied to " << c->name()
            << ", but only to classes.";
            return;
        }
        if (stype == "CORBAEnum") {
            UMLAttributeList atl = c->getAttributeList();

            idl << indent() << "enum " << classname << " {" << m_endl;
            m_indentLevel++;
            uint i = 0;
            foreach (UMLAttribute* at , atl ) {
                QString enumLiteral = cleanName(at->name());
                idl << indent() << enumLiteral;
                if (++i < ( uint )atl.count())
                    idl << ",";
                idl << m_endl;
            }
            m_indentLevel--;
            idl << indent() << "};" << m_endl << m_endl;
        } else if (stype == "CORBAStruct") {
            UMLAttributeList atl = c->getAttributeList();

            idl << indent() << "struct " << classname << " {" << m_endl;
            m_indentLevel++;
            foreach (UMLAttribute* at , atl ) {
                QString name = cleanName(at->name());
                idl << indent() << at->getTypeName() << " " << name << ";" << m_endl;
                // Initial value not possible in IDL.
            }
            UMLAssociationList compositions = c->getCompositions();
            if (!compositions.isEmpty()) {
                idl << indent() << "// Compositions." << m_endl;
                foreach (UMLAssociation *a , compositions ) {
                    QString memberType, memberName;
                    computeAssocTypeAndRole(a, c, memberType, memberName);
                    idl << indent() << memberType << " " << memberName << ";" << m_endl;
                }
            }
            UMLAssociationList aggregations = c->getAggregations();
            if (!aggregations.isEmpty()) {
                idl << indent() << "// Aggregations." << m_endl;
                foreach (UMLAssociation *a , aggregations ) {
                    QString memberType, memberName;
                    computeAssocTypeAndRole(a, c, memberType, memberName);
                    idl << indent() << memberType << " " << memberName << ";" << m_endl;
                }
            }
            m_indentLevel--;
            idl << indent() << "};" << m_endl << m_endl;
        } else if (stype == "CORBAUnion") {
            idl << indent() << "// " << stype << " " << c->name()
            << " is Not Yet Implemented" << m_endl << m_endl;
        } else if (stype == "CORBATypedef") {
            UMLClassifierList superclasses = c->getSuperClasses();
            UMLClassifier* firstParent = superclasses.first();
            idl << indent() << "typedef " << firstParent->name() << " "
            << c->name() << ";" << m_endl << m_endl;
        } else {
            idl << indent() << "// " << stype << ": Unknown stereotype" << m_endl << m_endl;
        }
        // Close the modules inside which we might be nested.
        for (int i = 0; i < pkgList.count(); ++i) {
            m_indentLevel--;
            idl << indent() << "};" << m_endl << m_endl;
        }
        return;
    }

    idl << indent();
    if (c->isAbstract())
        idl << "abstract ";
    bool isValuetype = (c->stereotype() == "CORBAValue");
    if (isValuetype)
        idl << "valuetype ";
    else
        idl << "interface ";
    idl << c->name();
    UMLClassifierList superclasses = c->getSuperClasses();
    if (! superclasses.isEmpty()) {
        idl << " : ";
        int count = superclasses.count();
        foreach( UMLClassifier* parent, superclasses ) {
            count--;
            idl << parent->fullyQualifiedName("::");
            if (count)
                idl << ", ";
        }
    }
    idl << " {" << m_endl << m_endl;
    m_indentLevel++;

    // Generate auxiliary declarations for multiplicity of associations

    bool didComment = false;
    UMLAssociationList assocs = c->getAssociations();
    foreach (UMLAssociation *a, assocs ) {
        if (! assocTypeIsMappableToAttribute(a->getAssocType()))
            continue;
        QString multiplicity = a->getMulti(Uml::A);
        if (multiplicity.isEmpty() || multiplicity == "1")
            continue;
        if (!didComment) {
            idl << indent() << "// Types for association multiplicities" << m_endl << m_endl;
            didComment = true;
        }
        UMLClassifier* other = (UMLClassifier*)a->getObject(Uml::A);
        QString bareName = cleanName(other->name());
        idl << indent() << "typedef sequence<" << other->fullyQualifiedName("::")
        << "> " << bareName << "Vector;" << m_endl << m_endl;
    }

    // Generate public attributes.
    if (isClass) {
        UMLAttributeList atl = c->getAttributeList();
        if (forceSections() || atl.count()) {
            idl << indent() << "// Attributes:" << m_endl << m_endl;
            foreach (UMLAttribute *at , atl ) {
                QString attName = cleanName(at->name());
                Uml::Visibility scope = at->visibility();
                idl << indent();
                if (isValuetype) {
                    if (scope == Uml::Visibility::Public)
                        idl << "public ";
                    else
                        idl << "private ";
                } else {
                    if (scope != Uml::Visibility::Public) {
                        idl << "// visibility should be: "
                        << scope.toString()
                        << m_endl;
                        idl << indent();
                    }
                    idl << "attribute ";
                }
                idl << at->getTypeName() << " " << attName << ";"
                << m_endl << m_endl;
            }
        }
    }

    // Generate public operations.
    UMLOperationList opl(c->getOpList());
    UMLOperationList oppub;

    foreach (UMLOperation* op , opl ) {
          if (op->visibility() == Uml::Visibility::Public)
            oppub.append(op);
    }
    if (forceSections() || oppub.count()) {
        idl << indent() << "// Public methods:" << m_endl << m_endl;
        foreach ( UMLOperation* op , oppub )
            writeOperation(op, idl);
        idl << m_endl;
    }

    if (forceSections() || !assocs.isEmpty()) {
        idl << indent() << "// Associations:" << m_endl << m_endl;
        foreach ( UMLAssociation* a , assocs ) {
            Uml::AssociationType at = a->getAssocType();
            if (! assocTypeIsMappableToAttribute(at))
                continue;
            QString typeName, roleName;
            computeAssocTypeAndRole(a, c, typeName, roleName);
            if (roleName.isEmpty())  // presumably because we are at the "wrong" end
                continue;
            idl << indent() << "// " << Uml::AssociationType::toString(at) << m_endl;
            idl << indent();
            if (isValuetype)
                idl << "public ";
            else
                idl << "attribute ";
            idl << typeName << " " << roleName << ";" << m_endl;
        }
        idl << m_endl;
    }

    m_indentLevel--;
    idl << indent() << "};" << m_endl << m_endl;

    // Close the modules inside which we might be nested.
    for (int i = 0; i < pkgList.count(); ++i) {
        m_indentLevel--;
        idl << indent() << "};" << m_endl << m_endl;
    }
    file.close();
    emit codeGenerated(c, true);
}

/**
 * Write one operation.
 * @param op the class for which we are generating code
 * @param idl the stream associated with the output file
 */
void IDLWriter::writeOperation(UMLOperation *op, QTextStream &idl, bool is_comment)
{
    UMLAttributeList atl = op->getParmList();
    QString rettype = op->getTypeName();

    if (rettype.isEmpty())
        rettype = "void";
    idl << indent();
    if (is_comment)
        idl << "// ";
    idl << rettype << " " << cleanName(op->name()) << " (";
    if (atl.count()) {
        idl << m_endl;
        m_indentLevel++;
        uint i = 0;
        foreach (UMLAttribute *at , atl ) {
            idl << indent();
            if (is_comment)
                idl << "// ";
            Uml::Parameter_Direction pk = at->getParmKind();
            if (pk == Uml::pd_Out)
                idl << "out ";
            else if (pk == Uml::pd_InOut)
                idl << "inout ";
            else
                idl << "in ";
            idl << at->getTypeName() << " " << cleanName(at->name());
            if (++i < ( uint )atl.count())
                idl << "," << m_endl;
        }
        m_indentLevel--;
    }
    idl << ");" << m_endl << m_endl;
}

QStringList IDLWriter::defaultDatatypes()
{
    QStringList l;
    l.append("boolean");
    l.append("char");
    l.append("octet");
    l.append("short");
    l.append("unsigned short");
    l.append("long");
    l.append("unsigned long");
    l.append("float");
    l.append("double");
    l.append("string");
    l.append("any");
    return l;
}

/**
 * Get list of reserved keywords.
 */
QStringList IDLWriter::reservedKeywords() const 
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords << "any"
        << "attribute"
        << "boolean"
        << "case"
        << "char"
        << "const"
        << "context"
        << "default"
        << "double"
        << "enum"
        << "exception"
        << "FALSE"
        << "float"
        << "in"
        << "inout"
        << "interface"
        << "long"
        << "module"
        << "octet"
        << "oneway"
        << "out"
        << "raises"
        << "readonly"
        << "sequence"
        << "short"
        << "string"
        << "struct"
        << "switch"
        << "TRUE"
        << "typedef"
        << "union"
        << "unsigned"
        << "void";
    }

    return keywords;
}
