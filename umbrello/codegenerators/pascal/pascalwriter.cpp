/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

#include <KLocalizedString>
#include <KMessageBox>

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

const QString PascalWriter::defaultPackageSuffix = QStringLiteral("_Holder");

/**
 * Basic Constructor.
 */
PascalWriter::PascalWriter()
 : SimpleCodeGenerator()
{
}

/**
 * Empty Destructor.
 */
PascalWriter::~PascalWriter()
{
}

/**
 * Returns "Pascal".
 * @return   the programming language identifier
 */
Uml::ProgrammingLanguage::Enum PascalWriter::language() const
{
    return Uml::ProgrammingLanguage::Pascal;
}

/**
 *
 */
bool PascalWriter::isOOClass(const UMLClassifier *c)
{
    UMLObject::ObjectType ot = c->baseType();
    if (ot == UMLObject::ot_Interface)
        return true;
    if (ot == UMLObject::ot_Enum || ot == UMLObject::ot_Datatype)
        return false;
    if (ot != UMLObject::ot_Class) {
        logWarn1("PascalWriter::isOOClass: unknown object type %1", UMLObject::toString(ot));
        return false;
    }
    QString stype = c->stereotype();
    if (stype == QStringLiteral("CORBAConstant") || stype == QStringLiteral("CORBATypedef") ||
            stype == QStringLiteral("CORBAStruct") || stype == QStringLiteral("CORBAUnion"))
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
        umlPkg = 0;

    const UMLClassifier *c = p->asUMLClassifier();
    if (umlPkg == 0) {
        retval = className;
        if (c == 0 || !isOOClass(c))
            retval.append(defaultPackageSuffix);
    } else {
        retval = umlPkg->fullyQualifiedName(QStringLiteral("."));
        if (c && isOOClass(c)) {
            retval.append(QStringLiteral("."));
            retval.append(className);
        }
    }
    if (! withType)
        return retval;
    if (c && isOOClass(c)) {
        retval.append(QStringLiteral(".Object"));
        if (! byValue)
            retval.append(QStringLiteral("_Ptr"));
    } else {
        retval.append(QStringLiteral("."));
        retval.append(className);
    }
    return retval;
}

void PascalWriter::computeAssocTypeAndRole
(UMLAssociation *a, QString& typeName, QString& roleName)
{
    roleName = a->getRoleName(Uml::RoleType::A);
    if (roleName.isEmpty()) {
        if (a->getMultiplicity(Uml::RoleType::A).isEmpty()) {
            roleName = QStringLiteral("M_");
            roleName.append(typeName);
        } else {
            roleName = typeName;
            roleName.append(QStringLiteral("_Vector"));
        }
    }
    const UMLClassifier* c = a->getObject(Uml::RoleType::A)->asUMLClassifier();
    if (c == 0)
        return;
    typeName = cleanName(c->name());
    if (! a->getMultiplicity(Uml::RoleType::A).isEmpty())
        typeName.append(QStringLiteral("_Array_Access"));
}

/**
 * Call this method to generate Pascal code for a UMLClassifier.
 * @param c   the class to generate code for
 */
void PascalWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        logWarn0("PascalWriter::writeClass: Cannot write class of NULL classifier");
        return;
    }

    const bool isClass = !c->isInterface();
    QString classname = cleanName(c->name());
    QString fileName = qualifiedName(c).toLower();
    fileName.replace(QLatin1Char('.'), QLatin1Char('-'));

    //find an appropriate name for our file
    fileName = overwritableName(c, fileName, QStringLiteral(".pas"));
    if (fileName.isEmpty()) {
        Q_EMIT codeGenerated(c, false);
        return;
    }

    QFile file;
    if (!openFile(file, fileName)) {
        Q_EMIT codeGenerated(c, false);
        return;
    }

    // Start generating the code.

    QTextStream pas(&file);
    //try to find a heading file(license, comments, etc)
    QString str;
    str = getHeadingFile(QStringLiteral(".pas"));
    if (!str.isEmpty()) {
        str.replace(QRegularExpression(QStringLiteral("%filename%")), fileName);
        str.replace(QRegularExpression(QStringLiteral("%filepath%")), file.fileName());
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
        for(UMLPackage* con : imports) {
            if (!con->isUMLDatatype()) {
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
        const UMLEnum *ue = c->asUMLEnum();
        UMLClassifierListItemList litList = ue->getFilteredList(UMLObject::ot_EnumLiteral);
        uint i = 0;
        pas << indent() << classname << " = (" << m_endl;
        m_indentLevel++;
        for(UMLClassifierListItem *lit : litList) {
            QString enumLiteral = cleanName(lit->name());
            pas << indent() << enumLiteral;
            if (++i < (uint)litList.count())
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
        if (stype == QStringLiteral("CORBAConstant")) {
            pas << indent() << "// " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if(stype == QStringLiteral("CORBAStruct")) {
            if (isClass) {

                pas << indent() << classname << " = record" << m_endl;
                m_indentLevel++;
                for(UMLAttribute* at : atl) {
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
        } else if(stype == QStringLiteral("CORBAUnion")) {
            pas << indent() << "// " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if(stype == QStringLiteral("CORBATypedef")) {
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
        pas << formatDoc(c->doc(), QStringLiteral("// "));
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

        for(UMLAttribute* at : atpub) {
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

    for(UMLOperation* op : opl) {
         if (op->visibility() == Uml::Visibility::Public)
            oppub.append(op);
    }
    if (forceSections() || oppub.count())
        pas << indent() << "// Public methods:" << m_endl << m_endl;
    for(UMLOperation* op : oppub)
        writeOperation(op, pas);

    UMLAttributeList atprot = c->getAttributeList(Uml::Visibility::Protected);
    if (atprot.count()) {
        pas << "protected" << m_endl << m_endl;

        for(UMLAttribute*  at : atprot) {
            // if (at->getStatic())
            //     continue;
            pas << indent() << cleanName(at->name()) << " : "
                << at->getTypeName();
            if (!at->getInitialValue().isEmpty())
                pas << " := " << at->getInitialValue();
            pas << ";" << m_endl;
        }
        pas << m_endl;
    }

    UMLAttributeList atpriv = c->getAttributeList(Uml::Visibility::Private);
    if (atpriv.count()) {
        pas << "private" << m_endl << m_endl;

        for(UMLAttribute* at : atpriv) {
            if (at) {
                pas << indent() << cleanName(at->name()) << " : "
                    << at->getTypeName();

                // if (at->getStatic())
                //     continue;

                if (!at->getInitialValue().isEmpty())
                    pas << " := " << at->getInitialValue();

                pas << ";" << m_endl;
            }
        }
        pas << m_endl;
    }
    pas << indent() << "end;" << m_endl << m_endl;

    pas << indent() << "P" << classname << " = ^" << classname <<";" << m_endl << m_endl;

    m_indentLevel--;
    pas << "end;" << m_endl << m_endl;
    file.close();
    Q_EMIT codeGenerated(c, true);
    Q_EMIT showGeneratedFile(file.fileName());
}

/**
 * Write one operation.
 * @param op the class for which we are generating code
 * @param pas the stream associated with the output file
 * @param is_comment  specifying true generates the operation as commented out
 */
void PascalWriter::writeOperation(UMLOperation *op, QTextStream &pas, bool is_comment)
{
    if (op->isStatic()) {
        pas << "// TODO: generate status method " << op->name() << m_endl;
        return;
    }
    UMLAttributeList atl = op->getParmList();
    QString rettype = op->getTypeName();
    bool use_procedure = (rettype.isEmpty() || rettype == QStringLiteral("void"));

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
        for(UMLAttribute *at : atl) {
            pas << indent();
            if (is_comment)
                pas << "// ";
            pas << cleanName(at->name()) << " : ";
            Uml::ParameterDirection::Enum pk = at->getParmKind();
            if (pk != Uml::ParameterDirection::In)
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

/**
 * Returns the default datatypes in a list.
 * @return  the list of default datatypes
 */
QStringList PascalWriter::defaultDatatypes() const
{
    QStringList l;
    l.append(QStringLiteral("AnsiString"));
    l.append(QStringLiteral("Boolean"));
    l.append(QStringLiteral("Byte"));
    l.append(QStringLiteral("ByteBool"));
    l.append(QStringLiteral("Cardinal"));
    l.append(QStringLiteral("Character"));
    l.append(QStringLiteral("Currency"));
    l.append(QStringLiteral("Double"));
    l.append(QStringLiteral("Extended"));
    l.append(QStringLiteral("Int64"));
    l.append(QStringLiteral("Integer"));
    l.append(QStringLiteral("Longint"));
    l.append(QStringLiteral("LongBool"));
    l.append(QStringLiteral("Longword"));
    l.append(QStringLiteral("QWord"));
    l.append(QStringLiteral("Real"));
    l.append(QStringLiteral("Shortint"));
    l.append(QStringLiteral("ShortString"));
    l.append(QStringLiteral("Single"));
    l.append(QStringLiteral("Smallint"));
    l.append(QStringLiteral("String"));
    l.append(QStringLiteral("WideString"));
    l.append(QStringLiteral("Word"));
    return l;
}

/**
 * Check whether the given string is a reserved word for the
 * language of this code generator.
 * @param rPossiblyReservedKeyword   the string to check
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
 * @return   the list of reserved keywords
 */
QStringList PascalWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords.append(QStringLiteral("absolute"));
        keywords.append(QStringLiteral("abstract"));
        keywords.append(QStringLiteral("and"));
        keywords.append(QStringLiteral("array"));
        keywords.append(QStringLiteral("as"));
        keywords.append(QStringLiteral("asm"));
        keywords.append(QStringLiteral("assembler"));
        keywords.append(QStringLiteral("automated"));
        keywords.append(QStringLiteral("begin"));
        keywords.append(QStringLiteral("case"));
        keywords.append(QStringLiteral("cdecl"));
        keywords.append(QStringLiteral("class"));
        keywords.append(QStringLiteral("const"));
        keywords.append(QStringLiteral("constructor"));
        keywords.append(QStringLiteral("contains"));
        keywords.append(QStringLiteral("default"));
        keywords.append(QStringLiteral("deprecated"));
        keywords.append(QStringLiteral("destructor"));
        keywords.append(QStringLiteral("dispid"));
        keywords.append(QStringLiteral("dispinterface"));
        keywords.append(QStringLiteral("div"));
        keywords.append(QStringLiteral("do"));
        keywords.append(QStringLiteral("downto"));
        keywords.append(QStringLiteral("dynamic"));
        keywords.append(QStringLiteral("else"));
        keywords.append(QStringLiteral("end"));
        keywords.append(QStringLiteral("except"));
        keywords.append(QStringLiteral("export"));
        keywords.append(QStringLiteral("exports"));
        keywords.append(QStringLiteral("external"));
        keywords.append(QStringLiteral("far"));
        keywords.append(QStringLiteral("file"));
        keywords.append(QStringLiteral("final"));
        keywords.append(QStringLiteral("finalization"));
        keywords.append(QStringLiteral("finally"));
        keywords.append(QStringLiteral("for"));
        keywords.append(QStringLiteral("forward"));
        keywords.append(QStringLiteral("function"));
        keywords.append(QStringLiteral("goto"));
        keywords.append(QStringLiteral("if"));
        keywords.append(QStringLiteral("implementation"));
        keywords.append(QStringLiteral("implements"));
        keywords.append(QStringLiteral("in"));
        keywords.append(QStringLiteral("index"));
        keywords.append(QStringLiteral("inherited"));
        keywords.append(QStringLiteral("initialization"));
        keywords.append(QStringLiteral("inline"));
        keywords.append(QStringLiteral("inline"));
        keywords.append(QStringLiteral("interface"));
        keywords.append(QStringLiteral("is"));
        keywords.append(QStringLiteral("label"));
        keywords.append(QStringLiteral("library"));
        keywords.append(QStringLiteral("library"));
        keywords.append(QStringLiteral("local"));
        keywords.append(QStringLiteral("message"));
        keywords.append(QStringLiteral("mod"));
        keywords.append(QStringLiteral("name"));
        keywords.append(QStringLiteral("near"));
        keywords.append(QStringLiteral("nil"));
        keywords.append(QStringLiteral("nodefault"));
        keywords.append(QStringLiteral("not"));
        keywords.append(QStringLiteral("object"));
        keywords.append(QStringLiteral("of"));
        keywords.append(QStringLiteral("or"));
        keywords.append(QStringLiteral("out"));
        keywords.append(QStringLiteral("overload"));
        keywords.append(QStringLiteral("override"));
        keywords.append(QStringLiteral("package"));
        keywords.append(QStringLiteral("packed"));
        keywords.append(QStringLiteral("pascal"));
        keywords.append(QStringLiteral("platform"));
        keywords.append(QStringLiteral("private"));
        keywords.append(QStringLiteral("procedure"));
        keywords.append(QStringLiteral("program"));
        keywords.append(QStringLiteral("property"));
        keywords.append(QStringLiteral("protected"));
        keywords.append(QStringLiteral("public"));
        keywords.append(QStringLiteral("published"));
        keywords.append(QStringLiteral("raise"));
        keywords.append(QStringLiteral("read"));
        keywords.append(QStringLiteral("readonly"));
        keywords.append(QStringLiteral("record"));
        keywords.append(QStringLiteral("register"));
        keywords.append(QStringLiteral("reintroduce"));
        keywords.append(QStringLiteral("repeat"));
        keywords.append(QStringLiteral("requires"));
        keywords.append(QStringLiteral("resident"));
        keywords.append(QStringLiteral("resourcestring"));
        keywords.append(QStringLiteral("safecall"));
        keywords.append(QStringLiteral("sealed"));
        keywords.append(QStringLiteral("set"));
        keywords.append(QStringLiteral("shl"));
        keywords.append(QStringLiteral("shr"));
        keywords.append(QStringLiteral("static"));
        keywords.append(QStringLiteral("stdcall"));
        keywords.append(QStringLiteral("stored"));
        keywords.append(QStringLiteral("string"));
        keywords.append(QStringLiteral("then"));
        keywords.append(QStringLiteral("threadvar"));
        keywords.append(QStringLiteral("to"));
        keywords.append(QStringLiteral("try"));
        keywords.append(QStringLiteral("type"));
        keywords.append(QStringLiteral("unit"));
        keywords.append(QStringLiteral("unsafe"));
        keywords.append(QStringLiteral("until"));
        keywords.append(QStringLiteral("uses"));
        keywords.append(QStringLiteral("var"));
        keywords.append(QStringLiteral("varargs"));
        keywords.append(QStringLiteral("virtual"));
        keywords.append(QStringLiteral("while"));
        keywords.append(QStringLiteral("with"));
        keywords.append(QStringLiteral("write"));
        keywords.append(QStringLiteral("writeonly"));
        keywords.append(QStringLiteral("xor"));
    }

    return keywords;
}

