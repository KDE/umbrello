
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*  This code generated by: 
 *      Author : thomas
 *      Date   : Wed Jun 18 2003
 */


#ifndef HIERARCHICALCODEBLOCK_H
#define HIERARCHICALCODEBLOCK_H

#include <qmap.h>
#include <qstring.h>
#include <qptrlist.h>

#include "codegenobjectwithtextblocks.h"
#include "codeblockwithcomments.h"

class CodeClassField;

class HierarchicalCodeBlock : public CodeBlockWithComments, public CodeGenObjectWithTextBlocks
{
	Q_OBJECT
public:

	// Constructors/Destructors
	//  

	/**
	 * Constructor
	 */
	HierarchicalCodeBlock ( CodeDocument * doc , QString startString = "", QString endString = "", QString comment = "");

	/**
	 * Empty Destructor
	 */
	virtual ~HierarchicalCodeBlock ( );

	// Public attributes
	//  

	// Public attribute accessor methods
	//  

	/**
	 * Set the value of m_endText
	 * @param new_var the new value of m_endText
	 */
	void setEndText ( QString new_var );

	/**
	 * Get the value of m_endText
	 * @return the value of m_endText
	 */
	QString getEndText ( );

	/**
	 * Add a TextBlock object to the m_textblockVector List
	 */
	bool addTextBlock ( TextBlock * add_object , bool replaceExisting = false);

        /**
         * Insert a new text block before/after the existing text block. Returns
         * false if it cannot insert the textblock.
         */
        bool insertTextBlock (TextBlock * newBlock, TextBlock * existingBlock, bool after = true);

	/**
	 * Remove a TextBlock object from m_textblockVector List
	 * returns boolean - true if successfull
	 */
	bool removeTextBlock ( TextBlock * remove_object );

	/**
	 * @param	text 
	 */
	void setStartText (QString text );

	/**
	 * @return	QString
	 */
	QString getStartText ( );

        /**
         * Save the XMI representation of this object
         * @return      bool    status of save
         */
        virtual bool saveToXMI ( QDomDocument & doc, QDomElement & root );

        /**
         * load params from the appropriate XMI element node.
         */
        virtual void loadFromXMI ( QDomElement & root );

	/**
	 * @return	QString
	 */
	virtual QString toString ( );

	// return a unique, and currently unallocated, text block tag for this hblock
	QString getUniqueTag(); 
	QString getUniqueTag(QString prefix); 

	/** 
	 * Utility method to add accessormethods in this object
         */
        void addCodeClassFieldMethods ( QPtrList<CodeClassField> &list );

       /**
         * create a new CodeAccesorMethod object belonging to this Hierarchical code block.
         * @return      CodeAccessorMethod
         */
        virtual CodeAccessorMethod * newCodeAccesorMethod( CodeClassField *cf, CodeAccessorMethod::AccessorType type);

       /**
         * Create a new CodeOperation object belonging to this Hierarchical code block.
         * @return      CodeOperation
         */
        virtual CodeOperation * newCodeOperation( UMLOperation *op);

	virtual CodeBlock * newCodeBlock();
	virtual CodeComment * newCodeComment();
	virtual CodeBlockWithComments * newCodeBlockWithComments();
	virtual HierarchicalCodeBlock * newHierarchicalCodeBlock();

	/** get the parent generator for this text block.. e.g. trace back
	  * up through the parent code document to find this.
	  */
	CodeGenerator * getParentGenerator();

protected:

       /** causes the text block to release all of its connections
         * and any other text blocks that it 'owns'.
         * needed to be called prior to deletion of the textblock.
         */
        virtual void release ();

        /** set attributes of the node that represents this class
         * in the XMI document.
         */
	virtual void setAttributesOnNode (QDomDocument & doc, QDomElement & elem );

        /** set the class attributes of this object from
         * the passed element node.
         */
        virtual void setAttributesFromNode ( QDomElement & element);

	/** set the class attributes from a passed object
         */
        virtual void setAttributesFromObject (TextBlock * obj);

        // this is needed by the parent codegenobjectwithtextblocks class
        // in order to search for text blocks by tag when loading from XMI
        CodeDocument * getCodeDocument();

	// look for specific text blocks which belong to code classfields
	TextBlock * findCodeClassFieldTextBlockByTag (QString tag);

private:

	QString m_startText; 
	QString m_endText; 

	QString childTextBlocksToString();
	void initAttributes ( ) ;

};

#endif // HIERARCHICALCODEBLOCK_H
