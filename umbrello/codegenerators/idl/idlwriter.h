/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003-2021 Oliver Kellogg <okellogg@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef IDLWRITER_H
#define IDLWRITER_H

#include "simplecodegenerator.h"
#include "umlobject.h"

class UMLAssociation;
class UMLOperation;
class QTextStream;

/**
 * Class IDLWriter is a code generator for UMLClassifier objects.
 * Create an instance of this class, and feed it a UMLClassifier when
 * calling writeClass and it will generate an IDL interface for that
 * concept.
 */
class IDLWriter : public SimpleCodeGenerator
{
public:

    IDLWriter();
    virtual ~IDLWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    QStringList defaultDatatypes() const;

    virtual QStringList reservedKeywords() const;

private:

    void writeOperation(UMLOperation* op, QTextStream& idl, bool is_comment = false);

    void computeAssocTypeAndRole(UMLAssociation* a, UMLClassifier *c,
                                 QString& typeName, QString& roleName);

    static bool isOOClass(UMLClassifier* c);

    static bool assocTypeIsMappableToAttribute(Uml::AssociationType::Enum at);

};

#endif // IDLWRITER_H
