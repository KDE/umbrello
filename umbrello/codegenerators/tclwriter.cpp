/***************************************************************************
    begin               : Thu Jul 26 2005
    copyright           : (C) 2005 by Rene Meyer
    email               : rene.meyer@sturmit.de
      (C) 2006  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                       *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                       *
 ***************************************************************************/

// own header
#include "tclwriter.h"
// qt/kde includes
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "classifierinfo.h"
#include "codegen_utils.h"
#include "../umldoc.h"
#include "../classifier.h"
#include "../operation.h"
#include "../template.h"
#include "../umltemplatelist.h"
#include "../umlclassifierlistitemlist.h"
#include "../classifierlistitem.h"
#include "../model_utils.h"

static const char *tclwords[] = {
                                 "body",
                                 "break",
                                 "case",
                                 "class",
                                 "common",
                                 "concat",
                                 "configbody",
                                 "constructor",
                                 "continue",
                                 "default",
                                 "destructor",
                                 "else",
                                 "elseif",
                                 "for",
                                 "foreach",
                                 "global",
                                 "if",
                                 "incr",
                                 "lappend",
                                 "lindex",
                                 "list",
                                 "llength",
                                 "load",
                                 "lrange",
                                 "lreplace",
                                 "method",
                                 "namespace",
                                 "private",
                                 "proc",
                                 "protected",
                                 "public",
                                 "return",
                                 "set",
                                 "source",
                                 "switch",
                                 "then",
                                 "upvar",
                                 "variable",
                                 "virtual",
                                 "while",
                                 0
                             };

TclWriter::TclWriter()
{
}

TclWriter::~TclWriter()
{
}

Uml::Programming_Language
TclWriter::getLanguage()
{
    return Uml::pl_Tcl;
}

void
TclWriter::writeClass(UMLClassifier * c)
{

    if (!c) {
        kDebug() << "Cannot write class of NULL concept!\n";
        return;
    }
    QFile           fileh, filetcl;

    // find an appropriate name for our file
    QString         fileName = findFileName(c, ".tcl");
    if (fileName.isEmpty()) {
        emit            codeGenerated(c, false);
        return;
    }

    if (!openFile(fileh, fileName)) {
        emit            codeGenerated(c, false);
        return;
    }
    // preparations
    classifierInfo = new ClassifierInfo(c);
    classifierInfo->fileName = fileName;
    classifierInfo->className = cleanName(c->getName());
    mClass = cleanName(c->getName());
    if (!c->getPackage().isEmpty()) {
        mNamespace = "::" + cleanName(c->getPackage());
        mClassGlobal = mNamespace + "::" + mClass;
    } else {
        mNamespace = "::";
        mClassGlobal = "::" + mClass;
    }

    // write Header file
    writeHeaderFile(c, fileh);
    fileh.close();

    // Determine whether the implementation file is required.
    // (It is not required if the class is an enumeration.)
    bool            need_impl = true;
    if (!classifierInfo->isInterface) {
        if (c->getBaseType() == Uml::ot_Enum)
            need_impl = false;
    }
    if (need_impl) {
        if (!openFile(filetcl, fileName + "body")) {
            emit            codeGenerated(c, false);
            return;
        }
        // write Source file
        writeSourceFile(c, filetcl);
        filetcl.close();
    }
    // Wrap up: free classifierInfo, emit done code
    classifierInfo = 0;

    emit            codeGenerated(c, true);

}

