
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "dcodegenerationpolicy.h"
// qt/kde includes
#include <kconfig.h>
// app includes
#include "dcodegenerationpolicypage.h"
#include "dcodegenerator.h"
#include "../uml.h"

const bool DCodeGenerationPolicy::DEFAULT_AUTO_GEN_ATTRIB_ACCESSORS = true;
const bool DCodeGenerationPolicy::DEFAULT_AUTO_GEN_ASSOC_ACCESSORS = true;

// Constructors/Destructors
/*

DCodeGenerationPolicy::DCodeGenerationPolicy(CodeGenerationPolicy *defaults)
        : CodeGenerationPolicy(defaults)
{
    init();
    setDefaults(defaults,false);
}
 */

DCodeGenerationPolicy::DCodeGenerationPolicy(KConfig *config)
  //      : CodeGenerationPolicy(config)
{
    init();
    setDefaults(config,false);
}

DCodeGenerationPolicy::~DCodeGenerationPolicy ( ) { }

//
// Methods
//

// Accessor methods
//

// Public attribute accessor methods
//

/**
 * Set the value of m_autoGenerateAttribAccessors
 * @param new_var the new value
 */
void DCodeGenerationPolicy::setAutoGenerateAttribAccessors( bool var ) {
    m_autoGenerateAttribAccessors = var;
    m_commonPolicy->emitModifiedCodeContentSig();
}

/**
 * Set the value of m_autoGenerateAssocAccessors
 * @param new_var the new value
 */
void DCodeGenerationPolicy::setAutoGenerateAssocAccessors( bool var ) {
    m_autoGenerateAssocAccessors = var;
    m_commonPolicy->emitModifiedCodeContentSig();
}

/**
 * Get the value of m_autoGenerateAttribAccessors
 * @return the value of m_autoGenerateAttribAccessors
 */
bool DCodeGenerationPolicy::getAutoGenerateAttribAccessors( ){
    return m_autoGenerateAttribAccessors;
}

/**
 * Get the value of m_autoGenerateAssocAccessors
 * @return the value of m_autoGenerateAssocAccessors
 */
bool DCodeGenerationPolicy::getAutoGenerateAssocAccessors( ){
    return m_autoGenerateAssocAccessors;
}

// Other methods
//

void DCodeGenerationPolicy::writeConfig ( KConfig * config )
{

    // write ONLY the D specific stuff
    KConfigGroup cg( config, "D Code Generation" );

    cg.writeEntry("autoGenAccessors",getAutoGenerateAttribAccessors());
    cg.writeEntry("autoGenAssocAccessors",getAutoGenerateAssocAccessors());

    CodeGenerator *codegen = UMLApp::app()->getGenerator();
    DCodeGenerator *dcodegen = dynamic_cast<DCodeGenerator*>(codegen);
//    if (dcodegen)
//        cg.writeEntry("buildANTDocument", dcodegen->getCreateANTBuildFile());

}

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

    if(emitUpdateSignal)
        m_commonPolicy->emitModifiedCodeContentSig();

}

void DCodeGenerationPolicy::setDefaults( KConfig * config, bool emitUpdateSignal )
{

    if(!config)
        return;

    // call method at the common policy to init default stuff
    m_commonPolicy->setDefaults(config, false);

    // NOW block signals (because call to super-class method will leave value at "true")
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    // now do d specific stuff
    KConfigGroup cg( config, "D Code Generation");

    setAutoGenerateAttribAccessors(cg.readEntry("autoGenAccessors",DEFAULT_AUTO_GEN_ATTRIB_ACCESSORS));
    setAutoGenerateAssocAccessors(cg.readEntry("autoGenAssocAccessors",DEFAULT_AUTO_GEN_ASSOC_ACCESSORS));

    CodeGenerator *codegen = UMLApp::app()->getGenerator();
    DCodeGenerator *dcodegen = dynamic_cast<DCodeGenerator*>(codegen);
/*    if (dcodegen) {
        bool mkant = cg.readEntry("buildANTDocument", DCodeGenerator::DEFAULT_BUILD_ANT_DOC);
        dcodegen->setCreateANTBuildFile(mkant);
    }*/

    blockSignals(false); // "as you were citizen"

    if(emitUpdateSignal)
        m_commonPolicy->emitModifiedCodeContentSig();
}


/**
 * Create a new dialog interface for this object.
 * @return dialog object
 */
CodeGenerationPolicyPage * DCodeGenerationPolicy::createPage ( QWidget *parent, const char *name ) {
    return new DCodeGenerationPolicyPage ( parent, name, this );
}

void DCodeGenerationPolicy::init() {
    m_commonPolicy = UMLApp::app()->getCommonPolicy();
    m_autoGenerateAttribAccessors = DEFAULT_AUTO_GEN_ATTRIB_ACCESSORS;
    m_autoGenerateAssocAccessors = DEFAULT_AUTO_GEN_ASSOC_ACCESSORS;
}


#include "dcodegenerationpolicy.moc"
