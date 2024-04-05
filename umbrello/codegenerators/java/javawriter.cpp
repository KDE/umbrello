/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <brian.thomas@gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "javawriter.h"

// app includes
#include "codegen_utils.h"
#include "classifier.h"
#include "debug_utils.h"
#include "operation.h"
#include "attribute.h"
#include "association.h"
#include "template.h"
#include "umldoc.h"
#include "uml.h"  // Only needed for log{Warn,Error}
#include "umltemplatelist.h"

// qt includes
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

/**
 * Constructor, initialises a couple of variables.
 */
JavaWriter::JavaWriter()
    : m_isInterface(false)
{
    m_startline = m_endl + m_indentation;
}

/**
 * Destructor, empty.
 */
JavaWriter::~JavaWriter()
{
}

/**
 * Returns "Java".
 * @return   the programming language identifier
 */
Uml::ProgrammingLanguage::Enum JavaWriter::language() const
{
    return Uml::ProgrammingLanguage::Java;
}

/**
 * Call this method to generate java code for a UMLClassifier.
 * @param c   the class to generate code for
 */
void JavaWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        logWarn0("JavaWriter::writeClass: Cannot write class of NULL classifier");
        return;
    }

    m_isInterface = c->isInterface();

    QString fileName = cleanName(c->name().toLower());

    //find an appropriate name for our file
    fileName = findFileName(c, QStringLiteral(".java"));
    if (fileName.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    // check that we may open that file for writing
    QFile file;
    if (!openFile(file, fileName)) {
        emit codeGenerated(c, false);
        return;
    }

    // Preparations
    //

    // sort attributes by Scope
    UMLAttributeList  atl;
    UMLAttributeList  atpub, atprot, atpriv;
    UMLAttributeList  final_atpub, final_atprot, final_atpriv;

    if (!m_isInterface) {
        UMLAttributeList atl = c->getAttributeList();
        Q_FOREACH(UMLAttribute *at,  atl) {
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
                break;
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

    bool hasAssociations = aggregations.count() > 0 || associations.count() > 0 || compositions.count() > 0 || uniAssociations.count() > 0;
    bool hasAttributes = (atl.count() > 0);
    bool hasAccessorMethods = hasAttributes || hasAssociations;
    bool hasOperationMethods = (c->getOpList().count() > 0);
    // this is a bit too simplistic..some associations are for
    // SINGLE objects, and WONT be declared as Vectors, so this
    // is a bit overly inclusive
    bool hasVectorFields = hasAssociations ? true : false;

    // open text stream to file
    QTextStream java(&file);

    //try to find a heading file (license, comments, etc)
    QString str;
    str = getHeadingFile(QStringLiteral(".java"));
    if (!str.isEmpty()) {
        str.replace(QRegularExpression(QStringLiteral("%filename%")), fileName);
        str.replace(QRegularExpression(QStringLiteral("%filepath%")), file.fileName());
        java << str << m_endl;
    }

    if (!c->package().isEmpty())
        java << "package " << c->package() << ";" << m_endl;

    // IMPORT statements
    // Q: Why all utils? Aren't just List and Vector the only classes we are using?
    // A: doesn't matter at all; it is more readable to just include '*' and java compilers
    //    don't slow down or anything. (TZ)
    if (hasVectorFields)
    {
        writeBlankLine(java);
        java << "import java.util.*;" << m_endl;
    }

    //only import classes in a different package as this class
    UMLPackageList imports;
    findObjectsRelated(c, imports);
    Q_FOREACH(UMLPackage* con,  imports) {
        if (con->isUMLDatatype())
            continue;
        QString pkg = con->package();
        if (!pkg.isEmpty() && pkg != c->package())
            java << "import " << pkg << "." << cleanName(con->name()) << ";"
                 << m_endl;
    }
    writeBlankLine(java);

    // write the opening declaration for the class incl any documentation,
    // interfaces and/or inheritance issues we have
    writeClassDecl(c, java);

    // start body of class
    java << " {" << m_endl;

    // ATTRIBUTES
    //

    // write comment for section IF needed
    if (forceDoc() || hasAccessorMethods)
    {
        writeComment(QString(), m_indentation, java);
        writeComment(QStringLiteral("Fields"), m_indentation, java);
        writeComment(QString(), m_indentation, java);
        writeBlankLine(java);
    }

    writeAttributeDecls(final_atpub, final_atprot, final_atpriv, java);
    writeAttributeDecls(atpub, atprot, atpriv, java);

    writeAssociationDecls(associations, c->id(), java);
    writeAssociationDecls(uniAssociations, c->id(), java);
    writeAssociationDecls(aggregations, c->id(), java);
    writeAssociationDecls(compositions, c->id(), java);

    // Constructors: anything we more we need to do here ?
    //
    if (!m_isInterface)
        writeConstructor(c, java);

    // METHODS
    //

    // write comment for section IF needed
    if (forceDoc() || hasAccessorMethods || hasOperationMethods)
    {
        java << m_startline;
        writeComment(QString(), m_indentation, java);
        writeComment(QStringLiteral("Methods"), m_indentation, java);
        writeComment(QString(), m_indentation, java);
        writeBlankLine(java);
        writeBlankLine(java);
    }

    // write comment for sub-section IF needed
    if (forceDoc() || hasAccessorMethods)
    {
        writeComment(QString(), m_indentation, java);
        writeComment(QStringLiteral("Accessor methods"), m_indentation, java);
        writeComment(QString(), m_indentation, java);
        writeBlankLine(java);
    }

    // Accessors for attributes
    writeAttributeMethods(final_atpub, Uml::Visibility::Public, java);
    writeAttributeMethods(final_atprot, Uml::Visibility::Public, java);
    writeAttributeMethods(final_atpriv, Uml::Visibility::Public, java);
    writeAttributeMethods(atpub, Uml::Visibility::Public, java);
    writeAttributeMethods(atprot, Uml::Visibility::Public, java);
    writeAttributeMethods(atpriv, Uml::Visibility::Public, java);

    // accessor methods for associations

    // first: determine the name of the other class
    writeAssociationMethods(associations, c, java);
    writeAssociationMethods(uniAssociations, c, java);
    writeAssociationMethods(aggregations, c, java);
    writeAssociationMethods(compositions, c, java);

    // Other operation methods
    // all other operations are now written

    // write comment for sub-section IF needed
    if (forceDoc() || hasOperationMethods)
    {
        writeComment(QString(), m_indentation, java);
        writeComment(QStringLiteral("Other methods"), m_indentation, java);
        writeComment(QString(), m_indentation, java);
        writeBlankLine(java);
    }
    writeOperations(c, java);

    writeBlankLine(java);
    java << "}" << m_endl; // end class

    file.close();
    emit codeGenerated(c, true);
}

/**
 * Writes class's documentation then the class header
 * "public abstract class Foo extents {".
 */
void JavaWriter::writeClassDecl(UMLClassifier *c, QTextStream &java)
{
    QString classname = cleanName(c->name()); // our class name

    // write documentation for class, if any, first
    if (forceDoc() || !c->doc().isEmpty())
    {
        if (m_isInterface)
            writeDocumentation(QStringLiteral("Interface ") + classname, c->doc(), QString(), QString(), java);
        else
            writeDocumentation(QStringLiteral("Class ") + classname, c->doc(), QString(), QString(), java);

        writeBlankLine(java);
    }

    // Now write the actual class declaration
    QString scope; // = c->getVisibility().toString();
    if (c->visibility() != Uml::Visibility::Public) {
        // We should emit a warning in here .. java doesn't like to allow
        // private/protected classes. The best we can do (I believe)
        // is to let these declarations default to "package visibility"
        // which is a level between traditional "private" and "protected"
        // scopes. To get this visibility level we just print nothing..
    } else
        scope = QStringLiteral("public ");

    java << ((c->isAbstract() && !m_isInterface) ? QStringLiteral("abstract ") : QString()) << scope;
    if (m_isInterface)
        java << "interface ";
    else
        java << "class ";

    java << classname;

    // Generics
    UMLTemplateList template_params = c->getTemplateList();
    if (template_params.count()) {
        java << "<";
        for (UMLTemplateListIt tlit(template_params); tlit.hasNext();) {
            UMLTemplate* t = tlit.next();
            QString formalName = t->name();
            java << formalName;
            QString typeName = t->getTypeName();
            if (typeName != QStringLiteral("class")) {
                java << " extends " << typeName;
            }
            if (tlit.hasNext()) {
                tlit.next();
                java << ", ";
            }
        }
        java << ">" << m_endl;
    }

    // write inheritances out
    UMLClassifierList superclasses = c->findSuperClassConcepts(UMLClassifier::CLASS);

    int i = 0;
    Q_FOREACH(UMLClassifier *classifier, superclasses) {
        if (i == 0)
        {
            java <<  " extends ";
        }
        else
        {
            //The java generated code is wrong ! : No multiple inheritance of class
            java <<  ", " ;
        }
        java <<  cleanName(classifier->name());
        i++;
    }

    UMLClassifierList superInterfaces = c->findSuperClassConcepts(UMLClassifier::INTERFACE);
    i = 0;
    Q_FOREACH(UMLClassifier *classifier, superInterfaces) {
        if (i == 0)
        {
            if (m_isInterface)
                java <<  " extends ";
            else
                java <<  " implements ";
        }
        else
        {
            //The java generated code is OK ! : multiple inheritance of interface
            java <<  ", " ;
        }
        java <<  cleanName(classifier->name());
        i++;
    }

}

/**
 * Writes the Attribute declarations.
 * @param atpub    list of public attributes
 * @param atprot   list of protected attributes
 * @param atpriv   list of private attributes
 * @param java     text stream
 */
void JavaWriter::writeAttributeDecls(UMLAttributeList &atpub, UMLAttributeList &atprot,
                                     UMLAttributeList &atpriv, QTextStream &java)
{
    Q_FOREACH(UMLAttribute *at, atpub) {
        QString documentation = at->doc();
        QString staticValue = at->isStatic() ? QStringLiteral("static ") : QString();
        QString typeName = fixTypeName(at->getTypeName());
        QString initialValue = fixInitialStringDeclValue(at->getInitialValue(), typeName);
        if (!documentation.isEmpty())
            writeComment(documentation, m_indentation, java, true);
        java << m_startline << staticValue << "public " << typeName << " " << cleanName(at->name())
             << (initialValue.isEmpty() ? QString() : QStringLiteral(" = ") + initialValue) << ";";
    }

    Q_FOREACH(UMLAttribute *at, atprot){
        QString documentation = at->doc();
        QString typeName = fixTypeName(at->getTypeName());
        QString staticValue = at->isStatic() ? QStringLiteral("static ") : QString();
        QString initialValue = fixInitialStringDeclValue(at->getInitialValue(), typeName);
        if (!documentation.isEmpty())
            writeComment(documentation, m_indentation, java, true);
        java << m_startline << staticValue << "protected " << typeName << " " << cleanName(at->name())
             << (initialValue.isEmpty() ? QString() : QStringLiteral(" = ") + initialValue) << ";";
    }

    Q_FOREACH(UMLAttribute *at, atpriv) {
        QString documentation = at->doc();
        QString typeName = fixTypeName(at->getTypeName());
        QString staticValue = at->isStatic() ? QStringLiteral("static ") : QString();
        QString initialValue = fixInitialStringDeclValue(at->getInitialValue(), typeName);
        if (!documentation.isEmpty())
            writeComment(documentation, m_indentation, java, true);
        java << m_startline << staticValue << "private " << typeName << " " << cleanName(at->name())
             << (initialValue.isEmpty() ? QString() : QStringLiteral(" = ") + initialValue) << ";";
    }

}

/**
 * Calls @ref writeSingleAttributeAccessorMethods() on each of the attributes in atpub.
 */
void JavaWriter::writeAttributeMethods(UMLAttributeList &atpub, Uml::Visibility::Enum visibility, QTextStream &java)
{
    Q_FOREACH(UMLAttribute *at, atpub){
        QString fieldName = cleanName(at->name());
        // force capitalizing the field name, this is silly,
        // from what I can tell, this IS the default behavior for
        // cleanName. I dunno why it is not working -b.t.
        fieldName.replace(0, 1, fieldName.at(0).toUpper());

        writeSingleAttributeAccessorMethods(at->getTypeName(),
                                            cleanName(at->name()),
                                            fieldName,
                                            at->doc(),
                                            visibility, Uml::Changeability::Changeable, at->isStatic(), java);
    }
}

/**
 * Writes a // style comment.
 */
void JavaWriter::writeComment(const QString &comment, const QString &myIndent,
                              QTextStream &java, bool javaDocStyle)
{
    // in the case we have several line comment..
    // NOTE: this part of the method has the problem of adopting UNIX newline,
    // need to resolve for using with MAC/WinDoze eventually I assume
    if (comment.contains(QRegularExpression(QStringLiteral("\n")))) {

        if (javaDocStyle)
            java << myIndent << "/**" << m_endl;
        QStringList lines = comment.split(QStringLiteral("\n"));
        for (int i= 0; i < lines.count(); i++)
        {
            writeBlankLine(java);
            if (javaDocStyle)
                java << myIndent << " * ";
            else
                java << myIndent << "// ";
            java << lines[i];
        }
        if (javaDocStyle)
            java << myIndent << " */" << m_endl;
    } else {
        // this should be more fancy in the future, breaking it up into 80 char
        // lines so that it doesn't look too bad
        writeBlankLine(java);
        if (javaDocStyle)
            java << myIndent << "/**" << m_endl << myIndent << " *";
        else
            java << myIndent << "//";
        if (comment.length() > 0)
            java << " " << comment;
        if (javaDocStyle)
            java << m_endl << myIndent << " */";
    }
}

/**
 * Writes a documentation comment.
 */
void JavaWriter::writeDocumentation(QString header, QString body, QString end, QString indent, QTextStream &java)
{
    writeBlankLine(java);
    java << indent << "/**" << m_endl;
    if (!header.isEmpty())
        java << formatDoc(header, indent + QStringLiteral(" * "));
    if (!body.isEmpty())
        java << formatDoc(body, indent + QStringLiteral(" * "));
    if (!end.isEmpty())
    {
        QStringList lines = end.split(QStringLiteral("\n"));
        for (int i= 0; i < lines.count(); i++)
            java << formatDoc(lines[i], indent + QStringLiteral(" * "));
    }
    java << indent << " */";
}

/**
 * Searches a list of associations for appropriate ones to write out as attributes.
 */
void JavaWriter::writeAssociationDecls(UMLAssociationList associations, Uml::ID::Type id, QTextStream &java)
{
    if (forceSections() || !associations.isEmpty())
    {
        bool printRoleA = false, printRoleB = false;
        Q_FOREACH(UMLAssociation *a, associations) {
            // it may seem counter intuitive, but you want to insert the role of the
            // *other* class into *this* class.
            if (a->getObjectId(Uml::RoleType::A) == id)
                printRoleB = true;

            if (a->getObjectId(Uml::RoleType::B) == id)
                printRoleA = true;

            // First: we insert documentation for association IF it has either role AND some documentation (!)
            if ((printRoleA || printRoleB) && !(a->doc().isEmpty()))
                writeComment(a->doc(), m_indentation, java);

            // print RoleB decl
            if (printRoleB)
            {
                QString fieldClassName = cleanName(getUMLObjectName(a->getObject(Uml::RoleType::B)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::RoleType::B), a->getMultiplicity(Uml::RoleType::B), a->getRoleDoc(Uml::RoleType::B), a->visibility(Uml::RoleType::B), java);
            }

            // print RoleA decl
            if (printRoleA)
            {
                QString fieldClassName = cleanName(getUMLObjectName(a->getObject(Uml::RoleType::A)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::RoleType::A), a->getMultiplicity(Uml::RoleType::A), a->getRoleDoc(Uml::RoleType::A), a->visibility(Uml::RoleType::A), java);
            }
        }
    }
}

/**
 * Writes out an association as an attribute using Vector.
 */
void JavaWriter::writeAssociationRoleDecl(QString fieldClassName,
        QString roleName, QString multi,
        QString doc, Uml::Visibility::Enum visib, QTextStream &java)
{
    // ONLY write out IF there is a rolename given
    // otherwise it is not meant to be declared in the code
    if (roleName.isEmpty())
        return;

    QString scope = Uml::Visibility::toString(visib);

    // always put space between this and prior decl, if any
    writeBlankLine(java);

    if (!doc.isEmpty())
        writeComment(doc, m_indentation, java);

    // declare the association based on whether it is this a single variable
    // or a List (Vector). One day this will be done correctly with special
    // multiplicity object that we don't have to figure out what it means via regex.
    if (multi.isEmpty() || multi.contains(QRegularExpression(QStringLiteral("^[01]$"))))
    {
        QString fieldVarName = QStringLiteral("m_") + roleName.replace(0, 1, roleName.left(1).toLower());
        java << m_startline << scope << " " << fieldClassName << " " << fieldVarName << ";";
    }
    else
    {
        QString fieldVarName = roleName.toLower() + QStringLiteral("Vector");
        java << m_startline << scope << " Vector " << fieldVarName << " = new Vector();";
        // from here we could initialize default values, or put in an init() section
        // of the constructors
    }

}

/**
 * Calls @ref writeAssociationRoleMethod() on each of the associations in the given list.
 */
void JavaWriter::writeAssociationMethods (UMLAssociationList associations, UMLClassifier *thisClass, QTextStream &java)
{
    if (forceSections() || !associations.isEmpty()) {

        Q_FOREACH(UMLAssociation *a, associations) {

            // insert the methods to access the role of the other
            // class in the code of this one
            if (a->getObjectId(Uml::RoleType::A) == thisClass->id())
            {
                // only write out IF there is a rolename given
                if (!a->getRoleName(Uml::RoleType::B).isEmpty()) {
                    QString fieldClassName = getUMLObjectName(a->getObject(Uml::RoleType::B));
                    writeAssociationRoleMethod(fieldClassName,
                                               a->getRoleName(Uml::RoleType::B),
                                               a->getMultiplicity(Uml::RoleType::B), a->getRoleDoc(Uml::RoleType::B),
                                               a->visibility(Uml::RoleType::B),
                                               a->changeability(Uml::RoleType::B), java);
                }
            }

            if (a->getObjectId(Uml::RoleType::B) == thisClass->id())
            {
                // only write out IF there is a rolename given
                if (!a->getRoleName(Uml::RoleType::A).isEmpty()) {
                    QString fieldClassName = getUMLObjectName(a->getObject(Uml::RoleType::A));
                    writeAssociationRoleMethod(fieldClassName, a->getRoleName(Uml::RoleType::A),
                                               a->getMultiplicity(Uml::RoleType::A),
                                               a->getRoleDoc(Uml::RoleType::A),
                                               a->visibility(Uml::RoleType::A),
                                               a->changeability(Uml::RoleType::A),
                                               java);
                }
            }

        }
    }
}

/**
 * Calls @ref writeSingleAttributeAccessorMethods() or @ref
 * writeVectorAttributeAccessorMethods() on the association
 * role.
 */
void JavaWriter::writeAssociationRoleMethod (QString fieldClassName, QString roleName, QString multi,
        QString description, Uml::Visibility::Enum visib, Uml::Changeability::Enum change,
        QTextStream &java)
{
    if (multi.isEmpty() || multi.contains(QRegularExpression(QStringLiteral("^[01]$"))))
    {
        QString fieldVarName = QStringLiteral("m_") + roleName.replace(0, 1, roleName.left(1).toLower());
        writeSingleAttributeAccessorMethods(fieldClassName, fieldVarName, roleName,
                                            description, visib, change, false, java);
    }
    else
    {
        QString fieldVarName = roleName.toLower() + QStringLiteral("Vector");
        writeVectorAttributeAccessorMethods(fieldClassName, fieldVarName, roleName,
                                            description, visib, change, java);
    }
}

/**
 * Writes addFoo() and removeFoo() accessor methods for the Vector attribute.
 */
void JavaWriter::writeVectorAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
        QString fieldName, QString description,
        Uml::Visibility::Enum visibility, Uml::Changeability::Enum changeType,
        QTextStream &java)
{
    fieldClassName = fixTypeName(fieldClassName);
    fieldName = Codegen_Utils::capitalizeFirstLetter(fieldName);
    QString strVis = Uml::Visibility::toString(visibility);

    // ONLY IF changeability is NOT Frozen
    if (changeType != Uml::Changeability::Frozen)
    {
        writeDocumentation(QStringLiteral("Add a ") + fieldName + QStringLiteral(" object to the ") + fieldVarName + QStringLiteral(" List"),
                           description, QString(), m_indentation, java);
        java << m_startline << strVis << " void add" << fieldName << " (" << fieldClassName << " new_object) {";
        java << m_startline << m_indentation << fieldVarName << ".add(new_object);";
        java << m_startline << "}" << m_endl;
    }

    // ONLY IF changeability is Changeable
    if (changeType == Uml::Changeability::Changeable)
    {
        writeDocumentation(QStringLiteral("Remove a ") + fieldName + QStringLiteral(" object from ") + fieldVarName + QStringLiteral(" List"),
                           description, QString(), m_indentation, java);
        java << m_startline << strVis << " void remove" << fieldName << " (" << fieldClassName << " new_object)";
        java << m_startline << "{";
        java << m_startline << m_indentation << fieldVarName << ".remove(new_object);";
        java << m_startline << "}" << m_endl;
    }

    // always allow getting the list of stuff
    writeDocumentation(QStringLiteral("Get the List of ") + fieldName + QStringLiteral(" objects held by ") + fieldVarName, description, QStringLiteral("@return List of ") + fieldName + QStringLiteral(" objects held by ") + fieldVarName, m_indentation, java);
    java << m_startline << strVis << " List get" << fieldName << "List () {";
    java << m_startline << m_indentation << "return (List) " << fieldVarName << ";";
    java << m_startline << "}" << m_endl;
    writeBlankLine(java);
}

/**
 * Writes getFoo() and setFoo() accessor methods for the attribute.
 */
void JavaWriter::writeSingleAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
        QString fieldName, QString description,
        Uml::Visibility::Enum visibility, Uml::Changeability::Enum change,
        bool isFinal, QTextStream &java)
{
    QString strVis = Uml::Visibility::toString(visibility);
    fieldClassName = fixTypeName(fieldClassName);
    fieldName = Codegen_Utils::capitalizeFirstLetter(fieldName);

    // set method
    if (change == Uml::Changeability::Changeable && !isFinal) {
        writeDocumentation(QStringLiteral("Set the value of ") + fieldVarName, description, QStringLiteral("@param newVar the new value of ") + fieldVarName, m_indentation, java);
        java << m_startline << strVis << " void set" << fieldName << " (" << fieldClassName << " newVar) {";
        java << m_startline << m_indentation << fieldVarName << " = newVar;";
        java << m_startline << "}" << m_endl;
    }

    // get method
    writeDocumentation(QStringLiteral("Get the value of ") + fieldVarName, description, QStringLiteral("@return the value of ") + fieldVarName, m_indentation, java);
    java << m_startline << strVis << " " << fieldClassName << " get" << fieldName << " () {";
    java << m_startline << m_indentation << "return " << fieldVarName << ";";
    java << m_startline << "}";
    writeBlankLine(java);
}

