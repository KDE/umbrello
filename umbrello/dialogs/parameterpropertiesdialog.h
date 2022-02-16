/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef PARAMETERPROPERTIESDIALOG_H
#define PARAMETERPROPERTIESDIALOG_H

#include "basictypes.h"

#include "singlepagedialogbase.h"

class DocumentationWidget;
class DefaultValueWidget;
class KComboBox;
class KLineEdit;
class QGroupBox;
class QLabel;
class QRadioButton;
class UMLAttribute;
class UMLDoc;
class UMLDatatypeWidget;
class UMLStereotypeWidget;

/**
 * Displays a dialog box that displays properties of a parameter.
 * You need to make sure that @ref UMLDoc is made to be the
 * parent.
 *
 * @short A properties dialog box for a parameter.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ParameterPropertiesDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:

    ParameterPropertiesDialog(QWidget * parent, UMLDoc * doc, UMLAttribute * attr);
    ~ParameterPropertiesDialog();

    virtual bool apply();

protected:
    QString getName();
    QString getInitialValue();
    Uml::ParameterDirection::Enum getParmKind();
    virtual bool validate();

private:
    QGroupBox * m_pParmGB;
    QGroupBox * m_pKindGB;
    QRadioButton * m_pIn, * m_pOut, *m_pInOut;
    QLabel * m_pNameL;
    KLineEdit * m_pNameLE;
    DefaultValueWidget * m_defaultValueWidget;
    DocumentationWidget * m_docWidget;
    UMLDoc * m_pUmldoc;
    UMLAttribute * m_pAtt;
    UMLDatatypeWidget *m_datatypeWidget;
    UMLStereotypeWidget *m_stereotypeWidget;
};

#endif
