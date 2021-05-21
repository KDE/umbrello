/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "diagramprintpage.h"

// local includes
#include "basictypes.h"
#include "debug_utils.h"
#include "model_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlviewlist.h"

// kde includes
#include <kcombobox.h>
#include <KLocalizedString>

// qt includes
#include <QGroupBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QRadioButton>
#include <QVBoxLayout>

/**
 *  Constructs the diagram print page.
 *  @param parent The parent to the page.
 *  @param doc    The @ref UMLDoc class instance being used.
 */
DiagramPrintPage::DiagramPrintPage(QWidget * parent, UMLDoc * doc)
  : DialogPageBase(parent),
    m_doc(doc)
{
    int margin = fontMetrics().height();
    setWindowTitle(i18n("&Diagrams"));
    QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setMargin(margin);

    m_pFilterGB = new QGroupBox(i18n("Filter"), this);
    mainLayout->addWidget(m_pFilterGB);

    QVBoxLayout * filter = new QVBoxLayout(m_pFilterGB);
    filter->setSpacing(10);
    filter->setMargin(margin);

    m_pCurrentRB = new QRadioButton(i18n("&Current diagram"), m_pFilterGB);
    filter->addWidget(m_pCurrentRB);
    m_pCurrentRB->setChecked(true);

    m_pAllRB = new QRadioButton(i18n("&All diagrams"), m_pFilterGB);
    filter->addWidget(m_pAllRB);

    m_pSelectRB = new QRadioButton(i18n("&Select diagrams"), m_pFilterGB);
    filter->addWidget(m_pSelectRB);

    m_pTypeRB = new QRadioButton(i18n("&Type of diagram"), m_pFilterGB);
    filter->addWidget(m_pTypeRB);

    m_pSelectGB = new QGroupBox(i18nc("diagram selection for printing", "Selection"), this);
    mainLayout->addWidget(m_pSelectGB);

    QVBoxLayout * select = new QVBoxLayout(m_pSelectGB);
    select->setSpacing(10);
    select->setMargin(margin);

    m_pTypeCB = new KComboBox(m_pSelectGB);
    select->addWidget(m_pTypeCB);
    m_pTypeCB->setEnabled(false);

    m_pSelectLW = new QListWidget(m_pSelectGB);
    select->addWidget(m_pSelectLW);
    m_pSelectLW->setEnabled(false);
    m_pSelectLW->setSelectionMode(QAbstractItemView::MultiSelection);
    m_pSelectLW->addItem(UMLApp::app()->currentView()->umlScene()->name());
    m_pSelectLW->setCurrentRow(0);
    m_nIdList.clear();
    m_nIdList.append(UMLApp::app()->currentView()->umlScene()->ID());

    m_ViewType = Uml::DiagramType::Class;
    connect(m_pAllRB, SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(m_pCurrentRB, SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(m_pSelectRB, SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(m_pTypeRB, SIGNAL(clicked()), this, SLOT(slotClicked()));

    connect(m_pTypeCB, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));

    QStringList types;
    // diagramNo 1 is Uml::DiagramType::Class
    // digaramNo 9 is Uml::DiagramType::EntityRelationship
    for (int diagramNo = 1; diagramNo < Uml::DiagramType::N_DIAGRAMTYPES; ++diagramNo) {
        Uml::DiagramType::Enum dt = Uml::DiagramType::fromInt(diagramNo);
        types << Uml::DiagramType::toString(dt);
    }

    m_pTypeCB->insertItems(0, types);
}

/**
 * Standard destructor.
 */
DiagramPrintPage::~DiagramPrintPage()
{
}

/**
 * Get selected print options.
 * @return number of selected items
 */
int DiagramPrintPage::printUmlCount()
{
    QList<QListWidgetItem *> selectedItems = m_pSelectLW->selectedItems();
    return selectedItems.count();
}

/**
 * Return ID string of UML diagram.
 * @param sel   index of selected item
 * @return      ID as string or empty string
 */
QString DiagramPrintPage::printUmlDiagram(int sel)
{
    int count = 0;

    for (int i = 0; i < m_pSelectLW->count(); ++i) {
        if (isSelected(i)) {
            if (count == sel) {
                UMLView *view = (UMLView *)m_doc->findView(m_nIdList[i]);
                QString sID = QString::fromLatin1("%1").arg(Uml::ID::toString(view->umlScene()->ID()));
                return sID;
            }
            count++;
        }
    }
    return QString();
}

/**
 * Overridden method.
 */
bool DiagramPrintPage::isValid(QString& msg)
{
    int listCount = m_pSelectLW->count();
    bool sel = false;
    for (int i = 0; i < listCount; ++i) {
        if (isSelected(i)) {
            sel = true;
            i = listCount;
        }
    }
    msg = i18n("No diagrams selected.");
    return sel;
}

/**
 * Check if item with given index is selected.
 * @param index   index of selected item
 * @return flag whether item is selected
 */
bool DiagramPrintPage::isSelected(int index)
{
    QList<QListWidgetItem *> selectedItems = m_pSelectLW->selectedItems();
    QListWidgetItem* itemAtIndex = m_pSelectLW->item(index);
    if (selectedItems.contains(itemAtIndex)) {
        return true;
    }
    else {
        return false;
    }
}

/**
 * Gets called when the users chooses to print all diagrams, the current
 * diagram, a selection of diagrams or diagrams by type. It will change the
 * listed diagrams in the diagram box.
 */
void DiagramPrintPage::slotClicked()
{
    UMLViewList list = m_doc->viewIterator();
    QString type;

    // clear list with diagrams to print
    m_nIdList.clear();

    UMLScene *currentScene = UMLApp::app()->currentView()->umlScene();
    if (m_pCurrentRB->isChecked()) {
        m_pTypeCB->setEnabled(false);
        m_pSelectLW->setEnabled(false);
        m_pSelectLW->clear();
        m_pSelectLW->addItem(currentScene->name());
        m_pSelectLW->setCurrentRow(0);
        m_nIdList.append(currentScene->ID());
    }

    if (m_pAllRB->isChecked()) {
        m_pTypeCB->setEnabled(false);
        m_pSelectLW->setEnabled(false);
        m_pSelectLW->clear();
        foreach (UMLView * view, list) {
            m_pSelectLW->addItem(view->umlScene()->name());
            m_nIdList.append(view->umlScene()->ID());
        }
        m_pSelectLW->selectAll();
    }

    if (m_pSelectRB->isChecked()) {
        m_pTypeCB->setEnabled(false);
        m_pSelectLW->setEnabled(true);
        m_pSelectLW->clear();
        foreach (UMLView * view, list) {
            m_pSelectLW->addItem(view->umlScene()->name());
            m_nIdList.append(view->umlScene()->ID());
        }
    }

    if (m_pTypeRB->isChecked()) {
        m_pTypeCB->setEnabled(true);
        m_pSelectLW->setEnabled(true);
        m_pSelectLW->clear();
        foreach (UMLView * view, list) {
            if(view->umlScene()->type() == m_ViewType) {
                m_pSelectLW->addItem(view->umlScene()->name());
                m_nIdList.append(view->umlScene()->ID());
            }
        }
        m_pSelectLW->selectAll();
    }
}

/**
 * Gets called when the user chooses another diagram type. Only diagrams of
 * this type will be shown in the diagram box.
 * @param index   diagram type (combo box index)
 */
void DiagramPrintPage::slotActivated(int index)
{
    UMLViewList list = m_doc->viewIterator();

    // combo box entries start from 0 index
    // valid diagram_type enum values start from 1
    m_ViewType = Uml::DiagramType::fromInt(index + 1);

    m_pSelectLW->clear();

    m_nIdList.clear();
    foreach (UMLView * view, list) {
        if (view->umlScene()->type() == m_ViewType) {
            m_pSelectLW->addItem(view->umlScene()->name());
            m_nIdList.append(view->umlScene()->ID());
        }
    }
    m_pSelectLW->selectAll();
}

