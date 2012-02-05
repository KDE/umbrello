/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlobject.h"

// app includes
#include "debug_utils.h"
#include "uniqueid.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "package.h"
#include "folder.h"
#include "stereotype.h"
#include "object_factory.h"
#include "model_utils.h"
#include "import_utils.h"
#include "docwindow.h"
#include "classpropdlg.h"
#include "cmds.h"

// kde includes
#include <klocale.h>

// qt includes
#include <QtCore/QPointer>
#include <QtGui/QApplication>

using namespace Uml;

/**
 * Creates a UMLObject.
 * @param parent   The parent of the object.
 * @param name     The name of the object.
 * @param id       The ID of the object (optional.) If omitted
 *                 then a new ID will be assigned internally.
 */
UMLObject::UMLObject(UMLObject* parent, const QString& name, Uml::IDType id)
  : QObject(parent),
    m_nId(id),
    m_name(name)
{
    init();
    if (id == Uml::id_None)
        m_nId = UniqueID::gen();
}

/**
 * Creates a UMLObject.
 * @param name     The name of the object.
 * @param id       The ID of the object (optional.) If omitted
 *                 then a new ID will be assigned internally.
 */
UMLObject::UMLObject(const QString& name, Uml::IDType id)
  : QObject(UMLApp::app()->document()),
    m_nId(id),
    m_name(name)
{
    init();
    if (id == Uml::id_None)
        m_nId = UniqueID::gen();
}

/**
 * Creates a UMLObject.
 * @param   parent   The parent of the object.
 */
UMLObject::UMLObject(UMLObject * parent)
  : QObject(parent),
    m_nId(Uml::id_None),
    m_name(QString())
{
    init();
}

/**
 * Standard destructor.
 */
UMLObject::~UMLObject()
{
}

/**
 * Initializes key variables of the class.
 */
void UMLObject::init()
{
    setObjectName("UMLObject");
    m_BaseType = ot_UMLObject;
    m_pUMLPackage = 0;
    m_Vis = Uml::Visibility::Public;
    m_pStereotype = 0;
    m_Doc.clear();
    m_bAbstract = false;
    m_bStatic = false;
    m_bInPaste = false;
    m_bCreationWasSignalled = false;
    m_pSecondary = 0;
}

/**
 * This method is called if you wish to see the properties of a
 * UMLObject.  A dialog box will be displayed from which you
 * can change the object's properties.
 *
 * @param page    The page to show.
 * @param assoc   Whether to show association page.
 * @return        True if we modified the object.
 */
bool UMLObject::showPropertiesPagedDialog(int page, bool assoc)
{
    Q_UNUSED(page);
    DocWindow *docwindow = UMLApp::app()->docWindow();
    docwindow->updateDocumentation(false);
    QPointer<ClassPropDlg> dlg = new ClassPropDlg((QWidget*)UMLApp::app(), this, assoc);
    bool modified = false;
    if (dlg->exec()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
        modified = true;
    }
    dlg->close();
    delete dlg;
    return modified;
}

/**
 * This should be reimplemented by subclasses if they wish to
 * accept certain types of associations. Note that this only
 * tells if this UMLObject can accept the association
 * type. When creating an association another check is made to
 * see if the association is valid. For example a UMLClass
 * (UMLClassifier) can accept generalizations and should
 * return true. If while creating a generalization the
 * superclass is already subclassed from this, the association
 * is not valid and will not be created.  The default accepts
 * nothing (returns false)
 */
bool UMLObject::acceptAssociationType(Uml::AssociationType type)
{
    Q_UNUSED(type);
    // A UMLObject accepts nothing. This should be reimplemented by the subclasses
    return false;
}

/**
 * Assigns a new Id to the object
 */
void UMLObject::setID(Uml::IDType NewID)
{
    m_nId = NewID;
    emitModified();
}

/**
 * Set the UMLObject's name
 */
void UMLObject::setName(const QString &strName)
{
    UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(this, strName));
}

/**
 * Method used by setName: it is called by  cmdSetName, Don't use it!
 */
void UMLObject::setNameCmd(const QString &strName)
{
    m_name = strName;
    emitModified();
}

/**
 * Returns a copy of m_name
 */
