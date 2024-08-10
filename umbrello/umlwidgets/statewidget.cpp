/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
#include <QtGlobal>
#include <QPointer>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(StateWidget)

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
    setStateType(stateType);
    m_drawVertical = true;
    // Set non zero size to avoid crash on painting.
    // We cannot call the reimplemented method minimumSize() in the constructor
    // because the vtable is not yet finalized (i.e. dynamic dispatch does not work).
    setSize(15, 15);
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
            const int xStar = fm.boundingRect(QStringLiteral("H")).width();
            const int yStar = fontHeight / 4;
            painter->drawText((w / 6),
                       (h / 4) + fontHeight, QStringLiteral("H"));
            painter->drawText((w / 6) + xStar,
                       (h / 4) + fontHeight - yStar, QStringLiteral("*"));
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
                       (h / 4) + fontHeight, QStringLiteral("H"));
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
    case StateWidget::Combined:
        {
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight = fm.lineSpacing();
            setPenFromSettings(painter);
            QPainterPath path;
            path.addRoundedRect(rect(), STATE_MARGIN, STATE_MARGIN);
            if (useFillColor())
                painter->fillPath(path, UMLWidget::fillColor());
            painter->drawPath(path);
            painter->drawLine(QPointF(0, fontHeight), QPointF(w, fontHeight));
            painter->setPen(textColor());
            QFont font = UMLWidget::font();
            font.setBold(false);
            painter->setFont(font);
            painter->drawText(STATE_MARGIN, 0, w - STATE_MARGIN * 2, fontHeight,
                              Qt::AlignCenter, name());
            if (!linkedDiagram()) {
                m_size = QSizeF(fm.width(name()) + STATE_MARGIN * 2, fm.lineSpacing() + STATE_MARGIN);
            } else {
                DiagramProxyWidget::setClientRect(rect().adjusted(STATE_MARGIN, fontHeight + STATE_MARGIN, - STATE_MARGIN, -STATE_MARGIN));
                DiagramProxyWidget::paint(painter, option, widget);
                QSizeF size = DiagramProxyWidget::sceneRect().size();
                m_size = QSizeF(qMax<qreal>(fm.width(linkedDiagram()->name()), size.width()) + STATE_MARGIN * 2, fm.lineSpacing() + STATE_MARGIN + size.height());
                setSize(m_size);
            }
            setPenFromSettings(painter);
        }
        break;
    default:
        logWarn1("StateWidget::paint: Unknown state type %1", stateTypeStr());
        break;
    }

    UMLWidget::paint(painter, option, widget);
}

/**
 * Overrides method from UMLWidget
 */
QSizeF StateWidget::minimumSize() const
{
    int width = 15, height = 15;
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
                    int w = fm.horizontalAdvance(*it);
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
            return m_size;
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
        case StateWidget::Combined:
            return m_size;
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

void StateWidget::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
#ifdef ENABLE_COMBINED_STATE_DIRECT_EDIT
    if (m_stateType == Combined)
        DiagramProxyWidget::contextMenuEvent(event);
    if (event->isAccepted())
#endif
        UMLWidget::contextMenuEvent(event);
}

void StateWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef ENABLE_COMBINED_STATE_DIRECT_EDIT
    if (m_stateType == Combined)
        DiagramProxyWidget::mouseDoubleClickEvent(event);
    if (event->isAccepted())
#endif
        UMLWidget::mouseDoubleClickEvent(event);
}

void StateWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef ENABLE_COMBINED_STATE_DIRECT_EDIT
    if (m_stateType == Combined)
        DiagramProxyWidget::mousePressEvent(event);
    if (event->isAccepted())
#endif
        UMLWidget::mousePressEvent(event);
}

void StateWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef ENABLE_COMBINED_STATE_DIRECT_EDIT
    if (m_stateType == Combined)
        DiagramProxyWidget::mouseMoveEvent(event);
    if (event->isAccepted())
#endif
        UMLWidget::mouseMoveEvent(event);
}

void StateWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef ENABLE_COMBINED_STATE_DIRECT_EDIT
    if (m_stateType == Combined)
        DiagramProxyWidget::mouseReleaseEvent(event);
    if (event->isAccepted())
#endif
        UMLWidget::mouseReleaseEvent(event);
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
    if (stateType == Combined) {
        setAutoResize(false);
        setResizable(false);
    }
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

/**
 * Creates the "statewidget" XMI element.
 */
void StateWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("statewidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeAttribute(QStringLiteral("statename"), m_Text);
    writer.writeAttribute(QStringLiteral("documentation"), m_Doc);
    writer.writeAttribute(QStringLiteral("statetype"), QString::number(m_stateType));
    if (m_stateType == Fork || m_stateType == Join)
        writer.writeAttribute(QStringLiteral("drawvertical"), QString::number(m_drawVertical));
    //save states activities
    writer.writeStartElement(QStringLiteral("Activities"));

    QStringList::Iterator end(m_Activities.end());
    for (QStringList::Iterator it(m_Activities.begin()); it != end; ++it) {
        writer.writeStartElement(QStringLiteral("Activity"));
        writer.writeAttribute(QStringLiteral("name"), *it);
        writer.writeEndElement();
    }
    writer.writeEndElement();            // Activities
    writer.writeEndElement();  // statewidget
}

/**
 * Loads a "statewidget" XMI element.
 */
bool StateWidget::loadFromXMI(QDomElement & qElement)
{
    if(!UMLWidget::loadFromXMI(qElement))
        return false;
    m_Text = qElement.attribute(QStringLiteral("statename"));
    m_Doc = qElement.attribute(QStringLiteral("documentation"));
    QString type = qElement.attribute(QStringLiteral("statetype"), QStringLiteral("1"));
    setStateType((StateType)type.toInt());
    setAspectRatioMode();
    QString drawVertical = qElement.attribute(QStringLiteral("drawvertical"), QStringLiteral("1"));
    m_drawVertical = (bool)drawVertical.toInt();
    //load states activities
    QDomNode node = qElement.firstChild();
    QDomElement tempElement = node.toElement();
    if(!tempElement.isNull() && tempElement.tagName() == QStringLiteral("Activities")) {
        QDomNode node = tempElement.firstChild();
        QDomElement activityElement = node.toElement();
        while(!activityElement.isNull()) {
            if(activityElement.tagName() == QStringLiteral("Activity")) {
                QString name = activityElement.attribute(QStringLiteral("name"));
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

    default:
        DiagramProxyWidget::slotMenuSelection(action);
        break;
    }
}

