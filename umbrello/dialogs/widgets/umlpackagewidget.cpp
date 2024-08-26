/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlpackagewidget.h"

#include "import_utils.h"
#include "model_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "folder.h"
#include "package.h"

#include <KComboBox>
#include <KLocalizedString>

#include <QHBoxLayout>
#include <QLabel>

UMLPackageWidget::UMLPackageWidget(UMLObject *o, QWidget *parent) :
    QWidget(parent),
    m_object(o)
{
    Q_ASSERT(o);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    m_label = new QLabel(i18n("Package path:"), this);
    layout->addWidget(m_label);

    m_editField = new KComboBox(this);
    m_editField->setEditable(true);
    layout->addWidget(m_editField, 2);
    setLayout(layout);

    m_label->setBuddy(m_editField);

    Uml::ModelType::Enum guess = Model_Utils::guessContainer(o);
    UMLPackageList packageList = UMLApp::app()->document()->packages(true, guess);
    QStringList packages;
    for(UMLPackage *package : packageList) {
        packages << package->name();
    }
    packages.sort();
    m_editField->insertItems(-1, packages);
    QString packagePath = o->package();
    UMLPackage* parentPackage = o->umlPackage();

    UMLPackage* folderLogicalView =
            UMLApp::app()->document()->rootFolder(Uml::ModelType::Logical)->asUMLPackage();
    if (parentPackage == nullptr ||
         parentPackage == folderLogicalView) {
        m_editField->setEditText(QString());
    }
    else {
        m_editField->setEditText(packagePath);
    }
}

UMLPackageWidget::~UMLPackageWidget()
{
    delete m_editField;
    delete m_label;
}

/**
 * Add this widget to a given grid layout. Umbrello dialogs places labels in column 0
 * and the editable field in column 1.
 * @param layout The layout to which the widget should be added
 * @param row The row in the grid layout where the widget should be placed
 */
void UMLPackageWidget::addToLayout(QGridLayout *layout, int row)
{
    layout->addWidget(m_label, row, 0);
    layout->addWidget(m_editField, row, 1);
}

/**
 * Apply changes to the related UMLObject.
 */
void UMLPackageWidget::apply()
{
    QString packageName = m_editField->currentText().trimmed();
    UMLObject *newPackage = nullptr;

    if (!packageName.isEmpty()) {
        if ((newPackage = UMLApp::app()->document()->findUMLObject(packageName, UMLObject::ot_Package)) == nullptr) {
            newPackage = Import_Utils::createUMLObject(UMLObject::ot_Package, packageName);
        }
    } else {
        Uml::ModelType::Enum guess = Model_Utils::guessContainer(m_object);
        newPackage = UMLApp::app()->document()->rootFolder(guess);
    }

    // adjust list view items
    Model_Utils::treeViewMoveObjectTo(newPackage, m_object);
}
