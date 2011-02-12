/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CSHARPWRITER_H
#define CSHARPWRITER_H

#include "classifierlistitem.h"
#include "simplecodegenerator.h"
#include "umlassociationlist.h"
#include "umlattributelist.h"
#include "umloperationlist.h"

/**
 * Class CSharpWriter is a C# code generator for UMLClassifier objects
 * Just call writeClass and feed it a UMLClassifier;
 * @author Ferenc Veres
 */
class CSharpWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:
    CSharpWriter();
    virtual ~CSharpWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage language() const;

    virtual QStringList reservedKeywords() const;

    QStringList defaultDatatypes();

private:

    /**
     * Adds extra indenting if the class is in a container (namespace).
     */
    QString m_container_indent;

    /**
     * Collection of included namespaces, to skip them from variable types.
     */
    UMLPackageList m_seenIncludes;

    /**
     * Counts associations without a role name for giving a default name.
     */
    int m_unnamedRoles;

    void writeRealizationsRecursive(UMLClassifier *currentClass,
                                    UMLAssociationList *realizations,
                                    QTextStream &cs);

    void writeOperations(UMLClassifier *c, QTextStream &cs);
    void writeOperations(UMLOperationList opList,
                         QTextStream &cs,
                         bool interface = false,
                         bool isOverride = false,
                         bool generateErrorStub = false);

    void writeOverridesRecursive(UMLClassifierList *superclasses, QTextStream &cs);

    void writeAttributes(UMLClassifier *c, QTextStream &cs);
    void writeAttributes(UMLAttributeList &atList, QTextStream &cs);

    void writeAssociatedAttributes(UMLAssociationList &associated, UMLClassifier *c, QTextStream &cs);

    void writeAttribute(const QString& doc,
                        Uml::Visibility visibility,
                        bool isStatic,
                        const QString& typeName,
                        const QString& name,
                        const QString& initialValue,
                        bool asProperty,
                        QTextStream &cs);

    QString makeLocalTypeName(UMLClassifierListItem *cl);

};

#endif
