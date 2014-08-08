/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "petaltree2uml.h"

// app includes
#include "debug_utils.h"
#include "petalnode.h"
#include "model_utils.h"
#include "object_factory.h"
#include "import_rose.h"
#include "uniqueid.h"
#include "package.h"
#include "folder.h"
#include "classifier.h"
#include "attribute.h"
#include "operation.h"
#include "association.h"
#include "umlrole.h"
#include "actor.h"
#include "usecase.h"
#include "component.h"
#include "node.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "widget_factory.h"
#include "notewidget.h"
#include "associationwidget.h"
#include "floatingtextwidget.h"

// qt includes
#include <QtGlobal>
#include <QFile>
#include <QRegExp>

namespace Import_Rose {

/**
 * The Rose diagram coordinate system is roughly twice the scale of Qt.
 * I.e. when going from Rose diagram-object location/width/height to Qt,
 * we need to shrink everything.
 * The exact factor can be configured here.
 */
const qreal Rose2Qt = 0.5;

/**
 * Return the given string without surrounding quotation marks.
 * Also remove a possible prefix "Logical View::", it is not modeled in Umbrello.
 */
QString clean(const QString& s)
{
    if (s.isNull())
        return QString();
    QString str = s;
    str.remove(QLatin1Char('\"'));
    str.remove(QRegExp(QLatin1String("^.+::")));
    return str;
}

/**
 * Extract the quid attribute from a petal node and return it as a Uml::ID::Type.
 */
Uml::ID::Type quid(const PetalNode *node)
{
    QString quidStr = node->findAttribute(QLatin1String("quid")).string;
    if (quidStr.isEmpty())
        return Uml::ID::None;
    quidStr.remove(QLatin1Char('\"'));
    return Uml::ID::fromString(quidStr);
}

/**
 * Extract the quidu attribute from a petal node.
 */
QString quidu(const PetalNode *node)
{
    QString quiduStr = node->findAttribute(QLatin1String("quidu")).string;
    if (quiduStr.isEmpty())
        return QString();
    quiduStr.remove(QLatin1Char('\"'));
    return quiduStr;
}

/**
 * Extract the location attribute from a petal node.
 */
QPointF fetchLocation(const PetalNode *node, qreal width, qreal height)
{
    QString location = node->findAttribute(QLatin1String("location")).string;
    QStringList a = location.split(QLatin1Char(' '));
    if (a.size() != 2) {
        return QPointF();
    }
    bool ok;
    qreal x = a[0].toDouble(&ok);
    if (!ok) {
        return QPointF();
    }
    qreal y = a[1].toDouble(&ok);
    if (!ok) {
        return QPointF();
    }
    x *= Rose2Qt;  // adjust scale to Qt
    y *= Rose2Qt;  // adjust scale to Qt
    // Rose diagram locations denote the object _center_ thus:
    // - for X we must subtract width/2
    // - for Y we must subtract height/2
    return QPointF(x - width / 2.0, y - height / 2.0);
}

/**
 * Extract a double attribute from a petal node.
 */
qreal fetchDouble(const PetalNode *node, const QString &attribute, qreal defaultValue = 0)
{
    bool ok;
    QString s = node->findAttribute(attribute).string;
    qreal value = s.toDouble(&ok);
    return ok ? value * Rose2Qt : defaultValue;
}

/**
 * Extract an int attribute from a petal node.
 */
qreal fetchInt(const PetalNode *node, const QString &attribute, int defaultValue = 0)
{
    bool ok;
    QString s = node->findAttribute(attribute).string;
    qreal value = s.toInt(&ok);
    return ok ? value : defaultValue;
}

/**
 * Determine the model type corresponding to a name.
 * If the given name consists only of letters, digits, underscores, and
 * scope separators then return ot_Class, else return ot_Datatype.
 */
UMLObject::ObjectType typeToCreate(const QString& name)
{
    QString n = name;
    n.remove(QRegExp(QLatin1String("^.*::")));  // don't consider the scope prefix, it may contain spaces
    UMLObject::ObjectType t = (n.contains(QRegExp(QLatin1String("\\W"))) ? UMLObject::ot_Datatype
                                                                         : UMLObject::ot_Class);
    return t;
}

/**
 * Transfer the Rose attribute "exportControl" to the Umbrello object given.
 *
 * @param from   Pointer to PetalNode from which to read the "exportControl" attribute
 * @param to     Pointer to UMLObject in which to set the Uml::Visibility
 */
void transferVisibility(const PetalNode *from, UMLObject *to)
{
    QString vis = from->findAttribute(QLatin1String("exportControl")).string;
    if (!vis.isEmpty()) {
        Uml::Visibility::Enum v = Uml::Visibility::fromString(clean(vis.toLower()));
        to->setVisibilityCmd(v);
    }
}

/**
 * ClassifierListReader factors the common processing for attributes, operations,
 * and operation parameters.
 */
class ClassifierListReader
{
public:
    /// constructor
    ClassifierListReader(const char* attributeTag,
                         const char* elementName,
                         const char* itemTypeDesignator) :
        m_attributeTag(QLatin1String(attributeTag)),
        m_elementName(QLatin1String(elementName)),
        m_itemTypeDesignator(QLatin1String(itemTypeDesignator)) {
    }
    /// destructor
    virtual ~ClassifierListReader() {}

