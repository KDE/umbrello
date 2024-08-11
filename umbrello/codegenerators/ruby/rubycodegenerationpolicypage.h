/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef RUBYCODEGENERATIONPOLICYPAGE_H
#define RUBYCODEGENERATIONPOLICYPAGE_H

#include "codegenerationpolicypage.h"
#include "ui_rubycodegenerationformbase.h"
#include "rubycodegenerationpolicy.h"

class RubyCodeGenerationPolicyPage : public CodeGenerationPolicyPage
{
    Q_OBJECT
public:

    explicit RubyCodeGenerationPolicyPage (QWidget    *parent= nullptr, const char *name= nullptr, RubyCodeGenerationPolicy *policy = nullptr);

    virtual ~RubyCodeGenerationPolicyPage();

protected:

    Ui::RubyCodeGenerationFormBase form;

public Q_SLOTS:

    void apply();

};

#endif