void
TclWriter::writeHeaderFile(UMLClassifier * c, QFile & fileh)
{
    // open stream for writing
    QTextStream     stream(&fileh);
    mStream = &stream;

    // reset the indent level
    m_indentLevel = 0;

    // write header blurb
    QString         str = getHeadingFile(".tcl");
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), classifierInfo->fileName);
        str.replace(QRegExp("%filepath%"), fileh.name());
        writeCode(str);
    }
    // set current namespace
    writeCode("namespace eval " + mNamespace + " {");
    m_indentLevel++;

    // check on already existing
    writeComm("Do not load twice");
    writeCode("if {[namespace exist " + mClass + "]} return");

    // source used superclass files
    UMLClassifierList superclasses = classifierInfo->superclasses;
    if (superclasses.count() > 0) {
        writeComm
        ("Source found and used class files and import class command if necessary");

        for (UMLClassifier * classifier = superclasses.first(); classifier;
                classifier = superclasses.next()) {
            writeUse(classifier);
        }
    }
    // write all "source" we need to include other classes, that arent us.
    if (classifierInfo->hasAssociations) {
        writeAssociationIncl(classifierInfo->plainAssociations, c->getID(),
                             "Associations");
        writeAssociationIncl(classifierInfo->aggregations, c->getID(),
                             "Aggregations");
        writeAssociationIncl(classifierInfo->compositions, c->getID(),
                             "Compositions");
    }
    //Write class Documentation
    writeDocu("\n@class\t" + mClass + m_endl + c->getDoc());

    //check if class is abstract and / or has abstract methods
    if ((c->getAbstract() || classifierInfo->isInterface)
            && !hasAbstractOps(c)) {
        writeComm("TODO abstract class" + mClass +
                  "\nInherit from it and create only objects from the derived classes");
    }
    // check on enum classes
    if (!classifierInfo->isInterface) {
        // use tcl-list for enum's
        if (c->getBaseType() == Uml::ot_Enum) {
            UMLClassifierListItemList litList =
                c->getFilteredList(Uml::ot_EnumLiteral);
            writeCode("set enum_" + mClass + " [list\\");
            m_indentLevel++;
            for (UMLClassifierListItem * lit = litList.first(); lit;
                    lit = litList.next()) {
                QString         enumLiteral = cleanName(lit->getName());
                writeCode(enumLiteral + "\\");
            }
            m_indentLevel--;
            writeCode("];# end of enum");
            m_indentLevel--;
            writeCode("};# end of namespace");
            return;
        }
    }
    // Generate template parameters.
    UMLTemplateList template_params = c->getTemplateList();
    if (template_params.count()) {
        writeCode("#TODO template<");
        for (UMLTemplate * t = template_params.first(); t;
                           t = template_params.next()) {
            QString         formalName = t->getName();
            QString         typeName = t->getTypeName();
            writeCode(typeName + "# " + formalName);
        }
    }
    // start my own class
    writeCode("class " + mClass + " {");
    m_indentLevel++;
    if (classifierInfo->superclasses.count() > 0) {
        QString         code = "inherit";
        for (UMLClassifier * superClass = classifierInfo->superclasses.first();
                superClass; superClass = classifierInfo->superclasses.next()) {
            /*
            if (superClass->getAbstract() || superClass->isInterface())
                stream << getIndent() << "virtual ";
            */
            if (superClass->getPackage().isEmpty()) {
                code += " ::" + cleanName(superClass->getName());
            } else {
                code +=
                    " ::" + cleanName(superClass->getPackage()) + "::" +
                    cleanName(superClass->getName());
            }
        }
        writeCode(code);
    }
    //
    //declarations of operations
    //
    // write out field and operations decl grouped by visibility
    //

    // PUBLIC attribs/methods
    // for public: constructors are first ops we print out
    if (!classifierInfo->isInterface) {
        writeConstructorHeader();
        writeDestructorHeader();
    }
    // attributes
    writeAttributeDecl(Uml::Visibility::Public, true);      // write static attributes first
    writeAttributeDecl(Uml::Visibility::Public, false);
    // associations
    writeAssociationDecl(classifierInfo->plainAssociations, Uml::Visibility::Public,
                         c->getID(), "Associations");
    writeAssociationDecl(classifierInfo->aggregations, Uml::Visibility::Public, c->getID(),
                         "Aggregations");
    writeAssociationDecl(classifierInfo->compositions, Uml::Visibility::Public, c->getID(),
                         "Compositions");
    //TODO  writeHeaderAccessorMethodDecl(c, Uml::Visibility::Public, stream);
    writeOperationHeader(c, Uml::Visibility::Public);

    // PROTECTED attribs/methods
    //
    // attributes
    writeAttributeDecl(Uml::Visibility::Protected, true);   // write static attributes first
    writeAttributeDecl(Uml::Visibility::Protected, false);
    // associations
    writeAssociationDecl(classifierInfo->plainAssociations, Uml::Visibility::Protected,
                         c->getID(), "Association");
    writeAssociationDecl(classifierInfo->aggregations, Uml::Visibility::Protected,
                         c->getID(), "Aggregation");
    writeAssociationDecl(classifierInfo->compositions, Uml::Visibility::Protected,
                         c->getID(), "Composition");
    //TODO  writeHeaderAccessorMethodDecl(c, Uml::Visibility::Protected, stream);
    writeOperationHeader(c, Uml::Visibility::Protected);

    // PRIVATE attribs/methods
    //
    // attributes
    writeAttributeDecl(Uml::Visibility::Private, true);     // write static attributes first
    writeAttributeDecl(Uml::Visibility::Private, false);
    // associations
    writeAssociationDecl(classifierInfo->plainAssociations, Uml::Visibility::Private,
                         c->getID(), "Associations");
    writeAssociationDecl(classifierInfo->aggregations, Uml::Visibility::Private, c->getID(),
                         "Aggregations");
    writeAssociationDecl(classifierInfo->compositions, Uml::Visibility::Private, c->getID(),
                         "Compositions");
    //TODO  writeHeaderAccessorMethodDecl(c, Uml::Visibility::Public, stream);
    writeOperationHeader(c, Uml::Visibility::Private);
    writeInitAttributeHeader(); // this is always private, used by constructors to initialize class

    // end of class header
    m_indentLevel--;
    writeCode("};# end of class");

    // end of class namespace, if any
    m_indentLevel--;
    writeCode("};# end of namespace");
}