/**
 * Writes the comment and class constructor.
 */
void JavaWriter::writeConstructor(UMLClassifier *c, QTextStream &java)
{
    if (forceDoc()) {
        java << m_startline;
        writeComment(QString(), m_indentation, java);
        writeComment(QStringLiteral("Constructors"), m_indentation, java);
        writeComment(QString(), m_indentation, java);
        writeBlankLine(java);
    }

    // write the first constructor
    QString className = cleanName(c->name());
    java << m_indentation << "public " << className << " () { };";
}

/**
 * Replaces `string' with `String' and `bool' with `boolean'.
 * IF the type is "string" we need to declare it as
 * the Java Object "String" (there is no string primitive in Java).
 * Same thing again for "bool" to "boolean".
 */
QString JavaWriter::fixTypeName(const QString& string)
{
    if (string.isEmpty())
        return QStringLiteral("void");
    if (string == QStringLiteral("string"))
        return QStringLiteral("String");
    if (string == QStringLiteral("bool"))
        return QStringLiteral("boolean");
    return string;
}

/**
 * Overrides method from class CodeGenerator.
 * @return   the list of default datatypes
 */
QStringList JavaWriter::defaultDatatypes() const
{
    QStringList l;
    l.append(QStringLiteral("int"));
    l.append(QStringLiteral("char"));
    l.append(QStringLiteral("boolean"));
    l.append(QStringLiteral("float"));
    l.append(QStringLiteral("double"));
    l.append(QStringLiteral("byte"));
    l.append(QStringLiteral("short"));
    l.append(QStringLiteral("long"));
    l.append(QStringLiteral("String"));
    l.append(QStringLiteral("Integer"));
    l.append(QStringLiteral("Character"));
    l.append(QStringLiteral("Boolean"));
    l.append(QStringLiteral("Float"));
    l.append(QStringLiteral("Double"));
    l.append(QStringLiteral("Byte"));
    l.append(QStringLiteral("Short"));
    l.append(QStringLiteral("Long"));
    l.append(QStringLiteral("StringBuffer"));
    l.append(QStringLiteral("StringBuilder"));
    return l;
}

