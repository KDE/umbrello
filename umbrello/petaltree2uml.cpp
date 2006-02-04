/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "petaltree2uml.h"
// qt/kde includes
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "petalnode.h"
#include "import_utils.h"
#include "package.h"
#include "classifier.h"
#include "attribute.h"
#include "operation.h"
#include "uml.h"
#include "umldoc.h"

namespace Import_Rose {

/**
 * Return the given string without surrounding quotation marks.
 */
QString clean(QString quotedStr) {
    if (quotedStr.isNull())
        return QString::null;
    return quotedStr.remove("\"");
}

/**
 * Extract the quid attribute from a petal node and return it as a Uml::IDType.
 */
Uml::IDType quid(const PetalNode *node) {
    QString quidStr = node->findAttribute("quid").string;
    if (quidStr.isEmpty())
        return Uml::id_None;
    quidStr.remove("\"");
    return STR2ID(quidStr);
}

/**
 * Extract the quidu attribute from a petal node.
 */
QString quidu(const PetalNode *node) {
    QString quiduStr = node->findAttribute("quidu").string;
    if (quiduStr.isEmpty())
        return QString::null;
    quiduStr.remove("\"");
    return quiduStr;
}

/**
 * Determine the model type corresponding to a name.
 * If the given name consists only of letters, digits, underscores, and
 * scope separators, then return Uml::ot_Class, else return Uml::ot_Datatype.
 */
Uml::Object_Type typeToCreate(QString name) {
    Uml::Object_Type t = Uml::ot_Datatype;
    if (name.contains(QRegExp("^[\\w:\\. ]*$")))
        t = Uml::ot_Class;
    return t;
}

/**
 * Transfer the Rose attribute "exportControl" to the Umbrello object given.
 *
 * @param from   Pointer to PetalNode from which to read the "exportControl" attribute
 * @param to     Pointer to UMLObject in which to set the Uml::Visibility
 */
void transferVisibility(const PetalNode *from, UMLObject *to) {
    QString vis = from->findAttribute("exportControl").string;
    if (vis != QString::null) {
        Uml::Visibility v = Uml::Visibility::fromString(clean(vis.lower()));
        to->setVisibility(v);
    }
}

struct PropertyNames {
    const QString m_attributeTag, m_elementName, m_itemTypeDesignator;
    PropertyNames(const QString attributeTag,
                  const QString elementName,
                  const QString itemTypeDesignator) :
        m_attributeTag(attributeTag),
        m_elementName(elementName),
        m_itemTypeDesignator(itemTypeDesignator) {
    }
    virtual ~PropertyNames() {}
};

template<const PropertyNames *pn>
class ClassifierListReader {
public:
    ClassifierListReader() {}
    virtual ~ClassifierListReader() {}
    virtual UMLClassifierListItem *createListItem() = 0;
    virtual void action(const PetalNode *node, UMLClassifierListItem *ucli) = 0;
    void read(const PetalNode *node, QString name) {
        PetalNode *attributes = node->findAttribute(pn->m_attributeTag).node;
        if (attributes == NULL) {
            kdDebug() << "umbrellify(" << name << "): no " << pn->m_attributeTag << " found"
                      << endl;
            return;
        }
        PetalNode::NameValueList attributeList = attributes->attributes();
        for (uint i = 0; i < attributeList.count(); i++) {
            PetalNode *attNode = attributeList[i].second.node;
            QStringList initialArgs = attNode->initialArgs();
            if (attNode->name() != pn->m_elementName) {
                kdDebug() << "umbrellify(" << name << "): expecting " << pn->m_elementName
                          << ", " << "found " << initialArgs[0] << endl;
                continue;
            }
            UMLClassifierListItem *item = createListItem();
            item->setName(clean(initialArgs[1]));
            item->setID(quid(attNode));
            QString type = clean(attNode->findAttribute(pn->m_itemTypeDesignator).string);
            QString quidref = quidu(attNode);
            if (type.isEmpty()) {
                if (quidref.isEmpty())
                    kdDebug() << "umbrellify(" << name << "): cannot find type of "
                              << item->getName() << endl;
                else
                    item->setSecondaryId(quidref);
            } else {
                UMLObject *o = Import_Utils::createUMLObject(typeToCreate(type), type);
                if (!quidref.isEmpty())
                    o->setID(STR2ID(quidref));
                item->setType(o);
            }
            transferVisibility(attNode, item);
            QString doc = attNode->findAttribute("documentation").string;
            if (! doc.isEmpty())
                item->setDoc(doc);
            action(attNode, item);
        }
    }
};

PropertyNames g_attPropNames("class_attributes", "ClassAttribute", "type");
typedef ClassifierListReader<&g_attPropNames> AttributesReaderBase;

class AttributesReader : public AttributesReaderBase {
public:
    AttributesReader(UMLClassifier *c) {
        m_classifier = c;
    }
    virtual ~AttributesReader() {}
    UMLClassifierListItem *createListItem() {
        return new UMLAttribute(m_classifier);
    }
    void action(const PetalNode *, UMLClassifierListItem *item) {
        m_classifier->addAttribute(static_cast<UMLAttribute*>(item));
    }
protected:
    UMLClassifier *m_classifier;
};

PropertyNames g_parmPropNames("parameters", "Parameter", "type");
typedef ClassifierListReader<&g_parmPropNames> ParametersReaderBase;

class ParametersReader : public ParametersReaderBase {
public:
    ParametersReader(UMLOperation *op) {
        m_operation = op;
    }
    virtual ~ParametersReader() {}
    UMLClassifierListItem *createListItem() {
        return new UMLAttribute(m_operation);
    }
    void action(const PetalNode *, UMLClassifierListItem *item) {
        m_operation->addParm(static_cast<UMLAttribute*>(item));
    }
protected:
    UMLOperation *m_operation;
};
 
PropertyNames g_opPropNames("operations", "Operation", "result");
typedef ClassifierListReader<&g_opPropNames> OperationsReaderBase;

class OperationsReader : public OperationsReaderBase {
public:
    OperationsReader(UMLClassifier *c) {
        m_classifier = c;
    }
    virtual ~OperationsReader() {}
    UMLClassifierListItem *createListItem() {
        return new UMLOperation(m_classifier);
    }
    void action(const PetalNode *node, UMLClassifierListItem *item) {
        UMLOperation *op = static_cast<UMLOperation*>(item);
        ParametersReader parmReader(op);
        parmReader.read(node, m_classifier->getName());
        m_classifier->addOperation(op);
    }
protected:
    UMLClassifier *m_classifier;
};

/**
 * Create an Umbrello object from a PetalNode.
 *
 * @return   True for success.
 *           Given a PetalNode for which the mapping to Umbrello is not yet
 *           implemented umbrellify() is a no-op but also returns true.
 */
bool umbrellify(PetalNode *node, UMLPackage *parentPkg = NULL) {
    if (node == NULL) {
        kdError() << "umbrellify: node is NULL" << endl;
        return false;
    }
    QStringList args = node->initialArgs();
    QString objType = args[0];
    QString name = clean(args[1]);
    Uml::IDType id = quid(node);
    if (objType == "Class_Category") {
        UMLObject *o = Import_Utils::createUMLObject(Uml::ot_Package, name, parentPkg);
        o->setID(id);
        PetalNode *logical_models = node->findAttribute("logical_models").node;
        if (logical_models == NULL) {
            kdError() << "umbrellify: cannot find logical_models" << endl;
            return false;
        }
        UMLPackage *localParent = static_cast<UMLPackage*>(o);
        PetalNode::NameValueList atts = logical_models->attributes();
        for (uint i = 0; i < atts.count(); i++) {
            umbrellify(atts[i].second.node, localParent);
        }
    } else if (objType == "Class") {
        UMLObject *o = Import_Utils::createUMLObject(Uml::ot_Class, name, parentPkg);
        o->setID(id);
        UMLClassifier *c = static_cast<UMLClassifier*>(o);
        // insert attributes
        AttributesReader attReader(c);
        attReader.read(node, c->getName());
        // insert operations
        OperationsReader opReader(c);
        opReader.read(node, c->getName());
    } else {
        kdDebug() << "umbrellify: object type " << objType
                  << " is not yet implemented" << endl;
    }
    return true;
}

bool petalTree2Uml(PetalNode *root) {
    if (root == NULL) {
        kdError() << "petalTree2Uml: root is NULL" << endl;
        return false;
    }
    if (root->name() != "Design") {
        kdError() << "petalTree2Uml: expecting root name Design" << endl;
        return false;
    }
    PetalNode *root_category = root->findAttribute("root_category").node;
    if (root_category == NULL) {
        kdError() << "petalTree2Uml: cannot find root_category" << endl;
        return false;
    }
    if (root_category->name() != "Class_Category") {
        kdError() << "petalTree2Uml: expecting root_category object Class_Category" << endl;
        return false;
    }
    PetalNode *logical_models = root_category->findAttribute("logical_models").node;
    if (logical_models == NULL) {
        kdError() << "petalTree2Uml: cannot find logical_models" << endl;
        return false;
    }
    Import_Utils::assignUniqueIdOnCreation(false);
    PetalNode::NameValueList atts = logical_models->attributes();
    for (uint i = 0; i < atts.count(); i++) {
        umbrellify(atts[i].second.node);
    }
    Import_Utils::assignUniqueIdOnCreation(true);
    UMLApp::app()->getDocument()->resolveTypes();
    return true;
}

}  // namespace Import_Rose

