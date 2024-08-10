/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
#include <KLocalizedString>

// qt includes
#include <QPainter>
#include <QString>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(CombinedFragmentWidget)

static const int defaultWidth = 100;
static const int defaultHeight = 50;
static const int initialLabelWidth = 45;
static const int labelHeight = 20;
static const int labelBorderMargin = 10;

/**
 * Creates a Combined Fragment widget.
 *
 * @param scene              The parent of the widget.
 * @param combinedfragmentType      The type of combined fragment.
 * @param id                The ID to assign (-1 will prompt a new ID.)
 */
CombinedFragmentWidget::CombinedFragmentWidget(UMLScene * scene, CombinedFragmentType combinedfragmentType, Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_CombinedFragment, id),
    m_labelWidth(initialLabelWidth)
{
    setZValue(-10);
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
    int line_width = initialLabelWidth;
    qreal old_Y;

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
        QString temp = QStringLiteral("loop");

    switch (m_CombinedFragment)
    {
        case Ref :
        painter->drawText(defaultMargin, textStartY, w - defaultMargin * 2, fontHeight, Qt::AlignCenter, combined_fragment_value);
        painter->drawText(defaultMargin, 0, w - defaultMargin * 2, fontHeight, Qt::AlignLeft, QStringLiteral("ref"));
        break;

        case Opt :
        painter->drawText(defaultMargin, 0,
            w - defaultMargin * 2, fontHeight, Qt::AlignLeft, QStringLiteral("opt"));
        break;

        case Break :
        painter->drawText(defaultMargin, 0,
            w - defaultMargin * 2, fontHeight, Qt::AlignLeft, QStringLiteral("break"));
        break;

        case Loop :
                if (combined_fragment_value != QStringLiteral("-"))
                {
                     temp += QStringLiteral(" [") + combined_fragment_value + QLatin1Char(']');
                     line_width += (combined_fragment_value.size() + 2) * 8;
                }
        painter->drawText(defaultMargin, 0, w - defaultMargin * 2, fontHeight, Qt::AlignLeft, temp);

        break;

        case Neg :
        painter->drawText(defaultMargin, 0,
            w - defaultMargin * 2, fontHeight, Qt::AlignLeft, QStringLiteral("neg"));
        break;

        case Crit :
        painter->drawText(defaultMargin, 0,
            w - defaultMargin * 2, fontHeight, Qt::AlignLeft, QStringLiteral("critical"));
        break;

        case Ass :
        painter->drawText(defaultMargin, 0,
            w - defaultMargin * 2, fontHeight, Qt::AlignLeft, QStringLiteral("assert"));
        break;

        case Alt :
                if (combined_fragment_value != QStringLiteral("-"))
                {
                     temp = QLatin1Char('[') + combined_fragment_value + QLatin1Char(']');
            painter->drawText(defaultMargin, labelHeight,
                              w - defaultMargin * 2, fontHeight, Qt::AlignLeft, temp);
                    if (m_dashLines.size() == 1 && m_dashLines.first()->y() < y() + labelHeight + fontHeight)
                        m_dashLines.first()->setY(y() + h/2);
                }
                painter->drawText(defaultMargin, 0,
            w - defaultMargin * 2, fontHeight, Qt::AlignLeft, QStringLiteral("alt"));
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
                painter->drawText(defaultMargin, 0,
            w - defaultMargin * 2, fontHeight, Qt::AlignLeft, QStringLiteral("parallel"));
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
    painter->drawLine(0, labelHeight, line_width, labelHeight);
    painter->drawLine(line_width, labelHeight, line_width + labelBorderMargin, labelHeight-labelBorderMargin);
    painter->drawLine(line_width + labelBorderMargin, labelHeight-labelBorderMargin, line_width + labelBorderMargin, 0);
    m_labelWidth = line_width;

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
    width = textWidth + 60 > defaultWidth ? textWidth + 60: defaultWidth;
    if (m_CombinedFragment == Loop)
         width += int((float)textWidth * 0.4f);
    if (m_CombinedFragment == Alt)
         height += fontHeight + 40;
    height = height > defaultHeight ? height : defaultHeight;
    width += defaultMargin * 2;
    height += defaultMargin * 2;

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
    bool isLoading = UMLApp::app()->document()->loading();
    // creates a dash line if the combined fragment type is alternative or parallel
    if (!isLoading && m_CombinedFragment == Alt && m_dashLines.isEmpty())
    {
        m_dashLines.push_back(new FloatingDashLineWidget(m_scene, Uml::ID::None, this));
        m_scene->addWidgetCmd(m_dashLines.back());
    }
}

/**
 * Returns the type of combined fragment.
 */
CombinedFragmentWidget::CombinedFragmentType CombinedFragmentWidget::combinedFragmentType(const QString& type) const
{
    if (type == QStringLiteral("Reference"))
        return (CombinedFragmentWidget::Ref);
    if (type == QStringLiteral("Option"))
        return (CombinedFragmentWidget::Opt);
    if (type == QStringLiteral("Break"))
        return (CombinedFragmentWidget::Break);
    if (type == QStringLiteral("Loop"))
        return (CombinedFragmentWidget::Loop);
    if (type == QStringLiteral("Negative"))
        return (CombinedFragmentWidget::Neg);
    if (type == QStringLiteral("Critical"))
        return (CombinedFragmentWidget::Crit);
    if (type == QStringLiteral("Assertion"))
        return (CombinedFragmentWidget::Ass);
    if (type == QStringLiteral("Alternative"))
        return (CombinedFragmentWidget::Alt);
    if (type == QStringLiteral("Parallel"))
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
                             << QStringLiteral("Reference")
                             << QStringLiteral("Option")
                             << QStringLiteral("Break")
                             << QStringLiteral("Loop")
                             << QStringLiteral("Negative")
                             << QStringLiteral("Critical")
                             << QStringLiteral("Assertion")
                             << QStringLiteral("Alternative")
                             << QStringLiteral("Parallel") ;
    QPointer<QInputDialog> inputDlg = new QInputDialog();
    inputDlg->setComboBoxItems(list);
    inputDlg->setOptions(QInputDialog::UseListViewForComboBoxItems);
    inputDlg->setWindowTitle(dialogTitle);
    inputDlg->setLabelText(dialogPrompt);
    pressedOK = inputDlg->exec();
    QStringList result;
    result.append(inputDlg->textValue());
    delete inputDlg;
    if (pressedOK) {
        QString type = result.join(QString());
        targetWidget->asCombinedFragmentWidget()->setCombinedFragmentType(type);
        if (type == QStringLiteral("Reference"))
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the name of the diagram referenced"), i18n("Enter the name of the diagram referenced"), i18n("Diagram name"));
        else if (type == QStringLiteral("Loop"))
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the guard of the loop"), i18n("Enter the guard of the loop"), i18n("-"));
        else if (type == QStringLiteral("Alternative"))
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the first alternative name"), i18n("Enter the first alternative name"), i18n("-"));
    } else {
        targetWidget->cleanup();
        delete targetWidget;
        targetWidget = nullptr;
    }
}

/**
 * Saves the widget to the "combinedFragmentwidget" XMI element.
 */
void CombinedFragmentWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("combinedFragmentwidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeAttribute(QStringLiteral("combinedFragmentname"), m_Text);
    writer.writeAttribute(QStringLiteral("documentation"), m_Doc);
    writer.writeAttribute(QStringLiteral("CombinedFragmenttype"), QString::number(m_CombinedFragment));

    // save the corresponding floating dash lines
    for (QList<FloatingDashLineWidget*>::iterator it = m_dashLines.begin() ; it != m_dashLines.end() ; ++it) {
        (*it)-> saveToXMI(writer);
    }

    writer.writeEndElement();
}

/**
 * Loads the widget from the "CombinedFragmentwidget" XMI element.
 */
bool CombinedFragmentWidget::loadFromXMI(QDomElement & qElement)
{
    if (!UMLWidget::loadFromXMI(qElement))
        return false;
    m_Text = qElement.attribute(QStringLiteral("combinedFragmentname"));
    m_Doc = qElement.attribute(QStringLiteral("documentation"));
    QString type = qElement.attribute(QStringLiteral("CombinedFragmenttype"));

    //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    while (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == QStringLiteral("floatingdashlinewidget")) {
            FloatingDashLineWidget * fdlwidget = new FloatingDashLineWidget(m_scene, Uml::ID::None, this);
            m_dashLines.push_back(fdlwidget);
            if (!fdlwidget->loadFromXMI(element)) {
              // Most likely cause: The FloatingTextWidget is empty.
                delete m_dashLines.back();
                return false;
            }
            else {
                m_scene->addWidgetCmd(fdlwidget);
                fdlwidget->clipSize();
            }
        } else {
            logError1("CombinedFragmentWidget::loadFromXMI: unknown tag %1", tag);
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
 * Overrides the function from UMLWidget. Deletes all FloatingDashLineWidgets
 * that are associated with this instance.
 */
void CombinedFragmentWidget::cleanup()
{
  for(FloatingDashLineWidget* w : m_dashLines)
  {
    if (!w->isSelected()) {
      // no need to make this undoable, since the dashlines will be
      // reconstructed when deleting the combined fragment is undone.
      umlScene()->removeWidgetCmd(w);
    }
  }
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
            m_dashLines.back()->setText(QStringLiteral("else"));
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
                ok = Dialog_Utils::askName(i18n("Enter first alternative"),
                                           i18n("Enter first alternative :"),
                                           name);
            }
            else if (m_CombinedFragment == Ref) {
                ok = Dialog_Utils::askName(i18n("Enter referenced diagram name"),
                                           i18n("Enter referenced diagram name :"),
                                           name);
            }
            else if (m_CombinedFragment == Loop) {
                ok = Dialog_Utils::askName(i18n("Enter the guard of the loop"),
                                           i18n("Enter the guard of the loop:"),
                                           name);
            }
            if (ok && name.length() > 0)
                m_Text = name;
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
 * Overrides UMLWidget method to optionally set-up a dashed line
 */
bool CombinedFragmentWidget::activate(IDChangeLog *ChangeLog)
{
    if(UMLWidget::activate(ChangeLog))
    {
        if (!UMLApp::app()->document()->loading())
            setDashLineGeometryAndPosition();
        return true;
    }
    return false;
}

/**
 * Sets the geometry and positions of the last dash line
 */
void CombinedFragmentWidget::setDashLineGeometryAndPosition() const
{
    if (m_CombinedFragment == Alt && !m_dashLines.isEmpty())
    {
        m_dashLines.back()->setText(QStringLiteral("else"));
        m_dashLines.back()->setX(x());
        m_dashLines.back()->setYMin(y());
        m_dashLines.back()->setYMax(y() + height());
        m_dashLines.back()->setY(y() + height() / 2);
        m_dashLines.back()->setSize(width(), m_dashLines.back()->height());
    }
}

void CombinedFragmentWidget::toForeground()
{
}

QRectF CombinedFragmentWidget::boundingRect() const
{
    const qreal r = defaultMargin / 2.0;
    return rect().adjusted(-r, -r, r, r);
}

QPainterPath CombinedFragmentWidget::shape() const
{
    const qreal w = width();
    const qreal h = height();
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight = fm.lineSpacing();

    const qreal s = selectionMarkerSize * resizeMarkerLineCount + 4;
    const qreal r = defaultMargin / 2.0;
    qreal lw = m_labelWidth + r;
    qreal lh = labelHeight + r;
    if (m_CombinedFragment == Alt) {
        const int textWidth = fm.width(name() + QStringLiteral("[]"));
        lh += fontHeight;
        if (lw < textWidth)
            lw = textWidth + r;
    }

    QPainterPath outerPath;
    outerPath.setFillRule(Qt::WindingFill);
    outerPath.addRect(boundingRect());

    QPolygonF inner;
    inner.append(QPointF(r, lh));
    inner.append(QPointF(lw, lh));
    inner.append(QPointF(lw + labelBorderMargin, lh - labelBorderMargin));
    inner.append(QPointF(lw + labelBorderMargin, r));
    inner.append(QPointF(w - selectionMarkerSize - r, r));
    inner.append(QPointF(w - r, selectionMarkerSize + r));
    inner.append(QPointF(w - r, h - s));
    inner.append(QPointF(w - s, h - r));
    inner.append(QPointF(selectionMarkerSize + r, h - r));
    inner.append(QPointF(r, h - selectionMarkerSize - r));
    inner.append(QPointF(r, lh));
    QPainterPath innerPath;
    innerPath.addPolygon(inner);

    QPainterPath result = outerPath.subtracted(innerPath);
    return result.simplified();
}
