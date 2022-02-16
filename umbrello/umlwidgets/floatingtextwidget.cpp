/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "floatingtextwidget.h"

// local includes
#include "association.h"
#include "associationwidget.h"
#include "associationpropertiesdialog.h"
#include "classifier.h"
#include "cmds.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "linkwidget.h"
#include "classifierwidget.h"
#include "listpopupmenu.h"
#include "messagewidget.h"
#include "model_utils.h"
#include "object_factory.h"
#include "operation.h"
#include "selectoperationdialog.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"

// kde includes
#if QT_VERSION < 0x050000
#include <kfontdialog.h>
#endif
#include <KLocalizedString>

// qt includes
#if QT_VERSION >= 0x050000
#include <QFontDialog>
#endif
#include <QPointer>
#include <QRegExp>
#include <QPainter>
#include <QValidator>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(FloatingTextWidget)

/**
 * Constructs a FloatingTextWidget instance.
 *
 * @param scene The parent of this FloatingTextWidget.
 * @param role The role this FloatingTextWidget will take up.
 * @param text The main text to display.
 * @param id The ID to assign (-1 will prompt a new ID.)
 */
FloatingTextWidget::FloatingTextWidget(UMLScene * scene, Uml::TextRole::Enum role, const QString& text, Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_Text, id),
    m_linkWidget(0),
    m_preText(QString()),
    m_postText(QString()),
    m_textRole(role),
    m_unconstrainedPositionX(0),
    m_unconstrainedPositionY(0),
    m_movementDirectionX(0),
    m_movementDirectionY(0)
{
    m_Text = text;
    m_resizable = false;
    setZValue(10); //make sure always on top.
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
        if (m_Text.length() <= 1 || m_Text == QLatin1String(": "))
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
        QString op;
        if (m_linkWidget)
            op = m_linkWidget->lwOperationText();
        if (op.length() > 0) {
            if (!m_scene->showOpSig())
                op.replace(QRegExp(QLatin1String("\\(.*\\)")), QLatin1String("()"));
            m_Text = op;
        }
        else
            m_Text = t;
    }
    else {
        m_Text = t;
    }

    QSizeF s = minimumSize();
    setSize(s.width(), s.height());

    updateGeometry();
    update();
}

/**
 * Set some text to be prepended to the main body of text.
 * @param t The text to prepend to main body which is displayed.
 */
void FloatingTextWidget::setPreText(const QString &t)
{
    m_preText = t;
    updateGeometry();
    update();
}

/**
 * Set some text to be appended to the main body of text.
 * @param t The text to append to main body which is displayed.
 */
void FloatingTextWidget::setPostText(const QString &t)
{
    m_postText = t;
    updateGeometry();
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
    QString displayText;
    if (!m_SequenceNumber.isEmpty())
        displayText = m_SequenceNumber + QLatin1String(": ") + m_Text;
    else
        displayText = m_Text;
    displayText.prepend(m_preText);
    displayText.append(m_postText);
    return displayText;
}

/**
 * Return state if no pre, post and main text is empty
 * @return true if widget contains no text
 */
