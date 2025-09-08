/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "defaultvaluewidget.h"

#include "enum.h"
#include "umldatatype.h"
#include "uml.h"
#include "umldoc.h"

#include <QLineEdit>
#include <KLocalizedString>

#include <QLabel>
#include <QHBoxLayout>
#include <QListWidget>

class DefaultValueWidget::Private {
public:
    DefaultValueWidget *p;
    QPointer<UMLObject>type;
    QPointer<QLabel> label;
    QPointer<QListWidget> listWidget;
    QPointer<QLineEdit> lineWidget;
    QString initialValue;

    Private(DefaultValueWidget *parent, UMLObject *_type, const QString &_value)
      : p(parent),
        type(_type),
        initialValue(_value)
    {
        QHBoxLayout *layout = new QHBoxLayout;
        layout->setContentsMargins(0,0,0,0);
        label = new QLabel(i18n("&Default value:"), p);
        layout->addWidget(label);

        listWidget = new QListWidget(p);
        layout->addWidget(listWidget, 2);

        lineWidget = new QLineEdit(p);
        layout->addWidget(lineWidget, 2);

        label->setBuddy(listWidget);
        p->setLayout(layout);
        p->setFocusProxy(listWidget);

        setupWidget();
    }

    /**
     * fill list widget with items
     */
    void setupWidget()
    {
        if (type && type->isUMLEnum()) {
            listWidget->clear();
            listWidget->addItem(new QListWidgetItem);
            const UMLEnum *e = type->asUMLEnum();
            UMLClassifierListItemList enumLiterals = e->getFilteredList(UMLObject::ot_EnumLiteral);
            for(UMLClassifierListItem* enumLiteral : enumLiterals) {
                QListWidgetItem *item = new QListWidgetItem(enumLiteral->name());
                listWidget->addItem(item);
            }
            QList<QListWidgetItem*> currentItem = listWidget->findItems(initialValue, Qt::MatchExactly);
            if (currentItem.size() > 0)
                listWidget->setCurrentItem(currentItem.at(0));
        } else {
            lineWidget->setText(initialValue);
        }
        setVisible(true);
    }

    void setVisible(bool state)
    {
        label->setVisible(state);
        if (type && type->isUMLEnum()) {
            listWidget->setVisible(state);
            lineWidget->setVisible(false);
        } else {
            listWidget->setVisible(false);
            lineWidget->setVisible(state);
        }
    }

    void setValue(const QString &value)
    {
        initialValue = value;
    }

    QString value() const
    {
        if (type && type->isUMLEnum()) {
            return listWidget && listWidget->currentItem() ? listWidget->currentItem()->text() : QString();
        } else {
            return lineWidget->text();
        }
    }
};

DefaultValueWidget::DefaultValueWidget(UMLObject *type, const QString &value, QWidget *parent)
  : QWidget(parent),
    m_d(new Private(this, type, value))
{
}

DefaultValueWidget::~DefaultValueWidget()
{
    delete m_d;
}

/**
 * Update widget with new data type
 *
 * @param type type to set widget from
 */
void DefaultValueWidget::setType(UMLObject *type)
{
    if (m_d->type == type)
        return;
    m_d->type = type;
    m_d->setupWidget();
}

/**
 * Update widget with data type from a text string
 *
 * The method searches for a uml object with the specified
 * name. If an object was found, the display of the selectable
 * options depends on the type.
 *
 * @param _type type as text to set the widget from
 */
void DefaultValueWidget::setType(const QString &_type)
{
    UMLObject *type = UMLApp::app()->document()->findUMLObject(_type, UMLObject::ot_UMLObject);
    m_d->type = type;
    m_d->setupWidget();
}

/**
 * Add this widget to a given grid layout. Umbrello dialogs places labels in column 0
 * and the editable field in column 1.
 * @param layout The layout to which the widget should be added
 * @param row The row in the grid layout where the widget should be placed
 */
void DefaultValueWidget::addToLayout(QGridLayout *layout, int row)
{
    layout->addWidget(m_d->label, row, 0);
    layout->addWidget(m_d->listWidget, row, 1);
    layout->addWidget(m_d->lineWidget, row, 1);
}

/**
 * return current text either from list box or from edit field
 * @return current text
 */
QString DefaultValueWidget::value() const
{
    return m_d->value();
}

/**
 * Reimplemented from QWidget to control widgets visible state
 * in case @ref addToLayout() has been called
 *
 * @param event show event
 */
void DefaultValueWidget::showEvent(QShowEvent *event)
{
    m_d->setVisible(true);
    QWidget::showEvent(event);
}

/**
 * Reimplemented from QWidget to control widgets visible state
 * in case @ref addToLayout() has been called
 *
 * @param event hide event
 */
void DefaultValueWidget::hideEvent(QHideEvent *event)
{
    m_d->setVisible(false);
    QWidget::hideEvent(event);
}
