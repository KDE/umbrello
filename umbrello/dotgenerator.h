/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DOTGENERATOR_H

class UMLScene;
#include <QHash>
#include <QPointF>

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

    bool usePosition();
    void setUsePosition(bool state);

    bool useFullNodeLabels();
    void setUseFullNodeLabels(bool state);

    static bool availableConfigFiles(UMLScene *scene, QHash<QString,QString> &configFiles);
    bool readConfigFile(QString diagramType, const QString &variant = "default");

    bool createDotFile(UMLScene *scene, const QString &fileName, const QString &variant = "default");

protected:
    bool findItem(QStringList &params, const QString &search);
    QString fixID(const QString &_id);

    double m_scale;        ///< scale factor
    QString m_configFileName; ///< template filename
    QHash<QString, QString> m_dotParameters;  ///< contains global graph parameters
    QHash<QString, QString> m_edgeParameters; ///< contains global edge parameters
    QHash<QString, QString> m_nodeParameters; ///< contains global node parameters
    QPointF m_origin;
    QString m_generator; ///< name of graphviz generator
    bool m_usePosition; ///< use position tag from dot (not used yet)
    bool m_useFullNodeLabels; ///< use full node labels

    friend QDebug operator<<(QDebug out, DotGenerator &c);
};

#endif
