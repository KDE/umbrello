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
#include "statewidget.h"

// app includes
#include "debug_utils.h"
#include "listpopupmenu.h"
#include "statedialog.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

// qt includes
#include <QPointer>

const QSizeF StateWidget::MinimumEllipseSize(50, 30);

/**
 * Creates a State widget.
 * @param stateType The type of state.
 * @param id The ID to assign (-1 will prompt a new ID.)
 */
StateWidget::StateWidget(StateType stateType, Uml::IDType id)
  : UMLWidget(WidgetBase::wt_State, id),
    m_stateType(stateType),
    m_drawVertical(true)
{
    createTextItemGroup();
    updateGeometry();
}

/**
 * Destructor.
 */
StateWidget::~StateWidget()
{
}

/**
 * Reimplemented from UMLWidget::paint to paint state widget.
 */
void StateWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    switch (m_stateType) {
    case StateWidget::Normal:
        {
            painter->drawRoundedRect(rect(), 15, 15);
            painter->drawLines(m_separatorLines);
        }
        break;
    case StateWidget::Initial:
        {
            painter->drawEllipse(rect());
        }
        break;
    case StateWidget::End:
        {
            // Draw inner ellipse with brush set.
            QRectF inner(rect());
            qreal adj = lineWidth() + 3;
            inner.adjust(+adj, +adj, -adj, -adj);
            painter->drawEllipse(inner);
            // Now draw outer ellipse with no brush
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(rect());
        }
        break;
    case StateWidget::Fork:
    case StateWidget::Join:
        {
            painter->setPen(Qt::black);
            painter->setBrush(Qt::black);
            painter->drawRect(rect());
        }
        break;
    case StateWidget::Junction:
        {
            painter->setPen(Qt::black);
            painter->setBrush(Qt::black);
            painter->drawEllipse(rect());
        }
        break;
    case StateWidget::DeepHistory:
        {
            painter->setBrush(Qt::white);
            painter->drawEllipse(rect());
            painter->setPen(Qt::black);
            painter->drawText(3, 13, "H");
            painter->drawText(11, 12, "*");
        }
        break;
    case StateWidget::ShallowHistory:
        {
            painter->setBrush(Qt::white);
            painter->drawEllipse(rect());
            painter->setPen(Qt::black);
            painter->drawText(5, 13, "H");
        }
        break;
    case StateWidget::Choice:
        {
            const qreal len = 25.0;
            const qreal pnt = len / 2.0;
            QPolygonF polygon;
            polygon << QPointF(pnt, 0) << QPointF(len, pnt) << QPointF(pnt, len) << QPointF(0, pnt);
            painter->drawPolygon(polygon);
        }
        break;
    default:
        uWarning() << "Unknown state type: " << stateTypeStr();
        break;
    }
}

/**
 * Returns the type of state.
 * @return StateType
 */
StateWidget::StateType StateWidget::stateType() const
{
    return m_stateType;
}

/**
 * Returns the type string of state.
 */
QString StateWidget::stateTypeStr() const
{
    return QLatin1String(ENUM_NAME(StateWidget, StateType, m_stateType));
}

/**
 * Sets the type of state.
 */
void StateWidget::setStateType(StateType stateType)
{
    m_stateType = stateType;
    updateTextItemGroups();
}

/**
 * Adds an activity to this widget.
 * @return true on success
 */
bool StateWidget::addActivity(const QString &activity)
{
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    grp->appendTextItem(new TextItem(activity));
    updateTextItemGroups();
    return true;
}

/**
 * Removes an \a activity from this widget if it exists
 * @return true on success
 */
bool StateWidget::removeActivity(const QString &activity)
{
    int removedCount = 0;
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    // Keep removing until all entries are removed
    while (1) {
        int sz = grp->textItemCount();
        int i = StateWidget::ActivityStartIndex;
        for (; i < sz; ++i) {
            if (grp->textItemAt(i)->text() == activity) {
                grp->deleteTextItemAt(i);
                ++removedCount;
                break;
            }
        }
        if (i == sz) {
            break;
        }
    }

    if (removedCount) {
        updateTextItemGroups();
        return true;
    }

    return false;
}

/**
 * Renames an \a activity to \a newName.
 * @return true on success.
 */
bool StateWidget::renameActivity(const QString &activity, const QString &newName)
{
    bool renamed = false;
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    int sz = grp->textItemCount();
    for (int i = StateWidget::ActivityStartIndex; i < sz; ++i) {
        TextItem *item = grp->textItemAt(i);
        if (item->text() == activity) {
            item->setText(newName);
            renamed = true;
            break;
        }
    }

    if (renamed) {
        updateTextItemGroups();
        return true;
    }
    return false;
}

