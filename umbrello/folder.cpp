/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "folder.h"

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "optionstate.h"
#include "object_factory.h"
#include "model_utils.h"
#include "umlscene.h"

// kde includes
#include <klocale.h>
#include <kmessagebox.h>

// qt includes
#include <QtCore/QFile>

/**
 * Sets up a Folder.
 * @param name    The name of the Folder.
 * @param id      The unique id of the Folder. A new ID will be generated
 *                if this argument is left away.
 */
UMLFolder::UMLFolder(const QString & name, Uml::IDType id)
  : UMLPackage(name, id)
{
    m_BaseType = UMLObject::ot_Folder;
    UMLObject::setStereotype("folder");
}

/**
 * Empty destructor.
 */
UMLFolder::~UMLFolder()
{
    qDeleteAll(m_diagrams);
    m_diagrams.clear();
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLFolder::clone() const
{
    UMLFolder *clone = new UMLFolder();
    UMLObject::copyInto(clone);
    return clone;
}

/**
 * Set the localized name of this folder.
 * This is set for the predefined root views (Logical,
 * UseCase, Component, Deployment, EntityRelationship,
 * and the Datatypes folder inside the Logical View.)
 */
void UMLFolder::setLocalName(const QString& localName)
{
    m_localName = localName;
}

/**
 * Return the localized name of this folder.
 * Only useful for the predefined root folders.
 */
QString UMLFolder::localName() const
{
    return m_localName;
}

/**
 * Add a view to the diagram list.
 */
void UMLFolder::addView(UMLView *view)
{
    m_diagrams.append(view);
}

/**
 * Remove a view from the diagram list.
 */
void UMLFolder::removeView(UMLView *view)
{
    m_diagrams.removeAll(view);
    delete view;
}

/**
 * Append the views in this folder to the given diagram list.
 * @param viewList       The UMLViewList to which to append the diagrams.
 * @param includeNested  Whether to include diagrams from nested folders
 *                       (default: true.)
 */
void UMLFolder::appendViews(UMLViewList& viewList, bool includeNested)
{
    if (includeNested) {
        foreach (UMLObject* o, m_objects ) {
            if (o->baseType() == UMLObject::ot_Folder) {
                UMLFolder *f = static_cast<UMLFolder*>(o);
                f->appendViews(viewList);
            }
        }
    }
    foreach (UMLView* v, m_diagrams) {
        viewList.append(v);
    }
}

/**
 * Acivate the views in this folder.
 * "Activation": Some widgets require adjustments after loading from file,
 * those are done here.
 */
void UMLFolder::activateViews()
{
    foreach (UMLObject* o, m_objects ) {
        if (o->baseType() == UMLObject::ot_Folder) {
            UMLFolder *f = static_cast<UMLFolder*>(o);
            f->activateViews();
        }
    }

    foreach (UMLView* v, m_diagrams) {
        v->umlScene()->activateAfterLoad();
    }
    // Make sure we have a treeview item for each diagram.
    // It may happen that we are missing them after switching off tabbed widgets.
    Settings::OptionState optionState = Settings::optionState();
    if (optionState.generalState.tabdiagrams) {
        return;
    }
    Model_Utils::treeViewAddViews(m_diagrams);
}

/**
 * Seek a view of the given ID in this folder.
 * @param id   ID of the view to find.
 * @return     Pointer to the view if found, NULL if no view found.
 */
UMLView *UMLFolder::findView(Uml::IDType id)
{
    foreach (UMLView* v, m_diagrams ) {
        if (v->umlScene()->getID() == id) {
            return v;
        }
    }

    UMLView* v = 0;
    foreach (UMLObject* o, m_objects ) {
        if (o->baseType() != UMLObject::ot_Folder) {
            continue;
        }
        UMLFolder *f = static_cast<UMLFolder*>(o);
        v = f->findView(id);
        if (v) {
            break;
        }
    }
    return v;
}

/**
 * Seek a view by the type and name given.
 * @param type              The type of view to find.
 * @param name              The name of the view to find.
 * @param searchAllScopes   Search in all subfolders (default: true.)
 * @return  Pointer to the view found, or NULL if not found.
 */
UMLView *UMLFolder::findView(Uml::DiagramType type, const QString &name, bool searchAllScopes)
{
    foreach (UMLView* v, m_diagrams) {
        if (v->umlScene()->type() == type && v->umlScene()->name() == name) {
            return v;
        }
    }

    UMLView* v = 0;
    if (searchAllScopes) {
        foreach (UMLObject* o, m_objects  ) {
            if (o->baseType() != UMLObject::ot_Folder) {
                continue;
            }
            UMLFolder *f = static_cast<UMLFolder*>(o);
            v = f->findView(type, name, searchAllScopes);
            if (v) {
                break;
            }
        }
    }
    return v;
}

/**
 * Set the options for the views in this folder.
 */
void UMLFolder::setViewOptions(const Settings::OptionState& optionState)
{
    // for each view update settings
    foreach (UMLView* v, m_diagrams ) {
        v->umlScene()->setOptionState(optionState);
    }
}

/**
 * Remove all views in this folder.
 */
void UMLFolder::removeAllViews()
{
    foreach (UMLObject* o, m_objects) {
        if (o->baseType() != UMLObject::ot_Folder)
            continue;
        UMLFolder *f = static_cast<UMLFolder*>(o);
        f->removeAllViews();
    }

    foreach (UMLView* v, m_diagrams ) {
        // TODO ------------------ check this code - bad: calling back to UMLDoc::removeView()
        v->umlScene()->removeAllAssociations(); // note : It may not be apparent, but when we remove all associations
        // from a view, it also causes any UMLAssociations that lack parent
        // association widgets (but once had them) to remove themselves from
        // this document.
        UMLApp::app()->document()->removeView(v, false);
    }

    qDeleteAll(m_diagrams);
    m_diagrams.clear();
}

/**
 * Set the folder file name for a separate submodel.
 */
void UMLFolder::setFolderFile(const QString& fileName)
{
    m_folderFile = fileName;
}

/**
 * Get the folder file name for a separate submodel.
 */
QString UMLFolder::folderFile() const
{
    return m_folderFile;
}

/**
 * Auxiliary to saveToXMI(): Save the contained objects and diagrams.
 * Can be used regardless of whether saving to the main model file
 * or to an external folder file (see m_folderFile.)
 */
void UMLFolder::saveContents(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement ownedElement = qDoc.createElement("UML:Namespace.ownedElement");
    UMLObject *obj = 0;
    // Save contained objects if any.
    for (UMLObjectListIt oit(m_objects); oit.hasNext(); ) {
        obj = oit.next();
        obj->saveToXMI (qDoc, ownedElement);
    }
    // Save asscociations if any.
    for (UMLObjectListIt ait(m_List); ait.hasNext(); ) {
        obj = ait.next();
        obj->saveToXMI (qDoc, ownedElement);
    }
    qElement.appendChild(ownedElement);
    // Save diagrams to `extension'.
    if (m_diagrams.count()) {
        QDomElement diagramsElement = qDoc.createElement("diagrams");

        foreach (UMLView* pView, m_diagrams ) {
            pView->umlScene()->saveToXMI(qDoc, diagramsElement);
        }
        QDomElement extension = qDoc.createElement("XMI.extension");
        extension.setAttribute("xmi.extender", "umbrello");
        extension.appendChild( diagramsElement );
        qElement.appendChild(extension);
    }
}

/**
 * Auxiliary to saveToXMI(): Creates a <UML:Model> element when saving
 * a predefined modelview, or a <UML:Package> element when saving a
 * user created folder. Invokes saveContents() with the newly created
 * element.
 */
void UMLFolder::save(QDomDocument& qDoc, QDomElement& qElement)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    QString elementName("UML:Package");
    const Uml::ModelType mt = umldoc->rootFolderType(this);
    if (mt != Uml::ModelType::N_MODELTYPES)
        elementName = "UML:Model";
    QDomElement folderElement = UMLObject::save(elementName, qDoc);
    saveContents(qDoc, folderElement);
    qElement.appendChild(folderElement);
}

