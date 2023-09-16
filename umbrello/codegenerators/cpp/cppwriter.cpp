/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas
    <brian.thomas@gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors
    <umbrello-devel@kde.org>
*/

// own header
#include "cppwriter.h"

// app includes
#include "association.h"
#include "classifier.h"
#include "codegen_utils.h"
#include "datatype.h"
#include "debug_utils.h"
#include "model_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "operation.h"
#include "template.h"
#include "umltemplatelist.h"
#include "umlclassifierlistitemlist.h"
#include "classifierlistitem.h"
#include "codegenerationpolicy.h"
#include "enumliteral.h"

// qt includes
#include <QFile>
#include <QRegExp>
#include <QTextStream>

// 3-14-2003: this code developed from the javawriter with parts of the
// original cppwriter by Luis De la Parra Blum

/**
 * Constructor, initialises a couple of variables.
 */
CppWriter::CppWriter()
  : m_stringIncludeRequired(false)
{
    // Probably we could resolve this better through the use of templates,
    // but it is a quick n dirty fix for the time being.. until codegeneration
    // template system is in place.
    // You can insert code here. 3 general variables exist: "%VARNAME%"
    // allows you to specify where the vector variable should be in your code,
    // and "%ITEMCLASS%", if needed, where the class of the item is declared.
    VECTOR_METHOD_APPEND = QStringLiteral("%VARNAME%.push_back(add_object);"); // for std::vector
    VECTOR_METHOD_REMOVE = QString(QStringLiteral("int i, size = %VARNAME%.size();\nfor (i = 0; i < size; ++i) {\n\t%ITEMCLASS% item = %VARNAME%.at(i);\n\tif(item == remove_object) {\n\t\t%1<%ITEMCLASS%>::iterator it = %VARNAME%.begin() + i;\n\t\t%VARNAME%.erase(it);\n\t\treturn;\n\t}\n }")).arg(policyExt()->getVectorClassName()); // for std::vector
    VECTOR_METHOD_INIT.clear(); // nothing to be done
    /*
        VECTOR_METHOD_APPEND = QStringLiteral("%VARNAME%.append(&add_object);"); // Qt lib implementation
        VECTOR_METHOD_REMOVE = QStringLiteral("%VARNAME%.removeRef(&remove_object);"); // Qt lib implementation
        VECTOR_METHOD_INIT = QStringLiteral("%VARNAME%.setAutoDelete(false);"); // Qt library
    */

    OBJECT_METHOD_INIT = QStringLiteral("%VARNAME% = new %ITEMCLASS%();"); // Qt library

    // boolean config params
    INLINE_ASSOCIATION_METHODS = false;
}

/**
 * Destructor, empty.
 */
CppWriter::~CppWriter()
{
}

/**
 * Returns "C++".
 * @return   the programming language identifier
 */
Uml::ProgrammingLanguage::Enum CppWriter::language() const
{
    return Uml::ProgrammingLanguage::Cpp;
}

/**
 * Return the policy object.
 */
CPPCodeGenerationPolicy *CppWriter::policyExt()
{
    return static_cast<CPPCodeGenerationPolicy*>(UMLApp::app()->policyExt());
}

/**
 * Call this method to generate cpp code for a UMLClassifier.
 * @param c   the class to generate code for
 */
void CppWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        logWarn0("CppWriter::writeClass: Cannot write class of NULL classifier!");
        return;
    }

    QFile fileh, filecpp;

    // find an appropriate name for our file
    fileName_ = findFileName(c, QStringLiteral(".h"));
    if (fileName_.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    className_ = cleanName(c->name());

    if (c->visibility() != Uml::Visibility::Implementation) {
        if (!openFile(fileh, fileName_)) {
            emit codeGenerated(c, false);
            return;
        }
        // write Header file
        writeHeaderFile(c, fileh);
        fileh.close();
    }

    // Determine whether the implementation file is required.
    // (It is not required if the class is an enumeration.)
    bool need_impl = true;
    if (c->baseType() == UMLObject::ot_Enum || c->isInterface()) {
        need_impl = false;
    }
    if (need_impl) {
        fileName_.replace(QRegExp(QStringLiteral(".h$")), QStringLiteral(".cpp"));
        if (!openFile(filecpp, fileName_)) {
            emit codeGenerated(c, false);
            return;
        }
        // write Source file
        writeSourceFile(c, filecpp);
        filecpp.close();
    }

    emit codeGenerated(c, true);
}

/**
 * Write the header file for this classifier.
 */
void CppWriter::writeHeaderFile (UMLClassifier *c, QFile &file)
{
    // open stream for writing
    QTextStream h(&file);

    // write header blurb
    QString str = getHeadingFile(QStringLiteral(".h"));
    if (!str.isEmpty()) {
        str.replace(QRegExp(QStringLiteral("%filename%")), fileName_ + QStringLiteral(".h"));
        str.replace(QRegExp(QStringLiteral("%filepath%")), file.fileName());
        h << str<< m_endl;
    }

    // Write the hash define stuff to prevent multiple parsing/inclusion of header
    QString hashDefine = className_.toUpper().simplified().replace(QRegExp(QStringLiteral(" ")),  QStringLiteral("_"));
    writeBlankLine(h);
    h << "#ifndef "<< hashDefine << "_H" << m_endl;
    h << "#define "<< hashDefine << "_H" << m_endl;
    h << m_endl;

    QString s;
    QTextStream t(&s);
    writeClassDecl(c, t);
    writeIncludes(c, h);
    h << s;

    // last thing..close our hashdefine
    h << m_endl << "#endif // " << hashDefine << "_H" << m_endl;
}

void CppWriter::writeHeaderAccessorMethodDecl(UMLClassifier *c, Uml::Visibility::Enum permitScope, QTextStream &stream)
{
    // attributes
    writeHeaderAttributeAccessorMethods(c, permitScope, true, stream); // write static attributes first
    writeHeaderAttributeAccessorMethods(c, permitScope, false, stream);

    // associations
    writeAssociationMethods(c->getSpecificAssocs(Uml::AssociationType::Association), permitScope,
                            true, INLINE_ASSOCIATION_METHODS, true, c->id(), stream);
    writeAssociationMethods(c->getUniAssociationToBeImplemented(), permitScope,
                            true, INLINE_ASSOCIATION_METHODS, true, c->id(), stream);
    writeAssociationMethods(c->getAggregations(), permitScope,
                            true,  INLINE_ASSOCIATION_METHODS, true, c->id(), stream);
    writeAssociationMethods(c->getCompositions(), permitScope,
                            true, INLINE_ASSOCIATION_METHODS, false, c->id(), stream);

    writeBlankLine(stream);
}

/**
 * Write out fields and operations for this class selected on a particular
 * visibility.
 */
