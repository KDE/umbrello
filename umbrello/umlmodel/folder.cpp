/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
#include "datatype.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QFile>
#include <QXmlStreamWriter>

DEBUG_REGISTER(UMLFolder)

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
    UMLObject::setStereotypeCmd(QStringLiteral("folder"));
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
 * Activate the views in this folder.
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
        UMLScene *scene = v->umlScene();
        scene->activateAfterLoad();
        uDebug() << "UMLFolder::activateViews: " << scene->name() << " sceneRect = " << scene->sceneRect();
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
        if (v && v->umlScene() && v->umlScene()->ID() == id) {
            return v;
        }
    }

    UMLView *v = nullptr;
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

    UMLView *v = nullptr;
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
        logDebug1("UMLFolder::removeAllViews: removing %1", v->umlScene()->name());
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
void UMLFolder::saveContents(QXmlStreamWriter& writer)
{
    if (! Settings::optionState().generalState.uml2) {
        writer.writeStartElement(QStringLiteral("UML:Namespace.ownedElement"));
    }
    // Save contained objects if any.
    foreach (UMLObject *obj, m_objects) {
        uIgnoreZeroPointer(obj);
        obj->saveToXMI (writer);
    }
    // Save associations if any.
    foreach (UMLObject *obj, subordinates()) {
        obj->saveToXMI (writer);
    }
    if (! Settings::optionState().generalState.uml2) {
        writer.writeEndElement();
    }
    // Save diagrams to `extension'.
    if (m_diagrams.count()) {
        if (Settings::optionState().generalState.uml2) {
            writer.writeStartElement(QStringLiteral("xmi:Extension"));
            writer.writeAttribute(QStringLiteral("extender"), QStringLiteral("umbrello"));
        } else {
            writer.writeStartElement(QStringLiteral("XMI.extension"));
            writer.writeAttribute(QStringLiteral("xmi.extender"), QStringLiteral("umbrello"));
        }
        writer.writeStartElement(QStringLiteral("diagrams"));
        if (!qFuzzyIsNull(UMLApp::app()->document()->resolution())) {
            writer.writeAttribute(QStringLiteral("resolution"),
                                  QString::number(UMLApp::app()->document()->resolution()));
        }
        foreach (UMLView* pView, m_diagrams) {
            pView->umlScene()->saveToXMI(writer);
        }
        writer.writeEndElement();            // diagrams
        writer.writeEndElement();   // XMI.extension
    }
}

/**
 * Auxiliary to saveToXMI():
 * - In UML1 mode it creates a <UML:Model> element when saving a predefined
 *   modelview, or a <UML:Package> element when saving a user created folder.
 * - In UML2 mode it creates a \<packagedElement xmi:type="uml:Model"> when
 *   saving a predefined view, or a \<packagedElement xmi:type="uml:Package">
 *   when saving a user created folder.
 * Invokes saveContents() with the newly created element.
 */
void UMLFolder::save1(QXmlStreamWriter& writer)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    QString elementName(QStringLiteral("Package"));
    QString elementTag (QStringLiteral("<use_type_as_tag>"));
    const Uml::ModelType::Enum mt = umldoc->rootFolderType(this);
    if (mt != Uml::ModelType::N_MODELTYPES) {
        elementName = QStringLiteral("Model");
        if (Settings::optionState().generalState.uml2)
            elementTag  = QStringLiteral("packagedElement");
    }
    UMLObject::save1(writer, elementName, elementTag);
    saveContents(writer);
    writer.writeEndElement();
}

/**
 * Saves the folder in XMI representation:
 * If m_folderFile is empty then calls save1().
 * If m_folderFile is non empty then
 * - creates an XMI Extension stub for the folder in the main XMI file;
 * - creates the external file for the submodel and writes its XMI.
 */
