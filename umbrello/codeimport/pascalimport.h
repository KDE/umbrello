/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006-2009                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef PASCALIMPORT_H
#define PASCALIMPORT_H

#include "nativeimportbase.h"

/**
 * Pascal code import
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PascalImport : public NativeImportBase
{
public:
    PascalImport(CodeImpThread* thread = 0);
    virtual ~PascalImport();

protected:
    void initVars();

    bool parseStmt();

    void fillSource(const QString& word);

    void checkModifiers(bool& isVirtual, bool& isAbstract);

    bool m_inInterface;  ///< becomes true when keyword "interface" is seen

    enum Section_Type { sect_NONE, sect_LABEL, sect_CONST, sect_RESOURCESTRING,
                        sect_TYPE, sect_VAR, sect_THREADVAR };

    Section_Type m_section;  ///< contains the current section
};

#endif

