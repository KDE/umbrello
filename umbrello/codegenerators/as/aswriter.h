/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Alexander Blum <blum@kewbee.de>               *
 *   copyright (C) 2004-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ASWRITER_H
#define ASWRITER_H

#include "simplecodegenerator.h"
#include "umloperationlist.h"
#include "umlassociationlist.h"

class QTextStream;

/**
 * Class ASWriter is a ActionScript code generator for UMLClassifier objects.
 * Just call writeClass and feed it a UMLClassifier.
 */
class ASWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    ASWriter();
    virtual ~ASWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList reservedKeywords() const;

private:
    void writeOperations(QString classname, UMLOperationList *opList, QTextStream &as);
    void writeAssociation(QString& classname, UMLAssociationList& assoclist, QTextStream &as);

};

#endif //ASWRITER
