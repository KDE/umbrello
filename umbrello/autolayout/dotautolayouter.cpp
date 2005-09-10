/*
 *  copyright (C) 2005
 *  Umbrello UML Modeller Authors <uml-devel @uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "dotautolayouter.h"

#include <graphviz/dotprocs.h>

namespace Autolayout {

DotAutolayouter::DotAutolayouter()
        : Autolayout::GraphvizAutolayouter()
{
}


DotAutolayouter::~DotAutolayouter()
{
    dot_cleanup(gg->_agraph);
}


void DotAutolayouter::run()
{
    //#ifndef internal_renderizer
    /* bind graph to GV context - currently must be done before layout */
    //  gvBindContext(gvc,g);
    //#endif
    // do layout
    //dot_layout();
    dot_layout( gg->_agraph );
}

}
