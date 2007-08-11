/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umllistviewitem.h"

// system includes
#include <cstdlib>

// qt/kde includes
#include <qfile.h>
#include <qregexp.h>
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

// app includes
#include "folder.h"
#include "classifier.h"
#include "template.h"
#include "attribute.h"
#include "operation.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlobjectlist.h"
#include "umlview.h"
#include "model_utils.h"
#include "uniqueid.h"
#include "uml.h"

UMLListView* UMLListViewItem::s_pListView = 0;

UMLListViewItem::UMLListViewItem( UMLListView * parent, const QString &name,
                                  Uml::ListView_Type t, UMLObject* o)
        : QListViewItem(parent, name) {
    init(parent);
    m_Type = t;
    m_pObject = o;
    if (o)
        m_nId = o->getID();
    setIcon(Uml::it_Home);
    setText( name );
    setRenameEnabled( 0, false );
}

UMLListViewItem::UMLListViewItem(UMLListView * parent)
        : QListViewItem(parent) {
    init(parent);
    if (parent == NULL)
        kDebug() << "UMLListViewItem constructor called with a NULL listview parent" << endl;
}

UMLListViewItem::UMLListViewItem(UMLListViewItem * parent)
        : QListViewItem(parent)  {
    init();
}

UMLListViewItem::UMLListViewItem(UMLListViewItem * parent, const QString &name, Uml::ListView_Type t,UMLObject*o)
        : QListViewItem(parent, name) {
    init();
    m_Type = t;
    m_pObject = o;
    if( !o ) {
        m_nId = Uml::id_None;
        updateFolder();
    } else {
        UMLClassifierListItem *umlchild = dynamic_cast<UMLClassifierListItem*>(o);
        if (umlchild)
            parent->addClassifierListItem(umlchild, this);
        updateObject();
        m_nId = o->getID();
    }
    setRenameEnabled( 0, !Model_Utils::typeIsRootView(t) );
    setText( name );
}

UMLListViewItem::UMLListViewItem(UMLListViewItem * parent, const QString &name, Uml::ListView_Type t,Uml::IDType id)
        : QListViewItem(parent, name) {
    init();
    m_Type = t;
    m_nId = id;
    switch (m_Type) {
    case Uml::lvt_Collaboration_Diagram:
        setIcon(Uml::it_Diagram_Collaboration);
        break;
    case Uml::lvt_Class_Diagram:
        setIcon(Uml::it_Diagram_Class);
        break;
    case Uml::lvt_State_Diagram:
        setIcon(Uml::it_Diagram_State);
        break;
    case Uml::lvt_Activity_Diagram:
        setIcon(Uml::it_Diagram_Activity);
        break;
    case Uml::lvt_Sequence_Diagram:
        setIcon(Uml::it_Diagram_Sequence);
        break;
    case Uml::lvt_Component_Diagram:
        setIcon(Uml::it_Diagram_Component);
        break;
    case Uml::lvt_Deployment_Diagram:
        setIcon(Uml::it_Diagram_Deployment);
        break;
    case Uml::lvt_UseCase_Diagram:
        setIcon(Uml::it_Diagram_Usecase);
        break;
    default:
        setIcon(Uml::it_Diagram);
    }
    /*
        Constructor also used by folder so just make sure we don't need to
        to set pixmap to folder.  doesn't hurt diagrams.
    */
    updateFolder();
    setText( name );
    setRenameEnabled( 0, true );
}

UMLListViewItem::~UMLListViewItem() {}

void UMLListViewItem::init(UMLListView * parent) {
    m_Type = Uml::lvt_Unknown;
    m_bCreating = false;
    m_pObject = NULL;
    m_nId = Uml::id_None;
    m_nChildren = 0;
    if (s_pListView == NULL && parent != NULL) {
        kDebug() << "UMLListViewItem::init: s_pListView still NULL, setting it now "
                  << endl;
        s_pListView = parent;
    }
}

Uml::ListView_Type UMLListViewItem::getType() const {
    return m_Type;
}

