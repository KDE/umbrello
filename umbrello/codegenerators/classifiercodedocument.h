/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CLASSIFIERCODEDOCUMENT_H
#define CLASSIFIERCODEDOCUMENT_H

#include "classifier.h"
#include "codeaccessormethod.h"
#include "codedocument.h"
#include "codeoperation.h"
#include "codeclassfield.h"
#include "codeclassfieldlist.h"
#include "umlassociationlist.h"

#include <QMap>
#include <QList>
#include <QString>


/**
  * class ClassifierCodeDocument
  * A CodeDocument which represents a UMLClassifier (e.g. a Class or Interface)
  */
class ClassifierCodeDocument : public CodeDocument
{
    friend class HierarchicalCodeBlock;
    Q_OBJECT
public:

    // Constructors/Destructors

    explicit ClassifierCodeDocument(UMLClassifier * parent);
    virtual ~ClassifierCodeDocument();

    bool addCodeClassField(CodeClassField * add_object);

    bool removeCodeClassField(CodeClassField * remove_object);

    CodeClassFieldList* getCodeClassFieldList();

    // some Utility methods

    bool parentIsInterface();
    bool parentIsClass();

    bool hasAssociationClassFields();
    bool hasAttributeClassFields();
    bool hasObjectVectorClassFields();
    bool hasClassFields();

    QList<CodeOperation*> getCodeOperations();

    CodeClassFieldList getSpecificClassFields(CodeClassField::ClassFieldType cfType);
    CodeClassFieldList getSpecificClassFields(CodeClassField::ClassFieldType cfType, bool isStatic);
    CodeClassFieldList getSpecificClassFields(CodeClassField::ClassFieldType cfType, Uml::Visibility::Enum visibility);
    CodeClassFieldList getSpecificClassFields(CodeClassField::ClassFieldType cfType, bool isStatic, Uml::Visibility::Enum visibility);

    CodeClassField * findCodeClassFieldFromParentID (Uml::ID::Type id, int role_id = -1);

    UMLClassifier * getParentClassifier();

    void addCodeClassFieldMethods(CodeClassFieldList &list);

    virtual void initCodeClassFields();

    virtual void synchronize();

    /** Will add the code operation in the correct place in the document.
     *  @return bool which is true IF the code operation was added successfully
     */
    virtual bool addCodeOperation(CodeOperation *opBlock) = 0;

    virtual void updateContent() = 0;

    virtual void saveToXMI1(QXmlStreamWriter& writer);
    virtual void loadFromXMI1(QDomElement &root);

protected:

    void loadClassFieldsFromXMI(QDomElement &childElem);

    virtual void setAttributesOnNode(QXmlStreamWriter& writer);
    virtual void setAttributesFromNode(QDomElement &element);

    TextBlock * findCodeClassFieldTextBlockByTag(const QString &tag);

    void declareClassFields(CodeClassFieldList &list, CodeGenObjectWithTextBlocks *parent);

    void syncClassFields();
    void syncNamesToParent();

private:

    CodeClassFieldList m_classfieldVector;
    UMLClassifier* m_parentclassifier;

    void updateAssociationClassFields(UMLAssociationList &assocList);
    void updateOperations();

    /**
     * Maps CodeClassFields to UMLObjects. Used to prevent re-adding a class
     * field.
     */
    QMap<UMLObject *, CodeClassField *> m_classFieldMap;

    void init(UMLClassifier *classifier);

public slots:

    void addAttributeClassField(UMLClassifierListItem *at, bool syncToParentIfAdded = true);
    void addAssociationClassField(UMLAssociation *assoc, bool syncToParentIfAdded = true);
    void removeAttributeClassField(UMLClassifierListItem *at);
    void removeAssociationClassField(UMLAssociation *assoc);
    void addOperation(UMLClassifierListItem *obj);
    void removeOperation(UMLClassifierListItem *obj);
    void syncToParent();

};

#endif // CLASSIFIERCODEDOCUMENT_H
