/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

//
// C++ Implementation: valawriter
//

#include "valawriter.h"

#include "association.h"
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "folder.h"
#include "operation.h"
#include "uml.h"
#include "umldoc.h"

#include <QRegularExpression>
#include <QTextStream>

static const char *reserved_words[] = {
    "abstract",
    "as",
    "base",
    "bool",
    "break",
    "byte",
    "case",
    "catch",
    "char",
    "checked",
    "class",
    "const",
    "continue",
    "decimal",
    "default",
    "delegate",
    "do",
    "double",
    "else",
    "enum",
    "event",
    "explicit",
    "extern",
    "false",
    "finally",
    "for",
    "foreach",
    "goto",
    "if",
    "implicit",
    "in",
    "int",
    "interface",
    "internal",
    "is",
    "lock",
    "long",
    "namespace",
    "new",
    "null",
    "object",
    "operator",
    "out",
    "override",
    "params",
    "private",
    "protected",
    "public",
    "readonly",
    "ref",
    "return",
    "sbyte",
    "sealed",
    "short",
    "sizeof",
    "stackalloc",
    "static",
    "string",
    "struct",
    "switch",
    "this",
    "throw",
    "true",
    "try",
    "typeof",
    "uint",
    "ulong",
    "unchecked",
    "unsafe",
    "ushort",
    "using",
    "virtual",
    "void",
    "volatile",
    "while",
    nullptr
};

/**
 * Constructor.
 */
ValaWriter::ValaWriter()
 : SimpleCodeGenerator(),
   m_unnamedRoles(0)
{
}

/**
 * Destructor.
 */
ValaWriter::~ValaWriter()
{
}

/**
 * Get list of predefined data types.
 * @return  the list of default data types
 */
QStringList ValaWriter::defaultDatatypes() const
{
    QStringList l;
    l.append(QStringLiteral("bool"));
    l.append(QStringLiteral("char"));
    l.append(QStringLiteral("uchar"));
    l.append(QStringLiteral("unichar"));
    l.append(QStringLiteral("int"));
    l.append(QStringLiteral("int8"));
    l.append(QStringLiteral("int16"));
    l.append(QStringLiteral("int32"));
    l.append(QStringLiteral("int64"));
    l.append(QStringLiteral("uint"));
    l.append(QStringLiteral("uint8"));
    l.append(QStringLiteral("uint16"));
    l.append(QStringLiteral("uint32"));
    l.append(QStringLiteral("uint64"));
    l.append(QStringLiteral("long"));
    l.append(QStringLiteral("ulong"));
    l.append(QStringLiteral("short"));
    l.append(QStringLiteral("ushort"));
    l.append(QStringLiteral("float"));
    l.append(QStringLiteral("double"));
    l.append(QStringLiteral("struct"));
    l.append(QStringLiteral("enum"));
    l.append(QStringLiteral("string"));
    l.append(QStringLiteral("bool[]"));
    l.append(QStringLiteral("char[]"));
    l.append(QStringLiteral("uchar[]"));
    l.append(QStringLiteral("unichar[]"));
    l.append(QStringLiteral("int[]"));
    l.append(QStringLiteral("int8[]"));
    l.append(QStringLiteral("int16[]"));
    l.append(QStringLiteral("int32[]"));
    l.append(QStringLiteral("int64[]"));
    l.append(QStringLiteral("uint[]"));
    l.append(QStringLiteral("uint8[]"));
    l.append(QStringLiteral("uint16[]"));
    l.append(QStringLiteral("uint32[]"));
    l.append(QStringLiteral("uint64[]"));
    l.append(QStringLiteral("long[]"));
    l.append(QStringLiteral("ulong[]"));
    l.append(QStringLiteral("short[]"));
    l.append(QStringLiteral("ushort[]"));
    l.append(QStringLiteral("float[]"));
    l.append(QStringLiteral("double[]"));
    l.append(QStringLiteral("struct[]"));
    l.append(QStringLiteral("enum[]"));
    l.append(QStringLiteral("string[]"));
    return l;
}

/**
 * Call this method to generate Vala code for a UMLClassifier
 * @param c the class you want to generate code for.
 */
void ValaWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        logWarn0("ValaWriter::writeClassL Cannot write class of NULL classifier");
        return;
    }

    QString classname = cleanName(c->name());
    //find an appropriate name for our file
    QString fileName = findFileName(c, QStringLiteral(".vala"));
    if (fileName.isEmpty()) {
        Q_EMIT codeGenerated(c, false);
        return;
    }

    QFile filecs;
    if (!openFile(filecs, fileName)) {
        Q_EMIT codeGenerated(c, false);
        return;
    }
    QTextStream cs(&filecs);

    //////////////////////////////
    //Start generating the code!!
    /////////////////////////////

    //try to find a heading file (license, comments, etc)
    QString str;
    str = getHeadingFile(QStringLiteral(".vala"));
    if (!str.isEmpty()) {
        str.replace(QRegularExpression(QStringLiteral("%filename%")), fileName);
        str.replace(QRegularExpression(QStringLiteral("%filepath%")), filecs.fileName());
        cs << str << m_endl;
    }

    UMLDoc *umldoc = UMLApp::app()->document();
    UMLFolder *logicalView = umldoc->rootFolder(Uml::ModelType::Logical);

    // write generic includes
    //cs << "using GLib;" << m_endl;
    //cs << "using System.Text;" << m_endl;
    //cs << "using System.Collections;" << m_endl;
    cs << "using GLib;" << m_endl << m_endl;

    //write includes and namespace
    UMLPackage *container = c->umlPackage();
    if (container == logicalView) {
        container = nullptr;
    }

    UMLPackageList includes;
    findObjectsRelated(c, includes);
    m_seenIncludes.clear();
    //m_seenIncludes.append(logicalView);
    if (includes.count()) {
        for (UMLPackage* p :  includes) {
            UMLClassifier *cl = p->asUMLClassifier();
            if (cl) {
                p = cl->umlPackage();
            }
            if (p != logicalView && m_seenIncludes.indexOf(p) == -1 && p != container) {
                cs << "using " << p->fullyQualifiedName(QStringLiteral(".")) << ";" << m_endl;
                m_seenIncludes.append(p);
            }
        }
        cs << m_endl;
    }

    m_container_indent = QString();

    if (container) {
        cs << "namespace " << container->fullyQualifiedName(QStringLiteral(".")) << m_endl;
        cs << "{" << m_endl << m_endl;
        m_container_indent = m_indentation;
        m_seenIncludes.append(container);
    }

    //Write class Documentation if there is something or if force option
    if (forceDoc() || !c->doc().isEmpty()) {
        cs << m_container_indent << "/**" << m_endl;
        if (c->doc().isEmpty()) {
            cs << formatDoc(c->doc(), m_container_indent + QStringLiteral(" * TODO: Add documentation here."));
        }
        else {
            cs << formatDoc(c->doc(), m_container_indent + m_indentation + QStringLiteral(" * "));
        }
        cs << m_container_indent << " */" << m_endl ;
    }

    UMLClassifierList superclasses = c->getSuperClasses();
    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();
    UMLAssociationList realizations = c->getRealizations();
    bool isInterface = c->isInterface();
    m_unnamedRoles = 1;

    cs << m_container_indent << "public ";

    //check if it is an interface or regular class
    if (isInterface) {
        cs << "interface " << classname;
    }
    else {
        //check if class is abstract and / or has abstract methods
        if (c->isAbstract() || c->hasAbstractOps()) {
            cs << "abstract ";
        }

        cs << "class " << classname << (superclasses.count() > 0 ? QStringLiteral(" : ") : QString());

        // write baseclass, ignore interfaces, write error on multiple inheritance
        if (superclasses.count() > 0) {
            int supers = 0;
            for (UMLClassifier* obj :  superclasses) {
                if (!obj->isInterface()) {
                    if (supers > 0) {
                        cs << " // AND ";
                    }
                    cs << cleanName(obj->name());
                    supers++;
                }
            }
            if (supers > 1) {
                cs << m_endl << "//WARNING: Vala does not support multiple inheritance but there is more than 1 superclass defined in your UML model!" << m_endl;
            }
        }
        //check for realizations
        UMLAssociationList realizations = c->getRealizations();

        if (!realizations.isEmpty()) {
            for (UMLAssociation* a :  realizations) {
                UMLClassifier *real = (UMLClassifier*)a->getObject(Uml::RoleType::B);
                if(real != c) {
                    // write list of realizations
                    cs << ", " << real->name();
                }
            }
        }
    }
    cs << m_endl << m_container_indent << '{' << m_endl;

    //associations
    if (forceSections() || !aggregations.isEmpty()) {
        cs << m_endl << m_container_indent << m_indentation << "//region Aggregations" << m_endl << m_endl;
        writeAssociatedAttributes(aggregations, c, cs);
        cs << m_endl << m_container_indent << m_indentation << "//endregion" << m_endl;
    }

    //compositions
    if (forceSections() || !compositions.isEmpty()) {
        cs << m_endl << m_container_indent << m_indentation << "//region Compositions" << m_endl << m_endl;
        writeAssociatedAttributes(compositions, c, cs);
        cs << m_endl << m_container_indent << m_indentation << "//endregion" << m_endl;
    }

    //attributes
    // FIXME: Vala allows Properties in interface!
    if (!isInterface) {
        writeAttributes(c, cs);
    }

    //operations
    writeOperations(c, cs);

    //finish file
    cs << m_endl << m_container_indent << "}" << m_endl << m_endl; // close class

    if (container) {
        cs << "}  // end of namespace "
            << container->fullyQualifiedName(QStringLiteral(".")) << m_endl << m_endl;
    }

    //close files and notfiy we are done
    filecs.close();
    Q_EMIT codeGenerated(c, true);
    Q_EMIT showGeneratedFile(filecs.fileName());
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

