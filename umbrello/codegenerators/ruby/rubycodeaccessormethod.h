/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef RUBYCODEACCESSORMETHOD_H
#define RUBYCODEACCESSORMETHOD_H

#include "codeaccessormethod.h"
#include "rubycodeclassfield.h"

#include <QString>

class RubyCodeAccessorMethod : public CodeAccessorMethod
{
    Q_OBJECT
public:

    RubyCodeAccessorMethod(CodeClassField * field, CodeAccessorMethod::AccessorType type);
    virtual ~RubyCodeAccessorMethod();

    void update();

    virtual void updateMethodDeclaration();
    virtual void updateContent();

protected:

    virtual void setAttributesOnNode(QXmlStreamWriter& writer);
    virtual void setAttributesFromNode(QDomElement & element);

private:

};

#endif // RUBYCODEACCESSORMETHOD_H
