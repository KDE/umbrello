/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "signalwidget.h"

// app includes
#include "basictypes.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "floatingtextwidget.h"
#include "linkwidget.h"
#include "listpopupmenu.h"
#include "uml.h"
#include "umldoc.h"
#include "uniqueid.h"
#include "umlview.h"
#include "umlwidget.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QEvent>
#include <QPolygon>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(SignalWidget)

/**
 * Creates a Signal widget.
 *
 * @param scene        The parent of the widget.
 * @param signalType   The type of Signal.
 * @param id           The ID to assign (-1 will prompt a new ID.)
 */
SignalWidget::SignalWidget(UMLScene *scene, SignalType signalType, Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_Signal, id),
    m_oldX(0),
    m_oldY(0)
{
    m_signalType = signalType;
    m_pName = 0;
    if (signalType == SignalWidget::Time) {
        m_pName = new FloatingTextWidget(scene, Uml::TextRole::Floating, QString());
        scene->setupNewWidget(m_pName);
        m_pName->setX(0);
        m_pName->setY(0);
        connect(m_pName, SIGNAL(destroyed()), this, SLOT(slotTextDestroyed()));
    }
}

/**
 * Destructor.
 */
SignalWidget::~SignalWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void SignalWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    setPenFromSettings(painter);
    const int w = width();
    const int h = height();
    QPolygon a;
    switch (m_signalType)
    {
    case Send :
        if(UMLWidget::useFillColor())
            painter->setBrush(UMLWidget::fillColor());
        {
            a.setPoints(5, 0,      0,
                           (w*2)/3, 0,
                            w,      h/2,
                           (w*2)/3, h,
                            0,      h);
            painter->drawPolygon(a);
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            int textStartY = (h / 2) - (fontHeight / 2);

            painter->setPen(textColor());
            QFont font = UMLWidget::font();
            font.setBold(false);
            painter->setFont(font);
            painter->drawText(SIGNAL_MARGIN, textStartY,
                           w - SIGNAL_MARGIN * 2, fontHeight,
                           Qt::AlignCenter, name());
            setPenFromSettings(painter);
        }
        break;
    case Accept :
        if(UMLWidget::useFillColor())
            painter->setBrush(UMLWidget::fillColor());
        {
            a.setPoints(5, 0,   0,
                            w/3, h/2,
                            0,   h,
                            w,   h,
                            w,   0);

            painter->drawPolygon(a);
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            int textStartY = (h / 2) - (fontHeight / 2);

            painter->setPen(textColor());
            QFont font = UMLWidget::font();
            font.setBold(false);
            painter->setFont(font);
            painter->drawText(SIGNAL_MARGIN, textStartY,
                           w - SIGNAL_MARGIN * 2 + (w/3), fontHeight,
                           Qt::AlignCenter, name());
            setPenFromSettings(painter);
        }
        break;
    case Time :
        if(UMLWidget::useFillColor())
            painter->setBrush(UMLWidget::fillColor());
        {
            a.setPoints(4, 0, 0,
                            w,  h,
                            0, h,
                            w,  0);

            painter->drawPolygon(a);
            //const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            //const int fontHeight  = fm.lineSpacing();
            //int textStartY = (h / 2) - (fontHeight / 2);
            painter->setPen(textColor());
            QFont font = UMLWidget::font();
            font.setBold(false);
            painter->setFont(font);

            setPenFromSettings(painter);
        }
        if (m_pName) {
            if (m_pName->x() == 0 && m_pName->y() == 0) {
                //the floating text has not been linked with the signal
                m_pName->setX(w/2 - m_pName->width()/2);
                m_pName->setY(h);
            }
            m_pName->setVisible((m_pName->text().length() > 0));
            m_pName->updateGeometry();
        }

        break;
    default:
        logWarn1("SignalWidget::paint: Unknown signal type %1", m_signalType);
        break;
    }

    UMLWidget::paint(painter, option, widget);
}

/**
 * Overrides the UMLWidget method.
 */
void SignalWidget::setX(qreal newX)
{
    m_oldX = x();
    UMLWidget::setX(newX);
}

/**
 * Overrides the UMLWidget method.
 */
void SignalWidget::setY(qreal newY)
{
    m_oldY = y();
    UMLWidget::setY(newY);
}

/**
 * Sets the name of the signal.
 */
void SignalWidget::setName(const QString &strName)
{
    UMLWidget::setName(strName);
    updateGeometry();
    if (signalType() == SignalWidget::Time) {
        if (!m_pName) {
            m_pName = new FloatingTextWidget(umlScene(), Uml::TextRole::Floating, m_Text);
            umlScene()->setupNewWidget(m_pName);
            m_pName->setX(0);
            m_pName->setY(0);
            connect(m_pName, SIGNAL(destroyed()), this, SLOT(slotTextDestroyed()));
        }
        else
            m_pName->setText(m_Text);
    }
}