/**
 * Sets the activities from \a list.
 */
void StateWidget::setActivities(const QStringList &list)
{
    const int reqdSize = list.size() + 1; // + 1 for name item
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    grp->setTextItemCount(reqdSize);

    for (int i = StateWidget::ActivityStartIndex; i < reqdSize; ++i) {
        grp->textItemAt(i)->setText(list[i-StateWidget::ActivityStartIndex]);
    }

    updateGeometry();
}

/**
 * @return The activity texts as a QStringList.
 */
QStringList StateWidget::activities() const
{
    QStringList retVal;
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    int sz = grp->textItemCount();
    for (int i = StateWidget::ActivityStartIndex; i < sz; ++i) {
        retVal << grp->textItemAt(i)->text();
    }
    return retVal;
}

/**
 * Get whether to draw a fork or join vertically.
 */
bool StateWidget::drawVertical() const
{
    return m_drawVertical;
}

/**
 * Set whether to draw a fork or join vertically.
 */
void StateWidget::setDrawVertical(bool to)
{
    m_drawVertical = to;
    updateGeometry();
}

/**
 * Reimplemented from WidgetBase::showPropertiesDialog to show
 * appropriate dialog for this widget.
 */
void StateWidget::showPropertiesDialog()
{
    umlScene()->updateDocumentation(false);

    QPointer<StateDialog> dialog = new StateDialog(umlScene()->activeView(), this);
    if (dialog->exec() && dialog->getChangesMade()) {
        umlScene()->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
    }
    delete dialog;
}

/**
 * Reimplemented from WidgetBase::saveToXMI to save StateWidget info
 * into XMI.
 */
void StateWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement stateElement = qDoc.createElement("statewidget");
    UMLWidget::saveToXMI(qDoc, stateElement);

    stateElement.setAttribute("statename", name());
    stateElement.setAttribute("documentation", documentation());
    stateElement.setAttribute("statetype", m_stateType);
    if (m_stateType == Fork || m_stateType == Join)
        stateElement.setAttribute("drawvertical", m_drawVertical);

    //save states activities
    QDomElement activitiesElement = qDoc.createElement("Activities");
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    for (int i = StateWidget::ActivityStartIndex; i < grp->textItemCount(); ++i) {
        QDomElement tempElement = qDoc.createElement("Activity");
        tempElement.setAttribute("name", grp->textItemAt(i)->text());
        activitiesElement.appendChild(tempElement);
    }//end for
    stateElement.appendChild(activitiesElement);

    qElement.appendChild(stateElement);
}

/**
 * Reimplemented from WidgetBase::loadFromXMI to load StateWidget
 * from XMI element.
 */
bool StateWidget::loadFromXMI(QDomElement & qElement)
{
    if ( !UMLWidget::loadFromXMI(qElement) ) {
        return false;
    }

    setName(qElement.attribute("statename", ""));
    setDocumentation(qElement.attribute("documentation", ""));
    QString type = qElement.attribute("statetype", "1");
    setStateType(static_cast<StateType>(type.toInt()));
    if (m_stateType == Fork || m_stateType == Join) {
        QString drawVerticalStr = qElement.attribute("drawvertical", "0");
        setDrawVertical((bool)drawVerticalStr.toInt());
    }

    //load states activities
    QDomNode node = qElement.firstChild();
    QDomElement tempElement = node.toElement();
    if ( !tempElement.isNull() && tempElement.tagName() == "Activities" ) {
        QDomNode node = tempElement.firstChild();
        QDomElement activityElement = node.toElement();
        while ( !activityElement.isNull() ) {
            if ( activityElement.tagName() == "Activity" ) {
                QString name = activityElement.attribute("name", "");
                if ( !name.isEmpty() ) {
                    addActivity(name);
                }
            }//end if
            node = node.nextSibling();
            activityElement = node.toElement();
        }//end while
    }//end if
    return true;
}

/**
 * Reimplementation from WidgetBase.
 */
