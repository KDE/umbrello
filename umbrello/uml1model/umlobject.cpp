/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "umlobject.h"

// app includes
#include "classpropertiesdialog.h"
#include "debug_utils.h"
#include "enumliteral.h"
#include "uniqueid.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlobjectprivate.h"
#include "models/objectsmodel.h"
#include "package.h"
#include "folder.h"
#include "stereotype.h"
#include "object_factory.h"
#include "model_utils.h"
#include "import_utils.h"
#include "docwindow.h"
#include "cmds.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QApplication>
#include <QPointer>

using namespace Uml;

DEBUG_REGISTER_DISABLED(UMLObject)

/**
 * Creates a UMLObject.
 * @param other object to created from
 */
UMLObject::UMLObject(const UMLObject &other)
  : QObject(other.umlParent()),
    m_d(new UMLObjectPrivate)
{
    other.copyInto(this);
    UMLApp::app()->document()->objectsModel()->add(this);
}

/**
 * Creates a UMLObject.
 * @param parent   The parent of the object.
 * @param name     The name of the object.
 * @param id       The ID of the object (optional.) If omitted
 *                 then a new ID will be assigned internally.
 */
UMLObject::UMLObject(UMLObject* parent, const QString& name, ID::Type id)
  : QObject(parent),
    m_nId(id),
    m_name(name),
    m_d(new UMLObjectPrivate)
{
    init();
    if (id == Uml::ID::None)
        m_nId = UniqueID::gen();
    UMLApp::app()->document()->objectsModel()->add(this);
}

/**
 * Creates a UMLObject.
 * @param name     The name of the object.
 * @param id       The ID of the object (optional.) If omitted
 *                 then a new ID will be assigned internally.
 */
UMLObject::UMLObject(const QString& name, ID::Type id)
  : QObject(0),
    m_nId(id),
    m_name(name),
    m_d(new UMLObjectPrivate)
{
    init();
    if (id == Uml::ID::None)
        m_nId = UniqueID::gen();
    UMLApp::app()->document()->objectsModel()->add(this);
}

/**
 * Creates a UMLObject.
 * @param   parent   The parent of the object.
 */
UMLObject::UMLObject(UMLObject * parent)
  : QObject(parent),
    m_nId(Uml::ID::None),
    m_name(QString()),
    m_d(new UMLObjectPrivate)
{
    init();
    UMLApp::app()->document()->objectsModel()->add(this);
}

/**
 * Standard destructor.
 */
UMLObject::~UMLObject()
{
    // unref stereotype
    setUMLStereotype(0);
    if (m_pSecondary && m_pSecondary->baseType() == ot_Stereotype) {
        UMLStereotype* stereotype = m_pSecondary->asUMLStereotype();
        if (stereotype)
            stereotype->decrRefCount();
    }
    UMLApp::app()->document()->objectsModel()->remove(this);
    delete m_d;
}

/**
 * Initializes key variables of the class.
 */
void UMLObject::init()
{
    setObjectName(QLatin1String("UMLObject"));
    m_BaseType = ot_UMLObject;
    m_visibility = Uml::Visibility::Public;
    m_pStereotype = 0;
    m_Doc.clear();
    m_bAbstract = false;
    m_bStatic = false;
    m_bCreationWasSignalled = false;
    m_pSecondary = 0;
}

/**
 * Display the properties configuration dialog for the object.
 *
 * @param parent    The parent widget.
 * @return  True for success of this operation.
 */
