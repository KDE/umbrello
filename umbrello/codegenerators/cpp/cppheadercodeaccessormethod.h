/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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
