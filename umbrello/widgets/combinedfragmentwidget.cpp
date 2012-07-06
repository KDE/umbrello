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
#include "combinedfragmentwidget.h"

// app includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "docwindow.h"
#include "listpopupmenu.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// kde includes
#include <kinputdialog.h>
#include <klocale.h>

// qt includes
#include <QPolygonF>

/**
 * Creates a Combined Fragment widget.
 *
 * @param combinedfragmentType      The type of combined fragment.
 * @param id                The ID to assign (-1 will prompt a new ID.)
 */
CombinedFragmentWidget::CombinedFragmentWidget(CombinedFragmentType combinedfragmentType, Uml::IDType id)
  : UMLWidget(WidgetBase::wt_CombinedFragment, id),
    m_combinedFragmentType(combinedfragmentType)
{
    // The first group is for fragment "type" text.
    createTextItemGroup();

    // The second one is only for "Ref" type to show ReferenceDiagramName.
    createTextItemGroup();
}

/**
 * Destructor.
 */
CombinedFragmentWidget::~CombinedFragmentWidget()
{
    // Dashlines are deleted by ~QGraphicsItem() as they are children
}

/**
 * Reimplemented from UMLWidget::paint to draw
 * Combinedfragmentwidget. The text drawing is taken care by the
 * underlying textitems.
 */
void CombinedFragmentWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush((m_combinedFragmentType == Ref) ? brush() : QBrush(Qt::NoBrush));

    painter->drawRect(rect());

    painter->drawLines(m_fragmentBox, 3);
}

/**
 * Sets the combined fragment type of this widget to \a
 * combinedFragmentType and then updates the dash lines based on the
 * new type.
 */
void CombinedFragmentWidget::setCombinedFragmentType( CombinedFragmentType combinedfragmentType )
{
    m_combinedFragmentType = combinedfragmentType;

    // creates a dash line if the combined fragment type is alternative or parallel
    if (m_combinedFragmentType == Alt  || m_combinedFragmentType == Par) {
        if (m_dashLines.isEmpty()) {
            FloatingDashLineWidget *flt = new FloatingDashLineWidget(this);
            m_dashLines << flt;

            if(m_combinedFragmentType == Alt) {
                flt->setText("else");
            }

            setupFloatingWidget(flt);
        }
    }
    else {
        // Other widgets do not have dash lines.
        qDeleteAll(m_dashLines);
        m_dashLines.clear();
    }

    updateTextItemGroups();
    updateFloatingWidgetsPosition();
}

/**
 * Utility method to convert a string to CombinedFragmentType.
 */
CombinedFragmentWidget::CombinedFragmentType CombinedFragmentWidget::stringToCombinedFragementType(const QString& string)
{
    if (string == "Reference")
        return Ref;
    else if (string == "Option")
        return Opt;
    else if (string == "Break")
        return Break;
    else if (string == "Loop")
        return Loop;
    else if (string == "Negative")
        return Neg;
    else if (string == "Critical")
        return Crit;
    else if (string == "Assertion")
        return Ass;
    else if (string == "Alternative")
        return Alt;
    else if (string == "Parallel")
        return Par;

    // Shouldn't happen
    Q_ASSERT(0);
    return Ref;
}

/**
 * Pops up a dialog box and asks for the name.
 */
