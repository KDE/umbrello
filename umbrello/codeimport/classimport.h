/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CLASSIMPORT_H
#define CLASSIMPORT_H

#include <QStringList>

class CodeImpThread;

/**
 * Interfaces classparser library to uml models
 * Abstract base for programming language specific import classes
 * @author Mikko Pasanen
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ClassImport
{
public:
    ClassImport(CodeImpThread* thread = 0);
    virtual ~ClassImport();

    bool importFiles(const QStringList& fileNames);
    bool importFile(const QString& fileName);
    void setRootPath(const QString &path);

    /**
     * Return state of the importer. It may be disabled because of 
     * missing dependencies for example. 
     * @return false - disabled, true - enabled
    */
    bool enabled() const { return m_enabled; }

    static ClassImport *createImporterByFileExt(const QString &fileName, CodeImpThread* thread = 0);

protected:

    /**
     * Initialize the importer.
     * This is called by importFiles() once, before entering
     * the loop for importing one or more files.
     * To be implemented by inheriting classes.
     */
    virtual void initialize() = 0;

    virtual void initPerFile();

    /**
     * Import a single file.
     * To be implemented by inheriting classes.
     *
     * @param fileName  The file to import.
     */
    virtual bool parseFile(const QString& fileName) = 0;

    void log(const QString& file, const QString& text);
    void log(const QString& text);

    CodeImpThread* m_thread;  ///< thread in which the work of importing is done
    bool m_enabled;           ///< state of importer
    QString m_rootPath;       ///< root path of import
};

#endif
