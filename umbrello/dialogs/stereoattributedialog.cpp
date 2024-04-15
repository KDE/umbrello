/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "stereoattributedialog.h"

// app includes
#include "stereotype.h"
#include "umldoc.h"
#include "uml.h"
#include "dialog_utils.h"
#include "debug_utils.h"

// kde includes
#include <QLineEdit>
#include <kcombobox.h>
#include <kcompletion.h>
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QDialogButtonBox>

DEBUG_REGISTER(StereoAttributeDialog)

StereoAttributeDialog::StereoAttributeDialog(QWidget *parent, UMLStereotype *stereotype)
  : SinglePageDialogBase(parent)
{
    setCaption(i18n("Stereotype Properties"));
    m_pStereotype = stereotype;
    setupDialog();
}

StereoAttributeDialog::~StereoAttributeDialog()
{
}

/**
 *   Sets up the dialog
 */
void StereoAttributeDialog::setupDialog()
{
    int margin = fontMetrics().height();

    QFrame * frame = new QFrame(this);
    setMainWidget(frame);
    QVBoxLayout * mainLayout = new QVBoxLayout(frame);

    m_pValuesGB = new QGroupBox(i18n("Stereotype Attributes for ") + m_pStereotype->name(true),
                                frame);
    QGridLayout * valuesLayout = new QGridLayout(m_pValuesGB);
    valuesLayout->setSpacing(10);

    /*
    QLabel    *m_pNameLabel[N_STEREOATTRS];
    QLineEdit *m_pNameEdit [N_STEREOATTRS];
    QLabel    *m_pTypeLabel[N_STEREOATTRS];
    QComboBox *m_pTypeCombo[N_STEREOATTRS];
     */
    const UMLStereotype::AttributeDefs& adefs = m_pStereotype->getAttributeDefs();

    for (int row = 0; row < N_STEREOATTRS; row++) {
        Dialog_Utils::makeLabeledEditField(valuesLayout, row,
                                    m_pNameLabel[row], i18nc("attribute name", "&Name:"),
                                    m_pNameEdit[row]);        // columns 0, 1

        m_pTypeLabel[row] = new QLabel(i18nc("attribute type", "&Type:"));
        valuesLayout->addWidget(m_pTypeLabel[row], row, 2);   // column 2
        m_pTypeCombo[row] = new QComboBox(this);
        for (int type = 0; type < Uml::PrimitiveTypes::n_types; type++) {
            m_pTypeCombo[row]->addItem(Uml::PrimitiveTypes::toString(type));
        }
        valuesLayout->addWidget(m_pTypeCombo[row], row, 3);   // column 3
        m_pTypeLabel[row]->setBuddy(m_pTypeCombo[row]);

        Dialog_Utils::makeLabeledEditField(valuesLayout, row,
                                    m_pDefaultValueLabel[row], i18nc("default value", "&Default:"),
                                    m_pDefaultValueEdit[row], QString(), 4);  // columns 4, 5

        if (adefs.count() > row) {
            const UMLStereotype::AttributeDef& adef = adefs.at(row);
            if (!adef.name.isEmpty()) {
                m_pNameEdit[row]->setText(adef.name.trimmed());
                int type = int(adef.type);
                if (type >= 0 && type < m_pTypeCombo[row]->count())
                    m_pTypeCombo[row]->setCurrentIndex(type);
                else
                    logDebug1("StereoAttributeDialog::setupDialog: Illegal type %1", type);
                const QString& dfltVal = adef.defaultVal;
                if (!dfltVal.isEmpty())
                    m_pDefaultValueEdit[row]->setText(dfltVal);
            }
        }
    }
    mainLayout->addWidget(m_pValuesGB);
}

/**
 * Used when the OK button is clicked. Applies the stereotype attribute changes.
 */
bool StereoAttributeDialog::apply()
{
    m_pStereotype->clearAttributeDefs();
    UMLStereotype::AttributeDefs adefs;
    for (int i = 0; i < N_STEREOATTRS; i++) {
        QString name = m_pNameEdit[i]->text().trimmed();
        if (!name.isEmpty()) {
            int typeIndex = m_pTypeCombo[i]->currentIndex();
            if (typeIndex < 0)
                typeIndex = 0;
            Uml::PrimitiveTypes::Enum type = Uml::PrimitiveTypes::Enum(typeIndex);
            QString dfltVal = m_pDefaultValueEdit[i]->text().trimmed();
            UMLStereotype::AttributeDef attrDef(name, type, dfltVal);
            adefs.append(attrDef);
        }
    }
    if (adefs.count())
        m_pStereotype->setAttributeDefs(adefs);
    return true;
}
