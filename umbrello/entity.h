/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

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
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLEntity : public UMLClassifier
{
    Q_OBJECT
public:
    explicit UMLEntity(const QString& name = QString(), Uml::IDType id = Uml::id_None);
    virtual ~UMLEntity();

    bool operator==(const UMLEntity& rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    virtual UMLAttribute* createAttribute(const QString &name = QString(),
                                  UMLObject *type = 0, Uml::Visibility vis = Uml::Visibility::Private,
                                  const QString &init = QString());

    UMLUniqueConstraint* createUniqueConstraint(const QString &name = QString());

    UMLForeignKeyConstraint* createForeignKeyConstraint(const QString &name = QString());

    UMLCheckConstraint* createCheckConstraint(const QString &name = QString());

    UMLObject* addEntityAttribute(const QString &name, Uml::IDType id = Uml::id_None);
    bool addEntityAttribute(UMLEntityAttribute* att, IDChangeLog* log = 0);
    bool addEntityAttribute(UMLEntityAttribute* att, int position );

    int removeEntityAttribute(UMLClassifierListItem* att);

    void signalEntityAttributeRemoved(UMLClassifierListItem *eattr);

    int entityAttributes() ;

    bool setAsPrimaryKey(UMLUniqueConstraint* uconstr);

    void unsetPrimaryKey();

    bool hasPrimaryKey() const;

    bool isPrimaryKey(UMLUniqueConstraint* uConstr) const;

    bool addConstraint(UMLEntityConstraint* constr);

    bool removeConstraint(UMLEntityConstraint* constr);

    virtual bool resolveRef();

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    UMLClassifierListItemList getFilteredList(UMLObject::ObjectType ot) const;

    UMLEntityAttributeList getEntityAttributes() const;

    virtual UMLClassifierListItem* makeChildObject(const QString& xmiTag);

private slots:
    void slotEntityAttributeRemoved(UMLClassifierListItem*);

signals:
    void entityAttributeAdded(UMLClassifierListItem*);
    void entityAttributeRemoved(UMLClassifierListItem*);
    void entityConstraintAdded(UMLClassifierListItem*);
    void entityConstraintRemoved(UMLClassifierListItem*);

protected:
    bool load(QDomElement& element);

private:

    /**
     * Primary Key of this Entity
     * This is a pointer kept for easy access to the primary key, and to distinguish it
     * from all other UniqueConstraints. It is also there in m_List ( inherited from
     * UMLCanvasObject )
     */
    UMLUniqueConstraint* m_PrimaryKey;

};

#endif // ENTITY_H

