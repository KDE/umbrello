/*
 *  copyright (C) 2003-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENTITY_H
#define ENTITY_H

#include "classifier.h"
#include "umlentityattributelist.h"
#include "umlclassifierlistitemlist.h"

/**
 * This class contains the non-graphical information required for a UML
 * Entity.
 * This class inherits from @ref UMLClassifier which contains most of the
 * information.
 *
 * The @ref UMLDoc class creates instances of this type.  All Entitys
 * will need a unique id.  This will be given by the @ref UMLDoc class.
 * If you don't leave it up to the @ref UMLDoc class then call the method
 * @ref UMLDoc::getUniqueID to get a unique id.
 *
 * @short Non-graphical Information for an Entity.
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLEntity : public UMLClassifier {
    Q_OBJECT
public:
    /**
     * Sets up an entity.
     *
     * @param name		The name of the Entity.
     * @param id		The unique id of the Entity.
     */
    UMLEntity(const QString& name = "", Uml::IDType id = Uml::id_None);

    /**
     * Standard deconstructor.
     */
    virtual ~UMLEntity();

    /**
     * Overloaded '==' operator.
     */
    bool operator==(UMLEntity& rhs);

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto(UMLEntity* rhs) const;

    /**
     * Make a clone of this object.
     */
    virtual UMLObject* clone() const;

    /**
     * Creates an entity attribute for the parent concept.
     *
     * @param o	The parent concept
     * @param name  An optional name, used by when creating through UMLListView
     * @return	The UMLEntityAttribute created
     */
    UMLObject* createEntityAttribute(const QString &name = QString::null);

    /**
    	 * Adds an entityAttribute to the entity.
    	 *
    	 * @param name		The name of the entityAttribute.
    * @param id		The id of the entityAttribute (optional.)
    *			If omitted a new ID is assigned internally.
    * @return	Pointer to the UMLEntityAttribute created.
    */
    UMLObject* addEntityAttribute(const QString &name, Uml::IDType id = Uml::id_None);

    /**
     * Adds an already created entityAttribute.
     * The entityAttribute object must not belong to any other concept.
     	 *
     	 * @param Att		Pointer to the UMLEntityAttribute.
     * @param Log		Pointer to the IDChangeLog.
     * @return	True if the entityAttribute was sucessfully added.
     */
    bool addEntityAttribute(UMLEntityAttribute* att, IDChangeLog* Log = 0);

    /**
     * Adds an entityAttribute to the entity, at the given position.
     * If position is negative or too large, the entityAttribute is added
     * to the end of the list.
     	 *
     	 * @param Att		Pointer to the UMLEntityAttribute.
     * @param position	Position index for the insertion.
     * @return	True if the entityAttribute was sucessfully added.
     */
    //TODO:  give default value -1 to position (append) - now it conflicts with the method above..
    bool addEntityAttribute(UMLEntityAttribute* att, int position );

    /**
     * Removes an entityAttribute from the class.
     *
     * @param a		The entityAttribute to remove.
     * @return	Count of the remaining entityAttributes after removal.
     *		Returns -1 if the given entityAttribute was not found.
     */
    int removeEntityAttribute(UMLObject* a);

    /**
     * Take and return an entityAttribute from class.
     * It is the callers responsibility to pass on ownership of
     * the returned entityAttribute (or to delete the entityAttribute)
     *
     * @param  el entityAttribute to tkae
     * @param wasAtIndex	if given, the index in m_List of the item taken
     *			is returned in the int pointed-to.
     * @return pointer to the entityAttribute or null if not found.
     */
    UMLEntityAttribute* takeEntityAttribute(UMLEntityAttribute* el, int *wasAtIndex = NULL);

    /**
     * Returns the number of entityAttributes for the class.
     *
     * @return	The number of entityAttributes for the class.
     */
    int entityAttributes();

    /**
     * Find a list of entityAttributes with the given name.
     *
     * @param t		The type to find.
     * @param n		The name of the object to find.
     * @return	List of objects found.  Will be empty if none found.
     */
    virtual UMLObjectList findChildObject(Uml::Object_Type t, const QString &n);

    /**
     * Creates the <UML:Entity> element including its entityliterals.
     */
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

signals:
    void entityAttributeAdded(UMLObject*);
    void entityAttributeRemoved(UMLObject*);

protected:
    /**
     * Loads the <UML:Entity> element including its entityAttributes.
     */
    bool load(QDomElement& element);

private:
    /**
     * Initializes key variables of the class.
     */
    void init();

};

#endif // ENTITY_H

