
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
 *      Date   : Tue Jun 24 2003
 */



#ifndef CPPMAKECODEDOCUMENT_H
#define CPPMAKECODEDOCUMENT_H

#include "../codedocument.h"
#include <qstring.h>

class CPPCodeGenerator;

/**
  * class CPPMakefileCodeDocument
  * Represents 
  */

class CPPMakefileCodeDocument : public CodeDocument
{
	Q_OBJECT
public:

	// Constructors/Destructors
	//  

	/**
	 * Constructor
	 */
	CPPMakefileCodeDocument ( CPPCodeGenerator * gen);

	/**
	 * Empty Destructor
	 */
	virtual ~CPPMakefileCodeDocument ( );

	/**
	 * @return	QString
	 */
	QString toString ( );

	QString getPath ( );

	void updateContent();

       /**
         * Save the XMI representation of this object
         * @return      bool    status of save
         */
        virtual bool saveToXMI ( QDomDocument & doc, QDomElement & root );


protected:

       /** set attributes of the node that represents this class
         * in the XMI document.
         */
        virtual void setAttributesOnNode ( QDomDocument & doc, QDomElement & blockElement);

        /** set the class attributes of this object from
         * the passed element node.
         */
        virtual void setAttributesFromNode ( QDomElement & element);


private:



};

#endif // CPPMAKECODEDOCUMENT_H
