/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "floatingtextwidget.h"

// local includes
#include "association.h"
#include "assocpropdlg.h"
#include "classifier.h"
#include "cmds.h"
#include "debug_utils.h"
#include "floatingtextwidgetcontroller.h"
#include "linkwidget.h"
#include "listpopupmenu.h"
#include "messagewidget.h"
#include "model_utils.h"
#include "object_factory.h"
#include "operation.h"
#include "selectopdlg.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"

// kde includes
#include <kinputdialog.h>
#include <klocale.h>

// qt includes
#include <QtCore/QPointer>
#include <QtCore/QRegExp>
#include <QtGui/QPainter>
#include <QtGui/QValidator>

/**
 * Constructs a FloatingTextWidget instance.
 *
 * @param scene The parent of this FloatingTextWidget.
 * @param role The role this FloatingTextWidget will take up.
 * @param text The main text to display.
 * @param id The ID to assign (-1 will prompt a new ID.)
 */
FloatingTextWidget::FloatingTextWidget(UMLScene * scene, Uml::TextRole role, const QString& text, Uml::IDType id)
  : UMLWidget(scene, WidgetBase::wt_Text, id, new FloatingTextWidgetController(this)),
    m_linkWidget(0),
    m_preText(QString()),
    m_postText(QString()),
    m_textRole(role)
{
    m_Text = text;
    UMLWidget::m_resizable = false;
    setZ(10); //make sure always on top.
}

/**
 * Destructor.
 */
FloatingTextWidget::~FloatingTextWidget()
{
}

/**
 * Use to get the _main body_ of text (e.g. prepended and appended
 * text is omitted) as currently displayed by the widget.
 *
 * @return The main text currently being displayed by the widget.
 */
QString FloatingTextWidget::text() const
{
    // test to make sure not just the ":" between the seq number and
    // the actual message widget

    // hmm. this section looks like it could have been avoided by
    // using pre-, post- text instead of storing in the main body of
    // the text -b.t.
    if (m_textRole == Uml::TextRole::Seq_Message || m_textRole == Uml::TextRole::Seq_Message_Self ||
            m_textRole == Uml::TextRole::Coll_Message || m_textRole == Uml::TextRole::Coll_Message_Self) {
        if (m_Text.length() <= 1 || m_Text == ": ")
            return QString();
    }
    return m_Text;
}

/**
 * Set the main body of text to display.
 *
 * @param t The text to display.
 */
void FloatingTextWidget::setText(const QString &t)
{
    if (m_textRole == Uml::TextRole::Seq_Message || m_textRole == Uml::TextRole::Seq_Message_Self) {
        QString seqNum, op;
        m_linkWidget->seqNumAndOp(seqNum, op);
        if (seqNum.length() > 0 || op.length() > 0) {
            if (! m_scene->getShowOpSig())
                op.replace( QRegExp("\\(.*\\)"), "()" );
            m_Text = seqNum.append(": ").append( op );
        }
        else
            m_Text = t;
    }
    else {
        m_Text = t;
    }

    QSize s = minimumSize();
    setSize(s.width(), s.height());

    updateComponentSize();
    update();
}

/**
 * Set some text to be prepended to the main body of text.
 * @param t The text to prepend to main body which is displayed.
 */
void FloatingTextWidget::setPreText(const QString &t)
{
    m_preText = t;
    updateComponentSize();
    update();
}

/**
 * Set some text to be appended to the main body of text.
 * @param t The text to append to main body which is displayed.
 */
void FloatingTextWidget::setPostText(const QString &t)
{
    m_postText = t;
    updateComponentSize();
    update();
}

/**
 * Use to get the total text (prepended + main body + appended)
 * currently displayed by the widget.
 *
 * @return The text currently being displayed by the widget.
 */
QString FloatingTextWidget::displayText() const
{
    QString displayText = m_Text;
    displayText.prepend(m_preText);
    displayText.append(m_postText);
    return displayText;
}

/**
 * Overrides method from UMLWidget.
 */
UMLSceneSize FloatingTextWidget::minimumSize()
{
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    int h = fm.lineSpacing();
    int w = fm.width( displayText() );
    return UMLSceneSize(w + 8, h + 4);  // give a small margin
}

/**
 * Method used by setText: its called by  cmdsetTxt, Don't use it!
 *
 * @param t The text to display.
 */
