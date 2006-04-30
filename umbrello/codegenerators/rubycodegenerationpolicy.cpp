/***************************************************************************
                          rubycodegenerationpolicy.cpp
                          Derived from the Java code generator by thomas

    begin                : Thur Jul 21 2005
    author               : Richard Dale
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "rubycodegenerationpolicy.h"
// qt/kde includes
#include <kconfig.h>
// app includes
#include "rubycodegenerationpolicypage.h"
#include "rubycodegenerator.h"
#include "../uml.h"

const bool RubyCodeGenerationPolicy::DEFAULT_AUTO_GEN_ATTRIB_ACCESSORS = true;
const bool RubyCodeGenerationPolicy::DEFAULT_AUTO_GEN_ASSOC_ACCESSORS = true;

// Constructors/Destructors
//

RubyCodeGenerationPolicy::RubyCodeGenerationPolicy(CodeGenerationPolicy *defaults)
        : CodeGenerationPolicy(defaults)
{
    init();
    setDefaults(defaults,false);
}

RubyCodeGenerationPolicy::RubyCodeGenerationPolicy(KConfig *config)
        : CodeGenerationPolicy(config)
{
    init();
    setDefaults(config,false);
}

RubyCodeGenerationPolicy::~RubyCodeGenerationPolicy ( ) { }

//
// Methods
//

// Accessor methods
//

// Public attribute accessor methods
//

/** Get the default scope for new accessor methods.
 */
RubyCodeGenerationPolicy::ScopePolicy RubyCodeGenerationPolicy::getAttributeAccessorScope () {
    return m_defaultAttributeAccessorScope;
}

/** Set the default scope for new accessor methods.
 */
void RubyCodeGenerationPolicy::setAttributeAccessorScope (ScopePolicy scope) {
    m_defaultAttributeAccessorScope = scope;
    emit modifiedCodeContent();
}

/** Get the default scope for new accessor methods.
 */
RubyCodeGenerationPolicy::ScopePolicy RubyCodeGenerationPolicy::getAssociationFieldScope() {
    return m_defaultAssociationFieldScope;
}

/** Set the default scope for new accessor methods.
 */
void RubyCodeGenerationPolicy::setAssociationFieldScope (ScopePolicy scope) {
    m_defaultAssociationFieldScope = scope;
    emit modifiedCodeContent();
}

/**
 * Set the value of m_autoGenerateAttribAccessors
 * @param new_var the new value
 */
void RubyCodeGenerationPolicy::setAutoGenerateAttribAccessors( bool var ) {
    m_autoGenerateAttribAccessors = var;
    emit modifiedCodeContent();
}

/**
 * Set the value of m_autoGenerateAssocAccessors
 * @param new_var the new value
 */
void RubyCodeGenerationPolicy::setAutoGenerateAssocAccessors( bool var ) {
    m_autoGenerateAssocAccessors = var;
    emit modifiedCodeContent();
}

/**
 * Get the value of m_autoGenerateAttribAccessors
 * @return the value of m_autoGenerateAttribAccessors
 */
bool RubyCodeGenerationPolicy::getAutoGenerateAttribAccessors( ){
    return m_autoGenerateAttribAccessors;
}

/**
 * Get the value of m_autoGenerateAssocAccessors
 * @return the value of m_autoGenerateAssocAccessors
 */
bool RubyCodeGenerationPolicy::getAutoGenerateAssocAccessors( ){
    return m_autoGenerateAssocAccessors;
}

// Other methods
//

void RubyCodeGenerationPolicy::writeConfig ( KConfig * config )
{

    // write ONLY the Ruby specific stuff
    config->setGroup("Ruby Code Generation");

    config->writeEntry("defaultAttributeAccessorScope",getAttributeAccessorScope());
    config->writeEntry("defaultAssocFieldScope",getAssociationFieldScope());
    config->writeEntry("commentStyle",getCommentStyle());
    config->writeEntry("autoGenEmptyConstructors",getAutoGenerateConstructors());
    config->writeEntry("autoGenAccessors",getAutoGenerateAttribAccessors());
    config->writeEntry("autoGenAssocAccessors",getAutoGenerateAssocAccessors());
}

void RubyCodeGenerationPolicy::setDefaults ( CodeGenerationPolicy * clone, bool emitUpdateSignal )
{

    RubyCodeGenerationPolicy * rclone;
    if (!clone)
        return;

    // do the super class
    CodeGenerationPolicy::setDefaults(clone, false);

    // NOW block signals for ruby param setting
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).


    // now do ruby-specific stuff IF our clone is also a RubyCodeGenerationPolicy object
    if((rclone = dynamic_cast<RubyCodeGenerationPolicy*>(clone)))
    {
        setAttributeAccessorScope(rclone->getAttributeAccessorScope());
        setAssociationFieldScope(rclone->getAssociationFieldScope());
        setCommentStyle(rclone->getCommentStyle());
        setAutoGenerateConstructors(rclone->getAutoGenerateConstructors());
        setAutoGenerateAttribAccessors(rclone->getAutoGenerateAttribAccessors());
        setAutoGenerateAssocAccessors(rclone->getAutoGenerateAssocAccessors());
    }

    blockSignals(false); // "as you were citizen"

    if(emitUpdateSignal)
        emit modifiedCodeContent();

}

void RubyCodeGenerationPolicy::setDefaults( KConfig * config, bool emitUpdateSignal )
{

    if(!config)
        return;

    // call the superclass to init default stuff
    CodeGenerationPolicy::setDefaults(config, false);

    // NOW block signals (because call to super-class method will leave value at "true")
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    // now do ruby specific stuff
    config -> setGroup("Ruby Code Generation");

    setAttributeAccessorScope((ScopePolicy)config->readNumEntry("defaultAttributeAccessorScope",defaultAttribAccessorScope()));
    setAssociationFieldScope((ScopePolicy)config->readNumEntry("defaultAssocFieldScope",defaultAssocFieldScope()));
    setCommentStyle((CommentStyle)config->readNumEntry("commentStyle",defaultCommentStyle()));
    setAutoGenerateConstructors(config->readBoolEntry("autoGenEmptyConstructors",defaultAutoGenerateConstructors()));
    setAutoGenerateAttribAccessors(config->readBoolEntry("autoGenAccessors",DEFAULT_AUTO_GEN_ATTRIB_ACCESSORS));
    setAutoGenerateAssocAccessors(config->readBoolEntry("autoGenAssocAccessors",DEFAULT_AUTO_GEN_ASSOC_ACCESSORS));

    blockSignals(false); // "as you were citizen"

    if(emitUpdateSignal)
        emit modifiedCodeContent();
}


/**
 * Create a new dialog interface for this object.
 * @return dialog object
 */
CodeGenerationPolicyPage * RubyCodeGenerationPolicy::createPage ( QWidget *parent, const char *name ) {
    return new RubyCodeGenerationPolicyPage ( parent, name, this );
}

void RubyCodeGenerationPolicy::init() {
    CodeGenerationPolicy::initFields();
    m_autoGenerateConstructors = defaultAutoGenerateConstructors();
    m_autoGenerateAttribAccessors = DEFAULT_AUTO_GEN_ATTRIB_ACCESSORS;
    m_autoGenerateAssocAccessors = DEFAULT_AUTO_GEN_ASSOC_ACCESSORS;
}


#include "rubycodegenerationpolicy.moc"
