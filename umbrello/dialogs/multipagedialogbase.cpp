/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "multipagedialogbase.h"

// local includes
#include "associationgeneralpage.h"
#include "associationrolepage.h"
#include "associationwidget.h"
#include "debug_utils.h"
#include "icon_utils.h"
#include "notepage.h"
#include "messagewidget.h"
#include "uml.h"
#include "selectoperationpage.h"
#include "umlwidget.h"
#include "umlwidgetstylepage.h"

#include <KFontChooser>
#include <KHelpClient>
#include <KLocalizedString>
#include <KPageDialog>
#include <KPageWidget>

// qt includes
#include <QApplication>
#include <QDockWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>

DEBUG_REGISTER(MultiPageDialogBase)

/**
 * Constructor
 */
MultiPageDialogBase::MultiPageDialogBase(QWidget *parent, bool withDefaultButton)
  : QWidget(parent),
    m_pAssocGeneralPage(nullptr),
    m_notePage(nullptr),
    m_operationGeneralPage(nullptr),
    m_pRolePage(nullptr),
    m_fontChooser(nullptr),
    m_pStylePage(nullptr),
    m_pageItem(nullptr),
    m_pageDialog(nullptr),
    m_pageWidget(nullptr),
    m_useDialog(!parent || strcmp(parent->metaObject()->className(),"PropertiesWindow") != 0),
    m_isModified(false)
{
    if (m_useDialog) {
        m_pageDialog = new KPageDialog(parent);
        m_pageDialog->setModal(true);
        m_pageDialog->setFaceType(KPageDialog::List);
        m_pageDialog->setStandardButtons(QDialogButtonBox::Ok |
                                         QDialogButtonBox::Apply |
                                         QDialogButtonBox::Cancel |
                                         QDialogButtonBox::Help);
        QDialogButtonBox * dlgButtonBox = m_pageDialog->findChild<QDialogButtonBox*>(QStringLiteral("buttonbox"));
        if (withDefaultButton) {
            QPushButton *defaultButton = new QPushButton(i18n("Default"));
            m_pageDialog->addActionButton(defaultButton);
            connect(defaultButton, SIGNAL(clicked()), this, SLOT(slotDefaultClicked()));
        }
        connect(dlgButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(slotButtonClicked(QAbstractButton*)));
    } else {
        m_pageWidget = new KPageWidget(this);
        m_pageWidget->setFaceType(KPageView::Tree);
    }
}

MultiPageDialogBase::~MultiPageDialogBase()
{
    delete m_pageDialog;
    delete m_pageWidget;
}

void MultiPageDialogBase::slotEnableButtonOk(bool state)
{
    m_pageDialog->button(QDialogButtonBox::Ok)->setEnabled(state);
    m_pageDialog->button(QDialogButtonBox::Apply)->setEnabled(state);
}

/**
 * Apply all used pages
 */
void MultiPageDialogBase::apply()
{
    if (m_pAssocGeneralPage)
        m_pAssocGeneralPage->apply();

    if (m_notePage)
        m_notePage->apply();

    if (m_operationGeneralPage)
        m_operationGeneralPage->apply();

    if (m_pRolePage) {
        applyAssociationRolePage();
    }

    if (m_pStylePage) {
        applyStylePage();
    }

    //TODO include applying font settings data
}

void MultiPageDialogBase::setCaption(const QString &caption)
{
    if (m_pageDialog)
        m_pageDialog->setWindowTitle(caption);
}

void MultiPageDialogBase::accept()
{
    if (m_pageDialog)
        m_pageDialog->accept();
}

void MultiPageDialogBase::reject()
{
    if (m_pageDialog)
        m_pageDialog->reject();
}

KPageWidgetItem *MultiPageDialogBase::currentPage() const
{
    if (m_pageDialog)
        return m_pageDialog->currentPage();
    else
        return m_pageWidget->currentPage();
}

void MultiPageDialogBase::addPage(KPageWidgetItem *page)
{
    if (m_pageDialog)
        m_pageDialog->addPage(page);
    else
        m_pageWidget->addPage(page);
}

/**
 * Set current page.
 *
 * @param page the page to set
 */
void MultiPageDialogBase::setCurrentPage(KPageWidgetItem *page)
{
    if (m_pageDialog)
        m_pageDialog->setCurrentPage(page);
    else
        m_pageWidget->setCurrentPage(page);
}

int MultiPageDialogBase::exec()
{
    if (m_pageDialog)
        return m_pageDialog->exec();
    else {
        return 0;
    }
}

/**
 * Return state if any data has been changed in the dialog.
 *
 * @return true data has been changed
 */
