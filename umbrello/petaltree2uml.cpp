/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "petaltree2uml.h"

// app includes
#include "debug_utils.h"
#include "petalnode.h"
#include "model_utils.h"
#include "import_utils.h"
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
    str.remove('\"');
    str.remove(QRegExp("^Logical View::"));
    return str;
}

/**
 * Extract the quid attribute from a petal node and return it as a Uml::ID::Type.
 */
Uml::ID::Type quid(const PetalNode *node)
{
    QString quidStr = node->findAttribute("quid").string;
    if (quidStr.isEmpty())
        return Uml::ID::None;
    quidStr.remove('\"');
    return Uml::ID::fromString(quidStr);
}

/**
 * Extract the quidu attribute from a petal node.
 */
QString quidu(const PetalNode *node)
{
    QString quiduStr = node->findAttribute("quidu").string;
    if (quiduStr.isEmpty())
        return QString();
    quiduStr.remove('\"');
    return quiduStr;
}

/**
 * Extract the location attribute from a petal node.
 */
QPointF fetchLocation(const PetalNode *node, qreal width, qreal height)
{
    QString location = node->findAttribute("location").string;
    QStringList a = location.split(' ');
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
    n.remove(QRegExp("^.*::"));  // don't consider the scope prefix, it may contain spaces
    UMLObject::ObjectType t = (n.contains(QRegExp("\\W")) ? UMLObject::ot_Datatype : UMLObject::ot_Class);
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
    QString vis = from->findAttribute("exportControl").string;
    if (!vis.isEmpty()) {
        Uml::Visibility::Enum v = Uml::Visibility::fromString(clean(vis.toLower()));
        to->setVisibility(v);
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
        m_attributeTag(attributeTag),
        m_elementName(elementName),
        m_itemTypeDesignator(itemTypeDesignator) {
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
            QString doc = attNode->findAttribute("documentation").string;
            if (! doc.isEmpty())
                item->setDoc(doc);
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
 * @param parentPkg  Pointer to the current parent UMLPackage.
 * @return      True if the node actually contained a controlled unit.
 */
bool handleControlledUnit(PetalNode *node, const QString& name, Uml::ID::Type id, UMLPackage * parentPkg)
{
    Q_UNUSED(id);
    if (node->findAttribute("is_unit").string != "TRUE")
        return false;
    //bool is_loaded = (node->findAttribute("is_loaded").string != "FALSE");
    QString file_name = node->findAttribute("file_name").string;
    if (file_name.isEmpty()) {
        uError() << name << ": attribute file_name not found (?)";
        return true;
    }
    file_name = file_name.mid(1, file_name.length() - 2);  // remove sourrounding ""
    /* I wanted to use
                  file_name.replace(QRegExp("\\\\+") "/");
       but this did not work using Qt 4.6.3. Workaround:
     */
    file_name.replace("\\\\", "/");
    file_name.replace(QRegExp("/+"), "/");
    /* End of workaround */

    if (file_name.startsWith("$")) {
        const int firstSlash = file_name.indexOf(QChar('/'));
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
            return false;
        }
        QString envVar(envVarBA);
        uDebug() << name << ": envVar " << envVarName << " contains " << envVar;
        if (envVar.endsWith("/"))
            envVar.chop(1);
        if (firstSlash < 0)
            file_name = envVar;
        else
            file_name = envVar + file_name.mid(firstSlash);
    }
    if (!file_name.startsWith("/")) {
        // Must have an absolute path by now.
        // If we don't then use the directory of the .mdl file.
        file_name = Import_Rose::mdlPath() + file_name;
    }
    QFile file(file_name);
    if (!file.exists()) {
        uError() << name << ": file_name " << file_name << " not found";
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        uError() << name << ": file_name " << file_name << " cannot be opened";
        return false;
    }
    bool status = loadFromMDL(file, parentPkg);
    file.close();
    return status;
}

void handleAssocView(PetalNode *attr,
                     const PetalNode::NameValueList& parentAttrs,
                     Uml::AssociationType::Enum assocType,
                     UMLView *view,
                     UMLObject *umlAssoc = 0)
{
    QString supplier = attr->findAttribute("supplier").string;
    QString client   = attr->findAttribute("client").string;
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
            if (supW && cliW) {
                // UMLAssociation *a = static_cast<UMLAssociation*>(o);
                AssociationWidget *aw = AssociationWidget::create
                                         (view->umlScene(), cliW, assocType,
                                          supW, umlAssoc);
                view->umlScene()->addAssociation(aw);
            } else {
                uError() << "InheritView: client widget " << clIdStr
                         << " is not on diagram (?)";
            }
        } else {
            uError() << "InheritView: bad or nonexistent quidu at client "
                     << client << " (" << cli->name() << ")";
        }
    } else {
        uError() << "InheritView: could not find client with tag " << client;
    }
}

