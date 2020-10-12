/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2020                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef STEREOATTRIBUTEDIALOG_H
#define STEREOATTRIBUTEDIALOG_H

#include "singlepagedialogbase.h"

class QGroupBox;
class QLabel;
class KLineEdit;
class QComboBox;
class UMLStereotype;

/**
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class StereoAttributeDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    StereoAttributeDialog(QWidget *parent, UMLStereotype *stereotype);
    ~StereoAttributeDialog();

protected:
    void setupDialog();
    bool apply();

    /**
     * The Stereotype to represent
     */
    UMLStereotype *m_pStereotype;

#define N_STEREOATTRS 3

    //GUI Widgets
    QGroupBox * m_pValuesGB;
    QLabel    * m_pNameLabel[N_STEREOATTRS];
    KLineEdit * m_pNameEdit [N_STEREOATTRS];
    QLabel    * m_pTypeLabel[N_STEREOATTRS];
    QComboBox * m_pTypeCombo[N_STEREOATTRS];

};

#endif
