/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPSOURCECODEACCESSORMETHOD_H
#define CPPSOURCECODEACCESSORMETHOD_H

#include "codeaccessormethod.h"

#include <QString>

class CodeClassField;

class CPPSourceCodeAccessorMethod : public CodeAccessorMethod
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    CPPSourceCodeAccessorMethod (CodeClassField * field, CodeAccessorMethod::AccessorType type);

    /**
     * Empty Destructor
     */
    virtual ~CPPSourceCodeAccessorMethod ();

    /**
     * Must be called before this object is usable
     */
    void update();

    virtual void updateMethodDeclaration();
    virtual void updateContent();

private:

};

#endif // CPPSOURCECODEACCESSORMETHOD_H
