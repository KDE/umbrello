/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLLISTVIEWITEM_H
#define UMLLISTVIEWITEM_H

#include <QtGui/QTreeWidget>
#include <QtCore/QMap>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include "basictypes.h"
#include "icon_utils.h"

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
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLListViewItem : public QTreeWidgetItem
{
public:
    enum ListViewType
    {
        //the values in this enum are saved out to the file
        //for file compatibility, only add new values to the end
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
        lvt_Unknown = -1
    };

    static QString toString(ListViewType type);

    UMLListViewItem(UMLListView * parent, const QString &name, ListViewType t, UMLObject* o = 0);
    UMLListViewItem(UMLListView * parent);
    UMLListViewItem(UMLListViewItem * parent);
    UMLListViewItem(UMLListViewItem * parent, const QString &name, ListViewType t, UMLObject* o = 0);
    UMLListViewItem(UMLListViewItem * parent, const QString &name, ListViewType t, Uml::IDType id);

    ~UMLListViewItem();

    ListViewType type() const;

    void setID(Uml::IDType id);
    Uml::IDType getID() const;

    void setUMLObject(UMLObject * obj);
    UMLObject * umlObject() const;

    bool isOwnParent(Uml::IDType listViewItemID);

    void updateObject();
    void updateFolder();

    void setText(int column, const QString &text);
    void setText(const QString &text );
    QString getText() const;
    void setVisible(bool state);

    QString toolTip();

    void setCreating(bool creating);

    void setIcon(Icon_Utils::IconType iconType);

    void startRename(int col);
    void cancelRename(int col);
    void okRename(int col);

    void addClassifierListItem(UMLClassifierListItem *child, UMLListViewItem *childItem);

    void deleteChildItem(UMLClassifierListItem *child);

    //virtual int compare(UMLListViewItem *other, int col, bool ascending) const;

    UMLListViewItem* deepCopy(UMLListViewItem *newParent);

    UMLListViewItem* findUMLObject(const UMLObject *o);
    UMLListViewItem* findChildObject(UMLClassifierListItem *cli);
    UMLListViewItem* findItem(Uml::IDType id);

    UMLListViewItem* childItem(int i);

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    bool loadFromXMI(QDomElement& qElement);

    bool isOpen() { return isExpanded(); }
    void setOpen(bool state);

    friend QDebug operator<<(QDebug out, const UMLListViewItem& item);

protected:
    void init();

    void cancelRenameWithMsg();

    /**
     * Auxiliary map of child UMLLisViewItems keyed by UMLClassifierListItem.
     * Used by findChildObject() for efficiency instead of looping using
     * firstChild()/nextSibling() because the latter incur enforceItemVisible()
     * and thus expensive sorting.
     */
    typedef QMap<UMLClassifierListItem*, UMLListViewItem*> ChildObjectMap;

    bool               m_bCreating;  ///< flag to set the state of creating
    ListViewType       m_type;
    Uml::IDType        m_id;
    UMLObject *        m_object;
    QString            m_label;
    ChildObjectMap     m_comap;
};

#endif
