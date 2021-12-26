/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef PACKAGE_H
#define PACKAGE_H

#include "umlcanvasobject.h"
#include "umlclassifierlist.h"
#include "umlentitylist.h"

// forward declarations
class UMLAssociation;

/**
 * This class contains the non-graphical information required for a UML
 * Package.
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 *
 * @short Non-graphical information for a Package.
 * @author Jonathan Riddell
 * @see UMLCanvasObject
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLPackage : public UMLCanvasObject
{
    Q_OBJECT
public:
    explicit UMLPackage(const QString & name = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~UMLPackage();

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    bool addObject(UMLObject *pObject, bool interactOnConflict = true);
    void removeObject(UMLObject *pObject);

    virtual void removeAllObjects();

    UMLObjectList containedObjects(bool includeInactive = false) const;

    void addAssocToConcepts(UMLAssociation* assoc);
    void removeAssocFromConcepts(UMLAssociation *assoc);

    UMLObject * findObject(const QString &name) const;
    UMLObject * findObjectById(Uml::ID::Type id) const;

    void appendPackages(UMLPackageList& packages, bool includeNested = true) const;
    void appendClassifiers(UMLClassifierList& classifiers,
                            bool includeNested = true) const;
    void appendClassesAndInterfaces(UMLClassifierList& classifiers,
                                    bool includeNested = true) const;
    void appendEntities(UMLEntityList& entities,
                        bool includeNested = true) const;

    virtual bool resolveRef();

    virtual void saveToXMI(QXmlStreamWriter& writer);

protected:
    virtual bool load1(QDomElement& element);

    /**
     * References to the objects contained in this package.
     * The UMLPackage is the owner of the objects.
     * The objects stored here are "standalone" objects which have an existence
     * on their own (such as classifiers, packages).
     * On the other hand, the objects stored in UMLCanvasObject::m_List are
     * "subordinate" objects which cannot exist on their own (such as attributes,
     * operations). They are always slaved to a "standalone" object.
     *
     * This design may be revisited - m_objects could be merged into
     * UMLCanvasObject::m_List.
     */
    UMLObjectList m_objects;

};

#endif
