/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "forkjoinwidget.h"

//app includes
#include "debug_utils.h"
#include "umlview.h"
#include "umldoc.h"
#include "listpopupmenu.h"

// qt includes
#include <QColorDialog>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(ForkJoinWidget)

/**
 * Constructs a ForkJoinWidget.
 *
 * @param scene   The parent to this widget.
 * @param ori     Whether to draw the plate horizontally or vertically.
 * @param id      The ID to assign (-1 will prompt a new ID.)
 */
ForkJoinWidget::ForkJoinWidget(UMLScene * scene, Qt::Orientation ori, Uml::ID::Type id)
  : BoxWidget(scene, id, WidgetBase::wt_ForkJoin),
    m_orientation(ori)
{
    setSize(10, 40);
    m_usesDiagramFillColor = false;
    setFillColorCmd(QColor("black"));
}

/**
 * Destructor.
 */
ForkJoinWidget::~ForkJoinWidget()
{
}

/**
 * Get whether to draw the plate vertically or horizontally.
 */
Qt::Orientation ForkJoinWidget::orientation() const
{
    return m_orientation;
}

/**
 * Set whether to draw the plate vertically or horizontally.
 */
void ForkJoinWidget::setOrientation(Qt::Orientation ori)
{
    m_orientation = ori;
    updateGeometry();
    UMLWidget::adjustAssocs(x(), y());
}

/**
 * Reimplemented from UMLWidget::paint to draw the plate of
 * fork join.
 */
void ForkJoinWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->fillRect(0, 0, width(), height(), fillColor());

    UMLWidget::paint(painter, option, widget);
}

/**
 * Reimplemented from UMLWidget::loadFromXMI to load widget
 * info from XMI element - 'forkjoin'.
 */
bool ForkJoinWidget::loadFromXMI(QDomElement& qElement)
{
    if (!UMLWidget::loadFromXMI(qElement)) {
        return false;
    }

    QString drawVerticalStr = qElement.attribute(QLatin1String("drawvertical"), QLatin1String("0"));
    bool drawVertical = (bool)drawVerticalStr.toInt();
    if (drawVertical) {
        setOrientation(Qt::Vertical);
    }
    else {
        setOrientation(Qt::Horizontal);
    }

    return true;
}

/**
 * Reimplemented from UMLWidget::saveToXMI to save widget info
 * into XMI element - 'forkjoin'.
 */
void ForkJoinWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("forkjoin"));
    UMLWidget::saveToXMI(writer);
    bool drawVertical = true;
    if (m_orientation == Qt::Horizontal) {
        drawVertical = false;
    }
    writer.writeAttribute(QLatin1String("drawvertical"), QString::number(drawVertical));
    writer.writeEndElement();
}

/**
 * Show a properties dialog for a Fork/Join Widget.
 */
bool ForkJoinWidget::showPropertiesDialog()
{
    QColor newColor = QColorDialog::getColor(fillColor()); // krazy:exclude=qclasses
    if (!newColor.isValid())
        return false;
    if (newColor != fillColor()) {
        setFillColor(newColor);
        setUsesDiagramFillColor(false);
        umlDoc()->setModified(true);
    }
    return true;
}

/**
 * Reimplemented form UMLWidget::slotMenuSelection to handle
 * Flip action.
 */
void ForkJoinWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch (sel) {
    case ListPopupMenu::mt_Fill_Color:
        showPropertiesDialog();
        break;
    case ListPopupMenu::mt_FlipHorizontal:
        setOrientation(Qt::Horizontal);
        break;
    case ListPopupMenu::mt_FlipVertical:
        setOrientation(Qt::Vertical);
        break;
    default:
        break;
    }
}

/**
 * Overrides the function from UMLWidget.
 */
QSizeF ForkJoinWidget::minimumSize() const
{
    if (m_orientation == Qt::Vertical) {
        return QSizeF(4, 40);
    } else {
        return QSizeF(40, 4);
    }
}

/**
 * Reimplement method from UMLWidget to suppress the resize corner.
 * Although the ForkJoinWidget supports resizing, we suppress the
 * resize corner because it is too large for this very slim widget.
 */
void ForkJoinWidget::paintSelected(QPainter * p, int offsetX, int offsetY)
{
    Q_UNUSED(p);
    Q_UNUSED(offsetX);
    Q_UNUSED(offsetY);
}

/**
 * Reimplement method from UMLWidget.
 */
void ForkJoinWidget::constrain(qreal& width, qreal& height)
{
    if (m_orientation == Qt::Vertical) {
        if (width < 4)
            width = 4;
        else if (width > 10)
            width = 10;
        if (height < 40)
            height = 40;
        else if (height > 100)
            height = 100;
    } else {
        if (height < 4)
            height = 4;
        else if (height > 10)
            height = 10;
        if (width < 40)
            width = 40;
        else if (width > 100)
            width = 100;
    }
}
