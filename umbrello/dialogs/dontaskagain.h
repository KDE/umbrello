/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DONTASKAGAIN_H
#define DONTASKAGAIN_H

// Qt includes
#include <QList>
#include <QString>
#include <QWidget>

// KDE includes
#include <KLocalizedString>

class QVBoxLayout;

/**
 * The DontAskAgainItem class holds a 'dont ask again' item,
 * see @ref DontAskAgainHandler for details.
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 */
class DontAskAgainItem {
public:
    DontAskAgainItem(const QString &name);
    virtual ~DontAskAgainItem();
    QString &name();
    virtual QString text() const = 0;
    bool isAll();
    bool isEnabled();
    void setEnabled(bool state = true);

protected:
    QString m_name;
};

#define DefineDontAskAgainItem(name,key,_text) \
class DontAskAgainItem##name : public DontAskAgainItem \
{ \
public: \
    DontAskAgainItem##name() : DontAskAgainItem(QStringLiteral(key)) {} \
    virtual QString text() const { return _text; } \
}; \
static DontAskAgainItem##name name;

/**
 * The DontAskAgainWidget provides a graphical user interface
 * to change 'dont ask again' settings and could be embedded
 * in dialogs or dialog pages.
 *
 * After adding the widget to a dialog call @ref apply() to apply
 * changed values and call @ref setDefaults() to reset changes to
 * default values, which is all messages enabled.
 *
 * The widget is designed as group box with embedded check boxes.
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 */
class DontAskAgainWidget : public QWidget
{
    Q_OBJECT
public:
    DontAskAgainWidget(QList<DontAskAgainItem *> &items, QWidget *parent = 0);
    bool apply();
    void setDefaults();

protected:
    void addItem(DontAskAgainItem *item);
    QVBoxLayout *m_layout;
    QList<DontAskAgainItem *> &m_items;

protected slots:
    void slotToggled(bool state);
};

/**
 * Dialogs provided by namespace KMessageBox support a feature to hide dialogs on user request
 * by specifying the parameter dontAskAgainName, which adds a checkbox named "Don't ask again"
 * to the related dialog.
 *
 * What is currently missing in KMessageBox namespace and therefore provided by class
 * DontAskAgainHandler, is a widget to reenable or disable dialogs using the "Don't ask again"
 * support in an application setting dialog.
 *
 * To use this support call macro @ref DefineDontAskAgainItem and add a call to method
 * DontAskAgainItem::name as parameter @p dontAskAgainName to related class KMessageBox
 * methods. See the following example:
 *
 * \code{.cpp}
 * DefineDontAskAgainItem(aDontAskAgainItem, "delete-diagram", i18n("Enable 'delete diagram' related messages"));
 * ...
 * return KMessageBox::warningContinueCancel(..., aDontAskAgainItem.name())
 * \endcode
 *
 * To add the mentioned widget to a settings dialog call @ref createWidget() and
 * add the result to a dialog layout.
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 */
class DontAskAgainHandler
{
public:
    void addItem(DontAskAgainItem *item);
    static DontAskAgainHandler &instance();
    DontAskAgainWidget *createWidget();
protected:
    QList<DontAskAgainItem*> m_items;
};

#endif // DONTASKAGAIN_H