bool MultiPageDialogBase::isModified() const
{
    return m_isModified;
}

/**
 * Handle click on ok button.
 */
void MultiPageDialogBase::slotOkClicked()
{
    Q_EMIT okClicked();
}

/**
 * Handle click on apply button.
 */
void MultiPageDialogBase::slotApplyClicked()
{
    Q_EMIT applyClicked();
}

/**
 * Handle click on default button, if enabled in constructor.
 */
void MultiPageDialogBase::slotDefaultClicked()
{
    Q_EMIT defaultClicked();
}

/**
 * Launch khelpcenter.
 */
void MultiPageDialogBase::slotHelpClicked()
{
    logDebug0("MultiPageDialogBase::slotHelpClicked is handled directly");
    KHelpClient::invokeHelp(QStringLiteral("settings"), QStringLiteral("umbrello"));
}

/**
 * Button clicked event handler for the dialog button box.
 * @param button  the button which was clicked
 */
void MultiPageDialogBase::slotButtonClicked(QAbstractButton *button)
{
    if (button == (QAbstractButton*)m_pageDialog->button(QDialogButtonBox::Apply)) {
        logDebug0("MultiPageDialogBase::slotButtonClicked: APPLY...");
        slotApplyClicked();
    }
    else if (button == (QAbstractButton*)m_pageDialog->button(QDialogButtonBox::Ok)) {
        logDebug0("MultiPageDialogBase::slotButtonClicked: OK...");
        slotOkClicked();
    }
    else if (button == (QAbstractButton*)m_pageDialog->button(QDialogButtonBox::Cancel)) {
        logDebug0("MultiPageDialogBase::slotButtonClicked: CANCEL...");
    }
    else if (button == (QAbstractButton*)m_pageDialog->button(QDialogButtonBox::Help)) {
        logDebug0("MultiPageDialogBase::slotButtonClicked: HELP...");
        slotHelpClicked();
    }
    else {
        logDebug0("MultiPageDialogBase::slotButtonClicked: unhandled button role");
    }
}

/**
 * Handle key press event.
 *
 * @param event key press event
 */
void MultiPageDialogBase::keyPressEvent(QKeyEvent *event)
{
    // Set modified state if any text has been typed in
    if (event->key() >= Qt::Key_Space
            && event->key() < Qt::Key_Multi_key)
        m_isModified = true;

    QWidget::keyPressEvent(event);
}

/**
 * Create a property page
 * @param name   The Text displayed in the page list
 * @param header The Text displayed above the page
 * @param icon  The icon to display in the page list
 * @return Pointer to created frame
 */
QFrame* MultiPageDialogBase::createPage(const QString& name, const QString& header, Icon_Utils::IconType icon)
{
    QFrame* page = new QFrame();
    m_pageItem = new KPageWidgetItem(page, name);
    if (!m_pageWidget) {
        m_pageItem->setHeader(header);
        m_pageItem->setIcon(QIcon(Icon_Utils::DesktopIcon(icon)));
    } else
        m_pageItem->setHeader(QString());
    addPage(m_pageItem);
    //page->setMinimumSize(310, 330);
    return page;
}

/**
 * create new page using a dedicated widget
 * @param name   The Text displayed in the page list
 * @param header The Text displayed above the page
 * @param icon  The icon to display in the page list
 * @param widget Widget to display in the page
 * @return page widget item instance
 */
KPageWidgetItem *MultiPageDialogBase::createPage(const QString& name, const QString& header, Icon_Utils::IconType icon, QWidget *widget)
{
    QFrame* page = createPage(name, header, icon);
    QHBoxLayout * topLayout = new QHBoxLayout(page);
    widget->setParent(page);
    topLayout->addWidget(widget);
    return m_pageItem;
}

/**
 * Sets up the general settings page.
 * @param widget The widget to load the initial data from
 */
void MultiPageDialogBase::setupGeneralPage(AssociationWidget *widget)
{
    QFrame *page = createPage(i18nc("general settings", "General"), i18n("General Settings"), Icon_Utils::it_Properties_General);
    QHBoxLayout *layout = new QHBoxLayout(page);
    m_pAssocGeneralPage = new AssociationGeneralPage (page, widget);
    layout->addWidget(m_pAssocGeneralPage);
}

/**
 * Sets up the general page for operations
 * @param widget The widget to load the initial data from
 */
KPageWidgetItem *MultiPageDialogBase::setupGeneralPage(MessageWidget *widget)
{
    m_operationGeneralPage = new SelectOperationPage(widget->umlScene()->activeView(), widget->lwClassifier(), widget);
    connect(m_operationGeneralPage, SIGNAL(enableButtonOk(bool)), this, SLOT(slotEnableButtonOk(bool)));
    return createPage(i18nc("general settings", "General"), i18n("General Settings"),
                      Icon_Utils::it_Properties_General, m_operationGeneralPage);
}