void
TclWriter::writeSourceFile(UMLClassifier * c, QFile & filetcl)
{
    // open stream for writing
    QTextStream     stream(&filetcl);
    mStream = &stream;

    // set the starting indentation at zero
    m_indentLevel = 0;

    //try to find a heading file (license, coments, etc)
    QString         str;
    str = getHeadingFile(".tclbody");
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), classifierInfo->fileName + "body");
        str.replace(QRegExp("%filepath%"), filetcl.name());
        writeCode(str);
    }
    // Start body of class

    // constructors are first ops we print out
    if (!classifierInfo->isInterface) {
        writeConstructorSource();
        writeDestructorSource();
    }
    // Public attributes have in tcl a configbody method
    writeAttributeSource();
    // Association access functions
    writeAssociationSource(classifierInfo->plainAssociations, c->getID());
    writeAssociationSource(classifierInfo->aggregations, c->getID());
    writeAssociationSource(classifierInfo->compositions, c->getID());
    // Procedures and methods
    writeOperationSource(c, Uml::Visibility::Public);
    writeOperationSource(c, Uml::Visibility::Protected);
    writeOperationSource(c, Uml::Visibility::Private);
    // Yep, bringing up the back of the bus, our initialization method for attributes
    writeInitAttributeSource();
}

void
TclWriter::writeCode(const QString &text)
{
    *mStream << getIndent() << text << m_endl;
}

void
TclWriter::writeComm(const QString &text)
{
    QStringList     lines = QStringList::split("\n", text, true);
    for (uint i = 0; i < lines.count(); i++) {
        *mStream << getIndent() << "# " << lines[i] << m_endl;
    }
}

void
TclWriter::writeDocu(const QString &text)
{
    QStringList     lines = QStringList::split("\n", text, true);
    for (uint i = 0; i < lines.count(); i++) {
        *mStream << getIndent() << "## " << lines[i] << m_endl;
    }
}

// To prevent circular including when both classifiers on either end
// of an association have roles we need to have forward declaration of
// the other class...but only IF its not THIS class (as could happen
// in self-association relationship).
void
TclWriter::writeAssociationIncl(UMLAssociationList list, Uml::IDType myId,
                                const QString &type)
{
    for (UMLAssociation * a = list.first(); a; a = list.next()) {
        UMLClassifier  *classifier = NULL;

        writeComm(m_endl + type + m_endl + a->toString() + m_endl + a->getDoc());
        // only use OTHER classes (e.g. we don't need to write includes for ourselves!!
        // AND only IF the roleName is defined, otherwise, its not meant to be noticed.
        if (a->getObjectId(Uml::A) == myId && !a->getRoleName(Uml::B).isEmpty()) {
            classifier = dynamic_cast < UMLClassifier * >(a->getObject(Uml::B));
            writeUse(classifier);
        } else if (a->getObjectId(Uml::B) == myId
                   && !a->getRoleName(Uml::A).isEmpty()) {
            classifier = dynamic_cast < UMLClassifier * >(a->getObject(Uml::A));
            if (classifier->getPackage().isEmpty())
                writeCode("namespace eval " + cleanName(classifier->getName()) +
                          " {}");
        } else {
            // CHECK: This crashes (classifier still NULL from above)
            /*
            writeCode("namespace eval " + cleanName(classifier->getPackage()) +
                      "::" + cleanName(classifier->getName()) + " {}");
             */
        }
    }
}

