/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef OPERATION_H
#define OPERATION_H

#include "umlattributelist.h"
#include "classifierlistitem.h"

class UMLClassifier;

/**
 * This class represents an operation in the UML model.
 *
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLOperation : public UMLClassifierListItem
{
    Q_OBJECT
public:
    UMLOperation(UMLClassifier * parent, const QString& name,
                 Uml::IDType id = Uml::id_None,
                 Uml::Visibility s = Uml::Visibility::Public,
                 UMLObject *rt = 0);

    UMLOperation(UMLClassifier * parent);

    virtual ~UMLOperation();

    bool operator==( const UMLOperation & rhs ) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    void setType(UMLObject* type);

    void moveParmLeft(UMLAttribute *a);
    void moveParmRight(UMLAttribute *a);

    void removeParm(UMLAttribute *a, bool emitModifiedSignal = true);

    UMLAttributeList getParmList() const;

    UMLAttribute * findParm(const QString &name);

    QString toString(Uml::SignatureType sig = Uml::SignatureType::NoSig);

    void addParm(UMLAttribute *parameter, int position = -1);

    bool resolveRef();

    QString getUniqueParameterName();

    bool showPropertiesDialog(QWidget* parent);

    bool isConstructorOperation();
    bool isDestructorOperation();
    bool isLifeOperation();

    void setConst(bool b);
    bool getConst() const;

    void setSourceCode(const QString& code);
    QString getSourceCode() const;

    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
    bool load( QDomElement & element );

private:
    Uml::IDType      m_returnId;  ///< Holds the xmi.id of the <UML:Parameter kind="return">
    UMLAttributeList m_List;      ///< Parameter list
    bool             m_bConst;    ///< Holds the isQuery attribute of the <UML:Operation>
    QString          m_Code;      ///< Holds the entered source code
};

#endif