void CombinedFragmentWidget::askNameForWidgetType(UMLWidget* &targetWidget,
                                                  const QString& dialogTitle,
                                                  const QString& dialogPrompt,
                                                  const QString& /*defaultName*/)
{
    bool pressedOK = false;
    const QStringList list = QStringList() << "Reference" << "Option"
                                           << "Break" << "Loop"
                                           << "Negative" << "Critical"
                                           << "Assertion" << "Alternative"
                                           << "Parallel" ;

    const QStringList select = list;

    QStringList result = KInputDialog::getItemList (dialogTitle, dialogPrompt, list, select, false, &pressedOK, UMLApp::app());

    if (pressedOK) {
        CombinedFragmentType type = CombinedFragmentWidget::stringToCombinedFragementType(result.join(""));
        dynamic_cast<CombinedFragmentWidget*>(targetWidget)->setCombinedFragmentType(type);
        if (type == Ref) {
            Dialog_Utils::askNameForWidget(targetWidget,
                                           i18n("Enter the name of the diagram referenced"),
                                           i18n("Enter the name of the diagram referenced"),
                                           i18n("Diagram name"));
        }
        else if (type == Loop) {
            Dialog_Utils::askNameForWidget(targetWidget,
                                           i18n("Enter the guard of the loop"),
                                           i18n("Enter the guard of the loop"),
                                           i18n("-"));
        }
        else if (type == Alt) {
            Dialog_Utils::askNameForWidget(targetWidget,
                                           i18n("Enter the first alternative name"),
                                           i18n("Enter the first alternative name"),
                                           i18n("-"));
        }

        foreach (FloatingDashLineWidget *fld, m_dashLines) {
            if (fld->isActivated() == false) {
                fld->activate();
            }
        }
    } else {
        delete targetWidget;
        targetWidget = 0;
    }
}

bool CombinedFragmentWidget::activate()
{
    setActivatedFlag(false);
    bool status = true;
    foreach (FloatingDashLineWidget *fld, m_dashLines) {
        status = status && fld->activate();
        if (!status) return false;
    }
    return UMLWidget::activate();
}

/**
 * Reimplemented from UMLWidget::saveToXMI to save widget data
 * into 'combinedfragmentwidget' XMI element.
 */
void CombinedFragmentWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement combinedFragmentElement = qDoc.createElement( "combinedFragmentwidget" );
    UMLWidget::saveToXMI( qDoc, combinedFragmentElement );

    combinedFragmentElement.setAttribute( "combinedFragmentname", name() );
    combinedFragmentElement.setAttribute( "documentation", documentation() );
    combinedFragmentElement.setAttribute( "CombinedFragmenttype", combinedFragmentType() );

    // save the corresponding floating dash lines
    foreach (FloatingDashLineWidget *flWid, m_dashLines) {
        flWid->saveToXMI( qDoc, combinedFragmentElement );
    }

    qElement.appendChild( combinedFragmentElement );
}

/**
 * Reimplemented from UMLWidget::loadFromXMI to load
 * CombinedFragmentWidget data from XMI.
 *
 * This method also loads the child FloatingDashLineWidgets.
 */
bool CombinedFragmentWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    setName(qElement.attribute( "combinedFragmentname", "" ));
    setDocumentation(qElement.attribute( "documentation", "" ));

    QString type = qElement.attribute( "CombinedFragmenttype", "");
    Uml::IDType dashlineId;

    //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    while ( !element.isNull() ) {
        QString tag = element.tagName();
        if (tag == "floatingdashlinewidget") {
            FloatingDashLineWidget * fdlwidget = new FloatingDashLineWidget(this);
            if( !fdlwidget->loadFromXMI(element) ) {
              // Most likely cause: The FloatingTextWidget is empty.
                delete fdlwidget;
                return false;
            }

            // No need to call setupFloatingWidget as that will reset
            // the line properties of FloatingDashLineWidget.
            m_dashLines.append(fdlwidget);
        } else {
            uError() << "unknown tag " << tag << endl;
        }
        node = node.nextSibling();
        element = node.toElement();
    }
    // m_dashLines = listline;
    setCombinedFragmentType( (CombinedFragmentType)type.toInt() );

    return true;
}

/**
 * Reimplemented from UMLWidget::slotMenuSelection to handle
 * some specific actions.
 */
void CombinedFragmentWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString text = name();

    ListPopupMenu *menu = ListPopupMenu::menuFromAction(action);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }
    ListPopupMenu::MenuType sel = menu->getMenuType(action);

    if (sel == ListPopupMenu::mt_AddInteractionOperand) {
        FloatingDashLineWidget *flwd = new FloatingDashLineWidget(this);
        bool status = flwd->activate(); // should always succeeed
        Q_ASSERT(status); Q_UNUSED(status);
        m_dashLines.append(flwd);

        if(combinedFragmentType() == Alt) {
            flwd->setText("else");
        }

        setupFloatingWidget(flwd);
        updateGeometry();
        updateFloatingWidgetsPosition();
    }
    else if (sel == ListPopupMenu::mt_Rename) {
        if (combinedFragmentType() == Alt) {
            text = KInputDialog::getText( i18n("Enter first alternative"),
                                          i18n("Enter first alternative :"),
                                          text, &ok );
        }
        else if (combinedFragmentType() == Ref) {
        text = KInputDialog::getText( i18n("Enter referenced diagram name"),
                                      i18n("Enter referenced diagram name :"),
                                      text, &ok );
        }
        else if (combinedFragmentType() == Loop) {
        text = KInputDialog::getText( i18n("Enter the guard of the loop"),
                                      i18n("Enter the guard of the loop:"),
                                      text, &ok );
        }

        if( ok && !text.isEmpty() ) {
            setName(text);
        }
    }
    else {
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Reimplemented from UMLWidget::updateGeometry to calculate
 * the minimum size for this widget.
 */
void CombinedFragmentWidget::updateGeometry()
{
    TextItemGroup *typeGroup = textItemGroupAt(TypeBoxIndex);
    const QSizeF typeGroupSize = typeGroup->minimumSize();
    QSizeF minSize = typeGroupSize;

    // Now ensure that there is enough space for
    // FloatingDashLineWidget in between
    QSizeF floatingMinSize(100, 50);
    foreach(FloatingDashLineWidget *fldw, m_dashLines) {
        floatingMinSize = floatingMinSize.expandedTo(fldw->minimumSize());
    }
    minSize.rwidth() = qMax(floatingMinSize.width(), minSize.width());
    minSize.rheight() += floatingMinSize.height();;

    if (combinedFragmentType() == Ref) {
        QSizeF refSize = textItemGroupAt(ReferenceDiagramNameBoxIndex)->minimumSize();
        minSize.rwidth() = qMax(minSize.width(), refSize.width());
        minSize.setHeight(typeGroupSize.height() + qMax(floatingMinSize.height(), refSize.height()));
    }

    setMinimumSize(minSize);

    UMLWidget::updateGeometry();
}

/**
 * Reimplemented from UMLWidget::updateTextItemGroups to update
 * the TextItem's values and TextItems visibility.
 */
void CombinedFragmentWidget::updateTextItemGroups()
{
    TextItemGroup *grp = textItemGroupAt(TypeBoxIndex);
    grp->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    grp->setMargin(0);
    TextItemGroup *nameGroup = textItemGroupAt(ReferenceDiagramNameBoxIndex);

    grp->setTextItemCount(FirstAlternativeItemIndex + 1);
    nameGroup->setTextItemCount(1);

    QString combinedFragmentValue = name();
    QString temp;

    TextItem *typeItem = grp->textItemAt(TypeItemIndex);
    TextItem *firstAltItem = grp->textItemAt(FirstAlternativeItemIndex);

    TextItem *refItem = nameGroup->textItemAt(0);

    firstAltItem->setExplicitVisibility(false);
    refItem->setExplicitVisibility(false);

    switch(combinedFragmentType()) {
    case Ref:
        typeItem->setText("ref");
        refItem->setText(combinedFragmentValue);
        refItem->setExplicitVisibility(true);
        break;

    case Opt:
        typeItem->setText("opt");
        break;

    case Break:
        typeItem->setText("break");
        break;

    case Loop:
        if (combinedFragmentValue != "-") {
            temp = combinedFragmentValue;
            temp.prepend(" [");
            temp.append(']');
        }
        temp.prepend("loop");
        typeItem->setText(temp);
        break;

    case Neg :
        typeItem->setText("neg");
        break;

    case Crit :
        typeItem->setText("critical");
        break;

    case Ass :
        typeItem->setText("assert");
        break;

    case Alt :
        typeItem->setText("alt");

        if (combinedFragmentValue != "-") {
            temp = combinedFragmentValue;
            temp.prepend('[');
            temp.append(']');

            firstAltItem->setText(temp);
            firstAltItem->setExplicitVisibility(true);
        }
        break;

    case Par :
        typeItem->setText("parallel");
        break;
    }

    UMLWidget::updateTextItemGroups();
}

/**
 * Reimplemented from UMLWidget::attributeChange to handle
 *
 * - SizeHasChanged -> To set text position and update floating
 *                     widget's position and limits.
 *
 * - FontHasChanged -> Sets the CombinedFragmentWidget's changed font
 *                     to all the FloatingDashLineWidgets.
 *
 * - TextColorHasChanged -> Sets the CombinedFragmentWidget's changed
 *                          textcolor to all the
 *                          FloatingDashLineWidgets.
 *
 * - LineColorHasChanged -> Sets the CombinedFragmentWidget's changed
 *                          line color to all the
 *                          FloatingDashLineWidgets.
 *
 * - LineWidthHasChanged -> Sets the CombinedFragmentWidget's changed
 *                          line width to all the
 *                          FloatingDashLineWidgets.
 */
QVariant CombinedFragmentWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if (change == SizeHasChanged) {
        TextItemGroup *typeGroup  = textItemGroupAt(TypeBoxIndex);
        QSizeF typeGroupMinSize = typeGroup->minimumSize();
        typeGroupMinSize.rwidth() /= 0.6;
        typeGroup->setGroupGeometry(QRectF(QPointF(0, 0), typeGroupMinSize));

        // Calculate lines only if the group has text items in it.
        if (typeGroup->textItemCount() > 0) {
            TextItem *typeItem = typeGroup->textItemAt(TypeItemIndex);
            // Calculate the bottom right of text item in parent
            // widget's coordinates i.e Parent = CombinedFragmentWidget
            QPointF bottomRight = typeItem->mapToParent(typeItem->boundingRect().bottomRight());

            m_fragmentBox[0].setLine(0, bottomRight.y(), .6 * bottomRight.x(), bottomRight.y());
            m_fragmentBox[2].setLine(bottomRight.x(), 0, bottomRight.x(), .6 * bottomRight.y());
            m_fragmentBox[1].setPoints(m_fragmentBox[0].p2(), m_fragmentBox[2].p2());
        }

        if (combinedFragmentType() == Ref) {
            TextItemGroup *refDiagNameGroup = textItemGroupAt(ReferenceDiagramNameBoxIndex);
            QRectF r = rect();
            r.setTop(r.top() + typeGroupMinSize.height());
            refDiagNameGroup->setGroupGeometry(r);
        }

        updateFloatingWidgetsPosition();
    }

    return UMLWidget::attributeChange(change, oldValue);
}

/**
 * Helper method which applies this CombinedFragmentWidget's
 * properties to the FloatingDashLineWidget.
 *
 * This method is used mostly during creation of
 * FloatingDashLineWidget.
 */
void CombinedFragmentWidget::setupFloatingWidget(FloatingDashLineWidget *flt)
{
    flt->setLineColor(lineColor());
    flt->setLineWidth(lineWidth());
    flt->setTextColor(textColor());
    flt->setFont(font());
}

/**
 * Helper method which updates the FloatingDashLineWidget's dimensions
 * and constraint values.
 */
void CombinedFragmentWidget::updateFloatingWidgetsPosition()
{
    QSizeF sz = size();
    TextItemGroup *grp = textItemGroupAt(TypeBoxIndex);
    qreal originY = grp->minimumSize().height();

    foreach(FloatingDashLineWidget *fldw, m_dashLines) {
        fldw->setPos(0, fldw->pos().y());
        fldw->setSize(sz.width(), 0);
        fldw->setYMin(originY);
        fldw->setYMax(sz.height() - fldw->minimumSize().height());
    }
}

#include "combinedfragmentwidget.moc"
