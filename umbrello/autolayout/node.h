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

#ifndef AUTOLAYOUTNODE_H
#define AUTOLAYOUTNODE_H

namespace Autolayout {

/**
@author Umbrello UML Modeller Authors
*/
class Node{

    //Node(Agnode_t* n);


    //
public:
    virtual ~Node() {}
    virtual int getX()=0;
    virtual int getY()=0;


};
}
#endif