void UMLListViewItem::addClassifierListItem(UMLClassifierListItem *child, UMLListViewItem *childItem) {
    m_comap[child] = childItem;
}

void UMLListViewItem::deleteChildItem(UMLClassifierListItem *child) {
    UMLListViewItem *childItem = findChildObject(child);
    if (childItem == NULL) {
        kError() << "UMLListViewItem::deleteChildItem(" << child->getName()
                  << "): child listview item not found" << endl;
        return;
    }
    m_comap.remove(child);
    delete childItem;
}

Uml::IDType UMLListViewItem::getID() const {
    if (m_pObject)
        return m_pObject->getID();
    return m_nId;
}

void UMLListViewItem::setID(Uml::IDType id) {
    if (m_pObject) {
        Uml::IDType oid = m_pObject->getID();
        if (id != Uml::id_None && oid != id)
            kDebug() << "UMLListViewItem::setID: new id " << ID2STR(id)
                << " does not agree with object id " << ID2STR(oid) << endl;
    }
    m_nId = id;
}

bool UMLListViewItem::isOwnParent(Uml::IDType listViewItemID) {
    QListViewItem *lvi = (QListViewItem*)s_pListView->findItem(listViewItemID);
    if (lvi == NULL) {
        kError() << "UMLListViewItem::isOwnParent: ListView->findItem("
            << ID2STR(listViewItemID) << ") returns NULL" << endl;
        return true;
    }
    for (QListViewItem *self = (QListViewItem*)this; self; self = self->parent()) {
        if (lvi == self)
            return true;
    }
    return false;
}

void UMLListViewItem::updateObject() {
    if( m_pObject == NULL )
        return;

    Uml::Visibility scope = m_pObject->getVisibility();
    Uml::Object_Type ot = m_pObject->getBaseType();
    QString modelObjText = m_pObject->getName();
    if (Model_Utils::isClassifierListitem(ot)) {
        UMLClassifierListItem *pNarrowed = static_cast<UMLClassifierListItem*>(m_pObject);
        modelObjText = pNarrowed->toString(Uml::st_SigNoVis);
    }
    setText(modelObjText);

    Uml::Icon_Type icon = Uml::it_Home;
    switch (ot) {
    case Uml::ot_Package:
        if (m_pObject->getStereotype() == "subsystem")
            icon = Uml::it_Subsystem;
        else
            icon = Uml::it_Package;
        break;
/*
    case Uml::ot_Folder:
        {
            Uml::ListView_Type lvt = Model_Utils::convert_OT_LVT(m_pObject);
            icon = Model_Utils::convert_LVT_IT(lvt);
        }
        break;
 */
    case Uml::ot_Operation:
        if (scope == Uml::Visibility::Public)
            icon = Uml::it_Public_Method;
        else if (scope == Uml::Visibility::Private)
            icon = Uml::it_Private_Method;
        else if (scope == Uml::Visibility::Implementation)
            icon = Uml::it_Private_Method;
        else
            icon = Uml::it_Protected_Method;
        break;

    case Uml::ot_Attribute:
    case Uml::ot_EntityAttribute:
        if (scope == Uml::Visibility::Public)
            icon = Uml::it_Public_Attribute;
        else if (scope == Uml::Visibility::Private)
            icon = Uml::it_Private_Attribute;
        else if (scope == Uml::Visibility::Implementation)
            icon = Uml::it_Private_Attribute;
        else
            icon = Uml::it_Protected_Attribute;
        break;
    default:
        icon = Model_Utils::convert_LVT_IT(m_Type);
        break;
    }//end switch
    if (icon)
        setIcon(icon);
}

void UMLListViewItem::updateFolder() {
    Uml::Icon_Type icon = Model_Utils::convert_LVT_IT(m_Type);
    if (icon) {
        if (Model_Utils::typeIsFolder(m_Type))
            icon = (Uml::Icon_Type)((int)icon + (int)isOpen());
        setIcon(icon);
    }
}

void UMLListViewItem::setOpen( bool open ) {
    QListViewItem::setOpen( open );
    updateFolder();
}

