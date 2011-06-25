/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003 Brian Thomas <brian.thomas@gsfc.nasa.gov>          *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

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
#include "umltemplatelist.h"

// qt includes
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>

/**
 * Constructor, initialises a couple of variables.
 */
JavaWriter::JavaWriter()
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
Uml::ProgrammingLanguage JavaWriter::language() const
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
        uDebug()<<"Cannot write class of NULL concept!\n";
        return;
    }

    m_isInterface = c->isInterface();

    QString fileName = cleanName(c->name().toLower());

    //find an appropriate name for our file
    fileName = findFileName(c,".java");
    if (fileName.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    // check that we may open that file for writing
    QFile file;
    if ( !openFile(file, fileName) ) {
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
        foreach (UMLAttribute *at ,  atl ) {
            switch(at->visibility())
            {
              case Uml::Visibility::Public:
                if(at->isStatic())
                    final_atpub.append(at);
                else
                    atpub.append(at);
                break;
              case Uml::Visibility::Protected:
                if(at->isStatic())
                    final_atprot.append(at);
                else
                    atprot.append(at);
                break;
              case Uml::Visibility::Private:
                if(at->isStatic())
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

    //try to find a heading file (license, coments, etc)
    QString str;
    str = getHeadingFile(".java");
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"),fileName);
        str.replace(QRegExp("%filepath%"),file.fileName());
        java<<str<<m_endl;
    }

    if (!c->package().isEmpty())
        java<<"package "<<c->package()<<";"<<m_endl;

    // IMPORT statements
    // Q: Why all utils? Aren't just List and Vector the only classes we are using?
    // A: doesn't matter at all; it is more readable to just include '*' and java compilers
    //    don't slow down or anything. (TZ)
    if (hasVectorFields )
    {
        writeBlankLine(java);
        java<<"import java.util.*;"<<m_endl;
    }

    //only import classes in a different package as this class
    UMLPackageList imports;
    findObjectsRelated(c,imports);
    foreach (UMLPackage* con,  imports ) {
        if (con->baseType() == UMLObject::ot_Datatype)
            continue;
        QString pkg = con->package();
        if (!pkg.isEmpty() && pkg != c->package())
            java << "import " << pkg << "." << cleanName(con->name()) << ";"
            << m_endl;
    }
    writeBlankLine(java);

    // write the opening declaration for the class incl any documentation,
    // interfaces and/or inheritence issues we have
    writeClassDecl(c, java);

    // start body of class
    java<<" {"<<m_endl;

    // ATTRIBUTES
    //

    // write comment for section IF needed
    if (forceDoc() || hasAccessorMethods)
    {
        writeComment("", m_indentation, java);
        writeComment("Fields", m_indentation, java);
        writeComment("", m_indentation, java);
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
        java<<m_startline;
        writeComment("", m_indentation, java);
        writeComment("Methods", m_indentation, java);
        writeComment("", m_indentation, java);
        writeBlankLine(java);
        writeBlankLine(java);
    }

    // write comment for sub-section IF needed
    if (forceDoc() || hasAccessorMethods )
    {
        writeComment("", m_indentation, java);
        writeComment("Accessor methods", m_indentation, java);
        writeComment("", m_indentation, java);
        writeBlankLine(java);
    }

    // Accessors for attributes
    writeAttributeMethods(final_atpub, Uml::Visibility::Public, java);
    writeAttributeMethods(final_atprot, Uml::Visibility::Protected, java);
    writeAttributeMethods(final_atpriv, Uml::Visibility::Private, java);
    writeAttributeMethods(atpub, Uml::Visibility::Public, java);
    writeAttributeMethods(atprot, Uml::Visibility::Protected, java);
    writeAttributeMethods(atpriv, Uml::Visibility::Private, java);

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
        writeComment("", m_indentation, java);
        writeComment("Other methods", m_indentation, java);
        writeComment("", m_indentation, java);
        writeBlankLine(java);
    }
    writeOperations(c,java);

    writeBlankLine(java);
    java<<"}"<<m_endl; // end class

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
            writeDocumentation("Interface "+classname,c->doc(),"","",java);
        else
            writeDocumentation("Class "+classname,c->doc(),"","",java);

        writeBlankLine(java);
    }

    // Now write the actual class declaration
    QString scope = ""; // = c->getVisibility().toString();
    if (c->visibility() != Uml::Visibility::Public) {
        // We should emit a warning in here .. java doesn't like to allow
        // private/protected classes. The best we can do (I believe)
        // is to let these declarations default to "package visibility"
        // which is a level between traditional "private" and "protected"
        // scopes. To get this visibility level we just print nothing..
    } else
        scope = "public ";

    java << ((c->isAbstract() && !m_isInterface) ? QString("abstract ") : QString("")) << scope;
    if (m_isInterface)
        java << "interface ";
    else
        java << "class ";

    java << classname;

    // Generics
    UMLTemplateList template_params = c->getTemplateList();
    if (template_params.count()) {
        java << "<";
        for (UMLTemplateListIt tlit( template_params ); tlit.hasNext(); ) {
            UMLTemplate* t = tlit.next();
            QString formalName = t->name();
            java << formalName;
            QString typeName = t->getTypeName();
            if (typeName != "class") {
                java << " extends " << typeName;
            }
            if ( tlit.hasNext() ) {
                tlit.next();
                java << ", ";
            }
        }
        java << ">" << m_endl;
    }

    // write inheritances out
    UMLClassifierList superclasses = c->findSuperClassConcepts(UMLClassifier::CLASS);

    int i = 0;
    foreach ( UMLClassifier *concept, superclasses ) {
        if (i == 0)
        {
            java<< " extends ";
        }
        else
        {
            //The java generated code is wrong ! : No multiple inheritence of class
            java<< ", " ;
        }
        java<< cleanName(concept->name());
        i++;
    }

    UMLClassifierList superInterfaces = c->findSuperClassConcepts(UMLClassifier::INTERFACE);
    i = 0;
    foreach ( UMLClassifier *concept, superInterfaces ) {
        if (i == 0)
        {
            if (m_isInterface)
                java<< " extends ";
            else
                java<< " implements ";
        }
        else
        {
            //The java generated code is OK ! : multiple inheritence of interface
            java<< ", " ;
        }
        java<< cleanName(concept->name());
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
                                     UMLAttributeList &atpriv, QTextStream &java )
{
    foreach (UMLAttribute *at, atpub ) {
        QString documentation = at->doc();
        QString staticValue = at->isStatic() ? "static " : "";
        QString typeName = fixTypeName(at->getTypeName());
        QString initialValue = fixInitialStringDeclValue(at->getInitialValue(), typeName);
        if (!documentation.isEmpty())
            writeComment(documentation, m_indentation, java, true);
        java<<m_startline<<staticValue<<"public "<<typeName<<" "<<cleanName(at->name())
        <<(initialValue.isEmpty()?QString(""):QString(" = ") + initialValue)<<";";
    }

    foreach (UMLAttribute *at, atprot ){
        QString documentation = at->doc();
        QString typeName = fixTypeName(at->getTypeName());
        QString staticValue = at->isStatic() ? "static " : "";
        QString initialValue = fixInitialStringDeclValue(at->getInitialValue(), typeName);
        if (!documentation.isEmpty())
            writeComment(documentation, m_indentation, java, true);
        java<<m_startline<<staticValue<<"protected "<<typeName<<" "<<cleanName(at->name())
        <<(initialValue.isEmpty()?QString(""):QString(" = ") + initialValue)<<";";
    }

    foreach (UMLAttribute *at, atpriv ) {
        QString documentation = at->doc();
        QString typeName = fixTypeName(at->getTypeName());
        QString staticValue = at->isStatic() ? "static " : "";
        QString initialValue = fixInitialStringDeclValue(at->getInitialValue(), typeName);
        if (!documentation.isEmpty())
            writeComment(documentation, m_indentation, java, true);
        java<<m_startline<<staticValue<<"private "<<typeName<<" "<<cleanName(at->name())
        <<(initialValue.isEmpty()?QString(""):QString(" = ") + initialValue)<<";";
    }

}

/**
 * Calls @ref writeSingleAttributeAccessorMethods() on each of the attributes in atpub.
 */
void JavaWriter::writeAttributeMethods(UMLAttributeList &atpub, Uml::Visibility visibility, QTextStream &java)
{
    foreach (UMLAttribute *at, atpub ){
        QString fieldName = cleanName(at->name());
        // force capitalizing the field name, this is silly,
        // from what I can tell, this IS the default behavior for
        // cleanName. I dunno why it is not working -b.t.
        fieldName.replace(0,1,fieldName.at(0).toUpper());

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
    if (comment.contains(QRegExp("\n"))) {

        if (javaDocStyle)
            java << myIndent << "/**" << m_endl;
        QStringList lines = comment.split( "\n" );
        for (int i= 0; i < lines.count(); i++)
        {
            writeBlankLine(java);
            if (javaDocStyle)
                java<<myIndent<<" * ";
            else
                java<<myIndent<<"// ";
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
            java<<myIndent<<"//";
        if(comment.length() > 0)
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
    java<<indent<<"/**"<<m_endl;
    if (!header.isEmpty())
        java<<formatDoc(header, indent+" * ");
    if (!body.isEmpty())
        java<<formatDoc(body, indent+" * ");
    if (!end.isEmpty())
    {
        QStringList lines = end.split( "\n" );
        for (int i= 0; i < lines.count(); i++)
            java<<formatDoc(lines[i], indent+" * ");
    }
    java<<indent<<" */";
}

/**
 * Searches a list of associations for appropriate ones to write out as attributes.
 */
void JavaWriter::writeAssociationDecls(UMLAssociationList associations, Uml::IDType id, QTextStream &java)
{
    if ( forceSections() || !associations.isEmpty() )
    {
        bool printRoleA = false, printRoleB = false;
        foreach (UMLAssociation *a , associations ) {
            // it may seem counter intuitive, but you want to insert the role of the
            // *other* class into *this* class.
            if (a->getObjectId(Uml::A) == id)
                printRoleB = true;

            if (a->getObjectId(Uml::B) == id)
                printRoleA = true;

            // First: we insert documentaion for association IF it has either role AND some documentation (!)
            if ((printRoleA || printRoleB) && !(a->doc().isEmpty()))
                writeComment(a->doc(), m_indentation, java);

            // print RoleB decl
            if (printRoleB)
            {
                QString fieldClassName = cleanName(getUMLObjectName(a->getObject(Uml::B)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::B), a->getMulti(Uml::B), a->getRoleDoc(Uml::B), a->getVisibility(Uml::B), java);
            }

            // print RoleA decl
            if (printRoleA)
            {
                QString fieldClassName = cleanName(getUMLObjectName(a->getObject(Uml::A)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::A), a->getMulti(Uml::A), a->getRoleDoc(Uml::A), a->getVisibility(Uml::A), java);
            }
        }
    }
}

/**
 * Writes out an association as an attribute using Vector.
 */
void JavaWriter::writeAssociationRoleDecl(QString fieldClassName,
        QString roleName, QString multi,
        QString doc, Uml::Visibility visib, QTextStream &java)
{
    // ONLY write out IF there is a rolename given
    // otherwise it is not meant to be declared in the code
    if (roleName.isEmpty())
        return;

    QString scope = visib.toString();

    // always put space between this and prior decl, if any
    writeBlankLine(java);

    if (!doc.isEmpty())
        writeComment(doc, m_indentation, java);

    // declare the association based on whether it is this a single variable
    // or a List (Vector). One day this will be done correctly with special
    // multiplicity object that we don't have to figure out what it means via regex.
    if (multi.isEmpty() || multi.contains(QRegExp("^[01]$")))
    {
        QString fieldVarName = "m_" + roleName.replace(0, 1, roleName.left(1).toLower());
        java<<m_startline<<scope<<" "<<fieldClassName<<" "<<fieldVarName<<";";
    }
    else
    {
        QString fieldVarName = roleName.toLower() + "Vector";
        java<<m_startline<<scope<<" Vector "<<fieldVarName<<" = new Vector();";
        // from here we could initialize default values, or put in an init() section
        // of the constructors
    }

}

/**
 * Calls @ref writeAssociationRoleMethod() on each of the associations in the given list.
 */
void JavaWriter::writeAssociationMethods (UMLAssociationList associations, UMLClassifier *thisClass, QTextStream &java)
{
    if ( forceSections() || !associations.isEmpty() ) {

        foreach(UMLAssociation *a , associations ) {

            // insert the methods to access the role of the other
            // class in the code of this one
            if (a->getObjectId(Uml::A) == thisClass->id())
            {
                // only write out IF there is a rolename given
                if (!a->getRoleName(Uml::B).isEmpty()) {
                    QString fieldClassName = getUMLObjectName(a->getObject(Uml::B));
                    writeAssociationRoleMethod(fieldClassName,
                                               a->getRoleName(Uml::B),
                                               a->getMulti(Uml::B), a->getRoleDoc(Uml::B),
                                               a->getVisibility(Uml::B),
                                               a->changeability(Uml::B), java);
                }
            }

            if (a->getObjectId(Uml::B) == thisClass->id())
            {
                // only write out IF there is a rolename given
                if (!a->getRoleName(Uml::A).isEmpty()) {
                    QString fieldClassName = getUMLObjectName(a->getObject(Uml::A));
                    writeAssociationRoleMethod(fieldClassName, a->getRoleName(Uml::A),
                                               a->getMulti(Uml::A),
                                               a->getRoleDoc(Uml::A),
                                               a->getVisibility(Uml::A),
                                               a->changeability(Uml::A),
                                               java);
                }
            }

        }
    }
}

/**
 * Calls @ref writeSingleAttributeAccessorMethods() or @ref
 * writeVectorAttributeAccessorMethods() on the assocaition
 * role.
 */
void JavaWriter::writeAssociationRoleMethod (QString fieldClassName, QString roleName, QString multi,
        QString description, Uml::Visibility visib, Uml::Changeability change,
        QTextStream &java)
{
    if (multi.isEmpty() || multi.contains(QRegExp("^[01]$")))
    {
        QString fieldVarName = "m_" + roleName.replace(0, 1, roleName.left(1).toLower());
        writeSingleAttributeAccessorMethods(fieldClassName, fieldVarName, roleName,
                                            description, visib, change, false, java);
    }
    else
    {
        QString fieldVarName = roleName.toLower() + "Vector";
        writeVectorAttributeAccessorMethods(fieldClassName, fieldVarName, roleName,
                                            description, visib, change, java);
    }
}

/**
 * Writes addFoo() and removeFoo() accessor methods for the Vector attribute.
 */
void JavaWriter::writeVectorAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
        QString fieldName, QString description,
        Uml::Visibility visibility, Uml::Changeability changeType,
        QTextStream &java)
{
    fieldClassName = fixTypeName(fieldClassName);
    fieldName = Codegen_Utils::capitalizeFirstLetter(fieldName);
    QString strVis = visibility.toString();

    // ONLY IF changeability is NOT Frozen
    if (changeType != Uml::Changeability::Frozen)
    {
        writeDocumentation("Add a "+fieldName+" object to the "+fieldVarName+" List",description,"",m_indentation,java);
        java<<m_startline<<strVis<<" void add"<<fieldName<<" ( "<<fieldClassName<<" new_object ) {";
        java<<m_startline<<m_indentation<<fieldVarName<<".add(new_object);";
        java<<m_startline<<"}"<<m_endl;
    }

    // ONLY IF changeability is Changeable
    if (changeType == Uml::Changeability::Changeable)
    {
        writeDocumentation("Remove a "+fieldName+" object from "+fieldVarName+" List",description,"",m_indentation,java);
        java<<m_startline<<strVis<<" void remove"<<fieldName<<" ( "<<fieldClassName<<" new_object )";
        java<<m_startline<<"{";
        java<<m_startline<<m_indentation<<fieldVarName<<".remove(new_object);";
        java<<m_startline<<"}"<<m_endl;
    }

    // always allow getting the list of stuff
    writeDocumentation("Get the List of "+fieldName+" objects held by "+fieldVarName,description,"@return List of "+fieldName+" objects held by "+fieldVarName,m_indentation,java);
    java<<m_startline<<strVis<<" List get"<<fieldName<<"List ( ) {";
    java<<m_startline<<m_indentation<<"return (List) "<<fieldVarName<<";";
    java<<m_startline<<"}"<<m_endl;
    writeBlankLine(java);
}

/**
 * Writes getFoo() and setFoo() accessor methods for the attribute.
 */
void JavaWriter::writeSingleAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
        QString fieldName, QString description,
        Uml::Visibility visibility, Uml::Changeability change,
        bool isFinal, QTextStream &java)
{
    QString strVis = visibility.toString();
    fieldClassName = fixTypeName(fieldClassName);
    fieldName = Codegen_Utils::capitalizeFirstLetter(fieldName);

    // set method
    if (change == Uml::Changeability::Changeable && !isFinal) {
        writeDocumentation("Set the value of "+fieldVarName,description,"@param newVar the new value of "+fieldVarName,m_indentation,java);
        java<<m_startline<<strVis<<" void set"<<fieldName<<" ( "<<fieldClassName<<" newVar ) {";
        java<<m_startline<<m_indentation<<fieldVarName<<" = newVar;";
        java<<m_startline<<"}"<<m_endl;
    }

    // get method
    writeDocumentation("Get the value of "+fieldVarName,description,"@return the value of "+fieldVarName,m_indentation,java);
    java<<m_startline<<strVis<<" "<<fieldClassName<<" get"<<fieldName<<" ( ) {";
    java<<m_startline<<m_indentation<<"return "<<fieldVarName<<";";
    java<<m_startline<<"}";
    writeBlankLine(java);
}

/**
 * Writes the comment and class constructor.
 */
void JavaWriter::writeConstructor(UMLClassifier *c, QTextStream &java)
{
    if (forceDoc()) {
        java<<m_startline;
        writeComment("", m_indentation, java);
        writeComment("Constructors", m_indentation, java);
        writeComment("", m_indentation, java);
        writeBlankLine(java);
    }

    // write the first constructor
    QString className = cleanName(c->name());
    java<<m_indentation<<"public "<<className<<" () { };";
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
        return "void";
    if (string == "string")
        return "String";
    if (string == "bool")
        return "boolean";
    return string;
}

/**
 * Overrides method from class CodeGenerator.
 * @return   the list of default datatypes
 */
QStringList JavaWriter::defaultDatatypes()
{
    QStringList l;
    l.append("int");
    l.append("char");
    l.append("boolean");
    l.append("float");
    l.append("double");
    l.append("byte");
    l.append("short");
    l.append("long");
    l.append("String");
    return l;
}

/**
 * Return true if the two operations have the same name and the same parameters.
 * @param op1   first operation to be compared
 * @param op2   second operation to be compared
 */
bool JavaWriter::compareJavaMethod(UMLOperation *op1, UMLOperation *op2)
{
    if (op1 == NULL || op2 == NULL)
        return false;
    if (op1 == op2)
        return true;
    if (op1->name() != op2->name())
        return false;
    UMLAttributeList atl1 = op1->getParmList();
    UMLAttributeList atl2 = op2->getParmList();
    if (atl1.count() != atl2.count())
        return false;
    for (UMLAttributeListIt atl1It( atl1 ), atl2It( atl2 ); atl1It.hasNext() && atl2It.hasNext(); ) {
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
    foreach (UMLOperation *op , opl ) {
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
void JavaWriter::getSuperImplementedOperations(UMLClassifier *c, UMLOperationList &yetImplementedOpList ,UMLOperationList &toBeImplementedOpList, bool noClassInPath)
{
    UMLClassifierList superClasses = c->findSuperClassConcepts();

    foreach (UMLClassifier *concept, superClasses) {

        getSuperImplementedOperations(concept, yetImplementedOpList, toBeImplementedOpList, (concept->isInterface() && noClassInPath));
        UMLOperationList opl = concept->getOpList();
        foreach (UMLOperation *op , opl ) {
            if (concept->isInterface() && noClassInPath) {
                if (!JavaWriter::javaMethodInList(op,toBeImplementedOpList))
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
 * @param opl   the list of operations used to append the operations
 */
void JavaWriter::getInterfacesOperationsToBeImplemented(UMLClassifier *c, UMLOperationList &opList )
{
    UMLOperationList yetImplementedOpList;
    UMLOperationList toBeImplementedOpList;

    getSuperImplementedOperations(c,yetImplementedOpList, toBeImplementedOpList);
    foreach (UMLOperation *op , toBeImplementedOpList) {
        if ( ! JavaWriter::javaMethodInList(op, yetImplementedOpList) && ! JavaWriter::javaMethodInList(op, opList) )
            opList.append(op);
    }
}

/**
 * Write all operations for a given class.
 * @param c   the class for which we are generating code
 * @param j   the stream associated with the output file
 */
void JavaWriter::writeOperations(UMLClassifier *c, QTextStream &java) {
    UMLOperationList opl;
    UMLOperationList oppub,opprot,oppriv;

    //sort operations by scope first and see if there are abstract methods
    opl = c->getOpList();
    if (! c->isInterface()) {
        getInterfacesOperationsToBeImplemented(c, opl);
    }
    foreach (UMLOperation *op , opl ) {
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
      if(forceSections() || oppub.count())
      {
      writeComment("public operations",m_indentation,java);
        writeBlankLine(java);
      }
    */
    writeOperations(oppub,java);

    /*
      if(forceSections() || opprot.count())
      {
      writeComment("protected operations",m_indentation,java);
        writeBlankLine(java);
      }
    */
    writeOperations(opprot,java);

    /*
      if(forceSections() || oppriv.count())
      {
      writeComment("private operations",m_indentation,java);
        writeBlankLine(java);
      }
    */
    writeOperations(oppriv,java);

}

/**
 * Write a list of operations for a given class.
 * @param list   the list of operations you want to write
 * @param j      the stream associated with the output file
 */
void JavaWriter::writeOperations(UMLOperationList &oplist, QTextStream &java)
{
    UMLAttributeList atl;
    int i,j;
    QString str;

    // generate method decl for each operation given
    foreach( UMLOperation* op ,  oplist ){

        QString doc = "";
        // write documentation

        QString methodReturnType = fixTypeName(op->getTypeName());
        if (methodReturnType != "void")
            doc += "@return       " + methodReturnType + '\n';

        str = ""; // reset for next method
        str += ((op->isAbstract() && !m_isInterface) ? "abstract ":"");
        str += op->visibility().toString() + ' ';
        str += (op->isStatic() ? "static ":"");
        str += methodReturnType + ' ' + cleanName(op->name()) + "( ";

        atl = op->getParmList();
        i= atl.count();
        j=0;
        foreach ( UMLAttribute* at , atl ) {
            QString typeName = fixTypeName(at->getTypeName());
            QString atName = cleanName(at->name());
            str += typeName + ' ' + atName +
                   (!(at->getInitialValue().isEmpty()) ?
                    (QString(" = ")+at->getInitialValue()) :
                    QString(""))
                   + ((j < i-1)?", ":"");
            doc += "@param        " + atName + ' ' + at->doc() + '\n';
            j++;
        }
        doc = doc.remove(doc.size() - 1, 1);  // remove last endl of comment
        str+= " )";

        // method only gets a body IF it is not abstract
        if (op->isAbstract() || m_isInterface)
            str+=";\n\n"; // terminate now
        else {
            str += m_startline + '{' + m_endl;
            QString sourceCode = op->getSourceCode();
            if (sourceCode.isEmpty()) {
                // empty method body - TODO: throw exception
            }
            else {
                str += formatSourceCode(sourceCode, m_indentation + m_indentation);
            }
            str += m_indentation + '}' + m_endl + m_endl;
        }

        // write it out
        writeDocumentation("", op->doc(), doc, m_indentation, java);
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
    if (!value.isEmpty() && type == "String") {
        if (!value.startsWith('"'))
            value.prepend('"');
        if (!value.endsWith('"'))
            value.append('"');
    }
    return value;
}

/**
 * Returns the name of the given object (if it exists).
 */
QString JavaWriter::getUMLObjectName(UMLObject *obj)
{
    return(obj!=0)?obj->name():QString("NULL");
}

/**
 * Write a blank line.
 * Note: Methods like this _shouldn't_ be needed IF we properly did things thruought the code.
 */
void JavaWriter::writeBlankLine(QTextStream &java)
{
    java<<m_endl;
}