/**
 * Creates a UML:Model or UML:Package element:
 * UML:Model is created for the predefined fixed folders,
 * UML:Package with stereotype "folder" is created for all else.
 */
void UMLFolder::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    if (m_folderFile.isEmpty()) {
        save(qDoc, qElement);
        return;
    }
    // See if we can create the external file.
    // If not then internalize the folder.
    UMLDoc *umldoc = UMLApp::app()->document();
    QString fileName = umldoc->url().directory() + '/' + m_folderFile;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        uError() << m_folderFile << ": "
            << "cannot create file, contents will be saved in main model file";
        m_folderFile.clear();
        save(qDoc, qElement);
        return;
    }
    // Okay, external file is writable.  Wrap up main file.
    QDomElement folderElement = UMLObject::save("UML:Package", qDoc);
    QDomElement extension = qDoc.createElement("XMI.extension");
    extension.setAttribute("xmi.extender", "umbrello");
    QDomElement fileElement = qDoc.createElement("external_file");
    fileElement.setAttribute("name", m_folderFile);
    extension.appendChild(fileElement);
    folderElement.appendChild(extension);
    qElement.appendChild(folderElement);

    // Save folder to external file.
    QDomDocument folderDoc;
    QDomElement folderRoot;
    QDomProcessingInstruction xmlHeading =
        folderDoc.createProcessingInstruction("xml",
                                              "version=\"1.0\" encoding=\"UTF-8\"");
    folderDoc.appendChild(xmlHeading);
    folderRoot = folderDoc.createElement("external_file");
    folderRoot.setAttribute("name", name());
    folderRoot.setAttribute("filename", m_folderFile);
    folderRoot.setAttribute("mainModel", umldoc->url().fileName());
    folderRoot.setAttribute("parentId", ID2STR(m_pUMLPackage->id()));
    folderRoot.setAttribute("parent", m_pUMLPackage->fullyQualifiedName("::", true));
    saveContents(folderDoc, folderRoot);
    folderDoc.appendChild(folderRoot);
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << folderDoc.toString();
    file.close();
}

