/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ENTITY_H
#define ENTITY_H

#include "classifier.h"

//forward declarations
class UMLEntityAttribute;
class UMLEntityConstraint;
class UMLUniqueConstraint;
class UMLForeignKeyConstraint;
class UMLCheckConstraint;
class UMLEntityConstraintList;
class UMLEntityAttributeList;

/**
 * This class contains the non-graphical information required for a UML
 * Entity.
 * This class inherits from @ref UMLClassifier which contains most of the
 * information.
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
     * @param name              The name of the Entity.
     * @param id                The unique id of the Entity.
     */
    explicit UMLEntity(const QString& name = "", Uml::IDType id = Uml::id_None);

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
     * Reimplementation of method from UMLClassifier.
     *
     * @param name  An optional name, used by when creating through UMLListView
     * @param type  An optional type, used by when creating through UMLListView
     * @return  The UMLEntityAttribute created
     */
    UMLAttribute* createAttribute(const QString &name = QString(),
                                  UMLObject *type = 0);

    /**
     * Creates a Unique Constraint for this Entity. 
     *
     * @param name An optional name
     * @return The UniqueConstraint created
     */ 
    UMLUniqueConstraint* createUniqueConstraint(const QString &name = QString());

    /**
     * Creates a Foreign Key  Constraint for this Entity. 
     *
     * @param name An optional name
     * @return The ForeignKeyConstraint created
     */ 
    UMLForeignKeyConstraint* createForeignKeyConstraint(const QString &name = QString());
    
    /**
     * Creates a Check  Constraint for this Entity. 
     *
     * @param name An optional name
     * @return The CheckConstraint created
     */ 
    UMLCheckConstraint* createCheckConstraint(const QString &name = QString());
    
    /**
         * Adds an entityAttribute to the entity.
         *
         * @param name          The name of the entityAttribute.
    * @param id         The id of the entityAttribute (optional.)
    *                   If omitted a new ID is assigned internally.
    * @return   Pointer to the UMLEntityAttribute created.
    */
    UMLObject* addEntityAttribute(const QString &name, Uml::IDType id = Uml::id_None);

    /**
     * Adds an already created entityAttribute.
     * The entityAttribute object must not belong to any other concept.
         *
         * @param att           Pointer to the UMLEntityAttribute.
     * @param Log               Pointer to the IDChangeLog.
     * @return  True if the entityAttribute was successfully added.
     */
    bool addEntityAttribute(UMLEntityAttribute* att, IDChangeLog* Log = 0);

    /**
     * Adds an entityAttribute to the entity, at the given position.
     * If position is negative or too large, the entityAttribute is added
     * to the end of the list.
         *
         * @param att           Pointer to the UMLEntityAttribute.
     * @param position  Position index for the insertion.
     * @return  True if the entityAttribute was successfully added.
     */
    //TODO:  give default value -1 to position (append) - now it conflicts with the method above..
    bool addEntityAttribute(UMLEntityAttribute* att, int position );

    /**
     * Removes an entityAttribute from the class.
     *
     * @param a         The entityAttribute to remove.
     * @return  Count of the remaining entityAttributes after removal.
     *          Returns -1 if the given entityAttribute was not found.
     */
    int removeEntityAttribute(UMLClassifierListItem* a);

    /**
     * Emit the entityAttributeRemoved signal.
     */
    void signalEntityAttributeRemoved(UMLClassifierListItem *eattr);

    /**
     * Returns the number of entityAttributes for the class.
     *
     * @return  The number of entityAttributes for the class.
     */
    int entityAttributes() ;

    /**
     * Sets the UniqueConstraint passed as the Primary Key of this Entity
     * If the UniqueConstraint exists, then it is made a primary key
     * Else the UniqueConstraint is added and set as PrimaryKey
     *
     * @param uconstr The Unique Constraint that is  to be set as Primary Key
     * @return true if Primary key could be set successfully
     */
    bool setAsPrimaryKey(UMLUniqueConstraint* uconstr);


    /**
     * Unset a Primary Key Constraint if it exists, else does nothing
     * This function will make the primary key into just another UniqueConstraint
     * if it exists
     */
    void unsetPrimaryKey();

    /**
     * Checks if This UMLEntity has a primary key set
     *
     * @return true if a Primary Key Exists for this UMLEntity
     */
    bool hasPrimaryKey() const;

    /**
     * Checks if a given Unique Constraint is primary key of this entity
     * @param uConstr A Unique Constraint
     * @return bool true if passed paramater is a primary key of this entity
     */
    bool isPrimaryKey(UMLUniqueConstraint* uConstr) const;

    /**
     * Adds a Constraint to this UMLEntity
     * To set a UMLUniqueConstraint as Primary Key use setAsPrimaryKey 
     *
     * @param constr The UMLEntityConstraint that is to be added
     * @return true if the constraint could be added successfully
     */
    bool addConstraint(UMLEntityConstraint* constr);


    /**
     * Removes an existing constraint from this UMLEntity
     * If the Contraint is a Primary Key, this Entity will no longer have a PrimaryKey 
     *
     * @param constr The constraint to be removed
     * @return true if the constraint could be removed successfully
     */
    bool removeConstraint(UMLEntityConstraint* constr);

    /**
     * Resolve the types referenced by our UMLEntityAttributes.
     * Reimplements the method from UMLClassifier.
     */
    virtual bool resolveRef();

    /**
     * Creates the <UML:Entity> element including its entityliterals.
     */
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Reimplementation of getFilteredList to support ot=Uml::ot_EntityConstraint
     */
    UMLClassifierListItemList getFilteredList(Uml::Object_Type ot) const;

    /**
     * Returns the Entity Attributes
     * Same as getFilteredList(Uml::ot_EntityAttribute)
     */
    UMLEntityAttributeList getEntityAttributes() const;

private slots:
    void slotEntityAttributeRemoved(UMLClassifierListItem*);

signals:
    void entityAttributeAdded(UMLClassifierListItem*);
    void entityAttributeRemoved(UMLClassifierListItem*);
    void entityConstraintAdded(UMLClassifierListItem*);
    void entityConstraintRemoved(UMLClassifierListItem*);

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

    /*
     * Primary Key of this Entity
     * This is a pointer kept for easy access to the primary key, and to distinguish it
     * from all other UniqueConstraints. It is also there in m_List ( inherited from
     * UMLCanvasObject )
     *
     */
    UMLUniqueConstraint* m_PrimaryKey;


};

#endif // ENTITY_H

