/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ENUM_H
#define ENUM_H

#include "umlclassifier.h"

class UMLEnumLiteral;

/**
 * This class contains the non-graphical information required for a UML
 * Enum.
 * This class inherits from @ref UMLClassifier which contains most of the
 * information.
 *
 * @short Non-graphical Information for an Enum.
 * @author Jonathan Riddell
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLEnum : public UMLClassifier
{
    Q_OBJECT
public:
    explicit UMLEnum(const QString& name = QString(), Uml::ID::Type id = Uml::ID::None);

    virtual ~UMLEnum();

    bool operator==(const UMLEnum& rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    UMLObject* createEnumLiteral(const QString& name = QString());

    UMLObject* addEnumLiteral(const QString &name,
                              Uml::ID::Type id = Uml::ID::None,
                              const QString& value = QString());

    bool addEnumLiteral(UMLEnumLiteral *literal, IDChangeLog *Log = nullptr);
    bool addEnumLiteral(UMLEnumLiteral* literal, int position);

    int removeEnumLiteral(UMLEnumLiteral* literal);

    int enumLiterals() const;

    void signalEnumLiteralRemoved(UMLClassifierListItem *elit);

    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual UMLClassifierListItem* makeChildObject(const QString& xmiTag);

Q_SIGNALS:
    void enumLiteralAdded(UMLClassifierListItem*);
    void enumLiteralRemoved(UMLClassifierListItem*);

protected:
    bool load1(QDomElement & element);

private:
    void init();

};

#endif // ENUM_H

