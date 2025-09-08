/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "dwriter.h"

// app includes
#include "umlassociation.h"
#include "umlattribute.h"
#include "umlclassifier.h"
#include "codegen_utils.h"
#include "debug_utils.h"
#include "umloperation.h"
#include "umltemplate.h"
#include "umldoc.h"
#include "umlapp.h"  // Only needed for log{Warn,Error}
#include "umltemplatelist.h"

// qt includes
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

/**
 * Constructor, initialises a couple of variables.
 */
DWriter::DWriter()
  : isInterface(false)
{
    startline = m_endl + m_indentation;
}

/**
 * Destructor, empty.
 */
DWriter::~DWriter()
{
}

/**
 * Returns "D".
 * @return   the programming language identifier
 */
Uml::ProgrammingLanguage::Enum DWriter::language() const
{
    return Uml::ProgrammingLanguage::D;
}

// FIXME: doesn't work yet
void DWriter::writeModuleDecl(UMLClassifier *c, QTextStream &d)
{
    if (!c->package().isEmpty())
        d << "module " << c->package() << ";" << m_endl;

    writeBlankLine(d);
}

void DWriter::writeModuleImports(UMLClassifier *c, QTextStream &d)
{
    // another preparation, determine what we have
    UMLAssociationList associations = c->getSpecificAssocs(Uml::AssociationType::Association); // BAD! only way to get "general" associations.
    UMLAssociationList uniAssociations = c->getUniAssociationToBeImplemented();

    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();

    bool hasAssociations = aggregations.count() + associations.count() +
         compositions.count() + uniAssociations.count() > 0;

    if (hasAssociations) {
        // import tango, if that mode is set
        writeBlankLine(d);
    }

    //only import classes in a different package as this class
    UMLPackageList imports;
    findObjectsRelated(c, imports);
    for(UMLPackage* con : imports) {
        if (con->isUMLDatatype())
            continue;
        QString pkg = con->package();
        if (!pkg.isEmpty() && pkg != c->package())
            d << "import " << pkg << "." << cleanName(con->name()) << ";"
            << m_endl;
    }

    writeBlankLine(d);
}

/**
 * Call this method to generate d code for a UMLClassifier.
 * @param c   the class to generate code for
 */
void DWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        logWarn0("DWriter::writeClass: Cannot write class of NULL classifier");
        return;
    }

    isInterface = c->isInterface();

    QString fileName = cleanName(c->name().toLower());

    //find an appropriate name for our file
    fileName = findFileName(c, QStringLiteral(".d"));
    if (fileName.isEmpty()) {
        Q_EMIT codeGenerated(c, false);
        return;
    }

    // check that we may open that file for writing
    QFile file;
    if (!openFile(file, fileName)) {
        Q_EMIT codeGenerated(c, false);
        return;
    }

    // open text stream to file
    QTextStream d(&file);

    //try to find a heading file (license, comments, etc)
    QString str;
    str = getHeadingFile(QStringLiteral(".d"));
    if (!str.isEmpty()) {
        str.replace(QRegularExpression(QStringLiteral("%filename%")), fileName);
        str.replace(QRegularExpression(QStringLiteral("%filepath%")), file.fileName());
        d<<str<<m_endl;
    }

    // source file begins with the module declaration
    writeModuleDecl(c, d);

    // imports
    writeModuleImports(c, d);

    // write the opening declaration for the class incl any documentation,
    // interfaces and/or inheritance issues we have
    writeClassDecl(c, d);

    // start body of class
    d << " {" << m_endl;

    // Preparations
    //

    // sort attributes by Scope
    UMLAttributeList  atl;
    UMLAttributeList  atpub, atprot, atpriv, atpkg, atexport;
    UMLAttributeList  final_atpub, final_atprot, final_atpriv, final_atpkg, final_atexport;

    if (!isInterface) {
        UMLAttributeList atl = c->getAttributeList();
        for(UMLAttribute* at : atl) {
            switch(at->visibility())
            {
                case Uml::Visibility::Public:
                    if (at->isStatic())
                        final_atpub.append(at);
                    else
                        atpub.append(at);
                    break;
                case Uml::Visibility::Protected:
                    if (at->isStatic())
                        final_atprot.append(at);
                    else
                        atprot.append(at);
                    break;
                case Uml::Visibility::Private:
                    if (at->isStatic())
                        final_atpriv.append(at);
                    else
                        atpriv.append(at);
                    break;/* TODO: requires support from the gui & other structures
                case Uml::Visibility::Package:
                    if (at->getStatic())
                    final_atpkg.append(at);
                    else
                    atpkg.append(at);
                    break;
                case Uml::Visibility::Export:
                    if (at->getStatic())
                    final_atexport.append(at);
                    else
                    atexport.append(at);
                    break;*/
                default:
                    break;
            }
        }
    }

    // another preparation, determine what we have
    UMLAssociationList associations = c->getSpecificAssocs(Uml::AssociationType::Association); // BAD! only way to get "general" associations.
    UMLAssociationList uniAssociations = c->getUniAssociationToBeImplemented();

    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();

    bool hasAssociations = aggregations.count() + associations.count() + compositions.count() + uniAssociations.count() > 0;
    bool hasAttributes = atl.count() > 0;
    bool hasAccessorMethods = hasAttributes || hasAssociations;
    bool hasOperationMethods = c->getOpList().count() > 0;

    // ATTRIBUTES
    //

    // write comment for section IF needed
    if (forceDoc() || hasAccessorMethods)
    {
        writeComment(QString(), m_indentation, d);
        writeComment(QStringLiteral("Fields"), m_indentation, d);
        writeComment(QString(), m_indentation, d);
        writeBlankLine(d);
    }

    writeAttributeDecls(final_atpub, final_atprot, final_atpriv, d);
    writeAttributeDecls(atpub, atprot, atpriv, d);

    writeAssociationDecls(associations, c->id(), d);
    writeAssociationDecls(uniAssociations, c->id(), d);
    writeAssociationDecls(aggregations, c->id(), d);
    writeAssociationDecls(compositions, c->id(), d);

    //FIXME: find constructors and write them here

    // write constructors
    if (!isInterface) writeConstructor(c, d);

    // METHODS
    //

    // write comment for sub-section IF needed
    if (forceDoc() || hasAccessorMethods) {
        writeComment(QString(), m_indentation, d);
        writeComment(QStringLiteral("Accessors"), m_indentation, d);
        writeComment(QString(), m_indentation, d);
        writeBlankLine(d);
    }

    // Accessors for attributes
    writeAttributeMethods(final_atpub, Uml::Visibility::Public, d);
    writeAttributeMethods(final_atprot, Uml::Visibility::Protected, d);
    writeAttributeMethods(final_atpriv, Uml::Visibility::Private, d);
    writeAttributeMethods(atpub, Uml::Visibility::Public, d);
    writeAttributeMethods(atprot, Uml::Visibility::Protected, d);
    writeAttributeMethods(atpriv, Uml::Visibility::Private, d);

    // accessor methods for associations

    // first: determine the name of the other class
    writeAssociationMethods(associations, c, d);
    writeAssociationMethods(uniAssociations, c, d);
    writeAssociationMethods(aggregations, c, d);
    writeAssociationMethods(compositions, c, d);

    // Other operation methods
    // all other operations are now written

    // write comment for sub-section IF needed
    if (forceDoc() || hasOperationMethods) {
        writeComment(QString(), m_indentation, d);
        writeComment(QStringLiteral("Other methods"), m_indentation, d);
        writeComment(QString(), m_indentation, d);
        writeBlankLine(d);
    }

    writeOperations(c, d);

    d << "}" << m_endl; // end class

    file.close();
    Q_EMIT codeGenerated(c, true);
}

