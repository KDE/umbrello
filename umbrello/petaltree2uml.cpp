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

namespace Import_Rose {

bool umbrellify(PetalNode *node, UMLPackage *parentPkg = NULL) {
    if (node == NULL) {
        kdError() << "umbrellify: node is NULL" << endl;
        return false;
    }
    QStringList args = node->initialArgs();
    QString objType = args[0];
    QString name = args[1];
    name.remove(QRegExp("\""));
    Uml::Object_Type ot;
    if (objType == "Class_Category") {
        ot = Uml::ot_Package;
        UMLObject *o = Import_Utils::createUMLObject(Uml::ot_Package, name, parentPkg);
        parentPkg = static_cast<UMLPackage*>(o);
        PetalNode *logical_models = node->findAttribute("logical_models").node;
        if (logical_models == NULL) {
            kdError() << "umbrellify: cannot find logical_models" << endl;
            return false;
        }
        PetalNode::NameValueList atts = logical_models->attributes();
        for (uint i = 0; i < atts.count(); i++) {
            umbrellify(atts[i].second.node, parentPkg);
        }
    } else if (objType == "Class") {
        UMLObject *o = Import_Utils::createUMLObject(Uml::ot_Class, name, parentPkg);
        UMLClassifier *c = static_cast<UMLClassifier*>(o);
        // .. to be continued: insert attributes, operations ..
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
    PetalNode::NameValueList atts = logical_models->attributes();
    for (uint i = 0; i < atts.count(); i++) {
        umbrellify(atts[i].second.node);
    }
    return true;
}

}  // namespace Import_Rose