/**
 * Create an Umbrello object from a PetalNode of the Logical View.
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

    if (objType == "Class_Category") {
        UMLObject *o = Import_Utils::createUMLObject(UMLObject::ot_Package, name, parentPkg);
        o->setID(id);
        PetalNode *logical_models = node->findAttribute("logical_models").node;
        if (logical_models) {
            UMLPackage *localParent = static_cast<UMLPackage*>(o);
            PetalNode::NameValueList atts = logical_models->attributes();
            for (int i = 0; i < atts.count(); ++i) {
                umbrellify(atts[i].second.node, localParent);
            }
        } else if (!handleControlledUnit(node, name, id, parentPkg)) {
            uWarning() << objType << " handleControlledUnit(" << name
                << ") returned an error";
        }

    } else if (objType == "Class") {
        QString stereotype = clean(node->findAttribute("stereotype").string);
        if (stereotype == "Actor") {
            UMLObject *o = Import_Utils::createUMLObject(UMLObject::ot_Actor, name, parentPkg);
            o->setID(id);
        } else {
            UMLObject *o = Import_Utils::createUMLObject(UMLObject::ot_Class, name, parentPkg);
            o->setID(id);
            UMLClassifier *c = static_cast<UMLClassifier*>(o);
            // set stereotype
            if (!stereotype.isEmpty()) {
                if (stereotype.toLower() == "interface")
                    c->setBaseType(UMLObject::ot_Interface);
                else
                    c->setStereotype(stereotype);
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

    } else if (objType == "UseCase") {
        UMLObject *o = Import_Utils::createUMLObject(UMLObject::ot_UseCase, name, parentPkg);
        o->setID(id);

    } else if (objType == "Association") {
        PetalNode *roles = node->findAttribute("roles").node;
        if (node == NULL) {
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
            if (roleNode->name() != "Role") {
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
                if (! roleName.startsWith(QLatin1String("$UNNAMED")))
                    role->setName(roleName);
            }
            role->setID(quid(roleNode));
            QString quidref = quidu(roleNode);
            QString type = clean(roleNode->findAttribute("supplier").string);
            if (!quidref.isEmpty()) {
                role->setSecondaryId(quidref);
            }
            if (!type.isEmpty()) {
                role->setSecondaryFallback(type);
            }
            QString label = clean(roleNode->findAttribute("label").string);
            if (!label.isEmpty()) {
                role->setName(label);
            }
            QString client_cardinality = clean(roleNode->findAttribute("client_cardinality").string);
            if (!client_cardinality.isEmpty()) {
                role->setMultiplicity(client_cardinality);
            }
            QString is_navigable = clean(roleNode->findAttribute("is_navigable").string);
            if (is_navigable == "FALSE") {
                assoc->setAssociationType(Uml::AssociationType::Association);
            }
            QString is_aggregate = clean(roleNode->findAttribute("is_aggregate").string);
            if (is_aggregate == "TRUE") {
                assoc->setAssociationType(Uml::AssociationType::Aggregation);
            }
            QString containment = clean(roleNode->findAttribute("Containment").string);
            if (containment == "By Value") {
                assoc->setAssociationType(Uml::AssociationType::Composition);
            }
            QString doc = roleNode->findAttribute("documentation").string;
            if (! doc.isEmpty())
                role->setDoc(doc);
        }
        UMLApp::app()->document()->addAssociation(assoc);

    } else if (objType == "ClassDiagram" || objType == "UseCaseDiagram") {
        Uml::DiagramType::Enum dt = (objType == "UseCaseDiagram" ? Uml::DiagramType::UseCase
                                                                 : Uml::DiagramType::Class);
        UMLDoc *umlDoc = UMLApp::app()->document();
        UMLFolder *rootFolder = umlDoc->rootFolder(Model_Utils::convert_DT_MT(dt));
        UMLView *view = umlDoc->createDiagram(rootFolder, dt, name, id);
        PetalNode *items = node->findAttribute("items").node;
        PetalNode::NameValueList atts = items->attributes();
        qreal width = 0.0;
        qreal height = 0.0;
        for (int i = 0; i < atts.count(); ++i) {
            PetalNode *attr = atts[i].second.node;
            QStringList args = attr->initialArgs();
            QString objType = args[0];
            QString name = clean(args[1]);
            UMLWidget *w = 0;
            if (objType == "CategoryView" || objType == "ClassView" || objType == "UseCaseView") {
                QString objID = quidu(attr);
                UMLObject *o = umlDoc->findObjectById(Uml::ID::fromString(objID));
                if (!o) {
                    uError() << "umbrellify: object type " << objType
                             << " could not be found";
                    continue;
                }
                w = Widget_Factory::createWidget(view->umlScene(), o);
                width = fetchDouble(attr, "width");
                height = fetchDouble(attr, "height");
                if (width > 0 && height > 0)
                    w->setSize(width, height);
            } else if (objType == "InheritView" || objType == "RealizeView") {
                QString idStr = quidu(attr);
                Uml::ID::Type assocID = Uml::ID::fromString(idStr);
                if (assocID == Uml::ID::None) {
                    uError() << "InheritView has illegal id " << idStr;
                } else {
                    UMLObject *o = umlDoc->findObjectById(assocID);
                    if (o) {
                        if (o->baseType() != UMLObject::ot_Association) {
                            uError() << "umbrellify: generalization " << idStr
                                     << " has wrong type " << o->baseType();
                        } else {
                            Uml::AssociationType::Enum t = Uml::AssociationType::Generalization;
                            if (objType == "RealizeView")
                                t = Uml::AssociationType::Realization;
                            handleAssocView(attr, atts, t, view, o);
                        }
                    } else {
                        uError() << "umbrellify: generalization " << idStr << " not found";
                    }
                }
                continue;   // code below dereferences `w' which is unused here
            } else if (objType == "AttachView" || objType == "UsesView") {
                QString idStr = quidu(attr);
                Uml::ID::Type assocID = Uml::ID::fromString(idStr);
                if (assocID == Uml::ID::None) {
                    uError() << "AttachView has illegal id " << idStr;
                } else {
                    Uml::AssociationType::Enum assocType = Uml::AssociationType::Anchor;
                    if (objType == "UsesView")
                        assocType = Uml::AssociationType::Dependency;
                    handleAssocView(attr, atts, assocType, view);
                }
                continue;
            } else if (objType == "NoteView") {
                w = new NoteWidget(view->umlScene(), NoteWidget::Normal);
                width = fetchDouble(attr, "width");
                height = fetchDouble(attr, "height");
                if (width > 0 && height > 0)
                    w->setSize(width, height);
                PetalNode *lblNode = attr->findAttribute("label").node;
                if (lblNode) {
                    QString label = lblNode->findAttribute("label").string;
                    w->setDocumentation(label);
                }
            } else if (objType == "Label") {
                QString label = attr->findAttribute("label").string;
                w = new FloatingTextWidget(view->umlScene(), Uml::TextRole::Floating, label);
                int nlines = fetchInt(attr, "nlines");
                width = fetchDouble(attr, "max_width");
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

bool importLogicalPresentations(PetalNode *root, const QString &category, UMLPackage *parent)
{
    PetalNode *root_category = root->findAttribute(category).node;
    if (root_category == NULL) {
        uError() << "importLogicalPresentations: cannot find" << category;
        return false;
    }
    if (root_category->name() != "Class_Category") {
        uError() << "importLogicalPresentations: expecting root_category object Class_Category";
        return false;
    }
    PetalNode *logical_presentations = root_category->findAttribute("logical_presentations").node;
    if (logical_presentations == NULL) {
        uError() << "importLogicalPresentations: cannot find logical_presentations";
        return false;
    }
    PetalNode::NameValueList atts = logical_presentations->attributes();
    for (int i = 0; i < atts.count(); ++i) {
        umbrellify(atts[i].second.node, parent);
    }
    return true;
}

/**
 * Auxiliary function for UseCase/Component/Deployment view import
 */
