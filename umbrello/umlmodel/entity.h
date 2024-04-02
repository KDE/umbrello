/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ENTITY_H
#define ENTITY_H

#include "classifier.h"

// forward declarations
class UMLEntityAttribute;
class UMLEntityConstraint;
class UMLUniqueConstraint;
class UMLForeignKeyConstraint;
class UMLCheckConstraint;
class UMLEntityAttributeList;

/**
 * This class contains the non-graphical information required for a UML
 * Entity.
 * This class inherits from @ref UMLClassifier which contains most of the
 * information.
 *
 * @short Non-graphical Information for an Entity.
 * @author Jonathan Riddell
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLEntity : public UMLClassifier
{
    Q_OBJECT
public:
    explicit UMLEntity(const QString& name = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~UMLEntity();

    bool operator==(const UMLEntity& rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    virtual UMLAttribute* createAttribute(const QString &name = QString(),
                                  UMLObject *type = nullptr, Uml::Visibility::Enum vis = Uml::Visibility::Private,
                                  const QString &init = QString());

    UMLUniqueConstraint* createUniqueConstraint(const QString &name = QString());
    UMLForeignKeyConstraint* createForeignKeyConstraint(const QString &name = QString());
    UMLCheckConstraint* createCheckConstraint(const QString &name = QString());

    UMLObject* addEntityAttribute(const QString &name, Uml::ID::Type id = Uml::ID::None);
    bool addEntityAttribute(UMLEntityAttribute* att, IDChangeLog* log = nullptr);
    bool addEntityAttribute(UMLEntityAttribute* att, int position);

    int removeEntityAttribute(UMLClassifierListItem* att);

    void signalEntityAttributeRemoved(UMLClassifierListItem *eattr);

    int entityAttributes() const;

    bool setAsPrimaryKey(UMLUniqueConstraint* uconstr);
    void unsetPrimaryKey();
    bool hasPrimaryKey() const;
    bool isPrimaryKey(const UMLUniqueConstraint* uConstr) const;

    bool addConstraint(UMLEntityConstraint* constr);
    bool removeConstraint(UMLEntityConstraint* constr);

    virtual bool resolveRef();

    virtual void saveToXMI(QXmlStreamWriter& writer);

    UMLClassifierListItemList getFilteredList(UMLObject::ObjectType ot) const;

    UMLEntityAttributeList getEntityAttributes() const;

    virtual UMLClassifierListItem* makeChildObject(const QString& xmiTag);

    Q_SIGNAL void entityAttributeAdded(UMLClassifierListItem*);
    Q_SIGNAL void entityAttributeRemoved(UMLClassifierListItem*);
    Q_SIGNAL void entityConstraintAdded(UMLClassifierListItem*);
    Q_SIGNAL void entityConstraintRemoved(UMLClassifierListItem*);

private:
    Q_SLOT void slotEntityAttributeRemoved(UMLClassifierListItem*);


protected:
    bool load1(QDomElement& element);

private:

    /**
     * Primary Key of this Entity
     * This is a pointer kept for easy access to the primary key, and to distinguish it
     * from all other UniqueConstraints. It is also there in subordinates (inherited from
     * UMLCanvasObject)
     */
    UMLUniqueConstraint* m_PrimaryKey;

};

#endif // ENTITY_H

