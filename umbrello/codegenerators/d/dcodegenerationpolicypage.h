/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DCODEGENERATIONPOLICYPAGE_H
#define DCODEGENERATIONPOLICYPAGE_H

#include "codegenerationpolicypage.h"
#include "dcodegenerationpolicy.h"
#include "ui_dcodegenerationformbase.h"

/**
 * @author Brian Thomas
 */

class DCodeGenerationFormBase : public QWidget, public Ui::DCodeGenerationFormBase
{
public:
    explicit DCodeGenerationFormBase(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
    }
};


class DCodeGenerationPolicyPage : public CodeGenerationPolicyPage
{
    Q_OBJECT
public:

    explicit DCodeGenerationPolicyPage(QWidget *parent=0, const char *name=0, DCodeGenerationPolicy * policy = 0);
    virtual ~DCodeGenerationPolicyPage();

protected:

    DCodeGenerationFormBase * form;

public slots:

    void apply();

};

#endif

