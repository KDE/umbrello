/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2002 Oliver Kellogg <okellogg@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

#include <KLocalizedString>
#include <KMessageBox>

#include <QFile>
#include <QRegExp>
#include <QTextStream>

const QString AdaWriter::defaultPackageSuffix = QLatin1String("_Holder");

/**
 * Basic Constructor
 */
AdaWriter::AdaWriter()
 : SimpleCodeGenerator()
{
    m_indentLevel = 1;  // due to different handling, see finalizeRun()
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
Uml::ProgrammingLanguage::Enum AdaWriter::language() const
{
    return Uml::ProgrammingLanguage::Ada;
}

/**
 * Return true if `c' is a tagged type or Ada2005 interface.
 */
bool AdaWriter::isOOClass(const UMLClassifier *c)
{
    UMLObject::ObjectType ot = c->baseType();
    if (ot == UMLObject::ot_Interface)
        return true;
    if (ot == UMLObject::ot_Enum)
        return false;
    if (ot != UMLObject::ot_Class) {
        logWarn1("AdaWriter::isOOClass unexpected object type %1", UMLObject::toString(ot));
        return false;
    }
    QString stype = c->stereotype();
    if (stype == QLatin1String("CORBAConstant") || stype == QLatin1String("CORBATypedef") ||
            stype == QLatin1String("CORBAStruct") || stype == QLatin1String("CORBAUnion"))
        return false;
    // CORBAValue, CORBAInterface, and all empty/unknown stereotypes are
    // assumed to be object oriented classes.
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
            retval = className + QLatin1Char('.');
        retval.append(QLatin1String("Object"));
    } else {
        if (! inOwnScope)
            retval = umlPkg->fullyQualifiedName(QLatin1String(".")) + QLatin1Char('.');
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
        umlPkg = 0;

    const UMLClassifier *c = p->asUMLClassifier();
    if (umlPkg == 0) {
        retval = className;
        if (c == 0 || !isOOClass(c))
            retval.append(defaultPackageSuffix);
    } else {
        retval = umlPkg->fullyQualifiedName(QLatin1String("."));
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
    UMLClassifier* assocEnd = a->getObject(Uml::RoleType::B)->asUMLClassifier();
    if (assocEnd == 0)
        return;
    const Uml::AssociationType::Enum assocType = a->getAssocType();
    if (assocType != Uml::AssociationType::Aggregation && assocType != Uml::AssociationType::Composition)
        return;
    const QString multi = a->getMultiplicity(Uml::RoleType::B);
    bool hasNonUnityMultiplicity = (!multi.isEmpty() && multi != QLatin1String("1"));
    hasNonUnityMultiplicity &= !multi.contains(QRegExp(QLatin1String("^1 *\\.\\. *1$")));
    roleName = cleanName(a->getRoleName(Uml::RoleType::B));
    if (roleName.isEmpty())
        roleName = cleanName(a->name());
    if (roleName.isEmpty()) {
        QString artificialName = cleanName(assocEnd->name());
        if (hasNonUnityMultiplicity) {
            roleName = artificialName;
            roleName.append(QLatin1String("_Vector"));
        } else {
            roleName = QLatin1String("M_");
            roleName.append(artificialName);
        }
    }
    typeName = className(assocEnd, (assocEnd == c));
    if (hasNonUnityMultiplicity)
        typeName.append(QLatin1String("_Array_Ptr"));
    else if (assocType == Uml::AssociationType::Aggregation)
        typeName.append(QLatin1String("_Ptr"));
}

void AdaWriter::declareClass(UMLClassifier *c, QTextStream &ada)
{
    UMLClassifierList superclasses = c->getSuperClasses();
    UMLClassifier *firstSuperClass = 0;
    if (!superclasses.isEmpty()) {
        foreach (UMLClassifier* super, superclasses) {
            if (!super->isInterface()) {
                firstSuperClass = super;
                break;
            }
        }
        if (firstSuperClass == 0)
            firstSuperClass = superclasses.first();
    }
    const QString name = className(c);
    ada << indent() << "type " << name << " is ";
    if (c->isAbstract())
        ada << "abstract ";
    if (superclasses.isEmpty()) {
        ada << "tagged ";
    } else {
        ada << "new " << className(firstSuperClass, false);
        foreach (UMLClassifier* super, superclasses) {
            if (super->isInterface() && super != firstSuperClass)
                ada << " and " << className(super, false);
        }
        ada << " with ";
    }
}

/**
 * Call this method to generate Ada code for a UMLClassifier.
 * @param c the class to generate code for
 */
void AdaWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        logWarn0("AdaWriter::writeClass: Cannot write class of NULL concept!");
        return;
    }
    if (m_classesGenerated.contains(c))
        return;

    const bool isClass = !c->isInterface();
    QString classname = cleanName(c->name());
    QString pkg = packageName(c);
    QString fileName = pkg.toLower();
    fileName.replace(QLatin1Char('.'), QLatin1Char('-'));

    //find an appropriate name for our file
    fileName = overwritableName(c, fileName, QLatin1String(".ads"));
    if (fileName.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    QFile *file = 0;
    bool isNewFile = false;
    PackageFileMap::iterator it = m_pkgsGenerated.find(pkg);
    if (it != m_pkgsGenerated.end()) {
        file = it.value();
    } else {
        file = new QFile();
        if (!openFile(*file, fileName)) {
            emit codeGenerated(c, false);
            delete file;
            return;
        }
        m_pkgsGenerated[pkg] = file;
        isNewFile = true;
    }

    // Start generating the code.

    QTextStream ada(file);
    if (isNewFile) {
        //try to find a heading file(license, comments, etc)
        QString str;
        str = getHeadingFile(QLatin1String(".ads"));
        if (!str.isEmpty()) {
            str.replace(QRegExp(QLatin1String("%filename%")), fileName);
            str.replace(QRegExp(QLatin1String("%filepath%")), file->fileName());
            ada << str << m_endl;
        }

        // Import referenced classes.
        UMLPackageList imports;
        findObjectsRelated(c, imports);
        if (imports.count()) {
            foreach (UMLPackage* con, imports) {
                if (con->isUMLDatatype())
                    continue;
                QString pkgDep = packageName(con);
                if (pkgDep != pkg)
                    ada << "with " << pkgDep << "; " << m_endl;
            }
            ada << m_endl;
        }

        // Generate generic formals.
        UMLTemplateList template_params = c->getTemplateList();
        if (template_params.count()) {
            ada << indent() << "generic" << m_endl;
            m_indentLevel++;
            foreach (UMLTemplate* t, template_params) {
                QString formalName = t->name();
                QString typeName = t->getTypeName();
                if (typeName == QLatin1String("class")) {
                    ada << indent() << "type " << formalName << " is tagged private;"
                        << m_endl;
                } else {
                    // Check whether it's a data type.
                    UMLClassifier *typeObj = t->getType();
                    if (typeObj == 0) {
                        logError1("template_param %1: typeObj is NULL", typeName);
                        ada << indent() << "type " << formalName << " is new " << typeName
                            << " with private;  -- CHECK: codegen error"
                            << m_endl;
                    } else if (typeObj->isUMLDatatype()) {
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
        ada << "package " << pkg << " is" << m_endl << m_endl;
    }

    if (c->baseType() == UMLObject::ot_Enum) {
        UMLEnum *ue = c->asUMLEnum();
        UMLClassifierListItemList litList = ue->getFilteredList(UMLObject::ot_EnumLiteral);
        uint i = 0;
        ada << indent() << "type " << classname << " is (" << m_endl;
        m_indentLevel++;
        foreach (UMLClassifierListItem* lit, litList) {
            QString enumLiteral = cleanName(lit->name());
            ada << indent() << enumLiteral;
            if (++i < (uint)litList.count())
                ada << "," << m_endl;
        }
        m_indentLevel--;
        ada << ");" << m_endl << m_endl;
        ada << "end " << pkg << ";" << m_endl << m_endl;
        return;
    }
    if (! isOOClass(c)) {
        QString stype = c->stereotype();
        if (stype == QLatin1String("CORBAConstant")) {
            ada << indent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if (stype == QLatin1String("CORBAStruct")) {
            if (isClass) {
                UMLAttributeList atl = c->getAttributeList();
                ada << indent() << "type " << classname << " is record" << m_endl;
                m_indentLevel++;
                foreach (UMLAttribute* at,  atl) {
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
        } else if (stype == QLatin1String("CORBAUnion")) {
            ada << indent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if (stype == QLatin1String("CORBATypedef")) {
            ada << indent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else {
            ada << indent() << "-- " << stype << ": Unknown stereotype" << m_endl << m_endl;
        }
        ada << "end " << pkg << ";" << m_endl << m_endl;
        return;
    }

    UMLClassifierList superclasses = c->getSuperClasses();
    if (!superclasses.isEmpty()) {
        // Ensure that superclasses in same package are declared before this class.
        foreach (UMLClassifier* super, superclasses) {
            if (packageName(super) == pkg && !m_classesGenerated.contains(super)) {
                writeClass(super);
            }
        }
    }
    m_classesGenerated.append(c);

    // Write class Documentation if non-empty or if force option set.
    if (forceDoc() || !c->doc().isEmpty()) {
        ada << "--" << m_endl;
        ada << "-- class " << classname << m_endl;
        ada << formatDoc(c->doc(), QLatin1String("-- "));
        ada << m_endl;
    }

    const QString name = className(c);
    if (isClass) {
        declareClass(c, ada);
        ada << "private;" << m_endl << m_endl;
    } else {
        ada << indent() << "type " << name << " is interface";
        foreach (UMLClassifier* super, superclasses) {
            if (super->isInterface())
                ada << " and " << className(super, false);
        }
        ada << ";" << m_endl << m_endl;
    }
    ada << indent() << "type " << name << "_Ptr is access all " << name << "'Class;" << m_endl << m_endl;
    ada << indent() << "type " << name << "_Array is array (Positive range <>) of " << name << "_Ptr;" << m_endl << m_endl;
    ada << indent() << "type " << name << "_Array_Ptr is access " << name << "_Array;" << m_endl << m_endl;

    // Generate accessors for public attributes.
    UMLAttributeList atl;
    if (isClass) {
        UMLAttributeList atpub;

        atl = c->getAttributeList();

        foreach (UMLAttribute* at, atl) {
            if (at->visibility() == Uml::Visibility::Public)
                atpub.append(at);
        }
        if (forceSections() || atpub.count())
            ada << indent() << "-- Accessors for public attributes:" << m_endl << m_endl;

        foreach (UMLAttribute* at, atpub) {
            QString member = cleanName(at->name());
            ada << indent() << "procedure Set_" << member << " (";
            if (! at->isStatic())
                ada << "Self : access " << name << "; ";
            ada << "To : " << at->getTypeName() << ");" << m_endl;
            ada << indent() << "function  Get_" << member;
            if (! at->isStatic())
                ada << " (Self : access " << name << ")";
            ada << " return " << at->getTypeName() << ";" << m_endl << m_endl;
        }
    }

    // Generate public operations.
    UMLOperationList opl(c->getOpList());
    UMLOperationList oppub;
    foreach (UMLOperation* op, opl) {
        if (op->visibility() == Uml::Visibility::Public)
            oppub.append(op);
    }
    if (forceSections() || oppub.count())
        ada << indent() << "-- Public methods:" << m_endl << m_endl;

    foreach (UMLOperation* op, oppub) {
        writeOperation(op, ada);
    }

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
    bool use_procedure = (rettype.isEmpty() || rettype == QLatin1String("void"));

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
    UMLClassifier *parentClassifier = op->umlParent()->asUMLClassifier();
    if (! op->isStatic()) {
        ada << "Self : access " << className(parentClassifier);
        if (atl.count())
            ada << ";" << m_endl;
    }
    if (atl.count()) {
        uint i = 0;
        m_indentLevel++;
        foreach (UMLAttribute* at, atl) {
            ada << indent();
            if (is_comment)
                ada << "-- ";
            ada << cleanName(at->name()) << " : ";
            Uml::ParameterDirection::Enum pk = at->getParmKind();
            if (pk == Uml::ParameterDirection::Out)
                ada << "out ";
            else if (pk == Uml::ParameterDirection::InOut)
                ada << "in out ";
            else
                ada << "in ";
            ada << at->getTypeName();
            if (! at->getInitialValue().isEmpty())
                ada << " := " << at->getInitialValue();
            if (++i < (uint)atl.count()) //FIXME gcc warning
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
QStringList AdaWriter::defaultDatatypes() const
{
    QStringList l;
    l.append(QLatin1String("Boolean"));
    l.append(QLatin1String("Character"));
    l.append(QLatin1String("Positive"));
    l.append(QLatin1String("Natural"));
    l.append(QLatin1String("Integer"));
    l.append(QLatin1String("Short_Integer"));
    l.append(QLatin1String("Long_Integer"));
    l.append(QLatin1String("Float"));
    l.append(QLatin1String("Long_Float"));
    l.append(QLatin1String("Duration"));
    l.append(QLatin1String("String"));
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

    if (keywords.isEmpty()) {
        keywords.append(QLatin1String("abort"));
        keywords.append(QLatin1String("abs"));
        keywords.append(QLatin1String("abstract"));
        keywords.append(QLatin1String("accept"));
        keywords.append(QLatin1String("access"));
        keywords.append(QLatin1String("aliased"));
        keywords.append(QLatin1String("all"));
        keywords.append(QLatin1String("and"));
        keywords.append(QLatin1String("Argument_Error"));
        keywords.append(QLatin1String("array"));
        keywords.append(QLatin1String("Assert_Failure"));
        keywords.append(QLatin1String("at"));
        keywords.append(QLatin1String("begin"));
        keywords.append(QLatin1String("body"));
        keywords.append(QLatin1String("Boolean"));
        keywords.append(QLatin1String("case"));
        keywords.append(QLatin1String("Character"));
        keywords.append(QLatin1String("constant"));
        keywords.append(QLatin1String("Constraint_Error"));
        keywords.append(QLatin1String("Conversion_Error"));
        keywords.append(QLatin1String("Data_Error"));
        keywords.append(QLatin1String("declare"));
        keywords.append(QLatin1String("delay"));
        keywords.append(QLatin1String("delta"));
        keywords.append(QLatin1String("Dereference_Error"));
        keywords.append(QLatin1String("Device_Error"));
        keywords.append(QLatin1String("digits"));
        keywords.append(QLatin1String("do"));
        keywords.append(QLatin1String("Duration"));
        keywords.append(QLatin1String("else"));
        keywords.append(QLatin1String("elsif"));
        keywords.append(QLatin1String("end"));
        keywords.append(QLatin1String("End_Error"));
        keywords.append(QLatin1String("entry"));
        keywords.append(QLatin1String("exception"));
        keywords.append(QLatin1String("exit"));
        keywords.append(QLatin1String("false"));
        keywords.append(QLatin1String("Float"));
        keywords.append(QLatin1String("for"));
        keywords.append(QLatin1String("function"));
        keywords.append(QLatin1String("generic"));
        keywords.append(QLatin1String("goto"));
        keywords.append(QLatin1String("if"));
        keywords.append(QLatin1String("in"));
        keywords.append(QLatin1String("Index_Error"));
        keywords.append(QLatin1String("Integer"));
        keywords.append(QLatin1String("interface"));
        keywords.append(QLatin1String("is"));
        keywords.append(QLatin1String("Layout_Error"));
        keywords.append(QLatin1String("Length_Error"));
        keywords.append(QLatin1String("limited"));
        keywords.append(QLatin1String("Long_Float"));
        keywords.append(QLatin1String("Long_Integer"));
        keywords.append(QLatin1String("Long_Long_Float"));
        keywords.append(QLatin1String("Long_Long_Integer"));
        keywords.append(QLatin1String("loop"));
        keywords.append(QLatin1String("mod"));
        keywords.append(QLatin1String("Mode_Error"));
        keywords.append(QLatin1String("Name_Error"));
        keywords.append(QLatin1String("Natural"));
        keywords.append(QLatin1String("new"));
        keywords.append(QLatin1String("not"));
        keywords.append(QLatin1String("null"));
        keywords.append(QLatin1String("of"));
        keywords.append(QLatin1String("or"));
        keywords.append(QLatin1String("others"));
        keywords.append(QLatin1String("out"));
        keywords.append(QLatin1String("package"));
        keywords.append(QLatin1String("Pattern_Error"));
        keywords.append(QLatin1String("Picture_Error"));
        keywords.append(QLatin1String("Pointer_Error"));
        keywords.append(QLatin1String("Positive"));
        keywords.append(QLatin1String("pragma"));
        keywords.append(QLatin1String("private"));
        keywords.append(QLatin1String("procedure"));
        keywords.append(QLatin1String("Program_Error"));
        keywords.append(QLatin1String("protected"));
        keywords.append(QLatin1String("raise"));
        keywords.append(QLatin1String("range"));
        keywords.append(QLatin1String("record"));
        keywords.append(QLatin1String("rem"));
        keywords.append(QLatin1String("renames"));
        keywords.append(QLatin1String("requeue"));
        keywords.append(QLatin1String("return"));
        keywords.append(QLatin1String("reverse"));
        keywords.append(QLatin1String("select"));
        keywords.append(QLatin1String("separate"));
        keywords.append(QLatin1String("Short_Float"));
        keywords.append(QLatin1String("Short_Integer"));
        keywords.append(QLatin1String("Short_Short_Float"));
        keywords.append(QLatin1String("Short_Short_Integer"));
        keywords.append(QLatin1String("Status_Error"));
        keywords.append(QLatin1String("Storage_Error"));
        keywords.append(QLatin1String("String"));
        keywords.append(QLatin1String("subtype"));
        keywords.append(QLatin1String("Tag_Error"));
        keywords.append(QLatin1String("tagged"));
        keywords.append(QLatin1String("task"));
        keywords.append(QLatin1String("Tasking_Error"));
        keywords.append(QLatin1String("terminate"));
        keywords.append(QLatin1String("Terminator_Error"));
        keywords.append(QLatin1String("then"));
        keywords.append(QLatin1String("Time_Error"));
        keywords.append(QLatin1String("Translation_Error"));
        keywords.append(QLatin1String("true"));
        keywords.append(QLatin1String("type"));
        keywords.append(QLatin1String("until"));
        keywords.append(QLatin1String("Update_Error"));
        keywords.append(QLatin1String("use"));
        keywords.append(QLatin1String("Use_Error"));
        keywords.append(QLatin1String("when"));
        keywords.append(QLatin1String("while"));
        keywords.append(QLatin1String("Wide_Character"));
        keywords.append(QLatin1String("Wide_String"));
        keywords.append(QLatin1String("with"));
        keywords.append(QLatin1String("xor"));
    }

    return keywords;
}

void AdaWriter::finalizeRun()
{
    PackageFileMap::iterator end(m_pkgsGenerated.end());
    for (PackageFileMap::iterator i = m_pkgsGenerated.begin(); i != end; ++i) {
        QString pkg = i.key();
        QFile *file = i.value();
        QTextStream ada(file);
        ada << m_endl << "private" << m_endl << m_endl;
        foreach (UMLClassifier* c, m_classesGenerated) {
            if (packageName(c) != pkg)
                continue;
            bool isClass = !c->isInterface();
            if (isClass) {
                declareClass(c, ada);
                ada << "record" << m_endl;
                m_indentLevel++;

                UMLAssociationList aggregations = c->getAggregations();
                UMLAssociationList compositions = c->getCompositions();

                if (forceSections() || !aggregations.isEmpty()) {
                    ada << indent() << "-- Aggregations:" << m_endl;
                    foreach (UMLAssociation *a, aggregations) {
                        if (c != a->getObject(Uml::RoleType::A))
                            continue;
                        QString typeName, roleName;
                        computeAssocTypeAndRole(c, a, typeName, roleName);
                        ada << indent() << roleName << " : " << typeName << ";" << m_endl;
                    }
                    ada << m_endl;
                }
                if (forceSections() || !compositions.isEmpty()) {
                    ada << indent() << "-- Compositions:" << m_endl;
                    foreach (UMLAssociation *a, compositions) {
                        if (c != a->getObject(Uml::RoleType::A))
                            continue;
                        QString typeName, roleName;
                        computeAssocTypeAndRole(c, a, typeName, roleName);
                        ada << indent() << roleName << " : " << typeName << ";" << m_endl;
                    }
                    ada << m_endl;
                }

                UMLAttributeList atl = c->getAttributeList();
                if (forceSections() || atl.count()) {
                    ada << indent() << "-- Attributes:" << m_endl;
                    foreach (UMLAttribute* at, atl) {
                        if (!at || at->isStatic())
                            continue;
                        ada << indent() << cleanName(at->name()) << " : "
                        << at->getTypeName();
                        if (!at->getInitialValue().isEmpty() && !at->getInitialValue().toLatin1().isEmpty())
                            ada << " := " << at->getInitialValue();
                        ada << ";" << m_endl;
                    }
                }
                const bool haveAttrs = (atl.count() != 0);
                if (aggregations.isEmpty() && compositions.isEmpty() && !haveAttrs)
                    ada << indent() << "null;" << m_endl;
                m_indentLevel--;
                ada << indent() << "end record;" << m_endl << m_endl;
                if (haveAttrs) {
                    bool seen_static_attr = false;
                    foreach (UMLAttribute* at, atl) {
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
                        if (at && ! at->getInitialValue().isEmpty() && ! at->getInitialValue().toLatin1().isEmpty())
                            ada << " := " << at->getInitialValue();
                        ada << ";" << m_endl;
                    }
                    if (seen_static_attr)
                        ada << m_endl;
                }
            }
            UMLOperationList opl(c->getOpList());
            // Generate protected operations.
            UMLOperationList opprot;
            foreach (UMLOperation* op,  opl) {
                if (op->visibility() == Uml::Visibility::Protected)
                    opprot.append(op);
            }
            if (forceSections() || opprot.count())
                ada << indent() << "-- Protected methods:" << m_endl << m_endl;
            foreach (UMLOperation* op, opprot) {
                writeOperation(op, ada);
            }

            // Generate private operations.
            // These are currently only generated as comments in the private part
            // of the spec.
            // Once umbrello supports the merging of automatically generated and
            // hand written code sections, private operations should be generated
            // into the package body.
            UMLOperationList oppriv;
            foreach (UMLOperation* op, opl) {
                const Uml::Visibility::Enum vis = op->visibility();
                if (vis == Uml::Visibility::Private ||
                    vis == Uml::Visibility::Implementation)
                oppriv.append(op);
            }
            if (forceSections() || oppriv.count())
                ada << indent() << "-- Private methods:" << m_endl << m_endl;
            foreach (UMLOperation* op, oppriv) {
                writeOperation(op, ada, true);
            }
        }
        ada << m_endl << "end " << i.key() << ";" << m_endl;
        file->close();
        emit showGeneratedFile(file->fileName());
        delete file;
    }
}


