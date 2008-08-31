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
    UMLForeignKeyConstraint(UMLObject *parent, const QString& name,
                            Uml::IDType id = Uml::id_None );

    /**
     * Sets up a constraint.
     *
     * @param parent    The parent of this UMLForeignKeyConstraint.
     */
    UMLForeignKeyConstraint(UMLObject *parent);

    /**
     * Overloaded '==' operator
     */
    bool operator==( const UMLForeignKeyConstraint &rhs);

    /**
     * destructor.
     */
    virtual ~UMLForeignKeyConstraint();

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    QString toString(Uml::Signature_Type sig = Uml::st_NoSig);

    void setReferencedEntity(UMLEntity* ent);

    UMLEntity* getReferencedEntity() const {
        return m_ReferencedEntity;
    }

    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    virtual bool showPropertiesDialog(QWidget* parent);


    bool addEntityAttributePair(UMLEntityAttribute* pAttr, UMLEntityAttribute* rAttr);

    bool removeEntityAttributePair(UMLEntityAttribute* /*key*/ pAttr);

    bool hasEntityAttributePair(UMLEntityAttribute* pAttr,UMLEntityAttribute* rAttr) const;

    /**
     * Retrieve all Pairs of Attributes
     */
    QMap<UMLEntityAttribute*,UMLEntityAttribute*> getEntityAttributePairs() {
         return m_AttributeMap;
    }

    UpdateDeleteAction getDeleteAction() const {
        return m_DeleteAction;
    }

    UpdateDeleteAction getUpdateAction() const {
        return m_UpdateAction;
    }

    void setDeleteAction(UpdateDeleteAction uda) {
        m_DeleteAction = uda;
    }

    void setUpdateAction(UpdateDeleteAction uda) {
        m_UpdateAction = uda;
    }

    void clearMappings();

    bool resolveRef();

signals:

    void sigReferencedEntityChanged();

private slots:
    void slotReferencedEntityChanged();

protected:

    bool load( QDomElement & element );

private:

    /**
     * Used to resolve forward references to UMLEntity
     */
    Uml::IDType m_pReferencedEntityID;

    /**
     * Used to resolve forward references to UMLEntityAttributes
     * Key->The local attribute
     * Value->Id of the attribute it is mapping to
     */
    QMap<UMLEntityAttribute*, Uml::IDType> m_pEntityAttributeIDMap;


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
