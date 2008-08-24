/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005      Richard Dale  <>                              *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "rubycodegenerationpolicy.h"
// qt/kde includes
#include <kconfig.h>
// app includes
#include "optionstate.h"
#include "rubycodegenerationpolicypage.h"
#include "rubycodegenerator.h"
#include "uml.h"
#include "umbrellosettings.h"

RubyCodeGenerationPolicy::RubyCodeGenerationPolicy()
{
    m_commonPolicy = UMLApp::app()->getCommonPolicy();
    init();
}

RubyCodeGenerationPolicy::~RubyCodeGenerationPolicy ( )
{
}

/**
 * Set the value of autoGenerateAttribAccessors
 * @param new_var the new value
 */
void RubyCodeGenerationPolicy::setAutoGenerateAttribAccessors( bool var )
{
    Settings::getOptionState().codeGenerationState.rubyCodeGenerationState.autoGenerateAttributeAccessors = var;
    m_commonPolicy->emitModifiedCodeContentSig();
}

/**
 * Set the value of autoGenerateAssocAccessors
 * @param new_var the new value
 */
void RubyCodeGenerationPolicy::setAutoGenerateAssocAccessors( bool var )
{
    Settings::getOptionState().codeGenerationState.rubyCodeGenerationState.autoGenerateAssocAccessors = var;
    m_commonPolicy->emitModifiedCodeContentSig();
}

/**
 * Get the value of autoGenerateAttribAccessors
 * @return the value of autoGenerateAttribAccessors
 */
bool RubyCodeGenerationPolicy::getAutoGenerateAttribAccessors( )
{
    return Settings::getOptionState().codeGenerationState.rubyCodeGenerationState.autoGenerateAttributeAccessors;
}

/**
 * Get the value of autoGenerateAssocAccessors
 * @return the value of autoGenerateAssocAccessors
 */
bool RubyCodeGenerationPolicy::getAutoGenerateAssocAccessors( )
{
    return Settings::getOptionState().codeGenerationState.rubyCodeGenerationState.autoGenerateAssocAccessors;
}

void RubyCodeGenerationPolicy::setDefaults ( CodeGenPolicyExt * clone, bool emitUpdateSignal )
{
    RubyCodeGenerationPolicy * rclone;
    if (!clone)
        return;

    // NOW block signals for ruby param setting
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    // now do ruby-specific stuff IF our clone is also a RubyCodeGenerationPolicy object
    if((rclone = dynamic_cast<RubyCodeGenerationPolicy*>(clone)))
    {
        setAutoGenerateAttribAccessors(rclone->getAutoGenerateAttribAccessors());
        setAutoGenerateAssocAccessors(rclone->getAutoGenerateAssocAccessors());
    }

    blockSignals(false); // "as you were citizen"

    if(emitUpdateSignal)
        m_commonPolicy->emitModifiedCodeContentSig();
}

void RubyCodeGenerationPolicy::setDefaults( bool emitUpdateSignal )
{
    // call the superclass to init default stuff
    m_commonPolicy->setDefaults(false);

    // NOW block signals (because call to super-class method will leave value at "true")
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    // now do ruby specific stuff

    setAutoGenerateAttribAccessors(UmbrelloSettings::autoGenerateAttributeAccessorsRuby());
    setAutoGenerateAssocAccessors(UmbrelloSettings::autoGenerateAssocAccessorsRuby());

    blockSignals(false); // "as you were citizen"

    if(emitUpdateSignal)
        m_commonPolicy->emitModifiedCodeContentSig();
}


/**
 * Create a new dialog interface for this object.
 * @return dialog object
 */
CodeGenerationPolicyPage * RubyCodeGenerationPolicy::createPage ( QWidget *parent, const char *name )
{
    return new RubyCodeGenerationPolicyPage ( parent, name, this );
}


void RubyCodeGenerationPolicy::init()
{
    blockSignals(true);

    Settings::OptionState optionState = Settings::getOptionState();
    setAutoGenerateAttribAccessors( optionState.codeGenerationState.rubyCodeGenerationState.autoGenerateAttributeAccessors);
    setAutoGenerateAssocAccessors( optionState.codeGenerationState.rubyCodeGenerationState.autoGenerateAssocAccessors);

    blockSignals(false);
}

#include "rubycodegenerationpolicy.moc"
