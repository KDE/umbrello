/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2002 Heiko Nardmann <h.nardmann@secunet.de>
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef PHPWRITER_H
#define PHPWRITER_H

#include "simplecodegenerator.h"
#include "umlattributelist.h"
#include "umloperationlist.h"

/**
 * Class PhpWriter is a PHP code generator for UMLClassifier objects.
 * Just call writeClass and feed it a UMLClassifier.
 */
class PhpWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    PhpWriter();
    virtual ~PhpWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList reservedKeywords() const;

private:
    void writeOperations(UMLClassifier *c, QTextStream &php);
    void writeOperations(const QString& classname,
                         UMLOperationList &opList,
                         QTextStream &php);

    void writeAttributes(UMLClassifier *c, QTextStream &php);
    void writeAttributes(UMLAttributeList &atList, QTextStream &php);
};

#endif //PHPWRITER