void StateWidget::updateGeometry()
{
    const qreal radius = 18.0;

    switch (m_stateType) {
    case StateWidget::Normal:
        {
            TextItemGroup *grp = textItemGroupAt(GroupIndex);
            if (grp->minimumSize().width() < StateWidget::MinimumEllipseSize.width()) {
                setMinimumSize(StateWidget::MinimumEllipseSize);
            }
            else {
                setMinimumSize(grp->minimumSize());
            }
        }
        break;
    case StateWidget::Initial:
        {
            const QSizeF sz = QSizeF(radius, radius);
            setMinimumSize(sz);
            setMaximumSize(sz);
            setSize(sz);
        }
        break;
    case StateWidget::End:
        {
            const QSizeF sz = QSizeF(radius, radius);
            setMinimumSize(sz);
            setMaximumSize(sz);
            setSize(sz);
        }
        break;
    case StateWidget::Fork:
    case StateWidget::Join:
        {
            QSizeF sz = QSizeF(60, 8);
            if (m_drawVertical) {
                sz = QSizeF(8, 60);
            }
            setMinimumSize(sz);
            setMaximumSize(sz);
            setSize(sz);
        }
        break;
    case StateWidget::Junction:
        {
            const QSizeF sz = QSizeF(radius, radius);
            setMinimumSize(sz);
            setMaximumSize(sz);
            setSize(sz);
        }
        break;
    case StateWidget::DeepHistory:
        {
            const QSizeF sz = QSizeF(radius, radius);
            setMinimumSize(sz);
            setMaximumSize(sz);
            setSize(sz);
        }
        break;
    case StateWidget::ShallowHistory:
        {
            const QSizeF sz = QSizeF(radius, radius);
            setMinimumSize(sz);
            setMaximumSize(sz);
            setSize(sz);
        }
        break;
    case StateWidget::Choice:
        {
            const qreal len = 25.0;
            const QSizeF sz = QSizeF(len, len);
            setMinimumSize(sz);
            setMaximumSize(sz);
            setSize(sz);
        }
        break;
    default:
        uWarning() << "Unknown state type:" << stateTypeStr();
        setMinimumSize(StateWidget::MinimumEllipseSize);
        break;
    }

    UMLWidget::updateGeometry();
}

/**
 *
 */
void StateWidget::updateTextItemGroups()
{
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    if (m_stateType != StateWidget::Normal) {
        for (int i = 0; i < grp->textItemCount(); ++i) {
            grp->textItemAt(i)->setExplicitVisibility(false);
        }
    }
    else {
        int sz = grp->textItemCount();
        // Ensure atleast there is one item, that is - Name Item.
        if (sz == 0) {
            grp->appendTextItem(new TextItem(name()));
            sz = 1;
        }
        for (int i = 0; i < sz; ++i) {
            TextItem *item = grp->textItemAt(i);
            item->setExplicitVisibility(true);
        }
        grp->textItemAt(StateWidget::NameItemIndex)->setText(name());
        grp->textItemAt(StateWidget::NameItemIndex)->setBold(sz > 1);
    }

    UMLWidget::updateTextItemGroups();
}

/**
 *
 */
QVariant StateWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if (change == SizeHasChanged && m_stateType == StateWidget::Normal) {
        TextItemGroup *grp = textItemGroupAt(GroupIndex);
        grp->setGroupGeometry(rect());

        // line after each "line of text" except for the last one
        // as it is unnecessary to draw line on round rect.
        int cnt = grp->textItemCount();
        if (cnt > 0) {
            m_separatorLines.resize(cnt - 1);
            for (int i = 0; i < cnt - 1; ++i) {
                const TextItem *item = grp->textItemAt(i);
                const QPointF bottomLeft = item->mapToParent(item->boundingRect().bottomLeft());
                const qreal y = bottomLeft.y();
                m_separatorLines[i].setLine(1, y, size().width() - 1, y);
            }
        }
    }
    return UMLWidget::attributeChange(change, oldValue);
}

/**
 * Reimplemented from WidgetBase::slotMenuSelection to react to
 * special menu actions.
 */
void StateWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString nameNew = name();

    ListPopupMenu *menu = ListPopupMenu::menuFromAction(action);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }

    ListPopupMenu::MenuType sel = menu->getMenuType(action);
    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        nameNew = KInputDialog::getText( i18n("Enter State Name"),
                                         i18n("Enter the name of the new state:"),
                                         name(), &ok );
        if ( ok && !nameNew.isEmpty()) {
            setName( nameNew );
        }
        break;

    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        break;

    case ListPopupMenu::mt_New_Activity:
        nameNew = KInputDialog::getText( i18n("Enter Activity"),
                                         i18n("Enter the name of the new activity:"),
                                         i18n("new activity"), &ok );
        if ( ok && !nameNew.isEmpty()) {
            addActivity( nameNew );
        }
        break;

    case ListPopupMenu::mt_Flip:
        setDrawVertical(!m_drawVertical);
        break;

    default:
        UMLWidget::slotMenuSelection(action);
        break;
    }
}

#include "statewidget.moc"
