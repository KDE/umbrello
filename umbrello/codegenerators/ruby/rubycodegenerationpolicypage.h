/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef RUBYCODEGENERATIONPOLICYPAGE_H
#define RUBYCODEGENERATIONPOLICYPAGE_H

#include "codegenerationpolicypage.h"
#include "ui_rubycodegenerationformbase.h"
#include "rubycodegenerationpolicy.h"

class RubyCodeGenerationPolicyPage : public CodeGenerationPolicyPage
{
    Q_OBJECT
public:

    explicit RubyCodeGenerationPolicyPage (QWidget *parent=0, const char *name=0, RubyCodeGenerationPolicy * policy = 0);

    virtual ~RubyCodeGenerationPolicyPage();

protected:

    Ui::RubyCodeGenerationFormBase form;

public slots:

    void apply();

};

#endif

