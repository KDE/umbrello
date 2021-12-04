/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umllistviewitem.h"

// app includes
#include "debug_utils.h"
#include "folder.h"
#include "classifier.h"
#include "entity.h"
#include "template.h"
#include "attribute.h"
#include "operation.h"
#include "instanceattribute.h"
#include "entityconstraint.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlobjectlist.h"
#include "umlscene.h"
#include "umlview.h"
#include "model_utils.h"
#include "uniqueid.h"
#include "uml.h"
#include "cmds.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QDrag>
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QXmlStreamWriter>

// system includes
#include <cstdlib>

#define DBG_LVI QLatin1String("UMLListViewItem")

DEBUG_REGISTER(UMLListViewItem)

/**
 * Sets up an instance.
 *
 * @param parent   The parent to this instance.
 * @param name     The name of this instance.
 * @param t        The type of this instance.
 * @param o        The object it represents.
 */
UMLListViewItem::UMLListViewItem(UMLListView * parent, const QString &name,
                                 ListViewType t, UMLObject* o)
  : QTreeWidgetItem(parent)
{
    init();
    if (parent == 0) {
        DEBUG(DBG_LVI) << "UMLListViewItem constructor called with a null listview parent";
    }
    m_type = t;
    m_object = o;
    if (o) {
        m_id = o->id();
    }
    setIcon(Icon_Utils::it_Home);
    setText(name);
}

/**
 * Sets up an instance for subsequent loadFromXMI1().
 *
 * @param parent   The parent to this instance.
 */
UMLListViewItem::UMLListViewItem(UMLListView * parent)
  : QTreeWidgetItem(parent)
{
    init();
    if (parent == 0) {
        DEBUG(DBG_LVI) << "UMLListViewItem constructor called with a NULL listview parent";
    }
}

/**
 * Sets up an instance for subsequent loadFromXMI1().
 *
 * @param parent   The parent to this instance.
 */
UMLListViewItem::UMLListViewItem(UMLListViewItem * parent)
  : QTreeWidgetItem(parent)
{
    init();
}

/**
 * Sets up an instance.
 *
 * @param parent   The parent to this instance.
 * @param name     The name of this instance.
 * @param t        The type of this instance.
 * @param o        The object it represents.
 */
UMLListViewItem::UMLListViewItem(UMLListViewItem * parent, const QString &name, ListViewType t, UMLObject *o)
  : QTreeWidgetItem(parent)
{
    init();
    m_type = t;
    m_object = o;
    if (!o) {
        m_id = Uml::ID::None;
        updateFolder();
    } else {
        parent->addChildItem(o, this);
        updateObject();
        m_id = o->id();
    }
    setText(name);
    if (!Model_Utils::typeIsRootView(t)) {
        setFlags(flags() | Qt::ItemIsEditable);
    }
}

/**
 * Sets up an instance.
 *
 * @param parent   The parent to this instance.
 * @param name     The name of this instance.
 * @param t        The type of this instance.
 * @param id       The id of this instance.
 */
UMLListViewItem::UMLListViewItem(UMLListViewItem * parent, const QString &name, ListViewType t, Uml::ID::Type id)
  : QTreeWidgetItem(parent)
{
    init();
    m_type = t;
    m_id = id;
    switch (m_type) {
    case lvt_Collaboration_Diagram:
        setIcon(Icon_Utils::it_Diagram_Collaboration);
        break;
    case lvt_Class_Diagram:
        setIcon(Icon_Utils::it_Diagram_Class);
        break;
    case lvt_State_Diagram:
        setIcon(Icon_Utils::it_Diagram_State);
        break;
    case lvt_Activity_Diagram:
        setIcon(Icon_Utils::it_Diagram_Activity);
        break;
    case lvt_Sequence_Diagram:
        setIcon(Icon_Utils::it_Diagram_Sequence);
        break;
    case lvt_Component_Diagram:
        setIcon(Icon_Utils::it_Diagram_Component);
        break;
    case lvt_Deployment_Diagram:
        setIcon(Icon_Utils::it_Diagram_Deployment);
        break;
    case lvt_UseCase_Diagram:
        setIcon(Icon_Utils::it_Diagram_Usecase);
        break;
    case lvt_Object_Diagram:
        setIcon(Icon_Utils::it_Diagram_Object);
        break;
    default:
        setIcon(Icon_Utils::it_Diagram);
    }
    //  Constructor also used by folder so just make sure we don't need to
    //  to set pixmap to folder.  doesn't hurt diagrams.
    updateFolder();
    setText(name);
    setFlags(flags() | Qt::ItemIsEditable);
}