void
TclWriter::writeUse(UMLClassifier * c)
{
    QString         myNs;

    if (!c->getPackage().isEmpty()) {
        myNs = cleanName(c->getPackage());
    } else {
        myNs = "";
    }
    // if different package
    if (("::"+myNs) != mNamespace) {
        if (c->getPackage().isEmpty()) {
            writeCode("source " + findFileName(c, ".tcl"));
            writeCode("namespace import ::" + cleanName(c->getName()));
        } else {
            writeCode("package require " + myNs);
            writeCode("namespace import ::" + myNs + "::" +
                      cleanName(c->getName()));
        }
    } else {
        // source the file
        writeCode("source " + findFileName(c, ".tcl"));
    }
}

void
TclWriter::writeConstructorHeader()
{

    writeDocu
    (m_endl + "@func constructor" + m_endl +
     "@par args contain all configuration parameters" + m_endl);

    writeCode("constructor {args} {}" + m_endl);
}

void
TclWriter::writeConstructorSource()
{
    writeComm(mClassGlobal + "::constructor");
    writeCode(mClassGlobal + "::constructor {args} {");
    m_indentLevel++;
    if (classifierInfo->hasAttributes) {
        writeCode("initAttributes");
    }
    writeCode("eval configure $args");
    m_indentLevel--;
    writeCode('}' + m_endl);
}

void
TclWriter::writeDestructorHeader()
{

    writeDocu(m_endl + "@func destructor" + m_endl);

    writeCode("destructor {} {}");
}

void
TclWriter::writeDestructorSource()
{
    writeComm(mClassGlobal + "::destructor");
    writeCode(mClassGlobal + "::destructor {} {" + m_endl + '}' + m_endl);
}

void
TclWriter::writeAttributeDecl(Uml::Visibility visibility, bool writeStatic)
{
    if (classifierInfo->isInterface)
        return;

    QString scope = visibility.toString();
    QString type;
    if (writeStatic) {
        type = "common";
    } else {
        type = "variable";
    }
    UMLAttributeList *list = NULL;
    switch (visibility) {
    case Uml::Visibility::Private:
        if (writeStatic) {
            list = &(classifierInfo->static_atpriv);
        } else {
            list = &(classifierInfo->atpriv);
        }
        break;

    case Uml::Visibility::Protected:
        if (writeStatic) {
            list = &(classifierInfo->static_atprot);
        } else {
            list = &(classifierInfo->atprot);
        }
        break;

    case Uml::Visibility::Public:
        if (writeStatic) {
            list = &(classifierInfo->static_atpub);
        } else {
            list = &(classifierInfo->atpub);
        }
        break;
    default:
        break;
    }

    if (list && list->count() > 0) {
        writeComm(m_endl + scope + ' ' + type + " attributes" + m_endl);
        // write attrib declarations now
        QString         documentation;
        for (UMLAttribute * at = list->first(); at; at = list->next()) {
            documentation = at->getDoc();
            QString         varName = cleanName(at->getName());
            QString         typeName = fixTypeName(at->getTypeName());
            writeDocu(m_endl + "@var " + scope + ' ' + type + ' ' + typeName + ' ' +
                      varName + m_endl + documentation);
            writeCode(scope + ' ' + type + ' ' + varName + m_endl);
        }
    }
}

