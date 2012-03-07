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

// app includes
#include "associationwidget.h"
#include "debug_utils.h"
#include "umlwidget.h"

// qt includes
#include <QFile>
#include <QHash>
#include <QProcess>
#include <QRectF>
#include <QRegExp>
#include <QString>
#include <QTemporaryFile>
#include <QTextStream>
#include <QtDebug>

// kde includes
#include <KConfigGroup>
#include <KDesktopFile>
#include <KStandardDirs>


#define DOTGENERATOR_DEBUG
/**
 * The class DotGenerator provides export of diagrams as dot files.
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 */
class DotGenerator
{
public:
    /**
     * constructor
    */
    DotGenerator()
      : m_scale(72),
        m_usePosition(false)
    {
    }

    /**
     * set usage of position attribute in dot file
     *
     * @param state The new state
    */
    void setUsePosition(bool state)
    {
        m_usePosition = state;
    }

    /**
     * return usage of position attribute
     *
     * @return true if position are used
    */
    bool usePosition()
    {
        return m_usePosition;
    }

    /**
     * Return a list of available templates for a given scene type
     *
     * @param scene The diagram
     * @param configFiles will contain the collected list of config files
     * @return true if collecting succeeds
     */
    static bool availableConfigFiles(UMLScene *scene, QHash<QString,QString> &configFiles)
    {
        QString diagramType = scene->type().toString().toLower();
        KStandardDirs dirs;

        QStringList fileNames = dirs.findAllResources("data", QString("umbrello/layouts/%1*.desktop").arg(diagramType));
        foreach(const QString &fileName, fileNames) {
            QFileInfo fi(fileName);
            QString baseName;
            if (fi.baseName().contains("-"))
                baseName = fi.baseName().remove(diagramType + "-");
            else if (fi.baseName() == diagramType)
                baseName = fi.baseName();
            else
                baseName = "default";
            KDesktopFile desktopFile(fileName);
            configFiles[baseName] = desktopFile.readName();
        }
        return true;
    }

    /**
     * Read a layout config file
     *
     * @param diagramType String identifing the diagram
     * @param variant String identifing the variant
     * @return true on success
     */
    bool readConfigFile(QString diagramType, const QString &variant = "default")
    {
        QStringList fileNames;

        if (!variant.isEmpty())
            fileNames << QString("%1-%2.desktop").arg(diagramType).arg(variant);
        fileNames << QString("%1-default.desktop").arg(diagramType);
        fileNames << "default.desktop";

        QString configFileName;
        foreach(const QString &fileName, fileNames) {
            configFileName = KStandardDirs::locate("data", QString("umbrello/layouts/%1").arg(fileName));
            if (!configFileName.isEmpty())
                break;
        }

        if (configFileName.isEmpty()) {
            uError() << "could not find layout config file name for diagram type" << diagramType << "and variant" << variant;
            return false;
        }

        m_configFileName = configFileName;
        KDesktopFile desktopFile(configFileName);
        KConfigGroup edgesAttributes(&desktopFile,"X-UMBRELLO-Dot-Edges");
        KConfigGroup nodesAttributes(&desktopFile,"X-UMBRELLO-Dot-Nodes");
        KConfigGroup attributes(&desktopFile,"X-UMBRELLO-Dot-Attributes");
        // settings are not needed by dotgenerator
        KConfigGroup settings(&desktopFile,"X-UMBRELLO-Dot-Settings");

        m_edgeParameters.clear();
        m_nodeParameters.clear();
        m_dotParameters.clear();

        foreach(const QString &key, attributes.keyList()) {
            QString value = attributes.readEntry(key);
            if (!value.isEmpty())
                m_dotParameters[key] = value;
        }

        foreach(const QString &key, nodesAttributes.keyList()) {
            QString value = nodesAttributes.readEntry(key);
            m_nodeParameters[key] = value;
        }

        foreach(const QString &key, edgesAttributes.keyList()) {
            QString value = edgesAttributes.readEntry(key);
            if (m_edgeParameters.contains(key)) {
                m_edgeParameters[key] += ',' + value;
            } else {
                m_edgeParameters[key] = value;
            }
        }

        QString value = settings.readEntry("origin");
        QStringList a = value.split(",");
        if (a.size() == 2)
            m_origin = QPointF(a[0].toDouble(), a[1].toDouble());
        else
            uError() << "illegal format of entry 'origin'" << value;

#ifdef LAYOUTGENERATOR_DATA_DEBUG
        uDebug() << m_edgeParameters;
        uDebug() << m_nodeParameters;
        uDebug() << m_dotParameters;
#endif
        return true;
    }

