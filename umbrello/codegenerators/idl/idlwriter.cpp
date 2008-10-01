/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "idlwriter.h"

#include "umldoc.h"
#include "classifier.h"
#include "enum.h"
#include "classifierlistitem.h"
#include "umlclassifierlistitemlist.h"
#include "package.h"
#include "association.h"
#include "attribute.h"
#include "operation.h"
#include "umlnamespace.h"

#include <kdebug.h>
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
    QString stype = c->getStereotype();
    if (stype == "CORBAConstant" || stype == "CORBAEnum" ||
            stype == "CORBAStruct" || stype == "CORBAUnion" ||
            stype == "CORBASequence" || stype == "CORBAArray" ||
            stype == "CORBATypedef")
        return false;

    // CORBAValue, CORBAInterface, and all empty/unknown stereotypes are
    // assumed to be OO classes.
    return true;
}

bool IDLWriter::assocTypeIsMappableToAttribute(Uml::Association_Type at)
{
    return (at == Uml::at_Aggregation || at == Uml::at_Association ||
            at == Uml::at_Composition || at == Uml::at_UniAssociation);
}

Uml::Programming_Language IDLWriter::getLanguage()
{
    return Uml::pl_IDL;
}

void IDLWriter::computeAssocTypeAndRole
(UMLAssociation *a, UMLClassifier *c, QString& typeName, QString& roleName)
{
    // Determine which is the "remote" end of the association:
    bool IAmRoleA = true;
    UMLObject *other = a->getObject(Uml::B);
    Uml::Association_Type at = a->getAssocType();
    if (c->getName() == other->getName()) {
        if (at == Uml::at_Aggregation || at == Uml::at_Composition ||
                at == Uml::at_UniAssociation) {
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
    typeName = cleanName(other->getName());
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
        roleName = a->getName();
        if (roleName.isEmpty()) {
            roleName = "m_" + typeName;
        }
    }
}

void IDLWriter::writeClass(UMLClassifier *c) 
{
    if (!c) {
        uDebug() << "Cannot write class of NULL concept!";
        return;
    }

    const bool isClass = !c->isInterface();
    QString classname = cleanName(c->getName());

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
            if (conc->getBaseType() == Uml::ot_Datatype)
                continue;
            QString incName = findFileName(conc, ".idl");
            if (!incName.isEmpty())
                idl << "#include \"" << incName << "\"" << m_endl;
        }
        idl << m_endl;
    }

    // Generate the module declaration(s) for the package(s) in which
    // we are embedded.
    UMLPackageList pkgList = c->getPackages();

    foreach ( UMLPackage* pkg,  pkgList ) {
        idl << getIndent() << "module " << pkg->getName() << " {" << m_endl << m_endl;
        m_indentLevel++;
    }

    // Write class Documentation if non-empty or if force option set.
    if (forceDoc() || !c->getDoc().isEmpty()) {
        idl << "//" << m_endl;
        idl << "// class " << classname << m_endl;
        idl << formatDoc(c->getDoc(), "// ");
        idl << m_endl;
    }

    if (c->getBaseType() == Uml::ot_Enum) {
        UMLClassifierListItemList litList = c->getFilteredList(Uml::ot_EnumLiteral);
        uint i = 0;
        idl << getIndent() << "enum " << classname << " {" << m_endl;
        m_indentLevel++;
        foreach (UMLClassifierListItem *lit , litList ) {
            QString enumLiteral = cleanName(lit->getName());
            idl << getIndent() << enumLiteral;
            if (++i < ( uint )litList.count())
                idl << ",";
            idl << m_endl;
        }
        m_indentLevel--;
        idl << getIndent() << "};" << m_endl << m_endl;
        // Close the modules inside which we might be nested.
        for (int i = 0; i < pkgList.count(); ++i) {
            m_indentLevel--;
            idl << getIndent() << "};" << m_endl << m_endl;
        }
        return;
    }
    if (! isOOClass(c)) {
        QString stype = c->getStereotype();
        if (stype == "CORBAConstant") {
            uError() << "The stereotype " << stype << " cannot be applied to "
            << c->getName() << ", but only to attributes.";
            return;
        }
        if (!isClass) {
            uError() << "The stereotype " << stype
            << " cannot be applied to " << c->getName()
            << ", but only to classes.";
            return;
        }
        if (stype == "CORBAEnum") {
            UMLAttributeList atl = c->getAttributeList();

            idl << getIndent() << "enum " << classname << " {" << m_endl;
            m_indentLevel++;
            uint i = 0;
            foreach (UMLAttribute* at , atl ) {
                QString enumLiteral = cleanName(at->getName());
                idl << getIndent() << enumLiteral;
                if (++i < ( uint )atl.count())
                    idl << ",";
                idl << m_endl;
            }
            m_indentLevel--;
            idl << getIndent() << "};" << m_endl << m_endl;
        } else if (stype == "CORBAStruct") {
            UMLAttributeList atl = c->getAttributeList();

            idl << getIndent() << "struct " << classname << " {" << m_endl;
            m_indentLevel++;
            foreach (UMLAttribute* at , atl ) {
                QString name = cleanName(at->getName());
                idl << getIndent() << at->getTypeName() << " " << name << ";" << m_endl;
                // Initial value not possible in IDL.
            }
            UMLAssociationList compositions = c->getCompositions();
            if (!compositions.isEmpty()) {
                idl << getIndent() << "// Compositions." << m_endl;
                foreach (UMLAssociation *a , compositions ) {
                    QString memberType, memberName;
                    computeAssocTypeAndRole(a, c, memberType, memberName);
                    idl << getIndent() << memberType << " " << memberName << ";" << m_endl;
                }
            }
            UMLAssociationList aggregations = c->getAggregations();
            if (!aggregations.isEmpty()) {
                idl << getIndent() << "// Aggregations." << m_endl;
                foreach (UMLAssociation *a , aggregations ) {
                    QString memberType, memberName;
                    computeAssocTypeAndRole(a, c, memberType, memberName);
                    idl << getIndent() << memberType << " " << memberName << ";" << m_endl;
                }
            }
            m_indentLevel--;
            idl << getIndent() << "};" << m_endl << m_endl;
        } else if (stype == "CORBAUnion") {
            idl << getIndent() << "// " << stype << " " << c->getName()
            << " is Not Yet Implemented" << m_endl << m_endl;
        } else if (stype == "CORBATypedef") {
            UMLClassifierList superclasses = c->getSuperClasses();
            UMLClassifier* firstParent = superclasses.first();
            idl << getIndent() << "typedef " << firstParent->getName() << " "
            << c->getName() << ";" << m_endl << m_endl;
        } else {
            idl << getIndent() << "// " << stype << ": Unknown stereotype" << m_endl << m_endl;
        }
        // Close the modules inside which we might be nested.
        for (int i = 0; i < pkgList.count(); ++i) {
            m_indentLevel--;
            idl << getIndent() << "};" << m_endl << m_endl;
        }
        return;
    }

    idl << getIndent();
    if (c->getAbstract())
        idl << "abstract ";
    bool isValuetype = (c->getStereotype() == "CORBAValue");
    if (isValuetype)
        idl << "valuetype ";
    else
        idl << "interface ";
    idl << c->getName();
    UMLClassifierList superclasses = c->getSuperClasses();
    if (! superclasses.isEmpty()) {
        idl << " : ";
        int count = superclasses.count();
        foreach( UMLClassifier* parent, superclasses ) {
            count--;
            idl << parent->getFullyQualifiedName("::");
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
            idl << getIndent() << "// Types for association multiplicities" << m_endl << m_endl;
            didComment = true;
        }
        UMLClassifier* other = (UMLClassifier*)a->getObject(Uml::A);
        QString bareName = cleanName(other->getName());
        idl << getIndent() << "typedef sequence<" << other->getFullyQualifiedName("::")
        << "> " << bareName << "Vector;" << m_endl << m_endl;
    }

    // Generate public attributes.
    if (isClass) {
        UMLAttributeList atl = c->getAttributeList();
        if (forceSections() || atl.count()) {
            idl << getIndent() << "// Attributes:" << m_endl << m_endl;
            foreach (UMLAttribute *at , atl ) {
                QString attName = cleanName(at->getName());
                Uml::Visibility scope = at->getVisibility();
                idl << getIndent();
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
                        idl << getIndent();
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
          if (op->getVisibility() == Uml::Visibility::Public)
            oppub.append(op);
    }
    if (forceSections() || oppub.count()) {
        idl << getIndent() << "// Public methods:" << m_endl << m_endl;
        foreach ( UMLOperation* op , oppub )
            writeOperation(op, idl);
        idl << m_endl;
    }

    if (forceSections() || !assocs.isEmpty()) {
        idl << getIndent() << "// Associations:" << m_endl << m_endl;
        foreach ( UMLAssociation* a , assocs ) {
            Uml::Association_Type at = a->getAssocType();
            if (! assocTypeIsMappableToAttribute(at))
                continue;
            QString typeName, roleName;
            computeAssocTypeAndRole(a, c, typeName, roleName);
            if (roleName.isEmpty())  // presumably because we are at the "wrong" end
                continue;
            idl << getIndent() << "// " << UMLAssociation::typeAsString(at) << m_endl;
            idl << getIndent();
            if (isValuetype)
                idl << "public ";
            else
                idl << "attribute ";
            idl << typeName << " " << roleName << ";" << m_endl;
        }
        idl << m_endl;
    }

    m_indentLevel--;
    idl << getIndent() << "};" << m_endl << m_endl;

    // Close the modules inside which we might be nested.
    for (int i = 0; i < pkgList.count(); ++i) {
        m_indentLevel--;
        idl << getIndent() << "};" << m_endl << m_endl;
    }
    file.close();
    emit codeGenerated(c, true);
}

void IDLWriter::writeOperation(UMLOperation *op, QTextStream &idl, bool is_comment)
{
    UMLAttributeList atl = op->getParmList();
    QString rettype = op->getTypeName();

    if (rettype.isEmpty())
        rettype = "void";
    idl << getIndent();
    if (is_comment)
        idl << "// ";
    idl << rettype << " " << cleanName(op->getName()) << " (";
    if (atl.count()) {
        idl << m_endl;
        m_indentLevel++;
        uint i = 0;
        foreach (UMLAttribute *at , atl ) {
            idl << getIndent();
            if (is_comment)
                idl << "// ";
            Uml::Parameter_Direction pk = at->getParmKind();
            if (pk == Uml::pd_Out)
                idl << "out ";
            else if (pk == Uml::pd_InOut)
                idl << "inout ";
            else
                idl << "in ";
            idl << at->getTypeName() << " " << cleanName(at->getName());
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

const QStringList IDLWriter::reservedKeywords() const 
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


