/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "dcodegenerationpolicy.h"
// qt/kde includes
#include <kconfig.h>
// app includes
#include "dcodegenerationpolicypage.h"
#include "dcodegenerator.h"
#include "uml.h"
#include "umbrellosettings.h"
#include "optionstate.h"

/*
DCodeGenerationPolicy::DCodeGenerationPolicy(CodeGenerationPolicy *defaults)
        : CodeGenerationPolicy(defaults)
{
    init();
    setDefaults(defaults,false);
}
*/

/**
 * Constructor.
 */
DCodeGenerationPolicy::DCodeGenerationPolicy()
  //      : CodeGenerationPolicy()
{
    m_commonPolicy = UMLApp::app()->commonPolicy();
    init();
}

/**
 * Destructor.
 */
DCodeGenerationPolicy::~DCodeGenerationPolicy()
{
}

/**
 * Set the value of m_autoGenerateAttribAccessors
 * @param var the new value
 */
void DCodeGenerationPolicy::setAutoGenerateAttribAccessors( bool var )
{
    Settings::optionState().codeGenerationState.dCodeGenerationState.autoGenerateAttributeAccessors = var;
    m_commonPolicy->emitModifiedCodeContentSig();
}

/**
 * Set the value of m_autoGenerateAssocAccessors
 * @param var the new value
 */
void DCodeGenerationPolicy::setAutoGenerateAssocAccessors( bool var )
{
    Settings::optionState().codeGenerationState.dCodeGenerationState.autoGenerateAssocAccessors = var;
    m_commonPolicy->emitModifiedCodeContentSig();
}

/**
 * Get the value of m_autoGenerateAttribAccessors
 * @return the value of m_autoGenerateAttribAccessors
 */
bool DCodeGenerationPolicy::getAutoGenerateAttribAccessors()
{
    return Settings::optionState().codeGenerationState.dCodeGenerationState.autoGenerateAttributeAccessors;
}

/**
 * Get the value of m_autoGenerateAssocAccessors
 * @return the value of m_autoGenerateAssocAccessors
 */
bool DCodeGenerationPolicy::getAutoGenerateAssocAccessors()
{
    return Settings::optionState().codeGenerationState.dCodeGenerationState.autoGenerateAssocAccessors;
}

/**
 * Set the defaults for this code generator from the passed generator.
 * @param clone   the code gen policy ext object
 * @param emitUpdateSignal   flag whether to emit the update signal
 */
void DCodeGenerationPolicy::setDefaults ( CodeGenPolicyExt * clone, bool emitUpdateSignal )
{
    DCodeGenerationPolicy * jclone;
    if (!clone)
        return;

    // NOW block signals for d param setting
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    // now do d-specific stuff IF our clone is also a DCodeGenerationPolicy object
    if((jclone = dynamic_cast<DCodeGenerationPolicy*>(clone)))
    {
        setAutoGenerateAttribAccessors(jclone->getAutoGenerateAttribAccessors());
        setAutoGenerateAssocAccessors(jclone->getAutoGenerateAssocAccessors());
    }

    blockSignals(false); // "as you were citizen"

    if(emitUpdateSignal) {
        m_commonPolicy->emitModifiedCodeContentSig();
    }
}

/**
 * Set the defaults from a config file for this code generator from the passed KConfig pointer.
 * @param emitUpdateSignal   flag whether to emit the update signal
 */
void DCodeGenerationPolicy::setDefaults(bool emitUpdateSignal)
{
    // call method at the common policy to init default stuff
    m_commonPolicy->setDefaults(false);

    // NOW block signals (because call to super-class method will leave value at "true")
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    // now do d specific stuff

    setAutoGenerateAttribAccessors(UmbrelloSettings::autoGenerateAttributeAccessorsD());
    setAutoGenerateAssocAccessors(UmbrelloSettings::autoGenerateAssocAccessorsD());

    /*
    CodeGenerator *codegen = UMLApp::app()->getGenerator();
    DCodeGenerator *dcodegen = dynamic_cast<DCodeGenerator*>(codegen);
     if (dcodegen) {
        bool mkant = UmbrelloSettings::buildANTDocumentD();
        dcodegen->setCreateANTBuildFile(mkant);
    }*/

    blockSignals(false); // "as you were citizen"

    if(emitUpdateSignal) {
        m_commonPolicy->emitModifiedCodeContentSig();
    }
}


/**
 * Create a new dialog interface for this object.
 * @param parent   the parent widget
 * @param name     name of policy page
 * @return dialog object
 */
CodeGenerationPolicyPage * DCodeGenerationPolicy::createPage ( QWidget *parent, const char *name )
{
    return new DCodeGenerationPolicyPage ( parent, name, this );
}

/**
 * Initialisation routine.
 */
void DCodeGenerationPolicy::init()
{
    blockSignals( true );

    Settings::OptionState optionState = Settings::optionState();
    setAutoGenerateAttribAccessors( optionState.codeGenerationState.dCodeGenerationState.autoGenerateAttributeAccessors);
    setAutoGenerateAssocAccessors( optionState.codeGenerationState.dCodeGenerationState.autoGenerateAssocAccessors);

    blockSignals( false );
}

#include "dcodegenerationpolicy.moc"
