/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef FLOATINGTEXTWIDGET_H
#define FLOATINGTEXTWIDGET_H

#include "basictypes.h"
#include "umlwidget.h"

class LinkWidget;
class UMLScene;

/**
 * @short Displays a line of text or an operation.
 *
 * This is a multipurpose class. In its simplest form it will display a
 * line of text.
 * It can also be setup to be the text for an operation with regard to the
 * @ref MessageWidget on the sequence diagram.
 * It is also used for the text required for an association.
 *
 * The differences between all these different uses will be the popup menu
 * that is associated with it.
 *
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class FloatingTextWidget : public UMLWidget
{
    Q_OBJECT
public:
    explicit FloatingTextWidget(UMLScene * scene, Uml::TextRole::Enum role = Uml::TextRole::Floating,
                                const QString& text = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~FloatingTextWidget();

    QString text() const;
    void setText(const QString &t);

    void setTextcmd(const QString &t);

    QString preText() const;
    void setPreText(const QString &t);

    QString postText() const;
    void setPostText(const QString &t);

    QString displayText() const;

    bool isEmpty();

    void showChangeTextDialog();
    bool showOperationDialog(bool enableAutoIncrement = true);
    virtual bool showPropertiesDialog();

    LinkWidget* link() const;
    void setLink(LinkWidget * l);

    bool activate(IDChangeLog *ChangeLog = nullptr);

    Uml::TextRole::Enum textRole() const;
    void setTextRole(Uml::TextRole::Enum role);

    bool handleRename();
    void changeName(const QString& newText);

    void setSequenceNumber(const QString &sequenceNumber);
    QString sequenceNumber() const;

    static bool isTextValid(const QString &text);

    UMLWidget* onWidget(const QPointF& p);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    virtual void saveToXMI(QXmlStreamWriter& writer);
    virtual bool loadFromXMI(QDomElement& qElement);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);
    void setMessageText();

protected:
    QSizeF minimumSize() const;

    virtual void moveWidgetBy(qreal diffX, qreal diffY);
    virtual void constrainMovementForAllWidgets(qreal &diffX, qreal &diffY);

private:
    QPointF constrainPosition(qreal diffX, qreal diffY);

    /// The association or message widget we may be linked to.
    LinkWidget * m_linkWidget;

    //////////////////// Data loaded/saved:

    /// Prepended text (such as for scope of association Role or method)
    QString m_preText;
    /// Ending text (such as bracket on changability notation for association Role)
    QString m_postText;
    /// The role the text widget will enact.
    Uml::TextRole::Enum m_textRole;

    ////////

    /// The horizontal position the widget would have if its move wasn't constrained.
    qreal m_unconstrainedPositionX;

    /// The vertical position the widget would have if its move wasn't constrained.
    qreal m_unconstrainedPositionY;

    /// The X direction the widget was moved when the constrain was applied.
    /// -1 means left, 1 means right.
    int m_movementDirectionX;

    /// The Y direction the widget was moved when the constrain was applied.
    /// -1 means up, 1 means down.
    int m_movementDirectionY;

    /// Contains sequence number for sequence or collaboration diagram message.
    QString m_SequenceNumber;
};

#endif
