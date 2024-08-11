/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H

#include "basictypes.h"
#include "umllistviewitem.h"
#include "umlobjectlist.h"
#include "umlviewlist.h"
#include "foreignkeyconstraint.h"
#include "icon_utils.h"
#include "multipagedialogbase.h"
#include "widgetbase.h"

#include <QList>
#include <QString>

// forward declarations
class UMLClassifier;
class UMLPackage;
class UMLEntity;

/**
 * General purpose model utilities.
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
namespace Model_Utils {

bool isCloneable(WidgetBase::WidgetType type);

QString normalize(QString type);

UMLObject* findObjectInList(Uml::ID::Type id, const UMLObjectList& inList);

UMLObject* findUMLObject(const UMLObjectList& inList,
                          const QString& name,
                          UMLObject::ObjectType type = UMLObject::ot_UMLObject,
                          UMLObject  *currentObj = nullptr);

UMLObject* findUMLObjectRaw(const UMLObjectList& inList,
                             const QString& name,
                             UMLObject::ObjectType type = UMLObject::ot_UMLObject,
                             UMLObject  *currentObj = nullptr);

UMLObject* findUMLObjectRecursive(const UMLObjectList& inList,
                                  const QString& name,
                                  UMLObject::ObjectType type = UMLObject::ot_UMLObject);

UMLPackage* rootPackage(UMLObject* obj);

void treeViewAddViews(const UMLViewList& viewList);
void treeViewChangeIcon(UMLObject* object, Icon_Utils::IconType to);
void treeViewSetCurrentItem(UMLObject* object);
void treeViewMoveObjectTo(UMLObject* container, UMLObject* object);
UMLObject*  treeViewGetCurrentObject();
UMLPackage* treeViewGetPackageFromCurrent();
QString treeViewBuildDiagramName(Uml::ID::Type id);

QString uniqObjectName(UMLObject::ObjectType type,
                       UMLPackage *parentPkg,
                       QString prefix = QString());
QString newTitle(UMLObject::ObjectType type);
QString newText(UMLObject::ObjectType type);
QString renameTitle(UMLObject::ObjectType type);
QString renameText(UMLObject::ObjectType type);

QString getXmiId(QDomElement element);
QString loadCommentFromXMI(QDomElement elem);
bool isCommonXMI1Attribute(const QString &tag);
bool isCommonDataType(QString type);
bool isClassifierListitem(UMLObject::ObjectType ot);

bool typeIsCanvasWidget(UMLListViewItem::ListViewType type);
bool typeIsRootView(UMLListViewItem::ListViewType type);
bool typeIsFolder(UMLListViewItem::ListViewType type);
bool typeIsContainer(UMLListViewItem::ListViewType type);
bool typeIsDiagram(UMLListViewItem::ListViewType type);
bool typeIsClassifierList(UMLListViewItem::ListViewType type);
bool typeIsClassifier(UMLListViewItem::ListViewType type);
bool typeIsProperties(UMLListViewItem::ListViewType type);
bool typeIsAllowedInType(UMLListViewItem::ListViewType childType,
    UMLListViewItem::ListViewType parentType);
bool typeIsAllowedInDiagram(UMLObject *o, UMLScene *scene);
bool typeIsAllowedInDiagram(UMLWidget *w, UMLScene *scene);
bool hasAssociations(UMLObject::ObjectType type);

Uml::ModelType::Enum convert_DT_MT(Uml::DiagramType::Enum dt);
UMLListViewItem::ListViewType convert_MT_LVT(Uml::ModelType::Enum mt);
Uml::ModelType::Enum convert_LVT_MT(UMLListViewItem::ListViewType lvt);
UMLListViewItem::ListViewType convert_DT_LVT(Uml::DiagramType::Enum dt);
UMLObject::ObjectType convert_LVT_OT(UMLListViewItem::ListViewType lvt);
UMLListViewItem::ListViewType convert_OT_LVT(UMLObject *o);
Icon_Utils::IconType convert_LVT_IT(UMLListViewItem::ListViewType lvt, UMLObject  *o= nullptr);
Uml::DiagramType::Enum convert_LVT_DT(UMLListViewItem::ListViewType lvt);
MultiPageDialogBase::PageType convert_LVT_PT(UMLListViewItem::ListViewType type);
Uml::ModelType::Enum convert_OT_MT(UMLObject::ObjectType ot);

Uml::ModelType::Enum guessContainer(UMLObject *o);  // deprecated !

int stringToDirection(QString input, Uml::ParameterDirection::Enum & result);

enum Parse_Status {  ///< Return type of parseOperation().
    PS_OK, PS_Empty, PS_Malformed_Arg, PS_Unknown_ArgType,
    PS_Illegal_MethodName, PS_Unknown_ReturnType, PS_Unspecified_Error
};

struct NameAndType {  ///< Data structure filled by parseAttribute().
    QString m_name;
    UMLObject *m_type;
    Uml::ParameterDirection::Enum m_direction;
    QString m_initialValue;
    NameAndType() : m_type(nullptr), m_direction(Uml::ParameterDirection::In) {}
    NameAndType(QString name, UMLObject *type,
                Uml::ParameterDirection::Enum direction = Uml::ParameterDirection::In,
                QString initialValue = QString())
            : m_name(name), m_type(type),
              m_direction(direction), m_initialValue(initialValue) {}
};

typedef QList<NameAndType> NameAndType_List;               ///< Auxiliary type for OpDescriptor.
typedef QList<NameAndType>::iterator NameAndType_ListIt;   ///< Auxiliary type for OpDescriptor.

struct OpDescriptor {  ///< Data structure filled by parseOperation().
    QString m_name;
    NameAndType_List m_args;
    UMLObject *m_pReturnType;
};

Parse_Status parseTemplate(QString t, NameAndType& nmTp, UMLClassifier *owningScope);
Parse_Status parseAttribute(QString a, NameAndType& nmTp, UMLClassifier *owningScope,
                            Uml::Visibility::Enum  *vis = nullptr);
Parse_Status parseOperation(QString m, OpDescriptor& desc, UMLClassifier *owningScope);
Parse_Status parseConstraint(QString m, QString& name, UMLEntity* owningScope);

QString psText(Parse_Status value);

QString updateDeleteActionToString(UMLForeignKeyConstraint::UpdateDeleteAction uda);

}

#endif
