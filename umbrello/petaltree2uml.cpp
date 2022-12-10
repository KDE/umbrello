/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "petaltree2uml.h"

// app includes
#define DBG_SRC QStringLiteral("Import_Rose")
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
#include "datatype.h"
#include "enum.h"
#include "enumliteral.h"
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
#include "activitywidget.h"
#include "associationwidget.h"
#include "boxwidget.h"
#include "classifierwidget.h"
#include "floatingtextwidget.h"
#include "forkjoinwidget.h"
#include "notewidget.h"
#include "objectnodewidget.h"
#include "statewidget.h"
#include "umlwidgetlist.h"
#include "widget_factory.h"

// qt includes
#include <QtGlobal>
#include <QFile>
#include <QRegExp>

DEBUG_REGISTER(petalTree2Uml)

namespace Import_Rose {

/**
 * The Rose diagram coordinate system is roughly twice the scale of Qt.
 * I.e. when going from Rose diagram-object location/width/height to Qt,
 * we need to shrink everything.
 * The exact factor can be configured here.
 */
const qreal Rose2Qt = 0.5;

typedef QMap<QString, UMLWidget*> ViewTagToWidget_Map;
// map of diagram objects (is reset on each diagram parsed)
ViewTagToWidget_Map viewTagToWidget;

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
    str.remove(QRegExp(QStringLiteral("^.+::")));
    return str;
}

/**
 * Extract the quid attribute from a petal node and return it as a Uml::ID::Type.
 */
Uml::ID::Type quid(const PetalNode *node)
{
    QString quidStr = node->findAttribute(QStringLiteral("quid")).string;
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
    QString quiduStr = node->findAttribute(QStringLiteral("quidu")).string;
    if (quiduStr.isEmpty())
        return QString();
    quiduStr.remove(QLatin1Char('\"'));
    return quiduStr;
}

/**
 * Extract the location attribute from a petal node.
 * The extracted X,Y coordinates will be adjusted by the factor Rose2Qt
 * unless width and/or height are given as 0.
 */
