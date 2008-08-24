/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "javacodegenerationpolicy.h"

// app includes
#include "javacodegenerationpolicypage.h"
#include "javacodegenerator.h"
#include "optionstate.h"
#include "uml.h"
#include "umbrellosettings.h"

// kde includes
#include <kconfig.h>

/*
JavaCodeGenerationPolicy::JavaCodeGenerationPolicy(CodeGenerationPolicy *defaults)
        : CodeGenerationPolicy(defaults)
{
    init();
    setDefaults(defaults,false);
}
 */

JavaCodeGenerationPolicy::JavaCodeGenerationPolicy()
  //      : CodeGenerationPolicy()
{
    m_commonPolicy = UMLApp::app()->getCommonPolicy();
    init();
}

JavaCodeGenerationPolicy::~JavaCodeGenerationPolicy ( )
{
}

/**
 * Set the value of autoGenerateAttribAccessors
 * @param new_var the new value
 */
void JavaCodeGenerationPolicy::setAutoGenerateAttribAccessors( bool var )
{
    Settings::getOptionState().codeGenerationState.javaCodeGenerationState.autoGenerateAttributeAccessors = var;
    m_commonPolicy->emitModifiedCodeContentSig();
}

/**
 * Set the value of m_autoGenerateAssocAccessors
 * @param new_var the new value
 */
void JavaCodeGenerationPolicy::setAutoGenerateAssocAccessors( bool var )
{
    Settings::getOptionState().codeGenerationState.javaCodeGenerationState.autoGenerateAssocAccessors = var;
    m_commonPolicy->emitModifiedCodeContentSig();
}

/**
 * Get the value of m_autoGenerateAttribAccessors
 * @return the value of m_autoGenerateAttribAccessors
 */
bool JavaCodeGenerationPolicy::getAutoGenerateAttribAccessors( )
{
    return Settings::getOptionState().codeGenerationState.javaCodeGenerationState.autoGenerateAttributeAccessors;
}

/**
 * Get the value of m_autoGenerateAssocAccessors
 * @return the value of m_autoGenerateAssocAccessors
 */
bool JavaCodeGenerationPolicy::getAutoGenerateAssocAccessors( )
{
    return Settings::getOptionState().codeGenerationState.javaCodeGenerationState.autoGenerateAssocAccessors;
}

void JavaCodeGenerationPolicy::setDefaults ( CodeGenPolicyExt * clone, bool emitUpdateSignal )
{
    JavaCodeGenerationPolicy * jclone;
    if (!clone)
        return;

    // NOW block signals for java param setting
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    // now do java-specific stuff IF our clone is also a JavaCodeGenerationPolicy object
    if((jclone = dynamic_cast<JavaCodeGenerationPolicy*>(clone)))
    {
        setAutoGenerateAttribAccessors(jclone->getAutoGenerateAttribAccessors());
        setAutoGenerateAssocAccessors(jclone->getAutoGenerateAssocAccessors());
    }

    blockSignals(false); // "as you were citizen"

    if(emitUpdateSignal)
        m_commonPolicy->emitModifiedCodeContentSig();
}

void JavaCodeGenerationPolicy::setDefaults( bool emitUpdateSignal )
{
    // call method at the common policy to init default stuff
    m_commonPolicy->setDefaults(false);

    // NOW block signals (because call to super-class method will leave value at "true")
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    setAutoGenerateAttribAccessors(UmbrelloSettings::autoGenerateAttributeAccessorsJava());
    setAutoGenerateAssocAccessors(UmbrelloSettings::autoGenerateAssocAccessorsJava());

    CodeGenerator *codegen = UMLApp::app()->getGenerator();
    JavaCodeGenerator *javacodegen = dynamic_cast<JavaCodeGenerator*>(codegen);
    if (javacodegen) {
        bool mkant = UmbrelloSettings::buildANTDocumentJava();
        javacodegen->setCreateANTBuildFile(mkant);
    }

    blockSignals(false); // "as you were citizen"

    if(emitUpdateSignal)
        m_commonPolicy->emitModifiedCodeContentSig();
}

/**
 * Create a new dialog interface for this object.
 * @return dialog object
 */
CodeGenerationPolicyPage * JavaCodeGenerationPolicy::createPage ( QWidget *parent, const char *name )
{
    return new JavaCodeGenerationPolicyPage ( parent, name, this );
}

void JavaCodeGenerationPolicy::init()
{
    blockSignals( true );

    Settings::OptionState optionState = Settings::getOptionState();
    setAutoGenerateAttribAccessors(optionState.codeGenerationState.javaCodeGenerationState.autoGenerateAttributeAccessors);
    setAutoGenerateAssocAccessors(optionState.codeGenerationState.javaCodeGenerationState.autoGenerateAssocAccessors);

    blockSignals( false );
}

#include "javacodegenerationpolicy.moc"
