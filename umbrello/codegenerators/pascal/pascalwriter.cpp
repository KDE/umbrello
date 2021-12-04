/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
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
#include <QRegExp>
#include <QTextStream>

const QString PascalWriter::defaultPackageSuffix = QLatin1String("_Holder");

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
        uDebug() << "unknown object type " << UMLObject::toString(ot);
        return false;
    }
    QString stype = c->stereotype();
    if (stype == QLatin1String("CORBAConstant") || stype == QLatin1String("CORBATypedef") ||
            stype == QLatin1String("CORBAStruct") || stype == QLatin1String("CORBAUnion"))
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
        retval = umlPkg->fullyQualifiedName(QLatin1String("."));
        if (c && isOOClass(c)) {
            retval.append(QLatin1String("."));
            retval.append(className);
        }
    }
    if (! withType)
        return retval;
    if (c && isOOClass(c)) {
        retval.append(QLatin1String(".Object"));
        if (! byValue)
            retval.append(QLatin1String("_Ptr"));
    } else {
        retval.append(QLatin1String("."));
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
            roleName = QLatin1String("M_");
            roleName.append(typeName);
        } else {
            roleName = typeName;
            roleName.append(QLatin1String("_Vector"));
        }
    }
    const UMLClassifier* c = a->getObject(Uml::RoleType::A)->asUMLClassifier();
    if (c == 0)
        return;
    typeName = cleanName(c->name());
    if (! a->getMultiplicity(Uml::RoleType::A).isEmpty())
        typeName.append(QLatin1String("_Array_Access"));
}

/**
 * Call this method to generate Pascal code for a UMLClassifier.
 * @param c   the class to generate code for
 */
void PascalWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        uDebug() << "Cannot write class of NULL concept!";
        return;
    }

    const bool isClass = !c->isInterface();
    QString classname = cleanName(c->name());
    QString fileName = qualifiedName(c).toLower();
    fileName.replace(QLatin1Char('.'), QLatin1Char('-'));

    //find an appropriate name for our file
    fileName = overwritableName(c, fileName, QLatin1String(".pas"));
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
    str = getHeadingFile(QLatin1String(".pas"));
    if (!str.isEmpty()) {
        str.replace(QRegExp(QLatin1String("%filename%")), fileName);
        str.replace(QRegExp(QLatin1String("%filepath%")), file.fileName());
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
        foreach (UMLPackage* con, imports) {
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
        foreach (UMLClassifierListItem *lit, litList) {
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
        if (stype == QLatin1String("CORBAConstant")) {
            pas << indent() << "// " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if(stype == QLatin1String("CORBAStruct")) {
            if (isClass) {

                pas << indent() << classname << " = record" << m_endl;
                m_indentLevel++;
                foreach (UMLAttribute* at, atl) {
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
        } else if(stype == QLatin1String("CORBAUnion")) {
            pas << indent() << "// " << stype << " is Not Yet Implemented" << m_endl << m_endl;
        } else if(stype == QLatin1String("CORBATypedef")) {
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
        pas << formatDoc(c->doc(), QLatin1String("// "));
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

        foreach (UMLAttribute* at, atpub) {
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

    foreach (UMLOperation* op, opl) {
         if (op->visibility() == Uml::Visibility::Public)
            oppub.append(op);
    }
    if (forceSections() || oppub.count())
        pas << indent() << "// Public methods:" << m_endl << m_endl;
    foreach (UMLOperation* op, oppub)
        writeOperation(op, pas);

    UMLAttributeList atprot = c->getAttributeList(Uml::Visibility::Protected);
    if (atprot.count()) {
        pas << "protected" << m_endl << m_endl;

        foreach (UMLAttribute*  at, atprot) {
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

        foreach (UMLAttribute* at, atpriv) {
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
    emit codeGenerated(c, true);
    emit showGeneratedFile(file.fileName());
}

/**
 * Write one operation.
 * @param op the class for which we are generating code
 * @param pas the stream associated with the output file
 */
void PascalWriter::writeOperation(UMLOperation *op, QTextStream &pas, bool is_comment)
{
    if (op->isStatic()) {
        pas << "// TODO: generate status method " << op->name() << m_endl;
        return;
    }
    UMLAttributeList atl = op->getParmList();
    QString rettype = op->getTypeName();
    bool use_procedure = (rettype.isEmpty() || rettype == QLatin1String("void"));

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
        foreach (UMLAttribute *at, atl) {
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
    l.append(QLatin1String("AnsiString"));
    l.append(QLatin1String("Boolean"));
    l.append(QLatin1String("Byte"));
    l.append(QLatin1String("ByteBool"));
    l.append(QLatin1String("Cardinal"));
    l.append(QLatin1String("Character"));
    l.append(QLatin1String("Currency"));
    l.append(QLatin1String("Double"));
    l.append(QLatin1String("Extended"));
    l.append(QLatin1String("Int64"));
    l.append(QLatin1String("Integer"));
    l.append(QLatin1String("Longint"));
    l.append(QLatin1String("LongBool"));
    l.append(QLatin1String("Longword"));
    l.append(QLatin1String("QWord"));
    l.append(QLatin1String("Real"));
    l.append(QLatin1String("Shortint"));
    l.append(QLatin1String("ShortString"));
    l.append(QLatin1String("Single"));
    l.append(QLatin1String("Smallint"));
    l.append(QLatin1String("String"));
    l.append(QLatin1String("WideString"));
    l.append(QLatin1String("Word"));
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
        keywords.append(QLatin1String("absolute"));
        keywords.append(QLatin1String("abstract"));
        keywords.append(QLatin1String("and"));
        keywords.append(QLatin1String("array"));
        keywords.append(QLatin1String("as"));
        keywords.append(QLatin1String("asm"));
        keywords.append(QLatin1String("assembler"));
        keywords.append(QLatin1String("automated"));
        keywords.append(QLatin1String("begin"));
        keywords.append(QLatin1String("case"));
        keywords.append(QLatin1String("cdecl"));
        keywords.append(QLatin1String("class"));
        keywords.append(QLatin1String("const"));
        keywords.append(QLatin1String("constructor"));
        keywords.append(QLatin1String("contains"));
        keywords.append(QLatin1String("default"));
        keywords.append(QLatin1String("deprecated"));
        keywords.append(QLatin1String("destructor"));
        keywords.append(QLatin1String("dispid"));
        keywords.append(QLatin1String("dispinterface"));
        keywords.append(QLatin1String("div"));
        keywords.append(QLatin1String("do"));
        keywords.append(QLatin1String("downto"));
        keywords.append(QLatin1String("dynamic"));
        keywords.append(QLatin1String("else"));
        keywords.append(QLatin1String("end"));
        keywords.append(QLatin1String("except"));
        keywords.append(QLatin1String("export"));
        keywords.append(QLatin1String("exports"));
        keywords.append(QLatin1String("external"));
        keywords.append(QLatin1String("far"));
        keywords.append(QLatin1String("file"));
        keywords.append(QLatin1String("final"));
        keywords.append(QLatin1String("finalization"));
        keywords.append(QLatin1String("finally"));
        keywords.append(QLatin1String("for"));
        keywords.append(QLatin1String("forward"));
        keywords.append(QLatin1String("function"));
        keywords.append(QLatin1String("goto"));
        keywords.append(QLatin1String("if"));
        keywords.append(QLatin1String("implementation"));
        keywords.append(QLatin1String("implements"));
        keywords.append(QLatin1String("in"));
        keywords.append(QLatin1String("index"));
        keywords.append(QLatin1String("inherited"));
        keywords.append(QLatin1String("initialization"));
        keywords.append(QLatin1String("inline"));
        keywords.append(QLatin1String("inline"));
        keywords.append(QLatin1String("interface"));
        keywords.append(QLatin1String("is"));
        keywords.append(QLatin1String("label"));
        keywords.append(QLatin1String("library"));
        keywords.append(QLatin1String("library"));
        keywords.append(QLatin1String("local"));
        keywords.append(QLatin1String("message"));
        keywords.append(QLatin1String("mod"));
        keywords.append(QLatin1String("name"));
        keywords.append(QLatin1String("near"));
        keywords.append(QLatin1String("nil"));
        keywords.append(QLatin1String("nodefault"));
        keywords.append(QLatin1String("not"));
        keywords.append(QLatin1String("object"));
        keywords.append(QLatin1String("of"));
        keywords.append(QLatin1String("or"));
        keywords.append(QLatin1String("out"));
        keywords.append(QLatin1String("overload"));
        keywords.append(QLatin1String("override"));
        keywords.append(QLatin1String("package"));
        keywords.append(QLatin1String("packed"));
        keywords.append(QLatin1String("pascal"));
        keywords.append(QLatin1String("platform"));
        keywords.append(QLatin1String("private"));
        keywords.append(QLatin1String("procedure"));
        keywords.append(QLatin1String("program"));
        keywords.append(QLatin1String("property"));
        keywords.append(QLatin1String("protected"));
        keywords.append(QLatin1String("public"));
        keywords.append(QLatin1String("published"));
        keywords.append(QLatin1String("raise"));
        keywords.append(QLatin1String("read"));
        keywords.append(QLatin1String("readonly"));
        keywords.append(QLatin1String("record"));
        keywords.append(QLatin1String("register"));
        keywords.append(QLatin1String("reintroduce"));
        keywords.append(QLatin1String("repeat"));
        keywords.append(QLatin1String("requires"));
        keywords.append(QLatin1String("resident"));
        keywords.append(QLatin1String("resourcestring"));
        keywords.append(QLatin1String("safecall"));
        keywords.append(QLatin1String("sealed"));
        keywords.append(QLatin1String("set"));
        keywords.append(QLatin1String("shl"));
        keywords.append(QLatin1String("shr"));
        keywords.append(QLatin1String("static"));
        keywords.append(QLatin1String("stdcall"));
        keywords.append(QLatin1String("stored"));
        keywords.append(QLatin1String("string"));
        keywords.append(QLatin1String("then"));
        keywords.append(QLatin1String("threadvar"));
        keywords.append(QLatin1String("to"));
        keywords.append(QLatin1String("try"));
        keywords.append(QLatin1String("type"));
        keywords.append(QLatin1String("unit"));
        keywords.append(QLatin1String("unsafe"));
        keywords.append(QLatin1String("until"));
        keywords.append(QLatin1String("uses"));
        keywords.append(QLatin1String("var"));
        keywords.append(QLatin1String("varargs"));
        keywords.append(QLatin1String("virtual"));
        keywords.append(QLatin1String("while"));
        keywords.append(QLatin1String("with"));
        keywords.append(QLatin1String("write"));
        keywords.append(QLatin1String("writeonly"));
        keywords.append(QLatin1String("xor"));
    }

    return keywords;
}

