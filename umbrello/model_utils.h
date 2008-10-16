/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2004-2008                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H

#include <QtCore/QString>
#include <QtCore/QLinkedList>

#include "umlnamespace.h"
#include "umlobjectlist.h"
#include "foreignkeyconstraint.h"
#include "icon_utils.h"


/**
 * General purpose model utilities.
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
// forward declarations
class UMLClassifier;
class UMLPackage;
class UMLEntity;
class UMLForeignKeyConstraint;

namespace Model_Utils {

bool isCloneable(Uml::Widget_Type type);

UMLObject * findObjectInList(Uml::IDType id, const UMLObjectList& inList);

UMLObject* findUMLObject( const UMLObjectList& inList,
                          const QString& name,
                          Uml::Object_Type type = Uml::ot_UMLObject,
                          UMLObject *currentObj = NULL);

QString uniqObjectName(Uml::Object_Type type,
                       UMLPackage *parentPkg,
                       QString prefix = QString());

bool isCommonXMIAttribute(const QString &tag);

bool isCommonDataType(QString type);

bool isClassifierListitem(Uml::Object_Type ot);

bool typeIsCanvasWidget(Uml::ListView_Type type);

bool typeIsRootView(Uml::ListView_Type type);

bool typeIsFolder(Uml::ListView_Type type);

bool typeIsContainer(Uml::ListView_Type type);

bool typeIsDiagram(Uml::ListView_Type type);

bool typeIsClassifierList(Uml::ListView_Type type);

bool typeIsClassifier(Uml::ListView_Type type);

Uml::Model_Type convert_DT_MT(Uml::Diagram_Type dt);

Uml::ListView_Type convert_MT_LVT(Uml::Model_Type mt);

Uml::Model_Type convert_LVT_MT(Uml::ListView_Type lvt);

Uml::ListView_Type convert_DT_LVT(Uml::Diagram_Type dt);

Uml::Object_Type convert_LVT_OT(Uml::ListView_Type lvt);

Uml::ListView_Type convert_OT_LVT(UMLObject *o);

Icon_Utils::Icon_Type convert_LVT_IT(Uml::ListView_Type lvt);

Uml::Diagram_Type convert_LVT_DT(Uml::ListView_Type lvt);

Uml::Model_Type convert_OT_MT(Uml::Object_Type ot);

Uml::Model_Type guessContainer(UMLObject *o);

int stringToDirection(QString input, Uml::Parameter_Direction & result);

QString progLangToString(Uml::Programming_Language pl);

Uml::Programming_Language stringToProgLang(QString str);

QString diagramTypeToString(Uml::Diagram_Type dt);

/**
 * Return type of parseOperation()
 */
enum Parse_Status {
    PS_OK, PS_Empty, PS_Malformed_Arg, PS_Unknown_ArgType,
    PS_Illegal_MethodName, PS_Unknown_ReturnType, PS_Unspecified_Error
};

/**
 * Data structure filled by parseAttribute()
 */
struct NameAndType {
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

/**
 * Auxiliary type for OpDescriptor
 */
typedef QLinkedList<NameAndType> NameAndType_List;
typedef QLinkedList<NameAndType>::iterator NameAndType_ListIt;

/**
 * Data structure filled by parseOperation()
 */
struct OpDescriptor {
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

/**
 * In a Q_OBJECT class define any enum as Q_ENUMS.
 * With the above the following macro returns the name of a given enum.
 * This can be used in debug output.
 * TODO: convert it to a function.
 */
#define ENUM_NAME(o,e,v) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))

}

#endif