void DWriter::writeClassDecl(UMLClassifier *c, QTextStream &d)
{
    // class documentation
    if (!c->doc().isEmpty()) {
        writeDocumentation(QString(), c->doc(), QString(), QString(), d);
    }

    /*
     * Class declaration
     *
     * (private) class foo(T, ..., Z) : class1, ..., classN, interface1, ..., interfaceN
     *     a       b    c      d      e          f                        g
     */

    // (a) visibility modifier
    switch(c->visibility()) {
        case Uml::Visibility::Private: d << "private "; break;
        default: break;
    }

    // (b) keyword
    // TODO what about structs?
    if (isInterface) {
        d << "interface ";
    } else {
        if (c->isAbstract()) {
            d << "abstract ";
        }

        d << "class ";
    }

    // (c) class name
    QString classname = cleanName(c->name()); // our class name
    d << classname;

    // (d) template parameters
    UMLTemplateList template_params = c->getTemplateList();
    if (template_params.count()) {
        d << "(";

        for (UMLTemplateListIt tlit(template_params); tlit.hasNext();) {
            UMLTemplate* t = tlit.next();
            // TODO: hm, leaving the type blank results in "class"
            // so we omit it (also because "class" in this context is illegal)
            if (t->getTypeName() != QStringLiteral("class")) {
                d << t->getTypeName();
                d << " ";
            }

            d << t->name();

            if (tlit.hasNext()) {
                tlit.next();
                d << ", ";
            }
        }

        d << ")";
    }

    // (e) inheritances
    UMLClassifierList superclasses =
        c->findSuperClassConcepts(UMLClassifier::CLASS);
    UMLClassifierList superinterfaces =
        c->findSuperClassConcepts(UMLClassifier::INTERFACE);

    int count = superclasses.count() + superinterfaces.count();

    if (count > 0) {
        d << " : ";

        // (f) base classes
        for(UMLClassifier* classifier : superclasses) {
            d << cleanName(classifier->name());

            count--;

            if (count>0) d << ", ";
        }

        // (g) interfaces
        for(UMLClassifier* classifier : superinterfaces) {
            d << cleanName(classifier->name());

            count--;

            if (count>0) d << ", ";
        }
    }
}

void DWriter::writeProtectionMod(Uml::Visibility::Enum visibility, QTextStream &d)
{
    d << m_indentation << Uml::Visibility::toString(visibility) << ":" << m_endl << m_endl;
}

void DWriter::writeAttributeDecl(Uml::Visibility::Enum visibility, UMLAttributeList &atlist, QTextStream &d)
{
    if (atlist.count()==0) return;

    writeProtectionMod(visibility, d);

    for(UMLAttribute* at : atlist) {
        // documentation
        if (!at->doc().isEmpty()) {
            writeComment(at->doc(), m_indentation, d, true);
        }

        d << m_indentation;

        // static attribute?
        if (at->isStatic()) d << "static ";

        // method return type
        d << fixTypeName(at->getTypeName()) << " ";

        // TODO: find out whether this class has accessors or not
        bool hasAccessorMethods = true;

        // attribute name
        if (hasAccessorMethods) {
            d << "m_";
        }
        d << cleanName(at->name());

        // initial value
        QString initVal = fixInitialStringDeclValue(at->getInitialValue(), at->getTypeName());
        if (!initVal.isEmpty()) d << " = " << initVal;
        d << ";" << m_endl << m_endl;
    }
}

void DWriter::writeAttributeDecls(UMLAttributeList &atpub, UMLAttributeList &atprot,
                                     UMLAttributeList &atpriv, QTextStream &d)
{
    writeAttributeDecl(Uml::Visibility::Public, atpub, d);
    writeAttributeDecl(Uml::Visibility::Protected, atprot, d);
    writeAttributeDecl(Uml::Visibility::Private, atpriv, d);
    //TODO: export and package
}

void DWriter::writeAttributeMethods(UMLAttributeList &atpub, Uml::Visibility::Enum visibility, QTextStream &d)
{
    if (atpub.count()==0) return;

    writeProtectionMod(visibility, d);

    for(UMLAttribute* at : atpub) {
        QString fieldName = cleanName(at->name());
        writeSingleAttributeAccessorMethods(
            at->getTypeName(), QStringLiteral("m_") + fieldName, fieldName, at->doc(),
            visibility, Uml::Changeability::Changeable, at->isStatic(), d);
    }
}