/**
 * Write all operations for a given class.
 * @param c the classifier we are generating code for
 * @param cs output stream
 */
void ValaWriter::writeOperations(UMLClassifier *c, QTextStream &cs)
{
    //Lists to store operations  sorted by scope
    UMLOperationList oppub, opprot, oppriv;

    bool isInterface = c->isInterface();
    bool generateErrorStub = true;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList opl(c->getOpList());
    for (UMLOperation* op :   opl) {
        switch (op->visibility()) {
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

    // write realizations (recursive)
    UMLAssociationList realizations = c->getRealizations();

    if (!isInterface && !realizations.isEmpty()) {
        writeRealizationsRecursive(c, &realizations, cs);
    }

    // write public operations
    if (forceSections() || !oppub.isEmpty()) {
        cs << m_endl << m_container_indent << m_indentation << "//region Public methods" << m_endl << m_endl;
        writeOperations(oppub, cs, isInterface, false, generateErrorStub);
        cs << m_container_indent << m_indentation << "//endregion" << m_endl << m_endl;
    }

    // write protected operations
    if (forceSections() || !opprot.isEmpty()) {
        cs << m_endl << m_container_indent << m_indentation << "//region Protected methods" << m_endl << m_endl;
        writeOperations(opprot, cs, isInterface, false, generateErrorStub);
        cs << m_container_indent << m_indentation << "//endregion" << m_endl << m_endl;
    }

    // write private operations
    if (forceSections() || !oppriv.isEmpty()) {
        cs << m_endl << m_container_indent << m_indentation << "//region Private methods" << m_endl << m_endl;
        writeOperations(oppriv, cs, isInterface, false, generateErrorStub);
        cs << m_container_indent << m_indentation << "//endregion" << m_endl << m_endl;
    }

    // write superclasses abstract methods
    UMLClassifierList superclasses = c->getSuperClasses();

    if (!isInterface && !c->isAbstract() && !c->hasAbstractOps() && superclasses.count() > 0) {
        writeOverridesRecursive(&superclasses, cs);
    }

}

/**
 * Write superclasses' abstract methods.
 * @param superclasses List of superclasses to start recursing on
 * @param cs output stream
 */
void ValaWriter::writeOverridesRecursive(UMLClassifierList *superclasses, QTextStream &cs)
{
    // oplist for implemented abstract operations
    UMLOperationList opabstract;

    for (UMLClassifier* obj :  *superclasses) {
        if (!obj->isInterface() && obj->hasAbstractOps()) {
            // collect abstract ops
            UMLOperationList opl(obj->getOpList());
            for (UMLOperation* op :  opl) {
                if (op->isAbstract()) {
                    opabstract.append(op);
                }
            }

            // write abstract implementations
            cs << m_endl << m_container_indent << m_indentation << "//region " << obj->name() << " members" << m_endl << m_endl;
            writeOperations(opabstract, cs, false, true, true);
            cs << m_container_indent << m_indentation << "//endregion" << m_endl << m_endl;

            opabstract.clear();
        }
        // Recurse to parent superclasses
        UMLClassifierList superRecursive = obj->getSuperClasses();
        UMLClassifierList *superRecursivePtr =& superRecursive;
        if (superRecursivePtr->count() > 0) {
            writeOverridesRecursive(superRecursivePtr, cs);
        }
    }
}

/**
 * Write realizations of a class and recurse to parent classes.
 * @param currentClass class to start with
 * @param realizations realizations of this class
 * @param cs output stream
 */
void ValaWriter::writeRealizationsRecursive(UMLClassifier *currentClass, UMLAssociationList *realizations, QTextStream &cs)
{
    for (UMLAssociationListIt alit(*realizations); alit.hasNext();) {
        UMLAssociation *a = alit.next();

        // we know it is a classifier if it is in the list
        UMLClassifier *real = (UMLClassifier*)a->getObject(Uml::RoleType::B);

        //FIXME: Interfaces realize themselves without this condition!?
        if (real == currentClass) {
            continue;
        }

        // collect operations of one realization
        UMLOperationList opreal = real->getOpList();

        // write realizations
        cs << m_endl << m_container_indent << m_indentation << "//region " << real->name() << " members" << m_endl << m_endl;
        writeOperations(opreal, cs, false, false, true);
        cs << m_container_indent << m_indentation << "//endregion" << m_endl << m_endl;

        // Recurse to parent realizations
        UMLAssociationList parentReal = real->getRealizations();
        if (!parentReal.isEmpty()) {
            writeRealizationsRecursive(real, &parentReal, cs);
        }
    }
}

/**
 * Write a list of class operations.
 * @param opList the list of operations
 * @param cs output stream
 * @param isInterface indicates if the operation is an interface member
 * @param isOverride implementation of an inherited abstract function
 * @param generateErrorStub  true generates a comment "The method or operation is not implemented"
 */
void ValaWriter::writeOperations(UMLOperationList opList,
                                 QTextStream &cs, bool isInterface /* = false */,
                                 bool isOverride /* = false */,
                                 bool generateErrorStub /* = false */)
{
    for (UMLOperation* op :  opList) {
        UMLAttributeList atl = op->getParmList();

        //write method doc if we have doc || if at least one of the params has doc
        bool writeDoc = forceDoc() || !op->doc().isEmpty();

        for (UMLAttribute* at :  atl) {
            writeDoc |= !at->doc().isEmpty();
        }

        //write method documentation
        if (writeDoc && !isOverride) {
            cs << m_container_indent << m_indentation << "/**" << m_endl;
            if (op->doc().isEmpty()) {
                cs << formatDoc(op->doc(), m_container_indent + m_indentation + QStringLiteral(" * TODO: Add documentation here. "));
            }
            else {
                cs << formatDoc(op->doc(), m_container_indent + m_indentation + QStringLiteral(" * "));
            }

            //write parameter documentation
            for (UMLAttribute* at :  atl) {
                if (forceDoc() || !at->doc().isEmpty()) {
                    cs << m_container_indent << m_indentation << " * @param " << cleanName(at->name());
                    QString doc = formatDoc(at->doc(), QString());
                    //removing newlines from parameter doc
                    doc.replace(QLatin1Char('\n'), QLatin1Char(' '));
                    doc.remove(QLatin1Char('\r'));
                    doc.remove(QRegularExpression(QStringLiteral(" $")));
                    cs << doc << m_endl;
                }
            }

            // FIXME: "returns" should contain documentation, not type.
            cs << m_container_indent << m_indentation << " * @return ";
            if (! op->getTypeName().isEmpty()) {
                cs << makeLocalTypeName(op);
            }
            cs << m_endl;
	        cs << m_container_indent << m_indentation << " */" << m_endl;
        }

        // method visibility
        cs << m_container_indent << m_indentation;
        if (!isInterface) {
            if (!isOverride) {
                if (op->isAbstract()) {
                    cs << "abstract ";
                }
                cs << Uml::Visibility::toString(op->visibility()) << " ";
                if (op->isStatic()) {
                    cs << "static ";
                }
            }
            else {
                // method overriding an abstract parent
                cs << Uml::Visibility::toString(op->visibility()) << " override ";
                if (op->isStatic()) {
                    cs << "static ";
                }
            }
        }

        // return type (unless constructor, destructor)
        if (!op->isLifeOperation()) {
            if (op->getTypeName().isEmpty()) {
                cs << "void ";
            }
            else {
                cs << makeLocalTypeName(op) << " ";
            }
        }

        // method name
        cs << cleanName(op->name()) << "(";

        // method parameters
        int i= atl.count();
        int j=0;
        for (UMLAttributeListIt atlIt(atl); atlIt.hasNext(); ++j) {
            UMLAttribute* at = atlIt.next();
            cs << makeLocalTypeName(at) << " " << cleanName(at->name());

            // no initial values in Vala
            //<< (!(at->getInitialValue().isEmpty()) ?
            //    (QStringLiteral(" = ")+at->getInitialValue()) :
            //    QString())
            cs << ((j < i-1) ? QStringLiteral(", ") : QString());
        }
        cs << ")";

        //FIXME: how to control generation of error stub?
        if (!isInterface && (!op->isAbstract() || isOverride)) {
            cs << m_endl << m_container_indent << m_indentation << "{" << m_endl;
            // write source code of operation else throw not implemented exception
            QString sourceCode = op->getSourceCode();
            if (sourceCode.isEmpty()) {
                if (generateErrorStub) {
                    cs << m_container_indent << m_indentation << m_indentation;
                    cs << "//TODO: The method or operation is not implemented." << m_endl;
                    if (!op->getTypeName().isEmpty()) {
                       cs << m_container_indent << m_indentation << m_indentation;
                       cs << "return 0; "<< m_endl;
                    }
                }
            }
            else {
                cs << formatSourceCode(sourceCode, m_container_indent + m_indentation + m_indentation);
            }
            cs << m_container_indent << m_indentation << "}" << m_endl;
        }
        else {
            cs << ';' << m_endl;
        }
        cs << m_endl;
    }
}

/**
 * Write all the attributes of a class.
 * @param c the class we are generating code for
 * @param cs output stream
 */
void ValaWriter::writeAttributes(UMLClassifier *c, QTextStream &cs)
{
    UMLAttributeList  atpub, atprot, atpriv, atdefval;

    //sort attributes by scope and see if they have a default value
    UMLAttributeList atl = c->getAttributeList();

    for (UMLAttribute* at :  atl) {
        if (!at->getInitialValue().isEmpty())
            atdefval.append(at);
        switch (at->visibility()) {
          case Uml::Visibility::Public:
            atpub.append(at);
            break;
          case Uml::Visibility::Protected:
            atprot.append(at);
            break;
          case Uml::Visibility::Private:
            atpriv.append(at);
            break;
          default:
            break;
        }
    }

    if (forceSections() || atl.count()) {
        cs << m_endl << m_container_indent << m_indentation << "//region Attributes" << m_endl << m_endl;
    }

    // write public attributes
    if (forceSections() || atpub.count()) {
        writeAttributes(atpub, cs);
    }

    // write protected attributes
    if (forceSections() || atprot.count()) {
        writeAttributes(atprot, cs);
    }

    // write private attributes
    if (forceSections() || atpriv.count()) {
        writeAttributes(atpriv, cs);
    }

    if (forceSections() || atl.count()) {
        cs << m_endl << m_container_indent << m_indentation << "//endregion" << m_endl << m_endl;
    }
}

/**
 * Write a list of class attributes.
 * @param atList the list of attributes
 * @param cs output stream
 */
void ValaWriter::writeAttributes(UMLAttributeList &atList, QTextStream &cs)
{
    for (UMLAttribute* at :  atList) {

        bool asProperty = true;
        if (at->visibility() == Uml::Visibility::Private) {
            asProperty = false;
        }
        writeAttribute(at->doc(), at->visibility(), at->isStatic(),
            makeLocalTypeName(at), at->name(), at->getInitialValue(), asProperty, cs);

        cs << m_endl;
    } // end for
    return;
}

/**
 * Write attributes from associated objects (compositions, aggregations).
 * @param associated list of associated objects
 * @param c currently written class, to see association direction
 * @param cs output stream
 */
void ValaWriter::writeAssociatedAttributes(UMLAssociationList &associated, UMLClassifier *c, QTextStream &cs)
{
    for (UMLAssociation *a :   associated) {
        if (c != a->getObject(Uml::RoleType::A)) { // we need to be at the A side
            continue;
        }

        UMLObject *o = a->getObject(Uml::RoleType::B);
        if (o == nullptr) {
            logError0("ValaWriter::writeAssociatedAttributes: composition role B object is NULL");
            continue;
        }
        // Take name and documentation from Role, take type name from the referenced object
        QString roleName = cleanName(a->getRoleName(Uml::RoleType::B));
        QString typeName = cleanName(o->name());
        if (roleName.isEmpty()) {
            roleName = QString::fromLatin1("UnnamedRoleB_%1").arg(m_unnamedRoles++);
        }
        QString roleDoc = a->getRoleDoc(Uml::RoleType::B);

        //FIXME:is this simple condition enough?
        if (a->getMultiplicity(Uml::RoleType::B).isEmpty() || a->getMultiplicity(Uml::RoleType::B) == QStringLiteral("1"))  {
            // normal attribute
            writeAttribute(roleDoc, a->visibility(Uml::RoleType::B), false, typeName, roleName, QString(), (a->visibility(Uml::RoleType::B) != Uml::Visibility::Private), cs);
        }
        else {
            // array
            roleDoc += QStringLiteral("\n(Array of ") + typeName + QLatin1Char(')');
            writeAttribute(roleDoc, a->visibility(Uml::RoleType::B), false, QStringLiteral("ArrayList"), roleName, QString(), (a->visibility(Uml::RoleType::B) != Uml::Visibility::Private), cs);
        }
    }
}

/**
 * Write a single attribute to the output stream.
 * @param doc attribute documentation
 * @param visibility attribute visibility
 * @param isStatic static attribute
 * @param typeName class/type of the attribute
 * @param name name of the attribute
 * @param initialValue initial value given to the attribute at declaration
 * @param asProperty true writes as property (get/set), false writes single line variable
 * @param cs output stream
 */
void ValaWriter::writeAttribute(const QString& doc,
                                  Uml::Visibility::Enum visibility,
                                  bool isStatic,
                                  const QString& typeName,
                                  const QString& name,
                                  const QString& initialValue,
                                  bool asProperty,
                                  QTextStream &cs)
{
    if (forceDoc() || !doc.isEmpty()) {
        cs << m_container_indent << m_indentation << "/**" << m_endl;
        if (doc.isEmpty()) {
            cs << formatDoc(doc, m_container_indent +  m_indentation + QStringLiteral(" * TODO: Add documentation here."));
        }
        else {
            cs << formatDoc(doc, m_container_indent + m_indentation + QStringLiteral(" * "));
        }
        cs << m_container_indent << m_indentation << " */" << m_endl;
    }
    cs << m_container_indent << m_indentation;
    cs << Uml::Visibility::toString(visibility) << " ";
    if (isStatic) {
        cs << "static ";
    }

    //Variable type with/without namespace path
    cs << typeName << " ";

    cs << cleanName(name);

    // FIXME: may need a GUI switch to not generate as Property?

    // Generate as Property if not private
    if (asProperty) {
        cs << m_endl;
        cs << m_container_indent << m_indentation << "{" << m_endl;
        cs << m_container_indent << m_indentation << m_indentation << "get" << m_endl;
        cs << m_container_indent << m_indentation << m_indentation << "{" << m_endl;
        cs << m_container_indent << m_indentation << m_indentation << m_indentation << "return m_" << cleanName(name) << ";" << m_endl;
        cs << m_container_indent << m_indentation << m_indentation << "}" << m_endl;

        cs << m_container_indent << m_indentation << m_indentation << "set" << m_endl;
        cs << m_container_indent << m_indentation << m_indentation << "{" << m_endl;
        cs << m_container_indent << m_indentation << m_indentation << m_indentation << "m_" << cleanName(name) << " = value;" << m_endl;
        cs << m_container_indent << m_indentation << m_indentation << "}" << m_endl;
        cs << m_container_indent << m_indentation << "}" << m_endl;
        cs << m_container_indent << m_indentation << "private ";
        if (isStatic) {
            cs << "static ";
        }
        cs << typeName << " m_" << cleanName(name);
    }

    if (!initialValue.isEmpty()) {
        cs << " = " << initialValue;
    }

    cs << ";" << m_endl << m_endl;
}

/**
 * Find the type in used namespaces, if namespace found return short name, complete otherwise.
 * @param cl Operation or Attribute to check type
 * @return   the local type name
 */
QString ValaWriter::makeLocalTypeName(UMLClassifierListItem *cl)
{
    UMLClassifier *c = cl->getType();
    if (c) {
        UMLPackage *p = c->umlPackage();
        if (m_seenIncludes.indexOf(p) != -1) {
            return c->name();
        }
    }
    return cl->getTypeName();
}

/**
 * Returns "Vala".
 * @return programming language id
 */
Uml::ProgrammingLanguage::Enum ValaWriter::language() const
{
    return Uml::ProgrammingLanguage::Vala;
}

/**
 * Get list of reserved keywords.
 * @return list of reserved keywords
 */
QStringList ValaWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        for (int i = 0; reserved_words[i]; ++i) {
            keywords.append(QLatin1String(reserved_words[i]));
        }
    }

    return keywords;
}

