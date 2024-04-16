/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "package.h"
#include "umlattributelist.h"
#include "umloperationlist.h"
#include "umlclassifierlistitemlist.h"
#include "classifierlistitem.h"
#include "umltemplatelist.h"
#include "model_utils.h"

// forward declarations
class UMLAssociation;
class IDChangeLog;
class UMLClassifierSet;

/**
 * This class defines the non-graphical information required for a
 * UML Classifier (ie a class or interface).
 * This class inherits from @ref UMLPackage which allows classifiers
 * to also act as namespaces, i.e. it allows classifiers to nest.
 *
 * NOTE: There is a unit test available for this class.
 *       Please, use and adapt it when necessary.
 *
 * @short Information for a non-graphical Concept/Class.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLClassifier : public UMLPackage
{
    Q_OBJECT
public:

    /**
     * Enumeration identifying the type of classifier.
     */
    enum ClassifierType { ALL = 0, CLASS, INTERFACE, DATATYPE };

    explicit UMLClassifier(const QString & name = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~UMLClassifier();

    virtual void copyInto(UMLObject *lhs) const;

    UMLObject* clone() const;

    virtual void setNameCmd(const QString &strName);

    virtual UMLAttribute* createAttribute(const QString &name = QString(),
                                          UMLObject  *type = nullptr,
                                          Uml::Visibility::Enum vis = Uml::Visibility::Private,
                                          const QString &init = QString());

    UMLAttribute* addAttribute(const QString &name, Uml::ID::Type id = Uml::ID::None);
    UMLAttribute* addAttribute(const QString &name, UMLObject *type, Uml::Visibility::Enum scope);

    bool addAttribute(UMLAttribute *att, IDChangeLog* log = nullptr,
                      int position = -1);

    int removeAttribute(UMLAttribute *att);

    UMLAttributeList getAttributeList() const;
    UMLAttributeList getAttributeList(Uml::Visibility::Enum scope) const;
    UMLAttributeList getAttributeListStatic(Uml::Visibility::Enum scope) const;

    UMLOperation* createOperation(const QString &name = QString(),
                                  bool  *isExistingOp = nullptr,
                                  Model_Utils::NameAndType_List  *params = nullptr);

    bool addOperation(UMLOperation* op, int position = -1);
    bool addOperation(UMLOperation* op, IDChangeLog* log);

    UMLOperation * checkOperationSignature(const QString& name,
                                           UMLAttributeList opParams,
                                           UMLOperation  *exemptOp = nullptr) const;

    int removeOperation(UMLOperation *op);

    int operations() const;

    UMLOperationList getOpList(bool includeInherited = false, UMLClassifierSet  *alreadyTraversed = nullptr) const;

    UMLObject* createTemplate(const QString& name = QString());

    UMLTemplate* addTemplate(const QString &name, Uml::ID::Type id = Uml::ID::None);

    bool addTemplate(UMLTemplate *newTemplate, IDChangeLog* log = nullptr);
    bool addTemplate(UMLTemplate* templt, int position);

    int removeTemplate(UMLTemplate* umltemplate);

    UMLTemplate *findTemplate(const QString& name) const;

    int templates() const;

    UMLTemplateList getTemplateList() const;

    int takeItem(UMLClassifierListItem* item);

    virtual UMLClassifierListItemList getFilteredList(UMLObject::ObjectType ot) const;

    virtual bool resolveRef();

    UMLOperationList findOperations(const QString &n) const;

    virtual UMLObject* findChildObjectById(Uml::ID::Type id,
                                           bool considerAncestors = false) const;

    UMLOperation* findOperation(const QString& name,
                                Model_Utils::NameAndType_List params) const;

    UMLClassifierList findSuperClassConcepts(ClassifierType type = ALL) const;

    UMLClassifierList findSubClassConcepts(ClassifierType type = ALL) const;

    virtual bool acceptAssociationType(Uml::AssociationType::Enum type) const;

    void setBaseType(UMLObject::ObjectType ot);

    bool isInterface() const;

    bool hasAbstractOps()      const;
    bool hasAssociations()     const;
    bool hasAttributes()       const;
    bool hasStaticAttributes() const;
    bool hasMethods()          const;
    bool hasAccessorMethods()  const;
    bool hasOperationMethods() const;
    bool hasVectorFields()     const;

    /**
     * utility functions to allow easy determination of what classifiers
     * are "owned" by the current one via named association type (e.g.
     * plain, aggregate or compositions).
     */
//    UMLClassifierList getPlainAssocChildClassifierList();
//    UMLClassifierList getAggregateChildClassifierList();
//    UMLClassifierList getCompositionChildClassifierList();

    virtual UMLClassifierListItem* makeChildObject(const QString& xmiTag);

    void removeObject(UMLObject *pObject);

    virtual UMLAssociationList  getUniAssociationToBeImplemented() const;

Q_SIGNALS:

    void operationAdded(UMLClassifierListItem *);
    void operationRemoved(UMLClassifierListItem *);

    void templateAdded(UMLClassifierListItem*);
    void templateRemoved(UMLClassifierListItem*);

    // only applies when (m_Type == ot_Class)
    void attributeAdded(UMLClassifierListItem*);
    void attributeRemoved(UMLClassifierListItem*);

private:

    UMLAssociation *m_pClassAssoc;

    /**
     * Utility method called by "get*ChildClassfierList()" methods. It basically
     * finds all the classifiers named in each association in the given association list
     * which aren't the current one. Useful for finding which classifiers are "owned" by the
     * current one via declared associations such as in aggregations/compositions.
     */
//    UMLClassifierList findAssocClassifierObjsInRoles (UMLAssociationList * list);

protected:

    void saveToXMI(QXmlStreamWriter& writer);

    virtual bool load1(QDomElement& element);

};

#endif // CLASSIFIER_H
