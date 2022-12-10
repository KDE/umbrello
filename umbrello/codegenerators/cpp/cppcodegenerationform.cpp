/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "cppcodegenerationform.h"

// kde includes
#if QT_VERSION < 0x050000
#include <kfiledialog.h>
#endif
#include <KLocalizedString>
#include <kcombobox.h>
#include <KMessageBox>

// qt includes
#if QT_VERSION >= 0x050000
#include <QFileDialog>
#endif
#include <QLabel>
#include <QListWidget>
#include <QRegExp>

/**
 * Constructor.
 * @param parent   the parent of this widget
 * @param name     the object name
 */
CPPCodeGenerationForm::CPPCodeGenerationForm(QWidget *parent, const char *name)
  : QWidget(parent)
{
    setObjectName(QLatin1String(name));
    setupUi(this);

    Qt::ItemFlags flags = Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    m_optionPackageIsANamespace =
        new QListWidgetItem(i18n("Package is a namespace"), ui_generalOptionsListWidget);
    m_optionPackageIsANamespace->setFlags(flags);

    m_optionVirtualDestructors =
        new QListWidgetItem(i18n("Virtual destructors"), ui_generalOptionsListWidget);
    m_optionVirtualDestructors->setFlags(flags);

    m_optionGenerateEmptyConstructors =
        new QListWidgetItem(i18n("Generate empty constructors"), ui_generalOptionsListWidget);
    m_optionGenerateEmptyConstructors->setFlags(flags);

    m_optionGenerateAccessorMethods =
        new QListWidgetItem(i18n("Generate accessor methods"), ui_generalOptionsListWidget);
    m_optionGenerateAccessorMethods->setFlags(flags);

    m_optionOperationsAreInline =
        new QListWidgetItem(i18n("Operations are inline"), ui_generalOptionsListWidget);
    m_optionOperationsAreInline->setFlags(flags);

    m_optionAccessorsAreInline =
        new QListWidgetItem(i18n("Accessors are inline"), ui_generalOptionsListWidget);
    m_optionAccessorsAreInline->setFlags(flags);

    m_optionAccessorsArePublic =
        new QListWidgetItem(i18n("Accessors are public"), ui_generalOptionsListWidget);
    m_optionAccessorsArePublic->setFlags(flags);

    m_optionGetterWithGetPrefix =
        new QListWidgetItem(i18n("Create getters with 'get' prefix"), ui_generalOptionsListWidget);
    m_optionGetterWithGetPrefix->setFlags(flags);

    m_optionRemovePrefixFromAccessorMethodName =
            new QListWidgetItem(i18n("Remove prefix '[a-zA-Z]_' from accessor method names"), ui_generalOptionsListWidget);
        m_optionRemovePrefixFromAccessorMethodName->setFlags(flags);

    m_optionAccessorMethodsStartWithUpperCase =
            new QListWidgetItem(i18n("Accessor methods start with capital letters"), ui_generalOptionsListWidget);
        m_optionAccessorMethodsStartWithUpperCase->setFlags(flags);

    m_optionDocToolTag =
        new QListWidgetItem(i18n("Use '\\' as documentation tag instead of '@'"), ui_generalOptionsListWidget);
    m_optionDocToolTag->setFlags(flags);

    connect(ui_generalOptionsListWidget,
            SIGNAL(itemClicked(QListWidgetItem*)), this,
            SLOT(generalOptionsListWidgetClicked(QListWidgetItem*)));

    connect(ui_generalOptionsListWidget,
            SIGNAL(itemDoubleClicked(QListWidgetItem*)), this,
            SLOT(editClassMemberPrefixDoubleClicked(QListWidgetItem*)));
}

/**
 * Destructor.
 */
CPPCodeGenerationForm::~CPPCodeGenerationForm()
{
}

/**
 * Slot for clicking on the browse buttons.
 */
void CPPCodeGenerationForm::browseClicked()
{
    QString button = sender()->objectName();
#if QT_VERSION >= 0x050000
    QString file = QFileDialog::getOpenFileName(this, QStringLiteral("Get Header File"), QString(), QStringLiteral("*.h"));
#else
    QString file = KFileDialog::getOpenFileName(KUrl(), QStringLiteral("*.h"), this, QStringLiteral("Get Header File"));
#endif
    if (file.isEmpty()) {
        return;
    }

    if (button == QStringLiteral("m_browseStringButton")) {
        // search for match in history list, if absent, then add it
        ui_stringIncludeFileHistoryCombo->setCurrentItem(file, true);
    }
    else if (button == QStringLiteral("m_browseListButton")) {
        // search for match in history list, if absent, then add it
        ui_listIncludeFileHistoryCombo->setCurrentItem(file, true);
    }
}

