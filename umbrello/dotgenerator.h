/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DOTGENERATOR_H

class UMLScene;
#include <QHash>
#include <QPointF>
#include <QString>

#define DOTGENERATOR_DEBUG
/**
 * The class DotGenerator provides export of diagrams as dot files.
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 */
class DotGenerator
{
public:
    DotGenerator();

    bool usePosition() const;
    void setUsePosition(bool state);

    bool useFullNodeLabels() const;
    void setUseFullNodeLabels(bool state);

    static bool availableConfigFiles(UMLScene *scene, QHash<QString, QString> &configFiles);
    bool readConfigFile(QString diagramType, const QString &variant = QLatin1String("default"));

    bool createDotFile(UMLScene *scene, const QString &fileName, const QString &variant = QLatin1String("default"));

    static QString currentDotPath();
    void setGeneratorName(const QString &name);
    QString generatorFullPath() const;

protected:
    bool findItem(QStringList &params, const QString &search);
    QString fixID(const QString &_id);
    int generatorVersion() const;

    double m_scale;        ///< scale factor
    QString m_configFileName; ///< template filename
    QHash<QString, QString> m_dotParameters;  ///< contains global graph parameters
    QHash<QString, QString> m_edgeParameters; ///< contains global edge parameters
    QHash<QString, QString> m_nodeParameters; ///< contains global node parameters
    QPointF m_origin;
    QString m_generator; ///< name of graphviz generator
    bool m_usePosition; ///< use position tag from dot (not used yet)
    bool m_useFullNodeLabels; ///< use full node labels
    QString m_dotPath;     ///< contains path to generator executable
    int m_version{0};         ///< version of graphviz generator

    friend QDebug operator<<(QDebug out, DotGenerator &c);
};

#endif
