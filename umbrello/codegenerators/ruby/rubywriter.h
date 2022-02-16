/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef RUBYWRITER_H
#define RUBYWRITER_H

#include "simplecodegenerator.h"
#include "umlattributelist.h"
#include "umloperationlist.h"

#include <QStringList>

/**
 * Class RubyWriter is a ruby code generator for UMLClassifier objects.
 * Just call writeClass and feed it a UMLClassifier.
 */
class RubyWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    RubyWriter();
    virtual ~RubyWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList reservedKeywords() const;

private:

    QString cppToRubyType(const QString &cppType);
    QString cppToRubyName(const QString &cppName);

    void writeAttributeMethods(UMLAttributeList attribs,
                               Uml::Visibility::Enum visibility, QTextStream &stream);

    void writeSingleAttributeAccessorMethods(const QString &fieldName,
                                             const QString &descr,
                                             QTextStream &h);

    void writeOperations(UMLClassifier *c, QTextStream &h);
    void writeOperations(const QString &classname, const UMLOperationList &opList,
                         Uml::Visibility::Enum permitScope, QTextStream &h);

};

#endif //RUBYWRITER
