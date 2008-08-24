/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

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
    DCodeGenerationFormBase(QWidget *parent ) : QWidget( parent )
    {
        setupUi( this );
    }
};


class DCodeGenerationPolicyPage : public CodeGenerationPolicyPage
{
    Q_OBJECT
public:

    explicit DCodeGenerationPolicyPage (QWidget *parent=0, const char *name=0, DCodeGenerationPolicy * policy = 0);

    virtual ~DCodeGenerationPolicyPage();

protected:

    DCodeGenerationFormBase * form;

public slots:

    void apply();

};

#endif

