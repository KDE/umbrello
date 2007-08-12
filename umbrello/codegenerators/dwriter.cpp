
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

/***************************************************************************
    This is the "old" code generator that does not support code editing
    in the Modeller but uses significantly less file space because the
    source code is not replicated in the XMI file.
 ***************************************************************************/

// own header
#include "dwriter.h"
// qt includes
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
// kde includes
#include <kdebug.h>
// app includes
#include "../umldoc.h"
#include "../classifier.h"
#include "../operation.h"
#include "../attribute.h"
#include "../association.h"
#include "../template.h"
#include "../umltemplatelist.h"
#include "codegen_utils.h"

DWriter::DWriter() {
    startline = m_endl + m_indentation;
}

DWriter::~DWriter() {}

Uml::Programming_Language DWriter::getLanguage() {
    return Uml::pl_D;
}

// FIXME: doesn't work yet
void DWriter::writeModuleDecl(UMLClassifier *c, QTextStream &d) {
    if(!c->getPackage().isEmpty())
        d << "module " << c->getPackage() << ";" << m_endl;

    writeBlankLine(d);
}

void DWriter::writeModuleImports(UMLClassifier *c, QTextStream &d) {
    // another preparation, determine what we have
    UMLAssociationList associations = c->getSpecificAssocs(Uml::at_Association); // BAD! only way to get "general" associations.
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
    for (UMLPackage *con = imports.first(); con; con = imports.next()) {
        if (con->getBaseType() == Uml::ot_Datatype)
            continue;
        QString pkg = con->getPackage();
        if (!pkg.isEmpty() && pkg != c->getPackage())
            d << "import " << pkg << "." << cleanName(con->getName()) << ";"
            << m_endl;
    }

    writeBlankLine(d);
}

