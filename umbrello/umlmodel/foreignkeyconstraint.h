/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef FOREIGNKEYCONSTRAINT_H
#define FOREIGNKEYCONSTRAINT_H

// appl includes
#include "basictypes.h"
#include "entityconstraint.h"

// qt includes
#include <QMap>

//forward declarations
class UMLEntityAttribute;
class UMLEntity;

/**
 * This class is used to set up information for a foreign key entity constraint.
 * @short Sets up Foreign Key  entity constraint information.
 * @author Sharan Rao
 * @see UMLObject UMLClassifierListItem UMLEntityConstraint
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLForeignKeyConstraint : public UMLEntityConstraint
{
     Q_OBJECT

public:

    /**
     * Update/Delete Action: Action to be taken on Update or Delete of a referenced attribute
     * is either, No Action, Restrict, Cascade, Set nullptr, Set Default.
     */
    enum UpdateDeleteAction { uda_NoAction = 0,
                              uda_Restrict,
                              uda_Cascade,
                              uda_SetNull,
                              uda_SetDefault };

    UMLForeignKeyConstraint(UMLObject *parent, const QString& name,
                            Uml::ID::Type id = Uml::ID::None);
    explicit UMLForeignKeyConstraint(UMLObject *parent);

    bool operator==(const UMLForeignKeyConstraint &rhs) const;

    virtual ~UMLForeignKeyConstraint();

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    QString toString(Uml::SignatureType::Enum sig = Uml::SignatureType::NoSig,
                     bool withStereotype=false) const;

    void setReferencedEntity(UMLEntity* ent);
    UMLEntity* getReferencedEntity() const;

    void saveToXMI(QXmlStreamWriter& writer);

    virtual bool showPropertiesDialog(QWidget* parent = nullptr);

    bool addEntityAttributePair(UMLEntityAttribute* pAttr, UMLEntityAttribute* rAttr);

    bool removeEntityAttributePair(UMLEntityAttribute* /*key*/ pAttr);

    bool hasEntityAttributePair(UMLEntityAttribute* pAttr, UMLEntityAttribute* rAttr) const;

    QMap<UMLEntityAttribute*, UMLEntityAttribute*> getEntityAttributePairs();

    void setDeleteAction(UpdateDeleteAction uda);
    UpdateDeleteAction getDeleteAction() const;

    void setUpdateAction(UpdateDeleteAction uda);
    UpdateDeleteAction getUpdateAction() const;

    void clearMappings();

    bool resolveRef();

    Q_SIGNAL void sigReferencedEntityChanged();

private:
    Q_SLOT void slotReferencedEntityChanged();

protected:
    bool load1(QDomElement & element);

private:

    Uml::ID::Type m_pReferencedEntityID;  ///< Used to resolve forward references to UMLEntity.

    /**
     * Used to resolve forward references to UMLEntityAttributes
     * Key -> The local attribute
     * Value -> Id of the attribute it is mapping to
     */
    QMap<UMLEntityAttribute*, Uml::ID::Type> m_pEntityAttributeIDMap;

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
