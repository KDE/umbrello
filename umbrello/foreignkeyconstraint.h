/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef FOREIGNKEYCONSTRAINT_H
#define FOREIGNKEYCONSTRAINT_H

//qt includes
#include <qmap.h>
#include "entityconstraint.h"
#include "umlnamespace.h"


//forward declarations
class UMLEntityAttribute;
class UMLEntity;

/**
 * This class is used to set up information for a foreign key entity constraint.
 *
 * @short Sets up Foreign Key  entity constraint information.
 * @author Sharan Rao
 * @see UMLObject UMLClassifierListItem UMLEntityConstraint
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLForeignKeyConstraint : public UMLEntityConstraint {

     Q_OBJECT

public:

    /**
     * Update/Delete Action: Action to be taken on Update or Delete of a referenced attribute
     * is either, No Action, Restrict, Cascade, Set NULL , Set Default
     */
    enum UpdateDeleteAction { uda_NoAction = 0 , uda_Restrict , 
			      uda_Cascade , uda_SetNull, uda_SetDefault } ;

    /**
     * Sets up a constraint.
     *
     * @param parent    The parent of this UMLForeignKeyConstraint.
     * @param name      The name of this UMLForeignKeyConstraint.
     * @param id        The unique id given to this UMLForeignKeyConstraint.
     */
    UMLForeignKeyConstraint(const UMLObject *parent, const QString& name,
			    Uml::IDType id = Uml::id_None );

    /**
     * Sets up a constraint.
     *
     * @param parent    The parent of this UMLForeignKeyConstraint.
     */
    UMLForeignKeyConstraint(const UMLObject *parent);

    /**
     * Overloaded '==' operator
     */
    bool operator==( UMLForeignKeyConstraint &rhs);

    /**
     * destructor.
     */
    virtual ~UMLForeignKeyConstraint();

    /**
     * Copy the internal presentation of this object into the UMLForeignKeyConstraint
     * object.
     */
    virtual void copyInto(UMLForeignKeyConstraint *rhs) const;

    /**
     * Make a clone of the UMLForeignKeyConstraint.
     */
    virtual UMLObject* clone() const;

    /**
     * Returns a string representation of the UMLForeignKeyConstraint.
     *
     * @param sig               If true will show the attribute type and
     *                  initial value.
     * @return  Returns a string representation of the UMLAttribute.
     */
    QString toString(Uml::Signature_Type sig = Uml::st_NoSig);

    /**
     * Set the Referenced Entity
     * @param ent The Entity to Reference
     */
    void setReferencedEntity(UMLEntity* ent);

    UMLEntity* getReferencedEntity(){
        return m_ReferencedEntity;    
    }

    /**
     * Creates the <UML:ForeignKeyConstraint> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Display the properties configuration dialog for the attribute.
     */
    virtual bool showPropertiesDialog(QWidget* parent);


    /**
     * Adds the attribute pair to the attributeMap
     * @param pAttr The Attribute of the Parent Entity
     * @param rAttr The Attribute of the Referenced Entity
     * @return true if the attribute pair could be added successfully
     */
    bool addEntityAttributePair(UMLEntityAttribute* pAttr, UMLEntityAttribute* rAttr);

    /**
     * Removes an Attribute pair
     * @param pAttr The Attribute of the Parent Entity in the map. This attribute is the 
                    key of the map
     * @return true of the attribute pair could be removed successfully
     */
    bool removeEntityAttributePair(UMLEntityAttribute* /*key*/ pAttr);

    /**
     * Check if a attribute pair already exists
     * @param pAttr The Attribute of the Parent Entity
     * @param rAttr The Attribute of the Referenced Entity
     * @return true if the attribute pair could be found. 
     */
    bool hasEntityAttributePair(UMLEntityAttribute* pAttr,UMLEntityAttribute* rAttr);

    /**
     * Retrieve all Pairs of Attributes
     */
    QMap<UMLEntityAttribute*,UMLEntityAttribute*>  getEntityAttributePairs(){
	return m_AttributeMap;  
    }

    /**
     * Get the Delete Action 
     */
    UpdateDeleteAction getDeleteAction(){
        return m_DeleteAction;      
    }

    /**
     * Get the Update Action 
     */
    UpdateDeleteAction getUpdateAction(){
        return m_UpdateAction;       
    }

    /**
     * Set the Delete Action to the specified UpdateDeleteAction
     */
    void setDeleteAction(UpdateDeleteAction uda){
        m_DeleteAction = uda;
    }

    /**
     * Set the Update Action to the specified UpdateDeleteAction
     */
    void setUpdateAction(UpdateDeleteAction uda){
	m_UpdateAction = uda;
    }

    /** 
     * Clears all mappings between local and referenced attributes
     */
    void clearMappings();

    /**
     * Remimplementation from base classes
     * Used to resolve forward references to referenced entities in xmi
     */
    bool resolveRef();

signals:

    void sigReferencedEntityChanged();

private slots:
    void slotReferencedEntityChanged();   

protected:
    /**
     * Loads the <UML:ForeignKeyConstraint> XMI element.
     */
    bool load( QDomElement & element );

private:

    /** 
     * Used to resolve forward references to UMLEntity
     */
    Uml::IDType m_pReferencedEntityID;

    /**
     * Used to resolve forward references to UMLEntityAttributes
     * Key -> The local attribute
     * Value -> Id of the attribute it is mapping to
     */
    QMap<UMLEntityAttribute*, Uml::IDType> m_pEntityAttributeIDMap;


    /**
     * Initialisation of common variables
     */
    void init();

    /**
     * The UMLEntity that this foreign key references
     */
    UMLEntity* m_ReferencedEntity;

    /**
     * Stores the Mapping of attributes between parent table and referenced table
     */
   
    QMap<UMLEntityAttribute*, UMLEntityAttribute*> m_AttributeMap;

    /**
     * What to do on Update of referenced attributes 
     */
    UpdateDeleteAction m_UpdateAction;

    
    /**
     * What to do on Deletion of referenced attributes 
     */
    UpdateDeleteAction m_DeleteAction;
};

#endif
