/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODEVIEWERSTATE_H
#define CODEVIEWERSTATE_H

#include <QColor>
#include <QFont>

namespace Settings {

/// configurable params for the code viewer tool
class CodeViewerState {
public:
    void load();
    void save();

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
