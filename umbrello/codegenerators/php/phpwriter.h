/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002      Heiko Nardmann  <h.nardmann@secunet.de>       *
 *   copyright (C) 2003-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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

    bool bPrivateSectionCommentIsWritten;  ///< write the comment "Private methods" only once

    void writeOperations(UMLClassifier *c, QTextStream &php);
    void writeOperations(const QString& classname,
                         UMLOperationList &opList,
                         QTextStream &php);

    void writeAttributes(UMLClassifier *c, QTextStream &php);
    void writeAttributes(UMLAttributeList &atList, QTextStream &php);
};

#endif //PHPWRITER
