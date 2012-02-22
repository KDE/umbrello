/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H

#include "basictypes.h"
#include "umllistviewitem.h"
#include "umlobjectlist.h"
#include "umlviewlist.h"
#include "foreignkeyconstraint.h"
#include "icon_utils.h"
#include "widgetbase.h"

#include <QtCore/QString>
#include <QtCore/QLinkedList>

// forward declarations
class UMLClassifier;
class UMLPackage;
class UMLEntity;

/**
 * General purpose model utilities.
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Model_Utils {

bool isCloneable(WidgetBase::WidgetType type);

UMLObject* findObjectInList(Uml::IDType id, const UMLObjectList& inList);

UMLObject* findUMLObject( const UMLObjectList& inList,
                          const QString& name,
                          UMLObject::ObjectType type = UMLObject::ot_UMLObject,
                          UMLObject *currentObj = 0);

UMLObject* findUMLObjectRaw( const UMLObjectList& inList,
                             const QString& name,
                             UMLObject::ObjectType type = UMLObject::ot_UMLObject,
                             UMLObject *currentObj = 0);

void treeViewAddViews(const UMLViewList& viewList);
void treeViewChangeIcon(UMLObject* object, Icon_Utils::IconType to);
void treeViewSetCurrentItem(UMLObject* object);
void treeViewMoveObjectTo(UMLObject* container, UMLObject* object);
UMLObject*  treeViewGetCurrentObject();
UMLPackage* treeViewGetPackageFromCurrent();
QString treeViewBuildDiagramName(Uml::IDType id);

QString uniqObjectName(UMLObject::ObjectType type,
                       UMLPackage *parentPkg,
                       QString prefix = QString());

bool isCommonXMIAttribute(const QString &tag);
bool isCommonDataType(QString type);
bool isClassifierListitem(UMLObject::ObjectType ot);

bool typeIsCanvasWidget(UMLListViewItem::ListViewType type);
bool typeIsRootView(UMLListViewItem::ListViewType type);
bool typeIsFolder(UMLListViewItem::ListViewType type);
bool typeIsContainer(UMLListViewItem::ListViewType type);
bool typeIsDiagram(UMLListViewItem::ListViewType type);
bool typeIsClassifierList(UMLListViewItem::ListViewType type);
bool typeIsClassifier(UMLListViewItem::ListViewType type);

Uml::ModelType convert_DT_MT(Uml::DiagramType dt);
UMLListViewItem::ListViewType convert_MT_LVT(Uml::ModelType mt);
Uml::ModelType convert_LVT_MT(UMLListViewItem::ListViewType lvt);
UMLListViewItem::ListViewType convert_DT_LVT(Uml::DiagramType dt);
UMLObject::ObjectType convert_LVT_OT(UMLListViewItem::ListViewType lvt);
UMLListViewItem::ListViewType convert_OT_LVT(UMLObject *o);
Icon_Utils::IconType convert_LVT_IT(UMLListViewItem::ListViewType lvt);
Uml::DiagramType convert_LVT_DT(UMLListViewItem::ListViewType lvt);
Uml::ModelType convert_OT_MT(UMLObject::ObjectType ot);

Uml::ModelType guessContainer(UMLObject *o);  // deprecated !

int stringToDirection(QString input, Uml::Parameter_Direction & result);

enum Parse_Status {  ///< Return type of parseOperation().
    PS_OK, PS_Empty, PS_Malformed_Arg, PS_Unknown_ArgType,
    PS_Illegal_MethodName, PS_Unknown_ReturnType, PS_Unspecified_Error
};

struct NameAndType {  ///< Data structure filled by parseAttribute().
    QString m_name;
    UMLObject *m_type;
    Uml::Parameter_Direction m_direction;
    QString m_initialValue;
    NameAndType() : m_type(0), m_direction(Uml::pd_In) {
    }
    NameAndType(QString name, UMLObject *type,
                Uml::Parameter_Direction direction = Uml::pd_In,
                QString initialValue = QString())
            : m_name(name), m_type(type),
              m_direction(direction), m_initialValue(initialValue) {
    }
};

typedef QLinkedList<NameAndType> NameAndType_List;               ///< Auxiliary type for OpDescriptor.
typedef QLinkedList<NameAndType>::iterator NameAndType_ListIt;   ///< Auxiliary type for OpDescriptor.

struct OpDescriptor {  ///< Data structure filled by parseOperation().
    QString m_name;
    NameAndType_List m_args;
    UMLObject *m_pReturnType;
};

Parse_Status parseTemplate(QString t, NameAndType& nmTp, UMLClassifier *owningScope);
Parse_Status parseAttribute(QString a, NameAndType& nmTp, UMLClassifier *owningScope,
                            Uml::Visibility *vis = 0);
Parse_Status parseOperation(QString m, OpDescriptor& desc, UMLClassifier *owningScope);
Parse_Status parseConstraint(QString m, QString& name, UMLEntity* owningScope);

QString psText(Parse_Status value);

QString updateDeleteActionToString( UMLForeignKeyConstraint::UpdateDeleteAction uda );

}

#endif
