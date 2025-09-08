/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

//
// C++ Interface: valawriter
//

#ifndef VALAWRITER_H
#define VALAWRITER_H

#include "umlclassifierlistitem.h"
#include "simplecodegenerator.h"
#include "umlassociationlist.h"
#include "umlattributelist.h"
#include "umloperationlist.h"

/**
 * Class ValaWriter is a Vala code generator for UMLClassifier objects
 * Just call writeClass and feed it a UMLClassifier;
 */
class ValaWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:
    ValaWriter();
    virtual ~ValaWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList reservedKeywords() const;

    QStringList defaultDatatypes() const;

private:

    /// Adds extra indenting if the class is in a container (namespace).
    QString m_container_indent;

    /// Collection of included namespaces, to skip them from variable types.
    UMLPackageList m_seenIncludes;

    /// Counts associations without a role name for giving a default name.
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
                        Uml::Visibility::Enum visibility,
                        bool isStatic,
                        const QString& typeName,
                        const QString& name,
                        const QString& initialValue,
                        bool asProperty,
                        QTextStream &cs);

    QString makeLocalTypeName(UMLClassifierListItem *cl);

};

#endif
