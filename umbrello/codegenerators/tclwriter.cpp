/***************************************************************************
    begin               : Thu Jul 26 2005
    copyright           : (C) 2005 by Rene Meyer
    email               : rene.meyer@sturmit.de
      (C) 2006-2007  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>
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
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>
#include <kdebug.h>
// app includes
#include "codegen_utils.h"
#include "../umldoc.h"
#include "../association.h"
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
        uDebug() << "Cannot write class of NULL concept!";
        return;
    }
    QFile fileh, filetcl;

    // find an appropriate name for our file
    fileName_ = findFileName(c, ".tcl");
    if (fileName_.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    if (!openFile(fileh, fileName_)) {
        emit codeGenerated(c, false);
        return;
    }
    // preparations
    className_ = cleanName(c->getName());
    if (!c->getPackage().isEmpty()) {
        mNamespace = "::" + cleanName(c->getPackage());
        mClassGlobal = mNamespace + "::" + className_;
    } else {
        mNamespace = "::";
        mClassGlobal = "::" + className_;
    }

    // write Header file
    writeHeaderFile(c, fileh);
    fileh.close();

    // Determine whether the implementation file is required.
    // (It is not required if the class is an enumeration.)
    bool need_impl = true;
    if (!c->isInterface()) {
        if (c->getBaseType() == Uml::ot_Enum)
            need_impl = false;
    }
    if (need_impl) {
        if (!openFile(filetcl, fileName_ + "body")) {
            emit codeGenerated(c, false);
            return;
        }
        // write Source file
        writeSourceFile(c, filetcl);
        filetcl.close();
    }
    // emit done code
    emit codeGenerated(c, true);
}

void
TclWriter::writeHeaderFile(UMLClassifier * c, QFile & fileh)
{
    // open stream for writing
    QTextStream stream(&fileh);
    mStream = &stream;

    // reset the indent level
    m_indentLevel = 0;

    // write header blurb
    QString str = getHeadingFile(".tcl");
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), fileName_);
        str.replace(QRegExp("%filepath%"), fileh.fileName());
        writeCode(str);
    }
    // set current namespace
    writeCode("namespace eval " + mNamespace + " {");
    m_indentLevel++;

    // check on already existing
    writeComm("Do not load twice");
    writeCode("if {[namespace exist " + className_ + "]} return");

    // source used superclass files
    UMLClassifierList superclasses = c->getSuperClasses();
    if (superclasses.count() > 0) {
        writeComm
        ("Source found and used class files and import class command if necessary");

        foreach (UMLClassifier * classifier , superclasses ) {
            writeUse(classifier);
        }
    }
    // write all "source" we need to include other classes, that arent us.
    if (c->hasAssociations()) {
        writeAssociationIncl(c->getSpecificAssocs(Uml::at_Association), c->getID(),
                             "Associations");
        writeAssociationIncl(c->getAggregations(), c->getID(),
                             "Aggregations");
        writeAssociationIncl(c->getCompositions(), c->getID(),
                             "Compositions");
    }
    //Write class Documentation
    writeDocu("\n@class\t" + className_ + m_endl + c->getDoc());

    //check if class is abstract and / or has abstract methods
    if ((c->getAbstract() || c->isInterface())
            && !hasAbstractOps(c)) {
        writeComm("TODO abstract class" + className_ +
                  "\nInherit from it and create only objects from the derived classes");
    }
    // check on enum classes
    if (!c->isInterface()) {
        // use tcl-list for enum's
        if (c->getBaseType() == Uml::ot_Enum) {
            UMLClassifierListItemList litList =
                c->getFilteredList(Uml::ot_EnumLiteral);
            writeCode("set enum_" + className_ + " [list\\");
            m_indentLevel++;
            foreach (UMLClassifierListItem * lit , litList ) {
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
        foreach (UMLTemplate * t , template_params ) {
            QString formalName = t->getName();
            QString typeName = t->getTypeName();
            writeCode(typeName + "# " + formalName);
        }
    }
    // start my own class
    writeCode("class " + className_ + " {");
    m_indentLevel++;
    if (c->getSuperClasses().count() > 0) {
        QString code = "inherit";
        foreach (UMLClassifier * superClass , c->getSuperClasses()) {
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
    if (!c->isInterface()) {
        writeConstructorHeader();
        writeDestructorHeader();
    }
    // attributes
    writeAttributeDecl(c, Uml::Visibility::Public, true);      // write static attributes first
    writeAttributeDecl(c, Uml::Visibility::Public, false);
    // associations
    writeAssociationDecl(c->getSpecificAssocs(Uml::at_Association), Uml::Visibility::Public,
                         c->getID(), "Associations");
    writeAssociationDecl(c->getAggregations(), Uml::Visibility::Public, c->getID(),
                         "Aggregations");
    writeAssociationDecl(c->getCompositions(), Uml::Visibility::Public, c->getID(),
                         "Compositions");
    //TODO  writeHeaderAccessorMethodDecl(c, Uml::Visibility::Public, stream);
    writeOperationHeader(c, Uml::Visibility::Public);

    // PROTECTED attribs/methods
    //
    // attributes
    writeAttributeDecl(c, Uml::Visibility::Protected, true);   // write static attributes first
    writeAttributeDecl(c, Uml::Visibility::Protected, false);
    // associations
    writeAssociationDecl(c->getSpecificAssocs(Uml::at_Association), Uml::Visibility::Protected,
                         c->getID(), "Association");
    writeAssociationDecl(c->getAggregations(), Uml::Visibility::Protected,
                         c->getID(), "Aggregation");
    writeAssociationDecl(c->getCompositions(), Uml::Visibility::Protected,
                         c->getID(), "Composition");
    //TODO  writeHeaderAccessorMethodDecl(c, Uml::Visibility::Protected, stream);
    writeOperationHeader(c, Uml::Visibility::Protected);

    // PRIVATE attribs/methods
    //
    // attributes
    writeAttributeDecl(c, Uml::Visibility::Private, true);     // write static attributes first
    writeAttributeDecl(c, Uml::Visibility::Private, false);
    // associations
    writeAssociationDecl(c->getSpecificAssocs(Uml::at_Association), Uml::Visibility::Private,
                         c->getID(), "Associations");
    writeAssociationDecl(c->getAggregations(), Uml::Visibility::Private, c->getID(),
                         "Aggregations");
    writeAssociationDecl(c->getCompositions(), Uml::Visibility::Private, c->getID(),
                         "Compositions");
    //TODO  writeHeaderAccessorMethodDecl(c, Uml::Visibility::Public, stream);
    writeOperationHeader(c, Uml::Visibility::Private);
    writeInitAttributeHeader(c); // this is always private, used by constructors to initialize class

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
    QTextStream stream(&filetcl);
    mStream = &stream;

    // set the starting indentation at zero
    m_indentLevel = 0;

    //try to find a heading file (license, coments, etc)
    QString         str;
    str = getHeadingFile(".tclbody");
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), fileName_ + "body");
        str.replace(QRegExp("%filepath%"), filetcl.fileName());
        writeCode(str);
    }
    // Start body of class

    // constructors are first ops we print out
    if (!c->isInterface()) {
        writeConstructorSource(c);
        writeDestructorSource();
    }
    // Public attributes have in tcl a configbody method
    writeAttributeSource(c);
    // Association access functions
    writeAssociationSource(c->getSpecificAssocs(Uml::at_Association), c->getID());
    writeAssociationSource(c->getAggregations(), c->getID());
    writeAssociationSource(c->getCompositions(), c->getID());
    // Procedures and methods
    writeOperationSource(c, Uml::Visibility::Public);
    writeOperationSource(c, Uml::Visibility::Protected);
    writeOperationSource(c, Uml::Visibility::Private);
    // Yep, bringing up the back of the bus, our initialization method for attributes
    writeInitAttributeSource(c);
}

void
TclWriter::writeCode(const QString &text)
{
    *mStream << getIndent() << text << m_endl;
}

void
TclWriter::writeComm(const QString &text)
{
    QStringList lines = text.split(QRegExp("\n"));
    for (int i = 0; i < lines.count(); i++) {
        *mStream << getIndent() << "# " << lines[i] << m_endl;
    }
}

void
TclWriter::writeDocu(const QString &text)
{
    QStringList lines = text.split(QRegExp("\n"));
    for (int i = 0; i < lines.count(); i++) {
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
    foreach (UMLAssociation * a , list ) {
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
    QString myNs;

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
    writeDocu(m_endl + "@func constructor" + m_endl +
        "@par args contain all configuration parameters" + m_endl);
    writeCode("constructor {args} {}" + m_endl);
}

void
TclWriter::writeConstructorSource(UMLClassifier * c)
{
    writeComm(mClassGlobal + "::constructor");
    writeCode(mClassGlobal + "::constructor {args} {");
    m_indentLevel++;
    if (c->hasAttributes()) {
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
TclWriter::writeAttributeDecl(UMLClassifier * c, Uml::Visibility visibility, bool writeStatic)
{
    if (c->isInterface())
        return;

    QString scope = visibility.toString();
    QString type;
    if (writeStatic) {
        type = "common";
    } else {
        type = "variable";
    }
    UMLAttributeList list;
    if (writeStatic) {
        list = c->getAttributeListStatic(visibility);
    }
    else {
        list = c->getAttributeList(visibility);
    }

    if (list.count() > 0) {
        writeComm(m_endl + scope + ' ' + type + " attributes" + m_endl);
        // write attrib declarations now
        QString documentation;
        foreach (UMLAttribute * at , list ) {
            documentation = at->getDoc();
            QString varName = cleanName(at->getName());
            QString typeName = fixTypeName(at->getTypeName());
            writeDocu(m_endl + "@var " + scope + ' ' + type + ' ' + typeName + ' ' +
                      varName + m_endl + documentation);
            writeCode(scope + ' ' + type + ' ' + varName + m_endl);
        }
    }
}

void
TclWriter::writeAssociationDecl(UMLAssociationList associations,
                                Uml::Visibility permitScope, Uml::IDType id,
                                const QString &type)
{
    Q_UNUSED(type);
    if (forceSections() || !associations.isEmpty()) {
        bool printRoleA = false, printRoleB = false;
        foreach (UMLAssociation * a , associations ) {

            // it may seem counter intuitive, but you want to insert the role of the
            // *other* class into *this* class.
            if (a->getObjectId(Uml::A) == id && !a->getRoleName(Uml::B).isEmpty())
                printRoleB = true;

            if (a->getObjectId(Uml::B) == id && !a->getRoleName(Uml::A).isEmpty())
                printRoleA = true;

            // First: we insert documentaion for association IF it has either role AND some documentation (!)
            // print RoleB decl
            if (printRoleB && a->getVisibility(Uml::B) == permitScope) {

                QString fieldClassName =
                    cleanName(getUMLObjectName(a->getObject(Uml::B)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::B),
                                         a->getMulti(Uml::B), a->getRoleDoc(Uml::B),
                                         permitScope.toString());
            }
            // print RoleA decl
            if (printRoleA && a->getVisibility(Uml::A) == permitScope) {
                QString fieldClassName =
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
        QString fieldVarName = roleName.toLower();

        // record this for later consideration of initialization IF the
        // multi value requires 1 of these objects
        if (ObjectFieldVariables.indexOf(fieldVarName) == -1 &&
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
        QString fieldVarName = roleName.toLower();

        // record unique occurrences for later when we want to check
        // for initialization of this vector
        if (VectorFieldVariables.indexOf(fieldVarName) == -1)
            VectorFieldVariables.append(fieldVarName);
        writeDocu(m_endl + "@var" + scope + " variable <" + fieldClassName +
                  "*> " + fieldVarName + m_endl + doc);
        writeCode(scope + " variable " + fieldVarName + m_endl);
    }
}

void
TclWriter::writeInitAttributeHeader(UMLClassifier * c)
{
    if (c->hasAttributes()) {
        writeDocu("@method private initAttributes" + m_endl +
                  "Initialize all internal variables");
        writeCode("private method initAttributes {}");
    }
}

void
TclWriter::writeInitAttributeSource(UMLClassifier* c)
{
    // only need to do this under certain conditions
    if (c->hasAttributes()) {
        QString varName;

        writeComm(mClassGlobal + "::initAttributes");
        writeCode("body " + mClassGlobal + "::initAttributes {} {");
        m_indentLevel++;

        // first, initiation of fields derived from attributes
        UMLAttributeList atl = c->getAttributeList();
        foreach (UMLAttribute * at , atl ) {
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
            QString fieldClassName = *it;
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
    int j;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList inputlist = c->getOpList();
    foreach (UMLOperation * op , inputlist ) {
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
    foreach ( UMLOperation* op , oplist ) {
        QString doc = "";
        QString code = "";
        QString methodReturnType = fixTypeName(op->getTypeName());
        QString name = cleanName(op->getName());
        QString scope = permitScope.toString();
        if (op->getAbstract() || c->isInterface()) {
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
        foreach ( UMLAttribute* at , atl ) {
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
            j++;
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
    int j;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList inputlist = c->getOpList();
    foreach (UMLOperation * op , inputlist ) {
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
    foreach ( UMLOperation* op , oplist ) {
        QString code = "";
        QString methodReturnType = fixTypeName(op->getTypeName());
        QString name;
        // no code needed
        if (op->getAbstract() || c->isInterface()) {
            continue;
        }
        name = mClassGlobal + "::" + cleanName(op->getName());
        writeComm(name);
        code = "body " + name + " {";
        // parameters
        UMLAttributeList atl = op->getParmList();
        j = 0;
        foreach ( UMLAttribute* at , atl ) {
            QString         atName = cleanName(at->getName());
            if (at->getInitialValue().isEmpty()) {
                code += ' ' + atName;
            } else {
                code += " {" + atName + ' ' + at->getInitialValue() + "} ";
            }
            j++;
        }
        writeCode(code += "} {");
        m_indentLevel++;
        QString sourceCode = op->getSourceCode();
        if (!sourceCode.isEmpty()) {
            *mStream << formatSourceCode(sourceCode, getIndent());
        }
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
TclWriter::writeAttributeSource(UMLClassifier * c)
{
    UMLAttributeList list = c->getAttributeList(Uml::Visibility::Public);

    foreach ( UMLAttribute* at , list ) {
        QString name = mClassGlobal + "::" + cleanName(at->getName());

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

    bool printRoleA = false, printRoleB = false;
    foreach (UMLAssociation * a , associations ) {

        // it may seem counter intuitive, but you want to insert the role of the
        // *other* class into *this* class.
        if (a->getObjectId(Uml::A) == id && !a->getRoleName(Uml::B).isEmpty())
            printRoleB = true;

        if (a->getObjectId(Uml::B) == id && !a->getRoleName(Uml::A).isEmpty())
            printRoleA = true;

        // print RoleB source
        if (printRoleB && a->getVisibility(Uml::B) == Uml::Visibility::Public) {

            QString fieldClassName =
                cleanName(getUMLObjectName(a->getObject(Uml::B)));
            writeAssociationRoleSource(fieldClassName, a->getRoleName(Uml::B),
                                       a->getMulti(Uml::B));
        }
        // print RoleA source
        if (printRoleA && a->getVisibility(Uml::A) == Uml::Visibility::Public) {
            QString fieldClassName =
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
        QString fieldVarName = roleName.toLower();

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
        QString fieldVarName = roleName.toLower();

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

const QStringList
TclWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty())
    {
        for (int i = 0; tclwords[i]; i++)
            keywords.append(tclwords[i]);
    }
    return keywords;
}