void UMLFolder::saveToXMI(QXmlStreamWriter& writer)
{
    if (m_folderFile.isEmpty()) {
        save1(writer);
        return;
    }
    // See if we can create the external file.
    // If not then internalize the folder.
    UMLDoc *umldoc = UMLApp::app()->document();
    QString fileName = umldoc->url().adjusted(QUrl::RemoveFilename).path() + m_folderFile;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        logError1("UMLFolder::saveToXMI(%1) : cannot create file. Content will be saved in main model file",
                  m_folderFile);
        m_folderFile.clear();
        save1(writer);
        return;
    }
    // External file is writable.  Create XMI.extension stub in main file.
    UMLObject::save1(writer, QStringLiteral("Package"));
    if (Settings::optionState().generalState.uml2) {
        writer.writeStartElement(QStringLiteral("xmi:Extension"));
        writer.writeAttribute(QStringLiteral("extender"), QStringLiteral("umbrello"));
    } else {
        writer.writeStartElement(QStringLiteral("XMI.extension"));
        writer.writeAttribute(QStringLiteral("xmi.extender"), QStringLiteral("umbrello"));
    }
    writer.writeStartElement(QStringLiteral("external_file"));
    writer.writeAttribute(QStringLiteral("name"), m_folderFile);
    writer.writeEndElement();            // external_file
    writer.writeEndElement();        // XMI.extension
    writer.writeEndElement();    // UML:Package

    // Write the external file.
    QXmlStreamWriter xfWriter(&file);
    xfWriter.setCodec("UTF-8");
    xfWriter.setAutoFormatting(true);
    xfWriter.setAutoFormattingIndent(2);
    xfWriter.writeStartDocument();
    xfWriter.writeStartElement(QStringLiteral("external_file"));
    xfWriter.writeAttribute(QStringLiteral("name"), name());
    xfWriter.writeAttribute(QStringLiteral("filename"), m_folderFile);
    xfWriter.writeAttribute(QStringLiteral("mainModel"), umldoc->url().fileName());
    xfWriter.writeAttribute(QStringLiteral("parentId"), Uml::ID::toString(umlPackage()->id()));
    xfWriter.writeAttribute(QStringLiteral("parent"), umlPackage()->fullyQualifiedName(QStringLiteral("::"), true));
    saveContents(xfWriter);
    xfWriter.writeEndElement();
    file.close();
}

/**
 * Auxiliary to load():
 * Load the diagrams from the "diagrams" in the <XMI.extension>
 */
