/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef FOREIGNKEYCONSTRAINT_H
#define FOREIGNKEYCONSTRAINT_H

// appl includes
#include "basictypes.h"
#include "entityconstraint.h"

// qt includes
#include <QtCore/QMap>

//forward declarations
class UMLEntityAttribute;
class UMLEntity;

/**
 * This class is used to set up information for a foreign key entity constraint.
 * @short Sets up Foreign Key  entity constraint information.
 * @author Sharan Rao
 * @see UMLObject UMLClassifierListItem UMLEntityConstraint
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLForeignKeyConstraint : public UMLEntityConstraint
{
     Q_OBJECT

public:

    /**
     * Update/Delete Action: Action to be taken on Update or Delete of a referenced attribute
     * is either, No Action, Restrict, Cascade, Set NULL, Set Default.
     */
    enum UpdateDeleteAction { uda_NoAction = 0,
                              uda_Restrict,
                              uda_Cascade,
                              uda_SetNull,
                              uda_SetDefault };

    UMLForeignKeyConstraint(UMLObject *parent, const QString& name,
                            Uml::IDType id = Uml::id_None );
    UMLForeignKeyConstraint(UMLObject *parent);

    bool operator==( const UMLForeignKeyConstraint &rhs) const;

    virtual ~UMLForeignKeyConstraint();

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    QString toString(Uml::SignatureType sig = Uml::SignatureType::NoSig);

    void setReferencedEntity(UMLEntity* ent);
    UMLEntity* getReferencedEntity() const;

    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    virtual bool showPropertiesDialog(QWidget* parent);

    bool addEntityAttributePair(UMLEntityAttribute* pAttr, UMLEntityAttribute* rAttr);

    bool removeEntityAttributePair(UMLEntityAttribute* /*key*/ pAttr);

    bool hasEntityAttributePair(UMLEntityAttribute* pAttr,UMLEntityAttribute* rAttr) const;

    QMap<UMLEntityAttribute*,UMLEntityAttribute*> getEntityAttributePairs();

    void setDeleteAction(UpdateDeleteAction uda);
    UpdateDeleteAction getDeleteAction() const;

    void setUpdateAction(UpdateDeleteAction uda);
    UpdateDeleteAction getUpdateAction() const;

    void clearMappings();

    bool resolveRef();

signals:
    void sigReferencedEntityChanged();

private slots:
    void slotReferencedEntityChanged();

protected:
    bool load( QDomElement & element );

private:

    Uml::IDType m_pReferencedEntityID;  ///< Used to resolve forward references to UMLEntity.

    /**
     * Used to resolve forward references to UMLEntityAttributes
     * Key -> The local attribute
     * Value -> Id of the attribute it is mapping to
     */
    QMap<UMLEntityAttribute*, Uml::IDType> m_pEntityAttributeIDMap;

    void init();

    UMLEntity* m_ReferencedEntity;  ///< The UMLEntity that this foreign key references.

    /**
     * Stores the Mapping of attributes between parent table and referenced table
     */
    QMap<UMLEntityAttribute*, UMLEntityAttribute*> m_AttributeMap;

    UpdateDeleteAction m_UpdateAction;  ///< What to do on Update of referenced attributes.
    UpdateDeleteAction m_DeleteAction;  ///< What to do on Deletion of referenced attributes.
};

#endif