void UMLListViewItem::setText(const QString &newText) {
    m_Label = newText;
    QListViewItem::setText(0, newText);
}

QString UMLListViewItem::getText() const {
    return m_Label;
}

void UMLListViewItem::setIcon(Uml::Icon_Type iconType) {
    setPixmap(0, s_pListView->getPixmap(iconType));
}

void UMLListViewItem::okRename( int col ) {
    QListViewItem::okRename( col );
    UMLDoc* doc = s_pListView->getDocument();
    if (m_bCreating) {
        m_bCreating = false;
        QString savedLabel = m_Label;
        m_Label = text(col);
        if ( s_pListView->itemRenamed( this, col ) ) {
            s_pListView->ensureItemVisible(this);
            doc->setModified(true);
        } else {
            delete this;
        }
        return;
    }
    QString newText = text( col );
    if ( newText == m_Label ) {
        return;
    }
    if( newText.isEmpty() ) {
        cancelRenameWithMsg();
        return;
    }
    switch( m_Type ) {
    case Uml::lvt_UseCase:
    case Uml::lvt_Actor:
    case Uml::lvt_Class:
    case Uml::lvt_Package:
    case Uml::lvt_UseCase_Folder:
    case Uml::lvt_Logical_Folder:
    case Uml::lvt_Component_Folder:
    case Uml::lvt_Deployment_Folder:
    case Uml::lvt_EntityRelationship_Folder:
    case Uml::lvt_Interface:
    case Uml::lvt_Datatype:
    case Uml::lvt_Enum:
    case Uml::lvt_EnumLiteral:
    case Uml::lvt_Subsystem:
    case Uml::lvt_Component:
    case Uml::lvt_Node:
        if (m_pObject == NULL || !doc->isUnique(newText)) {
            cancelRenameWithMsg();
            return;
        }
        m_pObject -> setName( newText );
        doc->setModified(true);
        m_Label = newText;
        break;

    case Uml::lvt_Operation:
        {
            if (m_pObject == NULL) {
                cancelRenameWithMsg();
                return;
            }
            UMLOperation *op = static_cast<UMLOperation*>(m_pObject);
            UMLClassifier *parent = static_cast<UMLClassifier *>( op -> parent() );
            Model_Utils::OpDescriptor od;
            Model_Utils::Parse_Status st = Model_Utils::parseOperation(newText, od, parent);
            if (st == Model_Utils::PS_OK) {
                // TODO: Check that no operation with the exact same profile exists.
                op->setName( od.m_name );
                op->setType( od.m_pReturnType );
                UMLAttributeList parmList = op->getParmList();
                const unsigned newParmListCount = parmList.count();
                if (newParmListCount > od.m_args.count()) {
                    // Remove parameters at end of of list that no longer exist.
                    for (unsigned i = od.m_args.count(); i < newParmListCount; i++) {
                        UMLAttribute *a = parmList.at(i);
                        op->removeParm(a, false);
                    }
                }
                Model_Utils::NameAndType_ListIt lit = od.m_args.begin();
                for (unsigned i = 0; lit != od.m_args.end(); ++lit, ++i) {
                    const Model_Utils::NameAndType& nm_tp = *lit;
                    UMLAttribute *a;
                    if (i < newParmListCount) {
                        a = parmList.at(i);
                    } else {
                        a = new UMLAttribute(op);
                        a->setID( UniqueID::gen() );
                    }
                    a->setName(nm_tp.m_name);
                    a->setType(nm_tp.m_type);
                    a->setParmKind(nm_tp.m_direction);
                    a->setInitialValue(nm_tp.m_initialValue);
                    if (i >= newParmListCount) {
                        op->addParm(a);
                    }
                }
                m_Label = op->toString(Uml::st_SigNoVis);
            } else {
                KMessageBox::error( kapp->mainWidget(),
                                    Model_Utils::psText(st),
                                    i18n("Rename canceled") );
            }
            QListViewItem::setText(0, m_Label);
            break;
        }

    case Uml::lvt_Attribute:
    case Uml::lvt_EntityAttribute:
        {
            if (m_pObject == NULL) {
                cancelRenameWithMsg();
                return;
            }
            UMLClassifier *parent = static_cast<UMLClassifier*>(m_pObject->parent());
            Model_Utils::NameAndType nt;
            Uml::Visibility vis;
            Model_Utils::Parse_Status st;
            st = Model_Utils::parseAttribute(newText, nt, parent, &vis);
            if (st == Model_Utils::PS_OK) {
                UMLObject *exists = parent->findChildObject(newText);
                if (exists) {
                    cancelRenameWithMsg();
                    return;
                }
                m_pObject->setName(nt.m_name);
                UMLAttribute *pAtt = static_cast<UMLAttribute*>(m_pObject);
                pAtt->setType(nt.m_type);
                pAtt->setVisibility(vis);
                pAtt->setParmKind(nt.m_direction);
                pAtt->setInitialValue(nt.m_initialValue);
                m_Label = pAtt->toString(Uml::st_SigNoVis);
            } else {
                KMessageBox::error( kapp->mainWidget(),
                                    Model_Utils::psText(st),
                                    i18n("Rename canceled") );
            }
            QListViewItem::setText(0, m_Label);
            break;
        }

    case Uml::lvt_Template:
        {
            if (m_pObject == NULL) {
                cancelRenameWithMsg();
                return;
            }
            UMLClassifier *parent = static_cast<UMLClassifier*>(m_pObject->parent());
            Model_Utils::NameAndType nt;
            Model_Utils::Parse_Status st = Model_Utils::parseTemplate(newText, nt, parent);
            if (st == Model_Utils::PS_OK) {
                UMLObject *exists = parent->findChildObject(newText);
                if (exists) {
                    cancelRenameWithMsg();
                    return;
                }
                m_pObject->setName(nt.m_name);
                UMLTemplate *tmpl = static_cast<UMLTemplate*>(m_pObject);
                tmpl->setType(nt.m_type);
                m_Label = tmpl->toString(Uml::st_SigNoVis);
            } else {
                KMessageBox::error( kapp->mainWidget(),
                                    Model_Utils::psText(st),
                                    i18n("Rename canceled") );
            }
            QListViewItem::setText(0, m_Label);
            break;
        }

    case Uml::lvt_UseCase_Diagram:
    case Uml::lvt_Class_Diagram:
    case Uml::lvt_Sequence_Diagram:
    case Uml::lvt_Collaboration_Diagram:
    case Uml::lvt_State_Diagram:
    case Uml::lvt_Activity_Diagram:
    case Uml::lvt_Component_Diagram:
    case Uml::lvt_Deployment_Diagram:
        {
            UMLView *view = doc -> findView( getID() );
            if (view == NULL) {
                cancelRenameWithMsg();
                return;
            }
            UMLView *anotherView = doc -> findView( view->getType(), newText );
            if( anotherView && anotherView -> getID() == getID() )
                anotherView = 0;
            if (anotherView) {
                cancelRenameWithMsg();
                return;
            }
            view->setName( newText );
            setText(newText);
            doc->signalDiagramRenamed(view);
            break;
        }
    default:
        KMessageBox::error( kapp->mainWidget() ,
                            i18n("Renaming an item of listview type %1 is not yet implemented.").arg(m_Type),
                            i18n("Function Not Implemented") );
        QListViewItem::setText(0, m_Label);
        break;
    }
    doc->setModified(true);
}