bool UMLFolder::loadDiagramsFromXMI1(QDomNode& node)
{
    qreal resolution = 0.0;
    QString res = node.toElement().attribute(QStringLiteral("resolution"), QStringLiteral(""));
    if (!res.isEmpty()) {
       resolution = res.toDouble();
    }
    if (!qFuzzyIsNull(resolution)) {
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
        if (tag != QStringLiteral("diagram")) {
            logDebug1("UMLFolder::loadDiagramsFromXMI1: ignoring %1 in <diagrams>", tag);
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
        KMessageBox::error(nullptr, i18n("The folderfile %1 does not exist.", path), i18n("Load Error"));
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        KMessageBox::error(nullptr, i18n("The folderfile %1 cannot be opened.", path), i18n("Load Error"));
        return false;
    }
    QTextStream stream(&file);
    QString data = stream.readAll();
    file.close();
    QDomDocument doc;
    QString error;
    int line;
    if (!doc.setContent(data, false, &error, &line)) {
        logError2("UMLFolder::loadFolderFile cannot set content: error %1 line %2", error, line);
        return false;
    }
    QDomNode rootNode = doc.firstChild();
    while (rootNode.isComment() || rootNode.isProcessingInstruction()) {
        rootNode = rootNode.nextSibling();
    }
    if (rootNode.isNull()) {
        logError0("UMLFolder::loadFolderFile: Root node is null");
        return false;
    }
    QDomElement element = rootNode.toElement();
    QString type = element.tagName();
    if (type != QStringLiteral("external_file")) {
        logError1("UMLFolder::loadFolderFile: Root node has unknown type %1", type);
        return false;
    }
    return load1(element);
}

/**
 * Loads the owned elements of the \<packagedElement xmi:type="uml:Model">
 * (in UML2 mode) or <UML:Model> (in UML1 mode).
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
        if (UMLDoc::tagEq(type, QStringLiteral("Namespace.ownedElement")) ||
                UMLDoc::tagEq(type, QStringLiteral("Namespace.contents"))) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore these tags are not further interpreted.
            if (! load1(tempElement)) {
                logDebug2("UMLFolder::load1 %1: An error happened while loading %2", name(), type);
                totalSuccess = false;
            }
            continue;
        } else if (type == QStringLiteral("packagedElement") ||
                   type == QStringLiteral("ownedElement")) {
            type = tempElement.attribute(QStringLiteral("xmi:type"));
        } else if (type == QStringLiteral("XMI.extension") ||
                   type == QStringLiteral("xmi:Extension")) {
            for (QDomNode xtnode = node.firstChild(); !xtnode.isNull();
                                              xtnode = xtnode.nextSibling()) {
                QDomElement el = xtnode.toElement();
                const QString xtag = el.tagName();
                if (xtag == QStringLiteral("diagrams")) {
                    umldoc->addDiagramToLoad(this, xtnode);
                } else if (xtag == QStringLiteral("external_file")) {
                    const QString rootDir(umldoc->url().adjusted(QUrl::RemoveFilename).path());
                    QString fileName = el.attribute(QStringLiteral("name"));
                    const QString path(rootDir + QLatin1Char('/') + fileName);
                    if (loadFolderFile(path))
                        m_folderFile = fileName;
                } else {
                    logDebug2("UMLFolder::load1 %1: ignoring XMI.extension %2", name(), xtag);
                    continue;
                }
            }
            continue;
        }
        // Do not re-create the predefined Datatypes folder in the Logical View,
        // it already exists.
        UMLFolder *logicalView = umldoc->rootFolder(Uml::ModelType::Logical);
        if (this == logicalView && UMLDoc::tagEq(type, QStringLiteral("Package"))) {
            QString thisName = tempElement.attribute(QStringLiteral("name"));
            if (thisName == QStringLiteral("Datatypes")) {
                UMLFolder *datatypeFolder = umldoc->datatypeFolder();
                if (!datatypeFolder->loadFromXMI(tempElement))
                    totalSuccess = false;
                continue;
            }
        }
        UMLObject  *pObject = nullptr;
        // Avoid duplicate creation of forward declared object
        QString idStr = Model_Utils::getXmiId(tempElement);
        if (!idStr.isEmpty()) {
            Uml::ID::Type id = Uml::ID::fromString(idStr);
            pObject = umldoc->findObjectById(id);
            if (pObject) {
                logDebug1("UMLFolder::load1: object %1 already exists", idStr);
            }
        }
        // Avoid duplicate creation of datatype
        if (pObject == nullptr && this == umldoc->datatypeFolder()) {
            QString name = tempElement.attribute(QStringLiteral("name"));
            foreach (UMLObject *o, m_objects) {
                uIgnoreZeroPointer(o);
                if (o->name() == name) {
                    UMLDatatype *dt = o->asUMLDatatype();
                    if (dt) {
                        QString isActive = tempElement.attribute(QStringLiteral("isActive"));
                        dt->setActive(isActive != QStringLiteral("false"));
                        pObject = dt;
                        break;
                    }
                }
            }
        }
        if (pObject == nullptr) {
            QString stereoID = tempElement.attribute(QStringLiteral("stereotype"));
            pObject = Object_Factory::makeObjectFromXMI(type, stereoID);
            if (!pObject) {
                logWarn1("UMLFolder::load1 unknown type of umlobject to create: %1", type);
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

bool UMLFolder::showPropertiesDialog(QWidget *parent)
{
    Q_UNUSED(parent);
    QString folderName = this->name();
    bool ok = Dialog_Utils::askRenameName(UMLObject::ot_Folder, folderName);
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