/**
 * Standard destructor.
 */
UMLListViewItem::~UMLListViewItem()
{
}

/**
 * Initializes key variables of the class.
 */
void UMLListViewItem::init()
{
    m_type = lvt_Unknown;
    m_object = 0;
    m_id = Uml::ID::None;
}

/**
 * Returns the signature of items that are operations.
 * @return signature of an operation item, else an empty string
 */
QString UMLListViewItem::toolTip() const
{
    UMLObject *obj = umlObject();
    if (obj) {
        switch (obj->baseType()) {
            case UMLObject::ot_Class:
                return obj->doc();
            case UMLObject::ot_Operation:
            {
                const UMLOperation *op = obj->asUMLOperation();
                return op->toString(Uml::SignatureType::ShowSig);
            }
            case UMLObject::ot_Attribute:
            {
                const UMLAttribute *at = obj->asUMLAttribute();
                return at->toString(Uml::SignatureType::ShowSig);
            }
            default:
                return QString();
        }
    }
    else {
        return QString();
    }
}

/**
 * Returns the type this instance represents.
 *
 * @return  The type this instance represents.
 */
UMLListViewItem::ListViewType UMLListViewItem::type() const
{
    return m_type;
}

/**
 * Adds the child listview item representing the given UMLObject.
 */
void UMLListViewItem::addChildItem(UMLObject *child, UMLListViewItem *childItem)
{
    if (!child) {
        uError() << "UMLListViewItem::addChildItem called with null child";
        return;
    }
    m_comap[child] = childItem;
}

/**
 * Deletes the child listview item representing the given UMLObject.
 */
void UMLListViewItem::deleteChildItem(UMLObject *child)
{
    if (!child) {
        uError() << "UMLListViewItem::deleteChildItem called with null child";
        return;
    }
    UMLListViewItem *childItem = findChildObject(child);
    if (childItem == 0) {
        uError() << child->name() << ": child listview item not found";
        return;
    }
    m_comap.remove(child);
    delete childItem;
}

void UMLListViewItem::setVisible(bool state)
{
    setHidden(!state);
}

/**
 * Returns the id this class represents.
 *
 * @return  The id this class represents.
 */
Uml::ID::Type UMLListViewItem::ID() const
{
    if (m_object) {
        return m_object->id();
    }
    return m_id;
}

/**
 * Sets the id this class represents.
 * This only sets the ID locally, not at the UMLObject that is perhaps
 * associated to this UMLListViewItem.
 * @param id   the id this class represents
 */
void UMLListViewItem::setID(Uml::ID::Type id)
{
    if (m_object) {
        Uml::ID::Type oid = m_object->id();
        if (id != Uml::ID::None && oid != id) {
            DEBUG(DBG_LVI) << "new id " << Uml::ID::toString(id) << " does not agree with object id "
                << Uml::ID::toString(oid);
        }
    }
    m_id = id;
}

/**
 * Set the UMLObject associated with this instance.
 *
 * @param obj  The object this class represents.
 */
void UMLListViewItem::setUMLObject(UMLObject * obj)
{
    m_object = obj;
}

/**
 * Return the UMLObject associated with this instance.
 *
 * @return  The object this class represents.
 */
UMLObject * UMLListViewItem::umlObject() const
{
    return m_object;
}

/**
 * Returns true if the UMLListViewItem of the given ID is a parent of
 * this UMLListViewItem.
 */
bool UMLListViewItem::isOwnParent(Uml::ID::Type listViewItemID)
{
    UMLListView* listView = static_cast<UMLListView*>(treeWidget());
    QTreeWidgetItem *lvi = static_cast<QTreeWidgetItem*>(listView->findItem(listViewItemID));
    if (lvi == 0) {
        uError() << "ListView->findItem(" << Uml::ID::toString(listViewItemID) << ") returns NULL";
        return true;
    }
    for (QTreeWidgetItem *self = static_cast<QTreeWidgetItem*>(this); self; self = self->parent()) {
        if (lvi == self)
            return true;
    }
    return false;
}

/**
 * Updates the representation of the object.
 */