QString UMLObject::name() const
{
    return m_name;
}

/**
 * Returns the fully qualified name, i.e. all package prefixes and then m_name.
 *
 * @param separator  The separator string to use (optional.)
 *                   If not given then the separator is chosen according
 *                   to the currently selected active programming language
 *                   of import and code generation.
 * @param includeRoot  Whether to prefix the root folder name to the FQN.
 *                     See UMLDoc::getRootFolder(). Default: false.
 * @return  The fully qualified name of this UMLObject.
 */
QString UMLObject::fullyQualifiedName(const QString& separator,
        bool includeRoot /* = false */) const
{
    QString fqn;
    if (m_pUMLPackage && m_pUMLPackage != this) {
        bool skipPackage = false;
        if (!includeRoot) {
            UMLDoc *umldoc = UMLApp::app()->document();
            if (umldoc->rootFolderType(m_pUMLPackage) != Uml::ModelType::N_MODELTYPES ||
                    m_pUMLPackage == umldoc->datatypeFolder())
                skipPackage = true;
        }
        if (!skipPackage) {
            QString tempSeparator = separator;
            if (tempSeparator.isEmpty())
                tempSeparator = UMLApp::app()->activeLanguageScopeSeparator();
            fqn = m_pUMLPackage->fullyQualifiedName(tempSeparator, includeRoot);
            fqn.append(tempSeparator);
        }
    }
    fqn.append(m_name);
    return fqn;
}

/**
 * Overloaded '==' operator
 */