void
TclWriter::writeAssociationDecl(UMLAssociationList associations,
                                Uml::Visibility permitScope, Uml::IDType id,
                                const QString &/*type*/)
{
    if (forceSections() || !associations.isEmpty()) {
        bool            printRoleA = false, printRoleB = false;
        for (UMLAssociation * a = associations.first(); a;
                a = associations.next()) {

            // it may seem counter intuitive, but you want to insert the role of the
            // *other* class into *this* class.
            if (a->getObjectId(Uml::A) == id && !a->getRoleName(Uml::B).isEmpty())
                printRoleB = true;

            if (a->getObjectId(Uml::B) == id && !a->getRoleName(Uml::A).isEmpty())
                printRoleA = true;

            // First: we insert documentaion for association IF it has either role AND some documentation (!)
            // print RoleB decl
            if (printRoleB && a->getVisibility(Uml::B) == permitScope) {

                QString         fieldClassName =
                    cleanName(getUMLObjectName(a->getObject(Uml::B)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::B),
                                         a->getMulti(Uml::B), a->getRoleDoc(Uml::B),
                                         permitScope.toString());
            }
            // print RoleA decl
            if (printRoleA && a->getVisibility(Uml::A) == permitScope) {
                QString         fieldClassName =
                    cleanName(getUMLObjectName(a->getObject(Uml::A)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::A),
                                         a->getMulti(Uml::A), a->getRoleDoc(Uml::A),
                                         permitScope.toString());
            }
            // reset for next association in our loop
            printRoleA = false;
            printRoleB = false;
        }
    }
}

void
TclWriter::writeAssociationRoleDecl(const QString &fieldClassName, const QString &roleName,
                                    const QString &multi, const QString &doc, const QString &scope)
{
    // ONLY write out IF there is a rolename given
    // otherwise its not meant to be declared in the code
    if (roleName.isEmpty())
        return;


    // declare the association based on whether it is this a single variable
    // or a List (Vector). One day this will be done correctly with special
    // multiplicity object that we don't have to figure out what it means via regex.
    if (multi.isEmpty() || multi.contains(QRegExp("^[01]$"))) {
        QString         fieldVarName = roleName.lower();

        // record this for later consideration of initialization IF the
        // multi value requires 1 of these objects
        if (ObjectFieldVariables.findIndex(fieldVarName) == -1 &&
                multi.contains(QRegExp("^1$"))
           ) {
            // ugh. UGLY. Storing variable name and its class in pairs.
            ObjectFieldVariables.append(fieldVarName);
            ObjectFieldVariables.append(fieldClassName);
        }
        writeDocu(m_endl + "@var " + scope + " variable <" + fieldClassName +
                  "> " + fieldVarName + m_endl + doc);
        writeCode(scope + " variable " + fieldVarName + m_endl);
    } else {
        QString         fieldVarName = roleName.lower();

        // record unique occurrences for later when we want to check
        // for initialization of this vector
        if (VectorFieldVariables.findIndex(fieldVarName) == -1)
            VectorFieldVariables.append(fieldVarName);
        writeDocu(m_endl + "@var" + scope + " variable <" + fieldClassName +
                  "*> " + fieldVarName + m_endl + doc);
        writeCode(scope + " variable " + fieldVarName + m_endl);
    }
}

void
TclWriter::writeInitAttributeHeader()
{
    if (classifierInfo->hasAttributes) {
        writeDocu("@method private initAttributes" + m_endl +
                  "Initialize all internal variables");
        writeCode("private method initAttributes {}");
    }
}

void
TclWriter::writeInitAttributeSource()
{
    // only need to do this under certain conditions
    if (classifierInfo->hasAttributes) {
        QString         varName;

        writeComm(mClassGlobal + "::initAttributes");
        writeCode("body " + mClassGlobal + "::initAttributes {} {");
        m_indentLevel++;

        // first, initiation of fields derived from attributes
        UMLAttributeList atl = classifierInfo->getAttList();
        for (UMLAttribute * at = atl.first(); at; at = atl.next()) {
            if (!at->getInitialValue().isEmpty()) {
                varName = cleanName(at->getName());
                writeCode("set " + varName + ' ' + at->getInitialValue());
            }
        }
        // Now initialize the association related fields (e.g. vectors)
        QStringList::Iterator it;
        for (it = VectorFieldVariables.begin();
                it != VectorFieldVariables.end(); ++it) {
            varName = *it;
            writeCode("set " + varName + " [list]");
        }

        for (it = ObjectFieldVariables.begin();
                it != ObjectFieldVariables.end(); ++it) {
            varName = *it;
            it++;
            QString         fieldClassName = *it;
            writeCode("set " + varName + " [list]");
        }
        // clean up
        ObjectFieldVariables.clear();   // shouldn't be needed?
        VectorFieldVariables.clear();   // shouldn't be needed?

        m_indentLevel--;
        writeCode('}' + m_endl);
    }
}

