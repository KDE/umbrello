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

#ifndef RUBYCODEGENERATIONPOLICY_H
#define RUBYCODEGENERATIONPOLICY_H

#include <qstring.h>
#include "../codegenerationpolicy.h"

class KConfig;
class CodeGenerationPolicyPage;

class RubyCodeGenerationPolicy : public CodeGenerationPolicy
{
    Q_OBJECT
public:

    enum ScopePolicy { Public=200, Private, Protected, FromParent };
    enum RubyCommentStyle { BeginEnd=0, Hash };

    static const ScopePolicy DEFAULT_ATTRIB_ACCESSOR_SCOPE;
    static const ScopePolicy DEFAULT_ASSOC_FIELD_SCOPE;
    static const RubyCommentStyle DEFAULT_COMMENT;
    static const bool DEFAULT_AUTO_GEN_EMPTY_CONSTRUCTORS;
    static const bool DEFAULT_AUTO_GEN_ATTRIB_ACCESSORS;
    static const bool DEFAULT_AUTO_GEN_ASSOC_ACCESSORS;

    // Constructors/Destructors
    //

    /**
     * Constructors 
     */
    RubyCodeGenerationPolicy (CodeGenerationPolicy * defaults = 0);
    RubyCodeGenerationPolicy (KConfig * config = 0);

    /**
     * Empty Destructor
     */
    virtual ~RubyCodeGenerationPolicy ( );

    // Public attributes
    //


    // Public attribute accessor methods
    //

    /** Get the default scope for new attribute accessor methods.
     */
    ScopePolicy getAttributeAccessorScope ();

    /** Set the default scope for new attribute accessor methods.
    */
    void setAttributeAccessorScope (ScopePolicy scope);

    /** Get the default scope for new association class fields.
      */
    ScopePolicy getAssociationFieldScope();

    /** Set the default scope for new association class fields.
    */
    void setAssociationFieldScope (ScopePolicy scope);


    /**
      * Set the value of m_autoGenerateAttribAccessors
      * @param new_var the new value
      */
    void setAutoGenerateAttribAccessors ( bool var );

    /**
      * Get the value of m_autoGenerateAttribAccessors
      * @return value the boolean value of m_autoGenerateAttribAccessors
      */
    bool getAutoGenerateAttribAccessors( );

    /**
      * Set the value of m_autoGenerateAssocAccessors
      * @param new_var the new value
      */
    void setAutoGenerateAssocAccessors ( bool var );

    /**
      * Get the value of m_autoGenerateAssocAccessors
      * @return value the boolean value of m_autoGenerateAssocAccessors
      */
    bool getAutoGenerateAssocAccessors( );

    /**
            * Set the value of m_autoGenerateConstructors
            * @param new_var the new value
            */
    void setAutoGenerateConstructors ( bool var );

    /**
      * Get the value of m_autoGenerateConstructors
      * @return value the boolean value of m_autoGenerateConstructors
      */
    bool getAutoGenerateConstructors ( );

    /**
     * Set the value of m_commentStyle
     * @param new_var the new value of m_commentStyle
     */
    void setCommentStyle ( RubyCommentStyle new_var );

    /**
     * Get the value of m_commentStyle
     * @return the value of m_commentStyle
     */
    RubyCommentStyle getCommentStyle ( );

    /**
     * set the defaults for this code generator from the passed generator.
     */
    virtual void setDefaults (CodeGenerationPolicy * defaults, bool emitUpdateSignal = true);

    /**
     * set the defaults from a config file for this code generator from the passed KConfig pointer.
     */
    virtual void setDefaults(KConfig * config, bool emitUpdateSignal = true);

    /**
            * write Default params to passed KConfig pointer.
            */
    virtual void writeConfig (KConfig * config);

    /**
            * Create a new dialog interface for this object.
            * @return dialog object
            */
    CodeGenerationPolicyPage * createPage ( QWidget *parent = 0, const char * name = 0);

protected:

    /**
     */
    void init ( );

private:

    ScopePolicy m_defaultAttributeAccessorScope;
    ScopePolicy m_defaultAssociationFieldScope;
    RubyCommentStyle m_commentStyle;
    bool m_autoGenerateConstructors;
    bool m_autoGenerateAttribAccessors;
    bool m_autoGenerateAssocAccessors;

};

#endif // RUBYCODEGENERATIONPOLICY_H
