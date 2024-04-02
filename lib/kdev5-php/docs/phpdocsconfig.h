/*  This file is part of the KDevelop PHP Documentation Plugin

    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PHPDOCS_CONFIG_H
#define PHPDOCS_CONFIG_H

#include <KCModule>

#include "ui_phpdocsconfig.h"

namespace Ui
{
    class PhpDocsConfigUI;
}

class PhpDocsConfig
    : public KCModule
{
  Q_OBJECT

  public:
    explicit PhpDocsConfig(QWidget *parent = nullptr, const QVariantList &args = QVariantList());
    virtual ~PhpDocsConfig();

    void save() override;

  private:
        Ui::PhpDocsConfigUI* m_configWidget;
};

#endif // PHPDOCS_CONFIG_H
