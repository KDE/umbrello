/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef OVERWRITEDIALOG_H
#define OVERWRITEDIALOG_H

#include "singlepagedialogbase.h"

class QCheckBox;

/**
 * Used by CodeGenerator::findFileName when it needs to ask
 * the user if they want to overwrite and existing file, generate a similar name
 * or cancel.  Gives an option to apply the choice to all remaining files.
 *
 * Uses OK and Apply buttons but overrides their text and behaviour, probably
 * better to use user defined buttons but I couldn't work out how.
 *
 * @author Jonathan Riddell <jr@jriddell.org>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class OverwriteDialog: public SinglePageDialogBase
{
    Q_OBJECT
public:
    OverwriteDialog(const QString& fileName, const QString& outputDirectory,
                    bool applyToAllRemaining, QWidget* parent = 0);
    ~OverwriteDialog();

    bool applyToAllRemaining();

    static const int No = SinglePageDialogBase::Apply;  // "Apply" button is repurposed for "No"

private:
    QCheckBox* m_applyToAllRemaining;
};

#endif
