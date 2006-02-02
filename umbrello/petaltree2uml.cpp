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
                att->setSecondaryId(clean(attNode->findAttribute("quidu").string));
                QString vis = attNode->findAttribute("exportControl").string;
                if (vis != QString::null) {
                    Uml::Visibility v = Uml::Visibility::fromString(clean(vis.lower()));
                    att->setVisibility(v);
                }
                c->addAttribute(att);
            }
        }
        // .. to be continued: insert operations ..
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

