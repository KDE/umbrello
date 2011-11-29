/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "cppcodegenerationform.h"

// kde includes
#include <kfiledialog.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kmessagebox.h>

// qt includes
#include <QtCore/QRegExp>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>

/**
 * Constructor.
 * @param parent   the parent of this widget
 * @param name     the object name
 */
CPPCodeGenerationForm::CPPCodeGenerationForm(QWidget *parent, const char *name)
  : QWidget(parent)
{
    setObjectName(name);
    setupUi(this);

    Qt::ItemFlags flags = Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    m_optionPackageIsANamespace =
        new QListWidgetItem(tr2i18n("Package is a namespace"), ui_generalOptionsListWidget);
    m_optionPackageIsANamespace->setFlags(flags);

    m_optionVirtualDestructors =
        new QListWidgetItem(tr2i18n("Virtual destructors"), ui_generalOptionsListWidget);
    m_optionVirtualDestructors->setFlags(flags);

    m_optionGenerateEmptyConstructors =
        new QListWidgetItem(tr2i18n("Generate empty constructors"), ui_generalOptionsListWidget);
    m_optionGenerateEmptyConstructors->setFlags(flags);

    m_optionGenerateAccessorMethods =
        new QListWidgetItem(tr2i18n("Generate accessor methods"), ui_generalOptionsListWidget);
    m_optionGenerateAccessorMethods->setFlags(flags);

    m_optionOperationsAreInline =
        new QListWidgetItem(tr2i18n("Operations are inline"), ui_generalOptionsListWidget);
    m_optionOperationsAreInline->setFlags(flags);

    m_optionAccessorsAreInline =
        new QListWidgetItem(tr2i18n("Accessors are inline"), ui_generalOptionsListWidget);
    m_optionAccessorsAreInline->setFlags(flags);

    m_optionAccessorsArePublic =
        new QListWidgetItem(tr2i18n("Accessors are public"), ui_generalOptionsListWidget);
    m_optionAccessorsArePublic->setFlags(flags);

    m_optionDocToolTag =
        new QListWidgetItem(tr2i18n("Use '\\' as documentation tag instead of '@'"), ui_generalOptionsListWidget);
    m_optionDocToolTag->setFlags(flags);

    connect(ui_generalOptionsListWidget,
            SIGNAL(itemClicked(QListWidgetItem*)), this,
            SLOT(generalOptionsListWidgetClicked(QListWidgetItem*)));
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
    QString file = KFileDialog::getOpenFileName( KUrl(), "*.h", this, "Get Header File");

    if (file.isEmpty()) {
        return;
    }

    if (button == "m_browseStringButton") {
        // search for match in history list, if absent, then add it
        ui_stringIncludeFileHistoryCombo->setCurrentItem(file, true);
    }
    else if (button == "m_browseListButton") {
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
        // reset the value if needed
        if (dontGenerateAccessorMethods) {
            m_optionAccessorsAreInline->setCheckState(Qt::Unchecked);
            m_optionAccessorsArePublic->setCheckState(Qt::Unchecked);
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
 * Set the doc display state of option "Doc Tool Tag".
 * @param value   the value of the tag
 */
void CPPCodeGenerationForm::setDocToolTag(const QString &value)
{
    m_optionDocToolTag->setCheckState(toCheckState(value == QLatin1String("\\")));
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

QString CPPCodeGenerationForm::getDocToolTag()
{
    return m_optionDocToolTag->checkState() == Qt::Checked ? QLatin1String("\\") : QLatin1String("@");
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

#include "cppcodegenerationform.moc"
