/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef OVERWRITEDIALOG_H
#define OVERWRITEDIALOG_H

#include <kdialog.h>

class QCheckBox;

/**
 * Used by CodeGenerator::findFileName when it needs to ask
 * the user if they want to overwrite and existing file, generate a similar name
 * or cancel.  Gives an option to apply the choice to all remaining files.
 *
 * Uses OK and Apply buttons but overrides their text and behaviour, probably
 * better to use user defined buttons but I couldn't work out how.  KDialogBase guru needed.
 *
 * @author Jonathan Riddell <jr@jriddell.org>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class OverwriteDialog: public KDialog
{
    Q_OBJECT

public:

    OverwriteDialog(const QString& fileName, const QString& outputDirectory,
                      bool applyToAllRemaining, QWidget* parent=0);
    ~OverwriteDialog();

    bool applyToAllRemaining();

protected slots:

    virtual void slotOk();

    virtual void slotApply();


    virtual void slotCancel();

private:
    QCheckBox* m_applyToAllRemaining;
};

#endif
