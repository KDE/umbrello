/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLTEMPLATEDIALOG_H
#define UMLTEMPLATEDIALOG_H

#include <kdialog.h>

class KComboBox;
class KLineEdit;
class QGroupBox;
class QLabel;
class UMLTemplate;

/**
 * A dialog to edit the properties of a class template (paramaterised class)
 *
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLTemplateDialog : public KDialog
{
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

    // GUI Widgets
    QGroupBox *m_pValuesGB;
    QLabel *m_pTypeL, *m_pNameL, *m_pStereoTypeL;
    KComboBox *m_pTypeCB;
    KLineEdit *m_pNameLE, *m_pStereoTypeLE;

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
     * Inserts @p type into the type-combobox.
     * The combobox is cleared and all types together with the optional new one
     * sorted and then added again.
     * @param type   a new type to add and selected
     */
    void insertTypesSorted(const QString& type = "");

};

#endif
