/***************************************************************************
                          cppwriter.cpp
    This is the "old" code generator that does not support code editing
    in the Modeller but uses significantly less file space because the
    source code is not replicated in the XMI file.
 
    Copyright (C) 2003       Brian Thomas
                             <brian.thomas@gsfc.nasa.gov>
              (C) 2004-2008  Umbrello UML Modeller Authors
                             <uml-devel@uml.sf.net>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "cppwriter.h"
// qt/kde includes
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>
#include <kdebug.h>
// app includes
#include "codegen_utils.h"
#include "../uml.h"
#include "../umldoc.h"
#include "../association.h"
#include "../classifier.h"
#include "../operation.h"
#include "../template.h"
#include "../umltemplatelist.h"
#include "../umlclassifierlistitemlist.h"
#include "../classifierlistitem.h"
#include "../model_utils.h"
#include "../codegenerationpolicy.h"

// 3-14-2003: this code developed from the javawriter with parts of the
// original cppwriter by Luis De la Parra Blum

CppWriter::CppWriter()
{
    // Probably we could resolve this better through the use of templates,
    // but its a quick n dirty fix for the timebeing.. until codegeneration
    // template system is in place.
    // You can insert code here. 3 general variables exist: "%VARNAME%"
    // allows you to specify where the vector variable should be in your code,
    // and "%ITEMCLASS%", if needed, where the class of the item is declared.
    VECTOR_METHOD_APPEND = "%VARNAME%.push_back(add_object);"; // for std::vector
    VECTOR_METHOD_REMOVE = "int i, size = %VARNAME%.size();\nfor ( i = 0; i < size; i++) {\n\t%ITEMCLASS% item = %VARNAME%.at(i);\n\tif(item == remove_object) {\n\t\tvector<%ITEMCLASS%>::iterator it = %VARNAME%.begin() + i;\n\t\t%VARNAME%.erase(it);\n\t\treturn;\n\t}\n }"; // for std::vector
    VECTOR_METHOD_INIT.clear(); // nothing to be done
    /*
        VECTOR_METHOD_APPEND = "%VARNAME%.append(&add_object);"; // Qt lib implementation
        VECTOR_METHOD_REMOVE = "%VARNAME%.removeRef(&remove_object);"; // Qt lib implementation
        VECTOR_METHOD_INIT = "%VARNAME%.setAutoDelete(false);"; // Qt library
    */

    OBJECT_METHOD_INIT = "%VARNAME% = new %ITEMCLASS%( );"; // Qt library

    // boolean config params
    INLINE_ASSOCIATION_METHODS = false;
}

CppWriter::~CppWriter()
{
}

Uml::Programming_Language CppWriter::getLanguage()
{
    return Uml::pl_Cpp;
}

CPPCodeGenerationPolicy *CppWriter::policyExt()
{
    return static_cast<CPPCodeGenerationPolicy*>(UMLApp::app()->getPolicyExt());
}

void CppWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        uDebug() << "Cannot write class of NULL concept!\n";
        return;
    }

    QFile fileh, filecpp;

    // find an appropriate name for our file
    fileName_ = findFileName(c, ".h");
    if (fileName_.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    className_ = cleanName(c->getName());

    if (c->getVisibility() != Uml::Visibility::Implementation) {
        if( !openFile(fileh, fileName_)) {
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
    if (c->getBaseType() == Uml::ot_Enum) {
        need_impl = false;
    }
    if (need_impl) {
        fileName_.replace( QRegExp(".h$"), ".cpp");
        if( !openFile(filecpp, fileName_)) {
            emit codeGenerated(c, false);
            return;
        }
        // write Source file
        writeSourceFile(c, filecpp);
        filecpp.close();
    }

    emit codeGenerated(c, true);
}

void CppWriter::writeHeaderFile (UMLClassifier *c, QFile &fileh)
{
    // open stream for writing
    QTextStream h (&fileh);

    // up the indent level to one
    m_indentLevel = 1;

    // write header blurb
    QString str = getHeadingFile(".h");
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), fileName_ + ".h");
        str.replace(QRegExp("%filepath%"), fileh.fileName());
        h << str<< m_endl;
    }

    // Write the hash define stuff to prevent multiple parsing/inclusion of header
    QString hashDefine = className_.toUpper().simplified().replace(QRegExp(" "),  "_");
    writeBlankLine(h);
    h << "#ifndef "<< hashDefine + "_H" << m_endl;
    h << "#define "<< hashDefine + "_H" << m_endl;

    writeClassDecl(c, h);

    // last thing..close our hashdefine
    h << m_endl << "#endif // " << hashDefine + "_H" << m_endl;
}

void CppWriter::writeHeaderAccessorMethodDecl(UMLClassifier *c, Uml::Visibility permitScope, QTextStream &stream)
{
    // attributes
    writeHeaderAttributeAccessorMethods(c, permitScope, true, stream); // write static attributes first
    writeHeaderAttributeAccessorMethods(c, permitScope, false, stream);

    // associations
    writeAssociationMethods(c->getSpecificAssocs(Uml::at_Association), permitScope,
                            true, INLINE_ASSOCIATION_METHODS, true, c->getID(), stream);
    writeAssociationMethods(c->getUniAssociationToBeImplemented(), permitScope,
                            true, INLINE_ASSOCIATION_METHODS, true, c->getID(), stream);
    writeAssociationMethods(c->getAggregations(), permitScope,
                            true,  INLINE_ASSOCIATION_METHODS, true, c->getID(), stream);
    writeAssociationMethods(c->getCompositions(), permitScope,
                            true, INLINE_ASSOCIATION_METHODS, false, c->getID(), stream);

    writeBlankLine(stream);
}

