/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
