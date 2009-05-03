/***************************************************************************
                          pluginloader.h
                             -------------------
    begin                : Mon Jan 13 2003
    copyright            : (C) 2003 by Andrew Sutton
    email                : ansutton@kent.edu
  Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMBRELLO_PLUGINLOADER_H
#define UMBRELLO_PLUGINLOADER_H

// Qt includes
#include <Q3ValueList>
#include <QMap>

// forward declarations
class QString;

namespace Umbrello
{
// forward declarations
class Plugin;

/**
 * @ingroup U2_Lib
 *
 * The plugin loader is an abstraction that sits on top of KLibLoader.
 * Whereas plugins are specialized shared objects, the plugin must
 * specialize the loading of those objects. Essentially, the plugin
 * loader provides a single unit of functionality - loading plugins.
 * In order to load a plugin, we must first load a shared library
 * and then use the libraries factory to create the plugin. However,
 * because a plugin is required to be a singleton, we must ensure
 * that no plugin is ever created more than once. To that end, the
 * loader must also retain a map of loaded plugins. When a loaded
 * plugin is requested, we can increase its reference count.
 *
 * On the subject of unloading, we actually have very little to do.
 * The unload method on a plugin is simply a reference decrementer.
 * When it reaches 0, the object destroys itself. Being a QObject,
 * it will emit the destroyed signal just before deletion, allowing
 * the plugin loader to respond to the event and remove the plugin
 * from its mapping.
 *
 * The PluginLoader also manages categories of plugins. The runtime
 * categories actually reflect the directory structure of the build
 * environment with each category represented by the name of a
 * directory. The categories are "pre-seeded" at startup.
 *
 * @bug Plugins are not removed from their respective categories
 * when they are destroyed. It may be acceptable to call
 * Plugin::category() from slotDestroyed because the category()
 * method doesn't reference any local variables - it just returns
 * a string.
 */
class PluginLoader : public QObject
{
    Q_OBJECT
public:

    /** Destry the plugin loader */
    ~PluginLoader();


    /** Just a container of plugins */
    typedef Q3ValueList<Plugin *> PluginList;

    /** The containment type for mapping plugins */
    typedef QMap<QString, Plugin *> PluginMap;

    /** Container of plugin categories */
    typedef QMap<QString, PluginList> CategoryMap;

    /** Singleton accessor */
    static PluginLoader *instance();

    /**
     * Load a plugin. Test to see if the plugin already exists. If it
     * does, just add a reference to it and continue on.
     */
    Plugin *loadPlugin(const QString &name);

    /** Find a plugin */
    Plugin *findPlugin(const QString &name);

    /**
     * Unload a plugin. Never use this method. It is only used by the deref
     * method of a plugin to cause this class to unload the corresponding
     * library. In fact, there is actually no corresponding plugin to unload,
     * we just unload the library.
     */
    void unloadPlugin(const QString &name);

    /**
     * Get a reference to the plugin mapping. This method wraps everything
     * in consts with the express purpose that no changes are made to the
     * plugin map after using this method.
     */
    const PluginMap &plugins() const;

    /** Get a reference to the plugin category mapping. */
    const CategoryMap &categories() const;

private slots:
    /**
     * This is used to connect to the destroyed signal emitted by plugins
     * when they are finally deleted. The plugin loader uses this signal
     * to remove the plugin from the plugin map.
     */
    void slotDestroyed(QObject *obj);

private:
    /** Private constructor - This must be created through the instance method */
    PluginLoader();

    static PluginLoader        *_instance;      ///< Singleton instance
    PluginMap                   _plugins;       ///< The plugin mapping
    CategoryMap                 _categories;    ///< Categories of plugins
};
}

#endif
