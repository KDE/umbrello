/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2004-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H

#include <qstring.h>
#include <qvaluelist.h>

#include "umlnamespace.h"
#include "umlobjectlist.h"

/**
 * General purpose model utilities.
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */


// forward declarations
class UMLClassifier;
class UMLPackage;

namespace Model_Utils {

/**
 * Determines whether the given widget type is cloneable.
 *
 * @param type          The input Widget_Type.
 * @return      True if the given type is cloneable.
 */
bool isCloneable(Uml::Widget_Type type);

/**
 * Seek the given id in the given list of objects.
 * Each list element may itself contain other objects
 * and the search is done recursively.
 *
 * @param id            The unique ID to seek.
 * @param inList        The UMLObjectList in which to search.
 * @return      Pointer to the UMLObject that matches the ID
 *              (NULL if none matches.)
 */
UMLObject * findObjectInList(Uml::IDType id, const UMLObjectList& inList);

/**
 * Find the UML object of the given type and name in the passed-in list.
 *
 * @param inList        List in which to seek the object.
 * @param name          Name of the object to find.
 * @param type          Object_Type of the object to find (optional.)
 *                      When the given type is ot_UMLObject the type is
 *                      disregarded, i.e. the given name is the only
 *                      search criterion.
 * @param currentObj    Object relative to which to search (optional.)
 *                      If given then the enclosing scope(s) of this
 *                      object are searched before the global scope.
 * @return      Pointer to the UMLObject found, or NULL if not found.
 */
UMLObject* findUMLObject( const UMLObjectList& inList,
                          const QString& name,
                          Uml::Object_Type type = Uml::ot_UMLObject,
                          UMLObject *currentObj = NULL);

/**
 * Returns a name for the new object, appended with a number
 * if the default name is taken e.g. new_actor, new_actor_1
 * etc.
 * @param type              The object type.
 * @param parentPkg The package in which to compare the name.
 * @param prefix    The prefix to use (optional.)
 *                  If no prefix is given then a type related
 *                  prefix will be chosen internally.
 */
QString uniqObjectName(Uml::Object_Type type,
                       UMLPackage *parentPkg,
                       QString prefix = QString::null);

/**
 * Return true if the given tag is a one of the common XMI
 * attributes, such as:
 * "name" | "visibility" | "isRoot" | "isLeaf" | "isAbstract" |
 * "isActive" | "ownerScope"
 */
bool isCommonXMIAttribute(const QString &tag);

/**
 * Return true if the given type is common among the majority
 * of programming languages, such as "bool" or "boolean".
 * TODO: Make this depend on the active programming language.
 */
bool isCommonDataType(QString type);

/**
 * Return true if the given object type is a classifier list item type.
 */
bool isClassifierListitem(Uml::Object_Type ot);

/**
 * Return true if the listview type also has a widget representation in diagrams.
 */
bool typeIsCanvasWidget(Uml::ListView_Type type);

/**
 * Return true if the listview type is one of the predefined root views
 * (root, logical, usecase, component, deployment, datatype, or entity-
 * relationship view.)
 */
bool typeIsRootView(Uml::ListView_Type type);

/**
 * Return true if the listview type is a logical, usecase or component folder.
 */
bool typeIsFolder(Uml::ListView_Type type);

/**
 * Return true if the listview type may act as a container for other objects,
 * i.e. if it is a folder, package, subsystem, or component.
 */
bool typeIsContainer(Uml::ListView_Type type);

/**
 * Return true if the listview type is a diagram.
 */
bool typeIsDiagram(Uml::ListView_Type type);

/**
 * Return true if the listview type is an attribute, operation, or template.
 */
bool typeIsClassifierList(Uml::ListView_Type type);

/**
 * Return the Model_Type which corresponds to the given Diagram_Type.
 */
Uml::Model_Type convert_DT_MT(Uml::Diagram_Type dt);

/**
 * Return the ListView_Type which corresponds to the given Model_Type.
 */
Uml::ListView_Type convert_MT_LVT(Uml::Model_Type mt);

/**
 * Return the Model_Type which corresponds to the given ListView_Type.
 * Returns Uml::N_MODELTYPES if the list view type given does not map
 * to a Model_Type.
 */
Uml::Model_Type convert_LVT_MT(Uml::ListView_Type lvt);

/**
 * Convert a diagram type enum to the equivalent list view type.
 */
Uml::ListView_Type convert_DT_LVT(Uml::Diagram_Type dt);

/**
 * Converts a list view type enum to the equivalent object type.
 *
 * @param lvt               The ListView_Type to convert.
 * @return  The converted Object_Type if the listview type
 *          has a Uml::Object_Type representation, else 0.
 */
Uml::Object_Type convert_LVT_OT(Uml::ListView_Type lvt);

/**
 * Convert an object's type to the equivalent list view type
 *
 * @param o  Pointer to the UMLObject whose type shall be converted
 *           to the equivalent Uml::ListView_Type.  We cannot just
 *           pass in a Uml::Object_Type because a UMLFolder is mapped
 *           to different Uml::ListView_Type values, depending on its
 *           location in one of the predefined modelviews (Logical/
 *           UseCase/etc.)
 * @return  The equivalent Uml::ListView_Type.
 */
Uml::ListView_Type convert_OT_LVT(UMLObject *o);

/**
 * Return the Icon_Type which corresponds to the given listview type.
 *
 * @param lvt  ListView_Type to convert.
 * @return  The Uml::Icon_Type corresponding to the lvt.
 *          Returns it_Home in case no mapping to Uml::Icon_Type exists.
 */
Uml::Icon_Type convert_LVT_IT(Uml::ListView_Type lvt);

/**
 * Return the Diagram_Type which corresponds to the given listview type.
 *
 * @param lvt  ListView_Type to convert.
 * @return  The Uml::Diagram_Type corresponding to the lvt.
 *          Returns dt_Undefined in case no mapping to Diagram_Type exists.
 */
Uml::Diagram_Type convert_LVT_DT(Uml::ListView_Type lvt);

/**
 * Return the Model_Type which corresponds to the given Object_Type.
 */
Uml::Model_Type convert_OT_MT(Uml::Object_Type ot);

/**
 * Try to guess the correct container folder type of an UMLObject.
 * Object types that can't be guessed are mapped to Uml::mt_Logical.
 * NOTE: This function exists mainly for handling pre-1.5.5 files
 *       and should not be used for new code.
 */
Uml::Model_Type guessContainer(UMLObject *o);

/**
 * Parse a direction string into the Uml::Parameter_Direction.
 *
 * @param input  The string to parse: "in", "out", or "inout"
 *               optionally followed by whitespace.
 * @param result The corresponding Uml::Parameter_Direction.
 * @return       Length of the string matched, excluding the optional
 *               whitespace.
 */
int stringToDirection(QString input, Uml::Parameter_Direction & result);

/**
 * Return string corresponding to the given Uml::Programming_Language.
 */
QString progLangToString(Uml::Programming_Language pl);

/**
 * Return Uml::Programming_Language corresponding to the given string.
 */
Uml::Programming_Language stringToProgLang(QString str);

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
                QString initialValue = QString::null)
            : m_name(name), m_type(type),
              m_direction(direction), m_initialValue(initialValue) {
    }
};

