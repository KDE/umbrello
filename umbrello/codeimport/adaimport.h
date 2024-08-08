/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ADAIMPORT_H
#define ADAIMPORT_H

#include "nativeimportbase.h"
#include "umlobjectlist.h"

#include <QMap>
#include <QStringList>

/**
 * Ada code import
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class AdaImport : public NativeImportBase
{
public:
    explicit AdaImport(CodeImpThread *thread = nullptr);
    virtual ~AdaImport();

protected:

    void initVars();

    bool parseStmt();

    QStringList split(const QString& line);

    void fillSource(const QString& word);

    QString expand(const QString& name);

    void parseStems(const QStringList& stems);

    bool m_inGenericFormalPart; ///< auxiliary variable

    /**
     * List for keeping track of tagged objects declared in the current scope.
     * This is required for distinguishing primitive from non primitive
     * methods.
     */
    UMLObjectList m_classesDefinedInThisScope;

    typedef QMap<QString, QString> StringMap;

    /**
     * Map of package renamings.
     * Keyed by the renaming. Value returns the expanded name.
     */
    StringMap m_renaming;

};

#endif

