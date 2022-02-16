/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPHEADERCODEACCESSORMETHOD_H
#define CPPHEADERCODEACCESSORMETHOD_H

#include "codeaccessormethod.h"

#include <QString>

class CodeClassField;

class CPPHeaderCodeAccessorMethod : public CodeAccessorMethod
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    CPPHeaderCodeAccessorMethod (CodeClassField * field, CodeAccessorMethod::AccessorType type);

    /**
     * Empty Destructor
     */
    virtual ~CPPHeaderCodeAccessorMethod ();

    /**
     * Must be called before this object is usable
     */
    void update();

    virtual void updateMethodDeclaration();
    virtual void updateContent();

private:

};

#endif // CPPHEADERCODEACCESSORMETHOD_H
