/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2011                                               *
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

/**
 * Constructor.
 */
RubyCodeGenerationPolicy::RubyCodeGenerationPolicy()
{
    m_commonPolicy = UMLApp::app()->commonPolicy();
    init();
}

/**
 * Destructor.
 */
RubyCodeGenerationPolicy::~RubyCodeGenerationPolicy()
{
}

/**
 * Set the value of m_autoGenerateAttribAccessors.
 * @param var the new value
 */
void RubyCodeGenerationPolicy::setAutoGenerateAttribAccessors(bool var)
{
    Settings::optionState().codeGenerationState.rubyCodeGenerationState.autoGenerateAttributeAccessors = var;
    m_commonPolicy->emitModifiedCodeContentSig();
}

/**
 * Set the value of m_autoGenerateAssocAccessors.
 * @param var the new value
 */
void RubyCodeGenerationPolicy::setAutoGenerateAssocAccessors(bool var)
{
    Settings::optionState().codeGenerationState.rubyCodeGenerationState.autoGenerateAssocAccessors = var;
    m_commonPolicy->emitModifiedCodeContentSig();
}

/**
 * Get the value of m_autoGenerateAttribAccessors
 * @return the value of m_autoGenerateAttribAccessors
 */
bool RubyCodeGenerationPolicy::getAutoGenerateAttribAccessors()
{
    return Settings::optionState().codeGenerationState.rubyCodeGenerationState.autoGenerateAttributeAccessors;
}

/**
 * Get the value of m_autoGenerateAssocAccessors.
 * @return the value of m_autoGenerateAssocAccessors
 */
bool RubyCodeGenerationPolicy::getAutoGenerateAssocAccessors()
{
    return Settings::optionState().codeGenerationState.rubyCodeGenerationState.autoGenerateAssocAccessors;
}

/**
 * Set the defaults for this code generator from the passed generator.
 * @param defaults           the defaults to set
 * @param emitUpdateSignal   flag whether to emit the update signal
 */
void RubyCodeGenerationPolicy::setDefaults ( CodeGenPolicyExt * defaults, bool emitUpdateSignal )
{
    RubyCodeGenerationPolicy * rclone;
    if (!defaults)
        return;

    // NOW block signals for ruby param setting
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    // now do ruby-specific stuff IF our clone is also a RubyCodeGenerationPolicy object
    if((rclone = dynamic_cast<RubyCodeGenerationPolicy*>(defaults)))
    {
        setAutoGenerateAttribAccessors(rclone->getAutoGenerateAttribAccessors());
        setAutoGenerateAssocAccessors(rclone->getAutoGenerateAssocAccessors());
    }

    blockSignals(false); // "as you were citizen"

    if(emitUpdateSignal)
        m_commonPolicy->emitModifiedCodeContentSig();
}

/**
 * Set the defaults from a config file for this code generator from the passed KConfig pointer.
 * @param emitUpdateSignal   flag whether to emit the update signal
 */
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
 * @param parent   the parent widget
 * @param name     the name of the page
 * @return dialog object
 */
CodeGenerationPolicyPage * RubyCodeGenerationPolicy::createPage ( QWidget *parent, const char *name )
{
    return new RubyCodeGenerationPolicyPage ( parent, name, this );
}

/**
 * Initialisation.
 */
void RubyCodeGenerationPolicy::init()
{
    blockSignals(true);

    Settings::OptionState optionState = Settings::optionState();
    setAutoGenerateAttribAccessors( optionState.codeGenerationState.rubyCodeGenerationState.autoGenerateAttributeAccessors);
    setAutoGenerateAssocAccessors( optionState.codeGenerationState.rubyCodeGenerationState.autoGenerateAssocAccessors);

    blockSignals(false);
}

#include "rubycodegenerationpolicy.moc"
