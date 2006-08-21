/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                 *
 ***************************************************************************/

// own header
#include "folder.h"
// qt/kde includes
#include <kdebug.h>
#include <klocale.h>
// app includes
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "object_factory.h"
#include "model_utils.h"

UMLFolder::UMLFolder(const QString & name, Uml::IDType id)
        : UMLPackage(name, id) {
    init();
}

UMLFolder::~UMLFolder() {
}

void UMLFolder::init() {
    m_BaseType = Uml::ot_Folder;
    m_bPredefined = false;
    m_diagrams.setAutoDelete(true);
    UMLObject::setStereotype("folder");
}

UMLObject* UMLFolder::clone() const {
    UMLFolder *clone = new UMLFolder();
    UMLObject::copyInto(clone);
    return clone;
}

void UMLFolder::markPredefined() {
    m_bPredefined = true;
}

bool UMLFolder::isPredefined() {
    return m_bPredefined;
}

void UMLFolder::addView(UMLView *view) {
    m_diagrams.append(view);
}

    /**** From UMLListViewItem::saveToXMI :
        if (UMLListView::typeIsFolder(m_Type) && !m_FolderFile.isEmpty()) {
            itemElement.setAttribute( "external_file", m_FolderFile );
            if (saveSubmodelFiles) {
                folderFile.setName(umldoc->URL().directory(false) + m_FolderFile);
                if( !folderFile.open( IO_WriteOnly ) ) {
                    KMessageBox::error(0,
                                       i18n("There was a problem saving file: %1").arg(m_FolderFile),
                                       i18n("Save Error"));
                    // TODO:
                    // Resetting the m_FolderFile here is too late -
                    // the folderfile contents are already lost.
                    // The test for folder file writability needs to
                    // be done before the main model is saved so that
                    // if a folder file turns out not to be creatable,
                    // the folder can be internalized in time.
                    // ("Internalization": As a fallback to ensure that
                    // nothing is lost, the folder contents are saved
                    // in the main model file.)
                    m_FolderFile = QString::null;
                    m_Label.remove( QRegExp("\\s+\\(.*$") );
                    QListViewItem::setText(0, m_Label);
                } else {
                    saveExternalFolder = true;
                }
            }
        }
     *****************/

void UMLFolder::setFolderFile(QString fileName) {
    m_folderFile = fileName;
}

QString UMLFolder::getFolderFile() {
    return m_folderFile;
}

void UMLFolder::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement folderElement;
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    QString elementName("UML:Package");
    for (int i = 0; i < Uml::N_MODELTYPES; i++) {
        if (this == umldoc->getRootFolder((Uml::Model_Type)i)) {
            elementName = "UML:Model";
            break;
        }
    }
    folderElement = UMLObject::save(elementName, qDoc);
    QDomElement extension = qDoc.createElement("XMI.extension");
    extension.setAttribute("xmi.extender", "umbrello");

    if (!m_folderFile.isEmpty()) {
        QDomElement fileElement = qDoc.createElement("external_file");
        fileElement.setAttribute("name", m_folderFile);
        extension.appendChild(fileElement);
    } else {
        QDomElement ownedElement = qDoc.createElement( "UML:Namespace.ownedElement" );
        UMLObject *obj;
        // Save contained objects if any.
        for (UMLObjectListIt oit(m_objects); (obj = oit.current()) != NULL; ++oit)
            obj->saveToXMI (qDoc, ownedElement);
        // Save asscociations if any.
        for (UMLObjectListIt ait(m_List); (obj = ait.current()) != NULL; ++ait)
            obj->saveToXMI (qDoc, ownedElement);
        folderElement.appendChild(ownedElement);
        // Save diagrams to `extension'.
        //........
    }
    folderElement.appendChild(extension);
    qElement.appendChild(folderElement);
}

bool UMLFolder::load(QDomElement& element) {
    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement tempElement = node.toElement();
        QString type = tempElement.tagName();
        if (Model_Utils::isCommonXMIAttribute(type))
            continue;
        if (Uml::tagEq(type, "Namespace.ownedElement") ||
                Uml::tagEq(type, "Namespace.contents")) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore these tags are not further interpreted.
            if (! load(tempElement))
                return false;
            continue;
        }
        QString stereoID = tempElement.attribute("stereotype", "");
        UMLObject *pObject = Object_Factory::makeObjectFromXMI(type, stereoID);
        if (!pObject) {
            kdWarning() << "UMLFolder::load: "
                        << "Unknown type of umlobject to create: "
                        << type << endl;
            continue;
        }
        pObject->setUMLPackage(this);
        if (pObject->loadFromXMI(tempElement)) {
            addObject(pObject);
        } else {
            delete pObject;
        }
    }
    return true;
}

#include "folder.moc"
