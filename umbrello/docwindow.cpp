/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "docwindow.h"

// local includes
#include "associationwidget.h"
#include "debug_utils.h"
#include "folder.h"
#include "icon_utils.h"
#include "uml.h"  // Only needed for log{Warn,Error}
#include "umldoc.h"
#include "umlobject.h"
#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <QTextEdit>
#include <KLocalizedString>

// qt includes
#include <QCheckBox>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>

/**
 * Displays the 'modified' state of class DocWindow documentation.
 *
 * Also provides left mouse click handler to apply documentation back
 * to the related object/widget.
 */
class ModifiedWidget : public QLabel
{
public:
    ModifiedWidget(DocWindow *_parent)
      : QLabel(_parent),
        parent(_parent)
    {
        setAlignment(Qt::AlignCenter);
        setToolTip(i18n("Flag whether documentation was modified. Press left mouse button to apply modified content."));
    }

    void setModified(bool state)
    {
        if (state)
            setPixmap(Icon_Utils::SmallIcon(Icon_Utils::it_Document_Edit));
        else
            setPixmap(QPixmap());
    }

    virtual void mousePressEvent(QMouseEvent *ev)
    {
        QLabel::mousePressEvent(ev);
        parent->updateDocumentation();
        setPixmap(QPixmap());
    }

    DocWindow *parent;
};

/**
 * Constructor.
 */
DocWindow::DocWindow(UMLDoc * doc, QWidget *parent)
  : QWidget(parent),
    m_pUMLObject(0),
    m_pUMLScene(0),
    m_pUMLDoc(doc),
    m_pUMLWidget(0),
    m_pAssocWidget(0),
    m_Showing(st_Project),
    m_focusEnabled(false)
{
    //setup visual display
    QGridLayout* statusLayout = new QGridLayout();
    m_typeLabel = createPixmapLabel();
    m_typeLabel->setToolTip(i18n("Documentation type"));
    statusLayout->addWidget(m_typeLabel, 0, 0, 1, 1);
    m_nameLabel = new QLabel(this);
    m_nameLabel->setFrameStyle(QFrame::Panel | QFrame::Raised);
    m_nameLabel->setAlignment(Qt::AlignHCenter);
    statusLayout->addWidget(m_nameLabel, 0, 1, 1, 4);
    QCheckBox *box = new QCheckBox();
    box->setToolTip(i18n("Activate documentation edit after focus change."));
    connect(box, SIGNAL(stateChanged(int)), this, SLOT(slotFocusEnabledChanged(int)));
    statusLayout->addWidget(box, 0, 5, 1, 1);
    m_modifiedWidget = new ModifiedWidget(this);
    statusLayout->addWidget(m_modifiedWidget, 0, 6, 1, 1);
    m_docTE = new KTextEdit(this);
    m_docTE->setText(QString());
    setFocusProxy(m_docTE);
    //m_docTE->setWordWrapMode(QTextEdit::WidgetWidth);
    QVBoxLayout* docLayout = new QVBoxLayout(this);
    docLayout->addLayout(statusLayout);
    docLayout->addWidget(m_docTE);
    docLayout->setMargin(0);

    connect(m_docTE, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));
}

/**
 * Destructor.
 */
DocWindow::~DocWindow()
{
}

/**
 * Called when a widget wishes to display its documentation in the
 * doc window.  If there was already documentation there, that will
 * be updated before being removed from the view.
 *
 * Also call this function if you update the documentation in another
 * place, such as a properties dialog. Just set overwrite to true.
 *
 * Overwrite is used when you believe that the documentation window
 * is already displaying documentation for the widget you wish to
 * display.
 * Overwrite just determines whose version is more up to date.
 */
void DocWindow::showDocumentation(UMLObject * object, bool overwrite)
{
    if (!object) {
        reset();
        return;
    }

    if (m_Showing == st_UMLObject && object == m_pUMLObject) {
        if (!overwrite) {
            return;
        }
    }
    else if (m_Showing == st_UMLWidget && object == m_pUMLWidget->umlObject()) {
        if (!overwrite) {
            updateDocumentation();
        }
    }
    else {
        updateDocumentation(true);
    }

    m_Showing = st_UMLObject;
    m_pUMLObject = object;
    m_docTE->setText(m_pUMLObject->doc());
    if (m_pUMLObject->baseType() == UMLObject::ot_Folder) {
        const UMLFolder *folder = m_pUMLObject->asUMLFolder();
        updateLabel(folder->localName());
    }
    else
        updateLabel(m_pUMLObject->name());
    toForeground();
}

