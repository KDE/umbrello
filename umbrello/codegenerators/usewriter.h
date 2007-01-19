/***************************************************************************
                          usewriter.h  -  description
                             -------------------
    begin                : Fri Jan 20 2007
    copyright            : (C) 2007 by Thibault <zenithar> Normand
    email                : thibault.normand@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _USEWRITER_H_
#define _USEWRITER_H_

#include "simplecodegenerator.h"
#include "../umloperationlist.h"

/**
  * class USEWriter is a Use code generator for UMLClassifier objects
  * Just call writeClass and feed it a UMLClassifier;
  */
class USEWriter : public SimpleCodeGenerator {
    Q_OBJECT
public:

    USEWriter();
    virtual ~USEWriter();

    /**
      * call this method to generate Actionscript code for a UMLClassifier
      * @param c the class you want to generate code for.
      */
    virtual void writeClass(UMLClassifier *c);

    /**
     * returns "JavaScript"
     */
    virtual Uml::Programming_Language getLanguage();

    /**
     * get list of reserved keywords
     */
    virtual const QStringList reservedKeywords() const;
};

#endif //JSWRITER