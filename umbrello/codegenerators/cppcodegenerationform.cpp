/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "cppcodegenerationform.h"

// qt includes
#include <QtCore/QRegExp>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>

// kde includes
#include <kfiledialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kmessagebox.h>


/**
 * Constructor.
 * @param parent   the parent of this widget
 * @param name     the object name
 */
CPPCodeGenerationForm::CPPCodeGenerationForm( QWidget *parent, const char *name )
        : QWidget (parent)
{
    setObjectName(name);
    setupUi(this);

    Qt::ItemFlags flags = Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    pOptionPackageIsANamespace =
        new QListWidgetItem(tr2i18n("Package is a namespace"), generalOptionsListWidget);
    pOptionPackageIsANamespace->setFlags(flags);

    pOptionVirtualDestructors =
        new QListWidgetItem(tr2i18n("Virtual destructors"), generalOptionsListWidget);
    pOptionVirtualDestructors->setFlags(flags);

    pOptionGenerateEmptyConstructors =
        new QListWidgetItem(tr2i18n("Generate empty constructors"), generalOptionsListWidget);
    pOptionGenerateEmptyConstructors->setFlags(flags);

    pOptionGenerateAccessorMethods =
        new QListWidgetItem(tr2i18n("Generate accessor methods"), generalOptionsListWidget);
    pOptionGenerateAccessorMethods->setFlags(flags);

    pOptionOperationsAreInline =
        new QListWidgetItem(tr2i18n("Operations are inline"), generalOptionsListWidget);
    pOptionOperationsAreInline->setFlags(flags);

    pOptionAccessorsAreInline =
        new QListWidgetItem(tr2i18n("Accessors are inline"), generalOptionsListWidget);
    pOptionAccessorsAreInline->setFlags(flags);

    pOptionAccessorsArePublic =
        new QListWidgetItem(tr2i18n("Accessors are public"), generalOptionsListWidget);
    pOptionAccessorsArePublic->setFlags(flags);

    connect(generalOptionsListWidget,
            SIGNAL(itemClicked(QListWidgetItem *)), this,
            SLOT(generalOptionsListWidgetClicked(QListWidgetItem *)));
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
        m_stringIncludeFileHistoryCombo->setCurrentItem(file, true);
    }
    else if (button == "m_browseListButton") {
        // search for match in history list, if absent, then add it
        m_listIncludeFileHistoryCombo->setCurrentItem(file, true);
    }
}

/**
 * Slot for clicking in the list widget.
 * @param pSender   the sender of the signal, the item in the list
 */
void CPPCodeGenerationForm::generalOptionsListWidgetClicked(QListWidgetItem *pSender)
{
    // operations are inline and accessors are operations :)
    if (pOptionOperationsAreInline->checkState() == Qt::Checked &&
        pOptionGenerateAccessorMethods->checkState() == Qt::Checked) {
        pOptionAccessorsAreInline->setCheckState(Qt::Checked);
    }

    if (pSender == pOptionPackageIsANamespace) {
#if 0
        KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                           "sender=pOptionPackageIsANamespace");
#endif
        return;
    }
    if (pSender == pOptionVirtualDestructors) {
#if 0
        KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                           "sender=pOptionVirtualDestructors");
#endif
        return;
    }
    if (pSender == pOptionGenerateEmptyConstructors) {
#if 0
        KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                           "sender=pOptionVirtualDestructors");
#endif
        return;
    }
    if (pSender == pOptionGenerateAccessorMethods) {
        bool dontGenerateAccessorMethods =
            (pOptionGenerateAccessorMethods->checkState() == Qt::Unchecked);
        pOptionAccessorsAreInline->setHidden(dontGenerateAccessorMethods);
        pOptionAccessorsArePublic->setHidden(dontGenerateAccessorMethods);
        // reset the value if needed
        if (dontGenerateAccessorMethods) {
            pOptionAccessorsAreInline->setCheckState(Qt::Unchecked);
            pOptionAccessorsArePublic->setCheckState(Qt::Unchecked);
        }
#if 0
        KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                           "sender=pOptionGenerateAccessorMethods");