void UMLListViewItem::cancelRenameWithMsg() {
    KMessageBox::error( kapp->mainWidget() ,
                        i18n("The name you entered was invalid.\nRenaming process has been canceled."),
                        i18n("Name Not Valid") );
    QListViewItem::setText(0, m_Label);
}

void UMLListViewItem::cancelRename(int col) {
    QListViewItem::cancelRename(col);
    if (m_bCreating) {
        s_pListView->cancelRename(this);
    }
}

// Sort the listview items by type and position within the corresponding list
// of UMLObjects. If the item does not have an UMLObject then place it last.
int UMLListViewItem::compare(QListViewItem *other, int col, bool ascending) const
{
    UMLListViewItem *ulvi = static_cast<UMLListViewItem*>(other);
    Uml::ListView_Type ourType = getType();
    Uml::ListView_Type otherType = ulvi->getType();

    if ( ourType < otherType )
        return -1;
    if ( ourType > otherType )
        return 1;
    // ourType == otherType
    const bool subItem = Model_Utils::typeIsClassifierList(ourType);
    const int alphaOrder = key(col, ascending).compare(other->key(col, ascending));
    int retval = 0;
    QString dbgPfx = "compare(type=" + QString::number((int)ourType)
                   + ", self=" + getText() + ", other=" + ulvi->getText()
                   + "): return ";
    UMLObject *otherObj = ulvi->getUMLObject();
    if (m_pObject == NULL) {
        retval = (subItem ? 1 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        kDebug() << dbgPfx << retval << " because (m_pObject==NULL)" << endl;
#endif
        return retval;
    }
    if (otherObj == NULL) {
        retval = (subItem ? -1 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        kDebug() << dbgPfx << retval << " because (otherObj==NULL)" << endl;
#endif
        return retval;
    }
    UMLClassifier *ourParent = dynamic_cast<UMLClassifier*>(m_pObject->parent());
    UMLClassifier *otherParent = dynamic_cast<UMLClassifier*>(otherObj->parent());
    if (ourParent == NULL) {
        retval = (subItem ? 1 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        kDebug() << dbgPfx << retval << " because (ourParent==NULL)" << endl;
#endif
        return retval;
    }
    if (otherParent == NULL) {
        retval = (subItem ? -1 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        kDebug() << dbgPfx << retval << " because (otherParent==NULL)" << endl;
#endif
        return retval;
    }
    if (ourParent != otherParent) {
        retval = (subItem ? 0 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        kDebug() << dbgPfx << retval << " because (ourParent != otherParent)" << endl;
#endif
        return retval;
    }
    UMLClassifierListItem *thisUmlItem = dynamic_cast<UMLClassifierListItem*>(m_pObject);
    UMLClassifierListItem *otherUmlItem = dynamic_cast<UMLClassifierListItem*>(otherObj);
    if (thisUmlItem == NULL) {
        retval = (subItem ? 1 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        kDebug() << dbgPfx << retval << " because (thisUmlItem==NULL)" << endl;
#endif
        return retval;
    }
    if (otherUmlItem == NULL) {
        retval = (subItem ? -1 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        kDebug() << dbgPfx << retval << " because (otherUmlItem==NULL)" << endl;
#endif
        return retval;
    }
    UMLClassifierListItemList items = ourParent->getFilteredList(thisUmlItem->getBaseType());
    int myIndex = items.findRef(thisUmlItem);
    int otherIndex = items.findRef(otherUmlItem);
    if (myIndex < 0) {
        retval = (subItem ? -1 : alphaOrder);
        kError() << dbgPfx << retval << " because (myIndex < 0)" << endl;
        return retval;
    }
    if (otherIndex < 0) {
        retval = (subItem ? 1 : alphaOrder);
        kError() << dbgPfx << retval << " because (otherIndex < 0)" << endl;
        return retval;
    }
    return (myIndex < otherIndex ? -1 : myIndex > otherIndex ? 1 : 0);
}

UMLListViewItem* UMLListViewItem::deepCopy(UMLListViewItem *newParent) {
    QString nm = getText();
    Uml::ListView_Type t = getType();
    UMLObject *o = getUMLObject();
    UMLListViewItem* newItem;
    if (o)
        newItem = new UMLListViewItem(newParent, nm, t, o);
    else
        newItem = new UMLListViewItem(newParent, nm, t, m_nId);
    UMLListViewItem *childItem = static_cast<UMLListViewItem*>(firstChild());
    while (childItem) {
        childItem->deepCopy(newItem);
        childItem = static_cast<UMLListViewItem*>(childItem->nextSibling());
    }
    return newItem;
}

UMLListViewItem* UMLListViewItem::findUMLObject(const UMLObject *o) {
    if (m_pObject == o)
        return this;
    UMLListViewItem *childItem = static_cast<UMLListViewItem*>(firstChild());
    while (childItem) {
        UMLListViewItem *inner = childItem->findUMLObject(o);
        if (inner)
            return inner;
        childItem = static_cast<UMLListViewItem*>(childItem->nextSibling());
    }
    return NULL;
}

UMLListViewItem* UMLListViewItem::findChildObject(UMLClassifierListItem *cli) {
    ChildObjectMap::iterator it = m_comap.find(cli);
    if (it != m_comap.end()) {
        return *it;
    }
    return NULL;
}

UMLListViewItem * UMLListViewItem::findItem(Uml::IDType id) {
    if (getID() == id)
        return this;
    UMLListViewItem *childItem = static_cast<UMLListViewItem*>(firstChild());
    while (childItem) {
        UMLListViewItem *inner = childItem->findItem(id);
        if (inner)
            return inner;
        childItem = static_cast<UMLListViewItem*>(childItem->nextSibling());
    }
    return NULL;
}

void UMLListViewItem::saveToXMI( QDomDocument & qDoc, QDomElement & qElement) {
    QDomElement itemElement = qDoc.createElement( "listitem" );
    Uml::IDType id = getID();
    QString idStr = ID2STR(id);
    //kDebug() << "UMLListViewItem::saveToXMI: id = " << idStr
    //    << ", type = " << m_Type << endl;
    if (id != Uml::id_None)
        itemElement.setAttribute( "id", idStr );
    itemElement.setAttribute( "type", m_Type );
    UMLFolder *extFolder = NULL;
    if (m_pObject == NULL) {
        if (! Model_Utils::typeIsDiagram(m_Type) && m_Type != Uml::lvt_View)
            kError() << "UMLListViewItem::saveToXMI(" << m_Label
                << "): m_pObject is NULL" << endl;
        itemElement.setAttribute( "label", m_Label );
    } else if (m_pObject->getID() == Uml::id_None) {
        if (m_Label.isEmpty()) {
            kDebug() << "UMLListViewItem::saveToXMI(): Skipping empty item"
                << endl;
            return;
        }
        kDebug() << "UMLListViewItem::saveToXMI(): saving local label "
            << m_Label << " because umlobject ID is not set" << endl;
        itemElement.setAttribute( "label", m_Label );
    } else if (m_pObject->getBaseType() == Uml::ot_Folder) {
        extFolder = static_cast<UMLFolder*>(m_pObject);
        if (!extFolder->getFolderFile().isEmpty()) {
            itemElement.setAttribute("open", "0");
            qElement.appendChild(itemElement);
            return;
        }
    }
    itemElement.setAttribute("open", isOpen());
    QDomElement folderRoot;
    UMLListViewItem *childItem = static_cast<UMLListViewItem*>( firstChild() );
    while (childItem) {
        childItem->saveToXMI(qDoc, itemElement);
        childItem = dynamic_cast<UMLListViewItem *> ( childItem->nextSibling() );
    }
    qElement.appendChild( itemElement );
}

bool UMLListViewItem::loadFromXMI(QDomElement& qElement) {
    QString id = qElement.attribute( "id", "-1" );
    QString type = qElement.attribute( "type", "-1" );
    QString label = qElement.attribute( "label", "" );
    QString open = qElement.attribute( "open", "1" );
    if (!label.isEmpty())
        setText( label );
    else if (id == "-1") {
        kError() << "UMLListViewItem::loadFromXMI: Item of type "
            << type << " has neither ID nor label" << endl;
        return false;
    }

    m_nChildren = qElement.childNodes().count();

    m_nId = STR2ID(id);
    if (m_nId != Uml::id_None)
        m_pObject = s_pListView->getDocument()->findObjectById( m_nId );
    m_Type = (Uml::ListView_Type)(type.toInt());
    if (m_pObject)
        updateObject();
    setOpen( (bool)open.toInt() );
    return true;
}

