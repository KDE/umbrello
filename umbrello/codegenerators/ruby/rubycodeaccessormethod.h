/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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

    virtual void setAttributesOnNode(QDomDocument & doc, QDomElement & blockElement);
    virtual void setAttributesFromNode(QDomElement & element);

private:

};

#endif // RUBYCODEACCESSORMETHOD_H
