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

#ifndef __DIAGRAM_INTERFACE_H
#define __DIAGRAM_INTERFACE_H

class DiagramInterface
{
public:
    DiagramInterface() {}
    virtual ~DiagramInterface() {}


private:
    DiagramInterface( const DiagramInterface& source );
    void operator = ( const DiagramInterface& source );
};


#endif // __DIAGRAM_INTERFACE_H
