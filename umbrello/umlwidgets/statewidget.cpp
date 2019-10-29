/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "statewidget.h"

// app includes
#include "cmds/cmdcreatediagram.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "docwindow.h"
#include "listpopupmenu.h"
#include "statedialog.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QPointer>

/**
 * Creates a State widget.
 *
 * @param scene       The parent of the widget.
 * @param stateType   The type of state.
 * @param id          The ID to assign (-1 will prompt a new ID.)
 */
StateWidget::StateWidget(UMLScene * scene, StateType stateType, Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_State, id)
{
    m_stateType = stateType;
    m_drawVertical = true;
    setAspectRatioMode();
    m_Text = QLatin1String("State");
    QSizeF size = minimumSize();
    setSize(size.width(), size.height());
}

/**
 * Destructor.
 */
StateWidget::~StateWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void StateWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const qreal w = width();
    const qreal h = height();
    if (w == 0 || h == 0)
        return;

    setPenFromSettings(painter);
    switch (m_stateType) {
    case StateWidget::Normal:
        {
            if (UMLWidget::useFillColor()) {
                painter->setBrush(UMLWidget::fillColor());
            }
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            int textStartY = (h / 2) - (fontHeight / 2);
            const int count = m_Activities.count();
            if (count == 0) {
                painter->drawRoundRect(0, 0, w, h, (h*40)/w, (w*40)/h);
                painter->setPen(textColor());
                QFont font = UMLWidget::font();
                font.setBold(false);
                painter->setFont(font);
                painter->drawText(STATE_MARGIN, textStartY,
                           w - STATE_MARGIN * 2, fontHeight,
                           Qt::AlignCenter, name());
                setPenFromSettings(painter);
            } else {
                painter->drawRoundRect(0, 0, w, h, (h*40)/w, (w*40)/h);
                textStartY = STATE_MARGIN;
                painter->setPen(textColor());
                QFont font = UMLWidget::font();
                font.setBold(true);
                painter->setFont(font);
                painter->drawText(STATE_MARGIN, textStartY, w - STATE_MARGIN * 2,
                           fontHeight, Qt::AlignCenter, name());
                font.setBold(false);
                painter->setFont(font);
                setPenFromSettings(painter);
                int linePosY = textStartY + fontHeight;

                QStringList::Iterator end(m_Activities.end());
                for(QStringList::Iterator it(m_Activities.begin()); it != end; ++it) {
                    textStartY += fontHeight;
                    painter->drawLine(0, linePosY, w, linePosY);
                    painter->setPen(textColor());
                    painter->drawText(STATE_MARGIN, textStartY, w - STATE_MARGIN * 2,
                               fontHeight, Qt::AlignCenter, *it);
                    setPenFromSettings(painter);
                    linePosY += fontHeight;
                }//end for
            }//end else
        }
        break;
    case StateWidget::Initial :
        painter->setBrush(WidgetBase::lineColor());
        painter->drawEllipse(0, 0, w, h);
        break;
    case StateWidget::End :
        painter->setBrush(WidgetBase::lineColor());
        painter->drawEllipse(0, 0, w, h);
        painter->setBrush(Qt::white);
        painter->drawEllipse(1, 1, w - 2, h - 2);
        painter->setBrush(WidgetBase::lineColor());
        painter->drawEllipse(3, 3, w - 6, h - 6);
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
            painter->setFont(UMLWidget::font());
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing() / 2;
            const int xStar = fm.boundingRect(QLatin1String("H")).width();
            const int yStar = fontHeight / 4;
            painter->drawText((w / 6),
                       (h / 4) + fontHeight, QLatin1String("H"));
            painter->drawText((w / 6) + xStar,
                       (h / 4) + fontHeight - yStar, QLatin1String("*"));
        }
        break;
    case StateWidget::ShallowHistory:
        {
            painter->setBrush(Qt::white);
            painter->drawEllipse(rect());
            painter->setPen(Qt::black);
            painter->setFont(UMLWidget::font());
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing() / 2;
            painter->drawText((w / 6),
                       (h / 4) + fontHeight, QLatin1String("H"));
        }
        break;
    case StateWidget::Choice:
        {
            const qreal x = w / 2;
            const qreal y = h / 2;
            QPolygonF polygon;
            polygon << QPointF(x, 0) << QPointF(w, y)
                    << QPointF(x, h) << QPointF(0, y);
            painter->setBrush(UMLWidget::fillColor());
            painter->drawPolygon(polygon);
        }
        break;
    case StateWidget::Combined: {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        painter->drawRoundedRect(rect(), 10.0, 10.0);
        painter->drawLine(QPointF(0, fontHeight), QPointF(w, fontHeight));
        painter->setPen(textColor());
        QFont font = UMLWidget::font();
        font.setBold(false);
        painter->setFont(font);
        UMLView *view = m_doc->findView(m_diagramLinkId);
        painter->drawText(STATE_MARGIN, 0,
                          w - STATE_MARGIN * 2, fontHeight,
                          Qt::AlignCenter, view->umlScene()->name());
        setPenFromSettings(painter);
        view->umlScene()->render(painter, rect().adjusted(2, fontHeight + 2, -2, -2));
        break;
    }
    default:
        uWarning() << "Unknown state type: " << stateTypeStr();
        break;
    }

    UMLWidget::paint(painter, option, widget);
}