void DWriter::writeComment(const QString &comment, const QString &myIndent,
                           QTextStream &d, bool dDocStyle)
{
    if (dDocStyle) {
        d << myIndent << "/**" << m_endl;
    }

    QStringList lines = comment.split(QStringLiteral("\n"));

    if (lines.count() == 0) lines << comment;

    for (int i = 0; i < lines.count(); ++i) {
        QString tmp = lines[i];

        while (tmp.count() > 77) {
            int l = tmp.left(77).lastIndexOf(QLatin1Char(' '));
            if (l < 1) l = tmp.indexOf(QLatin1Char(' '), 77);
            if (l < 1 || l > tmp.count()) {
                d << myIndent << (dDocStyle ? " * " : "// ") << tmp << m_endl;
                break;
            }
            d << myIndent << (dDocStyle ? " * " : "// ") << tmp.left(l) << m_endl;
            tmp = tmp.right(tmp.count() - l);
        }

        d << myIndent << (dDocStyle ? " * " : "// ") << tmp << m_endl;
    }

    if (dDocStyle) {
        d << myIndent << " */" << m_endl;
    }
}

void DWriter::writeDocumentation(QString header, QString body, QString end, QString indent, QTextStream &d)
{
    d << indent << "/**" << m_endl;
    if (!header.isEmpty())
        d << formatDoc(header, indent + QStringLiteral(" * "));
    if (!body.isEmpty())
        d << formatDoc(body, indent + QStringLiteral(" * "));
    if (!end.isEmpty())
    {
        QStringList lines = end.split(QStringLiteral("\n"));
        for (int i= 0; i < lines.count(); ++i) {
            d << formatDoc(lines[i], indent + QStringLiteral(" * "));
        }
    }
    d << indent << " */" << m_endl;
}

void DWriter::writeAssociationDecls(UMLAssociationList associations, Uml::ID::Type id, QTextStream &d)
{
    if (forceSections() || !associations.isEmpty())
    {
        bool printRoleA = false, printRoleB = false;
        for(UMLAssociation *a : associations) {
            // it may seem counter intuitive, but you want to insert the role of the
            // *other* class into *this* class.
            if (a->getObjectId(Uml::RoleType::A) == id)
                printRoleB = true;

            if (a->getObjectId(Uml::RoleType::B) == id)
                printRoleA = true;

            // First: we insert documentation for association IF it has either role AND some documentation (!)
            if ((printRoleA || printRoleB) && !(a->doc().isEmpty()))
                writeComment(a->doc(), m_indentation, d);

            // print RoleB decl
            if (printRoleB)
            {
                QString fieldClassName = cleanName(getUMLObjectName(a->getObject(Uml::RoleType::B)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::RoleType::B), a->getMultiplicity(Uml::RoleType::B), a->getRoleDoc(Uml::RoleType::B), a->visibility(Uml::RoleType::B), d);
            }

            // print RoleA decl
            if (printRoleA)
            {
                QString fieldClassName = cleanName(getUMLObjectName(a->getObject(Uml::RoleType::A)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::RoleType::A), a->getMultiplicity(Uml::RoleType::A), a->getRoleDoc(Uml::RoleType::A), a->visibility(Uml::RoleType::A), d);
            }
        }
    }
}

void DWriter::writeAssociationRoleDecl(QString fieldClassName,
        QString roleName, QString multi,
        QString doc, Uml::Visibility::Enum /*visib*/, QTextStream &d)
{
    // ONLY write out IF there is a rolename given
    // otherwise it is not meant to be declared in the code
    if (roleName.isEmpty()) return;

    if (!doc.isEmpty()) {
        writeComment(doc, m_indentation, d);
    }

    bool hasAccessors = true;

    // declare the association based on whether it is this a single variable
    // or a List (Vector). One day this will be done correctly with special
    // multiplicity object that we don't have to figure out what it means via regex.
    if (multi.isEmpty() || multi.contains(QRegularExpression(QStringLiteral("^[01]$")))) {
        d << m_indentation << fieldClassName << " ";

        if (hasAccessors) d << "m_";

        d << deCapitaliseFirstLetter(roleName) << ";";
    } else {
        d << m_indentation << fieldClassName << "[] ";
        //TODO: templated containers

        if (hasAccessors) d << "m_";

        d << pluralize(deCapitaliseFirstLetter(roleName)) << ";";
        // from here we could initialize default values, or put in an init() section
        // of the constructors
    }

    // always put space between this and following decl, if any
    writeBlankLine(d);
}