/**
 * Return true if the two operations have the same name and the same parameters.
 * @param op1   first operation to be compared
 * @param op2   second operation to be compared
 */
bool JavaWriter::compareJavaMethod(UMLOperation *op1, UMLOperation *op2)
{
    if (op1 == 0 || op2 == 0)
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
        UMLAttribute *at1 = atl1It.next();
        UMLAttribute *at2 = atl2It.next();
        if (at1->getTypeName() != at2->getTypeName())
            return false;
    }
    return true;

}

/**
 * Return true if the operation is in the list.
 * @param umlOp   operation to be searched
 * @param opl     list of operations
 */
bool JavaWriter::javaMethodInList(UMLOperation *umlOp, UMLOperationList &opl)
{
    Q_FOREACH(UMLOperation *op, opl) {
        if (JavaWriter::compareJavaMethod(op, umlOp)) {
            return true;
        }
    }
    return false;
}

/**
 * Get all operations which a given class inherit from all its super interfaces and get all operations
 * which this given class inherit from all its super classes.
 * @param c                       the class for which we are generating code
 * @param yetImplementedOpList    the list of yet implemented operations
 * @param toBeImplementedOpList   the list of to be implemented operations
 * @param noClassInPath           tells if there is a class between the base class and the current interface
 */
void JavaWriter::getSuperImplementedOperations(UMLClassifier *c, UMLOperationList &yetImplementedOpList, UMLOperationList &toBeImplementedOpList, bool noClassInPath)
{
    UMLClassifierList superClasses = c->findSuperClassConcepts();

    Q_FOREACH(UMLClassifier *classifier, superClasses) {

        getSuperImplementedOperations(classifier, yetImplementedOpList, toBeImplementedOpList, (classifier->isInterface() && noClassInPath));
        UMLOperationList opl = classifier->getOpList();
        Q_FOREACH(UMLOperation *op, opl) {
            if (classifier->isInterface() && noClassInPath) {
                if (!JavaWriter::javaMethodInList(op, toBeImplementedOpList))
                    toBeImplementedOpList.append(op);
            }
            else
            {
                if (!JavaWriter::javaMethodInList(op, yetImplementedOpList))
                    yetImplementedOpList.append(op);
            }
        }
    }

}

