/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef IMPORT_UTILS_H
#define IMPORT_UTILS_H

#include "basictypes.h"
#include "folder.h"
#include "umlattributelist.h"

#include <QStringList>

class UMLObject;
class UMLClassifier;
class UMLPackage;
class UMLOperation;
class UMLEnum;
class UMLScene;

class QMimeData;

/**
 * Utilities for code import
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
namespace Import_Utils {

    UMLFolder *createSubDir(const QString& name,
                            UMLFolder *parentPkg,
                            const QString &comment = QString());

    UMLObject *createArtifactFolder(const QString& name,
                                    UMLPackage *parentPkg,
                                    const QString &comment);

    UMLObject *createArtifact(const QString& name,
                              UMLFolder *parentPkg = nullptr,
                              const QString &comment = QString());

    void checkStdString(QString& typeName);

    UMLObject* createUMLObject(UMLObject::ObjectType type,
                               const QString& name,
                               UMLPackage *parentPkg = nullptr,
                               const QString& comment = QString(),
                               const QString& stereotype = QString(),
                               bool searchInParentPackageOnly = false,
                               bool remapParent = true);

    UMLObject* createUMLObjectHierarchy(UMLObject::ObjectType type, const QString &name, UMLPackage *parentPkg);

    void putAtGlobalScope(bool yesno);

    void setRelatedClassifier(UMLClassifier *c);

    void assignUniqueIdOnCreation(bool yesno);

    UMLAttribute* insertAttribute(UMLClassifier *klass, Uml::Visibility::Enum scope,
                               const QString& name,
                               const QString& type,
                               const QString& comment = QString(),
                               bool isStatic = false);
    UMLAttribute *insertAttribute(UMLClassifier *klass, Uml::Visibility::Enum scope,
                               const QString& name,
                               UMLClassifier *attrType,
                               const QString& comment = QString(),
                               bool isStatic = false);

    UMLOperation* makeOperation(UMLClassifier *parent, const QString &name);

    void insertMethod(UMLClassifier *klass, UMLOperation* &op,
                      Uml::Visibility::Enum scope, const QString& type,
                      bool isStatic, bool isAbstract,
                      bool isFriend = false, bool isConstructor = false, bool isDestructor = false,
                      const QString& comment = QString());

    UMLAttribute* addMethodParameter(UMLOperation *method,
                                     const QString& type,
                                     const QString& name);

    void addEnumLiteral(UMLEnum *enumType, const QString &literal,
                        const QString &comment = QString(),
                        const QString &value = QString());

    UMLAssociation *createGeneralization(UMLClassifier *child, UMLClassifier *parent);
    void createGeneralization(UMLClassifier *child, const QString &parentName);

    UMLEnum *remapUMLEnum(UMLObject *ns, UMLPackage *currentScope);

    QString formatComment(const QString &comment);

    QStringList includePathList();

    void addIncludePath(const QString& path);

    bool newUMLObjectWasCreated();

    bool isDatatype(const QString& name, UMLPackage *parentPkg = nullptr);

    UMLPackage *globalScope();

} // end namespace Import_Utils

#endif