void DWriter::writeAssociationMethods (UMLAssociationList associations, UMLClassifier *thisClass, QTextStream &d)
{
    if (forceSections() || !associations.isEmpty()) {
        for(UMLAssociation *a : associations) {
            // insert the methods to access the role of the other
            // class in the code of this one
            if (a->getObjectId(Uml::RoleType::A) == thisClass->id()) {
                // only write out IF there is a rolename given
                if (!a->getRoleName(Uml::RoleType::B).isEmpty()) {
                    QString fieldClassName = getUMLObjectName(a->getObject(Uml::RoleType::B));
                    writeAssociationRoleMethod(fieldClassName,
                                               a->getRoleName(Uml::RoleType::B),
                                               a->getMultiplicity(Uml::RoleType::B), a->getRoleDoc(Uml::RoleType::B),
                                               a->visibility(Uml::RoleType::B),
                                               a->changeability(Uml::RoleType::B), d);
                }
            }

            if (a->getObjectId(Uml::RoleType::B) == thisClass->id()) {
                // only write out IF there is a rolename given
                if (!a->getRoleName(Uml::RoleType::A).isEmpty()) {
                    QString fieldClassName = getUMLObjectName(a->getObject(Uml::RoleType::A));
                    writeAssociationRoleMethod(fieldClassName, a->getRoleName(Uml::RoleType::A),
                                               a->getMultiplicity(Uml::RoleType::A),
                                               a->getRoleDoc(Uml::RoleType::A),
                                               a->visibility(Uml::RoleType::A),
                                               a->changeability(Uml::RoleType::A),
                                               d);
                }
            }
        }
    }
}

void DWriter::writeAssociationRoleMethod (QString fieldClassName, QString roleName, QString multi,
        QString description, Uml::Visibility::Enum visib, Uml::Changeability::Enum change,
        QTextStream &d)
{
    if (multi.isEmpty() || multi.contains(QRegularExpression(QStringLiteral("^[01]$")))) {
        QString fieldVarName = QStringLiteral("m_") + deCapitaliseFirstLetter(roleName);

        writeSingleAttributeAccessorMethods(
            fieldClassName, fieldVarName, roleName, description, visib, change, false, d);
    } else {
        QString fieldVarName = QStringLiteral("m_") + pluralize(deCapitaliseFirstLetter(roleName));

        writeVectorAttributeAccessorMethods(
            fieldClassName, fieldVarName, pluralize(roleName), description, visib, change, d);
    }
}

void DWriter::writeVectorAttributeAccessorMethods (QString fieldClassName, QString fieldVarName,
        QString fieldName, QString description,
        Uml::Visibility::Enum visibility, Uml::Changeability::Enum changeType,
        QTextStream &d)
{
    Q_UNUSED(visibility);
    fieldClassName = fixTypeName(fieldClassName);
    QString fieldNameUP = unPluralize(fieldName);
    QString fieldNameUC = Codegen_Utils::capitalizeFirstLetter(fieldNameUP);

    // ONLY IF changeability is NOT Frozen
    if (changeType != Uml::Changeability::Frozen) {
        writeDocumentation(QStringLiteral("Adds a ") + fieldNameUP + QStringLiteral(" to the list of ") +
                           fieldName + QLatin1Char('.'), description, QString(), m_indentation, d);

        d << m_indentation << "void add" << fieldNameUC << "(";
        d << fieldClassName << " new" << fieldNameUC << ") {";
        d << startline << m_indentation << fieldVarName << " ~= new" << fieldNameUC << ";";
        d << startline << "}" << m_endl << m_endl;
    }

    // ONLY IF changeability is Changeable
    if (changeType == Uml::Changeability::Changeable) {
        writeDocumentation(QStringLiteral("Removes a ") + fieldNameUP + QStringLiteral(" from the list of ") +
                           fieldName + QLatin1Char('.'), description, QString(), m_indentation, d);

        d << m_indentation << "void remove" << fieldNameUC << "(";
        d << fieldClassName << " " << fieldNameUP << ") {" << startline;
        d << m_indentation << "int idx = " << fieldVarName << ".length;" << startline;
        d << m_indentation << "foreach(i, o; " << fieldVarName << ")" << startline;
        d << m_indentation << m_indentation << "if (o && o == " << fieldNameUP << ") {" << startline;
        d << m_indentation << m_indentation << m_indentation << "idx = i;" << startline;
        d << m_indentation << m_indentation << m_indentation << "break;" << startline;
        d << m_indentation << m_indentation << "}" << m_endl << startline;
        d << m_indentation << fieldVarName << " = " << fieldVarName;
        d << "[0..idx] ~ " << fieldVarName << "[idx..$];" << startline;
        d << "}" << m_endl << m_endl;
    }

    // always allow getting the list of stuff
    writeDocumentation(QStringLiteral("Returns the list of ") + fieldName + QLatin1Char('.'),
                       description, QStringLiteral("@return List of ") + fieldName + QLatin1Char('.'),
                       m_indentation, d);

    d << m_indentation << fieldClassName << "[] get" << fieldName << "() {";
    d << startline << m_indentation << "return " << fieldVarName << ";";
    d << startline << "}" << m_endl << m_endl;
}


