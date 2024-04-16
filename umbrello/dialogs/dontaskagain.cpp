/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "dontaskagain.h"

// Qt includes
#include <QCheckBox>
#include <QGroupBox>
#include <QMetaType>
#include <QVariant>
#include <QVBoxLayout>

// KDE includes
#include <KLocalizedString>
#include <KMessageBox>

Q_DECLARE_METATYPE(DontAskAgainItem*)

DontAskAgainItem::DontAskAgainItem(const QString &name)
  : m_name(name)
{
    DontAskAgainHandler::instance().addItem(this);
}

DontAskAgainItem::~DontAskAgainItem()
{
}

QString &DontAskAgainItem::name()
{
    return m_name;
}

bool DontAskAgainItem::isAll()
{
    return m_name == QStringLiteral("all");
}

bool DontAskAgainItem::isEnabled()
{
    return isAll() ? false : KMessageBox::shouldBeShownContinue(m_name);
}

void DontAskAgainItem::setEnabled(bool state)
{
    if (isAll())
        KMessageBox::enableAllMessages();
    else if(state)
        KMessageBox::enableMessage(m_name);
    else
        KMessageBox::saveDontShowAgainContinue(m_name);
}

DontAskAgainWidget::DontAskAgainWidget(QList<DontAskAgainItem *> &items, QWidget *parent)
  : QWidget(parent),
    m_items(items)
{
    setLayout(new QVBoxLayout(this));
    QGroupBox *box = new QGroupBox(i18n("Notifications"));
    layout()->addWidget(box);
    m_layout = new QVBoxLayout(box);
    for(DontAskAgainItem  *item : m_items) {
        addItem(item);
    }
}

bool DontAskAgainWidget::apply()
{
    // handle 'all messages' case
    for(QCheckBox  *c : this->findChildren<QCheckBox *>()) {
        DontAskAgainItem *item = c->property("data").value<DontAskAgainItem*>();
        if (item->isAll() && c->isChecked()) {
            item->setEnabled();
            return true;
        }
    }
    // handle 'single message' case
    for(QCheckBox  *c : this->findChildren<QCheckBox *>()) {
        DontAskAgainItem *item = c->property("data").value<DontAskAgainItem*>();
        if (!item->isAll() && c->isChecked() ^ item->isEnabled())
            item->setEnabled(c->isChecked());
    }
    return true;
}

void DontAskAgainWidget::setDefaults()
{
    for(QCheckBox  *c : this->findChildren<QCheckBox *>()) {
        DontAskAgainItem *item = c->property("data").value<DontAskAgainItem*>();
        if (item->isAll())
            c->setChecked(true);
        else
            c->setChecked(false);
    }
}

void DontAskAgainWidget::addItem(DontAskAgainItem *item)
{
    QCheckBox *c = new QCheckBox(item->text());
    c->setChecked(item->isEnabled());
    c->setProperty("data", QVariant::fromValue(item));
    connect(c, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));
    m_layout->addWidget(c);
}

void DontAskAgainWidget::slotToggled(bool state)
{
    QCheckBox *c = dynamic_cast<QCheckBox*>(sender());
    if (!c)
        return;

    DontAskAgainItem *item = c->property("data").value<DontAskAgainItem*>();
    if (item->isAll()) {
        for(QCheckBox  *cb : this->findChildren<QCheckBox *>()) {
            if (cb != c)
                cb->setEnabled(!state);
        }
    }
}

void DontAskAgainHandler::addItem(DontAskAgainItem *item)
{
    m_items.append(item);
}

DontAskAgainWidget *DontAskAgainHandler::createWidget()
{
    return new DontAskAgainWidget(m_items);
}

DontAskAgainHandler &DontAskAgainHandler::instance()
{
    static DontAskAgainHandler handler;
    return handler;
}
