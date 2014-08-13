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
#include "combinedfragmentwidget.h"

// app includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "listpopupmenu.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

// qt includes
#include <QPainter>
#include <QString>

/**
 * Creates a Combined Fragment widget.
 *
 * @param scene              The parent of the widget.
 * @param combinedfragmentType      The type of combined fragment.
 * @param id                The ID to assign (-1 will prompt a new ID.)
 */
CombinedFragmentWidget::CombinedFragmentWidget(UMLScene * scene, CombinedFragmentType combinedfragmentType, Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_CombinedFragment, id)
{
    setCombinedFragmentType(combinedfragmentType);
}

/**
 * Destructor.
 */
CombinedFragmentWidget::~CombinedFragmentWidget()
{
    for (QList<FloatingDashLineWidget*>::iterator it=m_dashLines.begin() ; it!=m_dashLines.end() ; ++it) {
        delete(*it);
    }
}

/**
 * Overrides the standard paint event.
 */
void CombinedFragmentWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    int w = width();
    int h = height();
    int line_width = 45;
    int old_Y;

    setPenFromSettings(painter);

    if (m_CombinedFragment == Ref) {
        if (UMLWidget::useFillColor()) {
            painter->setBrush(UMLWidget::fillColor());
        }
    }
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const QString combined_fragment_value =  name();
    int textStartY = (h / 2) - (fontHeight / 2);
    painter->drawRect(0, 0, w, h);

    painter->setPen(textColor());
    painter->setFont(UMLWidget::font());
        QString temp = QLatin1String("loop");

    switch (m_CombinedFragment)
    {
        case Ref :
        painter->drawText(COMBINED_FRAGMENT_MARGIN, textStartY, w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignCenter, combined_fragment_value);
        painter->drawText(COMBINED_FRAGMENT_MARGIN, 0, w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, QLatin1String("ref"));
        break;

        case Opt :
        painter->drawText(COMBINED_FRAGMENT_MARGIN, 0,
            w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, QLatin1String("opt"));
        break;

        case Break :
        painter->drawText(COMBINED_FRAGMENT_MARGIN, 0,
            w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, QLatin1String("break"));
        break;

        case Loop :
                if (combined_fragment_value != QLatin1String("-"))
                {
                     temp += QLatin1String(" [") + combined_fragment_value + QLatin1Char(']');
                     line_width += (combined_fragment_value.size() + 2) * 8;
                }
        painter->drawText(COMBINED_FRAGMENT_MARGIN, 0, w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, temp);

        break;

        case Neg :
        painter->drawText(COMBINED_FRAGMENT_MARGIN, 0,
            w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, QLatin1String("neg"));
        break;

        case Crit :
        painter->drawText(COMBINED_FRAGMENT_MARGIN, 0,
            w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, QLatin1String("critical"));
        break;

        case Ass :
        painter->drawText(COMBINED_FRAGMENT_MARGIN, 0,
            w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, QLatin1String("assert"));
        break;

        case Alt :
                if (combined_fragment_value != QLatin1String("-"))
                {
                     temp = QLatin1Char('[') + combined_fragment_value + QLatin1Char(']');
            painter->drawText(COMBINED_FRAGMENT_MARGIN, 20, w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, temp);
                    if (m_dashLines.size() == 1 && m_dashLines.first()->y() < y() + 20 + fontHeight)
                        m_dashLines.first()->setY(y() + h/2);
                }
                painter->drawText(COMBINED_FRAGMENT_MARGIN, 0,
            w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, QLatin1String("alt"));
                // dash lines
                //m_dashLines.first()->paint(painter);
                // TODO: move to UMLWidget::calculateSize api
                for (QList<FloatingDashLineWidget*>::iterator it=m_dashLines.begin() ; it!=m_dashLines.end() ; ++it) {
                    (*it)->setX(x());
                    old_Y = (*it)->getYMin();
                    (*it)->setYMin(y());
                    (*it)->setYMax(y() + height());
                    (*it)->setY(y() + (*it)->y() - old_Y);
                    (*it)->setSize(w, (*it)->height());
                    (*it)->setLineColor(lineColor());
                    (*it)->setLineWidth(lineWidth());
                }

        break;

        case Par :
                painter->drawText(COMBINED_FRAGMENT_MARGIN, 0,
            w - COMBINED_FRAGMENT_MARGIN * 2, fontHeight, Qt::AlignLeft, QLatin1String("parallel"));
                // dash lines
                if (m_dashLines.size() != 0) {
                    //m_dashLines.first()->paint(painter);
                    // TODO: move to UMLWidget::calculateSize api
                    for (QList<FloatingDashLineWidget*>::iterator it=m_dashLines.begin() ; it!=m_dashLines.end() ; ++it) {
                        (*it)->setX(x());
                        old_Y = (*it)->getYMin();
                        (*it)->setYMin(y());
                        (*it)->setYMax(y() + height());
                        (*it)->setY(y() + (*it)->y() - old_Y);
                        (*it)->setSize(w, (*it)->height());
                        (*it)->setLineColor(lineColor());
                        (*it)->setLineWidth(lineWidth());
                    }
                }
        break;

    default : break;
    }

    setPenFromSettings(painter);
    painter->drawLine(0, 20, line_width, 20);
    painter->drawLine(line_width, 20, line_width + 10, 10);
    painter->drawLine(line_width + 10, 10, line_width + 10, 0);

    UMLWidget::paint(painter, option, widget);
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF CombinedFragmentWidget::minimumSize() const
{
    int width = 10, height = 10;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int textWidth = fm.width(name());
    height = fontHeight;
    width = textWidth + 60 > COMBINED_FRAGMENT_WIDTH ? textWidth + 60: COMBINED_FRAGMENT_WIDTH;
    if (m_CombinedFragment == Loop)
         width += int((float)textWidth * 0.4f);
    if (m_CombinedFragment == Alt)
         height += fontHeight + 40;
    height = height > COMBINED_FRAGMENT_HEIGHT ? height : COMBINED_FRAGMENT_HEIGHT;
    width += COMBINED_FRAGMENT_MARGIN * 2;
    height += COMBINED_FRAGMENT_MARGIN * 2;

    return QSizeF(width, height);
}