void DWriter::writeSingleAttributeAccessorMethods(QString fieldClassName,
     QString fieldVarName, QString fieldName, QString description, Uml::Visibility::Enum /*visibility*/,
     Uml::Changeability::Enum change, bool isFinal, QTextStream &d) {

    fieldClassName = fixTypeName(fieldClassName);
    QString fieldNameUC = Codegen_Utils::capitalizeFirstLetter(fieldName);
    if (fieldName.left(2) == QStringLiteral("m_")) fieldName = fieldName.right(fieldName.count()-2);

    // set method
    if (change == Uml::Changeability::Changeable && !isFinal) {
        writeDocumentation(QStringLiteral("Sets the value of ") + fieldName + QLatin1Char('.'), description,
                           QStringLiteral("@param new") + fieldNameUC + QStringLiteral(" The new value of ") + fieldName + QLatin1Char('.'),
                           m_indentation, d);

        d << m_indentation << fieldClassName << " " << fieldName << "(";
        d << fieldClassName << " new" << fieldNameUC << ") {";
        d << startline << m_indentation << "return " << fieldVarName << " = new" << fieldNameUC << ";";
        d << startline << "}" << m_endl << m_endl;
    }

    // get method
    writeDocumentation(QStringLiteral("Returns the value of ") + fieldName + QLatin1Char('.'), description,
                       QStringLiteral("@return The value of ") + fieldName + QLatin1Char('.'),
                       m_indentation, d);

    d << m_indentation << fieldClassName << " " << fieldName << "() {";
    d << startline << m_indentation << "return " << fieldVarName << ";";
    d << startline << "}" << m_endl << m_endl;
}

void DWriter::writeConstructor(UMLClassifier *c, QTextStream &d)
{
    if (forceDoc())
    {
        d<<startline;
        writeComment(QString(), m_indentation, d);
        writeComment(QStringLiteral("Constructors"), m_indentation, d);
        writeComment(QString(), m_indentation, d);
        writeBlankLine(d);
    }

    // write the first constructor
    QString className = cleanName(c->name());
    d << m_indentation << "public this("<<") { }";
}

// IF the type is "string" we need to declare it as
// the D Object "String" (there is no string primitive in D).
// Same thing again for "bool" to "boolean"
QString DWriter::fixTypeName(const QString& string)
{
    if (string.isEmpty())
        return QStringLiteral("void");
    if (string == QStringLiteral("string"))
        return QStringLiteral("char[]");
    if (string == QStringLiteral("unsigned short"))
        return QStringLiteral("ushort");
    if (string == QStringLiteral("unsigned int"))
        return QStringLiteral("uint");
    if (string == QStringLiteral("unsigned long"))
        return QStringLiteral("ulong");
    return string;
}

/**
 * Return the default datatypes.
 * (Overrides method from class CodeGenerator.)
 * @return   list of default datatypes
 */
