/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006                                                     *
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
class PascalImport : public NativeImportBase {
public:
    PascalImport();
    virtual ~PascalImport();

protected:
    /**
     * Reimplement operation from NativeImportBase.
     */
    void initVars();

    /**
     * Implement abstract operation from NativeImportBase.
     */
    bool parseStmt();

    /**
     * Implement abstract operation from NativeImportBase.
     */
    void fillSource(const QString& word);

    /**
     * Check for, and skip over, all modifiers following a method.
     * Set the output arguments on encountering abstract and/or virtual.
     *
     * @param isVirtual   return value, set to true when "virtual" seen
     * @param isAbstract  return value, set to true when "abstract" seen
     */
    void checkModifiers(bool& isVirtual, bool& isAbstract);

    /**
     * Auxiliary variable, becomes true when keyword "interface" is seen
     */
    bool m_inInterface;

    enum Section_Type { sect_NONE, sect_LABEL, sect_CONST, sect_RESOURCESTRING,
                        sect_TYPE, sect_VAR, sect_THREADVAR };
    /**
     * Auxiliary variable, contains the current section
     */
    Section_Type m_section;
};

#endif

