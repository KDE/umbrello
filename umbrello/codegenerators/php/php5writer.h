/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2002 Heiko Nardmann <h.nardmann@secunet.de>
    SPDX-FileCopyrightText: 2004 Thorsten Kunz <tk AT bytecrash DOT net>
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef PHP5WRITER_H
#define PHP5WRITER_H

#include "simplecodegenerator.h"
#include "umlattributelist.h"
#include "umloperationlist.h"

/**
 * Class Php5Writer is a PHP code generator for UMLClassifier objects.
 * Just call writeClass and feed it a UMLClassifier.
 */
class Php5Writer : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    Php5Writer();
    virtual ~Php5Writer();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList reservedKeywords() const;

private:
    void writeOperations(UMLClassifier *c, QTextStream &php);
    void writeOperations(const QString &classname, UMLOperationList &opList,
                         QTextStream &php,
                         bool interface = false, bool generateErrorStub = false);

    void writeAttributes(UMLClassifier *c, QTextStream &php);
    void writeAttributes(UMLAttributeList &atList, QTextStream &php);
};

#endif //PHP5WRITER