    /**
     * Create dot file using displayed widgets
     * and associations of the provided scene
     * @note This method could also be used as a base to export diagrams as dot file
     *
     * @param fileName Filename where to create the dot file
     * @param scene The diagram from which the widget informations are fetched
     *
     * @return true if generating finished successfully
    */
    bool createDotFile(UMLScene *scene, const QString &fileName, const QString &variant = "default")
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;

        QString diagramType = scene->type().toString().toLower();
        if (!readConfigFile(diagramType, variant))
            return false;

        QString data;
        QTextStream out(&data);

        foreach(UMLWidget *widget, scene->getWidgetList()) {
            QStringList params;

            if (m_nodeParameters.contains("all"))
                params << m_nodeParameters["all"];

            params  << QString("width=\"%1\"").arg(widget->getWidth()/m_scale)
                    << QString("height=\"%1\"").arg(widget->getHeight()/m_scale);

            if (usePosition())
                params  << QString("pos=\"%1,%2\"").arg(widget->getX()+widget->getWidth()/2).arg(widget->getY()+widget->getHeight()/2);

            QString type = QString(widget->baseTypeStr()).toLower().remove("wt_");
            QString key = "type::" + type;
            QString label = widget->name() + "\\n" + type;

            if (m_nodeParameters.contains(key))
                params << m_nodeParameters[key];
            else if (m_nodeParameters.contains("type::default")) {
                params << m_nodeParameters["type::default"];
                if (label.isEmpty())
                    label = type;
            }

            params << QString("label=\"%1\"").arg(label);

#ifdef DOTGENERATOR_DATA_DEBUG
            uDebug() << type << params;
#endif
            QString id = fixID(ID2STR(widget->id()));
            if (widget->baseType() != WidgetBase::wt_Text)
                out << "\"" << id << "\""
                    << " [" << params.join(",") << "];\n";
        }

        foreach(AssociationWidget *assoc, scene->getAssociationList()) {
            QStringList params;
            QString label;
            QString type = assoc->associationType().toString().toLower();
            QString key = "type::" + type;
            label = type;
            QString edgeParameters;

            QString rkey = QLatin1String("ranking::") + key;
            if (m_edgeParameters.contains(rkey))
                edgeParameters = m_edgeParameters[rkey];
            else if (m_edgeParameters.contains("ranking::type::default")) {
                edgeParameters = m_edgeParameters["ranking::type::default"];
            }
            params << edgeParameters;

            QString vkey = QLatin1String("visual::") + key;
            if (m_edgeParameters.contains(vkey))
                edgeParameters = m_edgeParameters[vkey];
            else if (m_edgeParameters.contains("visual::type::default")) {
                edgeParameters = m_edgeParameters["visual::type::default"];
            }
            params << edgeParameters;

            QString aID = fixID(ID2STR(assoc->getWidgetID(Uml::A)));
            QString bID = fixID(ID2STR(assoc->getWidgetID(Uml::B)));
            params << QString("label=\"%1\"").arg(type);

#ifdef DOTGENERATOR_DATA_DEBUG
            uDebug() << type << params;
#endif
            if (m_edgeParameters.contains("id::" + key) && m_edgeParameters["id::" + key] == "swap")
                out << "\"" << aID << "\" -> \"" << bID << "\""
                    << " [" << params.join(",") << "];\n";
            else
                out << "\"" << bID << "\" -> \"" << aID << "\""
                    << " [" << params.join(",") << "];\n";
        }

        QTextStream o(&file);
        o << "# generated from " << m_configFileName << "\n";
        o << "digraph G {\n";

        foreach(const QString &key, m_dotParameters.keys()) {
            o << "\t" << key << " [" << m_dotParameters[key] << "];\n";
        }

        o << data << "\n";
        o << "}\n";

        return true;
    }

protected:
    /**
     * There are id wrapped with '"', remove it.
     */
    QString fixID(const QString &_id)
    {
        // FIXME: some widget's ids returned from the list are wrapped with "\"", find and fix them
        QString id(_id);
        id.remove("\"");
        return id;
    }

    double m_scale;        ///< scale factor
    QString m_executable;  ///< dot executable
    QString m_configFileName; ///< template filename
    QHash<QString, QString> m_dotParameters;  ///< contains global graph parameters
    QHash<QString, QString> m_edgeParameters; ///< contains global edge parameters
    QHash<QString, QString> m_nodeParameters; ///< contains global node parameters
    QPointF m_origin;
    bool m_usePosition;

    friend QDebug operator<<(QDebug out, DotGenerator &c);
};

#if 0
static QDebug operator<<(QDebug out, LayoutGenerator &c)
{
    out << "LayoutGenerator:"
        << "m_boundingRect:" << c.m_boundingRect
        << "m_nodes:" << c.m_nodes
        << "m_edges:" << c.m_edges
        << "m_scale:" << c.m_scale
        << "m_executable:" << c.m_executable;
    return out;
}
#endif
#endif
