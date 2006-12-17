/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef OVERWRITEDIALOGUE_H
#define OVERWRITEDIALOGUE_H

#include <kdialogbase.h>

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
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class OverwriteDialogue: public KDialogBase {
    Q_OBJECT

public:
    /**
     * constructor sets up the dialog, adding checkbox and label
     */
    OverwriteDialogue(const QString& fileName, const QString& outputDirectory,
                      bool applyToAllRemaining, QWidget* parent=0, const char* name=0);

    /**
     * destrictor doesn't do anything
     */
    ~OverwriteDialogue();

    /**
     * @return the value of the Apply To All Remaining Files checkbox
     */
    bool applyToAllRemaining();

protected slots:
    /**
     * Overrides standard operation to call QDialog::done(Yes).
     *  This is a kludge, see note in class description.
     */
    virtual void slotOk();

    /**
    * Overrides standard operation to call QDialog::done(No).
    */
    virtual void slotApply();


    /**
    * Overrides standard operation to call QDialog::done(Cancel).
    */
    virtual void slotCancel();

private:
    QCheckBox* m_applyToAllRemaining;
};

#endif
