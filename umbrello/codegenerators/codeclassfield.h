/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODECLASSFIELD_H
#define CODECLASSFIELD_H

#include "codeaccessormethodlist.h"
#include "codeclassfielddeclarationblock.h"
#include "codeparameter.h"
#include "codeaccessormethod.h"

class ClassifierCodeDocument;
class UMLAttribute;
class UMLObject;
class UMLRole;

/**
 * class CodeClassField
 * a special type of parameter.. occurs on class declarations.
 */
class CodeClassField : public CodeParameter
{
    Q_OBJECT
public:

    enum ClassFieldType { Attribute, PlainAssociation, Self, Aggregation, Composition, Unknown_Assoc };

    /**
     * Constructors
     */
    CodeClassField ( ClassifierCodeDocument * parentDoc , UMLAttribute * attrib );
    CodeClassField ( ClassifierCodeDocument * parentDoc , UMLRole * role);

    void finishInitialization();

    // CodeClassField ( ClassifierCodeDocument * doc , UMLRole role);

    /**
     * Empty Destructor
     */
    virtual ~CodeClassField ( );

//    /**
//     * Get the value of m_dialog
//     * @return the value of m_dialog
//     */
//    CodeClassFieldDialog getDialog ( );

    // Determine if this cf is attribute or some type of association
    QString getTypeName ( );

    CodeClassFieldDeclarationBlock * getDeclarationCodeBlock( );

    CodeAccessorMethodList getMethodList() const;

    CodeAccessorMethod * findMethodByType(CodeAccessorMethod::AccessorType type, int role_id = -1);

    bool fieldIsSingleValue ( );

    ClassFieldType getClassFieldType() const;

    // quick utility call to figure out if parent is an attribute or not
    bool parentIsAttribute ( ) const;

    // get the type of object that will be added/removed from lists
    // of objects (as per specification of associations)
    QString getListObjectType();

    bool getWriteOutMethods () const;

    void setWriteOutMethods( bool val);

    int minimumListOccurances( );

    int maximumListOccurances( );

    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    virtual void loadFromXMI ( QDomElement & root );

    virtual void synchronize ();

    void updateContent();

protected:

    void setParentUMLObject (UMLObject * obj);

    //  CodeClassFieldDialog * m_dialog;

    QString cleanName(const QString &name);

    QString fixInitialStringDeclValue(const QString& val, const QString &type);

    // set the list class name
    void setListClassName ( const QString &className );

    bool addMethod ( CodeAccessorMethod * add );

    QString getUMLObjectName(UMLObject *obj);

    bool removeMethod ( CodeAccessorMethod * remove);

private:

    QString m_listClassName;
    ClassFieldType m_classFieldType;
    CodeClassFieldDeclarationBlock * m_declCodeBlock;
    CodeAccessorMethodList m_methodVector; // the list of methods related to this codeclassfield
    bool m_parentIsAttribute;

    /**
     * This flag tells if we want the methods to have the possibility
     * of being written out. IF the value is false, then all methods
     * are never written out.
     */
    bool m_writeOutMethods;

    virtual void setAttributesOnNode ( QDomDocument & doc, QDomElement & blockElement);

    virtual void setAttributesFromNode ( QDomElement & element);

    void initFields(bool inConstructor = false);

    // initialize the accessor methods for this field
    void initAccessorMethods();

signals:

    void modified ();

};

#endif // CODECLASSFIELD_H
