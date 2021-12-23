/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2002 Vincent Decorges <vincent.decorges@eivd.ch>
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    QStringList defaultDatatypes() const;

private:

    void writeAttributes(UMLAttributeList atList, QTextStream &py);

    void writeOperations(UMLClassifier *c, QTextStream &h);
    void writeOperations(const QString& classname, UMLOperationList &opList,
                         QTextStream &h, Uml::Visibility::Enum access);

    static bool hasContainer(const QString &string);

    static QString fixTypeName(const QString &string);
    QString findIncludeFromType(const QString &string);

    bool m_bNeedPass;  ///< True as long as no "pass" has been written
};

#endif //PYTHONWRITER
