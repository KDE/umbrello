/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "activitywidget.h"

// qt includes
#include <qpainter.h>

// kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>

// app includes
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "umlview.h"
#include "listpopupmenu.h"
#include "dialogs/activitydialog.h"
#include "umlscene.h"
#include "textitemgroup.h"
#include "textitem.h"
#include "widget_utils.h"

//Added by qt3to4:
#include <QMouseEvent>
#include <QPolygon>

/**
 * @class ActivityWidget
 *
 * This class is the graphical version of a UML Activity.  A ActivityWidget is created
 * by a @ref UMLView.  An ActivityWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The ActivityWidget class inherits from the @ref NewUMLRectWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML Activity.
 * @author Paul Hensgen <phensgen@techie.com>
 * @author Gopala Krishna (port using TextItems)
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

/**
 * Creates a Activity widget.
 *
 * @param view              The parent of the widget.
 * @param activityType      The type of activity.
 * @param id                The ID to assign (-1 will prompt a new ID.)
 */
ActivityWidget::ActivityWidget(UMLScene * scene, ActivityType activityType, Uml::IDType id )
    : NewUMLRectWidget(scene, id),
      m_activityType(activityType)
{
    m_baseType = Uml::wt_Activity;
    m_textItemGroup = new TextItemGroup(this);
}

/**
 *  destructor
 */
ActivityWidget::~ActivityWidget()
{
}

void ActivityWidget::setActivityType( ActivityType activityType )
{
    m_activityType = activityType;
    updateGeometry();
}

void ActivityWidget::setPreconditionText(const QString& aPreText)
{
    m_preconditionText = aPreText;
    updateGeometry();
}

void ActivityWidget::setPostconditionText(const QString& aPostText)
{
    m_postconditionText = aPostText;
    updateGeometry();
}

void ActivityWidget::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *)
{
    QRectF r = rect();
    const QSizeF sz = size();
    qreal w = sz.width();
    qreal h = sz.height();

    p->setPen(QPen(lineColor(), lineWidth()));
    p->setBrush(brush());

    switch(m_activityType) {
    case Normal:
        p->drawRoundRect(r, (h * 60) / w, 60);
        break;

    case Initial:
        p->drawEllipse(r);
        break;

    case Final:
        p->setBrush(Qt::NoBrush);
        p->drawEllipse(r);
        Widget_Utils::drawCrossInEllipse(p, r);
        break;

    case End :
        p->setBrush(Qt::NoBrush);
        p->drawEllipse(r.adjusted(+1, +1, -1, -1));

        p->setBrush(lineColor());
        p->drawEllipse(r.adjusted(+3, +3, -3, -3));
        break;

    case Branch :
    {
        QPolygon array( 4 );
        array[0] = QPoint(w / 2, 0);
        array[1] = QPoint(w, h / 2);
        array[2] = QPoint(w / 2, h);
        array[3] = QPoint(0, h / 2);
        p->drawPolygon(array);
    }
    break;

    case Invok :
        p->setPen(QPen(lineColor(), lineWidth()));
        p->drawRoundRect(r, (h * 60) / w, 60);
        {
            qreal x = w - (w/5);
            qreal y = h - (h/3);

            p->drawLine(QLineF(x,  y, x, y + 20));
            p->drawLine(QLineF(x - 10, y + 10, x + 10, y + 10));
            p->drawLine(QLineF(x - 10, y + 10, x - 10, y + 20));
            p->drawLine(QLineF(x + 10, y + 10, x + 10, y + 20));
        }
        break;

    case Param :
        p->setPen(QPen(lineColor(), lineWidth()));
        p->drawRoundRect(rect(), (h * 60) / w, 60);
        break;

    }
}

QSizeF ActivityWidget::sizeHint(Qt::SizeHint which)
{
    if(which == Qt::MinimumSize) {
        return m_minimumSize;
    }
    return NewUMLRectWidget::sizeHint(which);
}