/**
 * Returns the type of Signal.
 */
SignalWidget::SignalType SignalWidget::signalType() const
{
    return m_signalType;
}

/**
 * Returns the type string of Signal.
 */
QString SignalWidget::signalTypeStr() const
{
    return QLatin1String(ENUM_NAME(SignalWidget, SignalType, m_signalType));
}

/**
 * Sets the type of Signal.
 */
void SignalWidget::setSignalType(SignalType signalType)
{
    m_signalType = signalType;
}

/**
 * Show a properties dialog for a UMLWidget.
 */
bool SignalWidget::showPropertiesDialog()
{
    return false;
}

/**
 * Overrides mouseMoveEvent.
 */
void SignalWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* me)
{
    UMLWidget::mouseMoveEvent(me);
    int diffX = m_oldX - x();
    int diffY = m_oldY - y();
    if (m_pName!=0) {
        m_pName->setX(m_pName->x() - diffX);
        m_pName->setY(m_pName->y() - diffY);
    }
}

/**
 * Loads a "signalwidget" XMI element.
 */
bool SignalWidget::loadFromXMI(QDomElement & qElement)
{
    if(!UMLWidget::loadFromXMI(qElement))
        return false;
    m_Text = qElement.attribute(QLatin1String("signalname"));
    m_Doc = qElement.attribute(QLatin1String("documentation"));
    QString type = qElement.attribute(QLatin1String("signaltype"));
    QString textid = qElement.attribute(QLatin1String("textid"), QLatin1String("-1"));
    Uml::ID::Type textId = Uml::ID::fromString(textid);

    setSignalType((SignalType)type.toInt());
    if (signalType() == Time) {

        if (textId != Uml::ID::None) {
            UMLWidget *flotext = m_scene -> findWidget(textId);
            if (flotext != 0) {
            // This only happens when loading files produced by
            // umbrello-1.3-beta2.
                m_pName = static_cast<FloatingTextWidget*>(flotext);
                return true;
            }
        } else {
            // no textid stored -> get unique new one
            textId = UniqueID::gen();
        }
    }
     //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == QLatin1String("floatingtext") || tag == QLatin1String("UML::FloatingTextWidget")) {
            m_pName = new FloatingTextWidget(m_scene, Uml::TextRole::Floating, m_Text, textId);
            if(! m_pName->loadFromXMI(element)) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete m_pName;
                m_pName = 0;
            }
            else
                connect(m_pName, SIGNAL(destroyed()), this, SLOT(slotTextDestroyed()));
        } else {
            logError1("SignalWidget::loadFromXMI: unknown tag %1", tag);
        }
    }
   return true;
}

/**
 * Creates the "signalwidget" XMI element.
 */
void SignalWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("signalwidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeAttribute(QLatin1String("signalname"), m_Text);
    writer.writeAttribute(QLatin1String("documentation"), m_Doc);
    writer.writeAttribute(QLatin1String("signaltype"), QString::number(m_signalType));
    if (m_pName && !m_pName->text().isEmpty()) {
        writer.writeAttribute(QLatin1String("textid"), Uml::ID::toString(m_pName->id()));
        m_pName -> saveToXMI(writer);
    }
    writer.writeEndElement();
}

/**
 * Show a properties dialog for a SignalWidget.
 *
 */
void SignalWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        {
            QString name = m_Text;
            bool ok = Dialog_Utils::askName(i18n("Enter signal name"),
                                            i18n("Enter the signal name :"),
                                            name);
            if (ok && name.length() > 0) {
                setName(name);
            }
        }
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Overrides method from UMLWidget
 */
QSizeF SignalWidget::minimumSize() const
{
    int width = SIGNAL_WIDTH, height = SIGNAL_HEIGHT;
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight  = fm.lineSpacing();
    int textWidth = fm.width(name());

    if (m_signalType == Accept)
         textWidth = int((float)textWidth * 1.3f);
    height  = fontHeight;
    if (m_signalType != Time)
    {
          width   = textWidth > SIGNAL_WIDTH?textWidth:SIGNAL_WIDTH;
          height  = height > SIGNAL_HEIGHT?height:SIGNAL_HEIGHT;
    }
    width  += SIGNAL_MARGIN * 2;
    height += SIGNAL_MARGIN * 2;

    return QSizeF(width, height);
}

/**
 * Called if user deletes text widget
 */
void SignalWidget::slotTextDestroyed()
{
    m_pName = 0;
}