/**
 * Sets up the general settings page.
 * @param widget The widget to load the initial data from
 */
void MultiPageDialogBase::setupGeneralPage(NoteWidget *widget)
{
    QFrame *page = createPage(i18nc("general settings", "General"), i18n("General Settings"), Icon_Utils::it_Properties_General);
    QHBoxLayout *layout = new QHBoxLayout(page);
    m_notePage = new NotePage (page, widget);
    layout->addWidget(m_notePage);
}

/**
 * Sets up the font selection page.
 * @param font The font to load the initial data from
 */
KPageWidgetItem *MultiPageDialogBase::setupFontPage(const QFont &font)
{
    m_fontChooser = new KFontChooser(KFontChooser::NoDisplayFlags, this);
    m_fontChooser->enableColumn(KFontChooser::StyleList, false);
    m_fontChooser->setFont(font);
    return createPage(i18n("Font"), i18n("Font Settings"),
                      Icon_Utils::it_Properties_Font, m_fontChooser);
}

/**
 * Sets up the font selection page.
 * @param widget The widget to load the initial data from
 */
KPageWidgetItem *MultiPageDialogBase::setupFontPage(UMLWidget *widget)
{
    return setupFontPage(widget->font());
}

/**
 * Sets up the font selection page.
 * @param widget The widget to load the initial data from
 */
KPageWidgetItem *MultiPageDialogBase::setupFontPage(AssociationWidget *widget)
{
    return setupFontPage(widget->font());
}

/**
 * Set the font page to show the font from the given widget
 * @param widget
 */
void MultiPageDialogBase::resetFontPage(QWidget *widget)
{
    Q_ASSERT(m_fontChooser);
    m_fontChooser->setFont(widget->font());
}

/**
 * updates the font page data
 * @param widget Widget to save the font data into
 */
void MultiPageDialogBase::applyFontPage(AssociationWidget *widget)
{
    Q_ASSERT(m_fontChooser);
    widget->lwSetFont(m_fontChooser->font());
}

void MultiPageDialogBase::applyFontPage(Settings::OptionState *state)
{
    Q_ASSERT(m_fontChooser);
    state->uiState.font = m_fontChooser->font();
}

/**
 * updates the font page data
 * @param scene Scene to save the font data into
 */
void MultiPageDialogBase::applyFontPage(UMLScene *scene)
{
    Q_ASSERT(m_fontChooser);
    scene->setFont(m_fontChooser->font(), true);
}

/**
 * updates the font page data
 * @param widget Widget to save the font data into
 */
void MultiPageDialogBase::applyFontPage(UMLWidget *widget)
{
    Q_ASSERT(m_fontChooser);
    widget->setFont(m_fontChooser->font());
}

/**
 * Sets up the style page.
 * @param widget The widget to load the initial data from
 */
KPageWidgetItem *MultiPageDialogBase::setupStylePage(WidgetBase *widget)
{
    m_pStylePage = new UMLWidgetStylePage(nullptr, widget);
    return createPage(i18nc("widget style page", "Style"), i18n("Widget Style"),
                      Icon_Utils::it_Properties_Color, m_pStylePage);
}

/**
 * Sets up the style page.
 * @param widget The widget to load the initial data from
 */
KPageWidgetItem *MultiPageDialogBase::setupStylePage(AssociationWidget *widget)
{
    m_pStylePage = new UMLWidgetStylePage(nullptr, widget);
    return createPage(i18nc("style page name", "Style"), i18n("Line Style"),
                      Icon_Utils::it_Properties_Color, m_pStylePage);
}

/**
 * Updates the style page.
 */
void MultiPageDialogBase::applyStylePage()
{
    Q_ASSERT(m_pStylePage);
    m_pStylePage->apply();
}

/**
 * Sets up the role settings page.
 * @param widget The widget to load the initial data from
 */
KPageWidgetItem *MultiPageDialogBase::setupAssociationRolePage(AssociationWidget *widget)
{
    m_pRolePage = new AssociationRolePage(nullptr, widget);
    return createPage(i18nc("role page name", "Roles"), i18n("Role Settings"),
                      Icon_Utils::it_Properties_Roles, m_pRolePage);
}

/**
 * Save all used pages
 */
void MultiPageDialogBase::applyAssociationRolePage()
{
    Q_ASSERT(m_pRolePage);
    m_pRolePage->apply();
}