#endif
        return;
    }
    if (pSender == pOptionOperationsAreInline) {
#if 0
        KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                           "sender=pOptionOperationsAreInline");
#endif
        return;
    }
    if (pSender == pOptionAccessorsAreInline) {
#if 0
        KMessageBox::error(0, "CPPCodeGenerationForm::generalOptionsListViewClicked(): "
                           "sender=pOptionAccessorsAreInline");
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
    pOptionPackageIsANamespace->setCheckState(toCheckState(bFlag));
}

/**
 * Set the display state of option "Virtual Destructors".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setVirtualDestructors(bool bFlag)
{
    pOptionVirtualDestructors->setCheckState(toCheckState(bFlag));
}

/**
 * Set the display state of option "Generate Empty Constructors".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setGenerateEmptyConstructors(bool bFlag)
{
    pOptionGenerateEmptyConstructors->setCheckState(toCheckState(bFlag));
}

/**
 * Set the display state of option "Generate Accessor Methods".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setGenerateAccessorMethods(bool bFlag)
{
    pOptionGenerateAccessorMethods->setCheckState(toCheckState(bFlag));
    // initial settings
    pOptionAccessorsAreInline->setHidden(pOptionGenerateAccessorMethods->checkState() == Qt::Unchecked);
    pOptionAccessorsArePublic->setHidden(pOptionGenerateAccessorMethods->checkState() == Qt::Unchecked);
    // reset the value if needed
    if (pOptionGenerateAccessorMethods->checkState() == Qt::Unchecked) {
        pOptionAccessorsAreInline->setCheckState(Qt::Unchecked);
        pOptionAccessorsArePublic->setCheckState(Qt::Unchecked);
    }
}

/**
 * Set the display state of option "Operations Are Inline".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setOperationsAreInline(bool bFlag)
{
    pOptionOperationsAreInline->setCheckState(toCheckState(bFlag));
}

/**
 * Set the display state of option "Accessors Are Inline".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setAccessorsAreInline(bool bFlag)
{
    pOptionAccessorsAreInline->setCheckState(toCheckState(bFlag));
}

/**
 * Set the display state of option "Accessors Are Public".
 * @param bFlag   the flag to set
 */
void CPPCodeGenerationForm::setAccessorsArePublic(bool bFlag)
{
    pOptionAccessorsArePublic->setCheckState(toCheckState(bFlag));
}

/**
 * Get the display state of option "Package Is Namespace".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getPackageIsANamespace()
{
    return pOptionPackageIsANamespace->checkState() == Qt::Checked;
}

/**
 * Get the display state of option "Virtual Destructors".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getVirtualDestructors()
{
    return pOptionVirtualDestructors->checkState() == Qt::Checked;
}

/**
 * Get the display state of option "Generate Empty Constructors".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getGenerateEmptyConstructors()
{
    return pOptionGenerateEmptyConstructors->checkState() == Qt::Checked;
}

/**
 * Get the display state of option "Generate Accessor Methods".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getGenerateAccessorMethods()
{
    return pOptionGenerateAccessorMethods->checkState() == Qt::Checked;
}

/**
 * Get the display state of option "Operations Are Inline".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getOperationsAreInline()
{
    return pOptionOperationsAreInline->checkState() == Qt::Checked;
}

/**
 * Get the display state of option "Accessors Are Inline".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getAccessorsAreInline()
{
    return pOptionAccessorsAreInline->checkState() == Qt::Checked;
}

/**
 * Get the display state of option "Accessors Are Public".
 * @return   the state of the flag
 */
bool CPPCodeGenerationForm::getAccessorsArePublic()
{
    return pOptionAccessorsArePublic->checkState() == Qt::Checked;
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
