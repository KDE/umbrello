/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLLISTVIEWITEM_H
#define UMLLISTVIEWITEM_H

#include "basictypes.h"
#include "icon_utils.h"

#include <QDomDocument>
#include <QDomElement>
#include <QHash>
#include <QPointer>
#include <QTreeWidget>
#include <QXmlStreamWriter>

// forward declarations
class UMLListView;
class UMLObject;
class UMLClassifierListItem;

typedef QTreeWidgetItemIterator UMLListViewItemIterator;

/**
 * Items used by the class @ref UMLListView.  This is needed as the type
 * and object information is required to be stored.
 *
 * @short  Items used by @ref UMLListView.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see    UMLListView
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLListViewItem : public QTreeWidgetItem
{
public:
    enum ListViewType
    {
        //the values in this enum are saved out to the file
        //for file compatibility, only add new values to the end
        lvt_Min = 799,
        lvt_View  =  800,
        lvt_Logical_View,
        lvt_UseCase_View,
        lvt_Logical_Folder,
        lvt_UseCase_Folder,
        lvt_UseCase_Diagram,
        lvt_Collaboration_Diagram,
        lvt_Class_Diagram,
        lvt_State_Diagram,
        lvt_Activity_Diagram,
        lvt_Sequence_Diagram,
        lvt_Actor,
        lvt_UseCase,
        lvt_Class,
        lvt_Attribute,
        lvt_Operation,
        lvt_Template,
        lvt_Interface,
        lvt_Package,
        lvt_Component_Diagram,
        lvt_Component_Folder,
        lvt_Component_View,
        lvt_Component,
        lvt_Diagrams,  // currently unused
        lvt_Artifact,
        lvt_Deployment_Diagram,
        lvt_Deployment_Folder,
        lvt_Deployment_View,
        lvt_Node,
        lvt_Datatype,
        lvt_Datatype_Folder,
        lvt_Enum,
        lvt_Entity,
        lvt_EntityAttribute,
        lvt_EntityRelationship_Diagram,
        lvt_EntityRelationship_Folder,
        lvt_EntityRelationship_Model,
        lvt_Subsystem,
        lvt_Model,
        lvt_EnumLiteral,
        lvt_UniqueConstraint,
        lvt_PrimaryKeyConstraint,
        lvt_ForeignKeyConstraint,
        lvt_CheckConstraint,
        lvt_Category,
        lvt_Port,
        lvt_Properties,
        lvt_Properties_AutoLayout,
        lvt_Properties_Class,
        lvt_Properties_CodeGeneration,
        lvt_Properties_CodeImport,
        lvt_Properties_CodeViewer,
        lvt_Properties_Font,
        lvt_Properties_General,
        lvt_Properties_UserInterface,
        lvt_Association,
        lvt_Object_Diagram,
        lvt_Instance,
        lvt_InstanceAttribute,
        // enter new values above
        lvt_Max,
        lvt_Unknown = -1
    };

    static QString toString(ListViewType type);

    UMLListViewItem(UMLListView  *parent, const QString &name, ListViewType t, UMLObject *o = nullptr);
    explicit UMLListViewItem(UMLListView * parent);
    explicit UMLListViewItem(UMLListViewItem * parent);
    UMLListViewItem(UMLListViewItem  *parent, const QString &name, ListViewType t, UMLObject *o = nullptr);
    UMLListViewItem(UMLListViewItem * parent, const QString &name, ListViewType t, Uml::ID::Type id);

    ListViewType type() const;

    void setID(Uml::ID::Type id);
    Uml::ID::Type ID() const;

    void setUMLObject(UMLObject * obj);
    UMLObject * umlObject() const;

    bool isOwnParent(Uml::ID::Type listViewItemID);

    void updateObject();
    void updateFolder();

    void setText(int column, const QString &text);
    void setText(const QString &text);
    QString getSavedText() const;
    void setVisible(bool state);

    QString toolTip() const;

    void setIcon(Icon_Utils::IconType iconType);

    void addChildItem(UMLObject *child, UMLListViewItem *childItem);

    void deleteChildItem(UMLObject *child);
    static void deleteItem(UMLListViewItem *childItem);

    //virtual int compare(UMLListViewItem *other, int col, bool ascending) const;

    UMLListViewItem* deepCopy(UMLListViewItem *newParent);

    UMLListViewItem* findUMLObject(const UMLObject *o);
    UMLListViewItem* findChildObject(const UMLObject *child);
    UMLListViewItem* findItem(Uml::ID::Type id);

    UMLListViewItem* childItem(int i);

    void saveToXMI(QXmlStreamWriter& writer);
    bool loadFromXMI(QDomElement& qElement);

    bool isOpen() const { return isExpanded(); }
    void setOpen(bool state);

public slots:
    void slotEditFinished(const QString &newText);

protected:
    void init();

    void cancelRenameWithMsg();

    UMLListViewItem* findUMLObject_r(const UMLObject *o);

    /**
     * Auxiliary map of child UMLListViewItems keyed by UMLObject.
     * Used by findChildObject() for efficiency instead of looping using
     * firstChild()/nextSibling() because the latter incur enforceItemVisible()
     * and thus expensive sorting.
     */
    typedef QHash<const UMLObject*, UMLListViewItem*> ChildObjectMap;

    ListViewType       m_type;
    Uml::ID::Type      m_id;
    QPointer<UMLObject> m_object;
    QString            m_label;
    static ChildObjectMap *s_comap;

};

QDebug operator<<(QDebug dbg, const UMLListViewItem& item);

#endif