void
TclWriter::writeOperationHeader(UMLClassifier * c, Uml::Visibility permitScope)
{

    UMLOperationList oplist;
    UMLOperation   *op;
    UMLAttribute   *at;
    int             j;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList inputlist = c->getOpList();
    for (UMLOperation * op = inputlist.first(); op; op = inputlist.next()) {
        switch (op->getVisibility()) {
        case Uml::Visibility::Public:
            if (permitScope == Uml::Visibility::Public)
                oplist.append(op);
            break;
        case Uml::Visibility::Protected:
            if (permitScope == Uml::Visibility::Protected)
                oplist.append(op);
            break;
        case Uml::Visibility::Private:
            if (permitScope == Uml::Visibility::Private)
                oplist.append(op);
            break;
        default:
            break;
        }
    }

    // generate method decl for each operation given
    if (oplist.count() > 0) {
        writeComm("Operations");
    }
    for (op = oplist.first(); op; op = oplist.next()) {
        QString         doc = "";
        QString         code = "";
        QString         methodReturnType = fixTypeName(op->getTypeName());
        QString         name = cleanName(op->getName());
        QString         scope = permitScope.toString();
        if (op->getAbstract() || classifierInfo->isInterface) {
            //TODO declare abstract method as 'virtual'
            // str += "virtual ";
        }
        // declaration for header file
        if (op->getStatic()) {
            doc = m_endl + "@fn " + scope + " proc " + name + m_endl;
            code = scope + " proc " + name + " {";
        } else {
            doc = m_endl + "@fn " + scope + " method " + name + m_endl;
            code = scope + " method " + name + " {";
        }
        // method parameters
        UMLAttributeList atl = op->getParmList();
        j = 0;
        for (at = atl.first(); at; at = atl.next(), j++) {
            QString         typeName = fixTypeName(at->getTypeName());
            QString         atName = cleanName(at->getName());
            if (at->getInitialValue().isEmpty()) {
                doc +=
                    "@param " + typeName + ' ' + atName + m_endl + at->getDoc() +
                    m_endl;
                code += ' ' + atName;
            } else {
                doc +=
                    "@param " + typeName + ' ' + atName + " (default=" +
                    at->getInitialValue() + ") " + m_endl + at->getDoc() + m_endl;
                code += " {" + atName + ' ' + at->getInitialValue() + "} ";
            }
        }
        if (methodReturnType != "void") {
            doc += "@return     " + methodReturnType + m_endl;
        }
        writeDocu(doc + op->getDoc());
        writeCode(code + "} {}" + m_endl);
    }
}

void
TclWriter::writeOperationSource(UMLClassifier * c, Uml::Visibility permitScope)
{

    UMLOperationList oplist;
    UMLOperation   *op;
    UMLAttribute   *at;
    int             j;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList inputlist = c->getOpList();
    for (UMLOperation * op = inputlist.first(); op; op = inputlist.next()) {
        switch (op->getVisibility()) {
        case Uml::Visibility::Public:
            if (permitScope == Uml::Visibility::Public)
                oplist.append(op);
            break;
        case Uml::Visibility::Protected:
            if (permitScope == Uml::Visibility::Protected)
                oplist.append(op);
            break;
        case Uml::Visibility::Private:
            if (permitScope == Uml::Visibility::Private)
                oplist.append(op);
            break;
        default:
            break;
        }
    }

    // generate source for each operation given
    for (op = oplist.first(); op; op = oplist.next()) {
        QString         code = "";
        QString         methodReturnType = fixTypeName(op->getTypeName());
        QString         name;
        // no code needed
        if (op->getAbstract() || classifierInfo->isInterface) {
            continue;
        }
        name = mClassGlobal + "::" + cleanName(op->getName());
        writeComm(name);
        code = "body " + name + " {";
        // parameters
        UMLAttributeList atl = op->getParmList();
        j = 0;
        for (at = atl.first(); at; at = atl.next(), j++) {
            QString         atName = cleanName(at->getName());
            if (at->getInitialValue().isEmpty()) {
                code += ' ' + atName;
            } else {
                code += " {" + atName + ' ' + at->getInitialValue() + "} ";
            }
        }
        writeCode(code += "} {");
        m_indentLevel++;
        if (methodReturnType != "void") {
            writeCode("return " + methodReturnType);
        } else {
            writeCode("return");
        }
        m_indentLevel--;
        writeCode('}' + m_endl);
    }
}

