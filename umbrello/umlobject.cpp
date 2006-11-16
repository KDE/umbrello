/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlobject.h"
// qt/kde includes
#include <qregexp.h>
#include <kdebug.h>
#include <kapplication.h>
// app includes
#include "uniqueid.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "package.h"
#include "folder.h"
#include "stereotype.h"
#include "object_factory.h"
#include "model_utils.h"
#include "codeimport/import_utils.h"
#include "docwindow.h"
#include "dialogs/classpropdlg.h"

UMLObject::UMLObject(const UMLObject * parent, const QString &name, Uml::IDType id)
        : QObject(const_cast<UMLObject*>(parent), "UMLObject" ) {
    init();
    if (id == Uml::id_None)
        m_nId = UniqueID::gen();
    else
        m_nId = id;
    m_Name = name;
}

UMLObject::UMLObject(const QString &name, Uml::IDType id)
        :  QObject(UMLApp::app()->getDocument()) {
    init();
    if (id == Uml::id_None)
        m_nId = UniqueID::gen();
    else
        m_nId = id;
    m_Name = name;
}

UMLObject::UMLObject(const UMLObject * parent)
        : QObject(const_cast<UMLObject*>(parent)) {
    init();
}

UMLObject::~UMLObject() {
}

void UMLObject::init() {
    m_BaseType = Uml::ot_UMLObject;
    m_nId = Uml::id_None;
    m_pUMLPackage = NULL;
    m_Name = "";
    m_Vis = Uml::Visibility::Public;
    m_pStereotype = NULL;
    m_Doc = "";
    m_bAbstract = false;
    m_bStatic = false;
    m_bInPaste = false;
    m_bCreationWasSignalled = false;
    m_pSecondary = NULL;
}

bool UMLObject::showProperties(int page, bool assoc) {
    DocWindow *docwindow = UMLApp::app()->getDocWindow();
    docwindow->updateDocumentation(false);
    ClassPropDlg* dlg = new ClassPropDlg((QWidget*)UMLApp::app(), this, page, assoc);
    bool modified = false;
    if (dlg->exec()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->getDocument()->setModified(true);
        modified = true;
    }
    dlg->close(true); //wipe from memory
    return modified;
}

bool UMLObject::acceptAssociationType(Uml::Association_Type)
{// A UMLObject accepts nothing. This should be reimplemented by the subclasses
    return false;
}

void UMLObject::setID(Uml::IDType NewID) {
    m_nId = NewID;
    emit modified();
}

void UMLObject::setName(const QString &strName) {
    m_Name = strName;
    emit modified();
}

QString UMLObject::getName() const {
    return m_Name;
}

QString UMLObject::getFullyQualifiedName(QString separator,
                                         bool includeRoot /* = false */) const {
    QString fqn;
    if (m_pUMLPackage) {
        bool skipPackage = false;
        if (!includeRoot) {
            UMLDoc *umldoc = UMLApp::app()->getDocument();
            if (m_pUMLPackage == umldoc->getDatatypeFolder()) {
                skipPackage = true;
            } else {
                for (int i = 0; i < Uml::N_MODELTYPES; i++) {
                    const Uml::Model_Type mt = (Uml::Model_Type)i;
                    if (m_pUMLPackage == umldoc->getRootFolder(mt)) {
                        skipPackage = true;
                        break;
                    }
                }
            }
        }
        if (!skipPackage) {
            if (separator.isEmpty())
                separator = UMLApp::app()->activeLanguageScopeSeparator();
            fqn = m_pUMLPackage->getFullyQualifiedName(separator, includeRoot);
            fqn.append(separator);
        }
    }
    fqn.append(m_Name);
    return fqn;
}

