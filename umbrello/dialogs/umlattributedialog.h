/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLATTRIBUTEDIALOG_H
#define UMLATTRIBUTEDIALOG_H

#include "singlepagedialogbase.h"
#include "n_stereoattrs.h"

class DocumentationWidget;
class QCheckBox;
class QGridLayout;
class QGroupBox;
class QRadioButton;
class QLabel;
class UMLObject;
class UMLAttribute;
class QLineEdit;
class UMLDatatypeWidget;
class UMLStereotypeWidget;
class VisibilityEnumWidget;

/**
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLAttributeDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    UMLAttributeDialog(QWidget * pParent, UMLAttribute * pAttribute);
    ~UMLAttributeDialog();

protected:
    void setupDialog();
    bool apply();

    /**
     *   The Attribute to represent
     */
    UMLAttribute * m_pAttribute;

    //GUI Widgets
    QGridLayout * m_pValuesLayout;
    QGroupBox * m_pValuesGB;
    QLabel * m_pNameL, * m_pInitialL;
    QLineEdit * m_pNameLE, * m_pInitialLE;
    QCheckBox* m_pStaticCB;
    UMLDatatypeWidget *m_datatypeWidget;
    UMLStereotypeWidget *m_stereotypeWidget;
    QLabel              *m_pTagL [N_STEREOATTRS];
    QLineEdit           *m_pTagLE[N_STEREOATTRS];
    VisibilityEnumWidget *m_visibilityEnumWidget;
    DocumentationWidget *m_docWidget;

public Q_SLOTS:
    void slotNameChanged(const QString &);
    void slotStereoTextChanged(const QString &);
};

#endif
