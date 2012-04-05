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
#include "linkwidget.h"
#include "listpopupmenu.h"
#include "messagewidget.h"
#include "model_utils.h"
#include "object_factory.h"
#include "operation.h"
#include "selectopdlg.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// kde includes
#include <kinputdialog.h>
#include <klocale.h>

// qt includes
#include <QtCore/QPointer>
#include <QtCore/QRegExp>
#include <QtGui/QValidator>


const UMLSceneValue FloatingTextWidget::restrictPositionMin = 0;
const UMLSceneValue FloatingTextWidget::restrictPositionMax = 3000;

/**
 * Constructs a FloatingTextWidget instance.
 *
 * @param role the role this FloatingTextWidget will take up
 * @param text the main text to display
 * @param id   the ID to assign (-1 will prompt a new ID)
 */
FloatingTextWidget::FloatingTextWidget(Uml::TextRole role, Uml::IDType id)
  : UMLWidget(WidgetBase::wt_Text, id),
    m_linkWidget(0),
    m_text(QString()),
    m_preText(QString()),
    m_postText(QString()),
    m_textRole(role)
{
    setResizable(false);
    // no margin
    setMargin(0);
    createTextItemGroup(); // Create a group to store text.
    setZValue(10); //make sure always on top.
}

/**
 * Destructor.
 */
FloatingTextWidget::~FloatingTextWidget()
{
}

/**
 * Return the text.
 * @return main body text to display
 */
QString FloatingTextWidget::text() const
{
    // test to make sure not just the ":" between the seq number and
    // the actual message widget

    // hmm. this section looks like it could have been avoided by
    // using pre-, post- text instead of storing in the main body of
    // the text -b.t.
    QString n = m_text;
    if (m_textRole == Uml::TextRole::Seq_Message  || m_textRole == Uml::TextRole::Seq_Message_Self ||
        m_textRole == Uml::TextRole::Coll_Message || m_textRole == Uml::TextRole::Coll_Message_Self) {
        if (n.length() <= 1 || n == ": ") {
            return QString();
        }
    }
    return n;
}

/**
 * Set the main body of text to display.
 *
 * @param t The text to display.
 */
void FloatingTextWidget::setText(const QString &t)
{
    bool wasVisible = isVisible();
    QString text;
    if (m_textRole == Uml::TextRole::Seq_Message || m_textRole == Uml::TextRole::Seq_Message_Self) {
        if (m_linkWidget) {
            QString seqNum, op;
            m_linkWidget->seqNumAndOp(seqNum, op);
            if (!seqNum.isEmpty() || !op.isEmpty()) {
                if (umlScene() && ! umlScene()->showOpSig()) {
                    op.replace(QRegExp("\\(.*\\)"), "()");
                }
                text = seqNum.append(": ").append(op);
            }
            else {
                text = t;
            }
        }
        else {
            uError() << "m_linkWidget is NULL!";
        }
    }
    else {
        text = t;
    }

    m_text = text;
    bool makeVisible = isTextValid(m_text);
    // Hide/Show this widget if its <Empty>/<Non Empty>
    setVisible(makeVisible);
    if (makeVisible) {
        updateTextItemGroups();
        if (!wasVisible && m_linkWidget && umlScene()) {
            qreal x = 3000;
            qreal y = 3000;
            m_linkWidget->constrainTextPos(x, y, width(), height(), textRole());
            setPos(QPointF(x, y));
        }
    }
}

/**
 * Set some text to be prepended to the main body of text.
 * @param t The text to prepend to main body which is displayed.
 */
void FloatingTextWidget::setPreText(const QString &t)
{
    m_preText = t;
    updateTextItemGroups();
}

/**
 * Set some text to be appended to the main body of text.
 * @param t The text to apppended to main body which is displayed.
 */
void FloatingTextWidget::setPostText(const QString &t)
{
    m_postText = t;
    updateTextItemGroups();
}

/**
 * Use to get the total text (prepended + main body + appended)
 * currently displayed by the widget.
 */
QString FloatingTextWidget::displayText() const
{
    QString displayText = m_text;
    displayText.prepend(m_preText);
    displayText.append(m_postText);
    return displayText;
}

/**
 * Shows a dialogbox to change text of this widget and also ensures
 * validity of the set text.
 */
void FloatingTextWidget::showChangeTextDialog()
{
    bool ok = false;
    QWidget *parent = umlScene() ? umlScene()->activeView() : 0;
    QString newText = KInputDialog::getText(i18n("Change Text"), i18n("Enter new text:"), text(), &ok, parent);

    if(ok && newText != text() && isTextValid(newText)) {
        setText(newText);
        setVisible(!text().isEmpty());
    }
}

