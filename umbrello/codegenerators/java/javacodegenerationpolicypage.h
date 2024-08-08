/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVACODEGENERATIONPOLICYPAGE_H
#define JAVACODEGENERATIONPOLICYPAGE_H

#include "codegenerationpolicypage.h"
#include "ui_javacodegenerationformbase.h"
#include "javacodegenerationpolicy.h"

/**
 * @author Brian Thomas
 */

class JavaCodeGenerationFormBase : public QWidget, public Ui::JavaCodeGenerationFormBase
{
public:
    explicit JavaCodeGenerationFormBase(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
    }
};


class JavaCodeGenerationPolicyPage : public CodeGenerationPolicyPage
{
    Q_OBJECT
public:

    explicit JavaCodeGenerationPolicyPage (QWidget    *parent= nullptr, const char *name= nullptr, JavaCodeGenerationPolicy * policy = nullptr);

    virtual ~JavaCodeGenerationPolicyPage();

protected:

    JavaCodeGenerationFormBase * form;

public slots:

    void apply();

};

#endif

