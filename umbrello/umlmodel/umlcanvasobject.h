/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CANVASOBJECT_H
#define CANVASOBJECT_H

#include "umlobject.h"
#include "umlobjectlist.h"
#include "umlclassifierlist.h"
#include "umlassociationlist.h"

/**
 * This class contains the non-graphical information required for UMLObjects
 * which appear as moveable widgets on the scene.
 *
 * This class inherits from @ref UMLObject which contains most of the
 * information.
 * It is not instantiated itself, it's just used as a super class for
 * actual model objects.
 * It adds a member of type UMLObjectList containing subordinate objects.
 * The subordinate objects could be associations, attributes, operations,
 * or other items depending on the inheriting class.
 *
 * @short Non-graphical information for a UMLCanvasObject.
 * @author Jonathan Riddell
 * @see UMLObject
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLCanvasObject : public UMLObject
{
    Q_OBJECT

public:
    explicit UMLCanvasObject(const QString & name = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~UMLCanvasObject();

    bool operator==(const UMLCanvasObject& rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    // The abstract method UMLObject::clone() is implemented
    // in the classes inheriting from UMLCanvasObject.

    bool addAssociationEnd(UMLAssociation* assoc);

    bool hasAssociation(UMLAssociation* assoc) const;

    int removeAssociationEnd(UMLAssociation *assoc);

    void removeAllAssociationEnds();

    int associations() const;

    UMLAssociationList getAssociations() const;

    UMLAssociationList getSpecificAssocs(Uml::AssociationType::Enum assocType) const;

    UMLClassifierList getSuperClasses(bool withRealizations = true) const;
    UMLClassifierList getSubClasses() const;

    virtual UMLAssociationList getRealizations() const;

    UMLAssociationList getAggregations() const;
    UMLAssociationList getCompositions() const;
    UMLAssociationList getRelationships() const;

    virtual UMLObject *findChildObject(const QString &n,
                                       UMLObject::ObjectType t = UMLObject::ot_UMLObject) const;
    virtual UMLObject *findChildObjectById(Uml::ID::Type id, bool considerAncestors = false) const;

    virtual QString uniqChildName(const UMLObject::ObjectType type,
                                  const QString &prefix = QString()) const;
    virtual void removeAllChildObjects();

    UMLObjectList &subordinates() const {
        m_List.removeAll(nullptr);
        return m_List;
    }

    virtual bool resolveRef();

    // The abstract method UMLObject::saveToXMI() is implemented
    // in the classes inheriting from UMLCanvasObject.

private:

    /**
     * List of all the associations in this object.
     * Inheriting classes add more types of objects that are possible in this list;
     * for example, UMLClassifier adds operations, attributes, and templates.
     *
     * @todo Only a pointer to the appropriate association end object
     *       (UMLRole) should be saved here, not the entire UMLAssociation.
     */
    mutable UMLObjectList m_List;

signals:

    void sigAssociationEndAdded(UMLAssociation * assoc);
    void sigAssociationEndRemoved(UMLAssociation * assoc);

};

#endif