/**
 * Shows a dialogbox for the user to choose an operation, creates a
 * new operation if the user enters a new one and updates linkWidget
 * status.
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
        uError() << "m_linkWidget->getSeqNumAndOp() returns a NULL classifier";
        return;
    }

    QPointer<SelectOpDlg> selectDlg = new SelectOpDlg(umlScene()->activeView(), c);
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
                opText = QString();
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
 *
 * Depending on the role of the floating text wiget, the options
 * dialog for the floating text widget, the rename dialog for floating
 * text or the options dialog for the link widget are shown.
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
 * Sets the link widget linked to this FloatingTextWidget to \a link
 * and also sets the link as its parent.
 *
 * @param link The LinkWidget to be linked (0 for removing link)
 *
 * @note If link is not null, then this floatingwidget is
 *       automatically parented to the corresponding linkwidget item
 *       and hence is made linkWidget's scene's member.
 */
void FloatingTextWidget::setLink(LinkWidget *link)
{
    m_linkWidget = link;
    QGraphicsItem *parent = 0;

    AssociationWidget *assoc = dynamic_cast<AssociationWidget*>(link);
    MessageWidget *msg = dynamic_cast<MessageWidget*>(link);

    if (assoc) {
        parent = static_cast<QGraphicsItem*>(assoc);
    }
    else if (msg) {
        parent = static_cast<QGraphicsItem*>(msg);
    }

    setParentItem(parent);
    updateTextItemGroups();
}

/**
 * Return the link widget.
 * @return the LinkWidget this floating text is related to
 */
LinkWidget* FloatingTextWidget::link() const
{
    return m_linkWidget;
}

/**
 * Sets the role of this FloatingTextWidget to \a role.
 * @param role   the new role
 */
void FloatingTextWidget::setTextRole(Uml::TextRole role)
{
    m_textRole = role;
    updateTextItemGroups();
}

/**
 * Return the text role of this FloatingTextWidget.
 * @return the TextRole of this FloatingTextWidget
 */
Uml::TextRole FloatingTextWidget::textRole() const
{
    return m_textRole;
}

/**
 * Handles renaming based on the text role of this widget.
 */
