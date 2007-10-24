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
 *   copyright (C) 2004-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "configurable.h"

// Qt includes
#include <qstringlist.h>

// KDE includes
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// local includes
#include "pluginloader.h"
#include "plugin.h"

using namespace Umbrello;


Configurable::Configurable() :
        _plugins()
{
}

Configurable::~Configurable()
{
    unloadPlugins();
}

bool
Configurable::loadPlugins(KConfig *config,
                          const QString &group,
                          const QString &key)
{
    bool ret = true;
    KConfigGroup grp(config, group);

    QStringList names = grp.readEntry(key,QStringList());
    for (int i = 0; i != names.size(); i++) {
        const QString &name = names[i];

        kDebug() << "loading plugin " << name;

        // load the plugin
        Plugin *plugin = PluginLoader::instance()->loadPlugin(name);

        // keep the plugin
        if(plugin) {
            _plugins.append(plugin);
        }
    }

    return ret;
}

bool
Configurable::unloadPlugins()
{
    // just iterate through and dereference all the
    // plugins.
    for(uint i = 0; i != _plugins.count(); i++) {
        Plugin *plugin = _plugins.at(i);
        plugin->unload();
    }
    _plugins.clear();
    return true;
}