void FloatingTextWidget::setTextcmd(const QString &t)
{
    UMLApp::app()->executeCommand(new Uml::CmdSetTxt(this,t));
}

/**
 * Displays a dialog box to change the text.
 */
void FloatingTextWidget::showChangeTextDialog()
{
    bool ok = false;
    QString newText = KInputDialog::getText(i18n("Change Text"), i18n("Enter new text:"), text(), &ok, m_scene);

    if (ok && newText != text() && isTextValid(newText)) {
        setText(newText);
        setVisible(!text().isEmpty());
        updateComponentSize();
        update();
    }
    if (!isTextValid(newText))
        hide();
}

/**
 * Shows an operation dialog box.
 */
void FloatingTextWidget::showOperationDialog()
{
    if (!m_linkWidget) {
        uError() << "m_linkWidget is NULL";
        return;
    }
    QString seqNum, opText;
    UMLClassifier* c = m_linkWidget->seqNumAndOp(seqNum, opText);
    if (!c) {
        uError() << "m_linkWidget->seqNumAndOp() returns a NULL classifier";
        return;
    }

    QPointer<SelectOpDlg> selectDlg = new SelectOpDlg(m_scene, c);
    selectDlg->setSeqNumber(seqNum);
    if (m_linkWidget->operation() == 0) {
        selectDlg->setCustomOp(opText);
    } else {
        selectDlg->setClassOp(opText);
    }
    if (selectDlg->exec()) {
        seqNum = selectDlg->getSeqNumber();
        opText = selectDlg->getOpText();
        if (selectDlg->isClassOp()) {
            Model_Utils::OpDescriptor od;
            Model_Utils::Parse_Status st = Model_Utils::parseOperation(opText, od, c);
            if (st == Model_Utils::PS_OK) {
                UMLClassifierList selfAndAncestors = c->findSuperClassConcepts();
                selfAndAncestors.prepend(c);
                UMLOperation *op = 0;
                foreach (UMLClassifier *cl , selfAndAncestors) {
                    op = cl->findOperation(od.m_name, od.m_args);
                    if (op) {
                        break;
                    }
                }
                if (!op) {
                    // The op does not yet exist. Create a new one.
                    UMLObject *o = c->createOperation(od.m_name, 0, &od.m_args);
                    op = static_cast<UMLOperation*>(o);
                }
                if (od.m_pReturnType) {
                    op->setType(od.m_pReturnType);
                }

                m_linkWidget->setOperation(op);
                opText.clear();
            } else {
                m_linkWidget->setOperation(0);
            }
        } else {
            m_linkWidget->setOperation(0);
        }
        m_linkWidget->setSeqNumAndOp(seqNum, opText);
        setMessageText();
    }
    delete selectDlg;
}

/**
 * Show the properties for a FloatingTextWidget.
 * Depending on the role of the floating text wiget, the options dialog
 * for the floating text widget, the rename dialog for floating text or
 * the options dialog for the link widget are shown.
 */
void FloatingTextWidget::showPropertiesDialog()
{
    if (m_textRole == Uml::TextRole::Coll_Message || m_textRole == Uml::TextRole::Coll_Message_Self ||
            m_textRole == Uml::TextRole::Seq_Message || m_textRole == Uml::TextRole::Seq_Message_Self) {
        showOperationDialog();
    } else if (m_textRole == Uml::TextRole::Floating) {
        // double clicking on a text line opens the dialog to change the text
        handleRename();
    } else if (m_linkWidget) {
        m_linkWidget->showPropertiesDialog();
    }
}

/**
 * Use to get the pre-text which is prepended to the main body of
 * text to be displayed.
 *
 * @return The pre-text currently displayed by the widget.
 */
QString FloatingTextWidget::preText() const
{
    return m_preText;
}

/**
 * Use to get the post-text which is appended to the main body of
 * text to be displayed.
 *
 * @return The post-text currently displayed by the widget.
 */
QString FloatingTextWidget::postText() const
{
    return m_postText;
}

/**
 * Activate the FloatingTextWidget after the saved data has been loaded
 *
 * @param ChangeLog Pointer to the IDChangeLog.
 * @return  true for success
 */
bool FloatingTextWidget::activate( IDChangeLog* ChangeLog /*= 0 */)
{
    if (! UMLWidget::activate(ChangeLog))
        return false;
    update();
    return true;
}

/**
 * Set the LinkWidget that this FloatingTextWidget is related to.
 *
 * @param l The related LinkWidget.
 */
