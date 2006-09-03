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
#include "umllistview.h"
#include "optionstate.h"
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

void UMLFolder::removeView(UMLView *view) {
    // m_diagrams is set to autodelete!
    m_diagrams.remove(view);
}

void UMLFolder::appendViews(UMLViewList& viewList, bool includeNested) {
    if (includeNested) {
        UMLObject *o;
        for (UMLObjectListIt oit(m_objects); (o = oit.current()) != NULL; ++oit) {
            if (o->getBaseType() == Uml::ot_Folder) {
                UMLFolder *f = static_cast<UMLFolder*>(o);
                f->appendViews(viewList);
            }
        }
    }
    UMLView *v;
    for (UMLViewListIt vit(m_diagrams); (v = vit.current()) != NULL; ++vit)
        viewList.append(v);
}

void UMLFolder::activateViews() {
    UMLView *v;
    for (UMLViewListIt vit(m_diagrams); (v = vit.current()) != NULL; ++vit)
        v->activateAfterLoad();
    // Make sure we have a treeview item for each diagram.
    // It may happen that we are missing them after switching off tabbed widgets.
    Settings::OptionState optionState = Settings::getOptionState();
    if (optionState.generalState.tabdiagrams)
        return;
    UMLListView *lv = UMLApp::app()->getListView();
    for (UMLViewListIt it(m_diagrams); (v = it.current()) != NULL; ++it) {
        if (lv->findItem(v->getID()) != NULL)
            continue;
        lv->createDiagramItem(v);
    }
}

UMLView *UMLFolder::findView(Uml::IDType id) {
    UMLView *v = NULL;
    for (UMLViewListIt vit(m_diagrams); (v = vit.current()) != NULL; ++vit) {
        if (v->getID() == id)
            return v;
    }
    UMLObject *o;
    for (UMLObjectListIt oit(m_objects); (o = oit.current()) != NULL; ++oit) {
        if (o->getBaseType() != Uml::ot_Folder)
            continue;
        UMLFolder *f = static_cast<UMLFolder*>(o);
        v = f->findView(id);
        if (v)
            break;
    }
    return v;
}

UMLView *UMLFolder::findView(Uml::Diagram_Type type, const QString &name, bool searchAllScopes) {
    UMLView *v = NULL;
    for (UMLViewListIt vit(m_diagrams); (v = vit.current()) != NULL; ++vit) {
        if (v->getType() == type && v->getName() == name)
            return v;
    }
    if (searchAllScopes) {
        UMLObject *o;
        for (UMLObjectListIt oit(m_objects); (o = oit.current()) != NULL; ++oit) {
            if (o->getBaseType() != Uml::ot_Folder)
                continue;
            UMLFolder *f = static_cast<UMLFolder*>(o);
            v = f->findView(type, name, searchAllScopes);
            if (v)
                break;
        }
    }
    return v;
}

void UMLFolder::setViewOptions(const Settings::OptionState& optionState) {
    // for each view update settings
    UMLView *v;
    for (UMLViewListIt vit(m_diagrams); (v = vit.current()) != NULL; ++vit)
        v->setOptionState(optionState);
}

