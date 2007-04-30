/***************************************************************************
                          plugin.h
                             -------------------
    begin                : Mon Jan 13 2003
    copyright            : (C) 2003 by Andrew Sutton
    email                : ansutton@kent.edu
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "plugin.h"

// KDE includes
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>

// app includes
#include "pluginloader.h"

using namespace Umbrello;

Plugin::Plugin(QObject *parent,
               const char *name,
               const QStringList & /* args */) :
        QObject(parent, name),
        Configurable(),
        _ref(0),
        _instanceName(name),
        _config(NULL)
{
}

Plugin::~Plugin()
{
}

void
Plugin::ref()
{
    _ref++;
}

void
Plugin::unload()
{
    _ref--;
    if(_ref == 0) {
        // save the name
        QString pluginName = _instanceName;

        // shutdown and delete
        shutdown();
        delete this;

        // once the object is destroyed, we can have the plugin loader unload
        // the library.
        PluginLoader::instance()->unloadPlugin(pluginName);
    }
}

bool
Plugin::init()
{
    bool ret = true;

    // initialize this plugin first - then load other plugins
    ret = onInit();
    if(!ret) {
        kdError() << "failed to initialize " << instanceName() << endl;
    }

    // configure on load plugins
    if(ret) {
        ret = configure();
        if(!ret) {
            kdError() << "failed configuration " << instanceName() << endl;
        }
    }

    return true;
}

bool
Plugin::shutdown()
{
    bool ret = true;

    // always unload plugins, even if things are failing
    unloadPlugins();

    // shutdown this plugin
    ret = onShutdown();
    if(!ret) {
        kdError() << "failed to shutdown " << instanceName() << endl;
    }

    return true;
}

QCString
Plugin::instanceName() const
{
    return _instanceName;
}

KConfig *
Plugin::config()
{
    return _config;
}

bool
Plugin::onInit()
{
    return true;
}

bool
Plugin::onShutdown()
{
    return true;
}

bool
Plugin::configure()
{
    bool ret = true;

    // grab the OnStartup map
    KConfig *conf = config();
    if(!conf) {
        kdDebug() << "no configuration for " << instanceName() << endl;
        ret = false;
    }

    if(ret) {
        // set the config group to Load Actions
        conf->setGroup("Load Actions");

        // load standard plugins by default
        loadPlugins(conf, "Load");

        // only load GUI plugins if this is not a terminal app
        if(KApplication::kApplication()->type() != QApplication::Tty) {
            loadPlugins(conf, "LoadGUI");
        }
    }

    return ret;
}

QString
Plugin::category()
{
    return QString("miscellaneous");
}

#include "plugin.moc"