void FloatingTextWidget::setLink(LinkWidget * l)
{
    m_linkWidget = l;
}

/**
 * Returns the LinkWidget this floating text is related to.
 *
 * @return The LinkWidget this floating text is related to.
 */
LinkWidget * FloatingTextWidget::link() const
{
    return m_linkWidget;
}

/**
 * Sets the role type of this FloatingTextWidget.
 *
 * @param role  The TextRole of this FloatingTextWidget.
 */
void FloatingTextWidget::setTextRole(Uml::TextRole role)
{
    m_textRole = role;
}

/**
 * Return the role of the text widget
 * @return The TextRole of this FloatingTextWidget.
 */
Uml::TextRole FloatingTextWidget::textRole() const
{
    return m_textRole;
}

/**
 * Handle the ListPopupMenu::mt_Rename case of the slotMenuSelection.
 * Given an own method because it requires rather lengthy code.
 */
void FloatingTextWidget::handleRename()
{
    QRegExpValidator v(QRegExp(".*"), 0);
    QString t;
    if (m_textRole == Uml::TextRole::RoleAName || m_textRole == Uml::TextRole::RoleBName) {
        t = i18n("Enter role name:");
    } else if (m_textRole == Uml::TextRole::MultiA || m_textRole == Uml::TextRole::MultiB) {
        t = i18n("Enter multiplicity:");
        /*
        // NO! shouldn't be allowed
        } else if (m_textRole == Uml::TextRole::ChangeA || m_textRole == Uml::TextRole::ChangeB ) {
        t = i18n("Enter changeability");
        */
    } else if (m_textRole == Uml::TextRole::Name) {
        t = i18n("Enter association name:");
    } else if (m_textRole == Uml::TextRole::Floating) {
        t = i18n("Enter new text:");
    } else {
        t = i18n("ERROR");
    }
    bool ok = false;
    QString newText = KInputDialog::getText(i18n("Rename"), t, text(), &ok, m_scene, &v);
    if (!ok || newText == text()) {
        return;
    }

    UMLApp::app()->executeCommand(new Uml::CmdHandleRename(this, newText));
}

/**
 * Changes the text of linked widget.
 * @param newText   the new text
 */
void FloatingTextWidget::changeName(const QString& newText)
{
    if (m_linkWidget && !isTextValid(newText)) {
        AssociationWidget *assoc = dynamic_cast<AssociationWidget*>(m_linkWidget);
        if (assoc) {
            switch (m_textRole) {
              case Uml::TextRole::MultiA:
                assoc->setMultiplicity(QString(), Uml::A);
                break;
              case Uml::TextRole::MultiB:
                assoc->setMultiplicity(QString(), Uml::B);
                break;
              case Uml::TextRole::RoleAName:
                assoc->setRoleName(QString(), Uml::A);
                break;
              case Uml::TextRole::RoleBName:
                assoc->setRoleName(QString(), Uml::B);
                break;
              case Uml::TextRole::ChangeA:
                assoc->setChangeability(Uml::Changeability::Changeable, Uml::A);
                break;
              case Uml::TextRole::ChangeB:
                assoc->setChangeability(Uml::Changeability::Changeable, Uml::B);
                break;
              default:
                assoc->setName(QString());
                break;
            }
        }
        else {
            MessageWidget *msg = dynamic_cast<MessageWidget*>(m_linkWidget);
            if (msg) {
                msg->setName(QString());
                m_scene->removeWidget(this);
            }
        }
        return;
    }

    if (m_linkWidget && m_textRole != Uml::TextRole::Seq_Message && m_textRole != Uml::TextRole::Seq_Message_Self) {
        m_linkWidget->setText(this, newText);
    }
    else {
        setText(newText);
        UMLApp::app()->document()->setModified(true);
    }

    setVisible(true);
    updateComponentSize();
    update();
}

/**
 * For a text to be valid it must be non-empty, i.e. have a length
 * larger than zero, and have at least one non whitespace character.
 *
 * @param text The string to analyze.
 * @return True if the given text is valid.
 */
bool FloatingTextWidget::isTextValid(const QString &text)
{
    int length = text.length();
    if(length < 1)
        return false;
    for(int i=0;i<length;i++) {
        if(!text.at(i).isSpace()) {
            return true;
        }
    }
    return false;
}

/**
 * Overrides default method
 */