void UMLListViewItem::updateObject()
{
    if (m_object == 0)
        return;

    // check if parent has been changed, remap parent if so
    UMLListViewItem *oldParent = dynamic_cast<UMLListViewItem*>(parent());
    if (oldParent && oldParent->m_object != m_object->parent() &&
                                dynamic_cast<UMLPackage*>(m_object->parent())) {
        UMLListViewItem *newParent = UMLApp::app()->listView()->findUMLObject(m_object->umlPackage());
        if (newParent) {
            oldParent->removeChild(this);
            newParent->addChild(this);
        }
    }

    Uml::Visibility::Enum scope = m_object->visibility();
    UMLObject::ObjectType ot = m_object->baseType();
    QString modelObjText = m_object->name();
    if (Model_Utils::isClassifierListitem(ot)) {
        const UMLClassifierListItem *pNarrowed = m_object->asUMLClassifierListItem();
        modelObjText = pNarrowed->toString(Uml::SignatureType::SigNoVis);
    } else if (ot == UMLObject::ot_InstanceAttribute) {
        const UMLInstanceAttribute *pNarrowed = m_object->asUMLInstanceAttribute();
        modelObjText = pNarrowed->toString();
    }
    setText(modelObjText);

    Icon_Utils::IconType icon = Icon_Utils::it_Home;
    switch (ot) {
    case UMLObject::ot_Package:
        if (m_object->stereotype() == QLatin1String("subsystem"))
            icon = Icon_Utils::it_Subsystem;
        else
            icon = Icon_Utils::it_Package;
        break;
    case UMLObject::ot_Operation:
        if (scope == Uml::Visibility::Public)
            icon = Icon_Utils::it_Public_Method;
        else if (scope == Uml::Visibility::Private)
            icon = Icon_Utils::it_Private_Method;
        else if (scope == Uml::Visibility::Implementation)
            icon = Icon_Utils::it_Private_Method;
        else
            icon = Icon_Utils::it_Protected_Method;
        break;

    case UMLObject::ot_Attribute:
    case UMLObject::ot_EntityAttribute:
    case UMLObject::ot_InstanceAttribute:
        if (scope == Uml::Visibility::Public)
            icon = Icon_Utils::it_Public_Attribute;
        else if (scope == Uml::Visibility::Private)
            icon = Icon_Utils::it_Private_Attribute;
        else if (scope == Uml::Visibility::Implementation)
            icon = Icon_Utils::it_Private_Attribute;
        else
            icon = Icon_Utils::it_Protected_Attribute;
        break;
    case UMLObject::ot_UniqueConstraint:
        m_type = Model_Utils::convert_OT_LVT(umlObject());
        icon = Model_Utils::convert_LVT_IT(m_type);
        break;

    case UMLObject::ot_Class:
        icon = Model_Utils::convert_LVT_IT(m_type, m_object);
        break;

    case UMLObject::ot_Association:
        icon = Model_Utils::convert_LVT_IT(m_type, m_object);
        break;

    default:
        icon = Model_Utils::convert_LVT_IT(m_type);
        break;
    }//end switch
    if (icon)
        setIcon(icon);
}

/**
 * Updates the icon on a folder.
 */
void UMLListViewItem::updateFolder()
{
    Icon_Utils::IconType icon = Model_Utils::convert_LVT_IT(m_type, m_object);

    if (icon) {
        if (Model_Utils::typeIsFolder(m_type)) {
            icon = (Icon_Utils::IconType)((int)icon + (int)isExpanded());
        }
        setIcon(icon);
    }
}

/**
 * Overrides default method.
 * Will call default method but also makes sure correct icon is shown.
 */
void UMLListViewItem::setOpen(bool expand)
{
    QTreeWidgetItem::setExpanded(expand);
    updateFolder();
}

/**
 * Changes the current text of column 0.
 */
void UMLListViewItem::setText(const QString &newText)
{
    setText(0, newText);
}

/**
 * Changes the current text.
 */
void UMLListViewItem::setText(int column, const QString &newText)
{
    m_label = newText;
    QTreeWidgetItem::setText(column, newText);
}

/**
 * Returns the saved text.
 */
QString UMLListViewItem::getSavedText() const
{
    return m_label;
}

/**
 * Set the pixmap corresponding to the given IconType.
 */
void UMLListViewItem::setIcon(Icon_Utils::IconType iconType)
{
    QPixmap p = Icon_Utils::SmallIcon(iconType);
    if (!p.isNull()) {
        QTreeWidgetItem::setIcon(0, QIcon(p));
    }
}