/**
 * This method is the same as the one for UMLObjects except it
 * displays documentation for a diagram.
 */
void DocWindow::showDocumentation(UMLScene * scene, bool overwrite)
{
    if (!scene) {
        reset();
        return;
    }

    if (m_Showing == st_UMLScene && scene == m_pUMLScene) {
        if (!overwrite) {
            return;
        }
    }
    else if (m_Showing == st_UMLWidget && scene == m_pUMLWidget->umlScene()) {
        if (!overwrite) {
            updateDocumentation();
        }
    }
    else {
        updateDocumentation(true);
    }

    m_Showing = st_UMLScene;
    m_pUMLScene = scene;
    m_docTE->setText(m_pUMLScene->documentation());
    updateLabel(m_pUMLScene->name());
    toForeground();
}

/**
 * This method is the same as the one for UMLObjects except it
 * displays documentation for an object instance (StateWidget/
 * ObjectWidget).
 */
void DocWindow::showDocumentation(UMLWidget * widget, bool overwrite)
{
    if (!widget) {
        reset();
        return;
    }

    if (m_Showing == st_UMLWidget && widget == m_pUMLWidget) {
        if (!overwrite) {
            return;
        }
    }
    else if (m_Showing == st_UMLObject && widget->umlObject() == m_pUMLObject)
    {
        if (!overwrite) {
            updateDocumentation();
        }
    }
    else if (m_Showing == st_UMLScene && widget->umlScene() == m_pUMLScene)
    {
        if (!overwrite) {
            updateDocumentation();
        }
    }
    else {
        updateDocumentation(true);
    }

    m_Showing = st_UMLWidget;
    m_pUMLWidget = widget;
    m_docTE->setText(m_pUMLWidget->documentation());
    updateLabel(m_pUMLWidget->name());
    toForeground();
}

/**
 * This method is the same as the one for UMLObjects except it
 * displays documentation for an association instance
 * (AssociationWidget).
 */
void DocWindow::showDocumentation(AssociationWidget * widget, bool overwrite)
{
    if (!widget) {
        reset();
        return;
    }
    if (widget == m_pAssocWidget) {
        if (!overwrite) {
            return;
        }
    }
    else {
        updateDocumentation(true);
    }
    m_Showing = st_Association;
    m_pAssocWidget = widget;
    m_docTE->setText(m_pAssocWidget->documentation());
    updateLabel(m_pAssocWidget->name());
    toForeground();
}

/**
 * Call when you wish move changes in the doc window back into the
 * members documentation.
 *
 * If clear is true the doc window will display the documentation
 * for the current project instead of the widget documentation.
 *
 * This is usually called before displaying a properties dialog.
 *
 * @param clear     If true, show the documentation of current project
 * @param startup   If true, no setModified(true) calls will be done and nothing is pushed to the undo stack
 */
void DocWindow::updateDocumentation(bool clear, bool startup)
{
    // the file is marked modified, if the documentation differs
    // we don't do this on startup/load of a xmi file, because every time
    // modified is set, we get another undo/redo backup point
    if (isModified()) {
        if (m_Showing == st_UMLObject && m_pUMLObject) {
            m_pUMLObject->setDoc(m_docTE->toPlainText());
        } else if(m_Showing == st_UMLScene &&  m_pUMLScene) {
            m_pUMLScene->setDocumentation(m_docTE->toPlainText());
        } else if (m_Showing == st_UMLWidget &&  m_pUMLWidget) {
            m_pUMLWidget->setDocumentation(m_docTE->toPlainText());
            m_pUMLWidget->updateGeometry();
        } else if (m_Showing == st_Association &&  m_pAssocWidget) {
            m_pAssocWidget->setDocumentation(m_docTE->toPlainText());
        } else if (m_Showing == st_Project) {
            m_pUMLDoc->setDocumentation(m_docTE->toPlainText());
        } else {
            logError1("DocWindow: Could not update doc due to unknown type and object combination "
                      "(m_Showing=%1)", m_Showing);
        }

        // now do the setModified call
        if (startup == false) {
            m_pUMLDoc->setModified(true);
        }
    }

    // we should show the documentation of the whole project
    if (clear) {
        reset();
    }
}

/**
 *  Re-initializes the class for a new document.
 */