/**
 * Slot for clicking in the list widget.
 * @param pSender   the sender of the signal, the item in the list
 */
void CPPCodeGenerationForm::generalOptionsListWidgetClicked(QListWidgetItem *pSender)
{
    // operations are inline and accessors are operations :)
    if (m_optionOperationsAreInline->checkState() == Qt::Checked &&
        m_optionGenerateAccessorMethods->checkState() == Qt::Checked) {
        m_optionAccessorsAreInline->setCheckState(Qt::Checked);
    }

    if (pSender == m_optionPackageIsANamespace) {
#if 0
        KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                           "sender=m_optionPackageIsANamespace");
#endif
        return;
    }
    if (pSender == m_optionVirtualDestructors) {
#if 0
        KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                           "sender=m_optionVirtualDestructors");
#endif
        return;
    }
    if (pSender == m_optionGenerateEmptyConstructors) {
#if 0
        KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                           "sender=m_optionVirtualDestructors");
#endif
        return;
    }
    if (pSender == m_optionGenerateAccessorMethods) {
        bool dontGenerateAccessorMethods =
            (m_optionGenerateAccessorMethods->checkState() == Qt::Unchecked);
        m_optionAccessorsAreInline->setHidden(dontGenerateAccessorMethods);
        m_optionAccessorsArePublic->setHidden(dontGenerateAccessorMethods);
        m_optionGetterWithGetPrefix->setHidden(dontGenerateAccessorMethods);
        m_optionRemovePrefixFromAccessorMethodName->setHidden(dontGenerateAccessorMethods);
        m_optionAccessorMethodsStartWithUpperCase->setHidden(dontGenerateAccessorMethods);
        // reset the value if needed
        if (dontGenerateAccessorMethods) {
            m_optionAccessorsAreInline->setCheckState(Qt::Unchecked);
            m_optionAccessorsArePublic->setCheckState(Qt::Unchecked);
            m_optionGetterWithGetPrefix->setCheckState(Qt::Unchecked);
            m_optionRemovePrefixFromAccessorMethodName->setCheckState(Qt::Unchecked);
            m_optionAccessorMethodsStartWithUpperCase->setHidden(Qt::Unchecked);
        }
#if 0
        KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                           "sender=m_optionGenerateAccessorMethods");
#endif
        return;
    }
    if (pSender == m_optionOperationsAreInline) {
#if 0
        KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                           "sender=m_optionOperationsAreInline");
#endif
        return;
    }
    if (pSender == m_optionAccessorsAreInline) {
#if 0
        KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                           "sender=m_optionAccessorsAreInline");
#endif
        return;
    }

#if 0
    KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                       "unknown sender");
#endif
    return;
}

/**
 * Set the display state of option "Package Is Namespace".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setPackageIsANamespace(bool bFlag)
{
    m_optionPackageIsANamespace->setCheckState(toCheckState(bFlag));
}

/**
 * Set the display state of option "Virtual Destructors".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setVirtualDestructors(bool bFlag)
{
    m_optionVirtualDestructors->setCheckState(toCheckState(bFlag));
}

/**
 * Set the display state of option "Generate Empty Constructors".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setGenerateEmptyConstructors(bool bFlag)
{
    m_optionGenerateEmptyConstructors->setCheckState(toCheckState(bFlag));
}

/**
 * Set the display state of option "Generate Accessor Methods".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setGenerateAccessorMethods(bool bFlag)
{
    m_optionGenerateAccessorMethods->setCheckState(toCheckState(bFlag));
    // initial settings
    m_optionAccessorsAreInline->setHidden(m_optionGenerateAccessorMethods->checkState() == Qt::Unchecked);
    m_optionAccessorsArePublic->setHidden(m_optionGenerateAccessorMethods->checkState() == Qt::Unchecked);
    // reset the value if needed
    if (m_optionGenerateAccessorMethods->checkState() == Qt::Unchecked) {
        m_optionAccessorsAreInline->setCheckState(Qt::Unchecked);
        m_optionAccessorsArePublic->setCheckState(Qt::Unchecked);
    }
}

/**
 * Set the display state of option "Operations Are Inline".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setOperationsAreInline(bool bFlag)
{
    m_optionOperationsAreInline->setCheckState(toCheckState(bFlag));
}

/**
 * Set the display state of option "Accessors Are Inline".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setAccessorsAreInline(bool bFlag)
{
    m_optionAccessorsAreInline->setCheckState(toCheckState(bFlag));
}

/**
 * Set the display state of option "Accessors Are Public".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setAccessorsArePublic(bool bFlag)
{
    m_optionAccessorsArePublic->setCheckState(toCheckState(bFlag));
}

/**
 * Set the display state of the related checkbox
 * @param bFlag  the flag to set
 */