void CppWriter::writeHeaderFieldDecl(UMLClassifier *c, Uml::Visibility::Enum permitScope, QTextStream &stream)
{
    // attributes
    writeAttributeDecls(c, permitScope, true, stream); // write static attributes first
    writeAttributeDecls(c, permitScope, false, stream);

    // associations
    writeAssociationDecls(c->getSpecificAssocs(Uml::AssociationType::Association), permitScope, c->id(), stream);
    writeAssociationDecls(c->getUniAssociationToBeImplemented(), permitScope, c->id(), stream);
    writeAssociationDecls(c->getAggregations(), permitScope, c->id(), stream);
    writeAssociationDecls(c->getCompositions(), permitScope, c->id(), stream);
}

/**
 * Write the source code body file for this classifier.
 */
void CppWriter::writeSourceFile(UMLClassifier *c, QFile &file)
{
    // open stream for writing
    QTextStream cpp (&file);

    // set the starting indentation at zero
    m_indentLevel = 0;

    //try to find a heading file (license, comments, etc)
    QString str;
    str = getHeadingFile(QStringLiteral(".cpp"));
    if (!str.isEmpty()) {
        str.replace(QRegExp(QStringLiteral("%filename%")), fileName_ + QStringLiteral(".cpp"));
        str.replace(QRegExp(QStringLiteral("%filepath%")), file.fileName());
        cpp << str << m_endl;
    }

    // IMPORT statements
    // Q: Why all utils? Isnt just List and Vector the only classes we are using?
    // Our import *should* also look at operations, and check that objects being
    // used arent in another package (and thus need to be explicitly imported here).
    cpp << "#include \"" << className_ << ".h\"" << m_endl;
    writeBlankLine(cpp);

    if (c->visibility() == Uml::Visibility::Implementation) {
        writeClassDecl(c, cpp);
    }

    // Start body of class

    // Constructors: anything we more we need to do here ?
    //
    if (!c->isInterface())
        writeConstructorMethods(c, cpp);

    // METHODS
    //

    // write comment for section IF needed
    QString indnt = indent();
    if (forceSections() || c->hasAccessorMethods() || c->hasOperationMethods()) {
        writeComment(QStringLiteral("Methods"), indnt, cpp);
        writeBlankLine(cpp);
        writeBlankLine(cpp);
    }

    // write comment for sub-section IF needed
    if (forceSections() || c->hasAccessorMethods()) {
        writeComment(QStringLiteral("Accessor methods"), indnt, cpp);
        writeBlankLine(cpp);
        writeBlankLine(cpp);
    }

    // Accessor methods for attributes
    const bool bInlineAccessors = policyExt()->getAccessorsAreInline();
    if (!bInlineAccessors && c->hasAttributes()) {
        writeAttributeMethods(c->getAttributeListStatic(Uml::Visibility::Public), Uml::Visibility::Public, false, true, !bInlineAccessors, cpp);
        writeAttributeMethods(c->getAttributeList(Uml::Visibility::Public), Uml::Visibility::Public, false, false, !bInlineAccessors, cpp);
        writeAttributeMethods(c->getAttributeListStatic(Uml::Visibility::Protected), Uml::Visibility::Protected, false, true, !bInlineAccessors, cpp);
        writeAttributeMethods(c->getAttributeList(Uml::Visibility::Protected), Uml::Visibility::Protected, false, false, !bInlineAccessors, cpp);
        writeAttributeMethods(c->getAttributeListStatic(Uml::Visibility::Private), Uml::Visibility::Private, false, true, !bInlineAccessors, cpp);
        writeAttributeMethods(c->getAttributeList(Uml::Visibility::Private), Uml::Visibility::Private, false, false, !bInlineAccessors, cpp);
    }

    // accessor methods for associations

    // public
    writeAssociationMethods(c->getSpecificAssocs(Uml::AssociationType::Association), Uml::Visibility::Public, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->id(), cpp);
    writeAssociationMethods(c->getUniAssociationToBeImplemented(), Uml::Visibility::Public, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->id(), cpp);
    writeAssociationMethods(c->getAggregations(), Uml::Visibility::Public, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->id(), cpp);
    writeAssociationMethods(c->getCompositions(), Uml::Visibility::Public, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->id(), cpp);

    // protected
    writeAssociationMethods(c->getSpecificAssocs(Uml::AssociationType::Association), Uml::Visibility::Protected, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->id(), cpp);
    writeAssociationMethods(c->getUniAssociationToBeImplemented(), Uml::Visibility::Protected, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->id(), cpp);
    writeAssociationMethods(c->getAggregations(), Uml::Visibility::Protected, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->id(), cpp);
    writeAssociationMethods(c->getCompositions(), Uml::Visibility::Protected, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->id(), cpp);

    // private
    writeAssociationMethods(c->getSpecificAssocs(Uml::AssociationType::Association), Uml::Visibility::Private, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->id(), cpp);
    writeAssociationMethods(c->getUniAssociationToBeImplemented(), Uml::Visibility::Private, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->id(), cpp);
    writeAssociationMethods(c->getAggregations(), Uml::Visibility::Private, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->id(), cpp);
    writeAssociationMethods(c->getCompositions(), Uml::Visibility::Private, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->id(), cpp);
    writeBlankLine(cpp);

    // Other operation methods -- all other operations are now written
    //

    // write comment for sub-section IF needed
    if (forceSections() || c->hasOperationMethods()) {
        writeComment(QStringLiteral("Other methods"), indnt, cpp);
        writeBlankLine(cpp);
        writeBlankLine(cpp);
    }

    if (!policyExt()->getOperationsAreInline()) {
        writeOperations(c, false, Uml::Visibility::Public, cpp);
        writeOperations(c, false, Uml::Visibility::Protected, cpp);
        writeOperations(c, false, Uml::Visibility::Private, cpp);
    }

    // Yep, bringing up the back of the bus, our initialization method for attributes
    writeInitAttributeMethod(c, cpp);

    writeBlankLine(cpp);
}

/**
 * write includes
 * @param c uml classifier
 * @param stream text stream
 */
void CppWriter::writeIncludes(UMLClassifier *c, QTextStream &stream)
{
    UMLClassifierList superclasses = c->getSuperClasses();
    foreach (UMLClassifier* classifier, superclasses) {
        QString headerName = findFileName(classifier, QStringLiteral(".h"));
        if (!headerName.isEmpty()) {
            stream << "#include \"" << headerName << "\"" << m_endl;
        }
    }
    if (superclasses.size() > 0)
        writeBlankLine(stream);

    if (m_stringIncludeRequired)
        stream << "#include " << policyExt()->getStringClassNameInclude() << m_endl;

    if (c->hasVectorFields())
        stream << "#include " << policyExt()->getVectorClassNameInclude() << m_endl;

    if (m_stringIncludeRequired || c->hasVectorFields())
        writeBlankLine(stream);
}

/**
 * Writes class's documentation to the class header
 * "public abstract class Foo extents {".
 */