/**
 * Overrides method from UMLWidget
 */
QSizeF StateWidget::minimumSize() const
{
    int width = 10, height = 10;
    switch (m_stateType) {
        case StateWidget::Normal:
        {
            const QFontMetrics &fm = getFontMetrics(FT_BOLD);
            const int fontHeight  = fm.lineSpacing();
            int textWidth = fm.width(name());
            const int count = m_Activities.count();
            height = fontHeight;
            if(count > 0) {
                height = fontHeight * (count + 1);

                QStringList::ConstIterator end(m_Activities.end());
                for(QStringList::ConstIterator it(m_Activities.begin()); it != end; ++it) {
                    int w = fm.width(*it);
                    if(w > textWidth)
                        textWidth = w;
                }//end for
            }//end if
            width = textWidth > STATE_WIDTH?textWidth:STATE_WIDTH;
            height = height > STATE_HEIGHT?height:STATE_HEIGHT;
            width += STATE_MARGIN * 2;
            height += STATE_MARGIN * 2;
            break;
        }
        case StateWidget::Fork:
        case StateWidget::Join:
            if (m_drawVertical) {
                width = 8;
                height = 60;
            } else {
                width = 60;
                height = 8;
            }
            break;
        case StateWidget::Junction:
            width = 18;
            height = 18;
            break;
        case StateWidget::DeepHistory:
        case StateWidget::ShallowHistory:
            {
                const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
                width = height = fm.lineSpacing();
            }
            break;
        case StateWidget::Choice:
            width = 25;
            height = 25;
            break;
        case StateWidget::Combined:
            height = getFontMetrics(FT_NORMAL).lineSpacing() + 2;
        default:
            break;
    }

    return QSizeF(width, height);
}

/**
 * Overrides method from UMLWidget
 */
QSizeF StateWidget::maximumSize()
{
    switch (m_stateType) {
        case StateWidget::Initial:
        case StateWidget::End:
        case StateWidget::Junction:
        case StateWidget::Choice:
            return QSizeF(35, 35);
            break;
        case StateWidget::DeepHistory:
        case StateWidget::ShallowHistory:
            {
                const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
                const int fontHeight  = fm.lineSpacing();
                return QSizeF(fontHeight + 10, fontHeight + 10);
            }
            break;
        default:
            break;
    }
    return UMLWidget::maximumSize();
}

/**
 * Set the aspect ratio mode.
 * Some state types have a fixed aspect ratio
 */
