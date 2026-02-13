/*
 *    SPDX-License-Identifier: GPL-2.0-or-later
 *
 *    SPDX-FileCopyrightText: 2003 Alexander Blum <blum@kewbee.de>
 *    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
 */

#ifndef TSWRITER_H
#define TSWRITER_H

#include "simplecodegenerator.h"
#include "umloperationlist.h"
#include "umlassociationlist.h"
#include "umlenum.h"

class QTextStream;

/**
 * Class TSWriter is a TypeScript code generator for UMLClassifier objects.
 * Just call writeClass and feed it a UMLClassifier.
 */
class TSWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    TSWriter();
    virtual ~TSWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList reservedKeywords() const;

    QStringList defaultDatatypes() const;

private:

    void writeEnum(UMLEnum *c);

    void writeOperations(QString classname, UMLOperationList *opList, QTextStream &ts, bool isInterface);

    void writeAssociation(QString& classname, UMLClassifier* c, QTextStream &ts);
};

#endif // TSWRITER_H