void CppWriter::writeClassDecl(UMLClassifier *c, QTextStream &cpp)
{
    if (c->hasAssociations())
    {
        // write all includes we need to include other classes, that arent us.
        printAssociationIncludeDecl (c->getSpecificAssocs(Uml::AssociationType::Association), c->id(), cpp);
        printAssociationIncludeDecl (c->getUniAssociationToBeImplemented(), c->id(), cpp);
        printAssociationIncludeDecl (c->getAggregations(), c->id(), cpp);
        printAssociationIncludeDecl (c->getCompositions(), c->id(), cpp);

        writeBlankLine(cpp);
    }

    foreach (UMLClassifier* classifier, c->getSuperClasses()) {
        if (classifier->package()!=c->package() && !classifier->package().isEmpty()) {
            cpp << "using " << cleanName(classifier->package()) << "::" << cleanName(classifier->name()) << ";" << m_endl;
        }
    }

    if (!c->package().isEmpty() && policyExt()->getPackageIsNamespace())
        cpp << m_endl << "namespace " << cleanName(c->package()) << " {" << m_endl << m_endl;

    //Write class Documentation if there is something or if force option
    if (forceDoc() || !c->doc().isEmpty()) {
        writeDocumentation(QString(), QStringLiteral("class ") + className_, c->doc(), cpp);
        writeBlankLine(cpp);
    }

    // Up the indent level to one.
    // Do this _after_ call to writeDocumentation() because the class documentation
    // shall not be indented.
    m_indentLevel = 1;

    //check if class is abstract and / or has abstract methods
    if ((c->isAbstract() || c->isInterface())
            && !hasAbstractOps(c))
        cpp << "/******************************* Abstract Class ****************************" << m_endl
        << className_ << " does not have any pure virtual methods, but its author" << m_endl
        << "  defined it as an abstract class, so you should not use it directly." << m_endl
        << "  Inherit from it instead and create only objects from the derived classes" << m_endl
        << "*****************************************************************************/" << m_endl << m_endl;

    if (c->baseType() == UMLObject::ot_Enum) {
        UMLClassifierListItemList litList = c->getFilteredList(UMLObject::ot_EnumLiteral);
        uint i = 0;
        cpp << "enum " << className_ << " {" << m_endl;
        foreach (UMLClassifierListItem* lit, litList) {
            UMLEnumLiteral *el = static_cast<UMLEnumLiteral *>(lit);
            QString enumLiteral = cleanName(lit->name());
            cpp << indent() << enumLiteral;
            if (!el->value().isEmpty())
                cpp << " = " << el->value();
            if (++i < (uint)litList.count())
                cpp << ",";
            cpp << m_endl;
        }
        cpp << m_endl << "};" << m_endl;  // end of class header
        if (!c->package().isEmpty() && policyExt()->getPackageIsNamespace())
            cpp << "}  // end of package namespace" << m_endl;
        return;
    }

    // Generate template parameters.
    UMLTemplateList template_params = c->getTemplateList();
    if (template_params.count()) {
        cpp << "template<";
        for (UMLTemplateListIt tlit(template_params); tlit.hasNext();) {
            UMLTemplate* t = tlit.next();
            QString formalName = t->name();
            QString typeName = t->getTypeName();
            cpp << typeName << " " << formalName;
            if (tlit.hasNext()) {
                tlit.next();
                cpp << ", ";
            }
        }
        cpp << ">" << m_endl;
    }

    cpp << "class " << className_;
    uint numOfSuperClasses = c->getSuperClasses().count();
    if (numOfSuperClasses > 0)
        cpp << " : ";
    uint i = 0;
    foreach (UMLClassifier* superClass, c->getSuperClasses()) {
        i++;
        if (superClass->isAbstract() || superClass->isInterface())
            cpp << "virtual ";
        cpp << "public " << cleanName(superClass->name());
        if (i < numOfSuperClasses)
            cpp << ", ";
    }

    cpp << m_endl << "{" << m_endl; // begin the body of the class

    //
    // write out field and operations decl grouped by visibility
    //

    QString s;
    QTextStream tmp(&s);

    // PUBLIC attribs/methods
    writeDataTypes(c, Uml::Visibility::Public, tmp);
    // for public: constructors are first ops we print out
    if (!c->isInterface())
        writeConstructorDecls(tmp);
    writeHeaderFieldDecl(c, Uml::Visibility::Public, tmp);
    writeHeaderAccessorMethodDecl(c, Uml::Visibility::Public, tmp);
    writeOperations(c, true, Uml::Visibility::Public, tmp);
    s = s.trimmed();
    if (!s.isEmpty()) {
        cpp << "public:" << m_endl << indent() << s << m_endl << m_endl;
    }

    s.clear();
    // PROTECTED attribs/methods
    writeDataTypes(c, Uml::Visibility::Protected, tmp);
    writeHeaderFieldDecl(c, Uml::Visibility::Protected, tmp);
    writeHeaderAccessorMethodDecl(c, Uml::Visibility::Protected, tmp);
    writeOperations(c, true, Uml::Visibility::Protected, tmp);
    s = s.trimmed();
    if (!s.isEmpty()) {
        cpp << "protected:" << m_endl << indent() << s << m_endl << m_endl;
    }

    s.clear();
    // PRIVATE attribs/methods
    writeDataTypes(c, Uml::Visibility::Private, tmp);
    writeHeaderFieldDecl(c, Uml::Visibility::Private, tmp);
    writeHeaderAccessorMethodDecl(c, Uml::Visibility::Private, tmp);
    writeOperations(c, true, Uml::Visibility::Private, tmp);
    writeInitAttributeDecl(c, tmp); // this is always private, used by constructors to initialize class
    s = s.trimmed();
    if (!s.isEmpty()) {
        cpp << "private:" << m_endl << indent() << s << m_endl;
    }

    // end of class header
    cpp << m_endl << "};" << m_endl;

    // end of class namespace, if any
    if (!c->package().isEmpty() && policyExt()->getPackageIsNamespace())
        cpp << "} // end of package namespace" << m_endl;

}

/**
 * Writes the attribute declarations.
 * @param c             the classifier
 * @param visibility    the visibility of the attribs to print out
 * @param writeStatic   whether to write static or non-static attributes out
 * @param stream        text stream
 */
