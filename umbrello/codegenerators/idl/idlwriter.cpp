/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

#include <KMessageBox>

#include <QFile>
#include <QRegExp>
#include <QTextStream>

IDLWriter::IDLWriter() : SimpleCodeGenerator(false)
{
}

IDLWriter::~IDLWriter()
{
}

bool IDLWriter::isOOClass(UMLClassifier *c)
{
    QString stype = c->stereotype();
    QRegExp nonOO(QLatin1String("(Constant|Enum|Struct|Union|Sequence|Array|Typedef)$"),
                  Qt::CaseInsensitive);
    if (stype.contains(nonOO))
        return false;

    // idlValue/CORBAValue, idlInterface/CORBAInterface, and empty/unknown stereotypes are
    // assumed to designate OO classes.
    return true;
}

bool IDLWriter::assocTypeIsMappableToAttribute(Uml::AssociationType::Enum at)
{
    return (at == Uml::AssociationType::Aggregation || at == Uml::AssociationType::Association ||
            at == Uml::AssociationType::Composition || at == Uml::AssociationType::UniAssociation);
}

/**
 * Returns "IDL".
 */
Uml::ProgrammingLanguage::Enum IDLWriter::language() const
{
    return Uml::ProgrammingLanguage::IDL;
}

void IDLWriter::computeAssocTypeAndRole(UMLAssociation *a, UMLClassifier *c,
                                        QString& typeName, QString& roleName)
{
    // Determine which is the "remote" end of the association:
    bool IAmRoleA = true;
    UMLObject *other = a->getObject(Uml::RoleType::B);
    Uml::AssociationType::Enum at = a->getAssocType();
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
        other = a->getObject(Uml::RoleType::A);
    }
    // Construct the type name:
    typeName = cleanName(other->name());
    QString multiplicity;
    if (IAmRoleA)
        multiplicity = a->getMultiplicity(Uml::RoleType::B);
    else
        multiplicity = a->getMultiplicity(Uml::RoleType::A);
    if (!multiplicity.isEmpty() && multiplicity != QLatin1String("1"))
        typeName.append(QLatin1String("Vector"));
    // Construct the member name:
    if (IAmRoleA)
        roleName = a->getRoleName(Uml::RoleType::B);
    else
        roleName = a->getRoleName(Uml::RoleType::A);
    if (roleName.isEmpty()) {
        roleName = a->name();
        if (roleName.isEmpty()) {
            roleName = QLatin1String("m_") + typeName;
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
    QString fileName = findFileName(c, QLatin1String(".idl"));
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
    str = getHeadingFile(QLatin1String(".idl"));
    if (!str.isEmpty()) {
        str.replace(QRegExp(QLatin1String("%filename%")), fileName);
        str.replace(QRegExp(QLatin1String("%filepath%")), file.fileName());
        idl << str << m_endl;
    }

    // Write includes.
    UMLPackageList includes;
    findObjectsRelated(c, includes);
    if (includes.count()) {
        foreach (UMLPackage* conc, includes) {
            if (conc->isUMLDatatype())
                continue;
            QString incName = findFileName(conc, QLatin1String(".idl"));
            if (!incName.isEmpty())
                idl << "#include \"" << incName << "\"" << m_endl;
        }
        idl << m_endl;
    }

    // Generate the module declaration(s) for the package(s) in which
    // we are embedded.
    UMLPackageList pkgList = c->packages();

    foreach (UMLPackage* pkg,  pkgList) {
        idl << indent() << "module " << pkg->name() << " {" << m_endl << m_endl;
        m_indentLevel++;
    }

    // Write class Documentation if non-empty or if force option set.
    if (forceDoc() || !c->doc().isEmpty()) {
        idl << "//" << m_endl;
        idl << "// class " << classname << m_endl;
        idl << formatDoc(c->doc(), QLatin1String("// "));
        idl << m_endl;
    }

    if (c->baseType() == UMLObject::ot_Enum) {
        UMLClassifierListItemList litList = c->getFilteredList(UMLObject::ot_EnumLiteral);
        uint i = 0;
        idl << indent() << "enum " << classname << " {" << m_endl;
        m_indentLevel++;
        foreach (UMLClassifierListItem *lit, litList) {
            QString enumLiteral = cleanName(lit->name());
            idl << indent() << enumLiteral;
            if (++i < (uint)litList.count())
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
        if (stype.contains(QLatin1String("Constant"))) {
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
        if (stype.contains(QLatin1String("Enum"))) {
            UMLAttributeList atl = c->getAttributeList();

            idl << indent() << "enum " << classname << " {" << m_endl;
            m_indentLevel++;
            uint i = 0;
            foreach (UMLAttribute* at, atl) {
                QString enumLiteral = cleanName(at->name());
                idl << indent() << enumLiteral;
                if (++i < (uint)atl.count())
                    idl << ",";
                idl << m_endl;
            }
            m_indentLevel--;
            idl << indent() << "};" << m_endl << m_endl;
        } else if (stype.contains(QLatin1String("Struct"))) {
            UMLAttributeList atl = c->getAttributeList();

            idl << indent() << "struct " << classname << " {" << m_endl;
            m_indentLevel++;
            foreach (UMLAttribute* at, atl) {
                QString name = cleanName(at->name());
                idl << indent() << at->getTypeName() << " " << name << ";" << m_endl;
                // Initial value not possible in IDL.
            }
            UMLAssociationList compositions = c->getCompositions();
            if (!compositions.isEmpty()) {
                idl << indent() << "// Compositions." << m_endl;
                foreach (UMLAssociation *a, compositions) {
                    QString memberType, memberName;
                    computeAssocTypeAndRole(a, c, memberType, memberName);
                    idl << indent() << memberType << " " << memberName << ";" << m_endl;
                }
            }
            UMLAssociationList aggregations = c->getAggregations();
            if (!aggregations.isEmpty()) {
                idl << indent() << "// Aggregations." << m_endl;
                foreach (UMLAssociation *a, aggregations) {
                    QString memberType, memberName;
                    computeAssocTypeAndRole(a, c, memberType, memberName);
                    idl << indent() << memberType << " " << memberName << ";" << m_endl;
                }
            }
            m_indentLevel--;
            idl << indent() << "};" << m_endl << m_endl;
        } else if (stype.contains(QLatin1String("Union"))) {
            // idl << indent() << "// " << stype << " " << c->name() << " is Not Yet Implemented" << m_endl << m_endl;
            UMLAttributeList atl = c->getAttributeList();
            if (atl.count()) {
                UMLAttribute *discrim = atl.front();
                idl << indent() << "union " << c->name() << " switch (" << discrim->getTypeName()
                                << ") {"  << m_endl << m_endl;
                atl.pop_front();
                m_indentLevel++;
                foreach (UMLAttribute *at, atl) {
                    QString attName = cleanName(at->name());
                    const QStringList& tags = at->tags();
                    if (tags.count()) {
                        const QString& caseVals = tags.front();
                        foreach (QString caseVal, caseVals.split(QLatin1Char(' '))) {
                            idl << indent() << "case " << caseVal << ":" << m_endl;
                        }
                    } else {
                        // uError() << "missing 'case' tag at union attribute " << attName;
                        idl << indent() << "default:" << m_endl;
                    }
                    idl << indent() << m_indentation << at->getTypeName() << " " << attName << ";"
                        << m_endl << m_endl;
                }
                m_indentLevel--;
                idl << indent() << "};" << m_endl << m_endl;
            } else {
                uError() << "Empty <<union>> " << c->name();
                idl << indent() << "// <<union>> " << c->name() << " is empty, please check model" << m_endl;
                idl << indent() << "union " << c->name() << ";" << m_endl << m_endl;
            }
        } else if (stype.contains(QLatin1String("Typedef"))) {
            UMLClassifierList superclasses = c->getSuperClasses();
            if (superclasses.count()) {
                UMLClassifier* firstParent = superclasses.first();
                idl << indent() << "typedef " << firstParent->name() << " "
                    << c->name() << ";" << m_endl << m_endl;
            } else {
                uError() << "typedef " << c->name() << " parent (origin type) is missing";
                idl << indent() << "// typedef origin type is missing, please check model" << m_endl;
                idl << indent() << "typedef long " << c->name() << ";" << m_endl << m_endl;
            }
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
    bool isValuetype = (c->stereotype().contains(QLatin1String("Value")));
    if (isValuetype)
        idl << "valuetype ";
    else
        idl << "interface ";
    idl << c->name();
    UMLClassifierList superclasses = c->getSuperClasses();
    if (! superclasses.isEmpty()) {
        idl << " : ";
        int count = superclasses.count();
        foreach(UMLClassifier* parent, superclasses) {
            count--;
            idl << parent->fullyQualifiedName(QLatin1String("::"));
            if (count)
                idl << ", ";
        }
    }
    idl << " {" << m_endl << m_endl;
    m_indentLevel++;

    // Generate auxiliary declarations for multiplicity of associations

    bool didComment = false;
    UMLAssociationList assocs = c->getAssociations();
    foreach (UMLAssociation *a, assocs) {
        if (! assocTypeIsMappableToAttribute(a->getAssocType()))
            continue;
        QString multiplicity = a->getMultiplicity(Uml::RoleType::A);
        if (multiplicity.isEmpty() || multiplicity == QLatin1String("1"))
            continue;
        if (!didComment) {
            idl << indent() << "// Types for association multiplicities" << m_endl << m_endl;
            didComment = true;
        }
        UMLClassifier* other = (UMLClassifier*)a->getObject(Uml::RoleType::A);
        QString bareName = cleanName(other->name());
        idl << indent() << "typedef sequence<" << other->fullyQualifiedName(QLatin1String("::"))
            << "> " << bareName << "Vector;" << m_endl << m_endl;
    }

    // Generate public attributes.
    if (isClass) {
        UMLAttributeList atl = c->getAttributeList();
        if (forceSections() || atl.count()) {
            idl << indent() << "// Attributes:" << m_endl << m_endl;
            foreach (UMLAttribute *at, atl) {
                QString attName = cleanName(at->name());
                Uml::Visibility::Enum scope = at->visibility();
                idl << indent();
                if (isValuetype) {
                    if (scope == Uml::Visibility::Public)
                        idl << "public ";
                    else
                        idl << "private ";
                } else {
                    if (scope != Uml::Visibility::Public) {
                        idl << "// visibility should be: "
                            << Uml::Visibility::toString(scope)
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

    foreach (UMLOperation* op, opl) {
          if (op->visibility() == Uml::Visibility::Public)
            oppub.append(op);
    }
    if (forceSections() || oppub.count()) {
        idl << indent() << "// Public methods:" << m_endl << m_endl;
        foreach (UMLOperation* op, oppub)
            writeOperation(op, idl);
        idl << m_endl;
    }

    if (forceSections() || !assocs.isEmpty()) {
        idl << indent() << "// Associations:" << m_endl << m_endl;
        foreach (UMLAssociation* a, assocs) {
            Uml::AssociationType::Enum at = a->getAssocType();
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
    emit showGeneratedFile(file.fileName());
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
        rettype = QLatin1String("void");
    idl << indent();
    if (is_comment)
        idl << "// ";
    idl << rettype << " " << cleanName(op->name()) << " (";
    if (atl.count()) {
        idl << m_endl;
        m_indentLevel++;
        uint i = 0;
        foreach (UMLAttribute *at, atl) {
            idl << indent();
            if (is_comment)
                idl << "// ";
            Uml::ParameterDirection::Enum pk = at->getParmKind();
            if (pk == Uml::ParameterDirection::Out)
                idl << "out ";
            else if (pk == Uml::ParameterDirection::InOut)
                idl << "inout ";
            else
                idl << "in ";
            idl << at->getTypeName() << " " << cleanName(at->name());
            if (++i < (uint)atl.count())
                idl << "," << m_endl;
        }
        m_indentLevel--;
    }
    idl << ");" << m_endl << m_endl;
}

QStringList IDLWriter::defaultDatatypes()
{
    QStringList l;
    l.append(QLatin1String("boolean"));
    l.append(QLatin1String("char"));
    l.append(QLatin1String("octet"));
    l.append(QLatin1String("short"));
    l.append(QLatin1String("unsigned short"));
    l.append(QLatin1String("long"));
    l.append(QLatin1String("unsigned long"));
    l.append(QLatin1String("long long"));
    l.append(QLatin1String("unsigned long long"));
    l.append(QLatin1String("float"));
    l.append(QLatin1String("double"));
    l.append(QLatin1String("long double"));
    l.append(QLatin1String("string"));
    l.append(QLatin1String("any"));
    return l;
}

/**
 * Get list of reserved keywords.
 */
QStringList IDLWriter::reservedKeywords() const 
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords
          << QLatin1String("any")
          << QLatin1String("attribute")
          << QLatin1String("boolean")
          << QLatin1String("case")
          << QLatin1String("char")
          << QLatin1String("const")
          << QLatin1String("context")
          << QLatin1String("default")
          << QLatin1String("double")
          << QLatin1String("enum")
          << QLatin1String("exception")
          << QLatin1String("FALSE")
          << QLatin1String("float")
          << QLatin1String("in")
          << QLatin1String("inout")
          << QLatin1String("interface")
          << QLatin1String("long")
          << QLatin1String("module")
          << QLatin1String("octet")
          << QLatin1String("oneway")
          << QLatin1String("out")
          << QLatin1String("raises")
          << QLatin1String("readonly")
          << QLatin1String("sequence")
          << QLatin1String("short")
          << QLatin1String("string")
          << QLatin1String("struct")
          << QLatin1String("switch")
          << QLatin1String("TRUE")
          << QLatin1String("typedef")
          << QLatin1String("union")
          << QLatin1String("unsigned")
          << QLatin1String("void");
    }

    return keywords;
}
