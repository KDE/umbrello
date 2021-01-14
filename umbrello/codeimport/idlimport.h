/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005-2021                                                *
 *  Umbrello UML Modeller Authors <umbrello-devel@kde.org>                 *
 ***************************************************************************/

#ifndef IDLIMPORT_H
#define IDLIMPORT_H

#include "nativeimportbase.h"

class UMLDoc;

/**
 * CORBA IDL code import
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class IDLImport : public NativeImportBase {
public:
    explicit IDLImport(CodeImpThread* thread = 0);
    virtual ~IDLImport();

    bool parseStmt();

    bool parseFile(const QString& file);

    bool preprocess(QString& line);

    void fillSource(const QString& word);

    bool skipStructure();

    bool isValidScopedName(QString text);

protected:
    QString joinTypename();
    UMLDoc *m_doc;
    bool m_isOneway, m_isReadonly, m_isAttribute, m_isUnionDefault;
    QStringList m_unionCases;
    static QString m_preProcessor;
    static QStringList m_preProcessorArguments;
    static bool m_preProcessorChecked;
};

#endif

