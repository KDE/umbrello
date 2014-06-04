/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002      Vincent Decorges  <vincent.decorges@eivd.ch>  *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef PYTHONWRITER_H
#define PYTHONWRITER_H

#include "simplecodegenerator.h"
#include "umlattributelist.h"
#include "umloperationlist.h"

/**
  * Class PythonWriter is a python code generator for UMLClassifier objects
  * Just call writeClass and feed it a UMLClassifier.
  *
  * NOTE: There is a unit test available for this class.
  *       Please, use and adapt it when necessary.
  */
class PythonWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    PythonWriter();
    virtual ~PythonWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList reservedKeywords() const;

    QStringList defaultDatatypes();

private:

    void writeAttributes(UMLAttributeList atList, QTextStream &py);

    void writeOperations(UMLClassifier *c, QTextStream &h);
    void writeOperations(const QString& classname, UMLOperationList &opList,
                         QTextStream &h, Uml::Visibility::Enum access);

    bool m_bNeedPass;  ///< True as long as no "pass" has been written
};

#endif //PYTHONWRITER