void CppWriter::writeHeaderFieldDecl(UMLClassifier *c, Uml::Visibility permitScope, QTextStream &stream)
{
    // attributes
    writeAttributeDecls(c, permitScope, true, stream); // write static attributes first
    writeAttributeDecls(c, permitScope, false, stream);

    // associations
    writeAssociationDecls(c->getSpecificAssocs(Uml::at_Association), permitScope, c->getID(), stream);
    writeAssociationDecls(c->getUniAssociationToBeImplemented(), permitScope, c->getID(), stream);
    writeAssociationDecls(c->getAggregations(), permitScope, c->getID(), stream);
    writeAssociationDecls(c->getCompositions(), permitScope, c->getID(), stream);
}

void CppWriter::writeSourceFile (UMLClassifier *c, QFile &filecpp )
{
    // open stream for writing
    QTextStream cpp (&filecpp);

    // set the starting indentation at zero
    m_indentLevel = 0;

    //try to find a heading file (license, coments, etc)
    QString str;
    str = getHeadingFile(".cpp");
    if (!str.isEmpty()) {
        str.replace(QRegExp("%filename%"), fileName_ + ".cpp");
        str.replace(QRegExp("%filepath%"), filecpp.fileName());
        cpp << str << m_endl;
    }

    // IMPORT statements
    // Q: Why all utils? Isnt just List and Vector the only classes we are using?
    // Our import *should* also look at operations, and check that objects being
    // used arent in another package (and thus need to be explicitly imported here).
    cpp << "#include \"" << className_ << ".h\"" << m_endl;
    writeBlankLine(cpp);

    if (c->getVisibility() == Uml::Visibility::Implementation) {
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
    QString indent = getIndent();
    if (forceDoc() || c->hasAccessorMethods() || c->hasOperationMethods())
    {
        writeComment(" ", indent, cpp);
        writeComment("Methods", indent, cpp);
        writeComment(" ", indent, cpp);
        writeBlankLine(cpp);
        writeBlankLine(cpp);
    }

    // write comment for sub-section IF needed
    if (forceDoc() || c->hasAccessorMethods() )
    {
        writeComment("Accessor methods", indent, cpp);
        writeComment(" ", indent, cpp);
        writeBlankLine(cpp);
    }

    // Accessor methods for attributes
    const bool bInlineAccessors = policyExt()->getAccessorsAreInline();
    if (!bInlineAccessors && c->hasAttributes())
    {
        writeAttributeMethods(c->getAttributeListStatic(Uml::Visibility::Public), Uml::Visibility::Public, false, true, !bInlineAccessors, cpp);
        writeAttributeMethods(c->getAttributeList(Uml::Visibility::Public), Uml::Visibility::Public, false, false, !bInlineAccessors, cpp);
        writeAttributeMethods(c->getAttributeListStatic(Uml::Visibility::Protected), Uml::Visibility::Protected, false, true, !bInlineAccessors, cpp);
        writeAttributeMethods(c->getAttributeList(Uml::Visibility::Protected), Uml::Visibility::Protected, false, false, !bInlineAccessors, cpp);
        writeAttributeMethods(c->getAttributeListStatic(Uml::Visibility::Private), Uml::Visibility::Private, false, true, !bInlineAccessors, cpp);
        writeAttributeMethods(c->getAttributeList(Uml::Visibility::Private), Uml::Visibility::Private, false, false, !bInlineAccessors, cpp);
    }

    // accessor methods for associations

    // public
    writeAssociationMethods(c->getSpecificAssocs(Uml::at_Association), Uml::Visibility::Public, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
    writeAssociationMethods(c->getUniAssociationToBeImplemented(), Uml::Visibility::Public, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
    writeAssociationMethods(c->getAggregations(), Uml::Visibility::Public, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
    writeAssociationMethods(c->getCompositions(), Uml::Visibility::Public, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);

    // protected
    writeAssociationMethods(c->getSpecificAssocs(Uml::at_Association), Uml::Visibility::Protected, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
    writeAssociationMethods(c->getUniAssociationToBeImplemented(), Uml::Visibility::Protected, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
    writeAssociationMethods(c->getAggregations(), Uml::Visibility::Protected, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
    writeAssociationMethods(c->getCompositions(), Uml::Visibility::Protected, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);


    // private
    writeAssociationMethods(c->getSpecificAssocs(Uml::at_Association), Uml::Visibility::Private, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
    writeAssociationMethods(c->getUniAssociationToBeImplemented(), Uml::Visibility::Private, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
    writeAssociationMethods(c->getAggregations(), Uml::Visibility::Private, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
    writeAssociationMethods(c->getCompositions(), Uml::Visibility::Private, false,
                            !INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
    writeBlankLine(cpp);

    // Other operation methods -- all other operations are now written
    //

    // write comment for sub-section IF needed
    if (forceDoc() || c->hasOperationMethods())
    {
        writeComment("Other methods", indent, cpp);
        writeComment(" ", indent, cpp);
        writeBlankLine(cpp);
    }

    if(!policyExt()->getOperationsAreInline())
    {
        writeOperations(c,false,Uml::Visibility::Public,cpp);
        writeOperations(c,false,Uml::Visibility::Protected,cpp);
        writeOperations(c,false,Uml::Visibility::Private,cpp);
    }

    // Yep, bringing up the back of the bus, our initialization method for attributes
    writeInitAttributeMethod(c, cpp);

    writeBlankLine(cpp);
}

void CppWriter::writeClassDecl(UMLClassifier *c, QTextStream &cpp)
{
    UMLClassifierList superclasses = c->getSuperClasses();
    foreach (UMLClassifier* classifier, superclasses ) {
        QString headerName = findFileName(classifier, ".h");
        if (!headerName.isEmpty()) {
            cpp << "#include \"" << headerName << "\"" << m_endl;
        }
    }

    writeBlankLine(cpp);
    cpp << "#include <" << policyExt()->getStringClassNameInclude() << ">" << m_endl;
    if (c->hasVectorFields())
    {
        cpp << "#include " << policyExt()->getVectorClassNameInclude() << m_endl;
        writeBlankLine(cpp);
    }

    if (c->hasAssociations())
    {
        // write all includes we need to include other classes, that arent us.
        printAssociationIncludeDecl (c->getSpecificAssocs(Uml::at_Association), c->getID(), cpp);
        printAssociationIncludeDecl (c->getUniAssociationToBeImplemented(), c->getID(), cpp);
        printAssociationIncludeDecl (c->getAggregations(), c->getID(), cpp);
        printAssociationIncludeDecl (c->getCompositions(), c->getID(), cpp);

        writeBlankLine(cpp);
    }

    foreach (UMLClassifier* classifier, superclasses ) {
        if (classifier->getPackage()!=c->getPackage() && !classifier->getPackage().isEmpty()) {
            cpp << "using " << cleanName(classifier->getPackage()) << "::" << cleanName(classifier->getName()) << ";" << m_endl;
        }
    }

    if (!c->getPackage().isEmpty() && policyExt()->getPackageIsNamespace())
        cpp << m_endl << "namespace " << cleanName(c->getPackage()) << " {" << m_endl << m_endl;

    //Write class Documentation if there is somthing or if force option
    if (forceDoc() || !c->getDoc().isEmpty()) {
        cpp << m_endl << "/**" << m_endl;
        cpp << "  * class " << className_ << m_endl;
        cpp << formatDoc(c->getDoc(),"  * ");
        cpp << "  */";
        writeBlankLine(cpp);
        writeBlankLine(cpp);
    }

    //check if class is abstract and / or has abstract methods
    if ((c->getAbstract() || c->isInterface())
            && !hasAbstractOps(c))
        cpp << "/******************************* Abstract Class ****************************" << m_endl
        << className_ << " does not have any pure virtual methods, but its author" << m_endl
        << "  defined it as an abstract class, so you should not use it directly." << m_endl
        << "  Inherit from it instead and create only objects from the derived classes" << m_endl
        << "*****************************************************************************/" << m_endl << m_endl;

    if (c->getBaseType() == Uml::ot_Enum) {
        UMLClassifierListItemList litList = c->getFilteredList(Uml::ot_EnumLiteral);
        uint i = 0;
        cpp << "enum " << className_ << " {" << m_endl;
        foreach (UMLClassifierListItem* lit, litList ) {
            QString enumLiteral = cleanName(lit->getName());
            cpp << getIndent() << enumLiteral;
            if (++i < ( uint )litList.count())
                cpp << ",";
            cpp << m_endl;
        }
        cpp << m_endl << "};" << m_endl;  // end of class header
        if (!c->getPackage().isEmpty() && policyExt()->getPackageIsNamespace())
            cpp << "}  // end of package namespace" << m_endl;
        return;
    }

    // Generate template parameters.
    UMLTemplateList template_params = c->getTemplateList();
    if (template_params.count()) {
        cpp << "template<";
        for (UMLTemplateListIt tlit( template_params ); tlit.hasNext(); ) {
            UMLTemplate* t = tlit.next();
            QString formalName = t->getName();
            QString typeName = t->getTypeName();
            cpp << typeName << " " << formalName;
            if ( tlit.hasNext() ) {
                t = tlit.next();
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
        if (superClass->getAbstract() || superClass->isInterface())
            cpp << "virtual ";
        cpp << "public " << cleanName(superClass->getName());
        if (i < numOfSuperClasses)
            cpp << ", ";
    }

    cpp << m_endl << "{" << m_endl; // begin the body of the class

    //
    // write out field and operations decl grouped by visibility
    //

    // PUBLIC attribs/methods
    cpp << "public:" << m_endl << m_endl; // print visibility decl.
    // for public: constructors are first ops we print out
    if (!c->isInterface())
        writeConstructorDecls(cpp);
    writeHeaderFieldDecl(c,Uml::Visibility::Public, cpp);
    writeHeaderAccessorMethodDecl(c, Uml::Visibility::Public, cpp);
    writeOperations(c,true,Uml::Visibility::Public,cpp);

    // PROTECTED attribs/methods
    //
    cpp << "protected" << ":" << m_endl << m_endl; // print visibility decl.
    writeHeaderFieldDecl(c,Uml::Visibility::Protected, cpp);
    writeHeaderAccessorMethodDecl(c, Uml::Visibility::Protected, cpp);
    writeOperations(c,true,Uml::Visibility::Protected,cpp);

    // PRIVATE attribs/methods
    //
    cpp << "private" << ":" << m_endl << m_endl; // print visibility decl.
    writeHeaderFieldDecl(c,Uml::Visibility::Private, cpp);
    writeHeaderAccessorMethodDecl(c, Uml::Visibility::Private, cpp);
    writeOperations(c,true,Uml::Visibility::Private,cpp);
    writeInitAttributeDecl(c, cpp); // this is always private, used by constructors to initialize class

    // end of class header
    cpp << m_endl << "};" << m_endl;

    // end of class namespace, if any
    if(!c->getPackage().isEmpty() && policyExt()->getPackageIsNamespace())
        cpp << "}; // end of package namespace" << m_endl;

}

void CppWriter::writeAttributeDecls (UMLClassifier *c, Uml::Visibility visibility, bool writeStatic, QTextStream &stream )
{
    if (c->isInterface())
        return;

    UMLAttributeList list;
    if (writeStatic)
        list = c->getAttributeListStatic(visibility);
    else
        list = c->getAttributeList(visibility);

    //write documentation
    if (forceDoc() || list.count() > 0)
    {
        QString strVis = Codegen_Utils::capitalizeFirstLetter(visibility.toString());
        QString strStatic = writeStatic ? "Static ":"";
        writeComment(strStatic + strVis + " attributes",getIndent(), stream);
        writeComment(" ",getIndent(), stream);
        writeBlankLine(stream);
    }

    if (list.count() > 0) {

        // write attrib declarations now
        bool isFirstAttrib = true;
        QString documentation;
        foreach (UMLAttribute* at, list ) {

            //                  bool noPriorDocExists = documentation.isEmpty();
            documentation = at->getDoc();

            // add a line for code clarity IF PRIOR attrib has comment on it
            // OR this line has documentation
            //                  if(!isFirstAttrib && (!documentation.isEmpty()||!noPriorDocExists))
            //                          writeBlankLine(stream);

            isFirstAttrib = false;

            QString varName = getAttributeVariableName(at);

            QString staticValue = at->getStatic() ? "static " : "";
            QString typeName = fixTypeName(at->getTypeName());
            if(!documentation.isEmpty())
                writeComment(documentation, getIndent(), stream);
            stream << getIndent() << staticValue << typeName << " " << varName << ";" << m_endl;

        }

        /*
        if(list.count() > 0)
            writeBlankLine(stream);
        */
    }
}

void CppWriter::writeHeaderAttributeAccessorMethods (UMLClassifier *c, Uml::Visibility visibility, bool writeStatic, QTextStream &stream )
{
    // check the current policy about generate accessors as public
    UMLAttributeList list;
    if (writeStatic)
        list = c->getAttributeListStatic(visibility);
    else
        list = c->getAttributeList(visibility);

    // switch to public
    if (visibility != Uml::Visibility::Public)
        stream << "public:" << m_endl << m_endl;

    // write accessor methods for attribs we found
    writeAttributeMethods(list, visibility, true, false, policyExt()->getAccessorsAreInline(), stream);

    // switch back to previous vis.
    if (visibility != Uml::Visibility::Public)
        stream << visibility.toString() << ":" << m_endl << m_endl;
}

// this is for writing *source* or *header* file attribute methods
//
void CppWriter::writeAttributeMethods(UMLAttributeList attribs,
                                      Uml::Visibility visibility, bool isHeaderMethod,
                                      bool isStatic,
                                      bool writeMethodBody, QTextStream &stream)
{
    if (!policyExt()->getAutoGenerateAccessors())
        return;

    if (forceDoc() || attribs.count() > 0)
    {
        QString strVis = Codegen_Utils::capitalizeFirstLetter(visibility.toString());
        QString strStatic = (isStatic ? " static" : "");
        writeBlankLine(stream);
        writeComment(strVis + strStatic + " attribute accessor methods",getIndent(),stream);
        writeComment(" ",getIndent(), stream);
        writeBlankLine(stream);
    }

    // return now if NO attributes to work on
    if (attribs.count() == 0)
        return;

    foreach (UMLAttribute* at, attribs ) {
        QString varName = getAttributeVariableName(at);
        QString methodBaseName = cleanName(at->getName());

        // force capitalizing the field name, this is silly,
        // from what I can tell, this IS the default behavior for
        // cleanName. I dunno why its not working -b.t.
        methodBaseName = methodBaseName.trimmed();
        methodBaseName.replace(0,1,methodBaseName.at(0).toUpper());

        writeSingleAttributeAccessorMethods(at->getTypeName(), varName,
                                            methodBaseName, at->getDoc(), Uml::chg_Changeable, isHeaderMethod,
                                            at->getStatic(), writeMethodBody, stream);
    }
}

void CppWriter::writeComment(const QString &comment, const QString &myIndent, QTextStream &cpp)
{
    // in the case we have several line comment..
    // NOTE: this part of the method has the problem of adopting UNIX newline,
    // need to resolve for using with MAC/WinDoze eventually I assume
    if (comment.contains(QRegExp("\n"))) {

        QStringList lines = comment.split( '\n' );
        for (int i= 0; i < lines.count(); i++)
        {
            cpp << myIndent << "// " << lines[i] << m_endl;
        }
    } else {
        // this should be more fancy in the future, breaking it up into 80 char
        // lines so that it doesn't look too bad
        cpp << myIndent << "// "<< comment << m_endl;
    }
}

void CppWriter::writeDocumentation(QString header, QString body, QString end, QTextStream &cpp)
{
    writeBlankLine(cpp);
    QString indent = getIndent();

    cpp << indent << "/**" << m_endl;
    if (!header.isEmpty())
        cpp << formatDoc(header, indent + " * ");
    if (!body.isEmpty())
        cpp << formatDoc(body, indent + " * ");
    if (!end.isEmpty())
    {
        QStringList lines = end.split( "\n" );
        for (int i= 0; i < lines.count(); i++)
            cpp << formatDoc(lines[i], indent + " * ");
    }
    cpp << indent << " */" << m_endl;
}

void CppWriter::writeAssociationDecls(UMLAssociationList associations, Uml::Visibility permitScope, Uml::IDType id, QTextStream &h)
{
    if( forceSections() || !associations.isEmpty() )
    {
        bool printRoleA = false, printRoleB = false;
        foreach (UMLAssociation *a , associations )
        {
            // it may seem counter intuitive, but you want to insert the role of the
            // *other* class into *this* class.
            if (a->getObjectId(Uml::A) == id && !a->getRoleName(Uml::B).isEmpty())
                printRoleB = true;

            if (a->getObjectId(Uml::B) == id && !a->getRoleName(Uml::A).isEmpty())
                printRoleA = true;

            // First: we insert documentaion for association IF it has either role AND some documentation (!)
            if ((printRoleA || printRoleB) && !(a->getDoc().isEmpty()))
                writeComment(a->getDoc(), getIndent(), h);

            // print RoleB decl
            if (printRoleB && a->getVisibility(Uml::B) == permitScope)
            {

                QString fieldClassName = cleanName(getUMLObjectName(a->getObject(Uml::B)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::B), a->getMulti(Uml::B), a->getRoleDoc(Uml::B), h);
            }

            // print RoleA decl
            if (printRoleA && a->getVisibility(Uml::A) == permitScope)
            {
                QString fieldClassName = cleanName(getUMLObjectName(a->getObject(Uml::A)));
                writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::A), a->getMulti(Uml::A), a->getRoleDoc(Uml::A), h);
            }

            // reset for next association in our loop
            printRoleA = false;
            printRoleB = false;
        }
    }
}

void CppWriter::writeAssociationRoleDecl(QString fieldClassName, QString roleName, QString multi,
        QString doc, QTextStream &stream)
{
    // ONLY write out IF there is a rolename given
    // otherwise its not meant to be declared in the code
    if (roleName.isEmpty())
        return;

    QString indent = getIndent();

    // always put space between this and prior decl, if any
    writeBlankLine(stream);

    if (!doc.isEmpty())
        writeComment(doc, indent, stream);

    // declare the association based on whether it is this a single variable
    // or a List (Vector). One day this will be done correctly with special
    // multiplicity object that we don't have to figure out what it means via regex.
    if (multi.isEmpty() || multi.contains(QRegExp("^[01]$")))
    {
        QString fieldVarName = "m_" + roleName.toLower();

        // record this for later consideration of initialization IF the
        // multi value requires 1 of these objects
        if (ObjectFieldVariables.indexOf(fieldVarName) == -1 &&
                multi.contains(QRegExp("^1$")))
        {
            // ugh. UGLY. Storing variable name and its class in pairs.
            ObjectFieldVariables.append(fieldVarName);
            ObjectFieldVariables.append(fieldClassName);
        }

        stream << indent << fieldClassName << " * " << fieldVarName << ";" << m_endl;
    }
    else
    {
        QString fieldVarName = "m_" + roleName.toLower() + "Vector";

        // record unique occurrences for later when we want to check
        // for initialization of this vector
        if (VectorFieldVariables.indexOf(fieldVarName) == -1)
            VectorFieldVariables.append(fieldVarName);

        stream << indent << policyExt()->getVectorClassName() <<"<" << fieldClassName << "*";
        stream << "> " << fieldVarName << ";" << m_endl;
    }
}

// for either source or header files
void CppWriter::writeAssociationMethods (UMLAssociationList associations,
        Uml::Visibility permitVisib,
        bool isHeaderMethod,
        bool writeMethodBody,
        bool writePointerVar,
        Uml::IDType myID, QTextStream &stream)
{
    if ( forceSections() || !associations.isEmpty() )
    {
        foreach (UMLAssociation *a , associations )
        {

            // insert the methods to access the role of the other
            // class in the code of this one
            if (a->getObjectId(Uml::A) == myID && a->getVisibility(Uml::B) == permitVisib)
            {
                // only write out IF there is a rolename given
                if (!a->getRoleName(Uml::B).isEmpty()) {
                    QString fieldClassName = getUMLObjectName(a->getObject(Uml::B)) + (writePointerVar ? " *":"");
                    writeAssociationRoleMethod(fieldClassName,
                                               isHeaderMethod,
                                               writeMethodBody,
                                               a->getRoleName(Uml::B),
                                               a->getMulti(Uml::B), a->getRoleDoc(Uml::B),
                                               a->getChangeability(Uml::B), stream);
                }
            }

            if (a->getObjectId(Uml::B) == myID && a->getVisibility(Uml::A) == permitVisib)
            {
                // only write out IF there is a rolename given
                if (!a->getRoleName(Uml::A).isEmpty()) {
                    QString fieldClassName = getUMLObjectName(a->getObject(Uml::A)) + (writePointerVar ? " *":"");
                    writeAssociationRoleMethod(fieldClassName,
                                               isHeaderMethod,
                                               writeMethodBody,
                                               a->getRoleName(Uml::A),
                                               a->getMulti(Uml::A),
                                               a->getRoleDoc(Uml::A),
                                               a->getChangeability(Uml::A),
                                               stream);
                }
            }

        }
    }
}

void CppWriter::writeAssociationRoleMethod (const QString &fieldClassName,
        bool isHeaderMethod,
        bool writeMethodBody,
        const QString &roleName, const QString &multi,
        const QString &description, Uml::Changeability_Type change,
        QTextStream &stream)
{
    if (multi.isEmpty() || multi.contains(QRegExp("^[01]$")))
    {
        QString fieldVarName = "m_" + roleName.toLower();
        writeSingleAttributeAccessorMethods(fieldClassName, fieldVarName, roleName,
                                            description, change, isHeaderMethod, false, writeMethodBody, stream);
    }
    else
    {
        QString fieldVarName = "m_" + roleName.toLower() + "Vector";
        writeVectorAttributeAccessorMethods(fieldClassName, fieldVarName, roleName,
                                            description, change, isHeaderMethod, writeMethodBody, stream);
    }
}

void CppWriter::writeVectorAttributeAccessorMethods (
        const QString &fieldClassName, const QString &fieldVarName,
        const QString &fieldName, const QString &description,
        Uml::Changeability_Type changeType,
        bool isHeaderMethod,
        bool writeMethodBody,
        QTextStream &stream)
{
    QString className = fixTypeName(fieldClassName);
    QString fldName = Codegen_Utils::capitalizeFirstLetter(fieldName);
    QString indent = getIndent();

    // ONLY IF changeability is NOT Frozen
    if (changeType != Uml::chg_Frozen)
    {
        writeDocumentation("Add a " + fldName + " object to the " + fieldVarName + " List",description,"",stream);
        stream << indent << "void ";
        if(!isHeaderMethod)
            stream << className_ << "::";
        stream << "add" << fldName << " ( " << className << " add_object )";
        if (writeMethodBody) {
            QString method = VECTOR_METHOD_APPEND;
            method.replace(QRegExp("%VARNAME%"),fieldVarName);
            method.replace(QRegExp("%VECTORTYPENAME%"), policyExt()->getVectorClassName());
            method.replace(QRegExp("%ITEMCLASS%"),className);
            stream << indent << " {" << m_endl;
            m_indentLevel++;
            printTextAsSeparateLinesWithIndent(method,getIndent(),stream);
            m_indentLevel--;
            stream << indent << "}" << m_endl;
        } else
            stream << ";" << m_endl;
    }

    // ONLY IF changeability is Changeable
    if (changeType == Uml::chg_Changeable)
    {
        writeDocumentation("Remove a " + fldName + " object from " + fieldVarName + " List",
                           description, "", stream);
        stream << indent << "void ";
        if(!isHeaderMethod)
            stream << className_ << "::";
        stream << "remove" << fldName << " ( " << className << " remove_object )";
        if (writeMethodBody) {
            QString method = VECTOR_METHOD_REMOVE;
            method.replace(QRegExp("%VARNAME%"),fieldVarName);
            method.replace(QRegExp("%VECTORTYPENAME%"), policyExt()->getVectorClassName());
            method.replace(QRegExp("%ITEMCLASS%"),className);
            stream << indent << " {" << m_endl;
            m_indentLevel++;
            printTextAsSeparateLinesWithIndent(method,getIndent(),stream);
            m_indentLevel--;
            stream << indent << "}" << m_endl;
        } else
            stream << ";" << m_endl;
    }

    // always allow getting the list of stuff
    QString returnVarName = policyExt()->getVectorClassName() + '<' + className + '>';
    writeDocumentation("Get the list of " + fldName + " objects held by " + fieldVarName,
                       description,
                       "@return " + returnVarName + " list of " + fldName + " objects held by " + fieldVarName,
                       stream);
    stream << indent << returnVarName << " ";
    if(!isHeaderMethod)
        stream << className_ << "::";
    stream << "get" << fldName << "List ( )";
    if(writeMethodBody) {
        stream << indent << " {" << m_endl;
        m_indentLevel++;
        stream << getIndent() << "return " << fieldVarName << ";" << m_endl;
        m_indentLevel--;
        stream << indent << "}" << m_endl;
    } else
        stream << ";" << m_endl;

}

void CppWriter::writeSingleAttributeAccessorMethods(
        const QString& fieldClassName, const QString& fieldVarName,
        const QString& fieldName, const QString &description,
        Uml::Changeability_Type change,
        bool isHeaderMethod,
        bool isStatic,
        bool writeMethodBody,
        QTextStream &stream)
{
    // DON'T write this IF its a source method AND writeMethodBody is "false"
    if (!isHeaderMethod && !writeMethodBody)
        return;

    QString className = fixTypeName(fieldClassName);
    QString fldName = Codegen_Utils::capitalizeFirstLetter(fieldName);
    QString indent = getIndent();

    // set method
    if (change == Uml::chg_Changeable && !isStatic) {
        writeDocumentation("Set the value of " + fieldVarName,description,"@param new_var the new value of " + fieldVarName,stream);
        stream << indent << "void ";
        if(!isHeaderMethod)
            stream << className_ << "::";
        stream << "set" << fldName << " ( " << className << " new_var )";

        if (writeMethodBody) {
            stream << indent << " {" << m_endl;
            m_indentLevel++;
            stream << getIndent() << indent;
            m_indentLevel--;
            if (isStatic)
                stream << className_ << "::";
            stream << fieldVarName << " = new_var;" << m_endl;
            stream << indent << "}" << m_endl;
        } else
            stream << ";" << m_endl;
    }

    // get method
    writeDocumentation("Get the value of " + fieldVarName,description,"@return the value of " + fieldVarName,stream);
    stream << indent << className << " ";
    if (!isHeaderMethod)
        stream << className_ << "::";
    stream << "get" << fldName << " ( )";

    if (writeMethodBody) {
        stream << indent << " {" << m_endl;
        m_indentLevel++;
        stream << getIndent() << "return ";
        m_indentLevel--;
        if (isStatic)
            stream << className_ << "::";
        stream << fieldVarName << ";" << m_endl;
        stream << indent << "}";
    } else
        stream << ";" << m_endl;

    writeBlankLine(stream);
}

// one day, this should print out non-empty constructor operations too.
void CppWriter::writeConstructorDecls(QTextStream &stream)
{
    const bool generateEmptyConstructors =
        UMLApp::app()->getCommonPolicy()->getAutoGenerateConstructors();
    if (forceDoc() || generateEmptyConstructors)
    {
        writeComment("Constructors/Destructors", getIndent(), stream);
        writeComment(" ", getIndent(), stream);
        writeBlankLine(stream);
    }
    if (!generateEmptyConstructors)
        return;

    writeDocumentation("", "Empty Constructor", "", stream);
    stream << getIndent() << className_ << " ( );" << m_endl;
    writeDocumentation("", "Empty Destructor", "", stream);
    stream << getIndent();
    stream << "virtual ~" << className_ << " ( );" << m_endl;
    writeBlankLine(stream);
}

void CppWriter::writeInitAttributeDecl(UMLClassifier * c, QTextStream &stream)
{
    if (UMLApp::app()->getCommonPolicy()->getAutoGenerateConstructors() &&
        c->hasAttributes())
        stream << getIndent() << "void initAttributes ( ) ;" << m_endl;
}

void CppWriter::writeInitAttributeMethod(UMLClassifier * c, QTextStream &stream)
{
    // only need to do this under certain conditions
    if (!UMLApp::app()->getCommonPolicy()->getAutoGenerateConstructors() ||
        !c->hasAttributes())
        return;

    QString indent = getIndent();

    stream << indent << "void " << className_ << "::" << "initAttributes ( ) {" << m_endl;

    m_indentLevel++;
    // first, initiation of fields derived from attributes
    UMLAttributeList atl = c->getAttributeList();
    foreach (UMLAttribute* at, atl ) {
        if(!at->getInitialValue().isEmpty()) {
            QString varName = getAttributeVariableName(at);
            stream << getIndent() << varName << " = " << at->getInitialValue() << ";" << m_endl;
        }
    }
    // Now initialize the association related fields (e.g. vectors)
    if (!VECTOR_METHOD_INIT.isEmpty()) {
        QStringList::Iterator it;
        for( it = VectorFieldVariables.begin(); it != VectorFieldVariables.end(); ++it ) {
            QString fieldVarName = *it;
            QString method = VECTOR_METHOD_INIT;
            method.replace(QRegExp("%VARNAME%"),fieldVarName);
            method.replace(QRegExp("%VECTORTYPENAME%"), policyExt()->getVectorClassName());
            stream << getIndent() << method << m_endl;
        }
    }

    if (!OBJECT_METHOD_INIT.isEmpty()) {
        QStringList::Iterator it;
        for( it = ObjectFieldVariables.begin(); it != ObjectFieldVariables.end(); ++it ) {
            QString fieldVarName = *it;
            it++;
            QString fieldClassName = *it;
            QString method = OBJECT_METHOD_INIT;
            method.replace(QRegExp("%VARNAME%"),fieldVarName);
            method.replace(QRegExp("%ITEMCLASS%"),fieldClassName);
            stream << getIndent() << method << m_endl;
        }
    }

    // clean up
    ObjectFieldVariables.clear(); // shouldn't be needed?
    VectorFieldVariables.clear(); // shouldn't be needed?

    m_indentLevel--;

    stream << indent << "}" << m_endl;
}

// one day, this should print out non-empty constructor operations too.
void CppWriter::writeConstructorMethods(UMLClassifier * c, QTextStream &stream)
{
    const bool generateEmptyConstructors =
        UMLApp::app()->getCommonPolicy()->getAutoGenerateConstructors();

    if (forceDoc() || generateEmptyConstructors) {
        writeComment("Constructors/Destructors", getIndent(), stream);
        writeComment(" ", getIndent(), stream);
        writeBlankLine(stream);
    }
    if (!generateEmptyConstructors)
        return;

    // empty constructor
    QString indent = getIndent();
    stream << indent << className_ << "::" << className_ << " ( ) {" << m_endl;
    if(c->hasAttributes())
        stream << indent << indent << "initAttributes();" << m_endl;
    stream << indent << "}" << m_endl;
    writeBlankLine(stream);

    // empty destructor
    stream << getIndent() << className_ << "::~" << className_ << " ( ) { }" << m_endl;
    writeBlankLine(stream);
}

QString CppWriter::fixTypeName(const QString &string)
{
    if (string.isEmpty())
        return "void";
    if (string == "string")
        return policyExt()->getStringClassName();
    return string;
}

void CppWriter::writeOperations(UMLClassifier *c, bool isHeaderMethod,
                                Uml::Visibility permitScope, QTextStream &cpp)
{
    UMLOperationList oplist;

    //sort operations by scope first and see if there are abstract methods
    UMLOperationList inputlist = c->getOpList();
    foreach (UMLOperation* op, inputlist ) {
        if (op->getVisibility() == permitScope) {
            oplist.append(op);
        }
    }

    // do people REALLY want these comments? Hmm.
    /*
      if(forceSections() || oppub.count())
      {
      writeComment("public operations",getIndent(),cpp);
        writeBlankLine(cpp);
      }
    */
    writeOperations(c, oplist, isHeaderMethod, cpp);
}

// write operation in either header or source file
void CppWriter::writeOperations(UMLClassifier *c, UMLOperationList &oplist, bool isHeaderMethod, QTextStream &cpp)
{
    const bool generateEmptyConstructors =
        UMLApp::app()->getCommonPolicy()->getAutoGenerateConstructors();

    // generate method decl for each operation given
    foreach (UMLOperation* op, oplist  ) {
        QString doc;  // buffer for documentation
        QString methodReturnType;
        UMLAttributeList atl = op->getParmList();  // method parameters

        if (op->isConstructorOperation()) {
            if (generateEmptyConstructors && atl.count() == 0)
                continue;  // it's already been written, see writeConstructor{Decls,Methods}
        } else if (op->isDestructorOperation()) {
            if (generateEmptyConstructors)
                continue;  // it's already been written, see writeConstructor{Decls,Methods}
        } else {
            methodReturnType = fixTypeName(op->getTypeName());
            if(methodReturnType != "void")
                doc += "@return " + methodReturnType + '\n';
        }

        QString str;
        if (op->getAbstract() || c->isInterface()) {
            if (isHeaderMethod) {
                // declare abstract method as 'virtual'
                str += "virtual ";
            }
        }

        // static declaration for header file
        if (isHeaderMethod && op->getStatic())
            str += "static ";

        // returntype of method
        str += methodReturnType + ' ';

        if (!isHeaderMethod)
            str += className_ + "::";

        str += cleanName(op->getName()) + " (";

        // generate parameters
        uint j = 0;
        for (UMLAttributeListIt atlIt( atl ); atlIt.hasNext() ; j++) {
            UMLAttribute* at = atlIt.next();
            QString typeName = fixTypeName(at->getTypeName());
            QString atName = cleanName(at->getName());
            str += typeName + ' ' + atName;
            const QString initVal = at->getInitialValue();
            if (! initVal.isEmpty())
                str += " = " + initVal;
            if (j < ( uint )( atl.count() - 1 ))
                str += ", ";
            doc += "@param  " + atName + ' ' + at->getDoc() + m_endl;
        }
        doc = doc.remove(doc.size() - 1, 1);  // remove last endl of comment
        str += " )";

        if (op->getConst())
            str += " const";

        // method body : only gets IF its not in a header
        if (isHeaderMethod && !policyExt()->getOperationsAreInline()) {
            str += ';'; // terminate now
        }
        else {
            str += m_endl + getIndent() + '{' + m_endl;
            QString sourceCode = op->getSourceCode();
            if (sourceCode.isEmpty()) {
                // empty method body - TODO: throw exception
            }
            else {
                str += formatSourceCode(sourceCode, getIndent() + getIndent());
            }
            str += getIndent() + '}';
        }

        // write it out
        writeDocumentation("", op->getDoc(), doc, cpp);
        cpp << getIndent() << str << m_endl;
        writeBlankLine(cpp);
    }
}

// To prevent circular including when both classifiers on either end
// of an association have roles we need to have forward declaration of
// the other class...but only IF its not THIS class (as could happen
// in self-association relationship).
void CppWriter::printAssociationIncludeDecl (UMLAssociationList list, Uml::IDType myId, QTextStream &stream)
{

    foreach (UMLAssociation *a , list ) {
        UMLClassifier *current = NULL;
        bool isFirstClass = true;

        // only use OTHER classes (e.g. we don't need to write includes for ourselves!!
        // AND only IF the roleName is defined, otherwise, its not meant to be noticed.
        if (a->getObjectId(Uml::A) == myId && !a->getRoleName(Uml::B).isEmpty()) {
            current = dynamic_cast<UMLClassifier*>(a->getObject(Uml::B));
        } else if (a->getObjectId(Uml::B) == myId && !a->getRoleName(Uml::A).isEmpty()) {
            current = dynamic_cast<UMLClassifier*>(a->getObject(Uml::A));
            isFirstClass = false;
        }

        // as header doc for this method indicates, we need to be a bit sophisticated on
        // how to declare some associations.
        if (current) {
            if( !isFirstClass && !a->getRoleName(Uml::A).isEmpty() && !a->getRoleName(Uml::B).isEmpty())
                stream << "class " << current->getName() << ";" << m_endl; // special case: use forward declaration
            else
                stream << "#include \"" << current->getName() << ".h\"" << m_endl; // just the include statement
        }
    }
}

QString CppWriter::fixInitialStringDeclValue(const QString &value, const QString &type)
{
    QString val = value;
    // check for strings only
    if (!val.isEmpty() && type == policyExt()->getStringClassName()) {
        if (!val.startsWith('"'))
            val.prepend('"');
        if (!val.endsWith('"'))
            val.append('"');
    }
    return val;
}

// methods like this _shouldn't_ be needed IF we properly did things thruought the code.
QString CppWriter::getUMLObjectName(UMLObject *obj)
{
    return(obj!=0)?obj->getName():QString("NULL");
}

void CppWriter::writeBlankLine(QTextStream &stream)
{
    stream << m_endl;
}

void CppWriter::printTextAsSeparateLinesWithIndent (const QString &text, const QString &indent, QTextStream &stream)
{
    if(text.isEmpty())
        return;

    QStringList lines = text.split( '\n' );
    for (int i= 0; i < lines.count(); i++)
        stream << indent << lines[i] << m_endl;
}

QString CppWriter::getAttributeVariableName (UMLAttribute *at)
{
    QString fieldName = cleanName(at->getName());
    return fieldName;
}

QStringList CppWriter::defaultDatatypes()
{
    return Codegen_Utils::cppDatatypes();
}

const QStringList CppWriter::reservedKeywords() const
{
    return Codegen_Utils::reservedCppKeywords();
}

