/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

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
    JavaCodeGenerationFormBase(QWidget *parent ) : QWidget( parent )
    {
        setupUi( this );
    }
};


class JavaCodeGenerationPolicyPage : public CodeGenerationPolicyPage
{
    Q_OBJECT
public:

    explicit JavaCodeGenerationPolicyPage (QWidget *parent=0, const char *name=0, JavaCodeGenerationPolicy * policy = 0);

    virtual ~JavaCodeGenerationPolicyPage();

protected:

    JavaCodeGenerationFormBase * form;

public slots:

    void apply();

};

#endif

