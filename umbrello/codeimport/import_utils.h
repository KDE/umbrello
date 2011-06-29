/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef IMPORT_UTILS_H
#define IMPORT_UTILS_H

#include "basictypes.h"
#include "umlattributelist.h"

#include <QtCore/QStringList>

class UMLObject;
class UMLClassifier;
class UMLPackage;
class UMLOperation;
class UMLEnum;

/**
 * Utilities for code import
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Import_Utils {

    UMLObject* createUMLObject(UMLObject::ObjectType type,
                               const QString& name,
                               UMLPackage *parentPkg = NULL,
                               const QString& comment = QString(),
                               const QString& stereotype = QString());

    void putAtGlobalScope(bool yesno);

    void setRelatedClassifier(UMLClassifier *c);

    void assignUniqueIdOnCreation(bool yesno);

    UMLObject* insertAttribute(UMLClassifier *klass, Uml::Visibility scope,
                               const QString& name,
                               const QString& type,
                               const QString& comment = QString(),
                               bool isStatic = false);
    UMLObject* insertAttribute(UMLClassifier *klass, Uml::Visibility scope,
                               const QString& name,
                               UMLClassifier *attrType,
                               const QString& comment /* ="" */,
                               bool isStatic /* =false */);

    UMLOperation* makeOperation(UMLClassifier *parent, const QString &name);

    void insertMethod(UMLClassifier *klass, UMLOperation* &op,
                      Uml::Visibility scope, const QString& type,
                      bool isStatic, bool isAbstract,
                      bool isFriend = false, bool isConstructor = false,
                      const QString& comment = QString());

    UMLAttribute* addMethodParameter(UMLOperation *method,
                                     const QString& type,
                                     const QString& name);

    void addEnumLiteral(UMLEnum *enumType, const QString &literal,
                        const QString &comment = QString());

    void createGeneralization(UMLClassifier *child, UMLClassifier *parent);
    void createGeneralization(UMLClassifier *child, const QString &parentName);

    QString formatComment(const QString &comment);

    QStringList includePathList();

    void addIncludePath(const QString& path);

    bool newUMLObjectWasCreated();

    bool isDatatype(const QString& name, UMLPackage *parentPkg = NULL);

} // end namespace Import_Utils

#endif