void DocWindow::reset()
{
    m_pUMLScene = 0;
    m_pUMLObject = 0;
    m_pUMLWidget = 0;
    m_pAssocWidget = 0;
    m_Showing = st_Project;
    m_docTE->setText(m_pUMLDoc->documentation());
    updateLabel(m_pUMLDoc->name());
}

/**
 * Checks if the user is typing in the documentation edit window.
 */
bool DocWindow::isTyping() const
{
    if (m_docTE->hasFocus())
        return true;
    else
        return false;
}

void DocWindow::setFocus()
{
    parentWidget()->setVisible(true);
    m_docTE->setFocus();
}

/**
 * Checks if the user is typing in the documentation edit window.
 */
bool DocWindow::isModified() const
{
    bool modified = false;
    const QString currentText = m_docTE->toPlainText();
    QString originalText;
    switch (m_Showing) {
    case st_UMLObject:
        if (m_pUMLObject) {
            originalText = m_pUMLObject->doc();
        }
        break;
    case st_UMLScene:
        if (m_pUMLScene) {
            originalText = m_pUMLScene->documentation();
        }
        break;
    case st_UMLWidget:
        if (m_pUMLWidget) {
            originalText = m_pUMLWidget->documentation();
        }
        break;
    case st_Association:
        if (m_pAssocWidget) {
            originalText = m_pAssocWidget->documentation();
        }
        break;
    case st_Project:
        if (m_pUMLDoc) {
            originalText = m_pUMLDoc->documentation();
        }
        break;
    default:
        break;
    }
    if (QString::compare(originalText, currentText) != 0) {
        modified = true;
    }
    return modified;
}

/**
 * An association was removed from the UMLScene.
 * If the association removed was the association whose documentation is
 * being shown, m_pAssocWidget is set to 0.
 */
void DocWindow::slotAssociationRemoved(AssociationWidget* association)
{
    if (association == m_pAssocWidget || association->umlObject() == m_pUMLObject) {
        // In old code, the below line crashed (bugs.kde.org/89860)
        // A hotfix was made and detailed analysis was To Be Done:
        // reset()
        // However, it seems to have been fixed and the below line seems to work fine
        updateDocumentation(true);
    }
}

/**
 * A widget was removed from the UMLScene.
 * If the association removed was the association which documentation is
 * being shown, m_pUMLWidget is set to 0.
 */
void DocWindow::slotWidgetRemoved(UMLWidget* widget)
{
    if (widget == m_pUMLWidget || widget->umlObject() == m_pUMLObject) {
        updateDocumentation(true);
    }
}

/**
 * text from the edit field has been changed
 */
void DocWindow::slotTextChanged()
{
    updateLabel();
}

/**
 * Set state of focus enabled support.
 */
void DocWindow::slotFocusEnabledChanged(int status)
{
    m_focusEnabled = status == Qt::Checked;
}

/**
 * Updates the info label with the current state.
 * If the given name is empty only the modified icon is set.
 */
void DocWindow::updateLabel(const QString& name)
{
    Icon_Utils::IconType icon = Icon_Utils::it_Home;
    switch (m_Showing) {
    case st_Project:
        icon = Icon_Utils::it_Code_Gen_Wizard;
        break;
    case st_UMLScene:
        icon = Icon_Utils::it_Diagram_Class;
        break;
    case st_UMLObject:
        icon = UMLObject::toIcon(m_pUMLObject->baseType());
        break;
    case st_UMLWidget:
        icon = WidgetBase::toIcon(m_pUMLWidget->baseType());
        break;
    case st_Association:
        icon = Icon_Utils::it_Association;
        break;
    }
    m_typeLabel->setPixmap(Icon_Utils::SmallIcon(icon));
    m_nameLabel->setText(name);
    m_modifiedWidget->setModified(isModified());
}

/**
 * Creates a QLabel used with a pixmap.
 * @return   the just created QLabel
 */
QLabel* DocWindow::createPixmapLabel()
{
    QLabel* label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    return label;
}

/**
 * Bring doc window to foreground if it is tabbed with other dock widgets.
 */
void DocWindow::toForeground()
{
    for(QTabBar  *tab : UMLApp::app()->findChildren<QTabBar *>()) {
        for(int i = 0; i < tab->count(); i++) {
            if (tab->tabText(i) == parentWidget()->windowTitle())
                tab->setCurrentIndex(i);
        }
    }
    if (m_focusEnabled)
        m_docTE->setFocus();
}
