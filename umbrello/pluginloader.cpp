/***************************************************************************
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
#include "pluginloader.h"

// Qt includes
#include <qstring.h>

// KDE includes
#include <kdebug.h>
#include <klibloader.h>

// u2 includes
#include "plugin.h"

using namespace Umbrello;

// static data
PluginLoader *PluginLoader::_instance = NULL;

PluginLoader::PluginLoader() :
        _plugins(),
        _categories()
{
    // preseed the categories
    _categories["metamodel"] = PluginList();
    _categories["storage"] = PluginList();
    _categories["visual"] = PluginList();
}

PluginLoader::~PluginLoader()
{
}

PluginLoader *
PluginLoader::instance()
{
    if(!_instance) _instance = new PluginLoader;
    return _instance;
}

Plugin *
PluginLoader::loadPlugin(const QString &name)
{
    KLibrary *lib = NULL;
    KLibFactory *factory = NULL;
    Plugin *plugin = NULL;
    PluginMap::iterator it;
    bool success = true;

    // if the plugin has already been loaded, increment
    // its reference and return.
    if((it = _plugins.find(name)) != _plugins.end()) {
        plugin = it.data();
        plugin->ref();
        return plugin;
    }

    // use KLibLoader to get a reference to the library
    lib = KLibLoader::self()->library(name.latin1());
    if(!lib) {
        kdError() << "failed loading plugin library " << name << endl;
        success = false;
    }

    // get the factory from the library
    if(success) {
        factory = lib->factory();
        if(!factory) {
            kdError() << "failed to find factory for " << name << endl;
            success = false;
        }
    }

    // use the factory to create the plugin
    if(success) {
        plugin = dynamic_cast<Plugin *>(factory->create((QObject*)0, name.latin1()));
        if(!plugin) {
            kdError() << "failed to create a plugin object for " << name << endl;
            success = false;
        }
        else {
            // we have to register the plugin here, otherwise, we can get
            // recursive loads
            _plugins[name] = plugin;
            _categories[plugin->category()].append(plugin);
        }
    }

    // initialize the plugin
    if(success && plugin) {
        success = plugin->init();
        if(!success) {
            // on failure, delete the plugin. this should cause the
            // library to unload.
            kdError() << "failure initializing " << name << endl;
            _categories[plugin->category()].remove(plugin);
            _plugins.remove(name);
            delete plugin;
        }
    }

    // finally, finally connect to the destroyed signal and keep a
    // reference to it
    if(success) {
        plugin->ref();
        connect(plugin, SIGNAL(destroyed(QObject *)), SLOT(slotDestroyed(QObject *)));
    }

    return plugin;
}

Plugin *
PluginLoader::findPlugin(const QString &name)
{
    Plugin *ret = NULL;
    PluginMap::iterator it = _plugins.find(name);
    if(it != _plugins.end()) {
        ret = it.data();
    }
    return ret;
}

void
PluginLoader::unloadPlugin(const QString &name)
{
    KLibLoader::self()->unloadLibrary(name.latin1());
}

const PluginLoader::PluginMap &
PluginLoader::plugins() const
{
    return _plugins;
}

const PluginLoader::CategoryMap &
PluginLoader::categories() const
{
    return _categories;
}

void
PluginLoader::slotDestroyed(QObject *obj)
{
    Plugin *plugin = static_cast<Plugin *>(obj);

    // we can't just use the name because its already been destroyed
    // at this point. we have to iterate thru and find the reference
    // by hand.

    PluginMap::iterator end(_plugins.end());
    for(PluginMap::iterator i = _plugins.begin(); i != end; ++i) {
        Plugin *p = i.data();
        if(p == plugin) {
            kdDebug() << "unloading plugin " << i.key() << endl;

            // remove it from the mapping
            _plugins.remove(i);
            break;
        }
    }
}

#include "pluginloader.moc"