void DWriter::writeClass(UMLClassifier *c) {
    if (!c) {
        kDebug()<<"Cannot write class of NULL concept!\n";
        return;
    }

    isInterface = c->isInterface();

    QString fileName = cleanName(c->getName().lower());

    //find an appropriate name for our file
    fileName = findFileName(c, ".d");
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

    // open text stream to file
    QTextStream d(&file);

    //try to find a heading file (license, coments, etc)
    QString str;
    str = getHeadingFile(".d");
    if(!str.isEmpty()) {
        str.replace(QRegExp("%filename%"),fileName);
        str.replace(QRegExp("%filepath%"),file.name());
        d<<str<<m_endl;
    }

    // source file begins with the module declaration
    writeModuleDecl(c, d);

    // imports
    writeModuleImports(c, d);

    // write the opening declaration for the class incl any documentation,
    // interfaces and/or inheritence issues we have
    writeClassDecl(c, d);

    // start body of class
    d << " {" << m_endl;


    // Preparations
    //

    // sort attributes by Scope
    UMLAttributeList  atl;
    UMLAttributeList  atpub, atprot, atpriv, atpkg, atexport;
    UMLAttributeList  final_atpub, final_atprot, final_atpriv, final_atpkg, final_atexport;

    atpub.setAutoDelete(false);
    final_atpub.setAutoDelete(false);
    atprot.setAutoDelete(false);
    final_atprot.setAutoDelete(false);
    atpriv.setAutoDelete(false);
    final_atpriv.setAutoDelete(false);
    atpkg.setAutoDelete(false);
    final_atpkg.setAutoDelete(false);
    atexport.setAutoDelete(false);
    final_atexport.setAutoDelete(false);

    if (!isInterface) {
        UMLAttributeList atl = c->getAttributeList();
        for (UMLAttribute *at = atl.first(); at ; at = atl.next()) {
            switch(at->getVisibility())
            {
                case Uml::Visibility::Public:
                    if(at->getStatic())
                        final_atpub.append(at);
                    else
                        atpub.append(at);
                    break;
                case Uml::Visibility::Protected:
                    if(at->getStatic())
                        final_atprot.append(at);
                    else
                        atprot.append(at);
                    break;
                case Uml::Visibility::Private:
                    if(at->getStatic())
                        final_atpriv.append(at);
                    else
                        atpriv.append(at);
                    break;/* TODO: requires support from the gui & other structures
                case Uml::Visibility::Package:
                    if(at->getStatic())
                    final_atpkg.append(at);
                    else
                    atpkg.append(at);
                    break;
                case Uml::Visibility::Export:
                    if(at->getStatic())
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
    UMLAssociationList associations = c->getSpecificAssocs(Uml::at_Association); // BAD! only way to get "general" associations.
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
        writeComment("", m_indentation, d);
        writeComment("Fields", m_indentation, d);
        writeComment("", m_indentation, d);
        writeBlankLine(d);
    }

    writeAttributeDecls(final_atpub, final_atprot, final_atpriv, d);
    writeAttributeDecls(atpub, atprot, atpriv, d);

    writeAssociationDecls(associations, c->getID(), d);
    writeAssociationDecls(uniAssociations, c->getID(), d);
    writeAssociationDecls(aggregations, c->getID(), d);
    writeAssociationDecls(compositions, c->getID(), d);

    //FIXME: find constructors and write them here

    // write constructors
    if(!isInterface) writeConstructor(c, d);


    // METHODS
    //

    // write comment for sub-section IF needed
    if (forceDoc() || hasAccessorMethods ) {
        writeComment("", m_indentation, d);
        writeComment("Accessors", m_indentation, d);
        writeComment("", m_indentation, d);
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
        writeComment("", m_indentation, d);
        writeComment("Other methods", m_indentation, d);
        writeComment("", m_indentation, d);
        writeBlankLine(d);
    }

    writeOperations(c, d);

    d << "}" << m_endl; // end class

    file.close();
    emit codeGenerated(c, true);
}

void DWriter::writeClassDecl(UMLClassifier *c, QTextStream &d) {

    // class documentation
    if (!c->getDoc().isEmpty()) {
        writeDocumentation("", c->getDoc(), "", "", d);
    }

    /*
     * Class declaration
     *
     * (private) class foo(T, ..., Z) : class1, ..., classN, interface1, ..., interfaceN
     *     a       b    c      d      e          f                        g
     */

    // (a) visibility modifier
    switch(c->getVisibility()) {
        case Uml::Visibility::Private: d << "private "; break;
        default: break;
    }

    // (b) keyword
    // TODO what about structs?
    if (isInterface) {
        d << "interface ";
    } else {
        if (c->getAbstract()) {
            d << "abstract ";
        }

        d << "class ";
    }

    // (c) class name
    QString classname = cleanName(c->getName()); // our class name
    d << classname;

    // (d) template parameters
    UMLTemplateList template_params = c->getTemplateList();
    if (template_params.count()) {
        d << "(";

        for (UMLTemplate *t = template_params.first(); t; ) {
            // TODO: hm, leaving the type blank results in "class"
            // so we omit it (also because "class" in this context is illegal)
            if (t->getTypeName() != "class") {
                d << t->getTypeName();
                d << " ";
            }

            d << t->getName();

            if ((t = template_params.next()) != NULL)
                d << ", ";
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
        for (UMLClassifier * concept= superclasses.first(); concept; concept = superclasses.next()) {
            d << cleanName(concept->getName());

            count--;

            if (count>0) d << ", ";
        }

        // (g) interfaces
        for (UMLClassifier * concept= superinterfaces.first(); concept; concept = superinterfaces.next()) {
            d << cleanName(concept->getName());

            count--;

            if (count>0) d << ", ";
        }
    }
}

void DWriter::writeProtectionMod(Uml::Visibility visibility, QTextStream &d) {
    d << m_indentation << scopeToDDecl(visibility) << ":" << m_endl << m_endl;
}

void DWriter::writeAttributeDecl(Uml::Visibility visibility, UMLAttributeList &atlist, QTextStream &d) {
    if (atlist.count()==0) return;

    writeProtectionMod(visibility, d);

    for(UMLAttribute *at=atlist.first(); at; at=atlist.next()) {
        // documentation
        if (!at->getDoc().isEmpty()) {
            writeComment(at->getDoc(), m_indentation, d, true);
        }

        d << m_indentation;

        // static attribute?
        if (at->getStatic()) d << "static ";

        // method return type
        d << fixTypeName(at->getTypeName()) << " ";

        // TODO: find out whether this class has accessors or not
        bool hasAccessorMethods = true;

        // attribute name
        if (hasAccessorMethods) {
            d << "m_";
        }
        d << cleanName(at->getName());

        // initial value
        QString initVal = fixInitialStringDeclValue(at->getInitialValue(), at->getTypeName());
        if (!initVal.isEmpty()) d << " = " << initVal;
        d << ";" << m_endl << m_endl;
    }
}

void DWriter::writeAttributeDecls(UMLAttributeList &atpub, UMLAttributeList &atprot,
                                     UMLAttributeList &atpriv, QTextStream &d ) {

    writeAttributeDecl(Uml::Visibility::Public, atpub, d);
    writeAttributeDecl(Uml::Visibility::Protected, atprot, d);
    writeAttributeDecl(Uml::Visibility::Private, atpriv, d);
    //TODO: export and package
}

void DWriter::writeAttributeMethods(UMLAttributeList &atpub, Uml::Visibility visibility, QTextStream &d) {
    if (atpub.count()==0) return;

    writeProtectionMod(visibility, d);

    for(UMLAttribute *at=atpub.first(); at; at=atpub.next()) {
        QString fieldName = cleanName(at->getName());
        writeSingleAttributeAccessorMethods(
            at->getTypeName(), "m_" + fieldName, fieldName, at->getDoc(),
            visibility, Uml::chg_Changeable, at->getStatic(), d);
    }
}

void DWriter::writeComment(const QString &comment, const QString &myIndent,
                           QTextStream &d, bool dDocStyle) {
    if(dDocStyle) {
        d << myIndent << "/**" << m_endl;
    }

    QStringList lines = QStringList::split("\n", comment);

    if (lines.count() == 0) lines << comment;

    for (uint i = 0; i < lines.count(); ++i) {
        QString tmp = lines[i];

        while (tmp.length() > 77) {
            uint l = tmp.left(77).findRev(' ');
            if (l < 1) l = tmp.find(' ', 77);
            if (l < 1 || l > tmp.length()) {
                d << myIndent << (dDocStyle ? " * " : "// ") << tmp << m_endl;
                break;
            }
            d << myIndent << (dDocStyle ? " * " : "// ") << tmp.left(l) << m_endl;
            tmp = tmp.right(tmp.length() - l);
        }

        d << myIndent << (dDocStyle ? " * " : "// ") << tmp << m_endl;
    }

    if(dDocStyle) {
        d << myIndent << " */" << m_endl;
    }
}

void DWriter::writeDocumentation(QString header, QString body, QString end, QString indent, QTextStream &d) {
    d << indent << "/**" << m_endl;
    if (!header.isEmpty())
        d << formatDoc(header, indent+" * ");
    if (!body.isEmpty())
        d << formatDoc(body, indent+" * ");
    if (!end.isEmpty())
    {
        QStringList lines = QStringList::split( "\n", end);
        for (uint i= 0; i < lines.count(); i++)
            d << formatDoc(lines[i], indent + " * ");
    }
    d<<indent<< " */" << m_endl;
}

void DWriter::writeAssociationDecls(UMLAssociationList associations, Uml::IDType id, QTextStream &d) {

    if( forceSections() || !associations.isEmpty() )
    {
        bool printRoleA = false, printRoleB = false;
        for(UMLAssociation *a = associations.first(); a; a = associations.next())
        {
            // it may seem counter intuitive, but you want to insert the role of the
            // *other* class into *this* class.
            if (a->getObjectId(Uml::A) == id)
                printRoleB = true;

            if (a->getObjectId(Uml::B) == id)
                printRoleA = true;

            // First: we insert documentaion for association IF it has either role AND some documentation (!)
            if ((printRoleA || printRoleB) && !(a->getDoc().isEmpty()))
                writeComment(a->getDoc(), m_indentation, d);

            // print RoleB decl
            if (printRoleB)
            {
                QString fieldClassName = cleanName(getUMLObjectName(a->getObject(Uml::B)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::B), a->getMulti(Uml::B), a->getRoleDoc(Uml::B), a->getVisibility(Uml::B), d);
            }

            // print RoleA decl
            if (printRoleA)
            {
                QString fieldClassName = cleanName(getUMLObjectName(a->getObject(Uml::A)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::A), a->getMulti(Uml::A), a->getRoleDoc(Uml::A), a->getVisibility(Uml::A), d);
            }
        }
    }
}

void DWriter::writeAssociationRoleDecl(QString fieldClassName,
        QString roleName, QString multi,
        QString doc, Uml::Visibility /*visib*/, QTextStream &d) {
    // ONLY write out IF there is a rolename given
    // otherwise its not meant to be declared in the code
    if (roleName.isEmpty()) return;

    if (!doc.isEmpty()) {
        writeComment(doc, m_indentation, d);
    }

    bool hasAccessors = true;

    // declare the association based on whether it is this a single variable
    // or a List (Vector). One day this will be done correctly with special
    // multiplicity object that we don't have to figure out what it means via regex.
    if(multi.isEmpty() || multi.contains(QRegExp("^[01]$"))) {
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

void DWriter::writeAssociationMethods (UMLAssociationList associations, UMLClassifier *thisClass, QTextStream &d) {
    if( forceSections() || !associations.isEmpty() ) {
        for(UMLAssociation *a = associations.first(); a; a = associations.next()) {
            // insert the methods to access the role of the other
            // class in the code of this one
            if (a->getObjectId(Uml::A) == thisClass->getID()) {
                // only write out IF there is a rolename given
                if(!a->getRoleName(Uml::B).isEmpty()) {
                    QString fieldClassName = getUMLObjectName(a->getObject(Uml::B));
                    writeAssociationRoleMethod(fieldClassName,
                                               a->getRoleName(Uml::B),
                                               a->getMulti(Uml::B), a->getRoleDoc(Uml::B),
                                               a->getVisibility(Uml::B),
                                               a->getChangeability(Uml::B), d);
                }
            }

            if (a->getObjectId(Uml::B) == thisClass->getID()) {
                // only write out IF there is a rolename given
                if(!a->getRoleName(Uml::A).isEmpty()) {
                    QString fieldClassName = getUMLObjectName(a->getObject(Uml::A));
                    writeAssociationRoleMethod(fieldClassName, a->getRoleName(Uml::A),
                                               a->getMulti(Uml::A),
                                               a->getRoleDoc(Uml::A),
                                               a->getVisibility(Uml::A),
                                               a->getChangeability(Uml::A),
                                               d);
                }
            }
        }
    }
}

void DWriter::writeAssociationRoleMethod (QString fieldClassName, QString roleName, QString multi,
        QString description, Uml::Visibility visib, Uml::Changeability_Type change,
        QTextStream &d) {
    if(multi.isEmpty() || multi.contains(QRegExp("^[01]$"))) {
        QString fieldVarName = "m_" + deCapitaliseFirstLetter(roleName);

        writeSingleAttributeAccessorMethods(
            fieldClassName, fieldVarName, roleName, description, visib, change, false, d);
    } else {
        QString fieldVarName = "m_" + pluralize(deCapitaliseFirstLetter(roleName));

        writeVectorAttributeAccessorMethods(
            fieldClassName, fieldVarName, pluralize(roleName), description, visib, change, d);
    }
}

void DWriter::writeVectorAttributeAccessorMethods (QString fieldClassName, QString fieldVarName,
        QString fieldName, QString description,
        Uml::Visibility /*visibility*/, Uml::Changeability_Type changeType,
        QTextStream &d) {

    fieldClassName = fixTypeName(fieldClassName);
    QString fieldNameUP = unPluralize(fieldName);
    QString fieldNameUC = Codegen_Utils::capitalizeFirstLetter(fieldNameUP);

    // ONLY IF changeability is NOT Frozen
    if (changeType != Uml::chg_Frozen) {
        writeDocumentation("Adds a " + fieldNameUP + " to the list of " +
                           fieldName + '.', description, "", m_indentation, d);

        d << m_indentation << "void add" << fieldNameUC << "(";
        d << fieldClassName << " new" << fieldNameUC << ") {";
        d << startline << m_indentation << fieldVarName << " ~= new" << fieldNameUC << ";";
        d << startline << "}" << m_endl << m_endl;
    }

    // ONLY IF changeability is Changeable
    if (changeType == Uml::chg_Changeable) {
        writeDocumentation("Removes a " + fieldNameUP + " from the list of " +
                           fieldName + '.', description, "", m_indentation, d);

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
    writeDocumentation("Returns the list of " + fieldName + '.',
                       description, "@return List of " + fieldName + '.',
                       m_indentation, d);

    d << m_indentation << fieldClassName << "[] get" << fieldName << "() {";
    d << startline << m_indentation << "return " << fieldVarName << ";";
    d << startline << "}" << m_endl << m_endl;
}


void DWriter::writeSingleAttributeAccessorMethods(QString fieldClassName,
     QString fieldVarName, QString fieldName, QString description, Uml::Visibility /*visibility*/,
     Uml::Changeability_Type change, bool isFinal, QTextStream &d) {

    fieldClassName = fixTypeName(fieldClassName);
    QString fieldNameUC = Codegen_Utils::capitalizeFirstLetter(fieldName);
    if (fieldName.left(2) == "m_") fieldName = fieldName.right(fieldName.length()-2);

    // set method
    if (change == Uml::chg_Changeable && !isFinal) {
        writeDocumentation("Sets the value of " + fieldName + '.', description,
                           "@param new" + fieldNameUC + " The new value of " + fieldName + '.',
                           m_indentation, d);

        d << m_indentation << fieldClassName << " " << fieldName << "(";
        d << fieldClassName << " new" << fieldNameUC << ") {";
        d << startline << m_indentation << "return " << fieldVarName << " = new" << fieldNameUC << ";";
        d << startline << "}" << m_endl << m_endl;
    }

    // get method
    writeDocumentation("Returns the value of " + fieldName + '.', description,
                       "@return The value of " + fieldName + '.',
                       m_indentation, d);

    d << m_indentation << fieldClassName << " " << fieldName << "() {";
    d << startline << m_indentation << "return " << fieldVarName << ";";
    d << startline << "}" << m_endl << m_endl;
}

void DWriter::writeConstructor(UMLClassifier *c, QTextStream &d) {

    if (forceDoc())
    {
        d<<startline;
        writeComment("", m_indentation, d);
        writeComment("Constructors", m_indentation, d);
        writeComment("", m_indentation, d);
        writeBlankLine(d);
    }

    // write the first constructor
    QString className = cleanName(c->getName());
    d << m_indentation << "public this("<<") { }";

}

// IF the type is "string" we need to declare it as
// the D Object "String" (there is no string primative in D).
// Same thing again for "bool" to "boolean"
QString DWriter::fixTypeName(const QString& string) {
    if (string.isEmpty())
        return "void";
    if (string == "string")
        return "char[]";
    if (string == "unsigned short")
        return "ushort";
    if (string == "unsigned int")
        return "uint";
    if (string == "unsigned long")
        return "ulong";
    return string;
}

QStringList DWriter::defaultDatatypes() {
    QStringList l;
    l << "void"
    << "bool"
    << "byte"
    << "ubyte"
    << "short"
    << "ushort"
    << "int"
    << "uint"
    << "long"
    << "ulong"
    << "cent"
    << "ucent"
    << "float"
    << "double"
    << "real"
    << "ifloat"
    << "idouble"
    << "ireal"
    << "cfloat"
    << "cdouble"
    << "creal"
    << "char"
    << "wchar"
    << "dchar";
    return l;
}


bool DWriter::compareDMethod(UMLOperation *op1, UMLOperation *op2) {
    if (op1 == NULL || op2 == NULL)
        return false;
    if (op1 == op2)
        return true;
    if (op1->getName() != op2->getName())
        return false;
    UMLAttributeList atl1 = op1->getParmList();
    UMLAttributeList atl2 = op2->getParmList();
    if (atl1.count() != atl2.count())
        return false;
    UMLAttribute *at1;
    UMLAttribute *at2;
    for (at1  = atl1.first(), at2 = atl2.first(); at1 && at2 ; at1 = atl1.next(),at2 = atl2.next())
    {
        if (at1->getTypeName() != at2->getTypeName())
            return false;
    }
    return true;

}

bool DWriter::dMethodInList(UMLOperation *umlOp, UMLOperationList &opl) {
    for (UMLOperation *op = opl.first(); op; op = opl.next()) {
        if (DWriter::compareDMethod(op, umlOp)) {
            return true;
        }
    }
    return false;
}

void DWriter::getSuperImplementedOperations(UMLClassifier *c, UMLOperationList &yetImplementedOpList ,UMLOperationList &toBeImplementedOpList, bool noClassInPath) {
    UMLClassifierList superClasses = c->findSuperClassConcepts();

    for (UMLClassifier *concept= superClasses.first(); concept; concept = superClasses.next())
    {
        getSuperImplementedOperations(concept, yetImplementedOpList, toBeImplementedOpList, (concept->isInterface() && noClassInPath));
        UMLOperationList opl = concept->getOpList();
        for (UMLOperation *op = opl.first(); op; op = opl.next()) {
            if (concept->isInterface() && noClassInPath) {
                if (!DWriter::dMethodInList(op,toBeImplementedOpList))
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

void DWriter::getInterfacesOperationsToBeImplemented(UMLClassifier *c, UMLOperationList &opList ) {
    UMLOperationList yetImplementedOpList;
    UMLOperationList toBeImplementedOpList;

    getSuperImplementedOperations(c,yetImplementedOpList, toBeImplementedOpList);
    for (UMLOperation *op = toBeImplementedOpList.first(); op; op = toBeImplementedOpList.next())
    {
        if ( ! DWriter::dMethodInList(op, yetImplementedOpList) && ! DWriter::dMethodInList(op, opList) )
            opList.append(op);
    }
}

void DWriter::writeOperations(UMLClassifier *c, QTextStream &d) {
    UMLOperationList opl;
    UMLOperationList oppub,opprot,oppriv;
    oppub.setAutoDelete(false);
    opprot.setAutoDelete(false);
    oppriv.setAutoDelete(false);

    //sort operations by scope first and see if there are abstract methods
    opl = c->getOpList();
    if (! c->isInterface()) {
        getInterfacesOperationsToBeImplemented(c, opl);
    }
    for (UMLOperation *op = opl.first(); op; op = opl.next()) {
        switch(op->getVisibility()) {
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
      if(forceSections() || oppub.count())
      {
      writeComment("public operations",m_indentation,d);
        writeBlankLine(d);
      }
    */

    if (oppub.count() > 0) {
        writeProtectionMod(Uml::Visibility::Public, d);

        writeOperations(oppub,d);
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

void DWriter::writeOperations(UMLOperationList &oplist, QTextStream &d) {
    UMLAttributeList atl;
    QString str;

    // generate method decl for each operation given
    for(UMLOperation *op=oplist.first(); op; op=oplist.next()) {
        QString returnStr = "";
        // write documentation

        QString methodReturnType = fixTypeName(op->getTypeName());

        //TODO: return type comment
        if(methodReturnType != "void") {
            returnStr += "@return " + methodReturnType + m_endl;
        }

        str = ""; // reset for next method
        if (op->getAbstract() && !isInterface) str += "abstract ";
        if (op->getStatic()) str += "static ";

        str += methodReturnType + ' ' +cleanName(op->getName()) + '(';

        atl = op->getParmList();
        int i = atl.count();
        int j = 0;
        for (UMLAttribute *at = atl.first(); at; at = atl.next(), j++) {
            QString typeName = fixTypeName(at->getTypeName());
            QString atName = cleanName(at->getName());
            str += typeName + ' ' + atName +
                   (!(at->getInitialValue().isEmpty()) ?
                    (QString(" = ")+at->getInitialValue()) :
                    QString(""))
                   + ((j < i-1)?", ":"");
            returnStr += "@param " + atName+' '+at->getDoc() + m_endl;
        }

        str+= ')';

        // method only gets a body IF its not abstract
        if (op->getAbstract() || isInterface)
            str += ';'; // terminate now
        else
            str += startline + '{' + startline + '}'; // empty method body

        // write it out
        writeDocumentation("", op->getDoc(), returnStr, m_indentation, d);
        d << m_indentation << str << m_endl << m_endl;
    }
}

QString DWriter::fixInitialStringDeclValue(QString value, QString type) {
    // check for strings only
    if (!value.isEmpty() && type == "String") {
        if (!value.startsWith("\""))
            value.prepend("\"");
        if (!value.endsWith("\""))
            value.append("\"");
    }
    return value;
}

QString DWriter::scopeToDDecl(Uml::Visibility scope) {
    QString scopeString;

    switch(scope) {
        case Uml::Visibility::Public: scopeString = "public"; break;
        case Uml::Visibility::Protected: scopeString = "protected"; break;
        case Uml::Visibility::Private: scopeString = "private"; break;
        default: break; //TODO: package and export
    }

    return scopeString;
}

// methods like this _shouldn't_ be needed IF we properly did things thruought the code.
QString DWriter::getUMLObjectName(UMLObject *obj) {
    return(obj!=0)?obj->getName():QString("NULL");
}

QString DWriter::deCapitaliseFirstLetter(QString string) {
    string.replace( 0, 1, string[0].lower());
    return string;
}

QString DWriter::pluralize(QString string) {
    return string + (string.right(1) == "s" ? "es" : "s");
}

QString DWriter::unPluralize(QString string) {
    // does not handle special cases liek datum -> data, etc.

    if (string.length() > 2 && string.right(3) == "ses") {
        return string.left(string.length() - 2);
    }

    if (string.right(1) == "s") {
        return string.left(string.length() - 1);
    }

    return string;
}

void DWriter::writeBlankLine(QTextStream &d) {
    d << m_endl;
}