bool UMLObject::operator==(const UMLObject & rhs) const
{
    if (this == &rhs)
        return true;

    //don't compare IDs, these are program specific and
    //don't mean the objects are the same
    //***** CHECK: Who put in this comment? What was the reason?
    //***** Currently some operator== in umbrello compare the IDs
    //***** while others don't.

    if (m_name != rhs.m_name)
        return false;

    // Packages create different namespaces, therefore they should be
    // part of the equality test.
    if (m_pUMLPackage != rhs.m_pUMLPackage)
        return false;

    // Making the type part of an object's identity has its problems:
    // Not all programming languages support declarations of the same
    // name but different type.
    // In such cases, the code generator is responsible for generating
    // the appropriate error message.
    if (m_BaseType != rhs.m_BaseType)
        return false;

    // The documentation should not be part of the equality test.
    // If two objects are the same but differ only in their documentation,
    // what does that mean?
    //if( m_Doc != rhs.m_Doc )
    //  return false;

    // The visibility should not be part of the equality test.
    // What does it mean if two objects are the same but differ in their
    // visibility? - I'm not aware of any programming language that would
    // support that.
    //if( m_Vis != rhs.m_Vis )
    //  return false;

    // See comments above
    //if( m_pStereotype != rhs.m_pStereotype )
    //  return false;

    // See comments above
    //if( m_bAbstract != rhs.m_bAbstract )
    //  return false;

    // See comments above
    //if( m_bStatic != rhs.m_bStatic )
    //  return false;

    return true;
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLObject::copyInto(UMLObject *lhs) const
{
    // Data members with copy constructor
    lhs->m_Doc = m_Doc;
    lhs->m_pStereotype = m_pStereotype;
    lhs->m_bAbstract = m_bAbstract;
    lhs->m_bStatic = m_bStatic;
    lhs->m_BaseType = m_BaseType;
    lhs->m_Vis = m_Vis;
    lhs->m_pUMLPackage = m_pUMLPackage;

    // We don't want the same name existing twice.
    lhs->m_name = Model_Utils::uniqObjectName(m_BaseType, m_pUMLPackage, m_name);

    // Create a new ID.
    lhs->m_nId = UniqueID::gen();

    // Hope that the parent from QObject is okay.
    if (lhs->parent() != parent())
        uDebug() << "copyInto has a wrong parent";
}

/**
 * Returns the abstract state of the object.
 */
bool UMLObject::isAbstract() const
{
    return m_bAbstract;
}

/**
 * Sets the paste state of the object.
 */
void UMLObject::setAbstract(bool bAbstract)
{
    m_bAbstract = bAbstract;
    emitModified();
}

void UMLObject::setInPaste(bool bInPaste /* =true */)
{
    m_bInPaste = bInPaste;
}

/**
 * Returns true if this UMLObject has classifier scope,
 * otherwise false (the default).
 */
bool UMLObject::isStatic() const
{
    return m_bStatic;
}

/**
 * Sets the value for m_bStatic.
 */
void UMLObject::setStatic(bool bStatic)
{
    m_bStatic = bStatic;
    emitModified();
}

/**
 * Forces the emission of the modified signal.  Useful when
 * updating several attributes at a time: you can block the
 * signals, update all atts, and then force the signal.
 */
void UMLObject::emitModified()
{
    UMLDoc *umldoc = UMLApp::app()->document();
    if (!umldoc->loading() && !umldoc->closing())
        emit modified();
}

/**
 * Sets the documentation for the object.
 *
 * @param d The documentation for the object.
 */
void UMLObject::setDoc(const QString &d)
{
    m_Doc = d;
    //emit modified();  No, this is done centrally at DocWindow::updateDocumentation()
}

/**
 * Returns the type of the object.
 *
 * @return  Returns the type of the object.
 */
UMLObject::ObjectType UMLObject::baseType() const
{
    return m_BaseType;
}

/**
 * @return The type used for rtti as string.
 */
QLatin1String UMLObject::baseTypeStr() const
{
    return QLatin1String(ENUM_NAME(UMLObject, ObjectType, m_BaseType));
}

/**
 * Set the type of the object.
 *
 * @param ot The ObjectType to set.
 */
void UMLObject::setBaseType(ObjectType ot)
{
    m_BaseType = ot;
}

/**
 * Returns the ID of the object.
 *
 * @return  Returns the ID of the object.
 */
Uml::IDType UMLObject::id() const
{
    return m_nId;
}

/**
 * Returns the documentation for the object.
 *
 * @return  Returns the documentation for the object.
 */
QString UMLObject::doc() const
{
    return m_Doc;
}

/**
 * Returns the visibility of the object.
 *
 * @return  Returns the visibility of the object.
 */
Uml::Visibility UMLObject::visibility() const
{
    return m_Vis;
}

/**
 * Sets the visibility of the object.
 *
 * @param s   The visibility of the object.
 */
void UMLObject::setVisibility(Uml::Visibility s)
{
    UMLApp::app()->executeCommand(new CmdSetVisibility(this, s));
}

/**
 * Method used by setVisibility: it is called by  cmdSetVisibility, Don't use it!
 */
void UMLObject::setVisibilityCmd(Uml::Visibility s)
{
    m_Vis = s;
    emitModified();
}

/**
 * Sets the class' UMLStereotype. Adjusts the reference counts
 * at the previously set stereotype and at the new stereotype.
 * If the previously set UMLStereotype's reference count drops
 * to zero then the UMLStereotype is removed at the UMLDoc and
 * it is then physically deleted.
 *
 * @param stereo Sets the classes UMLStereotype.
 */
void UMLObject::setUMLStereotype(UMLStereotype *stereo)
{
    if (stereo == m_pStereotype)
        return;
    if (stereo) {
        stereo->incrRefCount();
    }
    if (m_pStereotype) {
        m_pStereotype->decrRefCount();
        if (m_pStereotype->refCount() == 0) {
            UMLDoc *pDoc = UMLApp::app()->document();
            pDoc->removeStereotype(m_pStereotype);
            delete m_pStereotype;
        }
    }
    m_pStereotype = stereo;
    // TODO: don't emit modified() if predefined folder
    emitModified();
}

/**
 * Sets the classes stereotype name.
 * Internally uses setUMLStereotype().
 *
 * @param _name     Sets the classes stereotype name.
 */
void UMLObject::setStereotype(const QString &_name)
{
    // UMLDoc* pDoc = UMLApp::app()->document();
    // pDoc->executeCommand(new cmdSetStereotype(this,_name));
    if (_name.isEmpty()) {
        setUMLStereotype(NULL);
        return;
    }
    UMLDoc *pDoc = UMLApp::app()->document();
    UMLStereotype *s = pDoc->findOrCreateStereotype(_name);
    setUMLStereotype(s);
}

void UMLObject::setStereotypeCmd(const QString& /*_name*/)
{
//TODO: put SetStereotype into QundoStack
}

/**
 * Sets the classes Package.
 * DEPRECATED - use SetUMLPackage instead.
 *
 * @param _name   The classes Package name.
 */
void UMLObject::setPackage(const QString &_name)
{
    UMLObject *pkgObj = NULL;
    if (!_name.isEmpty()) {
        UMLDoc* umldoc = UMLApp::app()->document();
        pkgObj = umldoc->findUMLObject(_name);
        if (pkgObj == NULL) {
            uDebug() << "creating UMLPackage " << _name << " for " << m_name;
            pkgObj = Import_Utils::createUMLObject(ot_Package, _name);
        } else {
            const ObjectType ot = pkgObj->baseType();
            if (ot != ot_Package && ot != ot_Folder && ot != ot_Component) {
                uError() << m_name << ": " << "existing " << _name << " is not a container";
                // This should not happen - if it does, there may be further problems.
                // A container name should not overlap with another name in the same scope.
                pkgObj = Import_Utils::createUMLObject(ot_Package, _name);
            }
        }
    }
    setUMLPackage(static_cast<UMLPackage *>(pkgObj));
}

/**
 * Sets the UMLPackage in which this class is located.
 *
 * @param pPkg   Pointer to the class' UMLPackage.
 */
bool UMLObject::setUMLPackage(UMLPackage* pPkg)
{
    if (pPkg == this) {
        uDebug() << "setting parent to myself is not allowed";
        return false;
    }

    if (pPkg->umlPackage() == this) {
        uDebug() << "setting parent to an object of which I'm already the parent is not allowed";
        return false;
    }

    m_pUMLPackage = pPkg;
    emitModified();
    return true;
}

/**
 * Returns the classes UMLStereotype object.
 *
 * @return   Returns the classes UMLStereotype object.
 */
const UMLStereotype * UMLObject::umlStereotype()
{
    return m_pStereotype;
}

/**
 * Returns the stereotype.
 */
QString UMLObject::stereotype(bool includeAdornments /* = false */) const
{
    if (m_pStereotype == NULL)
        return "";
    QString name = m_pStereotype->name();
    if (includeAdornments)
        name = QString::fromUtf8("«") + name + QString::fromUtf8("»");
    return name;
}

/**
 * Return the package(s) in which this UMLObject is contained
 * as a text.
 *
 * @param separator Separator string for joining together the
 *                  individual package prefixes (optional.)
 *                  If no separator is given then the separator
 *                  of the currently selected language is used.
 * @param includeRoot  Whether to prefix the root folder name.
 *                     Default: false.
 * @return  The UMLObject's enclosing package(s) as a text.
 */
QString UMLObject::package(const QString& separator, bool includeRoot)
{
    QString tempSeparator = separator;
    if (tempSeparator.isEmpty())
        tempSeparator = UMLApp::app()->activeLanguageScopeSeparator();
    QString fqn = fullyQualifiedName(tempSeparator, includeRoot);
    if (!fqn.contains(tempSeparator))
        return "";
    QString scope = fqn.left(fqn.length() - tempSeparator.length() - m_name.length());
    return scope;
}

/**
 * Return a list of the packages in which this class is embedded.
 * The outermost package is first in the list.
 *
 * @param includeRoot  Whether to prefix the root folder name.
 *                     Default: false.
 * @return  UMLPackageList of the containing packages.
 */
UMLPackageList UMLObject::packages(bool includeRoot) const
{
    UMLPackageList pkgList;
    UMLPackage* pkg = m_pUMLPackage;
    while (pkg != NULL) {
        pkgList.prepend(pkg);
        pkg = pkg->umlPackage();
    }
    if (!includeRoot)
        pkgList.removeFirst();
    return pkgList;
}

/**
 * Returns the UMLPackage that this class is located in.
 *
 * @return  Pointer to the UMLPackage of this class.
 */
UMLPackage* UMLObject::umlPackage()
{
    return m_pUMLPackage;
}

/**
 * Return secondary ID. Required by resolveRef().
 */
QString UMLObject::secondaryId() const
{
    return m_SecondaryId;
}

/**
 * Set the secondary ID.
 * Currently only required by petalTree2Uml(); all other setting of the
 * m_SecondaryID is internal to the UMLObject class hierarchy.
 */
void UMLObject::setSecondaryId(const QString& id)
{
    m_SecondaryId = id;
}

/**
 * Return secondary ID fallback.
 * Required by resolveRef() for imported model files.
 */
QString UMLObject::secondaryFallback() const
{
    return m_SecondaryFallback;
}

/**
 * Set the secondary ID fallback.
 * Currently only used by petalTree2Uml().
 */
void UMLObject::setSecondaryFallback(const QString& id)
{
    m_SecondaryFallback = id;
}

/**
 * Calls UMLDoc::signalUMLObjectCreated() if m_BaseType affords
 * doing so.
 */
void UMLObject::maybeSignalObjectCreated()
{
    if (!m_bCreationWasSignalled &&
            m_BaseType != ot_Stereotype &&
            m_BaseType != ot_Association &&
            m_BaseType != ot_Role) {
        m_bCreationWasSignalled = true;
        UMLDoc* umldoc = UMLApp::app()->document();
        umldoc->signalUMLObjectCreated(this);
    }
}

/**
 * Resolve referenced objects (if any.)
 * Needs to be called after all UML objects are loaded from file.
 * This needs to be done after all model objects are loaded because
 * some of the xmi.id's might be forward references, i.e. they may
 * identify model objects which were not yet loaded at the point of
 * reference.
 * The default implementation attempts resolution of the m_SecondaryId.
 *
 * @return   True for success.
 */
bool UMLObject::resolveRef()
{
    if (m_pSecondary || (m_SecondaryId.isEmpty() && m_SecondaryFallback.isEmpty())) {
        maybeSignalObjectCreated();
        return true;
    }
#ifdef VERBOSE_DEBUGGING
    uDebug() << m_name << ": m_SecondaryId is " << m_SecondaryId;
#endif
    UMLDoc *pDoc = UMLApp::app()->document();
    // In the new, XMI standard compliant save format,
    // the type is the xmi.id of a UMLClassifier.
    if (! m_SecondaryId.isEmpty()) {
        m_pSecondary = pDoc->findObjectById(STR2ID(m_SecondaryId));
        if (m_pSecondary != NULL) {
            if (m_pSecondary->baseType() == ot_Stereotype) {
                m_pStereotype = static_cast<UMLStereotype*>(m_pSecondary);
                m_pStereotype->incrRefCount();
                m_pSecondary = NULL;
            }
            m_SecondaryId = "";
            maybeSignalObjectCreated();
            return true;
        }
        if (m_SecondaryFallback.isEmpty()) {
            uDebug() << "object with xmi.id=" << m_SecondaryId << " not found, setting to undef";
            UMLFolder *datatypes = pDoc->datatypeFolder();
            m_pSecondary = Object_Factory::createUMLObject(ot_Datatype, "undef", datatypes, false);
            return true;
        }
    }
    if (m_SecondaryFallback.isEmpty()) {
        uError() << m_name << ": cannot find type with id " << m_SecondaryId;
        return false;
    }
#ifdef VERBOSE_DEBUGGING
    uDebug() << m_name << ": could not resolve secondary ID " << m_SecondaryId
             << ", using secondary fallback " << m_SecondaryFallback;
#endif
    m_SecondaryId = m_SecondaryFallback;
    // Assume we're dealing with the older Umbrello format where
    // the type name was saved in the "type" attribute rather
    // than the xmi.id of the model object of the attribute type.
    m_pSecondary = pDoc->findUMLObject(m_SecondaryId, ot_UMLObject, this);
    if (m_pSecondary) {
        m_SecondaryId = "";
        maybeSignalObjectCreated();
        return true;
    }
    // Work around Object_Factory::createUMLObject()'s incapability
    // of on-the-fly scope creation:
    if (m_SecondaryId.contains("::")) {
        // TODO: Merge Import_Utils::createUMLObject() into Object_Factory::createUMLObject()
        m_pSecondary = Import_Utils::createUMLObject(ot_UMLObject, m_SecondaryId, m_pUMLPackage);
        if (m_pSecondary) {
            if (Import_Utils::newUMLObjectWasCreated()) {
                maybeSignalObjectCreated();
                qApp->processEvents();
                uDebug() << "Import_Utils::createUMLObject() created a new type for "
                         << m_SecondaryId;
            } else {
                uDebug() << "Import_Utils::createUMLObject() returned an existing type for "
                         << m_SecondaryId;
            }
            m_SecondaryId = "";
            return true;
        }
        uError() << "Import_Utils::createUMLObject() failed to create a new type for "
                 << m_SecondaryId;
        return false;
    }
    uDebug() << "Creating new type for " << m_SecondaryId;
    // This is very C++ specific - we rely on  some '*' or
    // '&' to decide it's a ref type. Plus, we don't recognize
    // typedefs of ref types.
    bool isReferenceType = (m_SecondaryId.contains('*') ||
                            m_SecondaryId.contains('&'));
    ObjectType ot = ot_Class;
    if (isReferenceType) {
        ot = ot_Datatype;
    } else {
        if (Model_Utils::isCommonDataType(m_SecondaryId))
            ot = ot_Datatype;
    }
    m_pSecondary = Object_Factory::createUMLObject(ot, m_SecondaryId, NULL);
    if (m_pSecondary == NULL)
        return false;
    m_SecondaryId = "";
    maybeSignalObjectCreated();
    //qApp->processEvents();
    return true;
}

/**
 * Auxiliary to saveToXMI.
 * Create a QDomElement with the given tag, and save the XMI attributes
 * that are common to all child classes to the newly created element.
 * This method does not need to be overridden by child classes.
 */
QDomElement UMLObject::save(const QString &tag, QDomDocument & qDoc)
{
    /*
      Call as the first action of saveToXMI() in child class:
      This creates the QDomElement with which to work.
    */
    QDomElement qElement = qDoc.createElement(tag);
    qElement.setAttribute("isSpecification", "false");
    if (m_BaseType != ot_Association &&
        m_BaseType != ot_Role &&
        m_BaseType != ot_Attribute) {
        qElement.setAttribute("isLeaf", "false");
        qElement.setAttribute("isRoot", "false");
        if (m_bAbstract)
            qElement.setAttribute("isAbstract", "true");
        else
            qElement.setAttribute("isAbstract", "false");
    }
    qElement.setAttribute("xmi.id", ID2STR(m_nId));
    qElement.setAttribute("name", m_name);
    if (m_BaseType != ot_Operation &&
        m_BaseType != ot_Role &&
        m_BaseType != ot_Attribute) {
        Uml::IDType nmSpc;
        if (m_pUMLPackage)
            nmSpc = m_pUMLPackage->id();
        else
            nmSpc = UMLApp::app()->document()->modelID();
        qElement.setAttribute("namespace", ID2STR(nmSpc));
    }
    if (! m_Doc.isEmpty())
        qElement.setAttribute("comment", m_Doc);    //CHECK: uml13.dtd compliance
#ifdef XMI_FLAT_PACKAGES
    if (m_pUMLPackage)             //FIXME: uml13.dtd compliance
        qElement.setAttribute("package", m_pUMLPackage->getID());
#endif
    QString visibility = m_Vis.toString(false);
    qElement.setAttribute("visibility", visibility);
    if (m_pStereotype != NULL)
        qElement.setAttribute("stereotype", ID2STR(m_pStereotype->id()));
    if (m_bStatic)
        qElement.setAttribute("ownerScope", "classifier");
    /* else
        qElement.setAttribute( "ownerScope", "instance" );
     *** ownerScope defaults to instance if not set **********/
    return qElement;
}

/**
 * Auxiliary to loadFromXMI.
 * This method is usually overridden by child classes.
 * It is responsible for loading the specific XMI structure
 * of the child class.
 */
bool UMLObject::load(QDomElement&)
{
    // This body is not usually executed because child classes
    // overwrite the load method.
    return true;
}

/**
 * Analyzes the given QDomElement for a reference to a stereotype.
 *
 * @param element   QDomElement to analyze.
 * @return          True if a stereotype reference was found, else false.
 */
bool UMLObject::loadStereotype(QDomElement & element)
{
    QString tag = element.tagName();
    if (!UMLDoc::tagEq(tag, "stereotype"))
        return false;
    QString stereo = element.attribute("xmi.value", "");
    if (stereo.isEmpty() && element.hasChildNodes()) {
        /* like so:
         <UML:ModelElement.stereotype>
           <UML:Stereotype xmi.idref = '07CD'/>
         </UML:ModelElement.stereotype>
         */
        QDomNode stereoNode = element.firstChild();
        QDomElement stereoElem = stereoNode.toElement();
        tag = stereoElem.tagName();
        if (UMLDoc::tagEq(tag, "Stereotype")) {
            stereo = stereoElem.attribute("xmi.idref", "");
        }
    }
    if (stereo.isEmpty())
        return false;
    Uml::IDType stereoID = STR2ID(stereo);
    UMLDoc *pDoc = UMLApp::app()->document();
    m_pStereotype = pDoc->findStereotypeById(stereoID);
    if (m_pStereotype)
        m_pStereotype->incrRefCount();
    else
        m_SecondaryId = stereo;  // leave it to resolveRef()
    return true;
}

/**
 * This method loads the generic parts of the XMI common to most model
 * classes.  It is not usually reimplemented by child classes.
 * Instead, it invokes the load() method which implements the loading
 * of the specifics of each child class.
 *
 * @param element   The QDomElement from which to load.
 */
bool UMLObject::loadFromXMI(QDomElement & element)
{
    UMLDoc* umldoc = UMLApp::app()->document();
    if (umldoc == 0) {
        uError() << "umldoc is NULL";
        return false;
    }
    // Read the name first so that if we encounter a problem, the error
    // message can say the name.
    m_name = element.attribute("name", "");
    QString id = element.attribute("xmi.id", "");
    if (id.isEmpty() || id == "-1") {
        if (m_BaseType == ot_Role) {
            // Before version 1.4, Umbrello did not save the xmi.id
            // of UMLRole objects.
            m_nId = UniqueID::gen();
        } else {
            uError() << m_name << ": nonexistent or illegal xmi.id";
            return false;
        }
    } else {
        m_nId = STR2ID(id);
        if (m_BaseType == ot_Role) {
            // Some older Umbrello versions had a problem with xmi.id's
            // of other objects being reused for the UMLRole, see e.g.
            // attachment 21179 at http://bugs.kde.org/147988 .
            // If the xmi.id is already being used then we generate a new one.
            UMLObject *o = umldoc->findObjectById(m_nId);
            if (o) {
                uError() << "loadFromXMI(UMLRole): id " << id
                         << " is already in use!!! Please fix your XMI file.";
            }
        }
    }

    if (element.hasAttribute("documentation"))  // for bkwd compat.
        m_Doc = element.attribute("documentation", "");
    else
        m_Doc = element.attribute("comment", "");    //CHECK: need a UML:Comment?

    m_Vis = Uml::Visibility::Public;
    if (element.hasAttribute("scope")) {        // for bkwd compat.
        QString scope = element.attribute("scope", "");
        if (scope == "instance_level")         // nsuml compat.
            m_bStatic = false;
        else if (scope == "classifier_level")  // nsuml compat.
            m_bStatic = true;
        else {
            int nScope = scope.toInt();
            switch (nScope) {
            case 200:
                m_Vis = Uml::Visibility::Public;
                break;
            case 201:
                m_Vis = Uml::Visibility::Private;
                break;
            case 202:
                m_Vis = Uml::Visibility::Protected;
                break;
            default:
                uError() << m_name << ": illegal scope " << nScope;
            }
        }
    } else {
        QString visibility = element.attribute("visibility", "public");
        if (visibility == "private"
                || visibility == "private_vis")    // for compatibility with other programs
            m_Vis = Uml::Visibility::Private;
        else if (visibility == "protected"
                 || visibility == "protected_vis")  // for compatibility with other programs
            m_Vis = Uml::Visibility::Protected;
        else if (visibility == "implementation")
            m_Vis = Uml::Visibility::Implementation;
    }

    QString stereo = element.attribute("stereotype", "");
    if (!stereo.isEmpty()) {
        Uml::IDType stereoID = STR2ID(stereo);
        m_pStereotype = umldoc->findStereotypeById(stereoID);
        if (m_pStereotype) {
            m_pStereotype->incrRefCount();
        } else {
            uDebug() << m_name << ": UMLStereotype " << ID2STR(stereoID)
                     << " not found, creating now.";
            setStereotype(stereo);
        }
    }

    if (element.hasAttribute("abstract")) {      // for bkwd compat.
        QString abstract = element.attribute("abstract", "0");
        m_bAbstract = (bool)abstract.toInt();
    } else {
        QString isAbstract = element.attribute("isAbstract", "false");
        m_bAbstract = (isAbstract == "true");
    }

    if (element.hasAttribute("static")) {        // for bkwd compat.
        QString staticScope = element.attribute("static", "0");
        m_bStatic = (bool)staticScope.toInt();
    } else {
        QString ownerScope = element.attribute("ownerScope", "instance");
        m_bStatic = (ownerScope == "classifier");
    }

    // If the node has child nodes, check whether attributes can be
    // extracted from them.
    if (element.hasChildNodes()) {
        QDomNode node = element.firstChild();
        if (node.isComment())
            node = node.nextSibling();
        QDomElement elem = node.toElement();
        while (!elem.isNull()) {
            QString tag = elem.tagName();
            if (UMLDoc::tagEq(tag, "name")) {
                m_name = elem.attribute("xmi.value", "");
                if (m_name.isEmpty())
                    m_name = elem.text();
            } else if (UMLDoc::tagEq(tag, "visibility")) {
                QString vis = elem.attribute("xmi.value", "");
                if (vis.isEmpty())
                    vis = elem.text();
                if (vis == "private" || vis == "private_vis")
                    m_Vis = Uml::Visibility::Private;
                else if (vis == "protected" || vis == "protected_vis")
                    m_Vis = Uml::Visibility::Protected;
                else if (vis == "implementation")
                    m_Vis = Uml::Visibility::Implementation;
            } else if (UMLDoc::tagEq(tag, "isAbstract")) {
                QString isAbstract = elem.attribute("xmi.value", "");
                if (isAbstract.isEmpty())
                    isAbstract = elem.text();
                m_bAbstract = (isAbstract == "true");
            } else if (UMLDoc::tagEq(tag, "ownerScope")) {
                QString ownerScope = elem.attribute("xmi.value", "");
                if (ownerScope.isEmpty())
                    ownerScope = elem.text();
                m_bStatic = (ownerScope == "classifier");
            } else {
                loadStereotype(elem);
            }
            node = node.nextSibling();
            if (node.isComment())
                node = node.nextSibling();
            elem = node.toElement();
        }
    }

    // Operations, attributes, enum literals, templates, stereotypes,
    // and association role objects get added and signaled elsewhere.
    if (m_BaseType != ot_Operation && m_BaseType != ot_Attribute &&
        m_BaseType != ot_EnumLiteral && m_BaseType != ot_EntityAttribute &&
        m_BaseType != ot_Template && m_BaseType != ot_Stereotype &&
        m_BaseType != ot_Role && m_BaseType != ot_UniqueConstraint &&
        m_BaseType != ot_ForeignKeyConstraint) {
        if (m_bInPaste) {
            m_pUMLPackage = Model_Utils::treeViewGetPackageFromCurrent();
        }
        if (m_pUMLPackage) {
            m_pUMLPackage->addObject(this);
        } else if (umldoc->rootFolderType(this) == Uml::ModelType::N_MODELTYPES) {
            // m_pUMLPackage is not set on the root folders.
            uDebug() << m_name << ": m_pUMLPackage is not set";
        }
    }
    return load(element);
}

/**
 * Helper function for debug output.
 * Returns the given enum value as string.
 * @param ot   ObjectType of which a string representation is wanted
 * @return   the ObjectType as string
 */
QString UMLObject::toString(ObjectType ot)
{
    return QLatin1String(ENUM_NAME(UMLObject, ObjectType, ot));
}

/**
 * Print UML Object to debug output stream, so it can be used like
 *   uDebug() << "This object shouldn't be here: " << illegalObject;
 */
QDebug operator<<(QDebug out, const UMLObject& obj)
{
    out.nospace() << "UMLObject: name= " << obj.name()
        << ", type= " << UMLObject::toString(obj.m_BaseType);
    return out.space();
}

#include "umlobject.moc"
