/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef GENERALOPTIONPAGE_H
#define GENERALOPTIONPAGE_H

#include <QtGui/QWidget>

class KIntSpinBox;
class KLineEdit;
class KComboBox;
class QGroupBox;
class QCheckBox;
class QLabel;
class QGridLayout;

/**
 * A dialog page to display auto layouts options
 * 
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 *
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class GeneralOptionPage : public QWidget
{
    Q_OBJECT
public:
    GeneralOptionPage(QWidget* parent);
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

        KIntSpinBox * timeISB;
        KComboBox * diagramKB;
        KComboBox * languageKB;

        QCheckBox * undoCB;
        QCheckBox * tabdiagramsCB;
        QCheckBox * newcodegenCB;
        QCheckBox * angularLinesCB;
        QCheckBox * footerPrintingCB;    void slotAutosaveCBClicked();
        
        QCheckBox * autosaveCB;
        QCheckBox * loadlastCB;

        // Allow definition of Suffix for autosave
        // ( Default: ".xmi" ), private Ui::GeneralOptionPage
        KLineEdit * autosaveSuffixT;
        QLabel    * autosaveSuffixL;
        // End AutoSave Suffix

        QLabel * startL;
        QLabel * autosaveL;
        QLabel * defaultLanguageL;
    } m_GeneralWidgets;

    void insertDiagram( const QString& type, int index );

protected slots:
    void slotAutosaveCBClicked();

};
#endif