/**
 * Auxiliary to load():
 * Load the diagrams from the "diagrams" in the <XMI.extension>
 */
bool UMLFolder::loadDiagramsFromXMI(QDomNode& diagrams)
{
    const Settings::OptionState optionState = Settings::optionState();
    UMLDoc *umldoc = UMLApp::app()->document();
    bool totalSuccess = true;
    for (QDomElement diagram = diagrams.toElement(); !diagram.isNull();
         diagrams = diagrams.nextSibling(), diagram = diagrams.toElement()) {
        QString tag = diagram.tagName();
        if (tag != "diagram") {
            uDebug() << "ignoring " << tag << " in <diagrams>";
            continue;
        }
        UMLView * pView = new UMLView(this);
        pView->umlScene()->setOptionState(optionState);
        if (pView->umlScene()->loadFromXMI(diagram)) {
            pView->hide();
            umldoc->addView(pView);
        } else {
            delete pView;
            totalSuccess = false;
        }
    }
    return totalSuccess;
}

/**
 * Folders in the listview can be marked such that their contents
 * are saved to a separate file.
 * This method loads the separate folder file.
 * CAVEAT: This is not XMI standard compliant.
 * If standard compliance is an issue then avoid folder files.
 * @param path  Fully qualified file name, i.e. absolute directory
 *              plus file name.
 * @return   True for success.
 */