QPointF fetchLocation(const PetalNode *node, qreal width, qreal height)
{
    QString location = node->findAttribute(QStringLiteral("location")).string;
    if (location.isEmpty())
        return QPointF();
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
    if (qFuzzyIsNull(width) || qFuzzyIsNull(height))
        return QPointF(x, y);

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
qreal fetchDouble(const PetalNode *node, const QString &attribute, bool applyRose2Qt = true)
{
    bool ok;
    QString s = node->findAttribute(attribute).string;
    if (s.isEmpty()) {
        logDebug1("fetchDouble(%1) : attribute not found, returning 0.0", attribute);
        return 0.0;
    }
    qreal value = s.toDouble(&ok);
    if (!ok) {
        logDebug2("fetchDouble(%1) value \"%2\" : error on converting to double (returning 0.0)",
                  attribute, s);
        return 0.0;
    }
    if (applyRose2Qt)
        value *= Rose2Qt;
    return value;
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
    n.remove(QRegExp(QStringLiteral("^.*::")));  // don't consider the scope prefix, it may contain spaces
    UMLObject::ObjectType t = (n.contains(QRegExp(QStringLiteral("\\W"))) ? UMLObject::ot_Datatype
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
    QString vis = from->findAttribute(QStringLiteral("exportControl")).string;
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
        if (attributes == 0) {
#ifdef VERBOSE_DEBUGGING
            logDebug2("petaltree2uml ClassifierListReader::read(%1): no %2 found",
                      name, m_attributeTag);
#endif
            return;
        }
        PetalNode::NameValueList attributeList = attributes->attributes();
        for (int i = 0; i < attributeList.count(); ++i) {
            PetalNode *attNode = attributeList[i].second.node;
            QStringList initialArgs = attNode->initialArgs();
            if (attNode->name() != m_elementName) {
                logDebug3("petaltree2uml ClassifierListReader::read(%1): expecting %2, found %3",
                          name, m_elementName, initialArgs[0]);
                continue;
            }
            UMLObject *item = createListItem();
            if (initialArgs.count() > 1)
                item->setName(clean(initialArgs[1]));
            item->setID(quid(attNode));
            QString quidref = quidu(attNode);
            QString type = clean(attNode->findAttribute(m_itemTypeDesignator).string);
            if (quidref.isEmpty() && !type.isEmpty()) {
                Object_Factory::assignUniqueIdOnCreation(true);
                UMLDatatype *dt = UMLApp::app()->document()->createDatatype(type);
                Object_Factory::assignUniqueIdOnCreation(false);
                quidref = Uml::ID::toString(dt->id());
            }
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
        logDebug2("petaltree2uml AttributesReader::insertAtParent(%1): Adding attribute %2",
                  m_classifier->name(), item->name());
        m_classifier->addAttribute(item->asUMLAttribute());
    }
protected:
    UMLClassifier *m_classifier;
};

class LiteralsReader : public ClassifierListReader
{
public:
    LiteralsReader(UMLEnum *e)
      : ClassifierListReader("class_attributes", "ClassAttribute", "type") {
        m_enum = e;
    }
    virtual ~LiteralsReader() {}
    UMLObject *createListItem() {
        return new UMLEnumLiteral(m_enum);
    }
    void insertAtParent(const PetalNode *, UMLObject *item) {
        UMLEnumLiteral *el = item->asUMLEnumLiteral();
        if (el) {
            logDebug2("petaltree2uml LiteralsReader::insertAtParent(%1): Adding enumliteral %2",
                      m_enum->name(), item->name());
            m_enum->addEnumLiteral(el);
        } else {
            logError1("petaltree2uml LiteralsReader insertAtParent: Cannot cast %1 to UMLEnumLiteral",
                      item->name());
        }
    }
protected:
    UMLEnum *m_enum;
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
        logDebug2("petaltree2uml ParametersReader::insertAtParent(%1): Adding parameter %2",
                  m_operation->name(), item->name());
        m_operation->addParm(item->asUMLAttribute());
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
        logDebug2("petaltree2uml OperationsReader::insertAtParent(%1): Adding operation %2",
                  m_classifier->name(), item->name());
        UMLOperation *op = item->asUMLOperation();
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
        UMLAssociation *assoc = item->asUMLAssociation();
        if (!quid.isEmpty()) {
            assoc->getUMLRole(Uml::RoleType::B)->setSecondaryId(quid);
        }
        if (!type.isEmpty()) {
            assoc->getUMLRole(Uml::RoleType::B)->setSecondaryFallback(type);
        }
    }
    void insertAtParent(const PetalNode *, UMLObject *item) {
        UMLAssociation *assoc = item->asUMLAssociation();
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
        UMLAssociation *assoc = item->asUMLAssociation();
        if (!quid.isEmpty()) {
            assoc->getUMLRole(Uml::RoleType::B)->setSecondaryId(quid);
        }
        if (!type.isEmpty()) {
            assoc->getUMLRole(Uml::RoleType::B)->setSecondaryFallback(type);
        }
    }
    void insertAtParent(const PetalNode *, UMLObject *item) {
        UMLAssociation *assoc = item->asUMLAssociation();
        assoc->setObject(m_classifier, Uml::RoleType::A);
        assoc->setUMLPackage(m_classifier->umlPackage());
        UMLApp::app()->document()->addAssociation(assoc);
    }
protected:
    UMLClassifier *m_classifier;
};

class UsesReader : public ClassifierListReader
{
public:
    UsesReader(UMLClassifier *c)
      : ClassifierListReader("used_nodes", "Uses_Relationship", "supplier") {
        m_classifier = c;
    }
    virtual ~UsesReader() {}
    UMLObject *createListItem() {
        return new UMLAssociation(Uml::AssociationType::Dependency);
    }
    /**
     * Override parent implementation: The secondary data is not for the
     * UMLAssociation itself but for its role B object.
     */
    void setTypeReferences(UMLObject *item,
                           const QString& quid, const QString& type) {
        UMLAssociation *assoc = item->asUMLAssociation();
        if (!quid.isEmpty()) {
            assoc->getUMLRole(Uml::RoleType::B)->setSecondaryId(quid);
        }
        if (!type.isEmpty()) {
            assoc->getUMLRole(Uml::RoleType::B)->setSecondaryFallback(type);
        }
    }
    void insertAtParent(const PetalNode *, UMLObject *item) {
        UMLAssociation *assoc = item->asUMLAssociation();
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
    if (node->findAttribute(QStringLiteral("is_unit")).string != QStringLiteral("TRUE"))
        return 0;
    //bool is_loaded = (node->findAttribute(QStringLiteral("is_loaded")).string != QStringLiteral("FALSE"));
    QString file_name = node->findAttribute(QStringLiteral("file_name")).string;
    if (file_name.isEmpty()) {
        logError1("Import_Rose::handleControlledUnit(%1): attribute file_name not found (?)", name);
        return 0;
    }
    file_name = file_name.mid(1, file_name.length() - 2);  // remove surrounding ""
    /* I wanted to use
                  file_name.replace(QRegExp("\\\\+") "/");
       but this did not work using Qt 4.6.3. Workaround:
     */
    file_name.replace(QStringLiteral("\\\\"), QStringLiteral("/"));
    file_name.replace(QRegExp(QStringLiteral("/+")), QStringLiteral("/"));
    /* End of workaround */

    if (file_name.startsWith(QStringLiteral("$"))) {
        logDebug2("handleControlledUnit(%1) file_name before pathmap subst: %2",
                  name, file_name);
        const int firstSlash = file_name.indexOf(QLatin1Char('/'));
        QString envVarName;
        if (firstSlash < 0) {
            envVarName = file_name.mid(1);
        } else {
            envVarName = file_name.mid(1, firstSlash - 1);
        }
        QByteArray envVarBA = qgetenv(envVarName.toLatin1());
        if (envVarBA.isNull() || envVarBA.isEmpty()) {
            logError3("handleControlledUnit(%1): cannot process file_name %2 because env var %3 is not set",
                      name, file_name, envVarName);
            return 0;
        }
        QString envVar(QString::fromLatin1(envVarBA));
        logDebug3("handleControlledUnit(%1) : envVar %2 contains %3", name, envVarName, envVar);
        if (envVar.endsWith(QLatin1Char('/')))
            envVar.chop(1);
        if (firstSlash < 0)
            file_name = envVar;
        else
            file_name = envVar + file_name.mid(firstSlash);
        logDebug2("handleControlledUnit(%1) file_name after pathmap subst: %2",
                  name, file_name);
    } else {
        logDebug2("handleControlledUnit(%1) file_name: %2", name, file_name);
    }

    QFileInfo fi(file_name);
    if (!fi.isAbsolute()) {
        // Must have an absolute path by now.
        // If we don't then use the directory of the .mdl file.
        file_name = Import_Rose::mdlPath() + file_name;
    }
    QFile file(file_name);
    if (!file.exists()) {
        logError2("Import_Rose::handleControlledUnit(%1): file_name %2 not found", name, file_name);
        return 0;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        logError2("Import_Rose::handleControlledUnit(%1): file_name %2 cannot be opened", name, file_name);
        return 0;
    }
    UMLPackage *controlledUnit = loadFromMDL(file, parentPkg);
    return controlledUnit;
}

void handleAssocView(PetalNode *attr,
                     const PetalNode::NameValueList& parentAttrs,
                     Uml::AssociationType::Enum assocType,
                     UMLView *view,
                     UMLObject *umlAssoc = 0)
{
    QString assocStr = Uml::AssociationType::toString(assocType);
    PetalNode *roleview_list = attr->findAttribute(QStringLiteral("roleview_list")).node;
    QString supplier, client;
    if (roleview_list) {
        PetalNode::StringOrNode supElem, cliElem;
        PetalNode::NameValueList roles = roleview_list->attributes();
        if (roles.length() < 2) {
            logError1("Import_Rose::handleAssocView: %1 roleview_list should have 2 elements", assocStr);
            return;
        }
        PetalNode *supNode = roles[0].second.node;
        PetalNode *cliNode = roles[1].second.node;
        if (supNode && cliNode) {
            supElem = supNode->findAttribute(QStringLiteral("supplier"));
            cliElem = cliNode->findAttribute(QStringLiteral("supplier"));  // not a typo, really "supplier"
        } else {
            logError1("Import_Rose::handleAssocView: %1 roleview_list roles are incomplete", assocStr);
            return;
        }
        supplier = supElem.string;
        client   = cliElem.string;
    } else {
        supplier = attr->findAttribute(QStringLiteral("supplier")).string;
        client   = attr->findAttribute(QStringLiteral("client")).string;
    }
    if (supplier.isEmpty()) {
        logError1("Import_Rose::handleAssocView: %1 attribute 'supplier' non-existent or empty", assocStr);
        return;
    }
    if (client.isEmpty()) {
        logError1("Import_Rose::handleAssocView: %1 attribute 'client' non-existent or empty", assocStr);
        return;
    }
    UMLWidget *supW = nullptr;
    UMLWidget *cliW = nullptr;
    if (viewTagToWidget.contains(supplier))
        supW = viewTagToWidget[supplier];
    else
        logDebug1("handleAssocView: %1 attribute 'supplier' not in viewTagToWidget", assocStr);
    if (viewTagToWidget.contains(client))
        cliW = viewTagToWidget[client];
    else
        logDebug1("handleAssocView: %1 attribute 'client' not in viewTagToWidget", assocStr);
    if (!supW || !cliW) {
        PetalNode *sup = 0, *cli = 0;
        for (int c = 0; c < parentAttrs.count(); ++c) {
            PetalNode *n = parentAttrs[c].second.node;
            QStringList initArgs = n->initialArgs();
            QString tag = initArgs.last();
            if (tag == client)
                cli = n;
            else if (tag == supplier)
                sup = n;
        }
        if (!sup || !cli) {
            logError2("Import_Rose::handleAssocView: %1 could not find client with tag %2", assocStr, client);
            return;
        }
        QString spIdStr = quidu(sup);
        Uml::ID::Type spId = Uml::ID::fromString(spIdStr);
        QString clIdStr = quidu(cli);
        Uml::ID::Type clId = Uml::ID::fromString(clIdStr);
        if (spId == Uml::ID::None || clId == Uml::ID::None) {
            logError3("Import_Rose::handleAssocView: %1 bad or nonexistent quidu at client %2 (%3)",
                      assocStr, client, cli->name());
            return;
        }
        supW = view->umlScene()->widgetOnDiagram(spId);
        cliW = view->umlScene()->widgetOnDiagram(clId);
        if (supW == 0) {
            logError2("Import_Rose::handleAssocView: %1 supplier widget %2 is not on diagram (?)",
                      assocStr, spIdStr);
            return;
        }
        if (cliW == 0) {
            logError2("Import_Rose::handleAssocView: %1 client widget is not on diagram (?)",
                      assocStr, clIdStr);
            return;
        }
    }
    AssociationWidget *aw = AssociationWidget::create (view->umlScene(), cliW, assocType,
                                                       supW, umlAssoc);
    view->umlScene()->addAssociation(aw);
}

Uml::DiagramType::Enum diagramType(QString objType)
{
    Uml::DiagramType::Enum dt;
    dt = (objType == QStringLiteral("ClassDiagram")    ? Uml::DiagramType::Class :
          objType == QStringLiteral("UseCaseDiagram")  ? Uml::DiagramType::UseCase :
          objType == QStringLiteral("State_Diagram")   ? Uml::DiagramType::State :
          objType == QStringLiteral("ActivityDiagram") ? Uml::DiagramType::Activity :
          objType == QStringLiteral("Module_Diagram")  ? Uml::DiagramType::Component :
          objType == QStringLiteral("Process_Diagram") ? Uml::DiagramType::Deployment :
          // not yet implemented: InteractionDiagram (Sequence),
          //                      ObjectDiagram (Collaboration)
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
    if (node == 0) {
        logError0("Import_Rose::umbrellify: node is NULL");
        return false;
    }
    QStringList args = node->initialArgs();
    QString objType = args[0];
    QString name = clean(args[1]);
    Uml::ID::Type id = quid(node);
    Uml::DiagramType::Enum dt = Uml::DiagramType::Undefined;

    if (objType == QStringLiteral("Class_Category") || objType == QStringLiteral("SubSystem")) {
        const bool isSubsystem = (objType == QStringLiteral("SubSystem"));
        QString modelsAttr(isSubsystem ? QStringLiteral("physical_models")
                                       : QStringLiteral("logical_models"));
        // statemachine    (object State_Machine "State/Activity Model"
        PetalNode *statemachine = node->findAttribute(QStringLiteral("statemachine")).node;
        PetalNode *models = node->findAttribute(modelsAttr).node;
        UMLObject *o = 0;
        if (models) {
            PetalNode::NameValueList atts = models->attributes();
            QString presAttr(isSubsystem ? QStringLiteral("physical_presentations")
                                         : QStringLiteral("logical_presentations"));
            PetalNode::NameValueList pratts;
            PetalNode *pres = node->findAttribute(presAttr).node;
            if (pres) {
                pratts = pres->attributes();
            }
            UMLObject::ObjectType containerType = UMLObject::ot_Package;
            if (!pratts.isEmpty() || statemachine)
                containerType = UMLObject::ot_Folder;
            o = Object_Factory::createUMLObject(containerType, name, parentPkg);
            o->setID(id);
            UMLPackage *localParent = o->asUMLPackage();
            for (int i = 0; i < atts.count(); ++i) {
                umbrellify(atts[i].second.node, localParent);
            }
            for (int i = 0; i < pratts.count(); ++i) {
                umbrellify(pratts[i].second.node, localParent);
            }
        } else {
            o = handleControlledUnit(node, name, id, parentPkg);
            if (o == 0) {
                logWarn2("Import_Rose::umbrellify: %1 handleControlledUnit(%2) returns error",
                         objType, name);
                return false;
            }
        }
        if (isSubsystem)
            o->setStereotypeCmd(QStringLiteral("subsystem"));
        parentPkg->addObject(o);

        if (statemachine) {
            //    statediagrams       (list StateDiagrams
            PetalNode *statediagrams = statemachine->findAttribute(QStringLiteral("statediagrams")).node;
            PetalNode::NameValueList diagramList = statediagrams->attributes();
            UMLPackage *localParent = static_cast<UMLPackage*>(o);
            for (int i = 0; i < diagramList.count(); ++i) {
                umbrellify(diagramList[i].second.node, localParent);
            }
        }

    } else if (objType == QStringLiteral("Class")) {
        QString stereotype = clean(node->findAttribute(QStringLiteral("stereotype")).string);
        UMLObject *o = 0;
        if (stereotype == QStringLiteral("Actor")) {
            o = Object_Factory::createUMLObject(UMLObject::ot_Actor, name, parentPkg, false);
            o->setID(id);
        } else if (stereotype.contains(QStringLiteral("enum"), Qt::CaseInsensitive)) {
            o = Object_Factory::createUMLObject(UMLObject::ot_Enum, name, parentPkg, false);
            o->setID(id);
            UMLEnum *e = o->asUMLEnum();
            // set stereotype
            if (stereotype.compare(QStringLiteral("enumeration"), Qt::CaseInsensitive) &&
                       stereotype.compare(QStringLiteral("enum"), Qt::CaseInsensitive) ) {
                e->setStereotypeCmd(stereotype);
            }
            // insert literals
            LiteralsReader litReader(e);
            litReader.read(node, e->name());
        } else {
            o = Object_Factory::createUMLObject(UMLObject::ot_Class, name, parentPkg, false);
            o->setID(id);
            UMLClassifier *c = o->asUMLClassifier();
            // set stereotype
            if (!stereotype.isEmpty()) {
                if (stereotype.toLower() == QStringLiteral("interface")) {
                    c->setBaseType(UMLObject::ot_Interface);
                } else {
                    if (stereotype == QStringLiteral("CORBAInterface"))
                        c->setBaseType(UMLObject::ot_Interface);
                    c->setStereotypeCmd(stereotype);
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
            // insert dependency associations
            UsesReader usesReader(c);
            usesReader.read(node, c->name());
        }
        o->setDoc(node->documentation());
        parentPkg->addObject(o);

    } else if (objType == QStringLiteral("UseCase")) {
        UMLObject *o = Object_Factory::createUMLObject(UMLObject::ot_UseCase, name, parentPkg, false);
        o->setID(id);
        o->setDoc(node->documentation());
        parentPkg->addObject(o);

    } else if (objType == QStringLiteral("Component") || objType == QStringLiteral("module")) {
        UMLObject *o = Object_Factory::createUMLObject(UMLObject::ot_Component, name, parentPkg, false);
        o->setID(id);
        o->setDoc(node->documentation());
        parentPkg->addObject(o);

    } else if (objType == QStringLiteral("Association")) {
        PetalNode *roles = node->findAttribute(QStringLiteral("roles")).node;
        if (roles == 0) {
            logError1("Import_Rose::umbrellify: cannot find roles of Association quid=%1",
                      Uml::ID::toString(id));
            return false;
        }
        UMLAssociation *assoc = new UMLAssociation(Uml::AssociationType::UniAssociation);
        PetalNode::NameValueList roleList = roles->attributes();
        for (uint i = 0; i <= 1; ++i) {
            PetalNode *roleNode = roleList[i].second.node;
            if (roleNode == 0) {
                logError1("Import_Rose::umbrellify: roleNode of Association %1 is NULL",
                          Uml::ID::toString(id));
                return false;
            }
            if (roleNode->name() != QStringLiteral("Role")) {
                logDebug2("umbrellify(%1): expecting Role, found '%2'", name, roleNode->name());
                continue;
            }
            // index 0 corresponds to Umbrello roleB
            // index 1 corresponds to Umbrello roleA
            UMLRole *role = assoc->getUMLRole(Uml::RoleType::fromInt(!i));
            QStringList initialArgs = roleNode->initialArgs();
            if (initialArgs.count() > 1) {
                QString roleName = clean(initialArgs[1]);
                if (! roleName.startsWith(QStringLiteral("$UNNAMED")))
                    role->setName(roleName);
            }
            role->setID(quid(roleNode));
            QString quidref = quidu(roleNode);
            QString type = clean(roleNode->findAttribute(QStringLiteral("supplier")).string);
            if (!quidref.isEmpty()) {
                role->setSecondaryId(quidref);
            }
            if (!type.isEmpty()) {
                role->setSecondaryFallback(type);
            }
            QString label = clean(roleNode->findAttribute(QStringLiteral("label")).string);
            if (!label.isEmpty()) {
                role->setName(label);
            }
            QString client_cardinality = clean(roleNode->findAttribute(QStringLiteral("client_cardinality")).string);
            if (!client_cardinality.isEmpty()) {
                role->setMultiplicity(client_cardinality);
            }
            QString is_navigable = clean(roleNode->findAttribute(QStringLiteral("is_navigable")).string);
            if (is_navigable == QStringLiteral("FALSE")) {
                assoc->setAssociationType(Uml::AssociationType::Association);
            }
            QString is_aggregate = clean(roleNode->findAttribute(QStringLiteral("is_aggregate")).string);
            if (is_aggregate == QStringLiteral("TRUE")) {
                assoc->setAssociationType(Uml::AssociationType::Aggregation);
            }
            QString containment = clean(roleNode->findAttribute(QStringLiteral("Containment")).string);
            if (containment == QStringLiteral("By Value")) {
                assoc->setAssociationType(Uml::AssociationType::Composition);
            }
            role->setDoc(roleNode->documentation());
        }
        assoc->setUMLPackage(parentPkg);
        UMLApp::app()->document()->addAssociation(assoc);

    } else if ((dt = diagramType(objType)) != Uml::DiagramType::Undefined) {
        if (parentPkg->baseType() != UMLObject::ot_Folder) {
            logError1("Import_Rose::umbrellify diagramType %1: internal error - "
                      "parentPkg must be UMLFolder for diagrams", dt);
            return false;
        }
        viewTagToWidget.clear();
        UMLDoc *umlDoc = UMLApp::app()->document();
        UMLFolder *rootFolder = parentPkg->asUMLFolder();
        UMLView *view = umlDoc->createDiagram(rootFolder, dt, name, id);
        PetalNode *items = node->findAttribute(QStringLiteral("items")).node;
        if (items == 0) {
            logError2("Import_Rose::umbrellify: diagramType %1 object %2 attribute 'items' not found",
                      dt, objType);
            return false;
        }
        PetalNode::NameValueList atts = items->attributes();
        UMLWidgetList swimlanes;  // auxiliary list for adjusting the final swimlane height to contain all widgets
        qreal maxY = 0.0;
        for (int i = 0; i < atts.count(); ++i) {
            PetalNode *attr = atts[i].second.node;
            QStringList args = attr->initialArgs();
            QString objType = args[0];
            QString name = clean(args[1]);
            QString widgetId = attr->viewTag();
            qreal width = 0.0;
            qreal height = 0.0;
            UMLWidget *w = 0;
            if (objType == QStringLiteral("CategoryView")
                                || objType == QStringLiteral("ClassView")
                                || objType == QStringLiteral("UseCaseView")
                                || objType == QStringLiteral("ModView")
                                || objType == QStringLiteral("SubSysView")) {
                QString objID = quidu(attr);
                UMLObject *o = umlDoc->findObjectById(Uml::ID::fromString(objID));
                if (!o) {
                    logError2("Import_Rose::umbrellify: %1 object with quidu %2 not found",
                              objType, objID);
                    continue;
                }
                w = Widget_Factory::createWidget(view->umlScene(), o);
                width = fetchDouble(attr, QStringLiteral("width"));
                if (width == 0) {
                    // Set default value to get it displayed at all.
                    width = name.length() * 12;  // to be verified
                    logError3("Import_Rose::umbrellify: %1 %2: no width found, using default %3",
                              objType, name, width);
                }
                height = fetchDouble(attr, QStringLiteral("height"));
                if (height == 0) {
                    // Set default value to get it displayed at all.
                    height = 100.0;
                    logError3("Import_Rose::umbrellify: %1 %2: no height found, using default %3",
                              objType, name, height);
                }
                w->setSize(width, height);
                const QString icon = clean(attr->findAttribute(QStringLiteral("icon")).string);
                if (icon == QStringLiteral("Interface")) {
                    ClassifierWidget *cw = dynamic_cast<ClassifierWidget*>(w);
                    if (cw)
                        cw->setVisualProperty(ClassifierWidget::DrawAsCircle);
                    else
                        logDebug2("Import_Rose::umbrellify(%1) : Setting %2 to circle is not yet implemented",
                                  objType, name);
                }
            } else if (objType == QStringLiteral("InheritView") ||
                       objType == QStringLiteral("RealizeView") ||
                       objType == QStringLiteral("UsesView")) {
                QString idStr = quidu(attr);
                Uml::ID::Type assocID = Uml::ID::fromString(idStr);
                if (assocID == Uml::ID::None) {
                    logError2("Import_Rose::umbrellify: %1 has illegal id %2", objType, idStr);
                } else {
                    UMLObject *o = umlDoc->findObjectById(assocID);
                    if (o) {
                        if (o->baseType() != UMLObject::ot_Association) {
                            logError3("Import_Rose::umbrellify: %1 (%2) has wrong type %3",
                                     objType, idStr, o->baseType());
                        } else {
                            Uml::AssociationType::Enum t = Uml::AssociationType::Generalization;
                            if (objType == QStringLiteral("RealizeView"))
                                t = Uml::AssociationType::Realization;
                            else if (objType == QStringLiteral("UsesView"))
                                t = Uml::AssociationType::Dependency;
                            handleAssocView(attr, atts, t, view, o);
                        }
                    } else {
                        logError2("Import_Rose::umbrellify: %1 with id %2 not found", objType, idStr);
                    }
                }
            } else if (objType == QStringLiteral("AssociationViewNew")) {
                QString idStr = quidu(attr);
                Uml::ID::Type assocID = Uml::ID::fromString(idStr);
                if (assocID == Uml::ID::None) {
                    logError2("Import_Rose::umbrellify: %1 has illegal id %2", objType, idStr);
                } else {
                    UMLObject *o = umlDoc->findObjectById(assocID);
                    Uml::AssociationType::Enum t = Uml::AssociationType::UniAssociation;
                    // @todo check if we need to change `t' to a different type for certain associations
                    handleAssocView(attr, atts, t, view, o);
                }
            } else if (objType == QStringLiteral("AttachView")) {
                QString idStr = quidu(attr);
                Uml::ID::Type assocID = Uml::ID::fromString(idStr);
                if (assocID == Uml::ID::None) {
                    logError1("Import_Rose::umbrellify: AttachView has illegal id %1", idStr);
                } else {
                    handleAssocView(attr, atts, Uml::AssociationType::Anchor, view);
                }
            } else if (objType == QStringLiteral("TransView")) {
                Uml::AssociationType::Enum assocType = Uml::AssociationType::UniAssociation;
                if (dt == Uml::DiagramType::Activity)
                    assocType = Uml::AssociationType::Activity;
                else if (dt == Uml::DiagramType::State)
                    assocType = Uml::AssociationType::State;
                const QString clientTag = attr->findAttribute(QStringLiteral("client")).string;
                if (clientTag.isEmpty()) {
                    logError1("umbrellify: TransView %1 is missing client viewTag", attr->viewTag());
                    continue;
                }
                UMLWidget *clientW = viewTagToWidget[clientTag];
                if (clientW == nullptr) {
                    logError2("umbrellify TransView %1 : clientTag %2 not found in viewTagToWidget",
                              attr->viewTag(), clientTag);
                    continue;
                }
                const QString supplierTag = attr->findAttribute(QStringLiteral("supplier")).string;
                if (supplierTag.isEmpty()) {
                    logError1("umbrellify: TransView %1 is missing supplier viewTag", attr->viewTag());
                    continue;
                }
                UMLWidget *supplierW = viewTagToWidget[supplierTag];
                if (supplierW == nullptr) {
                    logError2("umbrellify TransView %1 : supplierTag %2 not found in viewTagToWidget",
                              attr->viewTag(), supplierTag);
                    continue;
                }
                // (object TransView "" @19
                //     label   (object SegLabel @20
                //         Parent_View     @19
                //         location       (1788, 1438)
                //         anchor_loc     1
                //         nlines         1
                //         max_width      597
                //         justify        0
                //         label          "END / cntx.sayGoodBye"
                //         pctDist        0.523534
                //         height         47
                //         orientation     0)
                //     stereotype     TRUE
                //     quidu          "3D6780CD005D"
                //     client         @4
                //     supplier       @3
                //     vertices       (list Points
                //         (1921, 801)
                //         (1921, 1391)
                //         (1130, 1391))
                //     line_style     3
                //     origin_attachment     (1921, 801)
                //     terminal_attachment     (1130, 1391)
                //     x_offset       FALSE)
                //
                AssociationWidget * aw = AssociationWidget::create
                       (view->umlScene(), clientW, assocType, supplierW);
                view->umlScene()->addAssociation(aw);
                continue;
            } else if (objType == QStringLiteral("NoteView")) {
                w = new NoteWidget(view->umlScene(), NoteWidget::Normal);
                width = fetchDouble(attr, QStringLiteral("width"));
                height = fetchDouble(attr, QStringLiteral("height"));
                if (width > 0 && height > 0)
                    w->setSize(width, height);
                PetalNode *lblNode = attr->findAttribute(QStringLiteral("label")).node;
                if (lblNode) {
                    QString label = clean(lblNode->findAttribute(QStringLiteral("label")).string);
                    w->setDocumentation(label);
                }
                // Add an artificial "quidu" attribute onto `attr' because
                // handling of AttachView depends on it:
                PetalNode::NameValueList innerAtts = attr->attributes();
                PetalNode::StringOrNode value;
                value.string = Uml::ID::toString(w->id());
                PetalNode::NameValue synthQuidu(QStringLiteral("quidu"), value);
                innerAtts.append(synthQuidu);
                attr->setAttributes(innerAtts);
            } else if (objType == QStringLiteral("Label")) {
                QString label = clean(attr->findAttribute(QStringLiteral("label")).string);
                w = new FloatingTextWidget(view->umlScene(), Uml::TextRole::Floating, label);
                int nlines = fetchInt(attr, QStringLiteral("nlines"));
                width = fetchDouble(attr, QStringLiteral("max_width"));
                height = nlines * 12;  // TODO check line height
                w->setSize(width, height);
            } else if (objType == QStringLiteral("Swimlane")) {
                QString idStr = quidu(attr);
                Uml::ID::Type id = Uml::ID::fromString(idStr);
                width = fetchDouble(attr, QStringLiteral("width"));
                height = 880;  // initial guess, will be adjusted after loading (see UMLWidgetList swimlanes)
                //      (object Swimlane "Operator" @627
                //          location    (2716, 0)   // optional, if missing it's the first (leftmost)
                //          line_color  3342489
                //          quidu       "3ADE7D250155"
                //          width       980)
                w = new BoxWidget(view->umlScene(), id);
                w->setSize(width, height);
                QPointF pos = fetchLocation(attr, 0.0, 0.0);  // use 0 for width and height
                if (pos.isNull()) {
                    pos = QPointF(1.0, 1.0);
                } else {
                    pos.setX(pos.x() * Rose2Qt);
                    pos.setY(1.0);
                }
                w->setPos(pos);
                logDebug4("umbrellify(Swimlane %1) : x=%2, width=%3, m_attributes size=%4",
                          name, pos.x(), width, attr->attributes().size());
                view->umlScene()->setupNewWidget(w, false);
                swimlanes.append(w);

                // Create FloatingText for the swimlane title.
                qreal xOffset = (width - (name.length() * 12)) / 2.0;
                QPointF textPos(pos.x() + xOffset, 12.0);
                logDebug5("umbrellify(Swimlane %1) : x=%2 , y=%3 , w=%4 , h=%5",
                          name, textPos.x(), textPos.y(), width, height);
                UMLWidget *textW = new FloatingTextWidget(view->umlScene(), Uml::TextRole::Floating, name);
                textW->setSize(4.0 + (name.length() * 12.0), 14.0);
                textW->setPos(textPos);
                logDebug3("umbrellify(Swimlane %1) : textW width=%2 , height=%3", name, textW->width(), textW->height());
                view->umlScene()->setupNewWidget(textW, false);
                w = nullptr;   // Setup of `w` is all done, setting it to null to inform the code below.
            } else if (objType == QStringLiteral("ActivityStateView")) {
                QString idStr = quidu(attr);
                Uml::ID::Type id = Uml::ID::fromString(idStr);
                w = new ActivityWidget(view->umlScene(), ActivityWidget::ActivityType::Normal, id);
                width  = fetchDouble(attr, QStringLiteral("width"));
                if (qFuzzyIsNull(width)) {
                    width = 50.0 + (name.length() * 12);
                }
                height = fetchDouble(attr, QStringLiteral("height"));
                if (qFuzzyIsNull(height)) {
                    height = 50.0;
                }
                w->setSize(width, height);
                w->setName(name);
                //      (object ActivityStateView "Extract image" @628
                //          Parent_View         @627
                //          location    (501, 659)
                //          font        (object Font
                //              size            10
                //              ........
                //              default_color   TRUE)
                //          label       (object ItemLabel
                //              Parent_View     @628
                //              location        (501, 653)
                //              fill_color      13434879
                //              anchor_loc      1
                //              nlines          2
                //              max_width       293
                //              justify         0
                //              label           "Extract image")
                //          icon_style  "Icon"
                //          line_color  3342489
                //          fill_color  13434879
                //          quidu       "3ADE77DE00BD"
                //          width       411
                //          height      124
                //          annotation  1
                //          autoResize  TRUE)
            } else if (objType == QStringLiteral("StateView")) {
                QString idStr = quidu(attr);
                Uml::ID::Type id = Uml::ID::fromString(idStr);
                const bool isInitial = (name == QStringLiteral("StartState"));
                const bool isEnd     = (name == QStringLiteral("EndState"));
                if (dt == Uml::DiagramType::Activity) {
                    ActivityWidget::ActivityType type = ActivityWidget::ActivityType::Normal;
                    if (isInitial)
                        type = ActivityWidget::ActivityType::Initial;
                    else if (isEnd)
                        type = ActivityWidget::ActivityType::End;
                    w = new ActivityWidget(view->umlScene(), type, id);
                } else {
                    // dt == Uml::DiagramType::State
                    StateWidget::StateType type = StateWidget::StateType::Normal;
                    if (isInitial)
                        type = StateWidget::StateType::Initial;
                    else if (isEnd)
                        type = StateWidget::StateType::End;
                    w = new StateWidget(view->umlScene(), type, id);
                }
                width = w->width();
                height = w->height();
                const bool isNormal = (!isInitial && !isEnd);
                if (isNormal) {
                    PetalNode *lblNode = attr->findAttribute(QStringLiteral("label")).node;
                    if (lblNode) {
                        name = clean(lblNode->findAttribute(QStringLiteral("label")).string);
                    }
                    width  = fetchDouble(attr, QStringLiteral("width"));
                    if (qFuzzyIsNull(width)) {
                        width = 50.0 + (name.length() * 12);
                        logDebug3("umbrellify(%1 %2) : width not given, defaulting to %3",
                                  objType, name, width);
                    }
                    height = fetchDouble(attr, QStringLiteral("height"));
                    if (qFuzzyIsNull(height)) {
                        height = 50.0;
                        logDebug3("umbrellify(%1 %2) : height not given, defaulting to %3",
                                  objType, name, height);
                    }
                    if (!qFuzzyIsNull(width) && !qFuzzyIsNull(height))
                        w->setSize(width, height);
                    w->setName(name);
                }
            } else if (objType == QStringLiteral("ObjectView")) {
                QString idStr = quidu(attr);
                Uml::ID::Type id = Uml::ID::fromString(idStr);
                width = fetchDouble(attr, QStringLiteral("width"));
                height = fetchDouble(attr, QStringLiteral("height"));
                if (name.isEmpty()) {
                    PetalNode *lblNode = attr->findAttribute(QStringLiteral("label")).node;
                    if (lblNode) {
                        name = clean(lblNode->findAttribute(QStringLiteral("label")).string);
                    }
                }
                w = new ObjectNodeWidget(view->umlScene(), ObjectNodeWidget::ObjectNodeType::Normal, id);
                const int nameWidth = (name.length() + 1) * 12.0;   // to be verified
                if (width < nameWidth)
                    width = nameWidth;
                if (height < 12.0)
                    height = 12.0;
                logDebug4("umbrellify(%1 %2) : width=%3, height=%4", objType, name, width, height);
                w->setSize(width, height);
                w->setName(name);
                w->setInstanceName(name);
            } else if (objType == QStringLiteral("DecisionView")) {
                // (object DecisionView "Check image quality" @629
                //      Parent_View         @3997
                //      location    (1008, 1504)
                //      icon_style  "Icon"
                //      line_color  3342489
                //      fill_color  13434879
                //      quidu       "4385B29C023B"
                //      autoResize  TRUE)
                QString idStr = quidu(attr);
                Uml::ID::Type id = Uml::ID::fromString(idStr);
                w = new ActivityWidget(view->umlScene(), ActivityWidget::ActivityType::Branch, id);
                width = height = 40.0;
                w->setSize(width, height);
                if (!name.isEmpty() && !name.startsWith(QStringLiteral("$UNNAMED$")))
                    w->setName(name);
            } else if (objType == QStringLiteral("SynchronizationView")) {
                Qt::Orientation ori = Qt::Horizontal;
                QString sync_is_horizontal = attr->findAttribute(QStringLiteral("sync_is_horizontal")).string;
                if (sync_is_horizontal == QStringLiteral("FALSE"))
                    ori = Qt::Vertical;
                //  (object SynchronizationView "$UNNAMED$193" @656
                //      location    (500, 843)
                //      font        (object Font
                //          size            10
                //          face            "Arial"
                //          bold            FALSE
                //          italics         FALSE
                //          underline       FALSE
                //          strike          FALSE
                //          color           0
                //          default_color   TRUE)
                //      label       (object ItemLabel
                //          Parent_View     @656
                //          location        (662, 772)
                //          nlines          2
                //          max_width       300
                //          label           "")
                //      icon_style  "Icon"
                //      line_color  3342489
                //      quidu       "3ADE7F9500D1"
                //      autoResize  TRUE
                //      sync_flow_direction         1
                //      sync_is_horizontal  TRUE)
                w = new ForkJoinWidget(view->umlScene(), ori, id);
                width = w->width();
                height = w->height();
            } else {
                // CHECK:  ConnectionView DataFlowView DependencyView InterfaceView
                //         InterObjView LinkView
                logDebug2("umbrellify(%1) unsupported object type %2", name, objType);
            }
            if (!w)
                continue;

            QPointF pos = fetchLocation(attr, width, height);
            if (!pos.isNull()) {
                w->setPos(pos);
                if (pos.y() + height > maxY)
                    maxY = pos.y() + height;
            }

            QString line_color = attr->findAttribute(QStringLiteral("line_color")).string;
            if (!line_color.isEmpty()) {
                unsigned int lineColor = line_color.toUInt();
                const QString hexColor = QString::number(lineColor, 16);
                QString hexRGB = QString(QStringLiteral("%1")).arg(hexColor, 6, QLatin1Char('0'));
                logDebug3("%1 %2 : lineColor %3", objType, name, hexRGB);
                QColor c(QLatin1Char('#') + hexRGB);
                w->setLineColorCmd(c);
            }
            QString fill_color = attr->findAttribute(QStringLiteral("fill_color")).string;
            if (fill_color.isEmpty()) {
                w->setUseFillColor(false);
            } else {
                unsigned int fillColor = fill_color.toUInt();
                const QString hexColor = QString::number(fillColor, 16);
                QString hexRGB = QString(QStringLiteral("%1")).arg(hexColor, 6, QLatin1Char('0'));
                logDebug3("%1 %2 : fillColor %3", objType, name, hexRGB);
                QColor f(QLatin1Char('#') + hexRGB);
                w->setFillColorCmd(f);
            }

            if (!widgetId.isEmpty())
               w->setLocalID(Uml::ID::fromString(widgetId));
            view->umlScene()->setupNewWidget(w, pos.isNull());
            viewTagToWidget[widgetId] = w;
        }
        // Adjust swimlane Y extent to encompass all widgets
        maxY += 20.0;  // add a bottom margin
        foreach (UMLWidget *bw, swimlanes) {
            if (maxY > bw->height()) {
                logDebug2("Adjusting swimlane %1 to height %2", bw->name(), maxY);
                QRectF rect = bw->rect();
                rect.setHeight(maxY);
                bw->setRect(rect);
            }
        }
    } else {
        logDebug2("umbrellify(%1): object type %2 is not yet implemented", name, objType);
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
    if (viewRoot == 0) {
        logDebug1("petaltree2uml importView: cannot find %1", rootName);
        return false;
    }
    if (viewRoot->name() != firstNodeName) {
        logError3("Import_Rose::importView %1 : expecting first node name %2, found: %3",
                  modelsName, firstNodeName, viewRoot->name());
        return false;
    }
    PetalNode *models = viewRoot->findAttribute(modelsName).node;
    if (models == 0) {
        logError2("Import_Rose::importView: cannot find %1 of %2", rootName, modelsName);
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

    // statemachine    (object State_Machine "State/Activity Model"
    PetalNode *statemachine = viewRoot->findAttribute(QStringLiteral("statemachine")).node;
    if (statemachine) {
        //    states      (list States
        //    partitions  (list Partitions
        //    objects     (list Objects
        // We don't decode these because Umbrello does not have structural elements for the
        // states; they are diagram objects only. Luckily, the widgets in Rose's activity
        // diagram save format duplicate all the structural information needed.
        //
        //    statediagrams       (list StateDiagrams
        PetalNode *statediagrams = statemachine->findAttribute(QStringLiteral("statediagrams")).node;
        PetalNode::NameValueList diagramList = statediagrams->attributes();
        for (int i = 0; i < diagramList.count(); ++i) {
            umbrellify(diagramList[i].second.node, parent);
        }
    }

    if (presentationsName.isEmpty())
        return true;

    PetalNode *presentations = viewRoot->findAttribute(presentationsName).node;
    if (presentations == 0) {
        logError2("Import_Rose::importView %1: cannot find %2", modelsName, presentationsName);
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
    if (root == 0) {
        logError0("petalTree2Uml: root is NULL");
        return 0;
    }
    UMLPackage *rootPkg = Model_Utils::rootPackage(parentPkg);
    if (rootPkg == 0) {
        logError0("petalTree2Uml: internal error - rootPkg is NULL");
        return 0;
    }
    UMLDoc *umlDoc = UMLApp::app()->document();
    Uml::ModelType::Enum mt = umlDoc->rootFolderType(rootPkg);
    QString modelsAttr(mt == Uml::ModelType::Component ? QStringLiteral("physical_models")
                                                       : QStringLiteral("logical_models"));
    PetalNode *models = root->findAttribute(modelsAttr).node;
    if (models == 0) {
        logError1("petalTree2Uml: cannot find %1", modelsAttr);
        return 0;
    }
    QStringList args = root->initialArgs();
    QString name = clean(args[1]);
    const Uml::ID::Type id = quid(root);
    UMLObject *o = Object_Factory::createUMLObject(UMLObject::ot_Folder, name, parentPkg, false);
    o->setID(id);
    parentPkg = o->asUMLPackage();
    PetalNode::NameValueList atts = models->attributes();
    for (int i = 0; i < atts.count(); ++i) {
        if (!umbrellify(atts[i].second.node, parentPkg)) {
            break;
        }
    }
    return parentPkg;
}

}  // namespace Import_Rose