bool UMLObject::showPropertiesDialog(QWidget *parent)
{
    DocWindow *docwindow = UMLApp::app()->docWindow();
    docwindow->updateDocumentation(false);
    QPointer<ClassPropertiesDialog> dlg = new ClassPropertiesDialog(parent, this, false);
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
bool UMLObject::acceptAssociationType(Uml::AssociationType::Enum type)
{
    Q_UNUSED(type);
    // A UMLObject accepts nothing. This should be reimplemented by the subclasses
    return false;
}

/**
 * Assigns a new Id to the object
 */
void UMLObject::setID(ID::Type NewID)
{
    m_nId = NewID;
    emitModified();
}

/**
 * Set the UMLObject's name
 */
void UMLObject::setName(const QString &strName)
{
    if (name() != strName) {
        UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(this, strName));
    }
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
    UMLPackage *parent = umlPackage();
    if (parent && parent != this) {
        bool skipPackage = false;
        if (!includeRoot) {
            UMLDoc *umldoc = UMLApp::app()->document();
            if ((umldoc->rootFolderType(parent) != Uml::ModelType::N_MODELTYPES) ||
                    (parent == umldoc->datatypeFolder()))
                skipPackage = true;
        }
        if (!skipPackage) {
            QString tempSeparator = separator;
            if (tempSeparator.isEmpty())
                tempSeparator = UMLApp::app()->activeLanguageScopeSeparator();
            fqn = parent->fullyQualifiedName(tempSeparator, includeRoot);
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
    if (umlParent() != rhs.umlParent())
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
    //if(m_Doc != rhs.m_Doc)
    //  return false;

    // The visibility should not be part of the equality test.
    // What does it mean if two objects are the same but differ in their
    // visibility? - I'm not aware of any programming language that would
    // support that.
    //if(m_visibility != rhs.m_visibility)
    //  return false;

    // See comments above
    //if(m_pStereotype != rhs.m_pStereotype)
    //  return false;

    // See comments above
    //if(m_bAbstract != rhs.m_bAbstract)
    //  return false;

    // See comments above
    //if(m_bStatic != rhs.m_bStatic)
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
    if (lhs->m_pStereotype)
        lhs->m_pStereotype->incrRefCount();
    lhs->m_bAbstract = m_bAbstract;
    lhs->m_bStatic = m_bStatic;
    lhs->m_BaseType = m_BaseType;
    lhs->m_visibility = m_visibility;
    lhs->setUMLParent(umlParent());

    // We don't want the same name existing twice.
    lhs->m_name = Model_Utils::uniqObjectName(m_BaseType, umlPackage(), m_name);

    // Create a new ID.
    lhs->m_nId = UniqueID::gen();

    // Hope that the parent from QObject is okay.
    if (lhs->umlParent() != umlParent())
        uDebug() << "copyInto has a wrong parent";
}

UMLObject *UMLObject::clone() const
{
    UMLObject *clone = new UMLObject;
    UMLObject::copyInto(clone);
    return clone;
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
ID::Type UMLObject::id() const
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
 * Returns state of documentation for the object.
 *
 * @return false if documentation is empty
 */
bool UMLObject::hasDoc() const
{
    return !m_Doc.isEmpty();
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
 * Returns the visibility of the object.
 *
 * @return  Returns the visibility of the object.
 */
Visibility::Enum UMLObject::visibility() const
{
    return m_visibility;
}

/**
 * Sets the visibility of the object.
 *
 * @param visibility  The visibility of the object.
 */
void UMLObject::setVisibility(Visibility::Enum visibility)
{
    if (m_visibility != visibility) {
        UMLApp::app()->executeCommand(new CmdSetVisibility(this, visibility));
    }
}

/**
 * Method used by setVisibility: it is called by  cmdSetVisibility, Don't use it!
 */
void UMLObject::setVisibilityCmd(Visibility::Enum visibility)
{
    m_visibility = visibility;
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
 * @param name     Sets the classes stereotype name.
 */
void UMLObject::setStereotype(const QString &name)
{
    if (name != stereotype()) {
        UMLApp::app()->executeCommand(new CmdSetStereotype(this, name));
    }
}

void UMLObject::setStereotypeCmd(const QString& name)
{
    if (name.isEmpty()) {
        setUMLStereotype(0);
        return;
    }
    UMLDoc *pDoc = UMLApp::app()->document();
    UMLStereotype *s = pDoc->findOrCreateStereotype(name);
    setUMLStereotype(s);
}

/**
 * Returns the classes UMLStereotype object.
 *
 * @return   Returns the classes UMLStereotype object.
 */
UMLStereotype * UMLObject::umlStereotype()
{
    return m_pStereotype;
}

/**
 * Returns the stereotype.
 */
QString UMLObject::stereotype(bool includeAdornments /* = false */) const
{
    if (m_pStereotype == 0)
        return QString();
    return m_pStereotype->name(includeAdornments);
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
        return QString();
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
    UMLPackage* pkg = umlPackage();
    while (pkg != 0) {
        pkgList.prepend(pkg);
        pkg = pkg->umlPackage();
    }
    if (!includeRoot)
        pkgList.removeFirst();
    return pkgList;
}

/**
 * Sets the UMLPackage in which this class is located.
 *
 * @param pPkg   Pointer to the class' UMLPackage.
 */
bool UMLObject::setUMLPackage(UMLPackage *pPkg)
{
    if (pPkg == this) {
        uDebug() << "setting parent to myself is not allowed";
        return false;
    }

    if (pPkg == 0) {
        // Allow setting to NULL for stereotypes
        setParent(pPkg);
        return true;
    }

    if (pPkg->umlPackage() == this) {
        uDebug() << "setting parent to an object of which I'm already the parent is not allowed";
        return false;
    }

    setParent(pPkg);
    emitModified();
    return true;
}

/**
 * Returns the UMLPackage that this class is located in.
 *
 * This method is a shortcut for calling umlParent()->asUMLPackage().
 *
 * @return  Pointer to the UMLPackage of this class.
 */
UMLPackage* UMLObject::umlPackage() const
{
    return dynamic_cast<UMLPackage *>(parent());
}

/**
 * Set UML model parent.
 *
 * @param parent object to set as parent
 *
 * @TODO prevent setting parent to myself
 */
void UMLObject::setUMLParent(UMLObject *parent)
{
    setParent(parent);
}

/**
 * Return UML model parent.
 *
 * Model classes of type UMLClassifierListItem and below
 * uses QObject::parent to hold the model parent
 *
 * @return parent of uml object
 */
UMLObject *UMLObject::umlParent() const
{
    uCheckPointerAndReturnIfZero(this);
    return dynamic_cast<UMLObject *>(parent());
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
        m_pSecondary = pDoc->findObjectById(Uml::ID::fromString(m_SecondaryId));
        if (m_pSecondary != 0) {
            if (m_pSecondary->baseType() == ot_Stereotype) {
                if (m_pStereotype)
                    m_pStereotype->decrRefCount();
                m_pStereotype = m_pSecondary->asUMLStereotype();
                m_pStereotype->incrRefCount();
                m_pSecondary = 0;
            }
            m_SecondaryId = QString();
            maybeSignalObjectCreated();
            return true;
        }
        if (m_SecondaryFallback.isEmpty()) {
            uDebug() << "object with xmi.id=" << m_SecondaryId << " not found, setting to undef";
            UMLFolder *datatypes = pDoc->datatypeFolder();
            m_pSecondary = Object_Factory::createUMLObject(ot_Datatype, QLatin1String("undef"), datatypes, false);
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
        m_SecondaryId = QString();
        maybeSignalObjectCreated();
        return true;
    }
    // Work around Object_Factory::createUMLObject()'s incapability
    // of on-the-fly scope creation:
    if (m_SecondaryId.contains(QLatin1String("::"))) {
        // TODO: Merge Import_Utils::createUMLObject() into Object_Factory::createUMLObject()
        m_pSecondary = Import_Utils::createUMLObject(ot_UMLObject, m_SecondaryId, umlPackage());
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
            m_SecondaryId = QString();
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
    bool isReferenceType = (m_SecondaryId.contains(QLatin1Char('*')) ||
                            m_SecondaryId.contains(QLatin1Char('&')));
    ObjectType ot = ot_Class;
    if (isReferenceType) {
        ot = ot_Datatype;
    } else {
        if (Model_Utils::isCommonDataType(m_SecondaryId))
            ot = ot_Datatype;
    }
    m_pSecondary = Object_Factory::createUMLObject(ot, m_SecondaryId, 0);
    if (m_pSecondary == 0)
        return false;
    m_SecondaryId = QString();
    maybeSignalObjectCreated();
    //qApp->processEvents();
    return true;
}

void UMLObject::saveToXMI1(QDomDocument &qDoc, QDomElement &qElement)
{
    Q_UNUSED(qDoc); Q_UNUSED(qElement);
}

/**
 * Auxiliary to saveToXMI1.
 * Create a QDomElement with the given tag, and save the XMI attributes
 * that are common to all child classes to the newly created element.
 * This method does not need to be overridden by child classes.
 */
QDomElement UMLObject::save1(const QString &tag, QDomDocument & qDoc)
{
    m_d->isSaved = true;
    /*
      Call as the first action of saveToXMI1() in child class:
      This creates the QDomElement with which to work.
    */
    QDomElement qElement = qDoc.createElement(tag);
    qElement.setAttribute(QLatin1String("isSpecification"), QLatin1String("false"));
    if (m_BaseType != ot_Association &&
        m_BaseType != ot_Role &&
        m_BaseType != ot_Attribute) {
        qElement.setAttribute(QLatin1String("isLeaf"), QLatin1String("false"));
        qElement.setAttribute(QLatin1String("isRoot"), QLatin1String("false"));
        if (m_bAbstract)
            qElement.setAttribute(QLatin1String("isAbstract"), QLatin1String("true"));
        else
            qElement.setAttribute(QLatin1String("isAbstract"), QLatin1String("false"));
    }
    qElement.setAttribute(QLatin1String("xmi.id"), Uml::ID::toString(m_nId));
    qElement.setAttribute(QLatin1String("name"), m_name);
    if (m_BaseType != ot_Operation &&
        m_BaseType != ot_Role &&
        m_BaseType != ot_Attribute) {
        Uml::ID::Type nmSpc;
        if (umlPackage())
            nmSpc = umlPackage()->id();
        else
            nmSpc = UMLApp::app()->document()->modelID();
        qElement.setAttribute(QLatin1String("namespace"), Uml::ID::toString(nmSpc));
    }
    if (! m_Doc.isEmpty())
        qElement.setAttribute(QLatin1String("comment"), m_Doc);    //CHECK: uml13.dtd compliance
#ifdef XMI_FLAT_PACKAGES
    if (umlParent()->asUMLPackage())             //FIXME: uml13.dtd compliance
        qElement.setAttribute(QLatin1String("package"), umlParent()->asUMLPackage()->ID());
#endif
    QString visibility = Uml::Visibility::toString(m_visibility, false);
    qElement.setAttribute(QLatin1String("visibility"), visibility);
    if (m_pStereotype != 0)
        qElement.setAttribute(QLatin1String("stereotype"), Uml::ID::toString(m_pStereotype->id()));
    if (m_bStatic)
        qElement.setAttribute(QLatin1String("ownerScope"), QLatin1String("classifier"));
    /* else
        qElement.setAttribute("ownerScope", "instance");
     *** ownerScope defaults to instance if not set **********/
    return qElement;
}

/**
 * Auxiliary to loadFromXMI.
 * This method is usually overridden by child classes.
 * It is responsible for loading the specific XMI structure
 * of the child class.
 */
bool UMLObject::load1(QDomElement&)
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
    if (!UMLDoc::tagEq(tag, QLatin1String("stereotype")))
        return false;
    QString stereo = element.attribute(QLatin1String("xmi.value"));
    if (stereo.isEmpty() && element.hasChildNodes()) {
        /* like so:
         <UML:ModelElement.stereotype>
           <UML:Stereotype xmi.idref = '07CD'/>
         </UML:ModelElement.stereotype>
         */
        QDomNode stereoNode = element.firstChild();
        QDomElement stereoElem = stereoNode.toElement();
        tag = stereoElem.tagName();
        if (UMLDoc::tagEq(tag, QLatin1String("Stereotype"))) {
            stereo = stereoElem.attribute(QLatin1String("xmi.idref"));
        }
    }
    if (stereo.isEmpty())
        return false;
    Uml::ID::Type stereoID = Uml::ID::fromString(stereo);
    UMLDoc *pDoc = UMLApp::app()->document();
    if (m_pStereotype)
        m_pStereotype->decrRefCount();
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
bool UMLObject::loadFromXMI1(QDomElement & element)
{
    UMLDoc* umldoc = UMLApp::app()->document();
    if (umldoc == 0) {
        uError() << "umldoc is NULL";
        return false;
    }
    // Read the name first so that if we encounter a problem, the error
    // message can say the name.
    m_name = element.attribute(QLatin1String("name"));
    QString id = Model_Utils::getXmiId(element);
    if (id.isEmpty() || id == QLatin1String("-1")) {
        // Before version 1.4, Umbrello did not save the xmi.id of UMLRole objects.
        // Some tools (such as Embarcadero's) do not have an xmi.id on all attributes.
        m_nId = UniqueID::gen();
        uWarning() << m_name << ": xmi.id not present, generating a new one";
    } else {
        Uml::ID::Type nId = Uml::ID::fromString(id);
        if (m_BaseType == ot_Role) {
            // Some older Umbrello versions had a problem with xmi.id's
            // of other objects being reused for the UMLRole, see e.g.
            // attachment 21179 at http://bugs.kde.org/147988 .
            // If the xmi.id is already being used then we generate a new one.
            UMLObject *o = umldoc->findObjectById(nId);
            if (o) {
                uError() << "loadFromXMI1(UMLRole): id " << id
                         << " is already in use!!! Please fix your XMI file.";
            }
        }
        m_nId = nId;
    }

    if (element.hasAttribute(QLatin1String("documentation")))  // for bkwd compat.
        m_Doc = element.attribute(QLatin1String("documentation"));
    else
        m_Doc = element.attribute(QLatin1String("comment"));    //CHECK: need a UML:Comment?

    m_visibility = Uml::Visibility::Public;
    if (element.hasAttribute(QLatin1String("scope"))) {        // for bkwd compat.
        QString scope = element.attribute(QLatin1String("scope"));
        if (scope == QLatin1String("instance_level"))         // nsuml compat.
            m_bStatic = false;
        else if (scope == QLatin1String("classifier_level"))  // nsuml compat.
            m_bStatic = true;
        else {
            int nScope = scope.toInt();
            switch (nScope) {
            case 200:
                m_visibility = Uml::Visibility::Public;
                break;
            case 201:
                m_visibility = Uml::Visibility::Private;
                break;
            case 202:
                m_visibility = Uml::Visibility::Protected;
                break;
            default:
                uError() << m_name << ": illegal scope " << nScope;
            }
        }
    } else {
        QString visibility = element.attribute(QLatin1String("visibility"), QLatin1String("public"));
        if (visibility == QLatin1String("private")
                || visibility == QLatin1String("private_vis"))    // for compatibility with other programs
            m_visibility = Uml::Visibility::Private;
        else if (visibility == QLatin1String("protected")
                 || visibility == QLatin1String("protected_vis"))  // for compatibility with other programs
            m_visibility = Uml::Visibility::Protected;
        else if (visibility == QLatin1String("implementation"))
            m_visibility = Uml::Visibility::Implementation;
    }

    QString stereo = element.attribute(QLatin1String("stereotype"));
    if (!stereo.isEmpty()) {
        Uml::ID::Type stereoID = Uml::ID::fromString(stereo);
        if (m_pStereotype)
            m_pStereotype->decrRefCount();
        m_pStereotype = umldoc->findStereotypeById(stereoID);
        if (m_pStereotype) {
            m_pStereotype->incrRefCount();
        } else {
            uDebug() << m_name << ": UMLStereotype " << Uml::ID::toString(stereoID)
                     << " not found, creating now.";
            setStereotypeCmd(stereo);
        }
    }

    if (element.hasAttribute(QLatin1String("abstract"))) {      // for bkwd compat.
        QString abstract = element.attribute(QLatin1String("abstract"), QLatin1String("0"));
        m_bAbstract = (bool)abstract.toInt();
    } else {
        QString isAbstract = element.attribute(QLatin1String("isAbstract"), QLatin1String("false"));
        m_bAbstract = (isAbstract == QLatin1String("true"));
    }

    if (element.hasAttribute(QLatin1String("static"))) {        // for bkwd compat.
        QString staticScope = element.attribute(QLatin1String("static"), QLatin1String("0"));
        m_bStatic = (bool)staticScope.toInt();
    } else {
        QString ownerScope = element.attribute(QLatin1String("ownerScope"), QLatin1String("instance"));
        m_bStatic = (ownerScope == QLatin1String("classifier"));
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
            if (UMLDoc::tagEq(tag, QLatin1String("name"))) {
                m_name = elem.attribute(QLatin1String("xmi.value"));
                if (m_name.isEmpty())
                    m_name = elem.text();
            } else if (UMLDoc::tagEq(tag, QLatin1String("visibility"))) {
                QString vis = elem.attribute(QLatin1String("xmi.value"));
                if (vis.isEmpty())
                    vis = elem.text();
                if (vis == QLatin1String("private") || vis == QLatin1String("private_vis"))
                    m_visibility = Uml::Visibility::Private;
                else if (vis == QLatin1String("protected") || vis == QLatin1String("protected_vis"))
                    m_visibility = Uml::Visibility::Protected;
                else if (vis == QLatin1String("implementation"))
                    m_visibility = Uml::Visibility::Implementation;
            } else if (UMLDoc::tagEq(tag, QLatin1String("isAbstract"))) {
                QString isAbstract = elem.attribute(QLatin1String("xmi.value"));
                if (isAbstract.isEmpty())
                    isAbstract = elem.text();
                m_bAbstract = (isAbstract == QLatin1String("true"));
            } else if (UMLDoc::tagEq(tag, QLatin1String("ownerScope"))) {
                QString ownerScope = elem.attribute(QLatin1String("xmi.value"));
                if (ownerScope.isEmpty())
                    ownerScope = elem.text();
                m_bStatic = (ownerScope == QLatin1String("classifier"));
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
        m_BaseType != ot_ForeignKeyConstraint && m_BaseType != ot_CheckConstraint &&
        m_BaseType != ot_InstanceAttribute ) {
        if (umlPackage()) {
            umlPackage()->addObject(this);
        } else if (umldoc->rootFolderType(this) == Uml::ModelType::N_MODELTYPES) {
            // umlPackage() is not set on the root folders.
            uDebug() << m_name << ": umlPackage() is not set";
        }
    }
    return load1(element);
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
 * Returns the given object type value as localized string.
 * @param ot   ObjectType of which a string representation is wanted
 * @return   the ObjectType as localized string
 */
QString UMLObject::toI18nString(ObjectType t)
{
    QString name;

    switch (t) {
    case UMLObject::ot_Actor:
        name = i18n("Actor &name:");
        break;
    case  UMLObject::ot_Artifact:
        name = i18n("Artifact &name:");
        break;
    case UMLObject::ot_Association:
        name = i18n("Association &name:");
        break;
    case UMLObject::ot_Class:
        name = i18n("Class &name:");
        break;
    case  UMLObject::ot_Component:
        name = i18n("Component &name:");
        break;
    case  UMLObject::ot_Datatype:
        name = i18n("Datatype &name:");
        break;
    case  UMLObject::ot_Entity:
        name = i18n("Entity &name:");
        break;
    case  UMLObject::ot_Enum:
        name = i18n("Enum &name:");
        break;
    case  UMLObject::ot_Folder:
        name = i18n("Folder &name:");
        break;
    case  UMLObject::ot_Interface:
        name = i18n("Interface &name:");
        break;
    case  UMLObject::ot_Node:
        name = i18n("Node &name:");
        break;
    case  UMLObject::ot_Package:
        name = i18n("Package &name:");
        break;
    case  UMLObject::ot_Port:
        name = i18n("Port &name:");
        break;
    case  UMLObject::ot_Stereotype:
        name = i18n("Stereotype &name:");
        break;
    case  UMLObject::ot_UseCase:
        name = i18n("Use case &name:");
        break;
    case UMLObject::ot_Instance:
        name = i18n("Instance name:");
        break;

    default:
        name = QLatin1String("<unknown> &name:");
        uWarning() << "unknown object type";
        break;
    }
    return name;
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

//only required for getting types
#include "actor.h"
#include "artifact.h"
#include "association.h"
#include "attribute.h"
#include "umlcanvasobject.h"
#include "category.h"
#include "checkconstraint.h"
#include "classifier.h"
#include "component.h"
#include "datatype.h"
#include "entity.h"
#include "entityattribute.h"
#include "entityconstraint.h"
#include "enum.h"
#include "enumliteral.h"
#include "folder.h"
#include "foreignkeyconstraint.h"
#include "instance.h"
#include "instanceattribute.h"
#include "node.h"
#include "operation.h"
#include "package.h"
#include "port.h"
#include "umlrole.h"
#include "stereotype.h"
#include "template.h"
#include "uniqueconstraint.h"
#include "usecase.h"


UMLActor* UMLObject::asUMLActor() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLActor*>(this); }
UMLArtifact* UMLObject::asUMLArtifact() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLArtifact*>(this); }
UMLAssociation* UMLObject::asUMLAssociation() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLAssociation*>(this); }
UMLAttribute* UMLObject::asUMLAttribute() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLAttribute*>(this); }
UMLCanvasObject* UMLObject::asUMLCanvasObject() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLCanvasObject*>(this); }
UMLCategory* UMLObject::asUMLCategory() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLCategory*>(this); }
UMLCheckConstraint* UMLObject::asUMLCheckConstraint() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLCheckConstraint*>(this); }
UMLClassifier* UMLObject::asUMLClassifier() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLClassifier*>(this); }
UMLClassifierListItem *UMLObject::asUMLClassifierListItem() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLClassifierListItem*>(this); }
UMLComponent* UMLObject::asUMLComponent() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLComponent*>(this); }
UMLDatatype *UMLObject::asUMLDatatype() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLDatatype*>(this); }
UMLEntity* UMLObject::asUMLEntity() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLEntity*>(this); }
UMLEntityAttribute* UMLObject::asUMLEntityAttribute() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLEntityAttribute*>(this); }
UMLEntityConstraint* UMLObject::asUMLEntityConstraint() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLEntityConstraint*>(this); }
UMLEnum* UMLObject::asUMLEnum() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLEnum*>(this); }
UMLEnumLiteral* UMLObject::asUMLEnumLiteral() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLEnumLiteral*>(this); }
UMLFolder* UMLObject::asUMLFolder() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLFolder*>(this); }
UMLForeignKeyConstraint* UMLObject::asUMLForeignKeyConstraint() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLForeignKeyConstraint*>(this); }
UMLInstance *UMLObject::asUMLInstance() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLInstance*>(this); }
UMLInstanceAttribute *UMLObject::asUMLInstanceAttribute() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLInstanceAttribute*>(this); }
UMLNode* UMLObject::asUMLNode() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLNode*>(this); }
UMLObject* UMLObject::asUMLObject() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLObject*>(this); }
UMLOperation* UMLObject::asUMLOperation() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLOperation*>(this); }
UMLPackage* UMLObject::asUMLPackage() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLPackage*>(this); }
UMLPort* UMLObject::asUMLPort() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLPort*>(this); }
UMLRole* UMLObject::asUMLRole() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLRole*>(this); }
UMLStereotype* UMLObject::asUMLStereotype() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLStereotype*>(this); }
UMLTemplate* UMLObject::asUMLTemplate() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLTemplate*>(this); }
UMLUniqueConstraint* UMLObject::asUMLUniqueConstraint() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLUniqueConstraint*>(this); }
UMLUseCase* UMLObject::asUMLUseCase() { uCheckPointerAndReturnIfZero(this); return dynamic_cast<UMLUseCase*>(this); }

