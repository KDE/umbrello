/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2002-2020 Luis De la Parra <luis@delaparra.org>
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#ifndef CODEVIEWEROPTIONSPAGE_H
#define CODEVIEWEROPTIONSPAGE_H

#include "dialogpagebase.h"
#include "ui_codevieweroptionsbase.h"
#include "codeviewerstate.h"

/**
 * @author Brian Thomas
 */
class CodeViewerOptionsPage : public DialogPageBase, private Ui::CodeViewerOptionsBase
{
    Q_OBJECT
public:
    explicit CodeViewerOptionsPage (Settings::CodeViewerState options, QWidget   *parent = nullptr, const char *name = nullptr);
    ~CodeViewerOptionsPage();

    Settings::CodeViewerState getOptions();
    void apply();

private:
    Settings::CodeViewerState m_options;

protected Q_SLOTS:

Q_SIGNALS:
    void applyClicked();

};

#endif
