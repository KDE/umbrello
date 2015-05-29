/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef GENERALOPTIONPAGE_H
#define GENERALOPTIONPAGE_H

#include "dialogpagebase.h"

#if QT_VERSION < 0x050000
class KIntSpinBox;
#endif
class KLineEdit;
class KComboBox;
class QGroupBox;
class QCheckBox;
class QLabel;
#if QT_VERSION >= 0x050000
class QSpinBox;
#endif

/**
 * A dialog page to display auto layouts options
 * 
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 *
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class GeneralOptionPage : public DialogPageBase
{
    Q_OBJECT
public:
    explicit GeneralOptionPage(QWidget* parent = 0);
    virtual ~GeneralOptionPage();

    void setDefaults();
    void apply();

signals:
    void applyClicked();

protected:
    struct GeneralWidgets {
        QGroupBox * miscGB;
        QGroupBox * autosaveGB;
        QGroupBox * startupGB;

#if QT_VERSION >= 0x050000
        QSpinBox  * timeISB;
#else
        KIntSpinBox * timeISB;
#endif
        KComboBox * diagramKB;
        KComboBox * languageKB;

        QCheckBox * undoCB;
        QCheckBox * tabdiagramsCB;
        QCheckBox * newcodegenCB;
        QCheckBox * angularLinesCB;
        QCheckBox * footerPrintingCB;
        
        QCheckBox * autosaveCB;
        QCheckBox * loadlastCB;

        // Allow definition of Suffix for autosave
        // (Default: ".xmi"), private Ui::GeneralOptionPage
        KLineEdit * autosaveSuffixT;
        QLabel    * autosaveSuffixL;
        // End AutoSave Suffix

        QLabel * startL;
        QLabel * autosaveL;
        QLabel * defaultLanguageL;
    } m_GeneralWidgets;

    void insertDiagram(const QString& type, int index);

protected slots:
    void slotAutosaveCBClicked();

};
#endif