/**
 * Auxiliary type for OpDescriptor
 */
typedef QValueList<NameAndType> NameAndType_List;
typedef QValueListIterator<NameAndType> NameAndType_ListIt;

/**
 * Data structure filled by parseOperation()
 */
struct OpDescriptor {
    QString m_name;
    NameAndType_List m_args;
    UMLObject *m_pReturnType;
};

/**
 * Parses a template parameter given in UML syntax.
 *
 * @param t             Input text of the template parameter.
 *                      Example:  parname : partype
 *                      or just:  parname          (for class type)
 * @param nmTp          NameAndType returned by this method.
 * @param owningScope   Pointer to the owning scope of the template param.
 * @return      Error status of the parse, PS_OK for success.
 */
Parse_Status parseTemplate(QString t, NameAndType& nmTp, UMLClassifier *owningScope);

/**
 * Parses an attribute given in UML syntax.
 *
 * @param a             Input text of the attribute in UML syntax.
 *                      Example:  argname : argtype
 * @param nmTp          NameAndType returned by this method.
 * @param owningScope   Pointer to the owning scope of the attribute.
 * @param vis           Optional pointer to visibility (return value.)
 *                      The visibility may be given at the beginning of the
 *                      attribute text in mnemonic form as follows:
 *                      "+"  stands for public
 *                      "#"  stands for protected
 *                      "-"  stands for private
 *                      "~"  stands for implementation level visibility
 *
 * @return      Error status of the parse, PS_OK for success.
 */
Parse_Status parseAttribute(QString a, NameAndType& nmTp, UMLClassifier *owningScope,
                            Uml::Visibility *vis = 0);

/**
 * Parses an operation given in UML syntax.
 *
 * @param m             Input text of the operation in UML syntax.
 *                      Example of a two-argument operation returning "void":
 *                      methodname (arg1name : arg1type, arg2name : arg2type) : void
 * @param desc          OpDescriptor returned by this method.
 * @param owningScope   Pointer to the owning scope of the operation.
 * @return      Error status of the parse, PS_OK for success.
 */
Parse_Status parseOperation(QString m, OpDescriptor& desc, UMLClassifier *owningScope);

/**
 * Returns the Parse_Status as a text.
 */
QString psText(Parse_Status value);

}

#endif