/**
 * Returns the type of combined fragment.
 */
CombinedFragmentWidget::CombinedFragmentType CombinedFragmentWidget::combinedFragmentType() const
{
    return m_CombinedFragment;
}

/**
 * Sets the type of combined fragment.
 */
void CombinedFragmentWidget::setCombinedFragmentType(CombinedFragmentType combinedfragmentType)
{
    m_CombinedFragment = combinedfragmentType;
    UMLWidget::m_resizable =  true ; //(m_CombinedFragment == Normal);
    // creates a dash line if the combined fragment type is alternative or parallel
    if (m_CombinedFragment == Alt  && m_dashLines.isEmpty())
    {
        m_dashLines.push_back(new FloatingDashLineWidget(m_scene, Uml::ID::None, this));
        if (m_CombinedFragment == Alt)
        {
            m_dashLines.back()->setText(QLatin1String("else"));
        }
        // TODO: move to UMLWidget::calculateSize api
        m_dashLines.back()->setX(x());
        m_dashLines.back()->setYMin(y());
        m_dashLines.back()->setYMax(y() + height());
        m_dashLines.back()->setY(y() + height()/2);
        m_dashLines.back()->setSize(width(), m_dashLines.back()->height());
        m_scene->setupNewWidget(m_dashLines.back());
    }
}

/**
 * Returns the type of combined fragment.
 */
CombinedFragmentWidget::CombinedFragmentType CombinedFragmentWidget::combinedFragmentType(const QString& type) const
{
    if (type == QLatin1String("Reference"))
        return (CombinedFragmentWidget::Ref);
    if (type == QLatin1String("Option"))
        return (CombinedFragmentWidget::Opt);
    if (type == QLatin1String("Break"))
        return (CombinedFragmentWidget::Break);
    if (type == QLatin1String("Loop"))
        return (CombinedFragmentWidget::Loop);
    if (type == QLatin1String("Negative"))
        return (CombinedFragmentWidget::Neg);
    if (type == QLatin1String("Critical"))
        return (CombinedFragmentWidget::Crit);
    if (type == QLatin1String("Assertion"))
        return (CombinedFragmentWidget::Ass);
    if (type == QLatin1String("Alternative"))
        return (CombinedFragmentWidget::Alt);
    if (type == QLatin1String("Parallel"))
        return (CombinedFragmentWidget::Par);
    // Shouldn't happen
    Q_ASSERT(0);
    return (CombinedFragmentWidget::Ref);
}

/**
 * Sets the type of combined fragment.
 */
void CombinedFragmentWidget::setCombinedFragmentType(const QString& combinedfragmentType)
{
    setCombinedFragmentType(combinedFragmentType(combinedfragmentType));
}

/**
 * ...
 */