void CppWriter::writeAttributeDecls (UMLClassifier *c, Uml::Visibility::Enum visibility, bool writeStatic, QTextStream &stream)
{
    if (c->isInterface())
        return;

    UMLAttributeList list;
    if (writeStatic)
        list = c->getAttributeListStatic(visibility);
    else
        list = c->getAttributeList(visibility);

    //write documentation
    if (forceSections() || list.count() > 0)
    {
        QString strVis = Codegen_Utils::capitalizeFirstLetter(Uml::Visibility::toString(visibility));
        QString strStatic = writeStatic ? QStringLiteral("Static ") : QString();
        writeComment(strStatic + strVis + QStringLiteral(" attributes"), indent(), stream);
        writeBlankLine(stream);
        writeBlankLine(stream);
    }

    if (list.count() > 0) {

        // write attrib declarations now
        // bool isFirstAttrib = true;
        QString documentation;
        foreach (UMLAttribute* at, list) {

            //                  bool noPriorDocExists = documentation.isEmpty();
            documentation = at->doc();

            // add a line for code clarity IF PRIOR attrib has comment on it
            // OR this line has documentation
            //                  if (!isFirstAttrib && (!documentation.isEmpty()||!noPriorDocExists))
            //                          writeBlankLine(stream);

            // isFirstAttrib = false;

            QString varName = getAttributeVariableName(at);

            QString staticValue = at->isStatic() ? QStringLiteral("static ") : QString();
            QString typeName = fixTypeName(at->getTypeName());
            int i = typeName.indexOf(QLatin1Char('['));
            if (i > -1) {
                varName += typeName.mid(i);
                typeName = typeName.left(i);
            }
            if (!documentation.isEmpty())
                writeComment(documentation, indent(), stream);
            stream << indent() << staticValue << typeName << " " << varName << ";" << m_endl;

        }

        /*
        if (list.count() > 0)
            writeBlankLine(stream);
        */
    }
}

void CppWriter::writeHeaderAttributeAccessorMethods (UMLClassifier *c, Uml::Visibility::Enum visibility, bool writeStatic, QTextStream &stream)
{
    // check the current policy about generate accessors as public
    UMLAttributeList list;
    if (writeStatic)
        list = c->getAttributeListStatic(visibility);
    else
        list = c->getAttributeList(visibility);

    // write accessor methods for attribs we found
    // if getAccessorsArePublic policy is true, all attribute accessors are public.
    const Uml::Visibility::Enum vis = policyExt()->getAccessorsArePublic() ? Uml::Visibility::Public : visibility;
    writeAttributeMethods(list, vis, true, writeStatic, policyExt()->getAccessorsAreInline(), stream);
}

/**
 * This is for writing *source* or *header* file attribute methods.
 * Calls @ref writeSingleAttributeAccessorMethods() on each of the attributes in attribs list.
 */
void CppWriter::writeAttributeMethods(UMLAttributeList attribs,
                                      Uml::Visibility::Enum visibility, bool isHeaderMethod,
                                      bool isStatic,
                                      bool writeMethodBody, QTextStream &stream)
{
    if (!policyExt()->getAutoGenerateAccessors())
        return;

    if (forceSections() || attribs.count() > 0)
    {
        QString strVis = Codegen_Utils::capitalizeFirstLetter(Uml::Visibility::toString(visibility));
        QString strStatic = (isStatic ? QStringLiteral(" static") : QString());
        writeBlankLine(stream);
        writeComment(strVis + strStatic + QStringLiteral(" attribute accessor methods"), indent(), stream);
        writeBlankLine(stream);
        writeBlankLine(stream);
    }

    // return now if NO attributes to work on
    if (attribs.count() == 0)
        return;

    foreach (UMLAttribute* at, attribs) {
        QString varName = getAttributeVariableName(at);
        QString methodBaseName = getAttributeMethodBaseName(cleanName(at->name()));

        methodBaseName = methodBaseName.trimmed();
        writeSingleAttributeAccessorMethods(at->getTypeName(), varName,
                                            methodBaseName, at->doc(), Uml::Changeability::Changeable, isHeaderMethod,
                                            at->isStatic(), writeMethodBody, stream);
    }
}

/**
 * Writes a general comment.
 */
void CppWriter::writeComment(const QString &comment, const QString &myIndent, QTextStream &cpp)
{
    CodeGenerationPolicy::CommentStyle const cStyle = UMLApp::app()->commonPolicy()->getCommentStyle();
    QString commentHeadingStr;
    QString intermediateLinePrefixStr;
    QString commentFooterStr;

    if (cStyle == CodeGenerationPolicy::MultiLine) {
        commentHeadingStr         = QStringLiteral("/* ");
        intermediateLinePrefixStr = QStringLiteral(" * ");
        commentFooterStr          = QStringLiteral(" */");
    } else if (cStyle == CodeGenerationPolicy::SingleLine) {
        commentHeadingStr         = QStringLiteral("// ");
        intermediateLinePrefixStr = commentHeadingStr;
    }

    cpp << formatFullDocBlock(comment, myIndent + commentHeadingStr,
                              myIndent + commentFooterStr, myIndent + intermediateLinePrefixStr);
}

/**
 * Writes a documentation comment.
 */
void CppWriter::writeDocumentation(QString header, QString body, QString end, QTextStream &cpp)
{
    CodeGenerationPolicy::CommentStyle const cStyle = UMLApp::app()->commonPolicy()->getCommentStyle();
    QString commentHeadingStr;
    QString intermediateLinePrefixStr;
    QString commentFooterStr;

    if (cStyle == CodeGenerationPolicy::MultiLine) {
      commentHeadingStr         = QStringLiteral("/** ");
      intermediateLinePrefixStr = QStringLiteral(" * ");
      commentFooterStr          = QStringLiteral(" */");
    } else if (cStyle == CodeGenerationPolicy::SingleLine) {
      commentHeadingStr         = QStringLiteral("/// ");
      intermediateLinePrefixStr = commentHeadingStr;
    }
    writeBlankLine(cpp);
    QString indnt = indent();

    cpp << indnt << commentHeadingStr << m_endl;
    if (!header.isEmpty())
        cpp << formatDoc(header, indnt + intermediateLinePrefixStr);
    if (!body.isEmpty())
        cpp << formatDoc(body, indnt + intermediateLinePrefixStr);
    if (!end.isEmpty()) {
        QStringList lines = end.split(QLatin1Char('\n'));
        for (int i = 0; i < lines.count(); ++i) {
            cpp << indnt << intermediateLinePrefixStr << lines[i] << m_endl;
        }
    }
    if (!commentFooterStr.isEmpty()) {
        cpp << indnt << commentFooterStr << m_endl;
    }
}

/**
 * Searches a list of associations for appropriate ones to write out as attributes.
 */
