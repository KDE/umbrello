/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CODEVIEWERSTATE_H
#define CODEVIEWERSTATE_H

#include <QColor>
#include <QFont>

namespace Settings {

/// configurable params for the code viewer tool
struct CodeViewerState {
    int height;
    int width;
    bool showHiddenBlocks;
    bool blocksAreHighlighted;
    QFont font;
    QColor paperColor;
    QColor fontColor;
    QColor selectedColor;
    QColor editBlockColor;
    QColor nonEditBlockColor;
    QColor umlObjectColor;
    QColor hiddenColor;
};

}

#endif // CODEVIEWERSTATE_H