void
TclWriter::writeAttributeSource()
{
    UMLAttributeList *list = &(classifierInfo->atpub);
    UMLAttribute   *at;
    for (at = list->first(); at; at = list->next()) {
        QString         name = mClassGlobal + "::" + cleanName(at->getName());

        writeComm(name);
        writeCode("configbody " + name + " {} {" + m_endl + '}' + m_endl);
    }
}

void
TclWriter::writeAssociationSource(UMLAssociationList associations,
                                  Uml::IDType id)
{
    if (associations.isEmpty()) {
        return;
    }

    bool            printRoleA = false, printRoleB = false;
    for (UMLAssociation * a = associations.first(); a; a = associations.next()) {

        // it may seem counter intuitive, but you want to insert the role of the
        // *other* class into *this* class.
        if (a->getObjectId(Uml::A) == id && !a->getRoleName(Uml::B).isEmpty())
            printRoleB = true;

        if (a->getObjectId(Uml::B) == id && !a->getRoleName(Uml::A).isEmpty())
            printRoleA = true;

        // print RoleB source
        if (printRoleB && a->getVisibility(Uml::B) == Uml::Visibility::Public) {

            QString         fieldClassName =
                cleanName(getUMLObjectName(a->getObject(Uml::B)));
            writeAssociationRoleSource(fieldClassName, a->getRoleName(Uml::B),
                                       a->getMulti(Uml::B));
        }
        // print RoleA source
        if (printRoleA && a->getVisibility(Uml::A) == Uml::Visibility::Public) {
            QString         fieldClassName =
                cleanName(getUMLObjectName(a->getObject(Uml::A)));
            writeAssociationRoleSource(fieldClassName, a->getRoleName(Uml::A),
                                       a->getMulti(Uml::A));
        }
        // reset for next association in our loop
        printRoleA = false;
        printRoleB = false;
    }
}

void
TclWriter::writeAssociationRoleSource(const QString &fieldClassName,
                                      const QString &roleName, const QString &multi)
{
    // ONLY write out IF there is a rolename given
    // otherwise its not meant to be declared in the code
    if (roleName.isEmpty())
        return;

    // declare the association based on whether it is this a single variable
    // or a List (Vector). One day this will be done correctly with special
    // multiplicity object that we don't have to figure out what it means via regex.
    if (multi.isEmpty() || multi.contains(QRegExp("^[01]$"))) {
        QString         fieldVarName = roleName.lower();

        writeCode("configbody " + mClassGlobal + "::" + fieldVarName + " {} {");
        m_indentLevel++;
        writeCode("if {![$" + fieldVarName + " isa " + fieldClassName + "]} {");
        m_indentLevel++;
        writeCode("return -code error \"expected object of class: " +
                  fieldClassName + "\"");
        m_indentLevel--;
        writeCode("}");
        m_indentLevel--;

    } else {
        QString         fieldVarName = roleName.lower();

        writeCode("configbody " + mClassGlobal + "::" + fieldVarName + " {} {");
        m_indentLevel++;
        writeCode("foreach myObj $" + fieldVarName + " {");
        m_indentLevel++;
        writeCode("if {![$myObj isa " + fieldClassName + "]} {");
        m_indentLevel++;
        writeCode("return -code error \"expected object of class: " +
                  fieldClassName + "\"");
        m_indentLevel--;
        writeCode("}");
        m_indentLevel--;
        writeCode("}");
        m_indentLevel--;
    }
    writeCode('}' + m_endl);
}

QString
TclWriter::fixTypeName(const QString &string)
{
    if (string.isEmpty())
        return "void";
    return string;
}

// methods like this _shouldn't_ be needed IF we properly did things thruought the code.
QString
TclWriter::getUMLObjectName(UMLObject * obj)
{
    return (obj != 0) ? obj->getName() : QString("NULL");
}

const           QStringList
TclWriter::reservedKeywords() const
{
    static QStringList keywords;

    if              (keywords.isEmpty())
    {
        for (int i = 0; tclwords[i]; i++)
            keywords.append(tclwords[i]);
    }
    return          keywords;
}

