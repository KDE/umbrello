/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLENTITYATTRIBUTEDIALOG_H
#define UMLENTITYATTRIBUTEDIALOG_H

#include "singlepagedialogbase.h"

class DefaultValueWidget;
class QCheckBox;
class QGroupBox;
class QLabel;
class QRadioButton;
class UMLDatatypeWidget;
class UMLEntityAttribute;
class UMLStereotypeWidget;
class KComboBox;
class QLineEdit;

/**
 * @author Jonathan Riddell
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLEntityAttributeDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    UMLEntityAttributeDialog(QWidget* pParent, UMLEntityAttribute* pEntityAttribute);
    ~UMLEntityAttributeDialog();

protected:

    void setupDialog();

    virtual bool apply();

    void insertAttribute(const QString& type, int index = -1);

    /**
     * The EntityAttribute to represent.
     */
    UMLEntityAttribute * m_pEntityAttribute;

    //GUI Widgets
    QGroupBox * m_pAttsGB, * m_pValuesGB;
    QGroupBox * m_pScopeGB;
    QRadioButton * m_pPublicRB, * m_pPrivateRB,  * m_pProtectedRB, * m_pNoneRB;
    QLabel * m_pNameL, * m_pInitialL, * m_pValuesL, * m_pAttributesL ;
    KComboBox * m_pAttributesCB;
    QLineEdit * m_pNameLE, * m_pInitialLE, * m_pValuesLE;
    QCheckBox* m_pAutoIncrementCB;
    QCheckBox* m_pNullCB;
    UMLDatatypeWidget *m_datatypeWidget;
    UMLStereotypeWidget *m_stereotypeWidget;
    DefaultValueWidget *m_defaultValueWidget;

public slots:
    void slotAutoIncrementStateChanged(bool checked);
    void slotNameChanged(const QString &);
};

#endif
