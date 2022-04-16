/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef OPERATION_H
#define OPERATION_H

#include "umlattributelist.h"
#include "classifierlistitem.h"

class UMLClassifier;

/**
 * This class represents an operation in the UML model.
 *
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLOperation : public UMLClassifierListItem
{
    Q_OBJECT
public:
    UMLOperation(UMLClassifier * parent, const QString& name,
                 Uml::ID::Type id = Uml::ID::None,
                 Uml::Visibility::Enum s = Uml::Visibility::Public,
                 UMLObject *rt = 0);
    explicit UMLOperation(UMLClassifier * parent);
    virtual ~UMLOperation();

    bool operator==(const UMLOperation & rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    void setType(UMLObject* type);

    void moveParmLeft(UMLAttribute *a);
    void moveParmRight(UMLAttribute *a);

    void removeParm(UMLAttribute *a, bool emitModifiedSignal = true);

    UMLAttributeList getParmList() const;

    UMLAttribute * findParm(const QString &name) const;

    QString toString(Uml::SignatureType::Enum sig = Uml::SignatureType::NoSig,
                     bool withStereotype=false) const;

    void addParm(UMLAttribute *parameter, int position = -1);

    bool resolveRef();

    QString getUniqueParameterName() const;

    virtual bool showPropertiesDialog(QWidget* parent = 0);

    bool isConstructorOperation() const;
    bool isDestructorOperation() const;
    bool isLifeOperation() const;

    void setConst(bool b);
    bool getConst() const;
    void setOverride(bool b);
    bool getOverride() const;
    void setFinal(bool b);
    bool getFinal() const;
    void setVirtual(bool b);
    bool isVirtual() const;
    void setInline(bool b);
    bool isInline() const;

    void setSourceCode(const QString& code);
    QString getSourceCode() const;

    void saveToXMI(QXmlStreamWriter& writer);

protected:
    bool load1(QDomElement & element);

private:
    Uml::ID::Type    m_returnId;  ///< Holds the xmi.id of the <UML:Parameter kind="return">
    UMLAttributeList m_args;      ///< Parameter list
    bool             m_bConst;    ///< Holds the isQuery attribute of the <UML:Operation>
    bool             m_bOverride; ///< Holds the override attribute of the <UML:Operation>
    bool             m_bFinal;    ///< Holds the final attribute of the <UML:Operation>
    bool             m_bVirtual;  ///< Holds the virtual attribute of the <UML:Operation>
    bool             m_bInline;   ///< Holds the inline attribute of the <UML:Operation>
    QString          m_Code;      ///< Holds the entered source code
};

#endif