QStringList DWriter::defaultDatatypes() const
{
    QStringList l;
    l << QStringLiteral("void")
      << QStringLiteral("bool")
      << QStringLiteral("byte")
      << QStringLiteral("ubyte")
      << QStringLiteral("short")
      << QStringLiteral("ushort")
      << QStringLiteral("int")
      << QStringLiteral("uint")
      << QStringLiteral("long")
      << QStringLiteral("ulong")
      << QStringLiteral("cent")
      << QStringLiteral("ucent")
      << QStringLiteral("float")
      << QStringLiteral("double")
      << QStringLiteral("real")
      << QStringLiteral("ifloat")
      << QStringLiteral("idouble")
      << QStringLiteral("ireal")
      << QStringLiteral("cfloat")
      << QStringLiteral("cdouble")
      << QStringLiteral("creal")
      << QStringLiteral("char")
      << QStringLiteral("wchar")
      << QStringLiteral("dchar")
      << QStringLiteral("string");
    return l;
}


bool DWriter::compareDMethod(UMLOperation *op1, UMLOperation *op2)
{
    if (op1 == nullptr || op2 == nullptr)
        return false;
    if (op1 == op2)
        return true;
    if (op1->name() != op2->name())
        return false;
    UMLAttributeList atl1 = op1->getParmList();
    UMLAttributeList atl2 = op2->getParmList();
    if (atl1.count() != atl2.count())
        return false;

    for (UMLAttributeListIt atl1It(atl1), atl2It(atl2); atl1It.hasNext() && atl2It.hasNext();) {
        UMLAttribute* at1 = atl1It.next(), *at2 = atl2It.next();
        if (at1->getTypeName() != at2->getTypeName())
            return false;
    }
    return true;

}

bool DWriter::dMethodInList(UMLOperation *umlOp, UMLOperationList &opl)
{
    for(UMLOperation* op : opl) {
        if (DWriter::compareDMethod(op, umlOp)) {
            return true;
        }
    }
    return false;
}

void DWriter::getSuperImplementedOperations(UMLClassifier *c, UMLOperationList &yetImplementedOpList, UMLOperationList &toBeImplementedOpList, bool noClassInPath)
{
    UMLClassifierList superClasses = c->findSuperClassConcepts();

    for(UMLClassifier* classifier : superClasses) {
        getSuperImplementedOperations(classifier, yetImplementedOpList, toBeImplementedOpList, (classifier->isInterface() && noClassInPath));
        UMLOperationList opl = classifier->getOpList();
        for(UMLOperation* op : opl) {
            if (classifier->isInterface() && noClassInPath) {
                if (!DWriter::dMethodInList(op, toBeImplementedOpList))
                    toBeImplementedOpList.append(op);
            }
            else
            {
                if (!DWriter::dMethodInList(op, yetImplementedOpList))
                    yetImplementedOpList.append(op);
            }
        }
    }
}

void DWriter::getInterfacesOperationsToBeImplemented(UMLClassifier *c, UMLOperationList &opList)
{
    UMLOperationList yetImplementedOpList;
    UMLOperationList toBeImplementedOpList;

    getSuperImplementedOperations(c, yetImplementedOpList, toBeImplementedOpList);
    for(UMLOperation* op : toBeImplementedOpList) {
        if (! DWriter::dMethodInList(op, yetImplementedOpList) && ! DWriter::dMethodInList(op, opList))
            opList.append(op);
    }
}

void DWriter::writeOperations(UMLClassifier *c, QTextStream &d)
{
    UMLOperationList opl;
    UMLOperationList oppub, opprot, oppriv;

    //sort operations by scope first and see if there are abstract methods
    opl = c->getOpList();
    if (! c->isInterface()) {
        getInterfacesOperationsToBeImplemented(c, opl);
    }
    for(UMLOperation* op : opl) {
        switch(op->visibility()) {
          case Uml::Visibility::Public:
            oppub.append(op);
            break;
          case Uml::Visibility::Protected:
            opprot.append(op);
            break;
          case Uml::Visibility::Private:
            oppriv.append(op);
            break;
          default: //TODO: package, export
            break;
        }
    }

    // do people REALLY want these comments? Hmm.
    /*
      if (forceSections() || oppub.count())
      {
      writeComment("public operations", m_indentation, d);
        writeBlankLine(d);
      }
    */

    if (oppub.count() > 0) {
        writeProtectionMod(Uml::Visibility::Public, d);

        writeOperations(oppub, d);
    }

    if (opprot.count() > 0) {
        writeProtectionMod(Uml::Visibility::Protected, d);

        writeOperations(opprot, d);
    }

    if (oppriv.count() > 0) {
        writeProtectionMod(Uml::Visibility::Private, d);

        writeOperations(oppriv, d);
    }

}