void ActivityWidget::updateGeometry()
{
    int maxItemCount = 3; // In case of Param which has 3 texts to be drawn.

    TextItem dummy("");
    dummy.setDefaultTextColor(fontColor());
    dummy.setFont(font());
    dummy.setAlignment(Qt::AlignCenter);
    dummy.setBackgroundBrush(Qt::NoBrush);

    m_textItemGroup->ensureTextItemCount(maxItemCount);

    switch(m_activityType) {
    case Normal:
    case Invok:
    {
        // Hide unsused items
        m_textItemGroup->textItemAt(PrecondtionItemIndex)->hide();
        m_textItemGroup->textItemAt(PostconditionItemIndex)->hide();

        m_textItemGroup->textItemAt(NameItemIndex)->show();

        TextItem *nameItem = m_textItemGroup->textItemAt(NameItemIndex);
        nameItem->setText(name());
        dummy.copyAttributesTo(nameItem);

        m_minimumSize = m_textItemGroup->calculateMinimumSize();
        m_minimumSize.rheight() += 2 * ACTIVITY_MARGIN;
        m_minimumSize.rwidth() += 2 * ACTIVITY_MARGIN;

        if(m_activityType == Invok) {
            m_minimumSize.rheight() += 40;
        }
    }
    break;

    case Initial:
    case Final:
    case End:
    case Branch:
        m_textItemGroup->textItemAt(PrecondtionItemIndex)->hide();
        m_textItemGroup->textItemAt(PostconditionItemIndex)->hide();
        m_textItemGroup->textItemAt(NameItemIndex)->hide();

        m_minimumSize = QSizeF(20, 20);
        break;

    case Param:
    {
        TextItem *preconditionItem = m_textItemGroup->textItemAt(PrecondtionItemIndex);
        dummy.copyAttributesTo(preconditionItem);
        preconditionItem->setText(preconditionText().prepend("<<precondition>> "));
        preconditionItem->show();

        TextItem *postconditionItem = m_textItemGroup->textItemAt(PostconditionItemIndex);
        dummy.copyAttributesTo(postconditionItem);
        postconditionItem->setText(postconditionText().prepend("<<postcondition>> "));
        postconditionItem->show();

        TextItem *nameItem = m_textItemGroup->textItemAt(NameItemIndex);
        dummy.copyAttributesTo(nameItem);
        nameItem->setText(name());
        nameItem->show();

        m_minimumSize = m_textItemGroup->calculateMinimumSize();
        m_minimumSize.rwidth() += ACTIVITY_MARGIN * 2;
        m_minimumSize.rheight() += ACTIVITY_MARGIN * 2;
        break;
    }
    }
    NewUMLRectWidget::updateGeometry();
}

void ActivityWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString n = name();

    // menu is passed in as parent .
    ListPopupMenu *menu = qobject_cast<ListPopupMenu*>(action->parent());
    ListPopupMenu::Menu_Type sel = menu->getMenuType(action);

    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        n = KInputDialog::getText( i18n("Enter Activity Name"), i18n("Enter the name of the new activity:"), n, &ok );
        if( ok && !n.isEmpty()) {
            setName(n);
        }
        break;

    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        break;

    default:
        NewUMLRectWidget::slotMenuSelection(action);
    }
}

void ActivityWidget::showPropertiesDialog()
{
    DocWindow *docwindow = UMLApp::app()->getDocWindow();
    docwindow->updateDocumentation(false);

    ActivityDialog dialog(umlScene()->activeView(), this);
    if (dialog.exec() && dialog.getChangesMade()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->getDocument()->setModified(true);
    }
}

void ActivityWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement activityElement = qDoc.createElement( "activitywidget" );
    NewUMLRectWidget::saveToXMI( qDoc, activityElement );
    activityElement.setAttribute( "activityname", name() );
    activityElement.setAttribute( "documentation", documentation() );
    activityElement.setAttribute( "precondition", preconditionText() );
    activityElement.setAttribute( "postcondition", postconditionText() );
    activityElement.setAttribute( "activitytype", m_activityType );
    qElement.appendChild( activityElement );
}

bool ActivityWidget::loadFromXMI( QDomElement & qElement ) {
    if( !NewUMLRectWidget::loadFromXMI( qElement ) )
        return false;
    setName(qElement.attribute( "activityname", "" ));
    setDocumentation(qElement.attribute( "documentation", "" ));
    setPreconditionText(qElement.attribute( "precondition", "" ));
    setPostconditionText(qElement.attribute( "postcondition", "" ));

    QString type = qElement.attribute( "activitytype", "1" );
    setActivityType( (ActivityType)type.toInt() );

    return true;
}

void ActivityWidget::sizeHasChanged(const QSizeF &oldSize)
{
    QSizeF groupSize = size();
    groupSize.rwidth() -= ACTIVITY_MARGIN;
    groupSize.rheight() -= ACTIVITY_MARGIN;
    m_textItemGroup->alignVertically(groupSize);


    QPointF offset(ACTIVITY_MARGIN, ACTIVITY_MARGIN);
    m_textItemGroup->setPos(offset);
    NewUMLRectWidget::sizeHasChanged(oldSize);
}

#include "activitywidget.moc"