void CombinedFragmentWidget::askNameForWidgetType(UMLWidget* &targetWidget, const QString& dialogTitle,
    const QString& dialogPrompt, const QString& defaultName)
{
    Q_UNUSED(defaultName);
    bool pressedOK = false;
    const QStringList list = QStringList()
                             << QLatin1String("Reference")
                             << QLatin1String("Option")
                             << QLatin1String("Break")
                             << QLatin1String("Loop")
                             << QLatin1String("Negative")
                             << QLatin1String("Critical")
                             << QLatin1String("Assertion")
                             << QLatin1String("Alternative")
                             << QLatin1String("Parallel") ;
    const QStringList select = list;
    QStringList result = KInputDialog::getItemList (dialogTitle, dialogPrompt, list, select, false, &pressedOK, UMLApp::app());

    if (pressedOK) {
        QString type = result.join(QString());
        dynamic_cast<CombinedFragmentWidget*>(targetWidget)->setCombinedFragmentType(type);
        if (type == QLatin1String("Reference"))
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the name of the diagram referenced"), i18n("Enter the name of the diagram referenced"), i18n("Diagram name"));
        else if (type == QLatin1String(QLatin1String("Loop")))
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the guard of the loop"), i18n("Enter the guard of the loop"), i18n("-"));
        else if (type == QLatin1String("Alternative"))
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the first alternative name"), i18n("Enter the first alternative name"), i18n("-"));
    } else {
        targetWidget->cleanup();
        delete targetWidget;
        targetWidget = NULL;
    }
}

/**
 * Saves the widget to the "combinedFragmentwidget" XMI element.
 */
void CombinedFragmentWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement combinedFragmentElement = qDoc.createElement(QLatin1String("combinedFragmentwidget"));
    UMLWidget::saveToXMI(qDoc, combinedFragmentElement);
    combinedFragmentElement.setAttribute(QLatin1String("combinedFragmentname"), m_Text);
    combinedFragmentElement.setAttribute(QLatin1String("documentation"), m_Doc);
    combinedFragmentElement.setAttribute(QLatin1String("CombinedFragmenttype"), m_CombinedFragment);

    // save the corresponding floating dash lines
    for (QList<FloatingDashLineWidget*>::iterator it = m_dashLines.begin() ; it != m_dashLines.end() ; ++it) {
        (*it)-> saveToXMI(qDoc, combinedFragmentElement);
    }

    qElement.appendChild(combinedFragmentElement);
}

/**
 * Loads the widget from the "CombinedFragmentwidget" XMI element.
 */
bool CombinedFragmentWidget::loadFromXMI(QDomElement & qElement)
{
    if (!UMLWidget::loadFromXMI(qElement))
        return false;
    m_Text = qElement.attribute(QLatin1String("combinedFragmentname"));
    m_Doc = qElement.attribute(QLatin1String("documentation"));
    QString type = qElement.attribute(QLatin1String("CombinedFragmenttype"));

    //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    while (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == QLatin1String("floatingdashlinewidget")) {
            FloatingDashLineWidget * fdlwidget = new FloatingDashLineWidget(m_scene, Uml::ID::None, this);
            m_dashLines.push_back(fdlwidget);
            if (!fdlwidget->loadFromXMI(element)) {
              // Most likely cause: The FloatingTextWidget is empty.
                delete m_dashLines.back();
                return false;
            }
            else {
                m_scene->widgetList().append(fdlwidget);
                fdlwidget->clipSize();
            }
        } else {
            uError() << "unknown tag " << tag;
        }
        node = node.nextSibling();
        element = node.toElement();
    }
   // m_dashLines = listline;
    setCombinedFragmentType((CombinedFragmentType)type.toInt());

    return true;
}

void CombinedFragmentWidget::removeDashLine(FloatingDashLineWidget *line)
{
    if (m_dashLines.contains(line))
        m_dashLines.removeOne(line);
}

/**
 * Overrides the function from UMLWidget.
 *
 * @param action  The command to be executed.
 */
void CombinedFragmentWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch (sel) {
          // for alternative or parallel combined fragments
    case ListPopupMenu::mt_AddInteractionOperand:
        m_dashLines.push_back(new FloatingDashLineWidget(m_scene, Uml::ID::None, this));
        if (m_CombinedFragment == Alt)
        {
            m_dashLines.back()->setText(QLatin1String("else"));
        }
        // TODO: move to UMLWidget::calculateSize api
        m_dashLines.back()->setX(x());
        m_dashLines.back()->setYMin(y());
        m_dashLines.back()->setYMax(y() + height());
        m_dashLines.back()->setY(y() + height() / 2);
        m_dashLines.back()->setSize(width(), m_dashLines.back()->height());
        m_scene->setupNewWidget(m_dashLines.back());
        break;

    case ListPopupMenu::mt_Rename:
        {
            bool ok = false;
            QString name = m_Text;

            if (m_CombinedFragment == Alt) {
                name = KInputDialog::getText(i18n("Enter first alternative"), i18n("Enter first alternative :"), m_Text, &ok);
            }
            else if (m_CombinedFragment == Ref) {
            name = KInputDialog::getText(i18n("Enter referenced diagram name"), i18n("Enter referenced diagram name :"), m_Text, &ok);
            }
            else if (m_CombinedFragment == Loop) {
            name = KInputDialog::getText(i18n("Enter the guard of the loop"), i18n("Enter the guard of the loop:"), m_Text, &ok);
            }
            if (ok && name.length() > 0)
                m_Text = name;
        }
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

#include "combinedfragmentwidget.moc"