void DWriter::writeOperations(UMLOperationList &oplist, QTextStream &d)
{
    UMLAttributeList atl;
    QString str;

    // generate method decl for each operation given
    for(UMLOperation* op : oplist) {
        QString doc;
        // write documentation

        QString methodReturnType = fixTypeName(op->getTypeName());

        //TODO: return type comment
        if (methodReturnType != QStringLiteral("void")) {
            doc += QStringLiteral("@return ") + methodReturnType + m_endl;
        }

        str = QString(); // reset for next method
        if (op->isAbstract() && !isInterface) str += QStringLiteral("abstract ");
        if (op->isStatic()) str += QStringLiteral("static ");

        str += methodReturnType + QLatin1Char(' ') + cleanName(op->name()) + QLatin1Char('(');

        atl = op->getParmList();
        int i = atl.count();
        int j = 0;
        for (UMLAttributeListIt atlIt(atl); atlIt.hasNext();  ++j) {
            UMLAttribute* at = atlIt.next();
            QString typeName = fixTypeName(at->getTypeName());
            QString atName = cleanName(at->name());
            str += typeName + QLatin1Char(' ') + atName +
                   (!(at->getInitialValue().isEmpty()) ?
                    (QStringLiteral(" = ") + at->getInitialValue()) : QString())
                   + ((j < i-1) ? QStringLiteral(", ") : QString());
            doc += QStringLiteral("@param ") + atName + QLatin1Char(' ') + at->doc() + m_endl;
        }
        doc = doc.remove(doc.size() - 1, 1);  // remove last '\n' of comment
        str += QLatin1Char(')');

        // method only gets a body IF it is not abstract
        if (op->isAbstract() || isInterface)
            str += QLatin1Char(';'); // terminate now
        else {
            str += startline + QLatin1Char('{') + m_endl;
            QString sourceCode = op->getSourceCode();
            if (sourceCode.isEmpty()) {
                // empty method body - TODO: throw exception
            }
            else {
                str += formatSourceCode(sourceCode, m_indentation + m_indentation);
            }
            str += m_indentation + QLatin1Char('}');
        }

        // write it out
        writeDocumentation(QString(), op->doc(), doc, m_indentation, d);
        d << m_indentation << str << m_endl << m_endl;
    }
}

QString DWriter::fixInitialStringDeclValue(const QString& val, const QString& type)
{
    QString value = val;
    // check for strings only
    if (!value.isEmpty() && type == QStringLiteral("String")) {
        if (!value.startsWith(QLatin1Char('"')))
            value.prepend(QLatin1Char('"'));
        if (!value.endsWith(QLatin1Char('"')))
            value.append(QLatin1Char('"'));
    }
    return value;
}

// methods like this _shouldn't_ be needed IF we properly did things thruought the code.
QString DWriter::getUMLObjectName(UMLObject *obj)
{
    return (obj ? obj->name() : QStringLiteral("NULL"));
}

QString DWriter::deCapitaliseFirstLetter(const QString& str)
{
    QString string = str;
    string.replace(0, 1, string[0].toLower());
    return string;
}

QString DWriter::pluralize(const QString& string)
{
    return string + (string.right(1) == QStringLiteral("s") ? QStringLiteral("es") : QStringLiteral("s"));
}

QString DWriter::unPluralize(const QString& string)
{
    // does not handle special cases liek datum -> data, etc.

    if (string.count() > 2 && string.right(3) == QStringLiteral("ses")) {
        return string.left(string.count() - 2);
    }

    if (string.right(1) == QStringLiteral("s")) {
        return string.left(string.count() - 1);
    }

    return string;
}

void DWriter::writeBlankLine(QTextStream &d)
{
    d << m_endl;
}

