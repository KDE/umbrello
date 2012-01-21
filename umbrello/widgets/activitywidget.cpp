/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "activitywidget.h"

// app includes
#include "activitydialog.h"
#include "debug_utils.h"
#include "docwindow.h"
#include "listpopupmenu.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

/**
 * Creates a Activity widget.
 *
 * @param scene              The parent of the widget.
 * @param activityType      The type of activity.
 * @param id                The ID to assign (-1 will prompt a new ID.)
 */
ActivityWidget::ActivityWidget(UMLScene * scene, ActivityType activityType, Uml::IDType id)
  : UMLWidget(scene, WidgetBase::wt_Activity, id),
    m_activityType(activityType)
{
}

/**
 *  destructor
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
 * Sets the type of activity.
 */
void ActivityWidget::setActivityType( ActivityType activityType )
{
    m_activityType = activityType;
    updateComponentSize();
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
 * This method get the name of the preText attribute
 */
QString ActivityWidget::preconditionText()
{
    return m_preconditionText;
}

/**
 * This method set the name of the preText attribute
 */
void ActivityWidget::setPreconditionText(const QString& aPreText)
{
    m_preconditionText = aPreText;
    updateComponentSize();
    adjustAssocs( getX(), getY() );
}

/**
 * This method get the name of the postText attribute
 */
QString ActivityWidget::postconditionText()
{
    return m_postconditionText ;
}

 /**
 * This method set the name of the postText attribute
 */
void ActivityWidget::setPostconditionText(const QString& aPostText)
{
    m_postconditionText = aPostText;
    updateComponentSize();
    adjustAssocs( getX(), getY() );
}

/**
 * Reimplemented from UMLWidget::showPropertiesDialog to show a
 * properties dialog for an ActivityWidget.
 */
void ActivityWidget::showPropertiesDialog()
{
    DocWindow *docwindow = UMLApp::app()->docWindow();
    docwindow->updateDocumentation(false);

    QPointer<ActivityDialog> dialog = new ActivityDialog(umlScene(), this);
    if (dialog->exec() && dialog->getChangesMade()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
    }
    delete dialog;
}

/**
 * Overrides the standard paint event.
 */
void ActivityWidget::paint(QPainter & p, int offsetX, int offsetY)
{
    int w = width();
    int h = height();

    // Only for the final activity
    float x;
    float y;
    QPen pen = p.pen();

    switch ( m_activityType )
    {
    case Normal :
        UMLWidget::setPenFromSettings(p);
        if ( UMLWidget::useFillColor() ) {
            p.setBrush( UMLWidget::fillColor() );
        }
        {
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            int textStartY = (h / 2) - (fontHeight / 2);
            p.drawRoundRect(offsetX, offsetY, w, h, (h * 60) / w, 60);
            p.setPen(textColor());
            p.setFont( UMLWidget::font() );
            p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + textStartY,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, name());
        }
        break;

    case Initial :
        p.setPen( QPen(m_LineColor, 1) );
        p.setBrush( WidgetBase::lineColor() );
        p.drawEllipse( offsetX, offsetY, w, h );
        break;

    case Final :

        UMLWidget::setPenFromSettings(p);
        p.setBrush( Qt::white );
        pen.setWidth( 2 );
        pen.setColor ( Qt::red );
        p.setPen( pen );
        p.drawEllipse( offsetX, offsetY, w, h );
        x = offsetX + w/2 ;
        y = offsetY + h/2 ;
        {
            const float w2 = 0.7071 * (float)w / 2.0;
            p.drawLine((int)(x - w2 + 1), (int)(y - w2 + 1), (int)(x + w2), (int)(y + w2) );
            p.drawLine((int)(x + w2 - 1), (int)(y - w2 + 1), (int)(x - w2), (int)(y + w2) );
        }
        break;

    case End :
        p.setPen( QPen(m_LineColor, 1) );
        p.setBrush( WidgetBase::lineColor() );
        p.drawEllipse( offsetX, offsetY, w, h );
        p.setBrush( Qt::white );
        p.drawEllipse( offsetX + 1, offsetY + 1, w - 2, h - 2 );
        p.setBrush( WidgetBase::lineColor() );
        p.drawEllipse( offsetX + 3, offsetY + 3, w - 6, h - 6 );
        break;

    case Branch :
        UMLWidget::setPenFromSettings(p);
        p.setBrush( UMLWidget::fillColor() );
        {
            QPolygon array( 4 );
            array[ 0 ] = QPoint( offsetX + w / 2, offsetY );
            array[ 1 ] = QPoint( offsetX + w, offsetY  + h / 2 );
            array[ 2 ] = QPoint( offsetX + w / 2, offsetY + h );
            array[ 3 ] = QPoint( offsetX, offsetY + h / 2 );
            p.drawPolygon( array );
            p.drawPolyline( array );
        }
        break;

    case Invok :
        UMLWidget::setPenFromSettings(p);
        if ( UMLWidget::useFillColor() ) {
            p.setBrush( UMLWidget::fillColor() );
        }
        {
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            int textStartY = (h / 2) - (fontHeight / 2);
            p.drawRoundRect(offsetX, offsetY, w, h, (h * 60) / w, 60);
            p.setPen(textColor());
            p.setFont( UMLWidget::font() );
            p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + textStartY,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, name());

        }
        x = offsetX + w - (w/5);
        y = offsetY + h - (h/3);

        p.drawLine((int)x, (int) y, (int)x, (int)( y + 20));
        p.drawLine((int)(x - 10),(int)(y + 10), (int)(x + 10), (int)(y + 10));
        p.drawLine((int)(x - 10),(int)(y + 10), (int)(x - 10), (int)(y + 20));
        p.drawLine((int)(x + 10),(int)(y + 10), (int)(x + 10), (int)(y + 20));
        break;

    case Param :
        UMLWidget::setPenFromSettings(p);
        if ( UMLWidget::useFillColor() ) {
            p.setBrush( UMLWidget::fillColor() );
        }
        {
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const int fontHeight  = fm.lineSpacing();
            QString preCond= "<<precondition>> "+preconditionText();
            QString postCond= "<<postcondition>> "+postconditionText();
            //int textStartY = (h / 2) - (fontHeight / 2);
            p.drawRoundRect(offsetX, offsetY, w, h, (h * 60) / w, 60);
            p.setPen(textColor());
            p.setFont( UMLWidget::font() );
            p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + fontHeight + 10,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, preCond);
            p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + fontHeight * 2 + 10,
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, postCond);
            p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + (fontHeight / 2),
                       w - ACTIVITY_MARGIN * 2, fontHeight, Qt::AlignCenter, name());
        }

        break;
    }
    if(m_selected)
        drawSelected(&p, offsetX, offsetY);
}