/**
 * This slot is called to finish item editing
 */
void UMLListViewItem::slotEditFinished(const QString &newText)
{
    m_label = text(0);

    DEBUG(DBG_LVI) << this << "text=" << newText;
    UMLListView* listView = static_cast<UMLListView*>(treeWidget());
    UMLDoc* doc = listView->document();
    if (newText == m_label) {
        return;
    }
    if (newText.isEmpty()) {
        cancelRenameWithMsg();
        return;
    }
    switch (m_type) {
    case lvt_UseCase:
    case lvt_Actor:
    case lvt_Class:
    case lvt_Package:
    case lvt_UseCase_Folder:
    case lvt_Logical_Folder:
    case lvt_Component_Folder:
    case lvt_Deployment_Folder:
    case lvt_EntityRelationship_Folder:
    case lvt_Interface:
    case lvt_Datatype:
    case lvt_Enum:
    case lvt_EnumLiteral:
    case lvt_Subsystem:
    case lvt_Component:
    case lvt_Port:
    case lvt_Node:
    case lvt_Category:
        if (m_object == 0 || !doc->isUnique(newText)) {
            cancelRenameWithMsg();
            return;
        }
        UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(m_object, newText));
        doc->setModified(true);
        m_label = newText;
        break;

    case lvt_Operation: {
        if (m_object == 0) {
            cancelRenameWithMsg();
            return;
        }
        UMLOperation *op = m_object->asUMLOperation();
        if (!op) {
            cancelRenameWithMsg();
            return;
        }
        UMLClassifier *parent = op->umlParent()->asUMLClassifier();
        Model_Utils::OpDescriptor od;
        Model_Utils::Parse_Status st = Model_Utils::parseOperation(newText, od, parent);
        if (st == Model_Utils::PS_OK) {
            // TODO: Check that no operation with the exact same profile exists.
            UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(op, od.m_name));
            op->setType(od.m_pReturnType);
            UMLAttributeList parmList = op->getParmList();
            const int newParmListCount = parmList.count();
            if (newParmListCount > od.m_args.count()) {
                // Remove parameters at end of of list that no longer exist.
                for (int i = od.m_args.count(); i < newParmListCount; i++) {
                    UMLAttribute *a = parmList.at(i);
                    op->removeParm(a, false);
                }
            }
            Model_Utils::NameAndType_ListIt lit = od.m_args.begin();
            for (int i = 0; lit != od.m_args.end(); ++lit, ++i) {
                const Model_Utils::NameAndType& nm_tp = *lit;
                UMLAttribute *a;
                if (i < newParmListCount) {
                    a = parmList.at(i);
                } else {
                    a = new UMLAttribute(op);
                    a->setID(UniqueID::gen());
                }
                UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(a, nm_tp.m_name));
                a->setType(nm_tp.m_type);
                a->setParmKind(nm_tp.m_direction);
                a->setInitialValue(nm_tp.m_initialValue);
                if (i >= newParmListCount) {
                    op->addParm(a);
                }
            }
            m_label = op->toString(Uml::SignatureType::SigNoVis);
        } else {
            KMessageBox::error(0,
                               Model_Utils::psText(st),
                               i18n("Rename canceled"));
        }
        setText(m_label);
        break;
    }

    case lvt_Attribute:
    case lvt_EntityAttribute:
    case lvt_InstanceAttribute: {
        if (m_object == 0) {
            cancelRenameWithMsg();
            return;
        }
        UMLClassifier *parent = m_object->umlParent()->asUMLClassifier();
        Model_Utils::NameAndType nt;
        Uml::Visibility::Enum vis;
        Model_Utils::Parse_Status st;
        st = Model_Utils::parseAttribute(newText, nt, parent, &vis);
        if (st == Model_Utils::PS_OK) {
            UMLObject *exists = parent ? parent->findChildObject(newText) : 0;
            if (exists) {
                cancelRenameWithMsg();
                return;
            }
            UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(m_object, nt.m_name));
            UMLAttribute *pAtt = m_object->asUMLAttribute();
            pAtt->setType(nt.m_type);
            pAtt->setVisibility(vis);
            pAtt->setParmKind(nt.m_direction);
            pAtt->setInitialValue(nt.m_initialValue);
            m_label = pAtt->toString(Uml::SignatureType::SigNoVis);
        } else {
            KMessageBox::error(0,
                               Model_Utils::psText(st),
                               i18n("Rename canceled"));
        }
        setText(m_label);
        break;
    }

    case lvt_PrimaryKeyConstraint:
    case lvt_UniqueConstraint:
    case lvt_ForeignKeyConstraint:
    case lvt_CheckConstraint: {
        if (m_object == 0) {
            cancelRenameWithMsg();
            return;
        }
        UMLEntity *parent = m_object->umlParent()->asUMLEntity();
        QString name;
        Model_Utils::Parse_Status st;
        st = Model_Utils::parseConstraint(newText, name,  parent);
        if (st == Model_Utils::PS_OK) {
            UMLObject *exists = parent->findChildObject(name);
            if (exists) {
                cancelRenameWithMsg();
                return;
            }
            UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(m_object, name));

            UMLEntityConstraint* uec = m_object->asUMLEntityConstraint();
            m_label = uec->toString(Uml::SignatureType::SigNoVis);
        } else {
            KMessageBox::error(0,
                               Model_Utils::psText(st),
                               i18n("Rename canceled"));
        }
        setText(m_label);
        break;
    }

    case lvt_Template: {
        if (m_object == 0) {
            cancelRenameWithMsg();
            return;
        }
        UMLClassifier *parent = m_object->umlParent()->asUMLClassifier();
        Model_Utils::NameAndType nt;
        Model_Utils::Parse_Status st = Model_Utils::parseTemplate(newText, nt, parent);
        if (st == Model_Utils::PS_OK) {
            UMLObject *exists = parent ? parent->findChildObject(newText) : 0;
            if (exists) {
                cancelRenameWithMsg();
                return;
            }
            UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(m_object, nt.m_name));
            UMLTemplate *tmpl = m_object->asUMLTemplate();
            tmpl->setType(nt.m_type);
            m_label = tmpl->toString(Uml::SignatureType::SigNoVis);
        } else {
            KMessageBox::error(0,
                               Model_Utils::psText(st),
                               i18n("Rename canceled"));
        }
        setText(m_label);
        break;
    }

    case lvt_UseCase_Diagram:
    case lvt_Class_Diagram:
    case lvt_Sequence_Diagram:
    case lvt_Collaboration_Diagram:
    case lvt_State_Diagram:
    case lvt_Activity_Diagram:
    case lvt_Component_Diagram:
    case lvt_Deployment_Diagram:
    case lvt_Object_Diagram:{
        UMLView *view = doc->findView(ID());
        if (view == 0) {
            cancelRenameWithMsg();
            return;
        }
        UMLView *anotherView = doc->findView(view->umlScene()->type(), newText);
        if (anotherView && anotherView->umlScene()->ID() == ID()) {
            anotherView = 0;
        }
        if (anotherView) {
            cancelRenameWithMsg();
            return;
        }
        view->umlScene()->setName(newText);
        setText(newText);
        doc->signalDiagramRenamed(view);
        break;
    }
    default:
        KMessageBox::error(0,
                           i18n("Renaming an item of listview type %1 is not yet implemented.", m_type),
                           i18n("Function Not Implemented"));
        setText(m_label);
        break;
    }
    doc->setModified(true);
}

