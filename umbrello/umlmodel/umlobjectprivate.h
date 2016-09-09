/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLOBJECTPRIVATE_H
#define UMLOBJECTPRIVATE_H

class UMLObjectPrivate {
public:
    UMLObjectPrivate() : isSaved(false) { }
    bool isSaved;
};


#endif // UMLOBJECTPRIVATE_H
