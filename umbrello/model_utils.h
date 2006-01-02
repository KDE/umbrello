/*
 *  copyright (C) 2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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
UMLObject * findObjectInList(Uml::IDType id, UMLObjectList inList);

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
UMLObject* findUMLObject( UMLObjectList inList, QString name,
                          Uml::Object_Type type = Uml::ot_UMLObject,
                          UMLObject *currentObj = NULL);

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
 * Return string corresponding to the Uml::Programming_Language.
 */
QString progLang(Uml::Programming_Language pl);

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
 * @return      Error status of the parse, PS_OK for success.
 */
Parse_Status parseAttribute(QString a, NameAndType& nmTp, UMLClassifier *owningScope);

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
