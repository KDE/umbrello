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
Uml::IDType quid(PetalNode *node) {
    QString quidStr = node->findAttribute("quid").string;
    if (quidStr.isEmpty())
        return Uml::id_None;
    quidStr.remove("\"");
    return STR2ID(quidStr);
}

/**
 * Extract the quidu attribute from a petal node.
 */
QString quidu(PetalNode *node) {
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
        PetalNode *class_attributes = node->findAttribute("class_attributes").node;
        if (class_attributes) {
            PetalNode::NameValueList attributeList = class_attributes->attributes();
            for (uint i = 0; i < attributeList.count(); i++) {
                PetalNode *attNode = attributeList[i].second.node;
                QStringList initialArgs = attNode->initialArgs();
                if (initialArgs[0] != "ClassAttribute") {
                    kdDebug() << "umbrellify(" << name << "): expecting ClassAttribute, "
                              << "found " << initialArgs[0] << endl;
                    continue;
                }
                UMLAttribute *att = new UMLAttribute(c);
                att->setName(clean(initialArgs[1]));
                att->setID(quid(attNode));
                QString type = clean(attNode->findAttribute("type").string);
                QString quidref = quidu(attNode);
                if (type.isEmpty()) {
                    if (quidref.isEmpty())
                        kdError() << "umbrellify(" << name << "): cannot find type of attribute "
                                  << att->getName() << endl;
                    else
                        att->setSecondaryId(quidref);
                } else {
                    UMLObject *o = Import_Utils::createUMLObject(typeToCreate(type), type);
                    if (!quidref.isEmpty())
                        o->setID(STR2ID(quidref));
                    att->setType(o);
                }
                QString vis = attNode->findAttribute("exportControl").string;
                if (vis != QString::null) {
                    Uml::Visibility v = Uml::Visibility::fromString(clean(vis.lower()));
                    att->setVisibility(v);
                }
                c->addAttribute(att);
            }
        }
        // insert operations
        PetalNode *operations = node->findAttribute("operations").node;
        if (operations) {
            PetalNode::NameValueList attributeList = operations->attributes();
            for (uint i = 0; i < attributeList.count(); i++) {
                PetalNode *opNode = attributeList[i].second.node;
                QStringList initialArgs = opNode->initialArgs();
                if (initialArgs[0] != "Operation") {
                    kdDebug() << "umbrellify(" << name << "): expecting Operation, "
                              << "found " << initialArgs[0] << endl;
                    continue;
                }
                UMLOperation *op = new UMLOperation(c);
                op->setName(clean(initialArgs[1]));
                op->setID(quid(opNode));
                QString type = clean(opNode->findAttribute("result").string);
                QString quidref = quidu(opNode);
                if (type.isEmpty()) {
                    if (quidref.isEmpty())
                        kdError() << "umbrellify(" << name << "): cannot find type of operation "
                                  << op->getName() << endl;
                    else
                        op->setSecondaryId(quidref);
                } else {
                    UMLObject *o = Import_Utils::createUMLObject(typeToCreate(type), type);
                    if (!quidref.isEmpty())
                        o->setID(STR2ID(quidref));
                    op->setType(o);
                }
                QString vis = opNode->findAttribute("exportControl").string;
                if (vis != QString::null) {
                    Uml::Visibility v = Uml::Visibility::fromString(clean(vis.lower()));
                    op->setVisibility(v);
                }
                c->addOperation(op);
            }
        }
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

