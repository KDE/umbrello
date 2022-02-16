/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Alexander Blum <blum@kewbee.de>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JSWRITER_H
#define JSWRITER_H

#include "simplecodegenerator.h"
#include "umloperationlist.h"
#include "umlassociationlist.h"

class QTextStream;

/**
 * Class JSWriter is a JavaScript code generator for UMLClassifier objects.
 * Just call writeClass and feed it a UMLClassifier.
 */
class JSWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    JSWriter();
    virtual ~JSWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList reservedKeywords() const;

private:
    void writeOperations(QString classname, UMLOperationList *opList, QTextStream &js);

    void writeAssociation(QString& classname, UMLAssociationList& assoclist, QTextStream &js);
};

#endif //JSWRITER
