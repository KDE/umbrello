/*  This file is part of the KDevelop PHP Documentation Plugin

    Copyright 2012 Milian Wolff <mail@milianw.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "phpdocsconfig.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KConfigGroup>
#include <KUrlRequester>
#include <KAboutData>
#include <KFile>
#include <ksettings/Dispatcher>

#include "phpdocssettings.h"
#include "ui_phpdocsconfig.h"

K_PLUGIN_FACTORY_WITH_JSON(PhpDocsConfigFactory, "kcm_kdevphpdocs.json", registerPlugin<PhpDocsConfig>();)

PhpDocsConfig::PhpDocsConfig(QWidget *parent, const QVariantList &args)
    : KCModule(KAboutData::pluginData(QStringLiteral("kcm_kdevphpdocs")), parent, args)
{
    QVBoxLayout * l = new QVBoxLayout( this );

    QWidget* w = new QWidget;
    m_configWidget = new Ui::PhpDocsConfigUI;
    m_configWidget->setupUi( w );

    m_configWidget->kcfg_phpDocLocation->setMode( KFile::Directory | KFile::ExistingOnly );

    l->addWidget( w );

    addConfig( PhpDocsSettings::self(), w );

    load();
}

void PhpDocsConfig::save()
{
    KCModule::save();

    // looks like we have to force a write so readConfig() can get the new values
    PhpDocsSettings::self()->save();

    KSettings::Dispatcher::reparseConfiguration( componentData().componentName() );
}

PhpDocsConfig::~PhpDocsConfig()
{
    delete m_configWidget;
}

#include "phpdocsconfig.moc"
