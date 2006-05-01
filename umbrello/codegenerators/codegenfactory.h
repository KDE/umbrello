/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                 *
 ***************************************************************************/

#ifndef CODEGENFACTORY_H
#define CODEGENFACTORY_H

#include "../umlnamespace.h"

class CodeGenerator;

/**
 * CodeGeneratorFactory lists gives access to the available code generators, their names 
 * and allows them to be created
 *
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace CodeGenFactory {

    CodeGenerator* createObject(Uml::Programming_Language pl);

}

#endif //CODEGENFACTORY_H
