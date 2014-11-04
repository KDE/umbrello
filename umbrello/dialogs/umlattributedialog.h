/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLATTRIBUTEDIALOG_H
#define UMLATTRIBUTEDIALOG_H

#include <QDialog>

class QCheckBox;
class QGroupBox;
class QRadioButton;
class QLabel;
class UMLAttribute;
class KComboBox;
class KLineEdit;

/**
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLAttributeDialog : public QDialog
{
    Q_OBJECT
public:
    UMLAttributeDialog(QWidget * pParent, UMLAttribute * pAttribute);
    ~UMLAttributeDialog();

protected:
    void setupDialog();

    bool apply();

    void insertTypesSorted(const QString& type = QString());

    UMLAttribute * m_pAttribute;  //< the Attribute to represent

    //GUI Widgets
    QGroupBox * m_pValuesGB;
    QGroupBox * m_pScopeGB;
    QRadioButton * m_pPublicRB, * m_pPrivateRB,  * m_pProtectedRB, * m_pImplementationRB;
    QLabel * m_pTypeL, * m_pNameL, * m_pInitialL, * m_pStereoTypeL;
    KComboBox * m_pTypeCB;
    KLineEdit * m_pNameLE, * m_pInitialLE, * m_pStereoTypeLE;
    QCheckBox* m_pStaticCB;

public slots:
    void slotOk();
};

#endif