/**
 * Get all operations which a given class inherit from all its super interfaces and that should be implemented.
 * @param c     the class for which we are generating code
 * @param opList  the list of operations used to append the operations
 */
void JavaWriter::getInterfacesOperationsToBeImplemented(UMLClassifier *c, UMLOperationList &opList)
{
    UMLOperationList yetImplementedOpList;
    UMLOperationList toBeImplementedOpList;

    getSuperImplementedOperations(c, yetImplementedOpList, toBeImplementedOpList);
    Q_FOREACH(UMLOperation *op, toBeImplementedOpList) {
        if (! JavaWriter::javaMethodInList(op, yetImplementedOpList) && ! JavaWriter::javaMethodInList(op, opList))
            opList.append(op);
    }
}

/**
 * Write all operations for a given class.
 * @param c   the class for which we are generating code
 * @param java  the stream associated with the output file
 */
void JavaWriter::writeOperations(UMLClassifier *c, QTextStream &java) {
    UMLOperationList opl;
    UMLOperationList oppub, opprot, oppriv;

    //sort operations by scope first and see if there are abstract methods
    opl = c->getOpList();
    if (! c->isInterface()) {
        getInterfacesOperationsToBeImplemented(c, opl);
    }
    Q_FOREACH(UMLOperation *op, opl) {
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
          default:
            break;
        }
    }

    // do people REALLY want these comments? Hmm.
    /*
      if (forceSections() || oppub.count())
      {
      writeComment(QStringLiteral("public operations"), m_indentation, java);
        writeBlankLine(java);
      }
    */
    writeOperations(oppub, java);

    /*
      if (forceSections() || opprot.count())
      {
      writeComment(QStringLiteral("protected operations"), m_indentation, java);
        writeBlankLine(java);
      }
    */
    writeOperations(opprot, java);

    /*
      if (forceSections() || oppriv.count())
      {
      writeComment(QStringLiteral("private operations"), m_indentation, java);
        writeBlankLine(java);
      }
    */
    writeOperations(oppriv, java);

}

