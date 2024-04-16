/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef GENERALOPTIONPAGE_H
#define GENERALOPTIONPAGE_H

#include "dialogpagebase.h"

class QLineEdit;
class KComboBox;
class QGroupBox;
class QCheckBox;
class QLabel;
class QSpinBox;

class SelectLayoutTypeWidget;

/**
 * A dialog page to display auto layouts options
 * 
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 *
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class GeneralOptionPage : public DialogPageBase
{
    Q_OBJECT
public:
    explicit GeneralOptionPage(QWidget *parent = nullptr);
    virtual ~GeneralOptionPage();

    void setDefaults();
    void apply();

Q_SIGNALS:
    void applyClicked();

protected:
    struct GeneralWidgets {
        QGroupBox * miscGB;
        QGroupBox * autosaveGB;
        QGroupBox * startupGB;

        QSpinBox  * timeISB;
        KComboBox * diagramKB;
        KComboBox * languageKB;

        QCheckBox * undoCB;
        QCheckBox * tabdiagramsCB;
        QCheckBox * newcodegenCB;
        QCheckBox * footerPrintingCB;
        QCheckBox * uml2CB;

        QCheckBox * autosaveCB;
        QCheckBox * loadlastCB;

        // Allow definition of Suffix for autosave
        // (Default: ".xmi"), private Ui::GeneralOptionPage
        QLineEdit * autosaveSuffixT;
        QLabel    * autosaveSuffixL;
        // End AutoSave Suffix

        QLabel * startL;
        QLabel * autosaveL;
        QLabel * defaultLanguageL;
    } m_GeneralWidgets;

    void insertDiagram(const QString& type, int index);
    void insertLayoutType(const QString& type, int index);

protected Q_SLOTS:
    void slotAutosaveCBClicked();

};
#endif
