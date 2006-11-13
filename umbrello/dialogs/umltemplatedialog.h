/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLTEMPLATEDIALOG_H
#define UMLTEMPLATEDIALOG_H

#include <kdialogbase.h>

class QButtonGroup;
class QCheckBox;
class KComboBox;
class QGroupBox;
class QLineEdit;
class QRadioButton;
class UMLTemplate;

/**
 * A dialog to edit the properties of a class template (paramaterised class)
 *
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLTemplateDialog : public KDialogBase {
    Q_OBJECT
public:
    UMLTemplateDialog(QWidget* pParent, UMLTemplate* pAttribute);
    ~UMLTemplateDialog();

protected:
    /**
    *   Sets up the dialog
    */
    void setupDialog();

    /**
     * Checks if changes are valid and applies them if they are,
     * else returns false
     */
    bool apply();

    /**
    *   The Attribute to represent
    */
    UMLTemplate* m_pTemplate;

    //GUI Widgets
    QGroupBox* m_pTemplateGB;
    QGroupBox* m_pValuesGB;
    QLabel *m_pTypeL, *m_pNameL, *m_pStereoTypeL;
    KComboBox* m_pTypeCB;
    QLineEdit *m_pNameLE, *m_pStereoTypeLE;

public slots:
    /**
     * I don't think this is used, but if we had an apply button
     * it would slot into here
     */
    void slotApply();

    /**
     * Used when the OK button is clicked.  Calls apply()
     */
    void slotOk();
protected:
    /**
    * Inserts @p type into the type-combobox as well as its completion object.
    */
    void insertType( const QString& type, int index = -1 );

};

#endif
