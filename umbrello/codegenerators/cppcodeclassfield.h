
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
 *      Date   : Mon Sep 1 2003
 */



#ifndef CPPCODECLASSFIELD_H
#define CPPCODECLASSFIELD_H

#include <qstring.h>

#include "../codeclassfield.h"
 
class ClassifierCodeDocument;

class CPPCodeClassField : public CodeClassField
{
	Q_OBJECT
public:

	// Constructors/Destructors
	//  

	/**
	 * Constructors 
	 */
	CPPCodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role);
	CPPCodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib);

	/**
	 * Empty Destructor
	 */
	virtual ~CPPCodeClassField ( );

	QString fixTypeName(QString string); 
	QString getFieldType();
	QString getFieldName();
	QString getInitialValue();
	/** get the name of the class which holds lists, e.g. "QPtrlist" or
          * "Vector" or "List" and so on.
          */
        QString getListFieldClassName();


protected:

private:

};

#endif // CPPCODECLASSFIELD_H