void CppWriter::writeAssociationDecls(UMLAssociationList associations, Uml::Visibility::Enum permitScope, Uml::ID::Type id, QTextStream &h)
{
    if (forceSections() || !associations.isEmpty())
    {
        bool printRoleA = false, printRoleB = false;
        foreach (UMLAssociation *a, associations)
        {
            // it may seem counter intuitive, but you want to insert the role of the
            // *other* class into *this* class.
            if (a->getObjectId(Uml::RoleType::A) == id && !a->getRoleName(Uml::RoleType::B).isEmpty())
                printRoleB = true;

            if (a->getObjectId(Uml::RoleType::B) == id && !a->getRoleName(Uml::RoleType::A).isEmpty())
                printRoleA = true;

            // First: we insert documentation for association IF it has either role AND some documentation (!)
            if ((printRoleA || printRoleB) && !(a->doc().isEmpty()))
                writeComment(a->doc(), indent(), h);

            // print RoleB decl
            if (printRoleB && a->visibility(Uml::RoleType::B) == permitScope)
            {

                QString fieldClassName = cleanName(umlObjectName(a->getObject(Uml::RoleType::B)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::RoleType::B), a->getMultiplicity(Uml::RoleType::B), a->getRoleDoc(Uml::RoleType::B), h);
            }

            // print RoleA decl
            if (printRoleA && a->visibility(Uml::RoleType::A) == permitScope)
            {
                QString fieldClassName = cleanName(umlObjectName(a->getObject(Uml::RoleType::A)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::RoleType::A), a->getMultiplicity(Uml::RoleType::A), a->getRoleDoc(Uml::RoleType::A), h);
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
void CppWriter::writeAssociationRoleDecl(QString fieldClassName, QString roleName, QString multi,
        QString doc, QTextStream &stream)
{
    // ONLY write out IF there is a rolename given
    // otherwise it is not meant to be declared in the code
    if (roleName.isEmpty())
        return;

    QString indnt = indent();

    // always put space between this and prior decl, if any
    writeBlankLine(stream);

    if (!doc.isEmpty())
        writeComment(doc, indnt, stream);

    // declare the association based on whether it is this a single variable
    // or a List (Vector). One day this will be done correctly with special
    // multiplicity object that we don't have to figure out what it means via regex.
    if (multi.isEmpty() || multi.contains(QRegExp(QStringLiteral("^[01]$"))))
    {
        QString fieldVarName = QStringLiteral("m_") + roleName.toLower();

        // record this for later consideration of initialization IF the
        // multi value requires 1 of these objects
        if (ObjectFieldVariables.indexOf(fieldVarName) == -1 &&
                multi.contains(QRegExp(QStringLiteral("^1$"))))
        {
            // ugh. UGLY. Storing variable name and its class in pairs.
            ObjectFieldVariables.append(fieldVarName);
            ObjectFieldVariables.append(fieldClassName);
        }

        stream << indnt << fieldClassName << " * " << fieldVarName << ";" << m_endl;
    }
    else
    {
        QString fieldVarName = QStringLiteral("m_") + roleName.toLower() + QStringLiteral("Vector");

        // record unique occurrences for later when we want to check
        // for initialization of this vector
        if (VectorFieldVariables.indexOf(fieldVarName) == -1)
            VectorFieldVariables.append(fieldVarName);

        stream << indnt << policyExt()->getVectorClassName() << "<" << fieldClassName << "*";
        stream << "> " << fieldVarName << ";" << m_endl;
    }
}

/**
 * Calls @ref writeAssociationRoleMethod() on each of the associations in the given list
 * for either source or header files.
 */
void CppWriter::writeAssociationMethods (UMLAssociationList associations,
        Uml::Visibility::Enum permitVisib,
        bool isHeaderMethod,
        bool writeMethodBody,
        bool writePointerVar,
        Uml::ID::Type myID, QTextStream &stream)
{
    if (forceSections() || !associations.isEmpty())
    {
        foreach (UMLAssociation *a, associations)
        {

            // insert the methods to access the role of the other
            // class in the code of this one
            if (a->getObjectId(Uml::RoleType::A) == myID && a->visibility(Uml::RoleType::B) == permitVisib)
            {
                // only write out IF there is a rolename given
                if (!a->getRoleName(Uml::RoleType::B).isEmpty()) {
                    QString fieldClassName = umlObjectName(a->getObject(Uml::RoleType::B)) + (writePointerVar ? QStringLiteral(" *") : QString());
                    writeAssociationRoleMethod(fieldClassName,
                                               isHeaderMethod,
                                               writeMethodBody,
                                               a->getRoleName(Uml::RoleType::B),
                                               a->getMultiplicity(Uml::RoleType::B), a->getRoleDoc(Uml::RoleType::B),
                                               a->changeability(Uml::RoleType::B), stream);
                }
            }

            if (a->getObjectId(Uml::RoleType::B) == myID && a->visibility(Uml::RoleType::A) == permitVisib)
            {
                // only write out IF there is a rolename given
                if (!a->getRoleName(Uml::RoleType::A).isEmpty()) {
                    QString fieldClassName = umlObjectName(a->getObject(Uml::RoleType::A)) + (writePointerVar ? QStringLiteral(" *") : QString());
                    writeAssociationRoleMethod(fieldClassName,
                                               isHeaderMethod,
                                               writeMethodBody,
                                               a->getRoleName(Uml::RoleType::A),
                                               a->getMultiplicity(Uml::RoleType::A),
                                               a->getRoleDoc(Uml::RoleType::A),
                                               a->changeability(Uml::RoleType::A),
                                               stream);
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
void CppWriter::writeAssociationRoleMethod (const QString &fieldClassName,
        bool isHeaderMethod,
        bool writeMethodBody,
        const QString &roleName, const QString &multi,
        const QString &description, Uml::Changeability::Enum change,
        QTextStream &stream)
{
    if (multi.isEmpty() || multi.contains(QRegExp(QStringLiteral("^[01]$"))))
    {
        QString fieldVarName = QStringLiteral("m_") + roleName.toLower();
        writeSingleAttributeAccessorMethods(fieldClassName, fieldVarName, roleName,
                                            description, change, isHeaderMethod, false, writeMethodBody, stream);
    }
    else
    {
        QString fieldVarName = QStringLiteral("m_") + roleName.toLower() + QStringLiteral("Vector");
        writeVectorAttributeAccessorMethods(fieldClassName, fieldVarName, roleName,
                                            description, change, isHeaderMethod, writeMethodBody, stream);
    }
}

/**
 * Writes addFoo() and removeFoo() accessor methods for the Vector attribute.
 */
void CppWriter::writeVectorAttributeAccessorMethods (
        const QString &fieldClassName, const QString &fieldVarName,
        const QString &fieldName, const QString &description,
        Uml::Changeability::Enum changeType,
        bool isHeaderMethod,
        bool writeMethodBody,
        QTextStream &stream)
{
    QString className = fixTypeName(fieldClassName);
    QString fldName = Codegen_Utils::capitalizeFirstLetter(fieldName);
    QString indnt = indent();

    // ONLY IF changeability is NOT Frozen
    if (changeType != Uml::Changeability::Frozen)
    {
        writeDocumentation(QStringLiteral("Add a ") + fldName + QStringLiteral(" object to the ") + fieldVarName + QStringLiteral(" List"), description, QString(), stream);
        stream << indnt << QStringLiteral("void ");
        if (!isHeaderMethod)
            stream << className_ << "::";
        stream << "add" << fldName << " (" << className << " add_object)";
        if (writeMethodBody) {
            QString method = VECTOR_METHOD_APPEND;
            method.replace(QRegExp(QStringLiteral("%VARNAME%")), fieldVarName);
            method.replace(QRegExp(QStringLiteral("%VECTORTYPENAME%")), policyExt()->getVectorClassName());
            method.replace(QRegExp(QStringLiteral("%ITEMCLASS%")), className);
            stream << indnt << " {" << m_endl;
            m_indentLevel++;
            printTextAsSeparateLinesWithIndent(method, indent(), stream);
            m_indentLevel--;
            stream << indnt << "}" << m_endl;
        } else
            stream << ";" << m_endl;
    }

    // ONLY IF changeability is Changeable
    if (changeType == Uml::Changeability::Changeable)
    {
        writeDocumentation(QStringLiteral("Remove a ") + fldName + QStringLiteral(" object from ") + fieldVarName + QStringLiteral(" List"),
                           description, QString(), stream);
        stream << indnt << "void ";
        if (!isHeaderMethod)
            stream << className_ << "::";
        stream << "remove" << fldName << " (" << className << " remove_object)";
        if (writeMethodBody) {
            QString method = VECTOR_METHOD_REMOVE;
            method.replace(QRegExp(QStringLiteral("%VARNAME%")), fieldVarName);
            method.replace(QRegExp(QStringLiteral("%VECTORTYPENAME%")), policyExt()->getVectorClassName());
            method.replace(QRegExp(QStringLiteral("%ITEMCLASS%")), className);
            stream << indnt << " {" << m_endl;
            m_indentLevel++;
            printTextAsSeparateLinesWithIndent(method, indent(), stream);
            m_indentLevel--;
            stream << indnt << "}" << m_endl;
        } else
            stream << ";" << m_endl;
    }

    // always allow getting the list of stuff
    QString returnVarName = policyExt()->getVectorClassName() + QLatin1Char('<') + className + QLatin1Char('>');
    writeDocumentation(QStringLiteral("Get the list of ") + fldName + QStringLiteral(" objects held by ") + fieldVarName,
                       description,
                       policyExt()->getDocToolTag() + QStringLiteral("return ") + returnVarName + QStringLiteral(" list of ") + fldName + QStringLiteral(" objects held by ") + fieldVarName,
                       stream);
    stream << indnt << returnVarName << " ";
    if (!isHeaderMethod)
        stream << className_ << "::";
    stream << "get" << fldName << "List()";
    if (writeMethodBody) {
        stream << indnt << " {" << m_endl;
        m_indentLevel++;
        stream << indent() << "return " << fieldVarName << ";" << m_endl;
        m_indentLevel--;
        stream << indnt << "}" << m_endl;
    }
    else {
        stream << ";" << m_endl;
    }
}

/**
 * Writes getFoo() and setFoo() accessor methods for the attribute.
 */
void CppWriter::writeSingleAttributeAccessorMethods(
        const QString& fieldClassName, const QString& fieldVarName,
        const QString& fieldName, const QString &description,
        Uml::Changeability::Enum change,
        bool isHeaderMethod,
        bool isStatic,
        bool writeMethodBody,
        QTextStream &stream)
{
    // DON'T write this IF it is a source method AND writeMethodBody is "false"
    if (!isHeaderMethod && !writeMethodBody)
        return;

    QString className = fixTypeName(fieldClassName);
    QString indnt = indent();
    QString varName = QStringLiteral("value");
    QString fullVarName = varName;

    int i = className.indexOf(QLatin1Char('['));
    bool isArrayType = false;
    if (i > -1) {
        fullVarName += className.mid(i);
        className = className.left(i);
        isArrayType = true;
    }
    QString fldName = fieldName;
    if (policyExt()->getAccessorMethodsStartWithUpperCase())
        fldName = Codegen_Utils::capitalizeFirstLetter(fieldName);

    // set method
    if (change == Uml::Changeability::Changeable) {
        writeDocumentation(QStringLiteral("Set the value of ") + fieldVarName, description, policyExt()->getDocToolTag() + QString(QStringLiteral("param %1 the new value of ")).arg(varName) + fieldVarName, stream);
        stream << indnt << "void ";
        if (!isHeaderMethod)
            stream << className_ << "::";
        stream << "set" << fldName << "(" << className << " " << fullVarName << ")";

        if (writeMethodBody) {
            stream << m_endl << indnt << "{" << m_endl;
            m_indentLevel++;
            stream << indent();
            m_indentLevel--;
            if (isStatic)
                stream << className_ << "::";
            if (isArrayType)
                stream << "*" << fieldVarName << " = *" << varName << ";" << m_endl;
            else
                stream << fieldVarName << " = " << fullVarName << ";" << m_endl;
            stream << indnt << "}" << m_endl;
        } else
            stream << ";" << m_endl;
    }

    if (i > -1)
        className += QStringLiteral("*");

    // get method
    writeDocumentation(QStringLiteral("Get the value of ") + fieldVarName, description, policyExt()->getDocToolTag() + QStringLiteral("return the value of ") + fieldVarName, stream);
    stream << indnt << className << " ";
    if (!isHeaderMethod)
        stream << className_ << "::";
    if (policyExt()->getGetterWithGetPrefix())
        stream << "get" << fldName << "()";
    else
        stream << fieldName << "()";

    if (writeMethodBody) {
        stream << m_endl << indnt << "{" << m_endl;
        m_indentLevel++;
        stream << indent() << "return ";
        m_indentLevel--;
        if (isStatic)
            stream << className_ << "::";
        stream << fieldVarName << ";" << m_endl;
        stream << indnt << "}";
    } else
        stream << ";" << m_endl;

    writeBlankLine(stream);
}

/**
 * Writes the comment and class constructor declaration or methods.
 * Note:
 *   One day, this should print out non-empty constructor operations too.
 */
void CppWriter::writeConstructorDecls(QTextStream &stream)
{
    const bool generateEmptyConstructors =
        UMLApp::app()->commonPolicy()->getAutoGenerateConstructors();
    if (forceSections() || generateEmptyConstructors)
    {
        writeComment(QStringLiteral("Constructors/Destructors"), indent(), stream);
        writeBlankLine(stream);
        writeBlankLine(stream);
    }
    if (!generateEmptyConstructors)
        return;

    writeDocumentation(QString(), QStringLiteral("Empty Constructor"), QString(), stream);
    stream << indent() << className_ << "();" << m_endl;
    writeDocumentation(QString(), QStringLiteral("Empty Destructor"), QString(), stream);
    stream << indent();
    stream << "virtual ~" << className_ << "();" << m_endl;
    writeBlankLine(stream);
}

/**
 * If needed, write out the declaration for the method to initialize attributes of our class.
 */
void CppWriter::writeInitAttributeDecl(UMLClassifier * c, QTextStream &stream)
{
    if (UMLApp::app()->commonPolicy()->getAutoGenerateConstructors() &&
        c->hasAttributes())
        stream << indent() << "void initAttributes();" << m_endl;
}

/**
 * If needed, write out the method to initialize attributes of our class.
 */
void CppWriter::writeInitAttributeMethod(UMLClassifier * c, QTextStream &stream)
{
    // only need to do this under certain conditions
    if (!UMLApp::app()->commonPolicy()->getAutoGenerateConstructors() ||
        !c->hasAttributes())
        return;

    QString indnt = indent();

    stream << indnt << "void " << className_ << "::" << "initAttributes()" << m_endl
           << "{" << m_endl;

    m_indentLevel++;
    // first, initiation of fields derived from attributes
    UMLAttributeList atl = c->getAttributeList();
    foreach (UMLAttribute* at, atl) {
        if (!at->getInitialValue().isEmpty()) {
            QString varName = getAttributeVariableName(at);
            stream << indent() << varName << " = " << at->getInitialValue() << ";" << m_endl;
        }
    }
    // Now initialize the association related fields (e.g. vectors)
    if (!VECTOR_METHOD_INIT.isEmpty()) {
        QStringList::Iterator it;
        for (it = VectorFieldVariables.begin(); it != VectorFieldVariables.end(); ++it) {
            QString fieldVarName = *it;
            QString method = VECTOR_METHOD_INIT;
            method.replace(QRegExp(QStringLiteral("%VARNAME%")), fieldVarName);
            method.replace(QRegExp(QStringLiteral("%VECTORTYPENAME%")), policyExt()->getVectorClassName());
            stream << indent() << method << m_endl;
        }
    }

    if (!OBJECT_METHOD_INIT.isEmpty()) {
        QStringList::Iterator it;
        for (it = ObjectFieldVariables.begin(); it != ObjectFieldVariables.end(); ++it) {
            QString fieldVarName = *it;
            it++;
            QString fieldClassName = *it;
            QString method = OBJECT_METHOD_INIT;
            method.replace(QRegExp(QStringLiteral("%VARNAME%")), fieldVarName);
            method.replace(QRegExp(QStringLiteral("%ITEMCLASS%")), fieldClassName);
            stream << indent() << method << m_endl;
        }
    }

    // clean up
    ObjectFieldVariables.clear(); // shouldn't be needed?
    VectorFieldVariables.clear(); // shouldn't be needed?

    m_indentLevel--;

    stream << indnt << "}" << m_endl;
}

// one day, this should print out non-empty constructor operations too.
void CppWriter::writeConstructorMethods(UMLClassifier * c, QTextStream &stream)
{
    const bool generateEmptyConstructors =
        UMLApp::app()->commonPolicy()->getAutoGenerateConstructors();

    if (forceSections() || generateEmptyConstructors) {
        writeComment(QStringLiteral("Constructors/Destructors"), indent(), stream);
        writeBlankLine(stream);
        writeBlankLine(stream);
    }
    if (!generateEmptyConstructors)
        return;

    // empty constructor
    QString indnt = indent();
    stream << indent() << className_ << "::" << className_ << "()" << m_endl
           << indent() << "{" << m_endl;
    m_indentLevel++;
    if (c->hasAttributes())
        stream << indent() << "initAttributes();" << m_endl;
    m_indentLevel--;
    stream << indent() << "}" << m_endl;
    writeBlankLine(stream);

    // empty destructor
    stream << indent() << className_ << "::~" << className_ << "()" << m_endl
           << indent() << "{" << m_endl
           << indent() << "}" << m_endl;
    writeBlankLine(stream);
}

/**
 * Write all datatypes for a given class.
 * @param c                the class for which we are generating code
 * @param permitScope      what type of method to write (by Scope)
 * @param stream           QTextStream serialization target
 */
void CppWriter::writeDataTypes(UMLClassifier *c, Uml::Visibility::Enum permitScope, QTextStream &stream)
{
    foreach (UMLObject* o, c->containedObjects()) {
        uIgnoreZeroPointer(o);
        if (o->visibility() != permitScope)
            continue;
        if (!o->isUMLDatatype())
            continue;
        const UMLDatatype *d = o->asUMLDatatype();
        if (d && d->isReference() && d->originType()) {
            stream << indent() << "typedef " << d->originType()->name() << " " << d->name() << ";" << m_endl;
        }
    }
}

/**
 * Replaces `string' with STRING_TYPENAME.
 */
QString CppWriter::fixTypeName(const QString &string)
{
    if (string.isEmpty())
        return QStringLiteral("void");
    if (string == QStringLiteral("string")) {
        m_stringIncludeRequired = true;
        return policyExt()->getStringClassName();
    }
    return string;
}

    /**
     * Write all ooperations for a given class.
     * @param c                the class for which we are generating code
     * @param isHeaderMethod   true when writing to a header file, false for body file
     * @param permitScope      what type of method to write (by Scope)
     * @param cpp              the stream associated with the output file
     */
void CppWriter::writeOperations(UMLClassifier *c, bool isHeaderMethod,
                                Uml::Visibility::Enum permitScope, QTextStream &cpp)
{
    UMLOperationList oplist;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList inputlist = c->getOpList();
    foreach (UMLOperation* op, inputlist) {
        if (op->visibility() == permitScope) {
            oplist.append(op);
        }
    }

    // do people REALLY want these comments? Hmm.
    /*
      if (forceSections() || oppub.count())
      {
      writeComment(QStringLiteral("public operations"), indent(), cpp);
        writeBlankLine(cpp);
      }
    */
    writeOperations(c, oplist, isHeaderMethod, cpp);
}

/**
 * Write a list of operations for a given class.
 * Writes operation in either header or source file.
 * @param c                the class for which we are generating code
 * @param oplist           the list of operations you want to write
 * @param isHeaderMethod   true when writing to a header file, false for body file
 * @param cpp              the stream associated with the output file
 */
void CppWriter::writeOperations(UMLClassifier *c, UMLOperationList &oplist, bool isHeaderMethod, QTextStream &cpp)
{
    const bool generateEmptyConstructors =
        UMLApp::app()->commonPolicy()->getAutoGenerateConstructors();

    // generate method decl for each operation given
    foreach (UMLOperation* op, oplist) {
        QString doc;  // buffer for documentation
        QString methodReturnType;
        UMLAttributeList atl = op->getParmList();  // method parameters

        if (op->isConstructorOperation()) {
            if (generateEmptyConstructors && atl.count() == 0)
                continue;  // it's already been written, see writeConstructor{Decls, Methods}
        } else if (op->isDestructorOperation()) {
            if (generateEmptyConstructors)
                continue;  // it's already been written, see writeConstructor{Decls, Methods}
        } else {
            methodReturnType = fixTypeName(op->getTypeName());
            if (methodReturnType != QStringLiteral("void"))
                doc += policyExt()->getDocToolTag() + QStringLiteral("return ") + methodReturnType + QLatin1Char('\n');
        }

        QString str;
        if (op->isAbstract() || c->isInterface()) {
            if (isHeaderMethod) {
                // declare abstract method as 'virtual'
                str += QStringLiteral("virtual ");
            }
        }

        // static declaration for header file
        if (isHeaderMethod && op->isStatic())
            str += QStringLiteral("static ");

        // returntype of method
        str += methodReturnType + QLatin1Char(' ');

        if (!isHeaderMethod)
            str += className_ + QStringLiteral("::");

        str += cleanName(op->name()) + QStringLiteral("(");

        // generate parameters
        uint j = 0;
        for (UMLAttributeListIt atlIt(atl); atlIt.hasNext() ; ++j) {
            UMLAttribute* at = atlIt.next();
            QString typeName = fixTypeName(at->getTypeName());
            QString atName = cleanName(at->name());
            QString atNameType = atName;

            int i = typeName.indexOf(QLatin1Char('['));
            if (i > -1) {
                atNameType += typeName.mid(i);
                typeName = typeName.left(i);
            }
            str += typeName + QLatin1Char(' ') + atName;
            const QString initVal = at->getInitialValue();
            if (! initVal.isEmpty())
                str += QStringLiteral(" = ") + initVal;
            if (j < (uint)(atl.count() - 1))
                str += QStringLiteral(", ");
            doc += policyExt()->getDocToolTag() + QStringLiteral("param  ") + atName + QLatin1Char(' ') + at->doc() + m_endl;
        }
        doc = doc.remove(doc.size() - 1, 1);  // remove last endl of comment
        str += QLatin1Char(')');

        if (op->getConst())
            str += QStringLiteral(" const");

        if (op->getOverride())
            str += QStringLiteral(" override");

        if (isHeaderMethod && op->isAbstract()) {
            str += QStringLiteral(" = 0;");
        }
        // method body : only gets IF it is not in a header
        else if (isHeaderMethod && !policyExt()->getOperationsAreInline()) {
            str += QLatin1Char(';'); // terminate now
        }
        else {
            str += m_endl + indent() + QLatin1Char('{') + m_endl;
            QString sourceCode = op->getSourceCode();
            if (sourceCode.isEmpty()) {
                // empty method body - TODO: throw exception
            }
            else {
                str += formatSourceCode(sourceCode, indent() + indent());
            }
            str += indent() + QLatin1Char('}');
        }

        // write it out
        writeDocumentation(QString(), op->doc(), doc, cpp);
        cpp << indent() << str << m_endl;
        writeBlankLine(cpp);
    }
}

/**
 * Intelligently print out header include/forward decl. for associated classes.
 * Note:
 *   To prevent circular including when both classifiers on either end
 *   of an association have roles we need to have forward declaration of
 *   the other class...but only IF it is not THIS class (as could happen
 *   in self-association relationship).
 */
void CppWriter::printAssociationIncludeDecl(UMLAssociationList list, Uml::ID::Type myId, QTextStream &stream)
{
    foreach (UMLAssociation *a, list) {
        UMLClassifier *current = 0;
        bool isFirstClass = true;

        // only use OTHER classes (e.g. we don't need to write includes for ourselves!!
        // AND only IF the roleName is defined, otherwise, it is not meant to be noticed.
        if (a->getObjectId(Uml::RoleType::A) == myId && !a->getRoleName(Uml::RoleType::B).isEmpty()) {
            current = a->getObject(Uml::RoleType::B)->asUMLClassifier();
        } else if (a->getObjectId(Uml::RoleType::B) == myId && !a->getRoleName(Uml::RoleType::A).isEmpty()) {
            current = a->getObject(Uml::RoleType::A)->asUMLClassifier();
            isFirstClass = false;
        }

        // as header doc for this method indicates, we need to be a bit sophisticated on
        // how to declare some associations.
        if (current) {
            if (!isFirstClass && !a->getRoleName(Uml::RoleType::A).isEmpty() && !a->getRoleName(Uml::RoleType::B).isEmpty())
                stream << "class " << current->name() << ";" << m_endl; // special case: use forward declaration
            else
                stream << "#include \"" << current->name() << ".h\"" << m_endl; // just the include statement
        }
    }
}

/**
 * Check that initial values of strings have quotes around them.
 */
QString CppWriter::fixInitialStringDeclValue(const QString &value, const QString &type)
{
    QString val = value;
    // check for strings only
    if (!val.isEmpty() && type == policyExt()->getStringClassName()) {
        if (!val.startsWith(QLatin1Char('"')))
            val.prepend(QLatin1Char('"'));
        if (!val.endsWith(QLatin1Char('"')))
            val.append(QLatin1Char('"'));
    }
    return val;
}

/**
 * Returns the name of the given object (if it exists).
 * Note:
 *   Methods like this _shouldn't_ be needed IF we properly did things thruought the code.
 */
QString CppWriter::umlObjectName(UMLObject *obj)
{
    return (obj ? obj->name() : QStringLiteral("NULL"));
}

/**
 * Write a blank line.
 */
void CppWriter::writeBlankLine(QTextStream &stream)
{
    stream << m_endl;
}

/**
 * Utility method to break up a block of text, which has embedded newline chars,
 * and print them to a stream as separate lines of text, indented as directed.
 */
void CppWriter::printTextAsSeparateLinesWithIndent(const QString &text, const QString &indent, QTextStream &stream)
{
    if (text.isEmpty()) {
        return;
    }

    QStringList lines = text.split(QLatin1Char('\n'));
    for (int i= 0; i < lines.count(); ++i) {
        stream << indent << lines[i] << m_endl;
    }
}

/**
 * Determine what the variable name of this attribute should be.
 */
QString CppWriter::getAttributeVariableName(UMLAttribute *at)
{
    QString fieldName;
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy * policy = dynamic_cast<CPPCodeGenerationPolicy*>(pe);
    if (policy && !policy->getClassMemberPrefix().isEmpty())
        fieldName = policy->getClassMemberPrefix() + cleanName(at->name());
    else
        fieldName = cleanName(at->name());
    return fieldName;
}

QString CppWriter::getAttributeMethodBaseName(const QString &fieldName)
{
    QString fldName = fieldName;
    if (policyExt()->getRemovePrefixFromAccessorMethods())
        fldName.replace(QRegExp(QStringLiteral("^[a-zA-Z]_")), QStringLiteral(""));
    return fldName;
}

/**
 * Add C++ primitives as datatypes.
 * @return   the list of default datatypes
 */
QStringList CppWriter::defaultDatatypes() const
{
    return Codegen_Utils::cppDatatypes();
}

/**
 * Get list of reserved keywords.
 * @return   the list of reserved keywords
 */
QStringList CppWriter::reservedKeywords() const
{
    return Codegen_Utils::reservedCppKeywords();
}

