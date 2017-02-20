/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "folder.h"

// app includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "model_utils.h"
#include "object_factory.h"
#include "optionstate.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QFile>

/**
 * Sets up a Folder.
 * @param name    The name of the Folder.
 * @param id      The unique id of the Folder. A new ID will be generated
 *                if this argument is left away.
 */
UMLFolder::UMLFolder(const QString & name, Uml::ID::Type id)
  : UMLPackage(name, id)
{
    m_BaseType = UMLObject::ot_Folder;
    UMLObject::setStereotypeCmd(QLatin1String("folder"));
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
        foreach (UMLObject* o, m_objects) {
            uIgnoreZeroPointer(o);
            if (o->baseType() == UMLObject::ot_Folder) {
                UMLFolder *f = o->asUMLFolder();
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
    foreach (UMLObject* o, m_objects) {
        uIgnoreZeroPointer(o);
        if (o->baseType() == UMLObject::ot_Folder) {
            UMLFolder *f = o->asUMLFolder();
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
UMLView *UMLFolder::findView(Uml::ID::Type id)
{
    foreach (UMLView* v, m_diagrams) {
        if (v->umlScene()->ID() == id) {
            return v;
        }
    }

    UMLView* v = 0;
    UMLPackageList packages;
    appendPackages(packages);
    foreach (UMLPackage *o, packages) {
        if (o->baseType() != UMLObject::ot_Folder) {
            continue;
        }
        UMLFolder *f = o->asUMLFolder();
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
UMLView *UMLFolder::findView(Uml::DiagramType::Enum type, const QString &name, bool searchAllScopes)
{
    foreach (UMLView* v, m_diagrams) {
        if (v->umlScene()->type() == type && v->umlScene()->name() == name) {
            return v;
        }
    }

    UMLView* v = 0;
    if (searchAllScopes) {
        foreach (UMLObject* o, m_objects) {
            uIgnoreZeroPointer(o);
            if (o->baseType() != UMLObject::ot_Folder) {
                continue;
            }
            UMLFolder *f = o->asUMLFolder();
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
    foreach (UMLView* v, m_diagrams) {
        v->umlScene()->setOptionState(optionState);
    }
}

/**
 * Remove all views in this folder.
 */
void UMLFolder::removeAllViews()
{
    foreach (UMLObject* o, m_objects) {
        uIgnoreZeroPointer(o);
        if (o->baseType() != UMLObject::ot_Folder)
            continue;
        UMLFolder *f = o->asUMLFolder();
        f->removeAllViews();
    }

    foreach (UMLView* v, m_diagrams) {
        // TODO ------------------ check this code - bad: calling back to UMLDoc::removeView()
        v->umlScene()->removeAllAssociations(); // note : It may not be apparent, but when we remove all associations
        // from a view, it also causes any UMLAssociations that lack parent
        // association widgets (but once had them) to remove themselves from
        // this document.
        uDebug() << "removing " << v->umlScene()->name();
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
 * Auxiliary to saveToXMI1(): Save the contained objects and diagrams.
 * Can be used regardless of whether saving to the main model file
 * or to an external folder file (see m_folderFile.)
 */
void UMLFolder::saveContents1(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement ownedElement = qDoc.createElement(QLatin1String("UML:Namespace.ownedElement"));
    UMLObject *obj = 0;
    // Save contained objects if any.
    for (UMLObjectListIt oit(m_objects); oit.hasNext();) {
        obj = oit.next();
        uIgnoreZeroPointer(obj);
        obj->saveToXMI1 (qDoc, ownedElement);
    }
    // Save asscociations if any.
    for (UMLObjectListIt ait(m_List); ait.hasNext();) {
        obj = ait.next();
        obj->saveToXMI1 (qDoc, ownedElement);
    }
    qElement.appendChild(ownedElement);
    // Save diagrams to `extension'.
    if (m_diagrams.count()) {
        QDomElement diagramsElement = qDoc.createElement(QLatin1String("diagrams"));
        if (UMLApp::app()->document()->resolution() != 0.0)
            diagramsElement.setAttribute(QLatin1String("resolution"), UMLApp::app()->document()->resolution());

        foreach (UMLView* pView, m_diagrams) {
            pView->umlScene()->saveToXMI1(qDoc, diagramsElement);
        }
        QDomElement extension = qDoc.createElement(QLatin1String("XMI.extension"));
        extension.setAttribute(QLatin1String("xmi.extender"), QLatin1String("umbrello"));
        extension.appendChild(diagramsElement);
        qElement.appendChild(extension);
    }
}

/**
 * Auxiliary to saveToXMI1(): Creates a <UML:Model> element when saving
 * a predefined modelview, or a <UML:Package> element when saving a
 * user created folder. Invokes saveContents() with the newly created
 * element.
 */
void UMLFolder::save1(QDomDocument& qDoc, QDomElement& qElement)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    QString elementName(QLatin1String("UML:Package"));
    const Uml::ModelType::Enum mt = umldoc->rootFolderType(this);
    if (mt != Uml::ModelType::N_MODELTYPES) {
        elementName = QLatin1String("UML:Model");
    }
    QDomElement folderElement = UMLObject::save1(elementName, qDoc);
    saveContents1(qDoc, folderElement);
    qElement.appendChild(folderElement);
}

/**
 * Creates a UML:Model or UML:Package element:
 * UML:Model is created for the predefined fixed folders,
 * UML:Package with stereotype "folder" is created for all else.
 */
void UMLFolder::saveToXMI1(QDomDocument& qDoc, QDomElement& qElement)
{
    if (m_folderFile.isEmpty()) {
        save1(qDoc, qElement);
        return;
    }
    // See if we can create the external file.
    // If not then internalize the folder.
    UMLDoc *umldoc = UMLApp::app()->document();
#if QT_VERSION >= 0x050000
    QString fileName = umldoc->url().adjusted(QUrl::RemoveFilename).path() + m_folderFile;
#else
    QString fileName = umldoc->url().directory() + QLatin1Char('/') + m_folderFile;
#endif
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        uError() << m_folderFile << QLatin1String(": ")
            << "cannot create file, contents will be saved in main model file";
        m_folderFile.clear();
        save1(qDoc, qElement);
        return;
    }
    // Okay, external file is writable.  Wrap up main file.
    QDomElement folderElement = UMLObject::save1(QLatin1String("UML:Package"), qDoc);
    QDomElement extension = qDoc.createElement(QLatin1String("XMI.extension"));
    extension.setAttribute(QLatin1String("xmi.extender"), QLatin1String("umbrello"));
    QDomElement fileElement = qDoc.createElement(QLatin1String("external_file"));
    fileElement.setAttribute(QLatin1String("name"), m_folderFile);
    extension.appendChild(fileElement);
    folderElement.appendChild(extension);
    qElement.appendChild(folderElement);

    // Save folder to external file.
    QDomDocument folderDoc;
    QDomElement folderRoot;
    QDomProcessingInstruction xmlHeading =
        folderDoc.createProcessingInstruction(QLatin1String("xml"),
                                              QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\""));
    folderDoc.appendChild(xmlHeading);
    folderRoot = folderDoc.createElement(QLatin1String("external_file"));
    folderRoot.setAttribute(QLatin1String("name"), name());
    folderRoot.setAttribute(QLatin1String("filename"), m_folderFile);
    folderRoot.setAttribute(QLatin1String("mainModel"), umldoc->url().fileName());
    folderRoot.setAttribute(QLatin1String("parentId"), Uml::ID::toString(umlPackage()->id()));
    folderRoot.setAttribute(QLatin1String("parent"), umlPackage()->fullyQualifiedName(QLatin1String("::"), true));
    saveContents1(folderDoc, folderRoot);
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
bool UMLFolder::loadDiagramsFromXMI1(QDomNode& node)
{
    qreal resolution = 0.0;
    QString res = node.toElement().attribute(QLatin1String("resolution"), QLatin1String(""));
    if (!res.isEmpty()) {
       resolution = res.toDouble();
    }
    if (resolution != 0.0) {
       UMLApp::app()->document()->setResolution(resolution);
    } else {
       /* FIXME how to get dpi ?
        * 1. from user -> will open a dialog box for any old file
        * 2. after loading from user changeable document settings
        * 3. estimated from contained widgets
        */
       UMLApp::app()->document()->setResolution(0.0);
    }

    QDomNode diagrams = node.firstChild();
    const Settings::OptionState optionState = Settings::optionState();
    UMLDoc *umldoc = UMLApp::app()->document();
    bool totalSuccess = true;
    for (QDomElement diagram = diagrams.toElement(); !diagram.isNull();
         diagrams = diagrams.nextSibling(), diagram = diagrams.toElement()) {
        QString tag = diagram.tagName();
        if (tag != QLatin1String("diagram")) {
            uDebug() << "ignoring " << tag << " in <diagrams>";
            continue;
        }
        UMLView * pView = new UMLView(this);
        pView->umlScene()->setOptionState(optionState);
        if (pView->umlScene()->loadFromXMI1(diagram)) {
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
    if (!doc.setContent(data, false, &error, &line)) {
        uError() << "Cannot set content:" << error << " line:" << line;
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
    if (type != QLatin1String("external_file")) {
        uError() << "Root node has unknown type " << type;
        return false;
    }
    return load1(element);
}

/**
 * Loads the owned elements of the <UML:Model>.
 */
bool UMLFolder::load1(QDomElement& element)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    bool totalSuccess = true;
    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement tempElement = node.toElement();
        QString type = tempElement.tagName();
        if (Model_Utils::isCommonXMI1Attribute(type))
            continue;
        if (UMLDoc::tagEq(type, QLatin1String("Namespace.ownedElement")) ||
                UMLDoc::tagEq(type, QLatin1String("Namespace.contents"))) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore these tags are not further interpreted.
            if (! load1(tempElement)) {
                uDebug() << "An error happened while loading the " << type
                    << " of the " << name();
                totalSuccess = false;
            }
            continue;
        } else if (UMLDoc::tagEq(type, QLatin1String("packagedElement")) ||
                   UMLDoc::tagEq(type, QLatin1String("ownedElement"))) {
            type = tempElement.attribute(QLatin1String("xmi:type"));
        } else if (type == QLatin1String("XMI.extension")) {
            for (QDomNode xtnode = node.firstChild(); !xtnode.isNull();
                                              xtnode = xtnode.nextSibling()) {
                QDomElement el = xtnode.toElement();
                const QString xtag = el.tagName();
                if (xtag == QLatin1String("diagrams")) {
                    umldoc->addDiagramToLoad(this, xtnode);
                } else if (xtag == QLatin1String("external_file")) {
#if QT_VERSION >= 0x050000
                    const QString rootDir(umldoc->url().adjusted(QUrl::RemoveFilename).path());
#else
                    const QString rootDir(umldoc->url().directory());
#endif
                    QString fileName = el.attribute(QLatin1String("name"));
                    const QString path(rootDir + QLatin1Char('/') + fileName);
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
        if (this == logicalView && UMLDoc::tagEq(type, QLatin1String("Package"))) {
            QString thisName = tempElement.attribute(QLatin1String("name"));
            if (thisName == QLatin1String("Datatypes")) {
                UMLFolder *datatypeFolder = umldoc->datatypeFolder();
                if (!datatypeFolder->loadFromXMI1(tempElement))
                    totalSuccess = false;
                continue;
            }
        }
        UMLObject *pObject = 0;
        // Avoid duplicate creation of forward declared object
        QString idStr = Model_Utils::getXmiId(tempElement);
        if (!idStr.isEmpty()) {
            Uml::ID::Type id = Uml::ID::fromString(idStr);
            pObject = umldoc->findObjectById(id);
            if (pObject) {
                uDebug() << "object " << idStr << "already exists";
            }
        }
        if (pObject == 0) {
            QString stereoID = tempElement.attribute(QLatin1String("stereotype"));
            pObject = Object_Factory::makeObjectFromXMI(type, stereoID);
            if (!pObject) {
                uWarning() << "Unknown type of umlobject to create: " << type;
                continue;
            }
        }
        // check for invalid namespaces
        QString ns = tempElement.attribute(QLatin1String("namespace"));
        Uml::ID::Type id = Uml::ID::fromString(ns);
        if (id != this->id()) {
            uError() << "namespace" << ns << "not present; ignoring object with id" << idStr;
            delete pObject;
            pObject = 0;
            totalSuccess = false;
            continue;
        }
        pObject->setUMLPackage(this);
        if (!pObject->loadFromXMI1(tempElement)) {
            removeObject(pObject);
            delete pObject;
            totalSuccess = false;
        }
    }
    return totalSuccess;
}

bool UMLFolder::showPropertiesDialog(QWidget *parent)
{
    Q_UNUSED(parent);
    QString folderName = this->name();
    bool ok = Dialog_Utils::askName(i18n("Folder"),
                                    i18n("Enter name:"),
                                    folderName);
    if (ok) {
        setName(folderName);
    }
    return ok;
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