void FloatingTextWidget::handleRename()
{
    QRegExpValidator v(QRegExp(".*"), 0);
    QString t;
    if(m_textRole == Uml::TextRole::RoleAName || m_textRole == Uml::TextRole::RoleBName) {
        t = i18n("Enter role name:");
    } else if (m_textRole == Uml::TextRole::MultiA || m_textRole == Uml::TextRole::MultiB) {
        t = i18n("Enter multiplicity:");
        /*
        // NO! shouldn't be allowed
        } else if(m_textRole == Uml::TextRole::ChangeA || m_textRole == Uml::TextRole::ChangeB) {
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
    QString newText = KInputDialog::getText(i18n("Rename"), t, text(), &ok,
                                            umlScene()->activeView(), &v);
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
                umlScene()->removeWidget(this);
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
    updateTextItemGroups();
}

/**
 * Checks and returns whether text represented by this FloatingTextWidget is valid
 * or not.
 *
 * @see FloatingTextWidget::isTextValid()
 */
bool FloatingTextWidget::hasValidText() const
{
    return FloatingTextWidget::isTextValid(text());
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
 * Reimplemented from UMLWidget::paint . This method does
 * nothing as the text drawing is handled by TextItemGroup.
 */
void FloatingTextWidget::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    Q_UNUSED(p);
    Q_UNUSED(o);
    Q_UNUSED(w);
}

/**
 * Reimplemented from UMLWidget::loadFromXMI to load
 * FloatingTextWidget from XMI element.
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
    setText(qElement.attribute("text", ""));

    // If all texts are empty then this is a useless widget.
    // In that case we return false.
    // CAVEAT: The caller should not interpret the false return value
    //  as an indication of failure since previous umbrello versions
    //  saved lots of these empty FloatingTexts.
    bool usefullWidget = !(m_text.isEmpty() && m_preText.isEmpty() && m_postText.isEmpty());
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
    textElement.setAttribute("text", text());
    textElement.setAttribute("pretext", m_preText);
    textElement.setAttribute("posttext", m_postText);

    textElement.setAttribute("role", m_textRole);
    qElement.appendChild(textElement);
}

/**
 * Reimplemented from UMLWidget::updateGeometry to calculate
 * minimum size and maximum size.
 */
void FloatingTextWidget::updateGeometry()
{
    TextItemGroup *grp = textItemGroupAt(0);
    QSizeF minSize = grp->minimumSize();
    setMinimumSize(minSize);
    setMaximumSize(minSize);

    UMLWidget::updateGeometry();
}

/**
 * Reimplemented from UMLWidget::updateTextItemGroups to set
 * the text for the TextItem.
 */
void FloatingTextWidget::updateTextItemGroups()
{
    TextItemGroup *grp = textItemGroupAt(0);
    grp->setTextItemCount(1); // only one item to display name

    TextItem *item = grp->textItemAt(0);
    item->setText(displayText());
    item->setExplicitVisibility(true);

    UMLWidget::updateTextItemGroups();
}

/**
 * Reimplemented from UMLWidget::itemChange to notify custom items that some part of
 * the item's state changes.
 */
QVariant FloatingTextWidget::itemChange(GraphicsItemChange change, const QVariant& newValue)
{
    UMLScene *scene = umlScene();
    if (change == ItemPositionChange && scene && scene->isMouseMovingItems()) {
        QPointF newPos = newValue.toPointF();

        qreal x = newPos.x();
        qreal y = newPos.y();
        qreal w = width();
        qreal h = height();

        if (m_linkWidget) {
            m_linkWidget->constrainTextPos(x, y, w, h, textRole());
        }

        newPos = QPointF(x, y);
        return newPos;
    }

    return UMLWidget::itemChange(change, newValue);
}

/**
 * Reimplemented from UMLWidget::attributeChange to react to
 * notification SizeHasChanged.
 */
QVariant FloatingTextWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if (change == SizeHasChanged) {
        TextItemGroup *grp = textItemGroupAt(0);
        grp->setGroupGeometry(QRectF(QPointF(0, 0), grp->minimumSize()));
    }

    return UMLWidget::attributeChange(change, oldValue);
}

/**
 * Event handler for context menu events.
 */
void FloatingTextWidget::contextMenuEvent(UMLSceneContextMenuEvent *event)
{
    AssociationWidget *assoc = dynamic_cast<AssociationWidget*>(m_linkWidget);

    UMLScene *scene = umlScene();
    UMLView *view = scene ? scene->activeView() : 0;
    ListPopupMenu::MenuType menuType = ListPopupMenu::mt_Undefined;

    if (assoc) {
        if (textRole() == Uml::TextRole::MultiA) {
            menuType = ListPopupMenu::mt_MultiA;
        } else if (textRole() == Uml::TextRole::MultiB) {
            menuType = ListPopupMenu::mt_MultiB;
        }
    }

    if (!isSelected() && scene && !scene->selectedItems().isEmpty()) {
        Qt::KeyboardModifiers forSelection = (Qt::ControlModifier | Qt::ShiftModifier);
        if ((event->modifiers() & forSelection) == 0) {
            scene->clearSelection();
        }
    }
    setSelected(true);
    if (menuType != ListPopupMenu::mt_Undefined) {
        QPointer<ListPopupMenu> menu = new ListPopupMenu(view, menuType);
        QAction *triggered = menu->exec(event->screenPos());
        ListPopupMenu *parentMenu = ListPopupMenu::menuFromAction(triggered);

        if (!parentMenu) {
            uError() << "Action's data field does not contain ListPopupMenu pointer";
            return;
        }

        WidgetBase *ownerWidget = parentMenu->ownerWidget();
        // assert because logic is based on only WidgetBase being the owner of 
        // ListPopupMenu actions executed in this context menu.
        Q_ASSERT_X(ownerWidget != 0, "FloatingTextWidget::contextMenuEvent",
                "ownerWidget is null which means action belonging to UMLView, UMLScene"
                " or UMLObject is the one triggered in ListPopupMenu");

        ownerWidget->slotMenuSelection(triggered);

        delete menu.data();
    } else {
        UMLWidget::contextMenuEvent(event);
    }
}

/**
 * Reimplemented from UMLWidget::slotMenuSelection to handle
 * some specific actions.
 */
void FloatingTextWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu *menu = ListPopupMenu::menuFromAction(action);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }
    ListPopupMenu::MenuType sel = menu->getMenuType(action);

    switch(sel) {
    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        break;

    case ListPopupMenu::mt_Delete:
        umlScene()->removeWidget(this);
        break;

    case ListPopupMenu::mt_Operation:
        {
            if (m_linkWidget == 0) {
                uDebug() << "mt_Operation: " << "m_linkWidget is NULL!";
                return;
            }
            UMLClassifier* c = m_linkWidget->operationOwner();
            if (c == 0) {
                bool ok = false;
                QString opText = KInputDialog::getText(i18nc("operation name", "Name"),
                                                       i18n("Enter operation name:"),
                                                       text(), &ok, umlScene()->activeView());
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
            if(KFontDialog::getFont(fnt, KFontChooser::NoDisplayFlags, umlScene()->activeView())) {
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
    }
    setVisible(!text().isEmpty());
}

#include "floatingtextwidget.moc"