/**
 * Write a list of operations for a given class.
 * @param oplist the list of operations you want to write
 * @param java   the stream associated with the output file
 */
void JavaWriter::writeOperations(UMLOperationList &oplist, QTextStream &java)
{
    UMLAttributeList atl;
    int i, j;
    QString str;

    // generate method decl for each operation given
    Q_FOREACH(UMLOperation* op, oplist) {

        QString doc;
        // write documentation

        QString methodReturnType = fixTypeName(op->getTypeName());
        if (methodReturnType != QStringLiteral("void"))
            doc += QStringLiteral("@return       ") + methodReturnType + QLatin1Char('\n');

        str = QString(); // reset for next method
        str += ((op->isAbstract() && !m_isInterface) ? QStringLiteral("abstract ") : QString());
        str += Uml::Visibility::toString(op->visibility()) + QLatin1Char(' ');
        str += (op->isStatic() ? QStringLiteral("static ") : QString());
        str += methodReturnType + QLatin1Char(' ') + cleanName(op->name()) + QLatin1Char('(');

        atl = op->getParmList();
        i= atl.count();
        j=0;
        Q_FOREACH(UMLAttribute* at, atl) {
            QString typeName = fixTypeName(at->getTypeName());
            QString atName = cleanName(at->name());
            str += typeName + QLatin1Char(' ') + atName +
                   (!(at->getInitialValue().isEmpty()) ?
                    (QStringLiteral(" = ") + at->getInitialValue()) : QString())
                   + ((j < i-1) ? QStringLiteral(", ") : QString());
            doc += QStringLiteral("@param        ") + atName + QLatin1Char(' ') + at->doc() + QLatin1Char('\n');
            j++;
        }
        doc = doc.remove(doc.size() - 1, 1);  // remove last QLatin1Char('\n')of comment
        str += QLatin1Char(')');

        // method only gets a body IF it is not abstract
        if (op->isAbstract() || m_isInterface)
            str += QStringLiteral(";\n\n"); // terminate now
        else {
            str += m_startline + QLatin1Char('{') + m_endl;
            QString sourceCode = op->getSourceCode();
            if (sourceCode.isEmpty()) {
                // empty method body - TODO: throw exception
            }
            else {
                str += formatSourceCode(sourceCode, m_indentation + m_indentation);
            }
            str += m_indentation + QLatin1Char('}') + m_endl + m_endl;
        }

        // write it out
        writeDocumentation(QString(), op->doc(), doc, m_indentation, java);
        java << m_startline << str;
    }
}

/**
 * Check that initial values of strings have quotes around them.
 */
QString JavaWriter::fixInitialStringDeclValue(const QString& val, const QString& type)
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

/**
 * Returns the name of the given object (if it exists).
 */
QString JavaWriter::getUMLObjectName(UMLObject *obj)
{
    return (obj ? obj->name() : QStringLiteral("NULL"));
}

/**
 * Write a blank line.
 * Note: Methods like this _shouldn't_ be needed IF we properly did things thruought the code.
 */
void JavaWriter::writeBlankLine(QTextStream &java)
{
    java << m_endl;
}
