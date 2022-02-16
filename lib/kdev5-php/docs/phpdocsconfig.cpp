/*  This file is part of the KDevelop PHP Documentation Plugin

    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
