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

#ifndef UMLENTITYATTRIBUTEDIALOG_H
#define UMLENTITYATTRIBUTEDIALOG_H

#include <kdialog.h>
//Added by qt3to4:
#include <QLabel>

/**
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class Q3ButtonGroup;
class QCheckBox;
class Q3GroupBox;
class QRadioButton;
class UMLEntityAttribute;
class KComboBox;
class KLineEdit;

class UMLEntityAttributeDialog : public KDialog {
    Q_OBJECT
public:
    UMLEntityAttributeDialog( QWidget* pParent, UMLEntityAttribute* pEntityAttribute );
    ~UMLEntityAttributeDialog();

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
    * Inserts @p type into the type-combobox as well as its completion object.
    */
    void insertAttribute( const QString& type, int index = -1 );

    /**
    *   The EntityAttribute to represent
    */
    UMLEntityAttribute * m_pEntityAttribute;

    //GUI Widgets
    Q3GroupBox * m_pAttsGB, * m_pValuesGB;
    Q3ButtonGroup * m_pScopeBG;
    QRadioButton * m_pPublicRB, * m_pPrivateRB,  * m_pProtectedRB, * m_pNoneRB;
    QLabel * m_pTypeL, * m_pNameL, * m_pInitialL, * m_pStereoTypeL, * m_pValuesL, * m_pAttributesL ;
    KComboBox * m_pTypeCB;
    KComboBox * m_pAttributesCB;
    KLineEdit * m_pNameLE, * m_pInitialLE, * m_pStereoTypeLE, * m_pValuesLE;
    QCheckBox* m_pAutoIncrementCB;
    QCheckBox* m_pNullCB;

public slots:

    /**
     * is activated when the auto increment state is changed
     */
    void slotAutoIncrementStateChanged(bool checked);
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
