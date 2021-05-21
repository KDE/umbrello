/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas
    <brian.thomas@gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2014 Umbrello UML Modeller Authors
    <umbrello-devel@kde.org>
*/

#ifndef CPPWRITER_H
#define CPPWRITER_H

#include "simplecodegenerator.h"
#include "cppcodegenerationpolicy.h"
#include "umloperationlist.h"
#include "umlattributelist.h"
#include "umlassociationlist.h"

class QFile;

/**
 * Class CppWriter is a code generator for UMLClassifier objects.
 * Create an instance of this class, and feed it a UMLClassifier when
 * calling writeClass and it will generate both a header (.h) and
 * source (.cpp) file for that classifier.
 * Note:
 *   This is the "old" code generator that does not support code editing
 *   in the Modeller but uses significantly less file space because the
 *   source code is not replicated in the XMI file.
 */
class CppWriter : public SimpleCodeGenerator
{
public:

    CppWriter();
    virtual ~CppWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    QStringList defaultDatatypes();

    virtual QStringList reservedKeywords() const;

private:
    void writeIncludes(UMLClassifier *c, QTextStream &cpp);
    void writeClassDecl(UMLClassifier *c, QTextStream &cpp);
    void writeConstructorDecls(QTextStream &h);
    void writeConstructorMethods(UMLClassifier * c, QTextStream &cpp);

//    /**
//     * Write all field declarations, for both attributes and associations for the
//     * given permitted scope.
//     */
//    void writeFieldDecl(UMLClassifier *c, Uml::Visibility::Enum permitScope, QTextStream &stream);

//    /**
//     * Write all method declarations, for attributes and associations
//     * for the given permitted scope.
//     */
//    void writeAccessorMethodDecl(UMLClassifier *c, Uml::Visibility::Enum permitScope, QTextStream &stream);

    void writeOperations(UMLClassifier *c, bool isHeaderMethod, Uml::Visibility::Enum permitScope, QTextStream &cpp);
    void writeOperations(UMLClassifier *c, UMLOperationList &oplist, bool isHeaderMethod, QTextStream &cpp);

//    /**
//     * Write all attributes for a given class.
//     * @param c   the class for which we are generating code
//     * @param j   the stream associated with the output file
//     */
//    void writeAttributes(UMLClassifier *c, QTextStream &j);

    void writeAttributeDecls(UMLClassifier *c, Uml::Visibility::Enum visibility, bool writeStatic, QTextStream &stream);

    void writeHeaderFieldDecl(UMLClassifier *c, Uml::Visibility::Enum permitVisibility, QTextStream &stream);

    void writeHeaderAttributeAccessorMethods(UMLClassifier *c, Uml::Visibility::Enum visibility, bool writeStatic, QTextStream &stream);

    void writeHeaderAttributeAccessorMethodDecls(UMLClassifier *c, Uml::Visibility::Enum permitVisibility, QTextStream &stream);
    void writeHeaderAccessorMethodDecl(UMLClassifier *c, Uml::Visibility::Enum permitScope, QTextStream &stream);

    void writeAssociationDecls(UMLAssociationList associations, Uml::Visibility::Enum permit, Uml::ID::Type id, QTextStream &stream);

    void writeAssociationRoleDecl(QString fieldClassName, QString roleName, QString multi,
                                  QString doc, QTextStream &stream);

    void writeAttributeMethods(UMLAttributeList attribs, Uml::Visibility::Enum visib, bool isHeaderMethod,
                               bool isStatic,
                               bool writeMethodBody, QTextStream &stream);

    void writeAssociationMethods(UMLAssociationList associations, Uml::Visibility::Enum permitVisib,
                                 bool isHeaderMethod,
                                 bool writeMethodBody, bool writePointerVar, Uml::ID::Type id, QTextStream &stream);

    void writeAssociationRoleMethod(const QString &fieldClassName, bool isHeaderMethod, bool writeMethodBody,
                                    const QString &roleName, const QString &multi,
                                    const QString &description, Uml::Changeability::Enum change,
                                    QTextStream &stream);

    void writeSingleAttributeAccessorMethods(
            const QString &fieldClassName, const QString &Name,
            const QString &fieldName, const QString &description,
            Uml::Changeability::Enum change,
            bool isHeaderMethod,
            bool isStatic, bool writeMethodBody, QTextStream &cpp);

    void writeVectorAttributeAccessorMethods(
            const QString &fieldClassName, const QString &fieldVarName,
            const QString &fieldName, const QString &description,
            Uml::Changeability::Enum change,
            bool isHeaderMethod,
            bool writeMethodBody,
            QTextStream &stream);

    void writeComment(const QString &text, const QString &indent, QTextStream &cpp);
    void writeDocumentation(QString header, QString body, QString end, QTextStream &cpp);

    void writeHeaderFile(UMLClassifier *c, QFile &file);
    void writeSourceFile(UMLClassifier *c, QFile &file);

    void printTextAsSeparateLinesWithIndent (const QString &text, const QString &indent,
                                             QTextStream &stream);

    void printAssociationIncludeDecl(UMLAssociationList list, Uml::ID::Type this_id, QTextStream &stream);

    void writeInitAttributeMethod(UMLClassifier * c, QTextStream &stream);
    void writeInitAttributeDecl(UMLClassifier * c, QTextStream &stream);
    void writeDataTypes(UMLClassifier *c, Uml::Visibility::Enum permitScope, QTextStream &stream);

    QString umlObjectName(UMLObject *obj);

    QString fixTypeName(const QString &string);
    QString fixInitialStringDeclValue(const QString &value, const QString &type);

    QString getAttributeVariableName(UMLAttribute *at);
    QString getAttributeMethodBaseName(const QString &fieldName);

    void writeBlankLine(QTextStream &stream);

    CPPCodeGenerationPolicy *policyExt();

    QString VECTOR_METHOD_APPEND;
    QString VECTOR_METHOD_REMOVE;
    QString VECTOR_METHOD_INIT;
    QString OBJECT_METHOD_INIT;

    /**
     * Create association methods for class attributes/associations/operations as inline decl in header.
     */
    bool INLINE_ASSOCIATION_METHODS;

    QStringList ObjectFieldVariables;
    QStringList VectorFieldVariables;
    bool m_stringIncludeRequired;
};


#endif // CPPWRITER_H
