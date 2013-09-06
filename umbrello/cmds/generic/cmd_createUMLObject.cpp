/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_createUMLObject.h"

// app includes
#include "debug_utils.h"
#include "object_factory.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "widget_factory.h"

// kde includes
#include <klocale.h>

namespace Uml
{

    /**
     * Constructor.
     */
    CmdCreateUMLObject::CmdCreateUMLObject(UMLObject* o)
      : QUndoCommand(),
        m_obj(o)
    {
        if (m_obj) {
            setText(i18n("Create UML object : %1", m_obj->fullyQualifiedName()));
            m_package = m_obj->umlPackage();
            m_type    = m_obj->baseType();
            m_name    = m_obj->name();
            m_element.clear();
        }
        else {
            setText(i18n("Create UML object : CANNOT DO THAT, OBJECT IS NULL!"));
        }
    }

    /**
     *  Destructor.
     */
    CmdCreateUMLObject::~CmdCreateUMLObject()
    {
    }

    /**
     * Create the UMLObject.
     */
    void CmdCreateUMLObject::redo()
    {
        bool success = true;
//:TODO: unfinished idea for preventing crash because of invalid pointers
//        if (!m_element.isNull()) {
//            m_obj = Object_Factory::createUMLObject(m_type, m_name, m_package, false);
//            success = m_obj->loadFromXMI(m_element);
//            if (success) {
//                uDebug() << m_obj;
//                UMLScene* scene = UMLApp::app()->currentView()->umlScene();
//                if (scene) {
//                    UMLWidget* widget = scene->findWidget(m_obj->id());
//                    if (!widget) {
//                        UMLWidget* newWidget = Widget_Factory::createWidget(scene, m_obj);
//                        if (newWidget) {
//                            scene->setupNewWidget(newWidget);  // vielleicht nicht so gut - code wie bei UMLScene::slotObjectCreated
//                        }
//                    }
//                }
//                else {
//                    uDebug() << "There is no scene!";
//                }
//            }
//            else {
//                uDebug() << "UMLObject was not loaded from XMI structure!";
//            }
//        }

        if (success) {
            // This object was removed from it's package when it was deleted
            // so add it back to it's package (if it belonged to one)
            if (m_package) {
                // add this object to its parent package
                m_package->addObject(m_obj);

            } else {
                // object does not belong to any package
            }

            UMLDoc *doc = UMLApp::app()->document();
            doc->signalUMLObjectCreated(m_obj);
        }
    }

    /**
     * Suppress the UMLObject.
     */
    void CmdCreateUMLObject::undo()
    {
//:TODO: commented for preventing crash because of invalid pointers
//        QDomDocument qDoc;
//        m_obj->saveToXMI(qDoc, m_element);

//        UMLDoc *doc = UMLApp::app()->document();
//        doc->removeUMLObject(m_obj);
    }

}