/**
 * Auxiliary method for okRename().
 */
void UMLListViewItem::cancelRenameWithMsg()
{
    DEBUG(DBG_LVI) << this << " - column=" << ":TODO:col" << ", text=" << text(0);
    KMessageBox::error(0,
                       i18n("The name you entered was invalid.\nRenaming process has been canceled."),
                       i18n("Name Not Valid"));
    setText(m_label);
}

/**
 * Overrides the default sorting to sort by item type.
 * Sort the listview items by type and position within the corresponding list
 * of UMLObjects. If the item does not have a UMLObject then place it last.
 */
#if 0
int UMLListViewItem::compare(QTreeWidgetItem *other, int col, bool ascending) const
{
    UMLListViewItem *ulvi = other->asUMLListViewItem();
    ListViewType ourType = type();
    ListViewType otherType = ulvi->type();

    if (ourType < otherType)
        return -1;
    if (ourType > otherType)
        return 1;
    // ourType == otherType
    const bool subItem = Model_Utils::typeIsClassifierList(ourType);
    const int alphaOrder = key(col, ascending).compare(other->key(col, ascending));
    int retval = 0;
    QString dbgPfx = "compare(type=" + QString::number((int)ourType)
                     + ", self=" + text() + ", other=" + ulvi->text()
                     + "): return ";
    UMLObject *otherObj = ulvi->umlObject();
    if (m_object == 0) {
        retval = (subItem ? 1 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        DEBUG(DBG_LVI) << dbgPfx << retval << " because (m_object==0)";
#endif
        return retval;
    }
    if (otherObj == 0) {
        retval = (subItem ? -1 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        DEBUG(DBG_LVI) << dbgPfx << retval << " because (otherObj==0)";
#endif
        return retval;
    }
    UMLClassifier *ourParent = m_object->umlParent()->asUMLClassifier();
    UMLClassifier *otherParent = otherObj->umlParent()->asUMLClassifier();
    if (ourParent == 0) {
        retval = (subItem ? 1 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        DEBUG(DBG_LVI) << dbgPfx << retval << " because (ourParent==0)";
#endif
        return retval;
    }
    if (otherParent == 0) {
        retval = (subItem ? -1 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        DEBUG(DBG_LVI) << dbgPfx << retval << " because (otherParent==0)";
#endif
        return retval;
    }
    if (ourParent != otherParent) {
        retval = (subItem ? 0 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        DEBUG(DBG_LVI) << dbgPfx << retval << " because (ourParent != otherParent)";
#endif
        return retval;
    }
    UMLClassifierListItem *thisUmlItem = m_object->asUMLClassifierListItem();
    UMLClassifierListItem *otherUmlItem = otherObj->asUMLClassifierListItem();
    if (thisUmlItem == 0) {
        retval = (subItem ? 1 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        DEBUG(DBG_LVI) << dbgPfx << retval << " because (thisUmlItem==0)";
#endif
        return retval;
    }
    if (otherUmlItem == 0) {
        retval = (subItem ? -1 : alphaOrder);
#ifdef DEBUG_LVITEM_INSERTION_ORDER
        DEBUG(DBG_LVI) << dbgPfx << retval << " because (otherUmlItem==0)";
#endif
        return retval;
    }
    UMLClassifierListItemList items = ourParent->getFilteredList(thisUmlItem->baseType());
    int myIndex = items.indexOf(thisUmlItem);
    int otherIndex = items.indexOf(otherUmlItem);
    if (myIndex < 0) {
        retval = (subItem ? -1 : alphaOrder);
        uError() << dbgPfx << retval << " because (myIndex < 0)";
        return retval;
    }
    if (otherIndex < 0) {
        retval = (subItem ? 1 : alphaOrder);
        uError() << dbgPfx << retval << " because (otherIndex < 0)";
        return retval;
    }
    return (myIndex < otherIndex ? -1 : myIndex > otherIndex ? 1 : 0);
}
#endif

/**
 * Create a deep copy of this UMLListViewItem, but using the
 * given parent instead of the parent of this UMLListViewItem.
 * Return the new UMLListViewItem created.
 */
UMLListViewItem* UMLListViewItem::deepCopy(UMLListViewItem *newParent)
{
    QString nm = text(0);
    ListViewType t = type();
    UMLObject *o = umlObject();
    UMLListViewItem* newItem;
    if (o)
        newItem = new UMLListViewItem(newParent, nm, t, o);
    else
        newItem = new UMLListViewItem(newParent, nm, t, m_id);
    for (int i=0; i < childCount(); i++) {
        UMLListViewItem *childItem = static_cast<UMLListViewItem*>(child(i));
        childItem->deepCopy(newItem);
    }
    return newItem;
}

/**
 * Find the UMLListViewItem that is related to the given UMLObject
 * in the tree rooted at the current UMLListViewItem.
 * Return a pointer to the item or NULL if not found.
 */
UMLListViewItem* UMLListViewItem::findUMLObject(const UMLObject *o)
{
    if (m_object == o)
        return this;
    for (int i = 0; i < childCount(); i++) {
        UMLListViewItem *item = static_cast<UMLListViewItem*>(child(i));
        UMLListViewItem *testItem = item->findUMLObject(o);
        if (testItem)
            return testItem;
    }
    return 0;
}

/**
 * Find the UMLListViewItem that represents the given UMLObject in the
 * children of the current UMLListViewItem.
 * Return a pointer to the item or NULL if not found.
 */
UMLListViewItem* UMLListViewItem::findChildObject(UMLObject *child)
{
    ChildObjectMap::iterator it = m_comap.find(child);
    if (it != m_comap.end()) {
        return *it;
    }
    return 0;
}

/**
 * Find the UMLListViewItem of the given ID in the tree rooted at
 * the current UMLListViewItem.
 * Return a pointer to the item or NULL if not found.
 *
 * @param id   The ID to search for.
 * @return The item with the given ID or NULL if not found.
 */
UMLListViewItem * UMLListViewItem::findItem(Uml::ID::Type id)
{
    if (ID() == id) {
        return this;
    }
    for (int i = 0; i < childCount(); ++i) {
        UMLListViewItem *childItem = static_cast<UMLListViewItem*>(child(i));
        UMLListViewItem *inner = childItem->findItem(id);
        if (inner) {
            return inner;
        }
    }
    return 0;
}

/**
 * Saves the listview item to a "listitem" tag.
 */
void UMLListViewItem::saveToXMI1(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("listitem"));
    Uml::ID::Type id = ID();
    QString idStr = Uml::ID::toString(id);
    //DEBUG(DBG_LVI) << "id = " << idStr << ", type = " << m_type;
    if (id != Uml::ID::None)
        writer.writeAttribute(QLatin1String("id"), idStr);
    writer.writeAttribute(QLatin1String("type"), QString::number(m_type));
    if (m_object == 0) {
        if (! Model_Utils::typeIsDiagram(m_type) && m_type != lvt_View)
            uError() << text(0) << ": m_object is NULL";
        if (m_type != lvt_View)
            writer.writeAttribute(QLatin1String("label"), text(0));
    } else if (m_object->id() == Uml::ID::None) {
        if (text(0).isEmpty()) {
            DEBUG(DBG_LVI) << "Skipping empty item";
            return;
        }
        DEBUG(DBG_LVI) << "saving local label " << text(0) << " because umlobject ID is not set";
        if (m_type != lvt_View)
            writer.writeAttribute(QLatin1String("label"), text(0));
    } else if (m_object->baseType() == UMLObject::ot_Folder) {
        const UMLFolder *extFolder = m_object->asUMLFolder();
        if (!extFolder->folderFile().isEmpty()) {
            writer.writeAttribute(QLatin1String("open"), QLatin1String("0"));
            writer.writeEndElement();
            return;
        }
    }
    writer.writeAttribute(QLatin1String("open"), QString::number(isExpanded()));
    for (int i = 0; i < childCount(); i++) {
        UMLListViewItem *childItem = static_cast<UMLListViewItem*>(child(i));
        childItem->saveToXMI1(writer);
    }
    writer.writeEndElement();
}

/**
 * Loads a "listitem" tag, this is only used by the clipboard currently.
 */
bool UMLListViewItem::loadFromXMI1(QDomElement& qElement)
{
    QString id = qElement.attribute(QLatin1String("id"), QLatin1String("-1"));
    QString type = qElement.attribute(QLatin1String("type"), QLatin1String("-1"));
    QString label = qElement.attribute(QLatin1String("label"));
    QString open = qElement.attribute(QLatin1String("open"), QLatin1String("1"));
    if (!label.isEmpty())
        setText(label);
    else if (id == QLatin1String("-1")) {
        uError() << "Item of type " << type << " has neither ID nor label";
        return false;
    }

    m_id = Uml::ID::fromString(id);
    if (m_id != Uml::ID::None) {
        UMLListView* listView = static_cast<UMLListView*>(treeWidget());
        m_object = listView->document()->findObjectById(m_id);
    }
    m_type = (ListViewType)(type.toInt());
    if (m_object)
        updateObject();
    setOpen((bool)open.toInt());
    return true;
}

UMLListViewItem* UMLListViewItem::childItem(int i)
{
    return static_cast<UMLListViewItem*>(child(i));
}

QString UMLListViewItem::toString(ListViewType type)
{
    switch (type) {
        case lvt_View:
            return QLatin1String("lvt_View");
        case lvt_Logical_View:
            return QLatin1String("lvt_Logical_View");
        case lvt_UseCase_View:
            return QLatin1String("lvt_UseCase_View");
        case lvt_Logical_Folder:
            return QLatin1String("lvt_Logical_Folder");
        case lvt_UseCase_Folder:
            return QLatin1String("lvt_UseCase_Folder");
        case lvt_UseCase_Diagram:
            return QLatin1String("lvt_UseCase_Diagram");
        case lvt_Collaboration_Diagram:
            return QLatin1String("lvt_Collaboration_Diagram");
        case lvt_Class_Diagram:
            return QLatin1String("lvt_Class_Diagram");
        case lvt_State_Diagram:
            return QLatin1String("lvt_State_Diagram");
        case lvt_Activity_Diagram:
            return QLatin1String("lvt_Activity_Diagram");
        case lvt_Sequence_Diagram:
            return QLatin1String("lvt_Sequence_Diagram");
        case lvt_Object_Diagram:
            return QLatin1String("lvt_Object_Diagram");
        case lvt_Actor:
            return QLatin1String("lvt_Actor");
        case lvt_Association:
            return QLatin1String("lvt_Association");
        case lvt_UseCase:
            return QLatin1String("lvt_UseCase");
        case lvt_Class:
            return QLatin1String("lvt_Class");
        case lvt_Attribute:
            return QLatin1String("lvt_Attribute");
        case lvt_Operation:
            return QLatin1String("lvt_Operation");
        case lvt_Template:
            return QLatin1String("lvt_Template");
        case lvt_Interface:
            return QLatin1String("lvt_Interface");
        case lvt_Package:
            return QLatin1String("lvt_Package");
        case lvt_Component_Diagram:
            return QLatin1String("lvt_Component_Diagram");
        case lvt_Component_Folder:
            return QLatin1String("lvt_Component_Folder");
        case lvt_Component_View:
            return QLatin1String("lvt_Component_View");
        case lvt_Component:
            return QLatin1String("lvt_Component");
        case lvt_Diagrams:
            return QLatin1String("lvt_Diagrams");
        case lvt_Artifact:
            return QLatin1String("lvt_Artifact");
        case lvt_Deployment_Diagram:
            return QLatin1String("lvt_Deployment_Diagram");
        case lvt_Deployment_Folder:
            return QLatin1String("lvt_Deployment_Folder");
        case lvt_Deployment_View:
            return QLatin1String("lvt_Deployment_View");
        case lvt_Port:
            return QLatin1String("lvt_Port");
        case lvt_Node:
            return QLatin1String("lvt_Node");
        case lvt_Datatype:
            return QLatin1String("lvt_Datatype");
        case lvt_Datatype_Folder:
            return QLatin1String("lvt_Datatype_Folder");
        case lvt_Enum:
            return QLatin1String("lvt_Enum");
        case lvt_Entity:
            return QLatin1String("lvt_Entity");
        case lvt_EntityAttribute:
            return QLatin1String("lvt_EntityAttribute");
        case lvt_EntityRelationship_Diagram:
            return QLatin1String("lvt_EntityRelationship_Diagram");
        case lvt_EntityRelationship_Folder:
            return QLatin1String("lvt_EntityRelationship_Folder");
        case lvt_EntityRelationship_Model:
            return QLatin1String("lvt_EntityRelationship_Model");
        case lvt_Subsystem:
            return QLatin1String("lvt_Subsystem");
        case lvt_Model:
            return QLatin1String("lvt_Model");
        case lvt_EnumLiteral:
            return QLatin1String("lvt_EnumLiteral");
        case lvt_UniqueConstraint:
            return QLatin1String("lvt_UniqueConstraint");
        case lvt_PrimaryKeyConstraint:
            return QLatin1String("lvt_PrimaryKeyConstraint");
        case lvt_ForeignKeyConstraint:
            return QLatin1String("lvt_ForeignKeyConstraint");
        case lvt_CheckConstraint:
            return QLatin1String("lvt_CheckConstraint");
        case lvt_Category:
            return QLatin1String("lvt_Category");
        case lvt_Properties:
            return QLatin1String("lvt_Properties");
        case lvt_Unknown:
            return QLatin1String("lvt_Unknown");
        case lvt_Instance:
            return QLatin1String("lvt_Instance");
        case lvt_InstanceAttribute:
            return QLatin1String("lvt_InstanceAttribute");
        default:
            return QLatin1String("? ListViewType ?");
    }
}

/**
 * Overloading operator for debugging output.
 */
QDebug operator<<(QDebug dbg, const UMLListViewItem& item)
{
    dbg.nospace() << "UMLListViewItem: " << item.text(0)
        << ", type=" << UMLListViewItem::toString(item.type())
        << ", id=" << Uml::ID::toString(item.ID())
        << ", children=" << item.childCount();
    return dbg.space();
}
