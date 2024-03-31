/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    CodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib);
    CodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role);

    void finishInitialization();

    // CodeClassField (ClassifierCodeDocument * doc, UMLRole role);

    virtual ~CodeClassField ();

//    /**
//     * Get the value of m_dialog
//     * @return the value of m_dialog
//     */
//    CodeClassFieldDialog getDialog ();

    // Determine if this cf is attribute or some type of association
    QString getTypeName ();

    CodeClassFieldDeclarationBlock * getDeclarationCodeBlock();

    CodeAccessorMethodList getMethodList() const;

    CodeAccessorMethod * findMethodByType(CodeAccessorMethod::AccessorType type, int role_id = -1);

    bool fieldIsSingleValue ();

    ClassFieldType getClassFieldType() const;

    // quick utility call to figure out if parent is an attribute or not
    bool parentIsAttribute () const;

    // get the type of object that will be added/removed from lists
    // of objects (as per specification of associations)
    QString getListObjectType();

    bool getWriteOutMethods () const;

    void setWriteOutMethods(bool val);

    int minimumListOccurances();

    int maximumListOccurances();

    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual void loadFromXMI (QDomElement & root);

    virtual void synchronize ();

    void updateContent();

    Q_SIGNAL void modified ();

protected:

    void setParentUMLObject (UMLObject * obj);

    //  CodeClassFieldDialog * m_dialog;

    QString cleanName(const QString &name);

    QString fixInitialStringDeclValue(const QString& val, const QString &type);

    // set the list class name
    void setListClassName (const QString &className);

    bool addMethod (CodeAccessorMethod * add);

    QString getUMLObjectName(UMLObject *obj);

    bool removeMethod (CodeAccessorMethod * remove);

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

    virtual void setAttributesOnNode (QXmlStreamWriter& writer);

    virtual void setAttributesFromNode (QDomElement & element);

    void initFields(bool inConstructor = false);

    // initialize the accessor methods for this field
    void initAccessorMethods();
};

#endif // CODECLASSFIELD_H
