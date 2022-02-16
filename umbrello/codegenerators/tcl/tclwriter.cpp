/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Rene Meyer <rene.meyer@sturmit.de>
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "tclwriter.h"

// app includes
#include "association.h"
#include "classifier.h"
#include "classifierlistitem.h"
#include "codegen_utils.h"
#include "debug_utils.h"
#include "model_utils.h"
#include "operation.h"
#include "template.h"
#include "umltemplatelist.h"
#include "umlclassifierlistitemlist.h"
#include "umldoc.h"
#include "uml.h"  // Only needed for log{Warn,Error}

// qt includes
#include <QFile>
#include <QRegExp>
#include <QTextStream>

static const char *reserved_words[] = {
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

/**
 * Constructor, initialises a couple of variables.
 */
TclWriter::TclWriter()
  : mStream(0)
{
}

/**
 * Destructor, empty.
 */
TclWriter::~TclWriter()
{
}

/**
 * Returns "Tcl".
 * @return   the programming language identifier
 */
Uml::ProgrammingLanguage::Enum TclWriter::language() const
{
    return Uml::ProgrammingLanguage::Tcl;
}

/**
 * Call this method to generate tcl code for a UMLClassifier.
 * @param c   the class to generate code for
 */
void TclWriter::writeClass(UMLClassifier * c)
{
    if (!c) {
        logWarn0("TclWriter::writeClass: Cannot write class of NULL concept");
        return;
    }
    QFile fileh, filetcl;

    // find an appropriate name for our file
    fileName_ = findFileName(c, QLatin1String(".tcl"));
    if (fileName_.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    if (!openFile(fileh, fileName_)) {
        emit codeGenerated(c, false);
        return;
    }
    // preparations
    className_ = cleanName(c->name());
    if (!c->package().isEmpty()) {
        mNamespace = QLatin1String("::") + cleanName(c->package());
        mClassGlobal = mNamespace + QLatin1String("::") + className_;
    } else {
        mNamespace = QLatin1String("::");
        mClassGlobal = QLatin1String("::") + className_;
    }

    // write Header file
    writeHeaderFile(c, fileh);
    fileh.close();

    // Determine whether the implementation file is required.
    // (It is not required if the class is an enumeration.)
    bool need_impl = true;
    if (!c->isInterface()) {
        if (c->baseType() == UMLObject::ot_Enum)
            need_impl = false;
    }
    if (need_impl) {
        if (!openFile(filetcl, fileName_ + QLatin1String("body"))) {
            emit codeGenerated(c, false);
            return;
        }
        // write Source file
        writeSourceFile(c, filetcl);
        filetcl.close();
    }
    // emit done code
    emit codeGenerated(c, true);
    emit showGeneratedFile(fileh.fileName());
    if (need_impl) {
        emit showGeneratedFile(filetcl.fileName());
    }
}

/**
 * Write the header file for this classifier.
 */
void TclWriter::writeHeaderFile(UMLClassifier * c, QFile & fileh)
{
    // open stream for writing
    QTextStream stream(&fileh);
    mStream = &stream;

    // reset the indent level
    m_indentLevel = 0;

    // write header blurb
    QString str = getHeadingFile(QLatin1String(".tcl"));
    if (!str.isEmpty()) {
        str.replace(QRegExp(QLatin1String("%filename%")), fileName_);
        str.replace(QRegExp(QLatin1String("%filepath%")), fileh.fileName());
        writeCode(str);
    }
    // set current namespace
    writeCode(QLatin1String("namespace eval ") + mNamespace + QLatin1String(" {"));
    m_indentLevel++;

    // check on already existing
    writeComm(QLatin1String("Do not load twice"));
    writeCode(QLatin1String("if {[namespace exist ") + className_ + QLatin1String("]} return"));

    // source used superclass files
    UMLClassifierList superclasses = c->getSuperClasses();
    if (superclasses.count() > 0) {
        writeComm
        (QLatin1String("Source found and used class files and import class command if necessary"));

        foreach (UMLClassifier * classifier, superclasses) {
            writeUse(classifier);
        }
    }
    // write all "source" we need to include other classes, that arent us.
    if (c->hasAssociations()) {
        writeAssociationIncl(c->getSpecificAssocs(Uml::AssociationType::Association), c->id(),
                             QLatin1String("Associations"));
        writeAssociationIncl(c->getAggregations(), c->id(),
                             QLatin1String("Aggregations"));
        writeAssociationIncl(c->getCompositions(), c->id(),
                             QLatin1String("Compositions"));
    }
    //Write class Documentation
    writeDocu(QLatin1String("\n@class\t") + className_ + m_endl + c->doc());

    //check if class is abstract and / or has abstract methods
    if ((c->isAbstract() || c->isInterface())
            && !hasAbstractOps(c)) {
        writeComm(QLatin1String("TODO abstract class") + className_ +
                  QLatin1String("\nInherit from it and create only objects from the derived classes"));
    }
    // check on enum classes
    if (!c->isInterface()) {
        // use tcl-list for enum's
        if (c->baseType() == UMLObject::ot_Enum) {
            UMLClassifierListItemList litList =
                c->getFilteredList(UMLObject::ot_EnumLiteral);
            writeCode(QLatin1String("set enum_") + className_ + QLatin1String(" [list\\"));
            m_indentLevel++;
            foreach (UMLClassifierListItem * lit, litList) {
                QString enumLiteral = cleanName(lit->name());
                writeCode(enumLiteral + QLatin1String("\\"));
            }
            m_indentLevel--;
            writeCode(QLatin1String("];# end of enum"));
            m_indentLevel--;
            writeCode(QLatin1String("};# end of namespace"));
            return;
        }
    }
    // Generate template parameters.
    UMLTemplateList template_params = c->getTemplateList();
    if (template_params.count()) {
        writeCode(QLatin1String("#TODO template<"));
        foreach (UMLTemplate * t, template_params) {
            QString formalName = t->name();
            QString typeName = t->getTypeName();
            writeCode(typeName + QLatin1String("# ") + formalName);
        }
    }
    // start my own class
    writeCode(QLatin1String("class ") + className_ + QLatin1String(" {"));
    m_indentLevel++;
    if (c->getSuperClasses().count() > 0) {
        QString code = QLatin1String("inherit");
        foreach (UMLClassifier * superClass, c->getSuperClasses()) {
            /*
            if (superClass->getAbstract() || superClass->isInterface())
                stream << indent() << QLatin1String("virtual ");
            */
            if (superClass->package().isEmpty()) {
                code += QLatin1String(" ::") + cleanName(superClass->name());
            } else {
                code +=
                    QLatin1String(" ::") + cleanName(superClass->package()) + QLatin1String("::") +
                    cleanName(superClass->name());
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
    writeAssociationDecl(c->getSpecificAssocs(Uml::AssociationType::Association), Uml::Visibility::Public, c->id(),
                         QLatin1String("Associations"));
    writeAssociationDecl(c->getAggregations(), Uml::Visibility::Public, c->id(),
                         QLatin1String("Aggregations"));
    writeAssociationDecl(c->getCompositions(), Uml::Visibility::Public, c->id(),
                         QLatin1String("Compositions"));
    //TODO  writeHeaderAccessorMethodDecl(c, Uml::Visibility::Public, stream);
    writeOperationHeader(c, Uml::Visibility::Public);

    // PROTECTED attribs/methods
    //
    // attributes
    writeAttributeDecl(c, Uml::Visibility::Protected, true);   // write static attributes first
    writeAttributeDecl(c, Uml::Visibility::Protected, false);
    // associations
    writeAssociationDecl(c->getSpecificAssocs(Uml::AssociationType::Association), Uml::Visibility::Protected,
                         c->id(), QLatin1String("Association"));
    writeAssociationDecl(c->getAggregations(), Uml::Visibility::Protected,
                         c->id(), QLatin1String("Aggregation"));
    writeAssociationDecl(c->getCompositions(), Uml::Visibility::Protected,
                         c->id(), QLatin1String("Composition"));
    //TODO  writeHeaderAccessorMethodDecl(c, Uml::Visibility::Protected, stream);
    writeOperationHeader(c, Uml::Visibility::Protected);

    // PRIVATE attribs/methods
    //
    // attributes
    writeAttributeDecl(c, Uml::Visibility::Private, true);     // write static attributes first
    writeAttributeDecl(c, Uml::Visibility::Private, false);
    // associations
    writeAssociationDecl(c->getSpecificAssocs(Uml::AssociationType::Association), Uml::Visibility::Private,
                         c->id(), QLatin1String("Associations"));
    writeAssociationDecl(c->getAggregations(), Uml::Visibility::Private, c->id(),
                         QLatin1String("Aggregations"));
    writeAssociationDecl(c->getCompositions(), Uml::Visibility::Private, c->id(),
                         QLatin1String("Compositions"));
    //TODO  writeHeaderAccessorMethodDecl(c, Uml::Visibility::Public, stream);
    writeOperationHeader(c, Uml::Visibility::Private);
    writeInitAttributeHeader(c); // this is always private, used by constructors to initialize class

    // end of class header
    m_indentLevel--;
    writeCode(QLatin1String("};# end of class"));

    // end of class namespace, if any
    m_indentLevel--;
    writeCode(QLatin1String("};# end of namespace"));
}

/**
 * Write the source code body file for this classifier.
 */
void TclWriter::writeSourceFile(UMLClassifier * c, QFile & filetcl)
{
    // open stream for writing
    QTextStream stream(&filetcl);
    mStream = &stream;

    // set the starting indentation at zero
    m_indentLevel = 0;

    //try to find a heading file (license, comments, etc)
    QString         str;
    str = getHeadingFile(QLatin1String(".tclbody"));
    if (!str.isEmpty()) {
        str.replace(QRegExp(QLatin1String("%filename%")), fileName_ + QLatin1String("body"));
        str.replace(QRegExp(QLatin1String("%filepath%")), filetcl.fileName());
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
    writeAssociationSource(c->getSpecificAssocs(Uml::AssociationType::Association), c->id());
    writeAssociationSource(c->getAggregations(), c->id());
    writeAssociationSource(c->getCompositions(), c->id());
    // Procedures and methods
    writeOperationSource(c, Uml::Visibility::Public);
    writeOperationSource(c, Uml::Visibility::Protected);
    writeOperationSource(c, Uml::Visibility::Private);
    // Yep, bringing up the back of the bus, our initialization method for attributes
    writeInitAttributeSource(c);
}

/**
 * Write the source code text.
 */
void TclWriter::writeCode(const QString &text)
{
    *mStream << indent() << text << m_endl;
}

/**
 * Write comment text.
 */
void TclWriter::writeComm(const QString &text)
{
    QStringList lines = text.split(QRegExp(QLatin1String("\n")));
    for (int i = 0; i < lines.count(); ++i) {
        *mStream << indent() << QLatin1String("# ") << lines[i] << m_endl;
    }
}

/**
 * Write documentation text.
 */
void TclWriter::writeDocu(const QString &text)
{
    QStringList lines = text.split(QRegExp(QLatin1String("\n")));
    for (int i = 0; i < lines.count(); ++i) {
        *mStream << indent() << QLatin1String("## ") << lines[i] << m_endl;
    }
}

// To prevent circular including when both classifiers on either end
// of an association have roles we need to have forward declaration of
// the other class...but only IF it is not THIS class (as could happen
// in self-association relationship).
void TclWriter::writeAssociationIncl(UMLAssociationList list, Uml::ID::Type myId,
                                     const QString &type)
{
    foreach (UMLAssociation * a, list) {
        UMLClassifier  *classifier = 0;

        writeComm(m_endl + type + m_endl + a->toString() + m_endl + a->doc());
        // only use OTHER classes (e.g. we don't need to write includes for ourselves!!
        // AND only IF the roleName is defined, otherwise, it is not meant to be noticed.
        if (a->getObjectId(Uml::RoleType::A) == myId && !a->getRoleName(Uml::RoleType::B).isEmpty()) {
            classifier = a->getObject(Uml::RoleType::B)->asUMLClassifier();
            if (classifier == 0)
                continue;
            writeUse(classifier);
        } else if (a->getObjectId(Uml::RoleType::B) == myId
                   && !a->getRoleName(Uml::RoleType::A).isEmpty()) {
            classifier = a->getObject(Uml::RoleType::A)->asUMLClassifier();
            if (classifier == 0)
                continue;
            if (classifier->package().isEmpty())
                writeCode(QLatin1String("namespace eval ") + cleanName(classifier->name()) +
                          QLatin1String(" {}"));
        } else {
            // CHECK: This crashes (classifier still NULL from above)
            /*
            writeCode(QLatin1String("namespace eval ") + cleanName(classifier->getPackage()) +
                      QLatin1String("::") + cleanName(classifier->getName()) + QLatin1String(" {}"));
             */
        }
    }
}

void TclWriter::writeUse(UMLClassifier * c)
{
    QString myNs;

    if (!c->package().isEmpty()) {
        myNs = cleanName(c->package());
    }
    // if different package
    if (QString(QLatin1String("::") + myNs) != mNamespace) {
        if (c->package().isEmpty()) {
            writeCode(QLatin1String("source ") + findFileName(c, QLatin1String(".tcl")));
            writeCode(QLatin1String("namespace import ::") + cleanName(c->name()));
        } else {
            writeCode(QLatin1String("package require ") + myNs);
            writeCode(QLatin1String("namespace import ::") + myNs + QLatin1String("::") +
                      cleanName(c->name()));
        }
    } else {
        // source the file
        writeCode(QLatin1String("source ") + findFileName(c, QLatin1String(".tcl")));
    }
}

void TclWriter::writeConstructorHeader()
{
    writeDocu(m_endl + QLatin1String("@func constructor") + m_endl +
        QLatin1String("@par args contain all configuration parameters") + m_endl);
    writeCode(QLatin1String("constructor {args} {}") + m_endl);
}

void TclWriter::writeConstructorSource(UMLClassifier * c)
{
    writeComm(mClassGlobal + QLatin1String("::constructor"));
    writeCode(mClassGlobal + QLatin1String("::constructor {args} {"));
    m_indentLevel++;
    if (c->hasAttributes()) {
        writeCode(QLatin1String("initAttributes"));
    }
    writeCode(QLatin1String("eval configure $args"));
    m_indentLevel--;
    writeCode(QLatin1Char('}') + m_endl);
}

void TclWriter::writeDestructorHeader()
{
    writeDocu(m_endl + QLatin1String("@func destructor") + m_endl);
    writeCode(QLatin1String("destructor {} {}"));
}

void TclWriter::writeDestructorSource()
{
    writeComm(mClassGlobal + QLatin1String("::destructor"));
    writeCode(mClassGlobal + QLatin1String("::destructor {} {") + m_endl + QLatin1Char('}') + m_endl);
}

/**
 * Writes the Attribute declarations
 * @param c             classifier
 * @param writeStatic   whether to write static or non-static attributes out
 * @param visibility    the visibility of the attribs to print out
 */
void TclWriter::writeAttributeDecl(UMLClassifier * c, Uml::Visibility::Enum visibility, bool writeStatic)
{
    if (c->isInterface())
        return;

    QString scope = Uml::Visibility::toString(visibility);
    QString type;
    if (writeStatic) {
        type = QLatin1String("common");
    } else {
        type = QLatin1String("variable");
    }
    UMLAttributeList list;
    if (writeStatic) {
        list = c->getAttributeListStatic(visibility);
    }
    else {
        list = c->getAttributeList(visibility);
    }

    if (list.count() > 0) {
        writeComm(m_endl + scope + QLatin1Char(' ') + type + QLatin1String(" attributes") + m_endl);
        // write attrib declarations now
        QString documentation;
        foreach (UMLAttribute * at, list) {
            documentation = at->doc();
            QString varName = cleanName(at->name());
            QString typeName = fixTypeName(at->getTypeName());
            writeDocu(m_endl + QLatin1String("@var ") + scope + QLatin1Char(' ') + type + QLatin1Char(' ') + typeName + QLatin1Char(' ') +
                      varName + m_endl + documentation);
            writeCode(scope + QLatin1Char(' ') + type + QLatin1Char(' ') + varName + m_endl);
        }
    }
}

/**
 * Searches a list of associations for appropriate ones to write out as attributes.
 */
void TclWriter::writeAssociationDecl(UMLAssociationList associations,
                                Uml::Visibility::Enum permitScope, Uml::ID::Type id,
                                const QString &type)
{
    Q_UNUSED(type);
    if (forceSections() || !associations.isEmpty()) {
        bool printRoleA = false, printRoleB = false;
        foreach (UMLAssociation * a, associations) {

            // it may seem counter intuitive, but you want to insert the role of the
            // *other* class into *this* class.
            if (a->getObjectId(Uml::RoleType::A) == id && !a->getRoleName(Uml::RoleType::B).isEmpty())
                printRoleB = true;

            if (a->getObjectId(Uml::RoleType::B) == id && !a->getRoleName(Uml::RoleType::A).isEmpty())
                printRoleA = true;

            // First: we insert documentation for association IF it has either role AND some documentation (!)
            // print RoleB decl
            if (printRoleB && a->visibility(Uml::RoleType::B) == permitScope) {

                QString fieldClassName =
                    cleanName(getUMLObjectName(a->getObject(Uml::RoleType::B)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::RoleType::B),
                                         a->getMultiplicity(Uml::RoleType::B), a->getRoleDoc(Uml::RoleType::B),
                                         Uml::Visibility::toString(permitScope));
            }
            // print RoleA decl
            if (printRoleA && a->visibility(Uml::RoleType::A) == permitScope) {
                QString fieldClassName =
                    cleanName(getUMLObjectName(a->getObject(Uml::RoleType::A)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::RoleType::A),
                                         a->getMultiplicity(Uml::RoleType::A), a->getRoleDoc(Uml::RoleType::A),
                                         Uml::Visibility::toString(permitScope));
            }
            // reset for next association in our loop
            printRoleA = false;
            printRoleB = false;
        }
    }
}

/**
 * Writes out an association as an attribute using Vector.
 */
void TclWriter::writeAssociationRoleDecl(const QString &fieldClassName, const QString &roleName,
                                    const QString &multi, const QString &doc, const QString &scope)
{
    // ONLY write out IF there is a rolename given
    // otherwise it is not meant to be declared in the code
    if (roleName.isEmpty())
        return;

    // declare the association based on whether it is this a single variable
    // or a List (Vector). One day this will be done correctly with special
    // multiplicity object that we don't have to figure out what it means via regex.
    if (multi.isEmpty() || multi.contains(QRegExp(QLatin1String("^[01]$")))) {
        QString fieldVarName = roleName.toLower();

        // record this for later consideration of initialization IF the
        // multi value requires 1 of these objects
        if (ObjectFieldVariables.indexOf(fieldVarName) == -1 &&
                multi.contains(QRegExp(QLatin1String("^1$")))
          ) {
            // ugh. UGLY. Storing variable name and its class in pairs.
            ObjectFieldVariables.append(fieldVarName);
            ObjectFieldVariables.append(fieldClassName);
        }
        writeDocu(m_endl + QLatin1String("@var ") + scope + QLatin1String(" variable <") + fieldClassName +
                  QLatin1String("> ") + fieldVarName + m_endl + doc);
        writeCode(scope + QLatin1String(" variable ") + fieldVarName + m_endl);
    } else {
        QString fieldVarName = roleName.toLower();

        // record unique occurrences for later when we want to check
        // for initialization of this vector
        if (VectorFieldVariables.indexOf(fieldVarName) == -1)
            VectorFieldVariables.append(fieldVarName);
        writeDocu(m_endl + QLatin1String("@var") + scope + QLatin1String(" variable <") + fieldClassName +
                  QLatin1String("*> ") + fieldVarName + m_endl + doc);
        writeCode(scope + QLatin1String(" variable ") + fieldVarName + m_endl);
    }
}

/**
 * If needed, write out the declaration for the method to initialize attributes of our class.
 */
void TclWriter::writeInitAttributeHeader(UMLClassifier * c)
{
    if (c->hasAttributes()) {
        writeDocu(QLatin1String("@method private initAttributes") + m_endl +
                  QLatin1String("Initialize all internal variables"));
        writeCode(QLatin1String("private method initAttributes {}"));
    }
}

/**
 * If needed, write out the declaration for the method to initialize attributes of our class.
 */
void TclWriter::writeInitAttributeSource(UMLClassifier* c)
{
    // only need to do this under certain conditions
    if (c->hasAttributes()) {
        QString varName;

        writeComm(mClassGlobal + QLatin1String("::initAttributes"));
        writeCode(QLatin1String("body ") + mClassGlobal + QLatin1String("::initAttributes {} {"));
        m_indentLevel++;

        // first, initiation of fields derived from attributes
        UMLAttributeList atl = c->getAttributeList();
        foreach (UMLAttribute * at, atl) {
            if (!at->getInitialValue().isEmpty()) {
                varName = cleanName(at->name());
                writeCode(QLatin1String("set ") + varName + QLatin1Char(' ') + at->getInitialValue());
            }
        }
        // Now initialize the association related fields (e.g. vectors)
        QStringList::Iterator it;
        for (it = VectorFieldVariables.begin();
                it != VectorFieldVariables.end(); ++it) {
            varName = *it;
            writeCode(QLatin1String("set ") + varName + QLatin1String(" [list]"));
        }

        for (it = ObjectFieldVariables.begin();
                it != ObjectFieldVariables.end(); ++it) {
            varName = *it;
            it++;
            QString fieldClassName = *it;
            writeCode(QLatin1String("set ") + varName + QLatin1String(" [list]"));
        }
        // clean up
        ObjectFieldVariables.clear();   // shouldn't be needed?
        VectorFieldVariables.clear();   // shouldn't be needed?

        m_indentLevel--;
        writeCode(QLatin1Char('}') + m_endl);
    }
}

void TclWriter::writeOperationHeader(UMLClassifier * c, Uml::Visibility::Enum permitScope)
{
    UMLOperationList oplist;
    int j;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList inputlist = c->getOpList();
    foreach (UMLOperation * op, inputlist) {
        switch (op->visibility()) {
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
        writeComm(QLatin1String("Operations"));
    }
    foreach (UMLOperation* op, oplist) {
        QString doc;
        QString code;
        QString methodReturnType = fixTypeName(op->getTypeName());
        QString name = cleanName(op->name());
        QString scope = Uml::Visibility::toString(permitScope);
        if (op->isAbstract() || c->isInterface()) {
            //TODO declare abstract method as 'virtual'
            // str += "virtual ";
        }
        // declaration for header file
        if (op->isStatic()) {
            doc = m_endl + QLatin1String("@fn ") + scope + QLatin1String(" proc ") + name + m_endl;
            code = scope + QLatin1String(" proc ") + name + QLatin1String(" {");
        } else {
            doc = m_endl + QLatin1String("@fn ") + scope + QLatin1String(" method ") + name + m_endl;
            code = scope + QLatin1String(" method ") + name + QLatin1String(" {");
        }
        // method parameters
        UMLAttributeList atl = op->getParmList();
        j = 0;
        foreach (UMLAttribute* at, atl) {
            QString typeName = fixTypeName(at->getTypeName());
            QString atName = cleanName(at->name());
            if (at->getInitialValue().isEmpty()) {
                doc +=
                    QLatin1String("@param ") + typeName + QLatin1Char(' ') + atName + m_endl + at->doc() +
                    m_endl;
                code += QLatin1Char(' ') + atName;
            } else {
                doc +=
                    QLatin1String("@param ") + typeName + QLatin1Char(' ') + atName + QLatin1String(" (default=") +
                    at->getInitialValue() + QLatin1String(") ") + m_endl + at->doc() + m_endl;
                code += QLatin1String(" {") + atName + QLatin1Char(' ') + at->getInitialValue() + QLatin1String("} ");
            }
            j++;
        }
        if (methodReturnType != QLatin1String("void")) {
            doc += QLatin1String("@return     ") + methodReturnType + m_endl;
        }
        writeDocu(doc + op->doc());
        writeCode(code + QLatin1String("} {}") + m_endl);
    }
}

void TclWriter::writeOperationSource(UMLClassifier * c, Uml::Visibility::Enum permitScope)
{
    UMLOperationList oplist;
    int j;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList inputlist = c->getOpList();
    foreach (UMLOperation * op, inputlist) {
        switch (op->visibility()) {
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
    foreach (UMLOperation* op, oplist) {
        QString code;
        QString methodReturnType = fixTypeName(op->getTypeName());
        QString name;
        // no code needed
        if (op->isAbstract() || c->isInterface()) {
            continue;
        }
        name = mClassGlobal + QLatin1String("::") + cleanName(op->name());
        writeComm(name);
        code = QLatin1String("body ") + name + QLatin1String(" {");
        // parameters
        UMLAttributeList atl = op->getParmList();
        j = 0;
        foreach (UMLAttribute* at, atl) {
            QString atName = cleanName(at->name());
            if (at->getInitialValue().isEmpty()) {
                code += QLatin1Char(' ') + atName;
            } else {
                code += QLatin1String(" {") + atName + QLatin1Char(' ') + at->getInitialValue() + QLatin1String("} ");
            }
            j++;
        }
        writeCode(code += QLatin1String("} {"));
        m_indentLevel++;
        QString sourceCode = op->getSourceCode();
        if (!sourceCode.isEmpty()) {
            *mStream << formatSourceCode(sourceCode, indent());
        }
        if (methodReturnType != QLatin1String("void")) {
            writeCode(QLatin1String("return ") + methodReturnType);
        } else {
            writeCode(QLatin1String("return"));
        }
        m_indentLevel--;
        writeCode(QLatin1Char('}') + m_endl);
    }
}

void TclWriter::writeAttributeSource(UMLClassifier * c)
{
    UMLAttributeList list = c->getAttributeList(Uml::Visibility::Public);

    foreach (UMLAttribute* at, list) {
        QString name = mClassGlobal + QLatin1String("::") + cleanName(at->name());

        writeComm(name);
        writeCode(QLatin1String("configbody ") + name + QLatin1String(" {} {") + m_endl + QLatin1Char('}') + m_endl);
    }
}

void TclWriter::writeAssociationSource(UMLAssociationList associations,
                                  Uml::ID::Type id)
{
    if (associations.isEmpty()) {
        return;
    }

    bool printRoleA = false, printRoleB = false;
    foreach (UMLAssociation * a, associations) {

        // it may seem counter intuitive, but you want to insert the role of the
        // *other* class into *this* class.
        if (a->getObjectId(Uml::RoleType::A) == id && !a->getRoleName(Uml::RoleType::B).isEmpty())
            printRoleB = true;

        if (a->getObjectId(Uml::RoleType::B) == id && !a->getRoleName(Uml::RoleType::A).isEmpty())
            printRoleA = true;

        // print RoleB source
        if (printRoleB && a->visibility(Uml::RoleType::B) == Uml::Visibility::Public) {

            QString fieldClassName =
                cleanName(getUMLObjectName(a->getObject(Uml::RoleType::B)));
            writeAssociationRoleSource(fieldClassName, a->getRoleName(Uml::RoleType::B),
                                       a->getMultiplicity(Uml::RoleType::B));
        }
        // print RoleA source
        if (printRoleA && a->visibility(Uml::RoleType::A) == Uml::Visibility::Public) {
            QString fieldClassName =
                cleanName(getUMLObjectName(a->getObject(Uml::RoleType::A)));
            writeAssociationRoleSource(fieldClassName, a->getRoleName(Uml::RoleType::A),
                                       a->getMultiplicity(Uml::RoleType::A));
        }
        // reset for next association in our loop
        printRoleA = false;
        printRoleB = false;
    }
}

void TclWriter::writeAssociationRoleSource(const QString &fieldClassName,
                                      const QString &roleName, const QString &multi)
{
    // ONLY write out IF there is a rolename given
    // otherwise it is not meant to be declared in the code
    if (roleName.isEmpty())
        return;

    // declare the association based on whether it is this a single variable
    // or a List (Vector). One day this will be done correctly with special
    // multiplicity object that we don't have to figure out what it means via regex.
    if (multi.isEmpty() || multi.contains(QRegExp(QLatin1String("^[01]$")))) {
        QString fieldVarName = roleName.toLower();

        writeCode(QLatin1String("configbody ") + mClassGlobal + QLatin1String("::") + fieldVarName + QLatin1String(" {} {"));
        m_indentLevel++;
        writeCode(QLatin1String("if {![$") + fieldVarName + QLatin1String(" isa ") + fieldClassName + QLatin1String("]} {"));
        m_indentLevel++;
        writeCode(QLatin1String("return -code error \"expected object of class: ") +
                  fieldClassName + QLatin1String("\""));
        m_indentLevel--;
        writeCode(QLatin1String("}"));
        m_indentLevel--;

    } else {
        QString fieldVarName = roleName.toLower();

        writeCode(QLatin1String("configbody ") + mClassGlobal + QLatin1String("::") + fieldVarName + QLatin1String(" {} {"));
        m_indentLevel++;
        writeCode(QLatin1String("foreach myObj $") + fieldVarName + QLatin1String(" {"));
        m_indentLevel++;
        writeCode(QLatin1String("if {![$myObj isa ") + fieldClassName + QLatin1String("]} {"));
        m_indentLevel++;
        writeCode(QLatin1String("return -code error \"expected object of class: ") +
                  fieldClassName + QLatin1String("\""));
        m_indentLevel--;
        writeCode(QLatin1String("}"));
        m_indentLevel--;
        writeCode(QLatin1String("}"));
        m_indentLevel--;
    }
    writeCode(QLatin1Char('}') + m_endl);
}

/**
 * Replaces `string' with STRING_TYPENAME.
 */
QString TclWriter::fixTypeName(const QString &string)
{
    if (string.isEmpty())
        return QLatin1String("void");
    return string;
}

/**
 * Returns the name of the given object (if it exists).
 * Methods like this _shouldn't_ be needed IF we properly did things thruought the code.
 */
QString TclWriter::getUMLObjectName(UMLObject * obj)
{
    return (obj != 0) ? obj->name() : QLatin1String("NULL");
}

/**
 * Get list of reserved keywords.
 * @return   the list of reserved keywords
 */
QStringList TclWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        for (int i = 0; reserved_words[i]; ++i) {
            keywords.append(QLatin1String(reserved_words[i]));
        }
    }

    return keywords;
}
