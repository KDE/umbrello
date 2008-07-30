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

#ifndef RUBYCODEGENERATIONPOLICY_H
#define RUBYCODEGENERATIONPOLICY_H

#include "codegenpolicyext.h"
#include "codegenerationpolicy.h"

#include <QtCore/QString>

class KConfig;
class CodeGenerationPolicyPage;

class RubyCodeGenerationPolicy : public CodeGenPolicyExt
{
    Q_OBJECT
public:

    /**
     * Constructors
     */
    RubyCodeGenerationPolicy ();

    /**
     * Empty Destructor
     */
    virtual ~RubyCodeGenerationPolicy ( );

    /**
     * Set the value of m_autoGenerateAttribAccessors
     * @param var the new value
     */
    void setAutoGenerateAttribAccessors ( bool var );

    /**
     * Get the value of m_autoGenerateAttribAccessors
     * @return value the boolean value of m_autoGenerateAttribAccessors
     */
    bool getAutoGenerateAttribAccessors( );

    /**
     * Set the value of m_autoGenerateAssocAccessors
     * @param var the new value
     */
    void setAutoGenerateAssocAccessors ( bool var );

    /**
     * Get the value of m_autoGenerateAssocAccessors
     * @return value the boolean value of m_autoGenerateAssocAccessors
     */
    bool getAutoGenerateAssocAccessors( );

    /**
     * set the defaults for this code generator from the passed generator.
     */
    virtual void setDefaults (CodeGenPolicyExt * defaults, bool emitUpdateSignal = true);

    /**
     * set the defaults from a config file for this code generator from the passed KConfig pointer.
     */
    virtual void setDefaults(bool emitUpdateSignal = true);

    /**
     * Create a new dialog interface for this object.
     * @return dialog object
     */
    CodeGenerationPolicyPage * createPage ( QWidget *parent = 0, const char * name = 0);

private:

    CodeGenerationPolicy *m_commonPolicy;

    void init();
};

#endif // RUBYCODEGENERATIONPOLICY_H
