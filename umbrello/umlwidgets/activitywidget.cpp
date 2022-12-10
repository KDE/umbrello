/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "activitywidget.h"

// app includes
#include "activitydialog.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "docwindow.h"
#include "dialogspopupmenu.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "widget_utils.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QPointer>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(ActivityWidget)

/**
 * Creates an Activity widget.
 *
 * @param scene          The parent of the widget.
 * @param activityType   The type of activity.
 * @param id             The ID to assign (-1 will prompt a new ID.)
 */
ActivityWidget::ActivityWidget(UMLScene * scene, ActivityType activityType, Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_Activity, id),
    m_activityType(activityType)
{
    // Set non zero size to avoid crash on painting.
    // We cannot call the reimplemented method minimumSize() in the constructor
    // because the vtable is not yet finalized (i.e. dynamic dispatch does not work).
    setSize(15, 15);
}

/**
 *  Destructor.
 */
ActivityWidget::~ActivityWidget()
{
}

/**
 * Returns the type of activity.
 */
ActivityWidget::ActivityType ActivityWidget::activityType() const
{
    return m_activityType;
}

/**
 * Returns the type string of activity.
 */
QString ActivityWidget::activityTypeStr() const
{
    return QStringLiteral(ENUM_NAME(ActivityWidget, ActivityType, m_activityType));
}

/**
 * Sets the type of activity.
 */
void ActivityWidget::setActivityType(ActivityType activityType)
{
    m_activityType = activityType;
    updateGeometry();
    UMLWidget::m_resizable = true;
}

/**
 * Determines whether a toolbar button represents an Activity.
 * CHECK: currently unused - can this be removed?
 *
 * @param tbb               The toolbar button enum input value.
 * @param resultType        The ActivityType corresponding to tbb.
 *                  This is only set if tbb is an Activity.
 * @return  True if tbb represents an Activity.
 */
bool ActivityWidget::isActivity(WorkToolBar::ToolBar_Buttons tbb,
                                ActivityType& resultType)
{
    bool status = true;
    switch (tbb) {
    case WorkToolBar::tbb_Initial_Activity:
        resultType = Initial;
        break;
    case WorkToolBar::tbb_Activity:
        resultType = Normal;
        break;
    case WorkToolBar::tbb_End_Activity:
        resultType = End;
        break;
    case WorkToolBar::tbb_Final_Activity:
        resultType = Final;
        break;
    case WorkToolBar::tbb_Branch:
        resultType = Branch;
        break;
    default:
        status = false;
        break;
    }
    return status;
}

/**
 * This method get the name of the preText attribute.
 */
QString ActivityWidget::preconditionText() const
{
    return m_preconditionText;
}

/**
 * This method set the name of the preText attribute
 */
void ActivityWidget::setPreconditionText(const QString& aPreText)
{
    m_preconditionText = aPreText;
    updateGeometry();
    adjustAssocs(x(), y());
}

/**
 * This method get the name of the postText attribute.
 */
QString ActivityWidget::postconditionText() const
{
    return m_postconditionText;
}

/**
 * This method set the name of the postText attribute
 */
void ActivityWidget::setPostconditionText(const QString& aPostText)
{
    m_postconditionText = aPostText;
    updateGeometry();
    adjustAssocs(x(), y());
}

/**
 * Reimplemented from UMLWidget::showPropertiesDialog to show a
 * properties dialog for an ActivityWidget.
 */
bool ActivityWidget::showPropertiesDialog()
{
    bool result = false;
    UMLApp::app()->docWindow()->updateDocumentation(false);

    QPointer<ActivityDialog> dialog = new ActivityDialog(umlScene()->activeView(), this);
    if (dialog->exec() && dialog->getChangesMade()) {
        UMLApp::app()->docWindow()->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
        result = true;
    }
    delete dialog;
    return result;
}

/**
 * Overrides the standard paint event.
 */
void ActivityWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    int w = width();
    int h = height();

    // Only for the final activity
    float x;
    float y;
    QPen pen = painter->pen();

    switch (m_activityType)
    {
    case Normal:
        UMLWidget::setPenFromSettings(painter);
        if (UMLWidget::useFillColor()) {
            painter->setBrush(UMLWidget::fillColor());
        }
        {
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            int textStartY = (h / 2) - (fontHeight / 2);
            painter->drawRoundRect(0, 0, w, h, (h * 60) / w, 60);
            painter->setPen(textColor());
            painter->setFont(UMLWidget::font());
            painter->drawText(ACTIVITY_MARGIN, textStartY,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, name());
        }
        break;

    case Initial:
        painter->setPen(QPen(WidgetBase::lineColor(), 1));
        painter->setBrush(WidgetBase::lineColor());
        painter->drawEllipse(0, 0, w, h);
        break;

    case Final:
        UMLWidget::setPenFromSettings(painter);
        painter->setBrush(Qt::white);
        pen.setWidth(2);
        pen.setColor (Qt::red);
        painter->setPen(pen);
        painter->drawEllipse(0, 0, w, h);
        x = w/2 ;
        y = h/2 ;
        {
            const float w2 = 0.7071 * (float)w / 2.0;
            painter->drawLine((int)(x - w2 + 1), (int)(y - w2 + 1), (int)(x + w2), (int)(y + w2));
            painter->drawLine((int)(x + w2 - 1), (int)(y - w2 + 1), (int)(x - w2), (int)(y + w2));
        }
        break;

    case End:
        painter->setPen(QPen(WidgetBase::lineColor(), 1));
        painter->setBrush(WidgetBase::lineColor());
        painter->drawEllipse(0, 0, w, h);
        painter->setBrush(Qt::white);
        painter->drawEllipse(1, 1, w - 2, h - 2);
        painter->setBrush(WidgetBase::lineColor());
        painter->drawEllipse(3, 3, w - 6, h - 6);
        break;

    case Branch:
        UMLWidget::setPenFromSettings(painter);
        if (UMLWidget::useFillColor()) {
            painter->setBrush(UMLWidget::fillColor());
        }
        {
            QPolygon array(4);
            array[ 0 ] = QPoint(w / 2, 0);
            array[ 1 ] = QPoint(w, h / 2);
            array[ 2 ] = QPoint(w / 2, h);
            array[ 3 ] = QPoint(0, h / 2);
            painter->drawPolygon(array);
            painter->drawPolyline(array);
        }
        break;

    case Invok:
        UMLWidget::setPenFromSettings(painter);
        if (UMLWidget::useFillColor()) {
            painter->setBrush(UMLWidget::fillColor());
        }
        {
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            int textStartY = (h / 2) - (fontHeight / 2);
            painter->drawRoundRect(0, 0, w, h, (h * 60) / w, 60);
            painter->setPen(textColor());
            painter->setFont(UMLWidget::font());
            painter->drawText(ACTIVITY_MARGIN, textStartY,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, name());

        }
        x = w - (w/5);
        y = h - (h/3);

        painter->drawLine((int)x, (int) y, (int)x, (int)(y + 20));
        painter->drawLine((int)(x - 10), (int)(y + 10), (int)(x + 10), (int)(y + 10));
        painter->drawLine((int)(x - 10), (int)(y + 10), (int)(x - 10), (int)(y + 20));
        painter->drawLine((int)(x + 10), (int)(y + 10), (int)(x + 10), (int)(y + 20));
        break;

    case Param:
        UMLWidget::setPenFromSettings(painter);
        if (UMLWidget::useFillColor()) {
            painter->setBrush(UMLWidget::fillColor());
        }
        {
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            QString preCond = Widget_Utils::adornStereo(QStringLiteral("precondition")) + preconditionText();
            QString postCond = Widget_Utils::adornStereo(QStringLiteral("postcondition")) + postconditionText();
            //int textStartY = (h / 2) - (fontHeight / 2);
            painter->drawRoundRect(0, 0, w, h, (h * 60) / w, 60);
            painter->setPen(textColor());
            painter->setFont(UMLWidget::font());
            painter->drawText(ACTIVITY_MARGIN, fontHeight + 10,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, preCond);
            painter->drawText(ACTIVITY_MARGIN, fontHeight * 2 + 10,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, postCond);
            painter->drawText(ACTIVITY_MARGIN, (fontHeight / 2),
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, name());
        }

        break;
    }
    UMLWidget::paint(painter, option, widget);
}