void StateWidget::setAspectRatioMode()
{
    switch (m_stateType) {
        case StateWidget::Initial:
        case StateWidget::End:
        case StateWidget::Choice:
        case StateWidget::DeepHistory:
        case StateWidget::ShallowHistory:
        case StateWidget::Fork:
        case StateWidget::Join:
        case StateWidget::Junction:
            setFixedAspectRatio(true);
            break;
        default:
            setFixedAspectRatio(false);
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
    setAspectRatioMode();
}

/**
 * Adds an activity to this widget.
 * @return true on success
 */
bool StateWidget::addActivity(const QString &activity)
{
    m_Activities.append(activity);
    updateGeometry();
    return true;
}

/**
 * Removes the given activity from the state.
 */
bool StateWidget::removeActivity(const QString &activity)
{
    if(m_Activities.removeAll(activity) == 0)
        return false;
    updateGeometry();
    return true;
}

/**
 * Renames the given activity.
 */
bool StateWidget::renameActivity(const QString &activity, const QString &newName)
{
    int index = - 1;
    if((index = m_Activities.indexOf(activity)) == -1)
        return false;
    m_Activities[ index ] = newName;
    return true;
}

/**
 * Sets the states activities to the ones given.
 */
void StateWidget::setActivities(const QStringList &list)
{
    m_Activities = list;
    updateGeometry();
}

/**
 * Returns the list of activities.
 */
QStringList StateWidget::activities() const
{
    return m_Activities;
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
    setSize(height(), width());
    updateGeometry();
    UMLWidget::adjustAssocs(x(), y());
}

/**
 * Show a properties dialog for a StateWidget.
 */
bool StateWidget::showPropertiesDialog()
{
    bool result = false;
    UMLApp::app()->docWindow()->updateDocumentation(false);

    QPointer<StateDialog> dialog = new StateDialog(m_scene->activeView(), this);
    if (dialog->exec() && dialog->getChangesMade()) {
        UMLApp::app()->docWindow()->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
        result = true;
    }
    delete dialog;
    return result;
}

Uml::ID::Type StateWidget::diagramLink()
{
    return m_diagramLinkId;
}

void StateWidget::setDiagramLink(const Uml::ID::Type &id)
{
    m_diagramLinkId = id;
}

/**
 * Creates the "statewidget" XMI element.
 */
void StateWidget::saveToXMI1(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement stateElement = qDoc.createElement(QLatin1String("statewidget"));
    UMLWidget::saveToXMI1(qDoc, stateElement);
    stateElement.setAttribute(QLatin1String("statename"), m_Text);
    stateElement.setAttribute(QLatin1String("documentation"), m_Doc);
    stateElement.setAttribute(QLatin1String("statetype"), m_stateType);
    if (m_stateType == Fork || m_stateType == Join)
        stateElement.setAttribute(QLatin1String("drawvertical"), m_drawVertical);
    if (m_stateType == Combined)
        stateElement.setAttribute(QLatin1String("diagramlinkid"), Uml::ID::toString(m_diagramLinkId));
    //save states activities
    QDomElement activitiesElement = qDoc.createElement(QLatin1String("Activities"));

    QStringList::Iterator end(m_Activities.end());
    for(QStringList::Iterator it(m_Activities.begin()); it != end; ++it) {
        QDomElement tempElement = qDoc.createElement(QLatin1String("Activity"));
        tempElement.setAttribute(QLatin1String("name"), *it);
        activitiesElement.appendChild(tempElement);
    }//end for
    stateElement.appendChild(activitiesElement);
    qElement.appendChild(stateElement);
}

/**
 * Loads a "statewidget" XMI element.
 */
bool StateWidget::loadFromXMI1(QDomElement & qElement)
{
    if(!UMLWidget::loadFromXMI1(qElement))
        return false;
    m_Text = qElement.attribute(QLatin1String("statename"));
    m_Doc = qElement.attribute(QLatin1String("documentation"));
    QString type = qElement.attribute(QLatin1String("statetype"), QLatin1String("1"));
    m_stateType = (StateType)type.toInt();
    if (m_stateType == Combined) {
        QString linkID = qElement.attribute(QLatin1String("diagramlinkid"));
        m_diagramLinkId = Uml::ID::fromString(linkID);
    }
    setAspectRatioMode();
    QString drawVertical = qElement.attribute(QLatin1String("drawvertical"), QLatin1String("1"));
    m_drawVertical = (bool)drawVertical.toInt();
    //load states activities
    QDomNode node = qElement.firstChild();
    QDomElement tempElement = node.toElement();
    if(!tempElement.isNull() && tempElement.tagName() == QLatin1String("Activities")) {
        QDomNode node = tempElement.firstChild();
        QDomElement activityElement = node.toElement();
        while(!activityElement.isNull()) {
            if(activityElement.tagName() == QLatin1String("Activity")) {
                QString name = activityElement.attribute(QLatin1String("name"));
                if(!name.isEmpty())
                    m_Activities.append(name);
            }//end if
            node = node.nextSibling();
            activityElement = node.toElement();
        }//end while
    }//end if
    return true;
}

/**
 * Captures any popup menu signals for menus it created.
 */
void StateWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString nameNew = name();

    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        nameNew = name();
        ok = Dialog_Utils::askNewName(WidgetBase::WidgetType::wt_State, nameNew);
        if (ok && nameNew.length() > 0) {
            setName(nameNew);
        }
        break;

    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        break;

    case ListPopupMenu::mt_New_Activity:
        nameNew = i18n("new activity");
        ok = Dialog_Utils::askName(i18n("Enter Activity"),
                                   i18n("Enter the name of the new activity:"),
                                   nameNew);
        if (ok && nameNew.length() > 0) {
            addActivity(nameNew);
        }
        break;

    case ListPopupMenu::mt_FlipHorizontal:
        setDrawVertical(false);
        break;
    case ListPopupMenu::mt_FlipVertical:
        setDrawVertical(true);
        break;
    case ListPopupMenu::mt_CombinedState: {
        QString diagramName = m_doc->createDiagramName(Uml::DiagramType::State);
        Uml::CmdCreateDiagram* d = new Uml::CmdCreateDiagram(m_doc, Uml::DiagramType::State, diagramName);
        UMLApp::app()->executeCommand(d);
        m_diagramLinkId = d->view()->umlScene()->ID();
        setStateType(Combined);
        break;
    }

    default:
        UMLWidget::slotMenuSelection(action);
        break;
    }
}