/**
 * Loads the widget from the "activitywidget" XMI element.
 */
bool ActivityWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    setName(qElement.attribute( "activityname", "" ));
    setDocumentation(qElement.attribute( "documentation", "" ));
    setPreconditionText(qElement.attribute( "precondition", "" ));
    setPostconditionText(qElement.attribute( "postcondition", "" ));

    QString type = qElement.attribute( "activitytype", "1" );
    setActivityType( (ActivityType)type.toInt() );

    return true;
}

/**
 * Saves the widget to the "activitywidget" XMI element.
 */
void ActivityWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement activityElement = qDoc.createElement( "activitywidget" );
    UMLWidget::saveToXMI( qDoc, activityElement );
    activityElement.setAttribute( "activityname", name() );
    activityElement.setAttribute( "documentation", documentation() );
    activityElement.setAttribute( "precondition", preconditionText() );
    activityElement.setAttribute( "postcondition", postconditionText() );
    activityElement.setAttribute( "activitytype", m_activityType );
    qElement.appendChild( activityElement );
}

/**
 * Overrides Method from UMLWidget.
 */
void ActivityWidget::constrain(int& width, int& height)
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
    bool ok = false;
    QString n = name();

    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);

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
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Overrides method from UMLWidget
 */
UMLSceneSize ActivityWidget::minimumSize()
{
    if ( m_activityType == Normal || m_activityType == Invok || m_activityType == Param ) {
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
        return UMLSceneSize(width, height);
    }
    else if (m_activityType == Branch) {
        return UMLSceneSize(20, 20);
    }
    return UMLSceneSize(15, 15);
}

/**
 * Overrides method from UMLWidget
 */
UMLSceneSize ActivityWidget::maximumSize()
{
    if (m_activityType == Normal || m_activityType == Invok || m_activityType == Param) {
        return UMLWidget::maximumSize();
    }
    if (m_activityType == Branch) {
        return UMLSceneSize(50,50);
    }
    return UMLSceneSize(30,30);
}


#include "activitywidget.moc"
