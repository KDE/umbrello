/*
    SPDX-FileCopyrightText: 2003 Andrew Sutton <ansutton@kent.edu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// own header
#include "plugin.h"

// KDE includes
#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>

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
        _config(0)
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
        kError() << "failed to initialize " << instanceName() << endl;
    }

    // configure on load plugins
    if(ret) {
        ret = configure();
        if(!ret) {
            kError() << "failed configuration " << instanceName() << endl;
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
        kError() << "failed to shutdown " << instanceName() << endl;
    }

    return true;
}

QByteArray
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
        kDebug() << "no configuration for " << instanceName();
        ret = false;
    }

    if(ret) {
        // load standard plugins by default
        loadPlugins(conf, "Load Actions", "Load");

        // only load GUI plugins if this is not a terminal app
        if(KApplication::kApplication()->type() != QApplication::Tty) {
            loadPlugins(conf, "Load Actions", "LoadGUI");
        }
    }

    return ret;
}

QString
Plugin::category()
{
    return QString("miscellaneous");
}