bool importView(PetalNode *root, const QString& rootName,
                const QString& modelsName, UMLPackage *parent) 
{
    PetalNode *viewRoot = root->findAttribute(rootName).node;
    if (viewRoot == NULL) {
        uDebug() << "cannot find " << rootName;
        return false;
    }
    PetalNode *models = viewRoot->findAttribute(modelsName).node;
    if (models == NULL) {
        uError() << "cannot find " << modelsName << " of " << rootName;
        return false;
    }
    PetalNode::NameValueList atts = models->attributes();
    bool status = true;
    for (int i = 0; i < atts.count(); ++i) {
        if (!umbrellify(atts[i].second.node, parent))
            status = false;
    }
    return status;
}

/**
 * Auxiliary method for loadFromMDL() loading of controlled unit.
 * Is kept in a separate file to reflect the fact that it is not
 * coupled with the parser (other than by the PetalNode.)
 *
 * @param root   the root of the tree
 * @return  true for success.
 */
bool petalTree2Uml(PetalNode *root, UMLPackage *parentPkg)
{
    if (root == NULL) {
        uError() << "petalTree2Uml: root is NULL";
        return false;
    }
    QStringList args = root->initialArgs();
    QString name = clean(args[1]);
    const Uml::ID::Type id = quid(root);
    UMLObject *o = Import_Utils::createUMLObject(UMLObject::ot_Package, name, parentPkg);
    o->setID(id);
    parentPkg = static_cast<UMLPackage*>(o);
    PetalNode *logical_models = root->findAttribute("logical_models").node;
    if (logical_models == NULL) {
        uError() << "petalTree2Uml: cannot find logical_models";
        return false;
    }
    PetalNode::NameValueList atts = logical_models->attributes();
    bool status = true;
    for (int i = 0; i < atts.count(); ++i) {
        if (!umbrellify(atts[i].second.node, parentPkg)) {
            status = false;
            break;
        }
    }
    return status;
}

}  // namespace Import_Rose

