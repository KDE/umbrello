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

#ifndef UMLATTRIBUTEDIALOG_H
#define UMLATTRIBUTEDIALOG_H

#include <kdialogbase.h>

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QRadioButton;
class UMLAttribute;
class KComboBox;
class QLineEdit;

class UMLAttributeDialog : public KDialogBase {
    Q_OBJECT
public:
    UMLAttributeDialog( QWidget * pParent, UMLAttribute * pAttribute );
    ~UMLAttributeDialog();

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
    * Inserts @p type into the type-combobox as well as its completion object.
    */
    void insertType( const QString& type, int index = -1 );

    /**
    *   The Attribute to represent
    */
    UMLAttribute * m_pAttribute;

    //GUI Widgets
    QGroupBox * m_pAttsGB, * m_pValuesGB;
    QButtonGroup * m_pScopeBG;
    QRadioButton * m_pPublicRB, * m_pPrivateRB,  * m_pProtectedRB, * m_pImplementationRB;
    QLabel * m_pTypeL, * m_pNameL, * m_pInitialL, * m_pStereoTypeL;
    KComboBox * m_pTypeCB;
    QLineEdit * m_pNameLE, * m_pInitialLE, * m_pStereoTypeLE;
    QCheckBox* m_pStaticCB;

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
    void slotNameChanged( const QString & );
};

#endif