bool UMLObject::operator==(UMLObject & rhs ) {
    if( this == &rhs )
        return true;

    //don't compare IDs, these are program specific and
    //don't mean the objects are the same
    //***** CHECK: Who put in this comment? What was the reason?
    //***** Currently some operator== in umbrello compare the IDs
    //***** while others don't.

    if( m_Name != rhs.m_Name )
        return false;

    // Packages create different namespaces, therefore they should be
    // part of the equality test.
    if( m_pUMLPackage != rhs.m_pUMLPackage )
        return false;

    // Making the type part of an object's identity has its problems:
    // Not all programming languages support declarations of the same
    // name but different type.
    // In such cases, the code generator is responsible for generating
    // the appropriate error message.
    if( m_BaseType != rhs.m_BaseType )
        return false;

    // The documentation should not be part of the equality test.
    // If two objects are the same but differ only in their documentation,
    // what does that mean?
    //if( m_Doc != rhs.m_Doc )
    //  return false;

    // The scope should not be part of the equality test.
    // What does it mean if two objects are the same but differ in their
    // scope? - I'm not aware of any programming language that would
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

void UMLObject::copyInto(UMLObject *rhs) const
{
    // Data members with copy constructor
    rhs->m_Doc = m_Doc;
    rhs->m_pStereotype = m_pStereotype;
    rhs->m_bAbstract = m_bAbstract;
    rhs->m_bStatic = m_bStatic;
    rhs->m_BaseType = m_BaseType;
    rhs->m_Vis = m_Vis;
    rhs->m_pUMLPackage = m_pUMLPackage;

    // We don't want the same name existing twice.
    rhs->m_Name = Model_Utils::uniqObjectName(m_BaseType, m_pUMLPackage, m_Name);

    // Create a new ID.
    rhs->m_nId = UniqueID::gen();

    // Hope that the parent from QObject is okay.
    if (rhs->parent() != parent())
        kdDebug() << "copyInto has a wrong parent" << endl;
}


bool UMLObject::getAbstract() const{
    return m_bAbstract;
}

void UMLObject::setAbstract(bool bAbstract) {
    m_bAbstract = bAbstract;
    emit modified();
}

void UMLObject::setInPaste(bool bInPaste /* =true */) {
    m_bInPaste = bInPaste;
}

/** Returns true if this UMLObject has classifier scope, otherwise false (the default). */
bool UMLObject::getStatic() const
{
    return m_bStatic;
}
/** Sets the value for m_bStatic. */
void UMLObject::setStatic(bool bStatic)
{
    m_bStatic = bStatic;
    emit modified();
}

void UMLObject::emitModified()
{
    emit modified();
}

void UMLObject::setDoc(const QString &d) {
    m_Doc = d;
    emit modified();
}

Uml::Object_Type UMLObject::getBaseType() const {
    return m_BaseType;
}

void UMLObject::setBaseType(Uml::Object_Type ot) {
    m_BaseType = ot;
}

Uml::IDType UMLObject::getID() const {
    return m_nId;
}

QString UMLObject::getDoc() const {
    return m_Doc;
}

Uml::Visibility UMLObject::getVisibility() const {
    return m_Vis;
}

void UMLObject::setVisibility(Uml::Visibility s) {
    m_Vis = s;
    emit modified();
}

void UMLObject::setUMLStereotype(UMLStereotype *stereo) {
    if (stereo == m_pStereotype)
        return;
    if (stereo) {
        stereo->incrRefCount();
    }
    if (m_pStereotype) {
        m_pStereotype->decrRefCount();
        if (m_pStereotype->refCount() == 0) {
            UMLDoc *pDoc = UMLApp::app()->getDocument();
            pDoc->removeStereotype(m_pStereotype);
            delete m_pStereotype;
        }
    }
    m_pStereotype = stereo;
    // TODO: don't emit modified() if predefined folder
    emit modified();
}

void UMLObject::setStereotype(const QString &_name) {
    if (_name.isEmpty()) {
        setUMLStereotype(NULL);
        return;
    }
    UMLDoc *pDoc = UMLApp::app()->getDocument();
    UMLStereotype *s = pDoc->findOrCreateStereotype(_name);
    setUMLStereotype(s);
}

void UMLObject::setPackage(const QString &_name) {
    UMLObject *pkgObj = NULL;
    if (!_name.isEmpty()) {
        UMLDoc* umldoc = UMLApp::app()->getDocument();
        pkgObj = umldoc->findUMLObject(_name);
        if (pkgObj == NULL) {
            kdDebug() << "UMLObject::setPackage: creating UMLPackage "
                << _name << " for " << m_Name << endl;
            pkgObj = Import_Utils::createUMLObject(Uml::ot_Package, _name);
        } else {
            const Uml::Object_Type ot = pkgObj->getBaseType();
            if (ot != Uml::ot_Package && ot != Uml::ot_Folder && ot != Uml::ot_Component) {
                kdError() << "UMLObject::setPackage(" << m_Name << "): "
                    << "existing " << _name << " is not a container" << endl;
                // This should not happen - if it does, there may be further problems.
                // A container name should not overlap with another name in the same scope.
                pkgObj = Import_Utils::createUMLObject(Uml::ot_Package, _name);
            }
        }
    }
    setUMLPackage( static_cast<UMLPackage *>(pkgObj) );
}

void UMLObject::setUMLPackage(UMLPackage* pPkg) {
    m_pUMLPackage = pPkg;
    emit modified();
}

const UMLStereotype * UMLObject::getUMLStereotype() {
    return m_pStereotype;
}

QString UMLObject::getStereotype(bool includeAdornments /* = false */) const {
    if (m_pStereotype == NULL)
        return "";
    QString name = m_pStereotype->getName();
    if (includeAdornments)
        name = QString::fromUtf8("«") + name + QString::fromUtf8("»");
    return name;
}

QString UMLObject::getPackage(QString separator, bool includeRoot) {
    QString fqn = getFullyQualifiedName(separator, includeRoot);
    if (!fqn.contains(separator))
        return "";
    QString scope = fqn.left(fqn.length() - separator.length() - m_Name.length());
    return scope;
}

UMLPackageList UMLObject::getPackages(bool includeRoot) const {
    UMLPackageList pkgList;
    UMLPackage* pkg = m_pUMLPackage;
    while (pkg != NULL) {
        pkgList.prepend(pkg);
        pkg = pkg->getUMLPackage();
    }
    if (!includeRoot)
        pkgList.removeFirst();
    return pkgList;
}

UMLPackage* UMLObject::getUMLPackage() {
    return m_pUMLPackage;
}

QString UMLObject::getSecondaryId() const {
    return m_SecondaryId;
}

void UMLObject::setSecondaryId(QString id) {
    m_SecondaryId = id;
}

QString UMLObject::getSecondaryFallback() const {
    return m_SecondaryFallback;
}

void UMLObject::setSecondaryFallback(QString id) {
    m_SecondaryFallback = id;
}

void UMLObject::maybeSignalObjectCreated() {
    if (!m_bCreationWasSignalled &&
            m_BaseType != Uml::ot_Stereotype &&
            m_BaseType != Uml::ot_Association &&
            m_BaseType != Uml::ot_Role) {
        m_bCreationWasSignalled = true;
        UMLDoc* umldoc = UMLApp::app()->getDocument();
        umldoc->signalUMLObjectCreated(this);
    }
}

bool UMLObject::resolveRef() {
    if (m_pSecondary || (m_SecondaryId.isEmpty() && m_SecondaryFallback.isEmpty())) {
        maybeSignalObjectCreated();
        return true;
    }
#ifdef VERBOSE_DEBUGGING
    kdDebug() << "UMLObject::resolveRef(" << m_Name << "): m_SecondaryId is "
              << m_SecondaryId << endl;
#endif
    UMLDoc *pDoc = UMLApp::app()->getDocument();
    // In the new, XMI standard compliant save format,
    // the type is the xmi.id of a UMLClassifier.
    if (! m_SecondaryId.isEmpty()) {
        m_pSecondary = pDoc->findObjectById(STR2ID(m_SecondaryId));
        if (m_pSecondary != NULL) {
            if (m_pSecondary->getBaseType() == Uml::ot_Stereotype) {
                m_pStereotype = static_cast<UMLStereotype*>(m_pSecondary);
                m_pStereotype->incrRefCount();
                m_pSecondary = NULL;
            }
            m_SecondaryId = "";
            maybeSignalObjectCreated();
            return true;
        }
    }
    if (m_SecondaryFallback.isEmpty()) {
        kdError() << "UMLObject::resolveRef(" << m_Name
                  << "): cannot find type with id "
                  << m_SecondaryId << endl;
        return false;
    }
#ifdef VERBOSE_DEBUGGING
    kdDebug() << "UMLObject::resolveRef(" << m_Name 
              << "): could not resolve secondary ID " << m_SecondaryId
              << ", using secondary fallback " << m_SecondaryFallback
              << endl;
#endif
    m_SecondaryId = m_SecondaryFallback;
    // Assume we're dealing with the older Umbrello format where
    // the type name was saved in the "type" attribute rather
    // than the xmi.id of the model object of the attribute type.
    m_pSecondary = pDoc->findUMLObject( m_SecondaryId, Uml::ot_UMLObject, this );
    if (m_pSecondary) {
        m_SecondaryId = "";
        maybeSignalObjectCreated();
        return true;
    }
    //pDoc->setIsOldFile(true);
    // Work around Object_Factory::createUMLObject()'s incapability
    // of on-the-fly scope creation:
    if (m_SecondaryId.contains("::")) {
        // TODO: Merge Import_Utils::createUMLObject() into Object_Factory::createUMLObject()
        m_pSecondary = Import_Utils::createUMLObject(Uml::ot_UMLObject, m_SecondaryId, NULL);
        if (m_pSecondary) {
            if (Import_Utils::newUMLObjectWasCreated()) {
                maybeSignalObjectCreated();
                kapp->processEvents();
                kdDebug() << "UMLObject::resolveRef: Import_Utils::createUMLObject() "
                          << "created a new type for " << m_SecondaryId << endl;
            } else {
                kdDebug() << "UMLObject::resolveRef: Import_Utils::createUMLObject() "
                          << "returned an existing type for " << m_SecondaryId << endl;
            }
            m_SecondaryId = "";
            return true;
        }
        kdError() << "UMLObject::resolveRef: Import_Utils::createUMLObject() "
                  << "failed to create a new type for " << m_SecondaryId << endl;
        return false;
    }
    kdDebug() << "UMLObject::resolveRef: Creating new type for "
    << m_SecondaryId << endl;
    // This is very C++ specific - we rely on  some '*' or
    // '&' to decide it's a ref type. Plus, we don't recognize
    // typedefs of ref types.
    bool isReferenceType = ( m_SecondaryId.contains('*') ||
                             m_SecondaryId.contains('&') );
    Uml::Object_Type ot = Uml::ot_Class;
    if (isReferenceType) {
        ot = Uml::ot_Datatype;
    } else {
        if (Model_Utils::isCommonDataType(m_SecondaryId))
            ot = Uml::ot_Datatype;
    }
    m_pSecondary = Object_Factory::createUMLObject(ot, m_SecondaryId, NULL);
    if (m_pSecondary == NULL)
        return false;
    m_SecondaryId = "";
    maybeSignalObjectCreated();
    //kapp->processEvents();
    return true;
}

QDomElement UMLObject::save( const QString &tag, QDomDocument & qDoc ) {
    /*
      Call as the first action of saveToXMI() in child class:
      This creates the QDomElement with which to work.
    */
    QDomElement qElement = qDoc.createElement(tag);
    qElement.setAttribute( "isSpecification", "false" );
    if (m_BaseType != Uml::ot_Association &&
            m_BaseType != Uml::ot_Role &&
            m_BaseType != Uml::ot_Attribute) {
        qElement.setAttribute( "isLeaf", "false" );
        qElement.setAttribute( "isRoot", "false" );
        if (m_bAbstract)
            qElement.setAttribute( "isAbstract", "true" );
        else
            qElement.setAttribute( "isAbstract", "false" );
    }
    qElement.setAttribute( "xmi.id", ID2STR(m_nId) );
    qElement.setAttribute( "name", m_Name );
    if (m_BaseType != Uml::ot_Operation &&
            m_BaseType != Uml::ot_Role &&
            m_BaseType != Uml::ot_Attribute) {
        Uml::IDType nmSpc;
        if (m_pUMLPackage)
            nmSpc = m_pUMLPackage->getID();
        else
            nmSpc = UMLApp::app()->getDocument()->getModelID();
        qElement.setAttribute( "namespace", ID2STR(nmSpc) );
    }
    if (! m_Doc.isEmpty())
        qElement.setAttribute( "comment", m_Doc );  //CHECK: uml13.dtd compliance
#ifdef XMI_FLAT_PACKAGES
    if (m_pUMLPackage)             //FIXME: uml13.dtd compliance
        qElement.setAttribute( "package", m_pUMLPackage->getID() );
#endif
    QString visibility = m_Vis.toString(false);
    qElement.setAttribute( "visibility", visibility);
    if (m_pStereotype != NULL)
        qElement.setAttribute( "stereotype", ID2STR(m_pStereotype->getID()) );
    if (m_bStatic)
        qElement.setAttribute( "ownerScope", "classifier" );
    /* else
        qElement.setAttribute( "ownerScope", "instance" );
     *** ownerScope defaults to instance if not set **********/
    return qElement;
}

bool UMLObject::load( QDomElement& ) {
    // This body is not usually executed because child classes
    // overwrite the load method.
    return true;
}

bool UMLObject::loadFromXMI( QDomElement & element) {
    UMLDoc* umldoc = UMLApp::app()->getDocument();
    if (umldoc == NULL) {
        kdError() << "UMLObject::loadFromXMI: umldoc is NULL" << endl;
        return false;
    }
    // Read the name first so that if we encounter a problem, the error
    // message can say the name.
    m_Name = element.attribute( "name", "" );
    QString id = element.attribute( "xmi.id", "" );
    if (id.isEmpty() || id == "-1") {
        if (m_BaseType == Uml::ot_Role) {
            // Before version 1.4, Umbrello did not save the xmi.id
            // of UMLRole objects.
            m_nId = UniqueID::gen();
        } else {
            kdError() << "UMLObject::loadFromXMI(" << m_Name
            << "): nonexistent or illegal xmi.id" << endl;
            return false;
        }
    } else {
        m_nId = STR2ID(id);
    }

    if (element.hasAttribute("documentation"))  // for bkwd compat.
        m_Doc = element.attribute( "documentation", "" );
    else
        m_Doc = element.attribute( "comment", "" );  //CHECK: need a UML:Comment?

    m_Vis = Uml::Visibility::Public;
    if (element.hasAttribute("scope")) {        // for bkwd compat.
        QString scope = element.attribute( "scope", "" );
        if (scope == "instance_level")         // nsuml compat.
            m_bStatic = false;
        else if (scope == "classifier_level")  // nsuml compat.
            m_bStatic = true;
        else {
            int nScope = scope.toInt();
            if (nScope >= Uml::Visibility::Public && nScope <= Uml::Visibility::Protected)
              m_Vis = (Uml::Visibility::Value)nScope;
            else
                kdError() << "UMLObject::loadFromXMI(" << m_Name
                << "): illegal scope" << endl;  // soft error
        }
    } else {
        QString visibility = element.attribute( "visibility", "public" );
        if (visibility == "private"
                || visibility == "private_vis")    // for compatibility with other programs
              m_Vis = Uml::Visibility::Private;
        else if (visibility == "protected"
                       || visibility == "protected_vis")  // for compatibility with other programs
          m_Vis = Uml::Visibility::Protected;
        else if (visibility == "implementation")
          m_Vis = Uml::Visibility::Implementation;
    }

    QString stereo = element.attribute( "stereotype", "" );
    if (!stereo.isEmpty()) {
        Uml::IDType stereoID = STR2ID(stereo);
        m_pStereotype = umldoc->findStereotypeById(stereoID);
        if (m_pStereotype)
            m_pStereotype->incrRefCount();
        else
            kdError() << "UMLObject::loadFromXMI(" << m_Name << "): "
                << "UMLStereotype " << ID2STR(stereoID)
                << " not found" << endl;
    }

    if( element.hasAttribute("abstract") ) {     // for bkwd compat.
        QString abstract = element.attribute( "abstract", "0" );
        m_bAbstract = (bool)abstract.toInt();
    } else {
        QString isAbstract = element.attribute( "isAbstract", "false" );
        m_bAbstract = (isAbstract == "true");
    }

    if( element.hasAttribute("static") ) {       // for bkwd compat.
        QString staticScope = element.attribute( "static", "0" );
        m_bStatic = (bool)staticScope.toInt();
    } else {
        QString ownerScope = element.attribute( "ownerScope", "instance" );
        m_bStatic = (ownerScope == "classifier");
    }

    /**** Handle XMI_FLAT_PACKAGES and old files *************************/
    QString pkg = element.attribute( "packageid", "-1" );
    // Some interim versions used "packageid" so test for it.
    Uml::IDType pkgId = Uml::id_None;
    if (pkg != "-1") {
        pkgId = STR2ID(pkg);
    } else {
        pkg = element.attribute( "package", "" );
        if (! pkg.isEmpty()) {
            if (pkg.contains(QRegExp("\\D")))
                // Old versions saved the package name instead of the xmi.id.
                setPackage( pkg );
            else
                pkgId = STR2ID(pkg);
        }
    }
    if (pkgId != Uml::id_None) {
        UMLObject *pkgObj = umldoc->findObjectById( pkgId );
        if (pkgObj != NULL) {
            m_pUMLPackage = dynamic_cast<UMLPackage *>(pkgObj);
            if (m_pUMLPackage == NULL)  // soft error
                kdError() << "UMLObject::loadFromXMI(" << m_Name
                << "): object of packageid "
                << ID2STR(pkgId) << " is not a package" << endl;
        } else {  // soft error
            kdError() << "UMLObject::loadFromXMI(" << m_Name
            << "): cannot resolve packageid "
            << ID2STR(pkgId) << endl;
        }
    }
    /**** End of XMI_FLAT_PACKAGES and old files handling ****************/

    // If the node has child nodes, check whether attributes can be
    // extracted from them.
    if (element.hasChildNodes()) {
        QDomNode node = element.firstChild();
        if (node.isComment())
            node = node.nextSibling();
        QDomElement elem = node.toElement();
        while( !elem.isNull() ) {
            QString tag = elem.tagName();
            if (Uml::tagEq(tag, "name")) {
                m_Name = elem.attribute("xmi.value", "");
                if (m_Name.isEmpty())
                    m_Name = elem.text();
            } else if (Uml::tagEq(tag, "visibility")) {
                QString vis = elem.attribute("xmi.value", "");
                if (vis.isEmpty())
                    vis = elem.text();
                if (vis == "private" || vis == "private_vis")
                      m_Vis = Uml::Visibility::Private;
                else if (vis == "protected" || vis == "protected_vis")
                  m_Vis = Uml::Visibility::Protected;
                else if (vis == "implementation")
                  m_Vis = Uml::Visibility::Implementation;
            } else if (Uml::tagEq(tag, "isAbstract")) {
                QString isAbstract = elem.attribute("xmi.value", "");
                if (isAbstract.isEmpty())
                    isAbstract = elem.text();
                m_bAbstract = (isAbstract == "true");
            } else if (Uml::tagEq(tag, "ownerScope")) {
                QString ownerScope = elem.attribute("xmi.value", "");
                if (ownerScope.isEmpty())
                    ownerScope = elem.text();
                m_bStatic = (ownerScope == "classifier");
            } else if (Uml::tagEq(tag, "stereotype")) {
                QString stereo = elem.attribute("xmi.value", "");
                if (stereo.isEmpty() && elem.hasChildNodes()) {
                    /* like so:
                     <UML:ModelElement.stereotype>
                       <UML:Stereotype xmi.idref = '07CD'/>
                     </UML:ModelElement.stereotype>
                     */
                    QDomNode stereoNode = elem.firstChild();
                    QDomElement stereoElem = stereoNode.toElement();
                    tag = stereoElem.tagName();
                    if (Uml::tagEq(tag, "Stereotype")) {
                        stereo = stereoElem.attribute("xmi.idref", "");
                    }
                }
                if (! stereo.isEmpty()) {
                    Uml::IDType stereoID = STR2ID(stereo);
                    m_pStereotype = umldoc->findStereotypeById(stereoID);
                    if (m_pStereotype)
                        m_pStereotype->incrRefCount();
                    else
                        m_SecondaryId = stereo;  // leave it to resolveRef()
                }
            }
            node = node.nextSibling();
            if (node.isComment())
                node = node.nextSibling();
            elem = node.toElement();
        }
    }

    // Operations, attributes, enum literals, templates, stereotypes,
    // and association role objects get added and signaled elsewhere.
    if (m_BaseType != Uml::ot_Operation && m_BaseType != Uml::ot_Attribute &&
            m_BaseType != Uml::ot_EnumLiteral && m_BaseType != Uml::ot_EntityAttribute &&
            m_BaseType != Uml::ot_Template && m_BaseType != Uml::ot_Stereotype &&
            m_BaseType != Uml::ot_Role) {
        if (m_bInPaste) {
            m_pUMLPackage = NULL;  // forget any old parent
            UMLListView *listView = UMLApp::app()->getListView();
            UMLListViewItem *parentItem = (UMLListViewItem*)listView->currentItem();
            if (parentItem) {
                Uml::ListView_Type lvt = parentItem->getType();
                if (Model_Utils::typeIsContainer(lvt) ||
                        lvt == Uml::lvt_Class ||
                        lvt == Uml::lvt_Interface) {
                    UMLObject *o = parentItem->getUMLObject();
                    m_pUMLPackage = static_cast<UMLPackage*>( o );
                }
            }
        } else if (m_pUMLPackage) {
            m_pUMLPackage->addObject(this);
        } else if (umldoc->rootFolderType(this) == Uml::N_MODELTYPES) {
            // m_pUMLPackage is not set on the root folders.
            kdDebug() << "UMLObject::loadFromXMI(" << m_Name << "): m_pUMLPackage is not set"
                << endl;
        }
    }
    return load(element);
}

kdbgstream& operator<< (kdbgstream& s, const UMLObject& a) {
    s << a.getName();
    return s;
}

#include "umlobject.moc"
