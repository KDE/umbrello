/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                 *
 ***************************************************************************/

#ifndef NCGOF_H
#define NCGOF_H

/**
 * New Code Generator Object Factory
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

// fwd decls
class ClassifierCodeDocument;
class CodeOperation;
class UMLOperation;

namespace NCGOF {

    CodeOperation *newCodeOperation(ClassifierCodeDocument *ccd, UMLOperation * op);

}  // end namespace NCGOF

#endif