void UMLFolder::removeAllViews() {
    UMLObject *o;
    for (UMLObjectListIt oit(m_objects); (o = oit.current()) != NULL; ++oit) {
        if (o->getBaseType() != Uml::ot_Folder)
            continue;
        UMLFolder *f = static_cast<UMLFolder*>(o);
        f->removeAllViews();
    }
    UMLView *v = NULL;
    for (UMLViewListIt vit(m_diagrams); (v = vit.current()) != NULL; ++vit) {
        // TODO ------------------ check this code - bad: calling back to UMLDoc::removeView()
        v->removeAllAssociations(); // note : It may not be apparent, but when we remove all associations
        // from a view, it also causes any UMLAssociations that lack parent
        // association widgets (but once had them) to remove themselves from
        // this document.
        UMLApp::app()->getDocument()->removeView(v, false);
    }
    m_diagrams.clear();
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
        QDomElement ownedElement = qDoc.createElement("UML:Namespace.ownedElement");
        UMLObject *obj;
        // Save contained objects if any.
        for (UMLObjectListIt oit(m_objects); (obj = oit.current()) != NULL; ++oit)
            obj->saveToXMI (qDoc, ownedElement);
        // Save asscociations if any.
        for (UMLObjectListIt ait(m_List); (obj = ait.current()) != NULL; ++ait)
            obj->saveToXMI (qDoc, ownedElement);
        folderElement.appendChild(ownedElement);
        // Save diagrams to `extension'.
        if (m_diagrams.count()) {
            QDomElement diagramsElement = qDoc.createElement("diagrams");
            UMLView *pView;
            for (UMLViewListIt vit(m_diagrams); (pView = vit.current()) != NULL; ++vit) {
                if ( // saveSubmodelFiles &&
                    pView->isSavedInSeparateFile())
                    continue;
                pView->saveToXMI(qDoc, diagramsElement);
            }
            extension.appendChild( diagramsElement );
        }
    }
    folderElement.appendChild(extension);
    qElement.appendChild(folderElement);
}

bool UMLFolder::loadDiagramsFromXMI(QDomNode& diagrams) {
    const Settings::OptionState optionState = Settings::getOptionState();
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    bool totalSuccess = true;
    for (QDomElement diagram = diagrams.toElement(); !diagram.isNull();
         diagrams = diagrams.nextSibling(), diagram = diagrams.toElement()) {
        QString tag = diagram.tagName();
        if (tag != "diagram") {
            kdDebug() << "UMLFolder::loadDiagramsFromXMI: ignoring "
                << tag << " in <diagrams>" << endl;
            continue;
        }
        UMLView * pView = new UMLView(this);
        pView->setOptionState(optionState);
        if (pView->loadFromXMI(diagram)) {
            pView->hide();
            umldoc->addView(pView);
        } else {
            delete pView;
            totalSuccess = false;
        }
    }
    return totalSuccess;
}

bool UMLFolder::load(QDomElement& element) {
    bool totalSuccess = true;
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
        } else if (type == "XMI.extension") {
            for (QDomNode xtnode = node.firstChild(); !xtnode.isNull();
                                              xtnode = xtnode.nextSibling()) {
                QDomElement el = xtnode.toElement();
                if (el.tagName() != "diagrams") {
                    kdDebug() << "ignoring XMI.extension " << el.tagName() << endl;
                    continue;
                }
                QDomNode diagramNode = xtnode.firstChild();
                if (!loadDiagramsFromXMI(diagramNode))
                    totalSuccess = false;
            }
            continue;
        }
        // Do not re-create the predefined Datatypes folder in the Logical View,
        // it already exists.
        UMLDoc *umldoc = UMLApp::app()->getDocument();
        UMLFolder *logicalView = umldoc->getRootFolder(Uml::mt_Logical);
        if (this == logicalView && Uml::tagEq(type, "Package")) {
            QString thisName = tempElement.attribute("name", "");
            if (thisName == umldoc->datatypeFolderName()) {
                UMLFolder *datatypeFolder = umldoc->getDatatypeFolder();
                if (!datatypeFolder->loadFromXMI(tempElement))
                    totalSuccess = false;
                continue;
            }
        }
        QString stereoID = tempElement.attribute("stereotype", "");
        UMLObject *pObject = Object_Factory::makeObjectFromXMI(type, stereoID);
        if (!pObject) {
            kdWarning() << "UMLFolder::load: "
                << "Unknown type of umlobject to create: " << type << endl;
            continue;
        }
        pObject->setUMLPackage(this);
        if (!pObject->loadFromXMI(tempElement)) {
            removeObject(pObject);
            delete pObject;
            totalSuccess = false;
        }
    }
    return totalSuccess;
}

#include "folder.moc"
