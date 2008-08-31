/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLENTITYATTRIBUTEDIALOG_H
#define UMLENTITYATTRIBUTEDIALOG_H

#include <kdialog.h>

class QCheckBox;
class QGroupBox;
class QLabel;
class QRadioButton;
class UMLEntityAttribute;
class KComboBox;
class KLineEdit;

/**
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLEntityAttributeDialog : public KDialog
{
    Q_OBJECT
public:
    UMLEntityAttributeDialog( QWidget* pParent, UMLEntityAttribute* pEntityAttribute );
    ~UMLEntityAttributeDialog();

protected:

    void setupDialog();

    bool apply();

    void insertTypesSorted(const QString& type = "");

    void insertAttribute( const QString& type, int index = -1 );

    /**
     * The EntityAttribute to represent.
     */
    UMLEntityAttribute * m_pEntityAttribute;

    //GUI Widgets
    QGroupBox * m_pAttsGB, * m_pValuesGB;
    QGroupBox * m_pScopeGB;
    QRadioButton * m_pPublicRB, * m_pPrivateRB,  * m_pProtectedRB, * m_pNoneRB;
    QLabel * m_pTypeL, * m_pNameL, * m_pInitialL, * m_pStereoTypeL, * m_pValuesL, * m_pAttributesL ;
    KComboBox * m_pTypeCB;
    KComboBox * m_pAttributesCB;
    KLineEdit * m_pNameLE, * m_pInitialLE, * m_pStereoTypeLE, * m_pValuesLE;
    QCheckBox* m_pAutoIncrementCB;
    QCheckBox* m_pNullCB;

public slots:

    void slotAutoIncrementStateChanged(bool checked);

    void slotApply();

    void slotOk();
    void slotNameChanged( const QString & );
};

#endif
