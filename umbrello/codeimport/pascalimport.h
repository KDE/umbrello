/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef PASCALIMPORT_H
#define PASCALIMPORT_H

#include "nativeimportbase.h"

/**
 * Pascal code import
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class PascalImport : public NativeImportBase
{
public:
    explicit PascalImport(CodeImpThread *thread = nullptr);
    virtual ~PascalImport();

protected:
    void initVars();

    bool parseStmt();

    QStringList split(const QString& line);

    void fillSource(const QString& word);

    void checkModifiers(bool& isVirtual, bool& isAbstract);

    bool m_inInterface;  ///< becomes true when keyword "interface" is seen

    enum Section_Type { sect_NONE, sect_LABEL, sect_CONST, sect_RESOURCESTRING,
                        sect_TYPE, sect_VAR, sect_THREADVAR };

    Section_Type m_section;  ///< contains the current section
};

#endif

