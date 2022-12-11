/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "parameterpropertiesdialog.h"

// local includes
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "documentationwidget.h"
#include "defaultvaluewidget.h"
#include "umldatatypewidget.h"
#include "umlstereotypewidget.h"
#include "umltemplatelist.h"
#include "template.h"
#include "umldoc.h"
#include "dialog_utils.h"
#include "object_factory.h"
#include "stereotype.h"

// kde includes
#include <KComboBox>
#include <QLineEdit>
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QVBoxLayout>

/**
 * Constructs a ParameterPropertiesDialog.
 * @param parent   the parent of the dialog
 * @param doc      UMLDoc instance for access to classifiers and stereotypes
 * @param attr     the parameter to represent
 */
ParameterPropertiesDialog::ParameterPropertiesDialog(QWidget * parent, UMLDoc * doc, UMLAttribute * attr)
  : SinglePageDialogBase(parent)
{
    Q_ASSERT(attr);
    setCaption(i18n("Parameter Properties"));

    m_pUmldoc = doc;
    m_pAtt = attr;

    int margin = fontMetrics().height();
    QFrame *frame = new QFrame(this);
    setMainWidget(frame);
    QVBoxLayout * topLayout = new QVBoxLayout(frame);
    topLayout->setSpacing(10);
    topLayout->setMargin(margin);

    m_pParmGB = new QGroupBox(i18n("Properties"));
    topLayout->addWidget(m_pParmGB);

    QGridLayout * propLayout = new QGridLayout(m_pParmGB);
    propLayout->setSpacing(10);
    propLayout->setMargin(margin);

    m_datatypeWidget = new UMLDatatypeWidget(m_pAtt);
    m_datatypeWidget->addToLayout(propLayout, 0);

    Dialog_Utils::makeLabeledEditField(propLayout, 1,
                                    m_pNameL, i18nc("property name", "&Name:"),
                                    m_pNameLE, attr->name());

    m_defaultValueWidget = new DefaultValueWidget(attr->getType(), attr->getInitialValue(), this);
    m_defaultValueWidget->addToLayout(propLayout, 2);
    connect(m_datatypeWidget, SIGNAL(editTextChanged(QString)), m_defaultValueWidget, SLOT(setType(QString)));

    m_stereotypeWidget = new UMLStereotypeWidget(m_pAtt);
    m_stereotypeWidget->addToLayout(propLayout, 3);

    m_pKindGB =  new QGroupBox(i18n("Passing Direction"));
    m_pKindGB->setToolTip(i18n("\"in\" is a readonly parameter, \"out\" is a writeonly parameter and \"inout\" is a parameter for reading and writing."));

    QHBoxLayout * kindLayout = new QHBoxLayout(m_pKindGB);
    kindLayout->setMargin(margin);

    m_pIn =  new QRadioButton(QString::fromLatin1("in"), m_pKindGB);
    kindLayout->addWidget(m_pIn);

    m_pInOut =  new QRadioButton(QString::fromLatin1("inout"), m_pKindGB);
    kindLayout->addWidget(m_pInOut);

    m_pOut =  new QRadioButton(QString::fromLatin1("out"), m_pKindGB);
    kindLayout->addWidget(m_pOut);

    topLayout->addWidget(m_pKindGB);

    m_docWidget = new DocumentationWidget(m_pAtt);
    topLayout->addWidget(m_docWidget);

    // Check the proper Kind radiobutton.
    Uml::ParameterDirection::Enum kind = attr->getParmKind();
    if (kind == Uml::ParameterDirection::Out)
        m_pOut->setChecked(true);
    else if (kind == Uml::ParameterDirection::InOut)
        m_pInOut->setChecked(true);
    else
        m_pIn->setChecked(true);

    // set tab order
    setTabOrder(m_pKindGB, m_datatypeWidget);
    setTabOrder(m_datatypeWidget, m_pNameLE);
    setTabOrder(m_pNameLE, m_defaultValueWidget);
    setTabOrder(m_defaultValueWidget, m_stereotypeWidget);
    setTabOrder(m_stereotypeWidget, m_pIn);
    setTabOrder(m_pIn, m_docWidget);
    m_pNameLE->setFocus();
}

/**
 * Standard destructor.
 */
ParameterPropertiesDialog::~ParameterPropertiesDialog()
{
}

QString ParameterPropertiesDialog::getName()
{
    return m_pNameLE->text();
}

QString ParameterPropertiesDialog::getInitialValue()
{
    return m_defaultValueWidget->value();
}

/**
 * Return the kind of the parameter (in, out, or inout).
 * @return  The Uml::ParameterDirection::Enum corresponding to
 *          the selected "Kind" radiobutton.
 */
Uml::ParameterDirection::Enum ParameterPropertiesDialog::getParmKind()
{
    Uml::ParameterDirection::Enum pk = Uml::ParameterDirection::In;
    if (m_pOut->isChecked())
        pk = Uml::ParameterDirection::Out;
    else if (m_pInOut->isChecked())
        pk = Uml::ParameterDirection::InOut;
    return pk;
}

/**
 * Validates the fields in the dialog box.
 * @return success state
 */
bool ParameterPropertiesDialog::validate()
{
    // currently only validates whether the name is not null.
    if (getName().trimmed().length() == 0) {
            KMessageBox::error(this, i18n("You have entered an invalid parameter name."),
                               i18n("Parameter Name Invalid"), 0);
            return false;
    }
    return true;
}

bool ParameterPropertiesDialog::apply()
{
    if (m_pAtt != 0) {

        m_pAtt->setName(getName());         // set the name
        m_pAtt->setParmKind(getParmKind());  // set the direction
        m_stereotypeWidget->apply();
        m_datatypeWidget->apply();
        m_docWidget->apply();
        m_pAtt->setInitialValue(m_defaultValueWidget->value()); // set the initial value
    }
    return true;
}