void FloatingTextWidget::paint(QPainter & p, int offsetX, int offsetY)
{
    int w = width();
    int h = height();
    p.setFont( UMLWidget::font() );
    p.setPen(textColor());
    p.drawText( offsetX, offsetY,w,h, Qt::AlignCenter, displayText() );
    if(m_selected)
        drawSelected(&p, offsetX, offsetY);
}

/**
 * Loads the "floatingtext" XMI element.
 */
bool FloatingTextWidget::loadFromXMI(QDomElement & qElement)
{
    if(!UMLWidget::loadFromXMI(qElement))
        return false;

    QString role = qElement.attribute("role", "");
    if(!role.isEmpty())
        m_textRole = Uml::TextRole::Value(role.toInt());

    m_preText = qElement.attribute("pretext", "");
    m_postText = qElement.attribute("posttext", "");
    m_Text = qElement.attribute("text", "");
    // If all texts are empty then this is a useless widget.
    // In that case we return false.
    // CAVEAT: The caller should not interpret the false return value
    //  as an indication of failure since previous umbrello versions
    //  saved lots of these empty FloatingTexts.
    bool usefullWidget = !(m_Text.isEmpty() && m_preText.isEmpty() && m_postText.isEmpty());
    return usefullWidget;
}

/**
 * Reimplemented from UMLWidget::saveToXMI to save the widget
 * data into XMI 'floatingtext' element.
 */
void FloatingTextWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement textElement = qDoc.createElement("floatingtext");
    UMLWidget::saveToXMI(qDoc, textElement);
    textElement.setAttribute("text", m_Text);
    textElement.setAttribute("pretext", m_preText);
    textElement.setAttribute("posttext", m_postText);

    /* No need to save these - the messagewidget already did it.
    m_Operation  = qElement.attribute( "operation", "" );
    m_SeqNum = qElement.attribute( "seqnum", "" );
     */

    textElement.setAttribute("role", m_textRole);
    qElement.appendChild(textElement);
}

/**
 * Called when a menu selection has been made.
 *
 * @param action  The action that has been selected.
 */
void FloatingTextWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);

    switch(sel) {
    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        break;

    case ListPopupMenu::mt_Delete:
        m_scene->removeWidget(this);
        break;

    case ListPopupMenu::mt_New_Operation: // needed by AssociationWidget
    case ListPopupMenu::mt_Operation:
        {
            if (m_linkWidget == 0) {
                uDebug() << "mt_Operation: m_linkWidget is NULL";
                return;
            }
            UMLClassifier* c = m_linkWidget->operationOwner();
            if (c == 0) {
                bool ok = false;
                QString opText = KInputDialog::getText(i18nc("operation name", "Name"),
                                                       i18n("Enter operation name:"),
                                                       text(), &ok, m_scene);
                if (ok)
                    m_linkWidget->setCustomOpText(opText);
                return;
            }
            UMLClassifierListItem* umlObj = Object_Factory::createChildObject(c, UMLObject::ot_Operation);
            if (umlObj) {
                UMLOperation* newOperation = static_cast<UMLOperation*>(umlObj);
                m_linkWidget->setOperation(newOperation);
            }
        }
        break;

    case ListPopupMenu::mt_Select_Operation:
        showOperationDialog();
        break;

    case ListPopupMenu::mt_Rename:
        handleRename();
        break;

    case ListPopupMenu::mt_Change_Font:
        {
            QFont fnt = font();
            if(KFontDialog::getFont(fnt, KFontChooser::NoDisplayFlags, m_scene) ) {
                if(m_textRole == Uml::TextRole::Floating || m_textRole == Uml::TextRole::Seq_Message) {
                    setFont(fnt);
                } else if (m_linkWidget) {
                    m_linkWidget->lwSetFont(fnt);
                }
            }
        }
        break;

    case ListPopupMenu::mt_Reset_Label_Positions:
        if (m_linkWidget)
            m_linkWidget->resetTextPositions();
        break;

    default:
        UMLWidget::slotMenuSelection(action);
        break;
    }//end switch
}

/**
 * Sets the text for this label if it is acting as a sequence diagram
 * message or a collaboration diagram message.
 */
void FloatingTextWidget::setMessageText()
{
    if (m_linkWidget) {
        m_linkWidget->setMessageText(this);
        QSize s = minimumSize();
        setSize(s.width(), s.height());

    }
    setVisible(!text().isEmpty());
}

#include "floatingtextwidget.moc"
