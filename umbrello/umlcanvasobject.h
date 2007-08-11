/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CANVASOBJECT_H
#define CANVASOBJECT_H

#include "umlobject.h"
#include "umlobjectlist.h"
#include "umlclassifierlist.h"
#include "umlassociationlist.h"


/**
 * This class contains the non-graphical information required for UMLObjects
 * which appear as moveable widgets on the canvas.
 *
 * This class inherits from @ref UMLObject which contains most of the
 * information.
 * It is not instantiated itself, it's just used as a super class for
 * actual model objects.
 *
 * @short Non-graphical information for a UMLCanvasObject.
 * @author Jonathan Riddell
 * @see UMLObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLCanvasObject : public UMLObject {
    Q_OBJECT
public:
    /**
     * Sets up a UMLCanvasObject.
     *
     * @param name              The name of the Concept.
     * @param id                The unique id of the Concept.
     */
    explicit UMLCanvasObject(const QString & name = "", Uml::IDType id = Uml::id_None);

    /**
     * Standard deconstructor.
     */
    virtual ~UMLCanvasObject();

    /**
     *  Overloaded '==' operator
     */
    virtual bool operator==(UMLCanvasObject& rhs);

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto(UMLCanvasObject *rhs) const;

    // The abstract method UMLObject::clone() is implemented
    // in the classes inheriting from UMLCanvasObject.

    /**
     * Adds an association end to m_List.
     *
     * @param assoc  The association to add.
     *               @todo change param type to UMLRole
     */
    bool addAssociationEnd(UMLAssociation* assoc);

    /**
     * Determine if this canvasobject has the given association.
     *
     * @param assoc             The association to check.
     */
    bool hasAssociation(UMLAssociation* assoc);

    /**
     * Remove an association end from the CanvasObject.
     *
     * @param assoc         The association to remove.
     *                  @todo change param type to UMLRole
     */
    int removeAssociationEnd(UMLAssociation *assoc);

    /**
     * Remove all association ends from the CanvasObject.
     */
    void removeAllAssociationEnds();

    /**
     * Returns the number of associations for the CanvasObject.
     * This is the sum of the aggregations and compositions.
     *
     * @return  The number of associations for the Concept.
     */
    int associations();

    /**
    * Return the list of associations for the CanvasObject.
    *
    * @return   The list of associations for the CanvasObject.
    */
    UMLAssociationList getAssociations();

    /**
     * Return the subset of m_List that matches the given type.
     *
     * @param assocType The Association_Type to match.
     * @return  The list of associations that match assocType.
     */
    UMLAssociationList getSpecificAssocs(Uml::Association_Type assocType);

    /**
     * Return a list of the superclasses of this concept.
     * TODO: This overlaps with UMLClassifier::findSuperClassConcepts(),
     *       see if we can merge the two.
     *
     * @return  The list of superclasses for the concept.
     */
    UMLClassifierList getSuperClasses();

    /**
     * Return a list of the classes that inherit from this concept.
     * TODO: This overlaps with UMLClassifier::findSubClassConcepts(),
     *       see if we can merge the two.
     *
     * @return  The list of classes inheriting from the concept.
     */
    UMLClassifierList getSubClasses();

    /**
     * Shorthand for getSpecificAssocs(Uml::at_Realization)
     *
     * @return  The list of realizations for the Concept.
     */
    virtual UMLAssociationList getRealizations();

    /**
     * Shorthand for getSpecificAssocs(Uml::at_Aggregation)
     *
     * @return  The list of aggregations for the Concept.
     */
    UMLAssociationList getAggregations();

    /**
     * Shorthand for getSpecificAssocs(Uml::at_Composition)
     *
     * @return  The list of compositions for the Concept.
     */
    UMLAssociationList getCompositions();

    /**
     * Shorthand for getSpecificAssocs(Uml::at_Relationship)
     *
     * @return  The list of relationships for the entity.
     */
    UMLAssociationList getRelationships();

    /**
     * Find a child object with the given name.
     *
     * @param n         The name of the object to find.
     * @param t         The type to find (optional.) If not given then
     *                  any object type will match.
     * @return  Pointer to the object found; NULL if none found.
     */
    virtual UMLObject *findChildObject(const QString &n, Uml::Object_Type t = Uml::ot_UMLObject);

    /**
     * Find an association.
     *
     * @param id        The id of the object to find.
     * @param considerAncestors boolean switch to consider ancestors while searching
     * @return  Pointer to the object found (NULL if not found.)
     */
    virtual UMLObject *findChildObjectById(Uml::IDType id, bool considerAncestors = false);

    /**
     * Returns a name for the new association, operation, template
     * or attribute appended with a number if the default name is
     * taken e.g. new_association, new_association_1 etc.
     *
     * @param type      The object type for which to make a name.
     * @param prefix    Optional prefix to use for the name.
     *                  If not given then uniqChildName() will choose the prefix
     *                  internally based on the object type.
     * @return  Unique name string for the Object_Type given.
     */
    virtual QString uniqChildName(const Uml::Object_Type type,
                                  const QString &prefix = QString());

    virtual void removeAllChildObjects();

    /**
     * Return the list of subordinate items.
     */
    UMLObjectList subordinates() const {
        return m_List;
    }

    /**
     * Reimplementation of UMLObject method.
     */
    virtual bool resolveRef();

    // The abstract method UMLObject::saveToXMI() is implemented
    // in the classes inheriting from UMLCanvasObject.

protected:

    /**
     * List of all the associations in this object.
     * Inheriting classes add more types of objects that are possible in this list;
     * for example, UMLClassifier adds operations, attributes, and templates.
     *
     * @todo Only a pointer to the appropriate assocation end object
     *       (UMLRole) should be saved here, not the entire UMLAssociation.
     *
     */
    UMLObjectList m_List;

private:

    /**
     * Initialises key variables of the class.
     */
    void init();

signals:

    /**
     * Emit when new association is added.
     * @param assoc Pointer to the association which has been added.
     */
    void sigAssociationEndAdded(UMLAssociation * assoc);

    /**
     * Emit when new association is removed.
     * @param assoc Pointer to the association which has been removed.
     */
    void sigAssociationEndRemoved(UMLAssociation * assoc);

};

#endif
