/***************************************************************************
                          codegenerationpolicypage.h  -  description
                             -------------------
    begin                : Tue Jul 29 2003
    copyright            : (C) 2003 by Brian Thomas
    email                : brian.thomas@gsfc.nasa.gov
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CODEGENERATIONPOLICYPAGE_H
#define CODEGENERATIONPOLICYPAGE_H

#include <qwidget.h>
#include "codegenerationpolicybase.h"

class CodeGenerationPolicy;

/**
 * @author Brian Thomas
 */

class CodeGenerationPolicyPage : public CodeGenerationPolicyBase  {
    Q_OBJECT
public:
    CodeGenerationPolicyPage (QWidget *parent=0, const char *name=0, CodeGenerationPolicy * policy = 0);

    virtual ~CodeGenerationPolicyPage();

    void setDefaults();

protected:

    CodeGenerationPolicy * m_parentPolicy;

private:

public slots:

    virtual void apply();


};

#endif

