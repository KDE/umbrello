/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003       Alexander Blum  <blum@kewbee.de>             *
 *   copyright (C) 2004-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

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

    bool bPrivateSectionCommentIsWritten;  ///< write the comment "Private methods" only once

    void writeOperations(QString classname, UMLOperationList *opList, QTextStream &js);

    void writeAssociation(QString& classname, UMLAssociationList& assoclist , QTextStream &js);
};

#endif //JSWRITER
