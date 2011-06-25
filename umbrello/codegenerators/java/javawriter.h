/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003       Brian Thomas                                 *
 *                           <brian.thomas@gsfc.nasa.gov>                  *
 *   copyright (C) 2004-2011  Umbrello UML Modeller Authors                *
 *                           <uml-devel@uml.sf.net>                        *
 ***************************************************************************/

#ifndef JAVAWRITER_H
#define JAVAWRITER_H

#include "simplecodegenerator.h"
#include "umloperationlist.h"
#include "umlattributelist.h"
#include "umlassociationlist.h"

class UMLOperation;

/**
 * Class JavaWriter is a code generator for UMLClassifier objects.
 * Create an instance of this class, and feed it a UMLClassifier when
 * calling writeClass and it will generate a java source file for
 * that concept.
 * This is the "old" code generator that does not support code editing
 * in the Modeller but uses significantly less file space because the
 * source code is not replicated in the XMI file.
 */
class JavaWriter : public SimpleCodeGenerator
{
public:
    JavaWriter();
    virtual ~JavaWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage language() const;

    QStringList defaultDatatypes();

private:
    void writeClassDecl(UMLClassifier *c, QTextStream &java);

    void writeConstructor(UMLClassifier *c, QTextStream &java);

    static bool compareJavaMethod(UMLOperation *op1, UMLOperation *op2);

    static bool javaMethodInList(UMLOperation *umlOp, UMLOperationList &opl);

    void getSuperImplementedOperations(UMLClassifier *c, UMLOperationList &yetImplementedOpList ,UMLOperationList &toBeImplementedOpList, bool noClassInPath = true);

    void getInterfacesOperationsToBeImplemented(UMLClassifier *c, UMLOperationList &opl);

    void writeOperations(UMLClassifier *c, QTextStream &j);

    void writeOperations(UMLOperationList &list, QTextStream &j);

    void writeAttributeDecls(UMLAttributeList &atpub, UMLAttributeList &atprot,
                             UMLAttributeList &atpriv, QTextStream &java );

    void writeAssociationDecls(UMLAssociationList associations, Uml::IDType id, QTextStream &java);

    void writeAssociationRoleDecl(QString fieldClassName, QString roleName, QString multi,
                                  QString doc, Uml::Visibility visib, QTextStream &java);

    void writeAttributeMethods(UMLAttributeList &atpub, Uml::Visibility visibility, QTextStream &java);

    void writeAssociationMethods(UMLAssociationList associations, UMLClassifier *thisClass,
                                 QTextStream &java);

    void writeAssociationRoleMethod(QString fieldClassName, QString roleName, QString multi,
                                    QString description, Uml::Visibility visib, Uml::Changeability change,
                                    QTextStream &java);

    void writeSingleAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
            QString fieldName, QString description,
            Uml::Visibility visibility, Uml::Changeability change,
            bool isFinal, QTextStream &java);

    void writeVectorAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
            QString fieldName, QString description,
            Uml::Visibility visibility, Uml::Changeability change,
            QTextStream &java);

    void writeComment(const QString &text, const QString &indent, QTextStream &java, bool javaDocStyle=false);

    void writeDocumentation(QString header, QString body, QString end, QString indent, QTextStream &java);

    QString getUMLObjectName(UMLObject *obj);

    QString fixTypeName(const QString& string);

    QString fixInitialStringDeclValue(const QString& val, const QString& type);

    void writeBlankLine(QTextStream& java);

    QString m_startline;    ///< a \n, used at the end of each line
    bool    m_isInterface;  ///< whether or not this concept is an interface

};

#endif // JAVAWRITER_H