/**
 * Overridden from UMLWidget due to emission of signal sigActMoved()
 */
void ActivityWidget::moveWidgetBy(qreal diffX, qreal diffY)
{
    UMLWidget::moveWidgetBy(diffX, diffY);
    emit sigActMoved(diffX, diffY);
}

/**
 * Loads the widget from the "activitywidget" XMI element.
 */
bool ActivityWidget::loadFromXMI(QDomElement& qElement)
{
    if(!UMLWidget::loadFromXMI(qElement))
        return false;
    setName(qElement.attribute(QStringLiteral("activityname")));
    setDocumentation(qElement.attribute(QStringLiteral("documentation")));
    setPreconditionText(qElement.attribute(QStringLiteral("precondition")));
    setPostconditionText(qElement.attribute(QStringLiteral("postcondition")));

    QString type = qElement.attribute(QStringLiteral("activitytype"), QStringLiteral("1"));
    setActivityType((ActivityType)type.toInt());

    return true;
}

/**
 * Saves the widget to the "activitywidget" XMI element.
 */
void ActivityWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("activitywidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeAttribute(QStringLiteral("activityname"), name());
    writer.writeAttribute(QStringLiteral("documentation"), documentation());
    writer.writeAttribute(QStringLiteral("precondition"), preconditionText());
    writer.writeAttribute(QStringLiteral("postcondition"), postconditionText());
    writer.writeAttribute(QStringLiteral("activitytype"), QString::number(m_activityType));
    writer.writeEndElement();
}

/**
 * Overrides Method from UMLWidget.
 */
void ActivityWidget::constrain(qreal& width, qreal& height)
{
    if (m_activityType == Normal || m_activityType == Invok || m_activityType == Param) {
        UMLWidget::constrain(width, height);
        return;
    }

    if (width > height)
        width = height;
    else if (height > width)
        height = width;

    UMLWidget::constrain(width, height);
}

/**
 * Captures any popup menu signals for menus it created.
 */
void ActivityWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);

    switch(sel) {
    case ListPopupMenu::mt_Rename:
        {
            QString n = name();
            bool ok = Dialog_Utils::askRenameName(WidgetBase::wt_Activity, n);
            if (ok && !n.isEmpty()) {
                setName(n);
            }
        }
        break;

    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Overrides method from UMLWidget
 */
QSizeF ActivityWidget::minimumSize() const
{
    if (m_activityType == Normal || m_activityType == Invok || m_activityType == Param) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();

        int textWidth = fm.width(name());
        int height = fontHeight;
        height = height > ACTIVITY_HEIGHT ? height : ACTIVITY_HEIGHT;
        height += ACTIVITY_MARGIN * 2;

        textWidth = textWidth > ACTIVITY_WIDTH ? textWidth : ACTIVITY_WIDTH;

        if (m_activityType == Invok) {
             height += 40;
        } else if (m_activityType == Param) {
            QString maxSize;

            maxSize = name().length() > postconditionText().length() ? name() : postconditionText();
            maxSize = maxSize.length() > preconditionText().length() ? maxSize : preconditionText();

            textWidth = fm.width(maxSize);
            textWidth = textWidth + 50;
            height += 100;
        }

        int width = textWidth > ACTIVITY_WIDTH ? textWidth : ACTIVITY_WIDTH;

        width += ACTIVITY_MARGIN * 4;
        return QSizeF(width, height);
    }
    else if (m_activityType == Branch) {
        return QSizeF(20, 20);
    }
    return QSizeF(15, 15);
}

/**
 * Overrides method from UMLWidget
 */
QSizeF ActivityWidget::maximumSize()
{
    if (m_activityType == Normal || m_activityType == Invok || m_activityType == Param) {
        return UMLWidget::maximumSize();
    }
    if (m_activityType == Branch) {
        return QSizeF(50, 50);
    }
    return QSizeF(30, 30);
}


