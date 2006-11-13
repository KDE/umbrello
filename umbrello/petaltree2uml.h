/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PETALTREE2UML_H
#define PETALTREE2UML_H

// fwd decl
class PetalNode;

/**
 * Traverse the PetalNode tree and create corresponding Umbrello objects
 * for the PetalNodes encountered.
 *
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Import_Rose {

    /**
     * This is really an auxiliary method for loadFromMDL() but is kept in a
     * separate file to reflect that it is not coupled with the parser
     * (other than by the PetalNode.)
     *
     * @return  true for success.
     */
    bool petalTree2Uml(PetalNode *root);

}

#endif

