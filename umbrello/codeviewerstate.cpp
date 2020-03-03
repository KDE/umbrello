/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "codeviewerstate.h"

#include "umbrellosettings.h"

namespace Settings {

void CodeViewerState::load()
{
    height =  UmbrelloSettings::height();
    width =  UmbrelloSettings::width();
    font =  UmbrelloSettings::codeViewerFont();
    showHiddenBlocks =  UmbrelloSettings::showHiddenBlocks();
    blocksAreHighlighted =  UmbrelloSettings::blocksAreHighlighted();
    selectedColor =  UmbrelloSettings::selectedColor();
    paperColor =  UmbrelloSettings::paperColor();
    fontColor =  UmbrelloSettings::fontColor();
    editBlockColor =  UmbrelloSettings::editBlockColor();
    umlObjectColor =  UmbrelloSettings::umlObjectColor();
    nonEditBlockColor =  UmbrelloSettings::nonEditBlockColor();
    hiddenColor =  UmbrelloSettings::hiddenColor();
}

void CodeViewerState::save()
{
    UmbrelloSettings::setHeight(height);
    UmbrelloSettings::setWidth(width);
    UmbrelloSettings::setCodeViewerFont(font);
    UmbrelloSettings::setFontColor(fontColor);
    UmbrelloSettings::setPaperColor(paperColor);
    UmbrelloSettings::setSelectedColor(selectedColor);
    UmbrelloSettings::setEditBlockColor(editBlockColor);
    UmbrelloSettings::setNonEditBlockColor(nonEditBlockColor);
    UmbrelloSettings::setUmlObjectColor(umlObjectColor);
    UmbrelloSettings::setBlocksAreHighlighted(blocksAreHighlighted);
    UmbrelloSettings::setShowHiddenBlocks(showHiddenBlocks);
    UmbrelloSettings::setHiddenColor(hiddenColor);
}

}