    /**
     * Return a UMLClassifierListItem of the specific type desired.
     * Abstract method to be implemented by inheriting classes.
     */
    virtual UMLObject *createListItem() = 0;

    virtual void setTypeReferences(UMLObject *item,
                                   const QString& quid, const QString& type) {
        if (!quid.isEmpty()) {
            item->setSecondaryId(quid);
        }
        if (!type.isEmpty()) {
            item->setSecondaryFallback(type);
        }
    }

    /**
     * Insert the given UMLClassifierListItem at the parent Umbrello object.
     * Abstract method to be implemented by inheriting classes.
     * NB the parent Umbrello object is not included in the ClassifierListReader
     * class - it must be added at inheriting classes.
     *
     * @param node   The PetalNode which corresponds to the parent Umbrello object.
     * @param o      The UMLObject to insert.
     */
    virtual void insertAtParent(const PetalNode *node, UMLObject *o) = 0;

    /**
     * Iterate over the attributes of the given PetalNode and for each recognized
     * attribute do the following:
     *   - invoke createListItem()
     *   - fill common properties such as name, unique ID, visibility, etc. into
     *     the new UMLClassifierListItem
     *   - invoke insertAtParent() with the new classifier list item as the argument
     * This is the user entry point.
     */
    void read(const PetalNode *node, const QString& name)
    {
        PetalNode *attributes = node->findAttribute(m_attributeTag).node;
        if (attributes == NULL) {
#ifdef VERBOSE_DEBUGGING
            uDebug() << name << ": no " << m_attributeTag << " found";
#endif
            return;
        }
        PetalNode::NameValueList attributeList = attributes->attributes();
        for (int i = 0; i < attributeList.count(); ++i) {
            PetalNode *attNode = attributeList[i].second.node;
            QStringList initialArgs = attNode->initialArgs();
            if (attNode->name() != m_elementName) {
                uDebug() << name << ": expecting " << m_elementName
                         << ", " << "found " << initialArgs[0];
                continue;
            }
            UMLObject *item = createListItem();
            if (initialArgs.count() > 1)
                item->setName(clean(initialArgs[1]));
            item->setID(quid(attNode));
            QString quidref = quidu(attNode);
            QString type = clean(attNode->findAttribute(m_itemTypeDesignator).string);
            setTypeReferences(item, quidref, type);
            transferVisibility(attNode, item);
            item->setDoc(attNode->documentation());
            insertAtParent(attNode, item);
        }
    }
protected:
    const QString m_attributeTag, m_elementName, m_itemTypeDesignator;
};

class AttributesReader : public ClassifierListReader
{
public:
    AttributesReader(UMLClassifier *c)
      : ClassifierListReader("class_attributes", "ClassAttribute", "type") {
        m_classifier = c;
    }
    virtual ~AttributesReader() {}
    UMLObject *createListItem() {
        return new UMLAttribute(m_classifier);
    }
    void insertAtParent(const PetalNode *, UMLObject *item) {
        m_classifier->addAttribute(static_cast<UMLAttribute*>(item));
    }
protected:
    UMLClassifier *m_classifier;
};

class ParametersReader : public ClassifierListReader
{
public:
    ParametersReader(UMLOperation *op)
      : ClassifierListReader("parameters", "Parameter", "type") {
        m_operation = op;
    }
    virtual ~ParametersReader() {}
    UMLObject *createListItem() {
        return new UMLAttribute(m_operation);
    }
    void insertAtParent(const PetalNode *, UMLObject *item) {
        if (item->id() == Uml::ID::None)
           item->setID(UniqueID::gen());
        m_operation->addParm(static_cast<UMLAttribute*>(item));
    }
protected:
    UMLOperation *m_operation;
};

class OperationsReader : public ClassifierListReader
{
public:
    OperationsReader(UMLClassifier *c)
      : ClassifierListReader("operations", "Operation", "result") {
        m_classifier = c;
    }
    virtual ~OperationsReader() {}
    UMLObject *createListItem() {
        return new UMLOperation(m_classifier);
    }
    void insertAtParent(const PetalNode *node, UMLObject *item) {
        UMLOperation *op = static_cast<UMLOperation*>(item);
        ParametersReader parmReader(op);
        parmReader.read(node, m_classifier->name());
        m_classifier->addOperation(op);
    }
protected:
    UMLClassifier *m_classifier;
};

class SuperclassesReader : public ClassifierListReader
{
public:
    SuperclassesReader(UMLClassifier *c)
      : ClassifierListReader("superclasses", "Inheritance_Relationship", "supplier") {
        m_classifier = c;
    }
    virtual ~SuperclassesReader() {}
    UMLObject *createListItem() {
        return new UMLAssociation(Uml::AssociationType::Generalization);
    }
    /**
     * Override parent implementation: The secondary data is not for the
     * UMLAssociation itself but for its role B object.
     */
    void setTypeReferences(UMLObject *item,
                           const QString& quid, const QString& type) {
        UMLAssociation *assoc = static_cast<UMLAssociation*>(item);
        if (!quid.isEmpty()) {
            assoc->getUMLRole(Uml::RoleType::B)->setSecondaryId(quid);
        }
        if (!type.isEmpty()) {
            assoc->getUMLRole(Uml::RoleType::B)->setSecondaryFallback(type);
        }
    }
    void insertAtParent(const PetalNode *, UMLObject *item) {
        UMLAssociation *assoc = static_cast<UMLAssociation*>(item);
        assoc->setObject(m_classifier, Uml::RoleType::A);
        assoc->setUMLPackage(m_classifier->umlPackage());
        UMLApp::app()->document()->addAssociation(assoc);
    }
protected:
    UMLClassifier *m_classifier;
};

class RealizationsReader : public ClassifierListReader
{
public:
    RealizationsReader(UMLClassifier *c)
      : ClassifierListReader("realized_interfaces", "Realize_Relationship", "supplier") {
        m_classifier = c;
    }
    virtual ~RealizationsReader() {}
    UMLObject *createListItem() {
        return new UMLAssociation(Uml::AssociationType::Realization);
    }
    /**
     * Override parent implementation: The secondary data is not for the
     * UMLAssociation itself but for its role B object.
     */
    void setTypeReferences(UMLObject *item,
                           const QString& quid, const QString& type) {
        UMLAssociation *assoc = static_cast<UMLAssociation*>(item);
        if (!quid.isEmpty()) {
            assoc->getUMLRole(Uml::RoleType::B)->setSecondaryId(quid);
        }
        if (!type.isEmpty()) {
            assoc->getUMLRole(Uml::RoleType::B)->setSecondaryFallback(type);
        }
    }
    void insertAtParent(const PetalNode *, UMLObject *item) {
        UMLAssociation *assoc = static_cast<UMLAssociation*>(item);
        assoc->setObject(m_classifier, Uml::RoleType::A);
        assoc->setUMLPackage(m_classifier->umlPackage());
        UMLApp::app()->document()->addAssociation(assoc);
    }
protected:
    UMLClassifier *m_classifier;
};

/**
 * Handle a controlled unit.
 *
 * @param node       Pointer to the PetalNode which may contain a controlled unit
 * @param name       Name of the current node
 * @param id         QUID of the current node
 * @param parentPkg  Pointer to the current parent UMLPackage or UMLFolder.
 * @return      Pointer to UMLFolder created for controlled unit on success;
 *              NULL on error.
 */
UMLPackage* handleControlledUnit(PetalNode *node, const QString& name,
                                  Uml::ID::Type id, UMLPackage * parentPkg)
{
    Q_UNUSED(id);
    if (node->findAttribute(QLatin1String("is_unit")).string != QLatin1String("TRUE"))
        return NULL;
    //bool is_loaded = (node->findAttribute(QLatin1String("is_loaded")).string != QLatin1String("FALSE"));
    QString file_name = node->findAttribute(QLatin1String("file_name")).string;
    if (file_name.isEmpty()) {
        uError() << name << ": attribute file_name not found (?)";
        return NULL;
    }
    file_name = file_name.mid(1, file_name.length() - 2);  // remove sourrounding ""
    /* I wanted to use
                  file_name.replace(QRegExp("\\\\+") "/");
       but this did not work using Qt 4.6.3. Workaround:
     */
    file_name.replace(QLatin1String("\\\\"), QLatin1String("/"));
    file_name.replace(QRegExp(QLatin1String("/+")), QLatin1String("/"));
    /* End of workaround */

    if (file_name.startsWith(QLatin1String("$"))) {
        const int firstSlash = file_name.indexOf(QLatin1Char('/'));
        QString envVarName;
        if (firstSlash < 0) {
            envVarName = file_name.mid(1);
        } else {
            envVarName = file_name.mid(1, firstSlash - 1);
        }
        QByteArray envVarBA = qgetenv(envVarName.toLatin1());
        if (envVarBA.isNull() || envVarBA.isEmpty()) {
            uError() << name << "cannot process file_name " << file_name
                     << " because environment variable " << envVarName << " not set";
            return NULL;
        }
        QString envVar(QString::fromLatin1(envVarBA));
        uDebug() << name << ": envVar " << envVarName << " contains " << envVar;
        if (envVar.endsWith(QLatin1Char('/')))
            envVar.chop(1);
        if (firstSlash < 0)
            file_name = envVar;
        else
            file_name = envVar + file_name.mid(firstSlash);
    }

    QFileInfo fi(file_name);
    if (!fi.isAbsolute()) {
        // Must have an absolute path by now.
        // If we don't then use the directory of the .mdl file.
        file_name = Import_Rose::mdlPath() + file_name;
    }
    QFile file(file_name);
    if (!file.exists()) {
        uError() << name << ": file_name " << file_name << " not found";
        return NULL;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        uError() << name << ": file_name " << file_name << " cannot be opened";
        return NULL;
    }
    UMLPackage *controlledUnit = loadFromMDL(file, parentPkg);
    file.close();
    return controlledUnit;
}

void handleAssocView(PetalNode *attr,
                     const PetalNode::NameValueList& parentAttrs,
                     Uml::AssociationType::Enum assocType,
                     UMLView *view,
                     UMLObject *umlAssoc = 0)
{
    QString assocStr = Uml::AssociationType::toString(assocType);
    PetalNode *roleview_list = attr->findAttribute(QLatin1String("roleview_list")).node;
    if (roleview_list == NULL) {
        uError() << assocStr << " roleview_list not found";
        return;
    }
    PetalNode::StringOrNode supElem, cliElem;
    if (roleview_list) {
        PetalNode::NameValueList roles = roleview_list->attributes();
        if (roles.length() < 2) {
            uError() << assocStr << " roleview_list should have 2 elements";
            return;
        }
        PetalNode *supNode = roles[0].second.node;
        PetalNode *cliNode = roles[1].second.node;
        if (supNode && cliNode) {
            supElem = supNode->findAttribute(QLatin1String("supplier"));
            cliElem = cliNode->findAttribute(QLatin1String("supplier"));  // not a typo, really "supplier"
        } else {
            uError() << assocStr << " roleview_list roles are incomplete";
            return;
        }
    } else {
        supElem = attr->findAttribute(QLatin1String("supplier"));
        cliElem = attr->findAttribute(QLatin1String("client"));
    }
    QString supplier = supElem.string;
    QString client   = cliElem.string;
    if (supplier.isEmpty() || client.isEmpty()) {
        if (supElem.node || cliElem.node) {
            uError() << assocStr << " unexpected supplier and/or client type"
                     << " (actual: NODE; expected: STRING)";
        } else {
            uError() << assocStr << " supplier and/or client non-existent or empty";
        }
        return;
    }
    PetalNode *sup = NULL, *cli = NULL;
    for (int c = 0; c < parentAttrs.count(); ++c) {
        PetalNode *n = parentAttrs[c].second.node;
        QStringList initArgs = n->initialArgs();
        QString tag = initArgs.last();
        if (tag == client)
            cli = n;
        else if (tag == supplier)
            sup = n;
    }
    if (sup && cli) {
        QString spIdStr = quidu(sup);
        Uml::ID::Type spId = Uml::ID::fromString(spIdStr);
        QString clIdStr = quidu(cli);
        Uml::ID::Type clId = Uml::ID::fromString(clIdStr);
        if (spId != Uml::ID::None && clId != Uml::ID::None) {
            UMLWidget *supW = view->umlScene()->widgetOnDiagram(spId);
            UMLWidget *cliW = view->umlScene()->widgetOnDiagram(clId);
            if (supW == NULL) {
                uError() << assocStr << " supplier widget " << spIdStr
                         << " is not on diagram (?)";
            } else if (cliW == NULL) {
                uError() << assocStr << " client widget " << clIdStr
                         << " is not on diagram (?)";
            } else {
                // UMLAssociation *a = static_cast<UMLAssociation*>(o);
                AssociationWidget *aw = AssociationWidget::create
                                         (view->umlScene(), cliW, assocType,
                                          supW, umlAssoc);
                view->umlScene()->addAssociation(aw);
            }
        } else {
            uError() << assocStr << " bad or nonexistent quidu at client "
                     << client << " (" << cli->name() << ")";
        }
    } else {
        uError() << assocStr << " could not find client with tag " << client;
    }
}

Uml::DiagramType::Enum diagramType(QString objType)
{
    Uml::DiagramType::Enum dt;
    dt = (objType == QLatin1String("ClassDiagram")    ? Uml::DiagramType::Class :
          objType == QLatin1String("UseCaseDiagram")  ? Uml::DiagramType::UseCase :
          objType == QLatin1String("Module_Diagram")  ? Uml::DiagramType::Component :
          objType == QLatin1String("Process_Diagram") ? Uml::DiagramType::Deployment :
          // not yet implemented: Sequence, Collaboration, State, Activity
                                         Uml::DiagramType::Undefined);
    return dt;
}

/**
 * Create an Umbrello object from a PetalNode.
 *
 * @return   True for success.
 *           Given a PetalNode for which the mapping to Umbrello is not yet
 *           implemented umbrellify() is a no-op but also returns true.
 */
bool umbrellify(PetalNode *node, UMLPackage *parentPkg)
{
    if (node == NULL) {
        uError() << "node is NULL";
        return false;
    }
    QStringList args = node->initialArgs();
    QString objType = args[0];
    QString name = clean(args[1]);
    Uml::ID::Type id = quid(node);
    Uml::DiagramType::Enum dt = Uml::DiagramType::Undefined;

    if (objType == QLatin1String("Class_Category") || objType == QLatin1String("SubSystem")) {
        const bool isSubsystem = (objType == QLatin1String("SubSystem"));
        QString modelsAttr(isSubsystem ? QLatin1String("physical_models")
                                       : QLatin1String("logical_models"));
        PetalNode *models = node->findAttribute(modelsAttr).node;
        UMLObject *o = NULL;
        if (models) {
            PetalNode::NameValueList atts = models->attributes();
            QString presAttr(isSubsystem ? QLatin1String("physical_presentations")
                                         : QLatin1String("logical_presentations"));
            PetalNode::NameValueList pratts;
            PetalNode *pres = node->findAttribute(presAttr).node;
            if (pres) {
                pratts = pres->attributes();
            }
            if (pratts.isEmpty())
                o = Object_Factory::createUMLObject(UMLObject::ot_Package, name, parentPkg);
            else
                o = Object_Factory::createUMLObject(UMLObject::ot_Folder, name, parentPkg);
            o->setID(id);
            UMLPackage *localParent = static_cast<UMLPackage*>(o);
            for (int i = 0; i < atts.count(); ++i) {
                umbrellify(atts[i].second.node, localParent);
            }
            for (int i = 0; i < pratts.count(); ++i) {
                umbrellify(pratts[i].second.node, localParent);
            }
        } else {
            o = handleControlledUnit(node, name, id, parentPkg);
            if (o == NULL) {
                uWarning() << objType << " handleControlledUnit(" << name << ") returns error";
                return false;
            }
        }
        if (isSubsystem)
            o->setStereotype(QLatin1String("subsystem"));
        parentPkg->addObject(o);

    } else if (objType == QLatin1String("Class")) {
        QString stereotype = clean(node->findAttribute(QLatin1String("stereotype")).string);
        UMLObject *o = NULL;
        if (stereotype == QLatin1String("Actor")) {
            o = Object_Factory::createUMLObject(UMLObject::ot_Actor, name, parentPkg, false);
            o->setID(id);
        } else {
            o = Object_Factory::createUMLObject(UMLObject::ot_Class, name, parentPkg, false);
            o->setID(id);
            UMLClassifier *c = static_cast<UMLClassifier*>(o);
            // set stereotype
            if (!stereotype.isEmpty()) {
                if (stereotype.toLower() == QLatin1String("interface")) {
                    c->setBaseType(UMLObject::ot_Interface);
                } else {
                    if (stereotype == QLatin1String("CORBAInterface"))
                        c->setBaseType(UMLObject::ot_Interface);
                    c->setStereotype(stereotype);
                }
            }
            // insert attributes
            AttributesReader attReader(c);
            attReader.read(node, c->name());
            // insert operations
            OperationsReader opReader(c);
            opReader.read(node, c->name());
            // insert generalizations
            SuperclassesReader superReader(c);
            superReader.read(node, c->name());
            // insert realizations
            RealizationsReader realReader(c);
            realReader.read(node, c->name());
        }
        o->setDoc(node->documentation());
        parentPkg->addObject(o);

    } else if (objType == QLatin1String("UseCase")) {
        UMLObject *o = Object_Factory::createUMLObject(UMLObject::ot_UseCase, name, parentPkg, false);
        o->setID(id);
        o->setDoc(node->documentation());
        parentPkg->addObject(o);

    } else if (objType == QLatin1String("Component") || objType == QLatin1String("module")) {
        UMLObject *o = Object_Factory::createUMLObject(UMLObject::ot_Component, name, parentPkg, false);
        o->setID(id);
        o->setDoc(node->documentation());
        parentPkg->addObject(o);

    } else if (objType == QLatin1String("Association")) {
        PetalNode *roles = node->findAttribute(QLatin1String("roles")).node;
        if (roles == NULL) {
            uError() << "cannot find roles of Association";
            return false;
        }
        UMLAssociation *assoc = new UMLAssociation(Uml::AssociationType::UniAssociation);
        PetalNode::NameValueList roleList = roles->attributes();
        for (uint i = 0; i <= 1; ++i) {
            PetalNode *roleNode = roleList[i].second.node;
            if (roleNode == NULL) {
                uError() << "roleNode of Association is NULL";
                return false;
            }
            if (roleNode->name() != QLatin1String("Role")) {
                uDebug() << name << ": expecting Role, found \""
                         << roleNode->name();
                continue;
            }
            // index 0 corresponds to Umbrello roleB
            // index 1 corresponds to Umbrello roleA
            UMLRole *role = assoc->getUMLRole(Uml::RoleType::fromInt(!i));
            QStringList initialArgs = roleNode->initialArgs();
            if (initialArgs.count() > 1) {
                QString roleName = clean(initialArgs[1]);
                if (! roleName.startsWith(QLatin1String(QLatin1String("$UNNAMED"))))
                    role->setName(roleName);
            }
            role->setID(quid(roleNode));
            QString quidref = quidu(roleNode);
            QString type = clean(roleNode->findAttribute(QLatin1String("supplier")).string);
            if (!quidref.isEmpty()) {
                role->setSecondaryId(quidref);
            }
            if (!type.isEmpty()) {
                role->setSecondaryFallback(type);
            }
            QString label = clean(roleNode->findAttribute(QLatin1String("label")).string);
            if (!label.isEmpty()) {
                role->setName(label);
            }
            QString client_cardinality = clean(roleNode->findAttribute(QLatin1String("client_cardinality")).string);
            if (!client_cardinality.isEmpty()) {
                role->setMultiplicity(client_cardinality);
            }
            QString is_navigable = clean(roleNode->findAttribute(QLatin1String("is_navigable")).string);
            if (is_navigable == QLatin1String("FALSE")) {
                assoc->setAssociationType(Uml::AssociationType::Association);
            }
            QString is_aggregate = clean(roleNode->findAttribute(QLatin1String("is_aggregate")).string);
            if (is_aggregate == QLatin1String("TRUE")) {
                assoc->setAssociationType(Uml::AssociationType::Aggregation);
            }
            QString containment = clean(roleNode->findAttribute(QLatin1String("Containment")).string);
            if (containment == QLatin1String("By Value")) {
                assoc->setAssociationType(Uml::AssociationType::Composition);
            }
            role->setDoc(roleNode->documentation());
        }
        assoc->setUMLPackage(parentPkg);
        UMLApp::app()->document()->addAssociation(assoc);

    } else if ((dt = diagramType(objType)) != Uml::DiagramType::Undefined) {
        if (parentPkg->baseType() != UMLObject::ot_Folder) {
            uError() << "umbrellify: internal error - parentPkg must be UMLFolder for diagrams";
            return false;
        }
        UMLDoc *umlDoc = UMLApp::app()->document();
        UMLFolder *rootFolder = static_cast<UMLFolder*>(parentPkg);
        UMLView *view = umlDoc->createDiagram(rootFolder, dt, name, id);
        PetalNode *items = node->findAttribute(QLatin1String("items")).node;
        if (items == NULL) {
            uError() << "umbrellify: " << objType << " attribute 'items' not found";
            return false;
        }
        PetalNode::NameValueList atts = items->attributes();
        qreal width = 0.0;
        qreal height = 0.0;
        for (int i = 0; i < atts.count(); ++i) {
            PetalNode *attr = atts[i].second.node;
            QStringList args = attr->initialArgs();
            QString objType = args[0];
            QString name = clean(args[1]);
            UMLWidget *w = 0;
            if (objType == QLatin1String("CategoryView")
                                || objType == QLatin1String("ClassView")
                                || objType == QLatin1String("UseCaseView")
                                || objType == QLatin1String("ModView")
                                || objType == QLatin1String("SubSysView")) {
                QString objID = quidu(attr);
                UMLObject *o = umlDoc->findObjectById(Uml::ID::fromString(objID));
                if (!o) {
                    uError() << "umbrellify: " << objType << " " << objID
                             << " could not be found";
                    continue;
                }
                w = Widget_Factory::createWidget(view->umlScene(), o);
                width = fetchDouble(attr, QLatin1String("width"));
                if (width == 0) {
                    // Set default value to get it displayed at all.
                    width = name.length() * 12;  // to be verified
                    uError() << objType << " " << name << ": no width found, using default" << width;
                }
                height = fetchDouble(attr, QLatin1String("height"));
                if (height == 0) {
                    // Set default value to get it displayed at all.
                    height = 100.0;
                    uError() << objType << " " << name << ": no height found, using default" << height;
                }
                w->setSize(width, height);
            } else if (objType == QLatin1String("InheritView") ||
                       objType == QLatin1String("RealizeView")) {
                QString idStr = quidu(attr);
                Uml::ID::Type assocID = Uml::ID::fromString(idStr);
                if (assocID == Uml::ID::None) {
                    uError() << objType << " has illegal id " << idStr;
                } else {
                    UMLObject *o = umlDoc->findObjectById(assocID);
                    if (o) {
                        if (o->baseType() != UMLObject::ot_Association) {
                            uError() << objType << " " << idStr
                                     << " has wrong type " << o->baseType();
                        } else {
                            Uml::AssociationType::Enum t = Uml::AssociationType::Generalization;
                            if (objType == QLatin1String("RealizeView"))
                                t = Uml::AssociationType::Realization;
                            handleAssocView(attr, atts, t, view, o);
                        }
                    } else {
                        uError() << objType << " " << idStr << " not found";
                    }
                }
                continue;   // code below dereferences `w' which is unused here
            } else if (objType == QLatin1String("AssociationViewNew")) {
                QString idStr = quidu(attr);
                Uml::ID::Type assocID = Uml::ID::fromString(idStr);
                if (assocID == Uml::ID::None) {
                    uError() << objType << " has illegal id " << idStr;
                } else {
                    UMLObject *o = umlDoc->findObjectById(assocID);
                    Uml::AssociationType::Enum t = Uml::AssociationType::UniAssociation;
                    // @todo check if we need to change `t' to a different type for certain associations
                    handleAssocView(attr, atts, t, view, o);
                }
                continue;
            } else if (objType == QLatin1String("AttachView") || objType == QLatin1String("UsesView")) {
                QString idStr = quidu(attr);
                Uml::ID::Type assocID = Uml::ID::fromString(idStr);
                if (assocID == Uml::ID::None) {
                    uError() << "AttachView has illegal id " << idStr;
                } else {
                    Uml::AssociationType::Enum assocType = Uml::AssociationType::Anchor;
                    if (objType == QLatin1String("UsesView"))
                        assocType = Uml::AssociationType::Dependency;
                    handleAssocView(attr, atts, assocType, view);
                }
                continue;
            } else if (objType == QLatin1String("NoteView")) {
                w = new NoteWidget(view->umlScene(), NoteWidget::Normal);
                width = fetchDouble(attr, QLatin1String("width"));
                height = fetchDouble(attr, QLatin1String("height"));
                if (width > 0 && height > 0)
                    w->setSize(width, height);
                PetalNode *lblNode = attr->findAttribute(QLatin1String("label")).node;
                if (lblNode) {
                    QString label = lblNode->findAttribute(QLatin1String("label")).string;
                    w->setDocumentation(label);
                }
                // Add an artificial "quidu" attribute onto `attr' because
                // handling of AttachView depends on it:
                PetalNode::NameValueList innerAtts = attr->attributes();
                PetalNode::StringOrNode value;
                value.string = Uml::ID::toString(w->id());
                PetalNode::NameValue synthQuidu(QLatin1String("quidu"), value);
                innerAtts.append(synthQuidu);
                attr->setAttributes(innerAtts);
            } else if (objType == QLatin1String("Label")) {
                QString label = attr->findAttribute(QLatin1String("label")).string;
                w = new FloatingTextWidget(view->umlScene(), Uml::TextRole::Floating, label);
                int nlines = fetchInt(attr, QLatin1String("nlines"));
                width = fetchDouble(attr, QLatin1String("max_width"));
                height = nlines * 12;  // TODO check line height
                w->setSize(width, height);
            }
            else {
                uDebug() << "unsupported object type" << objType;
                continue;
            }

            QPointF pos = fetchLocation(attr, width, height);
            if (!pos.isNull()) {
                w->setPos(pos);
            }
#if 0
            QString line_color = attr->findAttribute("line_color").string;
            unsigned int lineColor = line_color.toUInt();
            QString hexLineColor = QLatin1Char('#') + QString::number(lineColor, 16);
            QColor c(hexLineColor);
            w->setLineColorCmd(c);

            QString fill_color = attr->findAttribute("fill_color").string;
            unsigned int fillColor = fill_color.toUInt();
            QString hexFillColor = QLatin1Char('#') + QString::number(fillColor, 16);
            QColor f(hexFillColor);
            w->setFillColorCmd(f);
#endif
            view->umlScene()->setupNewWidget(w, false);
        }
    } else {
        uDebug() << "umbrellify: object type " << objType
                 << " is not yet implemented";
    }
    return true;
}

/**
 * Auxiliary function for UseCase/Component/Deployment view import
 */
bool importView(PetalNode *root,
                UMLPackage *parent,
                const QString& rootName,
                const QString& modelsName,
                const QString& firstNodeName,
                const QString& presentationsName) 
{
    PetalNode *viewRoot = root->findAttribute(rootName).node;
    if (viewRoot == NULL) {
        uDebug() << "cannot find " << rootName;
        return false;
    }
    if (viewRoot->name() != firstNodeName) {
        uError() << modelsName << ": expecting first node name "
                 << firstNodeName << ", found: " << viewRoot->name();
        return false;
    }
    PetalNode *models = viewRoot->findAttribute(modelsName).node;
    if (models == NULL) {
        uError() << "cannot find " << modelsName << " of " << rootName;
        return false;
    }
    parent->setDoc(viewRoot->documentation());

    PetalNode::NameValueList atts = models->attributes();
    bool status = true;
    for (int i = 0; i < atts.count(); ++i) {
        if (!umbrellify(atts[i].second.node, parent))
            status = false;
    }
    if (!status)
        return false;
    if (presentationsName.isEmpty())
        return true;

    PetalNode *presentations = viewRoot->findAttribute(presentationsName).node;
    if (presentations == NULL) {
        uError() << modelsName << ": cannot find " << presentationsName;
        return false;
    }
    PetalNode::NameValueList pratts = presentations->attributes();
    for (int i = 0; i < pratts.count(); ++i) {
        umbrellify(pratts[i].second.node, parent);
    }
    return true;
}

/**
 * Auxiliary method for loadFromMDL() loading of controlled unit.
 * Is kept in a separate file to reflect the fact that it is not
 * coupled with the parser (other than by the PetalNode.)
 *
 * @param root   the root of the tree
 * @param parentPkg   the owning package within which objects are created
 * @return  pointer to the newly created UMLPackage on success, NULL on error
 */
UMLPackage * petalTree2Uml(PetalNode *root, UMLPackage *parentPkg)
{
    if (root == NULL) {
        uError() << "petalTree2Uml: root is NULL";
        return NULL;
    }
    UMLPackage *rootPkg = Model_Utils::rootPackage(parentPkg);
    if (rootPkg == NULL) {
        uError() << "petalTree2Uml: internal error - rootPkg is NULL";
        return NULL;
    }
    UMLDoc *umlDoc = UMLApp::app()->document();
    Uml::ModelType::Enum mt = umlDoc->rootFolderType(rootPkg);
    QString modelsAttr(mt == Uml::ModelType::Component ? QLatin1String("physical_models")
                                                       : QLatin1String("logical_models"));
    PetalNode *models = root->findAttribute(modelsAttr).node;
    if (models == NULL) {
        uError() << "petalTree2Uml: cannot find " << modelsAttr;
        return NULL;
    }
    QStringList args = root->initialArgs();
    QString name = clean(args[1]);
    const Uml::ID::Type id = quid(root);
    UMLObject *o = Object_Factory::createUMLObject(UMLObject::ot_Folder, name, parentPkg, false);
    o->setID(id);
    parentPkg = static_cast<UMLPackage*>(o);
    PetalNode::NameValueList atts = models->attributes();
    for (int i = 0; i < atts.count(); ++i) {
        if (!umbrellify(atts[i].second.node, parentPkg)) {
            break;
        }
    }
    return parentPkg;
}

}  // namespace Import_Rose