void CPPCodeGenerationForm::setGetterWithoutGetPrefix(bool bFlag)
{
    m_optionGetterWithGetPrefix->setCheckState(toCheckState(toCheckState(bFlag)));
}

/**
 * Set the display state of the related checkbox
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setRemovePrefixFromAccessorMethodName(bool bFlag)
{
    m_optionRemovePrefixFromAccessorMethodName->setCheckState(toCheckState(toCheckState(bFlag)));
}

/**
 * Set the display state of the related checkbox
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setAccessorMethodsStartWithUpperCase(bool bFlag)
{
    m_optionAccessorMethodsStartWithUpperCase->setCheckState(toCheckState(toCheckState(bFlag)));
}

/**
 * Set the doc display state of option "Doc Tool Tag".
 * @param value   the value of the tag
 */
void CPPCodeGenerationForm::setDocToolTag(const QString &value)
{
    m_optionDocToolTag->setCheckState(toCheckState(value == QStringLiteral("\\")));
}

/**
 * Set the class member prefix
 * @param value  the value to set
 */
void CPPCodeGenerationForm::setClassMemberPrefix(const QString &value)
{
    ui_classMemberPrefixEdit->setText(value);
}

/**
 * Get the display state of option "Package Is Namespace".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getPackageIsANamespace()
{
    return m_optionPackageIsANamespace->checkState() == Qt::Checked;
}

/**
 * Get the display state of option "Virtual Destructors".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getVirtualDestructors()
{
    return m_optionVirtualDestructors->checkState() == Qt::Checked;
}

/**
 * Get the display state of option "Generate Empty Constructors".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getGenerateEmptyConstructors()
{
    return m_optionGenerateEmptyConstructors->checkState() == Qt::Checked;
}

/**
 * Get the display state of option "Generate Accessor Methods".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getGenerateAccessorMethods()
{
    return m_optionGenerateAccessorMethods->checkState() == Qt::Checked;
}

/**
 * Get the display state of option "Operations Are Inline".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getOperationsAreInline()
{
    return m_optionOperationsAreInline->checkState() == Qt::Checked;
}

/**
 * Get the display state of option "Accessors Are Inline".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getAccessorsAreInline()
{
    return m_optionAccessorsAreInline->checkState() == Qt::Checked;
}

/**
 * Get the display state of option "Accessors Are Public".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getAccessorsArePublic()
{
    return m_optionAccessorsArePublic->checkState() == Qt::Checked;
}

/**
 * Get the display state of the related option
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getGettersWithGetPrefix()
{
    return m_optionGetterWithGetPrefix->checkState() == Qt::Checked;
}

/**
 * Get the display state of the related option
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getRemovePrefixFromAccessorMethodName()
{
    return m_optionRemovePrefixFromAccessorMethodName->checkState() == Qt::Checked;
}

/**
 * Get the display state of the related option
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getAccessorMethodsStartWithUpperCase()
{
    return m_optionAccessorMethodsStartWithUpperCase->checkState() == Qt::Checked;
}

/**
 * Get the display state of the related option
 * @return   the state of the flag
 */
QString CPPCodeGenerationForm::getDocToolTag()
{
    return m_optionDocToolTag->checkState() == Qt::Checked ? QStringLiteral("\\") : QStringLiteral("@");
}

/**
 * Get the class member prefix
 * @return  value
 */
QString CPPCodeGenerationForm::getClassMemberPrefix()
{
    return ui_classMemberPrefixEdit->text();
}

/**
 * Conversion utility (static) from bool to Qt::CheckState.
 * @param value   the value to be converted
 * @return        the check state
 */
Qt::CheckState CPPCodeGenerationForm::toCheckState(bool value)
{
    if (value) {
        return Qt::Checked;
    }
    else {
        return Qt::Unchecked;
    }
}

