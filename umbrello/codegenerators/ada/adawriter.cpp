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

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

const QString AdaWriter::defaultPackageSuffix = QStringLiteral("_Holder");

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
    if (stype == QStringLiteral("CORBAConstant") || stype == QStringLiteral("CORBATypedef") ||
            stype == QStringLiteral("CORBAStruct") || stype == QStringLiteral("CORBAUnion"))
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
        retval.append(QStringLiteral("Object"));
    } else {
        if (! inOwnScope)
            retval = umlPkg->fullyQualifiedName(QStringLiteral(".")) + QLatin1Char('.');
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
        umlPkg = nullptr;

    const UMLClassifier *c = p->asUMLClassifier();
    if (umlPkg == nullptr) {
        retval = className;
        if (c == nullptr || !isOOClass(c))
            retval.append(defaultPackageSuffix);
    } else {
        retval = umlPkg->fullyQualifiedName(QStringLiteral("."));
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
    if (assocEnd == nullptr)
        return;
    const Uml::AssociationType::Enum assocType = a->getAssocType();
    if (assocType != Uml::AssociationType::Aggregation && assocType != Uml::AssociationType::Composition)
        return;
    const QString multi = a->getMultiplicity(Uml::RoleType::B);
    bool hasNonUnityMultiplicity = (!multi.isEmpty() && multi != QStringLiteral("1"));
    hasNonUnityMultiplicity &= !multi.contains(QRegularExpression(QStringLiteral("^1 *\\.\\. *1$")));
    roleName = cleanName(a->getRoleName(Uml::RoleType::B));
    if (roleName.isEmpty())
        roleName = cleanName(a->name());
    if (roleName.isEmpty()) {
        QString artificialName = cleanName(assocEnd->name());
        if (hasNonUnityMultiplicity) {
            roleName = artificialName;
            roleName.append(QStringLiteral("_Vector"));
        } else {
            roleName = QStringLiteral("M_");
            roleName.append(artificialName);
        }
    }
    typeName = className(assocEnd, (assocEnd == c));
    if (hasNonUnityMultiplicity)
        typeName.append(QStringLiteral("_Array_Ptr"));
    else if (assocType == Uml::AssociationType::Aggregation)
        typeName.append(QStringLiteral("_Ptr"));
}

void AdaWriter::declareClass(UMLClassifier *c, QTextStream &ada)
{
    UMLClassifierList superclasses = c->getSuperClasses();
    UMLClassifier *firstSuperClass = nullptr;
    if (!superclasses.isEmpty()) {
        for(UMLClassifier* super : superclasses) {
            if (!super->isInterface()) {
                firstSuperClass = super;
                break;
            }
        }
        if (firstSuperClass == nullptr)
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
        for(UMLClassifier* super : superclasses) {
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
        logWarn0("AdaWriter::writeClass: Cannot write class of NULL classifier!");
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
    fileName = overwritableName(c, fileName, QStringLiteral(".ads"));
    if (fileName.isEmpty()) {
        Q_EMIT codeGenerated(c, false);
        return;
    }

    QFile *file = nullptr;
    bool isNewFile = false;
    PackageFileMap::iterator it = m_pkgsGenerated.find(pkg);
    if (it != m_pkgsGenerated.end()) {
        file = it.value();
    } else {
        file = new QFile();
        if (!openFile(*file, fileName)) {
            Q_EMIT codeGenerated(c, false);
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
        str = getHeadingFile(QStringLiteral(".ads"));
        if (!str.isEmpty()) {
            str.replace(QRegularExpression(QStringLiteral("%filename%")), fileName);
            str.replace(QRegularExpression(QStringLiteral("%filepath%")), file->fileName());
            ada << str << m_endl;
        }

        // Import referenced classes.
        UMLPackageList imports;
        findObjectsRelated(c, imports);
        if (imports.count()) {
            for(UMLPackage* con : imports) {
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
            for(UMLTemplate* t : template_params) {
                QString formalName = t->name();
                QString typeName = t->getTypeName();
                if (typeName == QStringLiteral("class")) {
                    ada << indent() << "type " << formalName << " is tagged private;"
                        << m_endl;
                } else {
                    // Check whether it's a data type.
                    UMLClassifier *typeObj = t->getType();
                    if (typeObj == nullptr) {
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
        for(UMLClassifierListItem* lit : litList) {
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
        if (stype == QStringLiteral("CORBAConstant")) {
            ada << indent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if (stype == QStringLiteral("CORBAStruct")) {
            if (isClass) {
                UMLAttributeList atl = c->getAttributeList();
                ada << indent() << "type " << classname << " is record" << m_endl;
                m_indentLevel++;
                for(UMLAttribute* at :  atl) {
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
        } else if (stype == QStringLiteral("CORBAUnion")) {
            ada << indent() << "-- " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if (stype == QStringLiteral("CORBATypedef")) {
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
        for(UMLClassifier* super : superclasses) {
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
        ada << formatDoc(c->doc(), QStringLiteral("-- "));
        ada << m_endl;
    }

    const QString name = className(c);
    if (isClass) {
        declareClass(c, ada);
        ada << "private;" << m_endl << m_endl;
    } else {
        ada << indent() << "type " << name << " is interface";
        for(UMLClassifier* super : superclasses) {
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

        for(UMLAttribute* at : atl) {
            if (at->visibility() == Uml::Visibility::Public)
                atpub.append(at);
        }
        if (forceSections() || atpub.count())
            ada << indent() << "-- Accessors for public attributes:" << m_endl << m_endl;

        for(UMLAttribute* at : atpub) {
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
    for(UMLOperation* op : opl) {
        if (op->visibility() == Uml::Visibility::Public)
            oppub.append(op);
    }
    if (forceSections() || oppub.count())
        ada << indent() << "-- Public methods:" << m_endl << m_endl;

    for(UMLOperation* op : oppub) {
        writeOperation(op, ada);
    }

    Q_EMIT codeGenerated(c, true);
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
    bool use_procedure = (rettype.isEmpty() || rettype == QStringLiteral("void"));

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
        for(UMLAttribute* at : atl) {
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
    l.append(QStringLiteral("Boolean"));
    l.append(QStringLiteral("Character"));
    l.append(QStringLiteral("Positive"));
    l.append(QStringLiteral("Natural"));
    l.append(QStringLiteral("Integer"));
    l.append(QStringLiteral("Short_Integer"));
    l.append(QStringLiteral("Long_Integer"));
    l.append(QStringLiteral("Float"));
    l.append(QStringLiteral("Long_Float"));
    l.append(QStringLiteral("Duration"));
    l.append(QStringLiteral("String"));
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
        keywords.append(QStringLiteral("abort"));
        keywords.append(QStringLiteral("abs"));
        keywords.append(QStringLiteral("abstract"));
        keywords.append(QStringLiteral("accept"));
        keywords.append(QStringLiteral("access"));
        keywords.append(QStringLiteral("aliased"));
        keywords.append(QStringLiteral("all"));
        keywords.append(QStringLiteral("and"));
        keywords.append(QStringLiteral("Argument_Error"));
        keywords.append(QStringLiteral("array"));
        keywords.append(QStringLiteral("Assert_Failure"));
        keywords.append(QStringLiteral("at"));
        keywords.append(QStringLiteral("begin"));
        keywords.append(QStringLiteral("body"));
        keywords.append(QStringLiteral("Boolean"));
        keywords.append(QStringLiteral("case"));
        keywords.append(QStringLiteral("Character"));
        keywords.append(QStringLiteral("constant"));
        keywords.append(QStringLiteral("Constraint_Error"));
        keywords.append(QStringLiteral("Conversion_Error"));
        keywords.append(QStringLiteral("Data_Error"));
        keywords.append(QStringLiteral("declare"));
        keywords.append(QStringLiteral("delay"));
        keywords.append(QStringLiteral("delta"));
        keywords.append(QStringLiteral("Dereference_Error"));
        keywords.append(QStringLiteral("Device_Error"));
        keywords.append(QStringLiteral("digits"));
        keywords.append(QStringLiteral("do"));
        keywords.append(QStringLiteral("Duration"));
        keywords.append(QStringLiteral("else"));
        keywords.append(QStringLiteral("elsif"));
        keywords.append(QStringLiteral("end"));
        keywords.append(QStringLiteral("End_Error"));
        keywords.append(QStringLiteral("entry"));
        keywords.append(QStringLiteral("exception"));
        keywords.append(QStringLiteral("exit"));
        keywords.append(QStringLiteral("false"));
        keywords.append(QStringLiteral("Float"));
        keywords.append(QStringLiteral("for"));
        keywords.append(QStringLiteral("function"));
        keywords.append(QStringLiteral("generic"));
        keywords.append(QStringLiteral("goto"));
        keywords.append(QStringLiteral("if"));
        keywords.append(QStringLiteral("in"));
        keywords.append(QStringLiteral("Index_Error"));
        keywords.append(QStringLiteral("Integer"));
        keywords.append(QStringLiteral("interface"));
        keywords.append(QStringLiteral("is"));
        keywords.append(QStringLiteral("Layout_Error"));
        keywords.append(QStringLiteral("Length_Error"));
        keywords.append(QStringLiteral("limited"));
        keywords.append(QStringLiteral("Long_Float"));
        keywords.append(QStringLiteral("Long_Integer"));
        keywords.append(QStringLiteral("Long_Long_Float"));
        keywords.append(QStringLiteral("Long_Long_Integer"));
        keywords.append(QStringLiteral("loop"));
        keywords.append(QStringLiteral("mod"));
        keywords.append(QStringLiteral("Mode_Error"));
        keywords.append(QStringLiteral("Name_Error"));
        keywords.append(QStringLiteral("Natural"));
        keywords.append(QStringLiteral("new"));
        keywords.append(QStringLiteral("not"));
        keywords.append(QStringLiteral("null"));
        keywords.append(QStringLiteral("of"));
        keywords.append(QStringLiteral("or"));
        keywords.append(QStringLiteral("others"));
        keywords.append(QStringLiteral("out"));
        keywords.append(QStringLiteral("package"));
        keywords.append(QStringLiteral("Pattern_Error"));
        keywords.append(QStringLiteral("Picture_Error"));
        keywords.append(QStringLiteral("Pointer_Error"));
        keywords.append(QStringLiteral("Positive"));
        keywords.append(QStringLiteral("pragma"));
        keywords.append(QStringLiteral("private"));
        keywords.append(QStringLiteral("procedure"));
        keywords.append(QStringLiteral("Program_Error"));
        keywords.append(QStringLiteral("protected"));
        keywords.append(QStringLiteral("raise"));
        keywords.append(QStringLiteral("range"));
        keywords.append(QStringLiteral("record"));
        keywords.append(QStringLiteral("rem"));
        keywords.append(QStringLiteral("renames"));
        keywords.append(QStringLiteral("requeue"));
        keywords.append(QStringLiteral("return"));
        keywords.append(QStringLiteral("reverse"));
        keywords.append(QStringLiteral("select"));
        keywords.append(QStringLiteral("separate"));
        keywords.append(QStringLiteral("Short_Float"));
        keywords.append(QStringLiteral("Short_Integer"));
        keywords.append(QStringLiteral("Short_Short_Float"));
        keywords.append(QStringLiteral("Short_Short_Integer"));
        keywords.append(QStringLiteral("Status_Error"));
        keywords.append(QStringLiteral("Storage_Error"));
        keywords.append(QStringLiteral("String"));
        keywords.append(QStringLiteral("subtype"));
        keywords.append(QStringLiteral("Tag_Error"));
        keywords.append(QStringLiteral("tagged"));
        keywords.append(QStringLiteral("task"));
        keywords.append(QStringLiteral("Tasking_Error"));
        keywords.append(QStringLiteral("terminate"));
        keywords.append(QStringLiteral("Terminator_Error"));
        keywords.append(QStringLiteral("then"));
        keywords.append(QStringLiteral("Time_Error"));
        keywords.append(QStringLiteral("Translation_Error"));
        keywords.append(QStringLiteral("true"));
        keywords.append(QStringLiteral("type"));
        keywords.append(QStringLiteral("until"));
        keywords.append(QStringLiteral("Update_Error"));
        keywords.append(QStringLiteral("use"));
        keywords.append(QStringLiteral("Use_Error"));
        keywords.append(QStringLiteral("when"));
        keywords.append(QStringLiteral("while"));
        keywords.append(QStringLiteral("Wide_Character"));
        keywords.append(QStringLiteral("Wide_String"));
        keywords.append(QStringLiteral("with"));
        keywords.append(QStringLiteral("xor"));
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
        for(UMLClassifier* c : m_classesGenerated) {
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
                    for(UMLAssociation *a : aggregations) {
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
                    for(UMLAssociation *a : compositions) {
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
                    for(UMLAttribute* at : atl) {
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
                    for(UMLAttribute* at : atl) {
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
            for(UMLOperation* op : opl) {
                if (op->visibility() == Uml::Visibility::Protected)
                    opprot.append(op);
            }
            if (forceSections() || opprot.count())
                ada << indent() << "-- Protected methods:" << m_endl << m_endl;
            for(UMLOperation* op : opprot) {
                writeOperation(op, ada);
            }

            // Generate private operations.
            // These are currently only generated as comments in the private part
            // of the spec.
            // Once umbrello supports the merging of automatically generated and
            // hand written code sections, private operations should be generated
            // into the package body.
            UMLOperationList oppriv;
            for(UMLOperation* op : opl) {
                const Uml::Visibility::Enum vis = op->visibility();
                if (vis == Uml::Visibility::Private ||
                    vis == Uml::Visibility::Implementation)
                oppriv.append(op);
            }
            if (forceSections() || oppriv.count())
                ada << indent() << "-- Private methods:" << m_endl << m_endl;
            for(UMLOperation* op : oppriv) {
                writeOperation(op, ada, true);
            }
        }
        ada << m_endl << "end " << i.key() << ";" << m_endl;
        file->close();
        Q_EMIT showGeneratedFile(file->fileName());
        delete file;
    }
}