bool FloatingTextWidget::isEmpty()
{
    return m_Text.isEmpty() && m_preText.isEmpty() && m_postText.isEmpty();
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF FloatingTextWidget::minimumSize() const
{
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    int h = fm.lineSpacing();
    int w = fm.width(displayText());
    return QSizeF(w + 8, h + 4);  // give a small margin
}

/**
 * Method used by setText: its called by  cmdsetTxt, Don't use it!
 *
 * @param t The text to display.
 */
void FloatingTextWidget::setTextcmd(const QString &t)
{
    UMLApp::app()->executeCommand(new Uml::CmdSetTxt(this, t));
}

/**
 * Displays a dialog box to change the text.
 */
void FloatingTextWidget::showChangeTextDialog()
{
    QString newText = text();
    bool ok = Dialog_Utils::askName(i18n("Change Text"),
                                    i18n("Enter new text:"),
                                    newText);
    if (ok && newText != text() && isTextValid(newText)) {
        setText(newText);
        setVisible(!text().isEmpty());
        updateGeometry();
        update();
    }
    if (!isTextValid(newText))
        hide();
}

/**
 * Shows an operation dialog box.
 *
 * @param enableAutoIncrement Enable auto increment checkbox
 */
bool FloatingTextWidget::showOperationDialog(bool enableAutoIncrement)
{
    if (!m_linkWidget) {
        logError0("FloatingTextWidget::showOperationDialog: m_linkWidget is NULL");
        return false;
    }
    if (!m_linkWidget->lwClassifier()) {
        logError0("FloatingTextWidget::showOperationDialog: m_linkWidget->lwClassifier() returns a NULL classifier");
        return false;
    }
    bool result = false;

    QPointer<SelectOperationDialog> selectDialog = new SelectOperationDialog(m_scene->activeView(), m_linkWidget->lwClassifier(), m_linkWidget, enableAutoIncrement);
    if (selectDialog->exec()) {
        selectDialog->apply();
        setMessageText();
        result = true;
    }
    delete selectDialog;
    return result;
}

/**
 * Show the properties for a FloatingTextWidget.
 * Depending on the role of the floating text widget, the options dialog
 * for the floating text widget, the rename dialog for floating text or
 * the options dialog for the link widget are shown.
 */
bool FloatingTextWidget::showPropertiesDialog()
{
    UMLWidget *p = dynamic_cast<UMLWidget*>(parentItem());
    if (p && p->isInterfaceWidget()) {
        if (p->showPropertiesDialog())
            setText(p->name());
    } else if (m_textRole == Uml::TextRole::Coll_Message || m_textRole == Uml::TextRole::Coll_Message_Self ||
            m_textRole == Uml::TextRole::Seq_Message || m_textRole == Uml::TextRole::Seq_Message_Self) {
        return showOperationDialog(false);
    } else if (m_textRole == Uml::TextRole::Floating) {
        // double clicking on a text line opens the dialog to change the text
        return handleRename();
    } else if (m_linkWidget) {
        return m_linkWidget->showPropertiesDialog();
    }
    return false;
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
bool FloatingTextWidget::activate(IDChangeLog* ChangeLog /*= 0 */)
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
 * @param role  The TextRole::Enum of this FloatingTextWidget.
 */
void FloatingTextWidget::setTextRole(Uml::TextRole::Enum role)
{
    m_textRole = role;
}

/**
 * Return the role of the text widget
 * @return The TextRole::Enum of this FloatingTextWidget.
 */
Uml::TextRole::Enum FloatingTextWidget::textRole() const
{
    return m_textRole;
}

/**
 * Handle the ListPopupMenu::mt_Rename case of the slotMenuSelection.
 * Given an own method because it requires rather lengthy code.
 */
bool FloatingTextWidget::handleRename()
{
    QString t;
    if (m_textRole == Uml::TextRole::RoleAName || m_textRole == Uml::TextRole::RoleBName) {
        t = i18n("Enter role name:");
    } else if (m_textRole == Uml::TextRole::MultiA || m_textRole == Uml::TextRole::MultiB) {
        t = i18n("Enter multiplicity:");
        /*
        // NO! shouldn't be allowed
        } else if (m_textRole == Uml::TextRole::ChangeA || m_textRole == Uml::TextRole::ChangeB) {
        t = i18n("Enter changeability");
        */
    } else if (m_textRole == Uml::TextRole::Name) {
        t = i18n("Enter association name:");
    } else if (m_textRole == Uml::TextRole::Floating) {
        t = i18n("Enter new text:");
    } else {
        t = i18n("ERROR");
    }
    QString newText = text();
    bool ok = Dialog_Utils::askName(i18n("Rename"), t, newText);
    if (!ok || newText == text()) {
        return false;
    }

    UMLApp::app()->executeCommand(new Uml::CmdHandleRename(this, newText));
    return true;
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
                assoc->setMultiplicity(QString(), Uml::RoleType::A);
                break;
              case Uml::TextRole::MultiB:
                assoc->setMultiplicity(QString(), Uml::RoleType::B);
                break;
              case Uml::TextRole::RoleAName:
                assoc->setRoleName(QString(), Uml::RoleType::A);
                break;
              case Uml::TextRole::RoleBName:
                assoc->setRoleName(QString(), Uml::RoleType::B);
                break;
              case Uml::TextRole::ChangeA:
                assoc->setChangeability(Uml::Changeability::Changeable, Uml::RoleType::A);
                break;
              case Uml::TextRole::ChangeB:
                assoc->setChangeability(Uml::Changeability::Changeable, Uml::RoleType::B);
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

    if (m_linkWidget && m_textRole != Uml::TextRole::Seq_Message
                     && m_textRole != Uml::TextRole::Seq_Message_Self) {
        m_linkWidget->setText(this, newText);
    }
    else {
        setText(newText);
        UMLApp::app()->document()->setModified(true);
    }

    setVisible(true);
    updateGeometry();
    update();
}

/**
 * Write property of QString m_SequenceNumber.
 */
void FloatingTextWidget::setSequenceNumber(const QString &sequenceNumber)
{
    m_SequenceNumber = sequenceNumber;
}

/**
 * Read property of QString m_SequenceNumber.
 */
QString FloatingTextWidget::sequenceNumber() const
{
    return m_SequenceNumber;
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
 * Returns a constrained position for the widget after applying the position
 * difference.
 * If no link widget exists, the position returned is the current widget
 * position with the difference applied. If there's a link, the position
 * to be returned is constrained using constrainTextPos method from the
 * LinkWidget, if any.
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 * @return A QPointF with the constrained new position.
 */
QPointF FloatingTextWidget::constrainPosition(qreal diffX, qreal diffY)
{
    qreal newX = x() + diffX;
    qreal newY = y() + diffY;

    if (link()) {
        link()->constrainTextPos(newX, newY, width(), height(), textRole());
    }

    return QPointF(newX, newY);
}

/**
 * Overridden from UMLWidget.
 * Moves the widget to a new position using the difference between the
 * current position and the new position.
 * If the floating text widget is part of a sequence message, and the
 * message widget is selected, it does nothing: the message widget will
 * update the text position when it's moved.
 * In any other case, the floating text widget constrains its move using
 * constrainPosition. When the position of the floating text is constrained,
 * it's kept at that position until it can be moved to another valid
 * position (m_unconstrainedPositionX/Y and m_movementDirectionX/Y are
 * used for that).
 * Moreover, if is part of a sequence message (and the message widget
 * isn't selected), it updates the position of the message widget.
 * @see constrainPosition
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 */
void FloatingTextWidget::moveWidgetBy(qreal diffX, qreal diffY)
{
    if (textRole() == Uml::TextRole::Seq_Message_Self)
        return;

    if (textRole() == Uml::TextRole::Seq_Message
            && link() && dynamic_cast<MessageWidget*>(link())
            && dynamic_cast<MessageWidget*>(link())->isSelected()) {
        return;
    }

    m_unconstrainedPositionX += diffX;
    m_unconstrainedPositionY += diffY;
    QPointF constrainedPosition = constrainPosition(diffX, diffY);

    qreal newX = constrainedPosition.x();
    qreal newY = constrainedPosition.y();

    if (!m_movementDirectionX) {
        if (m_unconstrainedPositionX != constrainedPosition.x()) {
            m_movementDirectionX = (diffX > 0)? 1: -1;
        }
    } else if ((m_movementDirectionX < 0 && m_unconstrainedPositionX > x()) ||
               (m_movementDirectionX > 0 && m_unconstrainedPositionX < x()) ) {
        newX = m_unconstrainedPositionX;
        m_movementDirectionX = 0;
    }

    if (!m_movementDirectionY) {
        if (m_unconstrainedPositionY != constrainedPosition.y()) {
            m_movementDirectionY = (diffY > 0)? 1: -1;
        }
    } else if ((m_movementDirectionY < 0 && m_unconstrainedPositionY > y()) ||
               (m_movementDirectionY > 0 && m_unconstrainedPositionY < y()) ) {
        newY = m_unconstrainedPositionY;
        m_movementDirectionY = 0;
    }

    setX(newX);
    setY(newY);

    if (link()) {
        link()->calculateNameTextSegment();
        if (textRole() == Uml::TextRole::Seq_Message) {
            MessageWidget* messageWidget = (MessageWidget*)link();
            messageWidget->setY(newY + height());
        }
    }
}

/**
 * Overridden from UMLWidget.
 * Modifies the value of the diffX and diffY variables used to move the
 * widgets.
 * The values are constrained using constrainPosition.
 * @see constrainPosition
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 */
void FloatingTextWidget::constrainMovementForAllWidgets(qreal &diffX, qreal &diffY)
{
    QPointF constrainedPosition = constrainPosition(diffX, diffY);

    diffX = constrainedPosition.x() - x();
    diffY = constrainedPosition.y() - y();
}

/**
 * Override method from UMLWidget in order to additionally check widget parentage.
 *
 * @param p Point to be checked.
 *
 * @return 'this' if UMLWidget::onWidget(p) returns non 0;
 *         else NULL.
 */
UMLWidget* FloatingTextWidget::onWidget(const QPointF &p)
{
    WidgetBase *pw = dynamic_cast<WidgetBase*>(parentItem());
    if (pw == 0) {
        return WidgetBase::onWidget(p);
    }
    const WidgetBase::WidgetType t = pw->baseType();
    bool isWidgetChild = false;
    if (t == WidgetBase::wt_Interface) {
        ClassifierWidget *cw = static_cast<ClassifierWidget*>(pw);
        isWidgetChild = cw->getDrawAsCircle();
    } else if (t == wt_Association ||
               t == WidgetBase::wt_Port || t == WidgetBase::wt_Pin) {
        isWidgetChild = true;
    }
    if (!isWidgetChild) {
        return WidgetBase::onWidget(p);
    }
    const qreal w = width();
    const qreal h = height();
    const qreal left = pw->x() + x();
    const qreal right = left + w;
    const qreal top = pw->y() + y();
    const qreal bottom = top + h;
    // uDebug() << "child_of_pw; p=(" << p.x() << "," << p.y() << "), "
    //          << "x=" << left << ", y=" << top << ", w=" << w << ", h=" << h
    //          << "; right=" << right << ", bottom=" << bottom;
    if (p.x() >= left && p.x() <= right &&
        p.y() >= top && p.y() <= bottom) { // Qt coord.sys. origin in top left corner
        // uDebug() << "floatingtext: " << m_Text;
        return this;
    }
    return 0;
}

/**
 * Overrides default method
 */
void FloatingTextWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    int w = width();
    int h = height();
    painter->setFont(UMLWidget::font());
    painter->setPen(textColor());
    painter->drawText(0, 0, w, h, Qt::AlignCenter, displayText());

    UMLWidget::paint(painter, option, widget);
}

/**
 * Loads the "floatingtext" XMI element.
 */
bool FloatingTextWidget::loadFromXMI(QDomElement & qElement)
{
    if(!UMLWidget::loadFromXMI(qElement))
        return false;

    m_unconstrainedPositionX = x();
    m_unconstrainedPositionY = y();
    QString role = qElement.attribute(QLatin1String("role"));
    if(!role.isEmpty())
        m_textRole = Uml::TextRole::fromInt(role.toInt());

    m_preText = qElement.attribute(QLatin1String("pretext"));
    m_postText = qElement.attribute(QLatin1String("posttext"));
    setText(qElement.attribute(QLatin1String("text")));  // use setText for geometry update
    // If all texts are empty then this is a useless widget.
    // In that case we return false.
    // CAVEAT: The caller should not interpret the false return value
    //  as an indication of failure since previous umbrello versions
    //  saved lots of these empty FloatingTexts.
    bool usefulWidget = !isEmpty();
    return usefulWidget;
}

/**
 * Reimplemented from UMLWidget::saveToXMI to save the widget
 * data into XMI 'floatingtext' element.
 */
void FloatingTextWidget::saveToXMI(QXmlStreamWriter& writer)
{
    if (isEmpty())
        return;

    writer.writeStartElement(QLatin1String("floatingtext"));
    UMLWidget::saveToXMI(writer);
    writer.writeAttribute(QLatin1String("text"), m_Text);
    writer.writeAttribute(QLatin1String("pretext"), m_preText);
    writer.writeAttribute(QLatin1String("posttext"), m_postText);

    /* No need to save these - the messagewidget already did it.
    m_Operation  = qElement.attribute("operation");
    m_SeqNum = qElement.attribute("seqnum");
     */

    writer.writeAttribute(QLatin1String("role"), QString::number(m_textRole));
    writer.writeEndElement();
}

/**
 * Called when a menu selection has been made.
 *
 * @param action  The action that has been selected.
 */
void FloatingTextWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        break;

    case ListPopupMenu::mt_Delete:
        hide();   // This is only a workaround; if set then m_linkWidget should
                  // really delete this FloatingTextWidget.
        update();
        m_scene->removeWidget(this);
        break;

    case ListPopupMenu::mt_New_Operation: // needed by AssociationWidget
    case ListPopupMenu::mt_Operation:
        {
            if (m_linkWidget == 0) {
                logDebug0("FloatingTextWidget::slotMenuSelection(mt_Operation): m_linkWidget is NULL");
                return;
            }
            UMLClassifier* c = m_linkWidget->operationOwner();
            if (c == 0) {
                QString opText = text();
                bool ok = Dialog_Utils::askName(i18nc("operation name", "Name"),
                                                i18n("Enter operation name:"),
                                                opText);
                if (ok)
                    m_linkWidget->setCustomOpText(opText);
                return;
            }
            UMLClassifierListItem* umlObj = Object_Factory::createChildObject(c, UMLObject::ot_Operation);
            if (umlObj) {
                UMLOperation* newOperation = umlObj->asUMLOperation();
                m_linkWidget->setOperation(newOperation);
            }
        }
        break;

    case ListPopupMenu::mt_Select_Operation:
        showOperationDialog(false);
        break;

    case ListPopupMenu::mt_Rename:
        handleRename();
        break;

    case ListPopupMenu::mt_Change_Font:
        {
#if QT_VERSION >= 0x050000
            bool ok = false;
            QFont fnt = QFontDialog::getFont(&ok, font(), m_scene->activeView());
            if (ok) {
#else
            QFont fnt = font();
            if(KFontDialog::getFont(fnt, KFontChooser::NoDisplayFlags, m_scene->activeView())) {
#endif
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
        QSizeF s = minimumSize();
        setSize(s.width(), s.height());

    }
    setVisible(!text().isEmpty());
}