bool UMLFolder::loadFolderFile(const QString& path)
{
    QFile file(path);
    if (!file.exists()) {
        KMessageBox::error(0, i18n("The folderfile %1 does not exist.", path), i18n("Load Error"));
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        KMessageBox::error(0, i18n("The folderfile %1 cannot be opened.", path), i18n("Load Error"));
        return false;
    }
    QTextStream stream(&file);
    QString data = stream.readAll();
    file.close();
    QDomDocument doc;
    QString error;
    int line;
    if (!doc.setContent( data, false, &error, &line)) {
        uError() << "Can not set content:" << error << " line:" << line;
        return false;
    }
    QDomNode rootNode = doc.firstChild();
    while (rootNode.isComment() || rootNode.isProcessingInstruction()) {
        rootNode = rootNode.nextSibling();
    }
    if (rootNode.isNull()) {
        uError() << "Root node is Null";
        return false;
    }
    QDomElement element = rootNode.toElement();
    QString type = element.tagName();
    if (type != "external_file") {
        uError() << "Root node has unknown type " << type;
        return false;
    }
    return load(element);
}

/**
 * Loads the UML:Component element.
 */
bool UMLFolder::load(QDomElement& element)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    bool totalSuccess = true;
    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement tempElement = node.toElement();
        QString type = tempElement.tagName();
        if (Model_Utils::isCommonXMIAttribute(type))
            continue;
        if (UMLDoc::tagEq(type, "Namespace.ownedElement") ||
                UMLDoc::tagEq(type, "Namespace.contents")) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore these tags are not further interpreted.
            if (! load(tempElement)) {
                uDebug() << "An error happened while loading the " << type
                    << " of the " << name();
                totalSuccess = false;
            }
            continue;
        } else if (type == "XMI.extension") {
            for (QDomNode xtnode = node.firstChild(); !xtnode.isNull();
                                              xtnode = xtnode.nextSibling()) {
                QDomElement el = xtnode.toElement();
                const QString xtag = el.tagName();
                if (xtag == "diagrams") {
                    QDomNode diagramNode = xtnode.firstChild();
                    if (!loadDiagramsFromXMI(diagramNode))
                        totalSuccess = false;
                } else if (xtag == "external_file") {
                    const QString rootDir(umldoc->url().directory());
                    QString fileName = el.attribute("name", "");
                    const QString path(rootDir + '/' + fileName);
                    if (loadFolderFile(path))
                        m_folderFile = fileName;
                } else {
                    uDebug() << name() << ": ignoring XMI.extension " << xtag;
                    continue;
                }
            }
            continue;
        }
        // Do not re-create the predefined Datatypes folder in the Logical View,
        // it already exists.
        UMLFolder *logicalView = umldoc->rootFolder(Uml::ModelType::Logical);
        if (this == logicalView && UMLDoc::tagEq(type, "Package")) {
            QString thisName = tempElement.attribute("name", "");
            if (thisName == "Datatypes") {
                UMLFolder *datatypeFolder = umldoc->datatypeFolder();
                if (!datatypeFolder->loadFromXMI(tempElement))
                    totalSuccess = false;
                continue;
            }
        }
        UMLObject *pObject = 0;
        // Avoid duplicate creation of forward declared object
        QString idStr = tempElement.attribute("xmi.id", "");
        if (!idStr.isEmpty()) {
            Uml::IDType id = STR2ID(idStr);
            pObject = umldoc->findObjectById(id);
            if (pObject) {
                uDebug() << "object " << idStr << "already exists";
            }
        }
        if (pObject == 0) {
            QString stereoID = tempElement.attribute("stereotype", "");
            pObject = Object_Factory::makeObjectFromXMI(type, stereoID);
            if (!pObject) {
                uWarning() << "Unknown type of umlobject to create: " << type;
                continue;
            }
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

/**
 * Overloading operator for debugging output.
 */
QDebug operator<<(QDebug out, const UMLFolder& item)
{
    out.nospace() << "UMLFolder: localName=" << item.m_localName
        << ", folderFile=" << item.m_folderFile
        << ", diagrams=" << item.m_diagrams.count();
    return out.space();
}

#include "folder.moc"
