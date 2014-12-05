/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLENTITYATTRIBUTEDIALOG_H
#define UMLENTITYATTRIBUTEDIALOG_H

#include <QDialog>

class QCheckBox;
class QGroupBox;
class QLabel;
class QRadioButton;
class UMLEntityAttribute;
class UMLStereotypeWidget;
class KComboBox;
class KLineEdit;

/**
 * @author Jonathan Riddell
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLEntityAttributeDialog : public QDialog
{
    Q_OBJECT
public:
    UMLEntityAttributeDialog(QWidget* pParent, UMLEntityAttribute* pEntityAttribute);
    ~UMLEntityAttributeDialog();

protected:
    void setupDialog();

    bool apply();

    void insertTypesSorted(const QString& type = QString());
    void insertAttribute(const QString& type, int index = -1);

    UMLEntityAttribute * m_pEntityAttribute;  //< the EntityAttribute to represent

    //GUI Widgets
    QGroupBox * m_pAttsGB, * m_pValuesGB;
    QGroupBox * m_pScopeGB;
    QRadioButton * m_pPublicRB, * m_pPrivateRB,  * m_pProtectedRB, * m_pNoneRB;
    QLabel * m_pTypeL, * m_pNameL, * m_pInitialL, * m_pValuesL, * m_pAttributesL ;
    KComboBox * m_pTypeCB;
    KComboBox * m_pAttributesCB;
    KLineEdit * m_pNameLE, * m_pInitialLE, * m_pValuesLE;
    QCheckBox* m_pAutoIncrementCB;
    QCheckBox* m_pNullCB;
    UMLStereotypeWidget *m_stereotypeWidget;

public slots:
    void slotAutoIncrementStateChanged(bool checked);
    void slotOk();
};

#endif
