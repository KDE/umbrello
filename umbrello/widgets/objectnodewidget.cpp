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
#include "objectnodewidget.h"

// qt includes
#include <QtCore/QPointer>
#include <QtGui/QPainter>

// kde includes
#include <klocale.h>
#include <kinputdialog.h>
#include <kdialog.h>
#include <kcombobox.h>

#include <cmath>

// app includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "umlview.h"
#include "listpopupmenu.h"
#include "objectnodedialog.h"

ObjectNodeWidget::ObjectNodeWidget(UMLScene * scene, ObjectNodeType objectNodeType, Uml::IDType id )
  : UMLWidget(scene, WidgetBase::wt_ObjectNode, id)
{
    setObjectNodeType( objectNodeType );
    setState("");
}

ObjectNodeWidget::~ObjectNodeWidget()
{
}

void ObjectNodeWidget::paint(QPainter & p, int offsetX, int offsetY)
{
    int w = width();
    int h = height();

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    int textStartY = (h / 2) - (fontHeight / 2);

    setPenFromSettings(p);

    if ( UMLWidget::useFillColor() ) {
        p.setBrush( UMLWidget::fillColor() );
    }

    p.drawRect(offsetX, offsetY, w, h);
    p.setFont( UMLWidget::font() );

    switch ( m_ObjectNodeType )
    {
    case Normal : break;
    case Buffer :
        {
            p.setPen(textColor());
            p.drawText(offsetX + OBJECTNODE_MARGIN, (offsetY + textStartY/2), w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, "<< centralBuffer >>");
            p.drawText(offsetX + OBJECTNODE_MARGIN, (offsetY + textStartY/2) + fontHeight + 5, w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, name());
        }
        break;
    case Data :
        {
            p.setPen(textColor());
            p.drawText(offsetX + OBJECTNODE_MARGIN, (offsetY + textStartY/2), w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, "<< datastore >>");
            p.drawText(offsetX + OBJECTNODE_MARGIN, (offsetY + textStartY/2) + fontHeight + 5, w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, name());
        }
        break;
    case Flow :
        {
            QString objectflow_value;
            if(state() == "-" || state() == NULL)
            {
                objectflow_value = ' ';
            }
            else
            {
                objectflow_value = '[' + state() + ']';
            }

            p.drawLine(offsetX + 10 , offsetY + h/2, (offsetX + w)-10, offsetY + h/2  );
            p.setPen(textColor());
            p.setFont( UMLWidget::font() );
            p.drawText(offsetX + OBJECTNODE_MARGIN, offsetY + textStartY/2 - OBJECTNODE_MARGIN , w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, name());
            p.drawText(offsetX + OBJECTNODE_MARGIN, offsetY + textStartY/2 + textStartY + OBJECTNODE_MARGIN, w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, objectflow_value);
        }
        break;
    }
    setPenFromSettings(p);

    if(m_selected)
        drawSelected(&p, offsetX, offsetY);

}

UMLSceneSize ObjectNodeWidget::minimumSize()
{
    int widthtmp = 10, height = 10,width=10;
    if ( m_ObjectNodeType == Buffer ) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const int textWidth = fm.width("<< centrelBuffer >>");
        const int namewidth = fm.width(name());
        height = fontHeight * 2;
        widthtmp = textWidth > OBJECTNODE_WIDTH ? textWidth : OBJECTNODE_WIDTH;
        width = namewidth > widthtmp ? namewidth : widthtmp;
        height = height > OBJECTNODE_HEIGHT ? height : OBJECTNODE_HEIGHT;
        width += OBJECTNODE_MARGIN * 2;
        height += OBJECTNODE_MARGIN * 2 + 5;
    } else if ( m_ObjectNodeType == Data ) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const int textWidth = fm.width("<< datastore >>");
        const int namewidth = fm.width(name());
        height = fontHeight * 2;
        widthtmp = textWidth > OBJECTNODE_WIDTH ? textWidth : OBJECTNODE_WIDTH;
        width = namewidth > widthtmp ? namewidth : widthtmp;
        height = height > OBJECTNODE_HEIGHT ? height : OBJECTNODE_HEIGHT;
        width += OBJECTNODE_MARGIN * 2;
        height += OBJECTNODE_MARGIN * 2 + 5;
    } else if ( m_ObjectNodeType == Flow ) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const int textWidth = fm.width('[' + state() + ']');
        const int namewidth = fm.width(name());
        height = fontHeight * 2;
        widthtmp = textWidth > OBJECTNODE_WIDTH ? textWidth : OBJECTNODE_WIDTH;
        width = namewidth > widthtmp ? namewidth : widthtmp;
        height = height > OBJECTNODE_HEIGHT ? height : OBJECTNODE_HEIGHT;
        width += OBJECTNODE_MARGIN * 2;
        height += OBJECTNODE_MARGIN * 4;
    }

    return UMLSceneSize(width, height);
}

ObjectNodeWidget::ObjectNodeType ObjectNodeWidget::objectNodeType() const
{
    return m_ObjectNodeType;
}

ObjectNodeWidget::ObjectNodeType ObjectNodeWidget::objectNodeType(const QString& type) const
{
    if (type == "Central buffer")
       return ObjectNodeWidget::Buffer;
    if (type == "Data store")
       return ObjectNodeWidget::Data;
    if (type == "Object Flow")
       return ObjectNodeWidget::Flow;
    // Shouldn't happen
    Q_ASSERT(0);
    return ObjectNodeWidget::Flow;
}

void ObjectNodeWidget::setObjectNodeType( ObjectNodeType objectNodeType )
{
    m_ObjectNodeType = objectNodeType;
    UMLWidget::m_resizable = true;
}

void ObjectNodeWidget::setObjectNodeType( const QString& type )
{
   setObjectNodeType(ObjectNodeWidget::objectNodeType(type) );
}

void ObjectNodeWidget::setState(const QString& state)
{
    m_State = state;
    updateComponentSize();
}

QString ObjectNodeWidget::state()
{
    return m_State;
}

void ObjectNodeWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString name = m_Text;

    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);
    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        name = KInputDialog::getText( i18n("Enter Object Node Name"), i18n("Enter the name of the object node :"), m_Text, &ok );
        if( ok && name.length() > 0 )
            m_Text = name;
        break;

    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

void ObjectNodeWidget::showPropertiesDialog()
{
    DocWindow *docwindow = UMLApp::app()->docWindow();
    docwindow->updateDocumentation(false);

    QPointer<ObjectNodeDialog> dialog = new ObjectNodeDialog(m_scene, this);
    if (dialog->exec() && dialog->getChangesMade()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
    }
    delete dialog;
}

void ObjectNodeWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement objectNodeElement = qDoc.createElement( "objectnodewidget" );
    UMLWidget::saveToXMI( qDoc, objectNodeElement );
    objectNodeElement.setAttribute( "objectnodename", m_Text );
    objectNodeElement.setAttribute( "documentation", m_Doc );
    objectNodeElement.setAttribute( "objectnodetype", m_ObjectNodeType );
    objectNodeElement.setAttribute( "objectnodestate", m_State );
    qElement.appendChild( objectNodeElement );
}

bool ObjectNodeWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "objectnodename", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    QString type = qElement.attribute( "objectnodetype", "1" );
    m_State = qElement.attribute("objectnodestate","");
    setObjectNodeType( (ObjectNodeType)type.toInt() );
    return true;
}

void ObjectNodeWidget::askForObjectNodeType(UMLWidget* &targetWidget)
{
    bool pressedOK = false;
    int current = 0;
    const QStringList list = QStringList() << "Central buffer" << "Data store" <<"Object Flow";

    QString type = KInputDialog::getItem ( i18n("Select Object node type"),  i18n("Select the object node type"),list,current, false, &pressedOK, UMLApp::app());

    if (pressedOK) {
       // QString type = result.join("");
        dynamic_cast<ObjectNodeWidget*>(targetWidget)->setObjectNodeType(type);
        if (type == "Data store")
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the name of the data store node"), i18n("Enter the name of the data store node"), i18n("data store name"));
        if (type == "Central buffer")
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the name of the buffer node"), i18n("Enter the name of the buffer"), i18n("centralBuffer"));
        if (type == "Object Flow") {
            Dialog_Utils::askNameForWidget(targetWidget, i18n("Enter the name of the object flow"), i18n("Enter the name of the object flow"), i18n("object flow"));
            askStateForWidget();
        }
    } else {
        targetWidget->cleanup();
        delete targetWidget;
        targetWidget = NULL;
    }
}

void ObjectNodeWidget::askStateForWidget()
{
    bool pressedOK = false;
    QString state = KInputDialog::getText(i18n("Enter Object Flow State"),i18n("Enter State (keep '-' if there is no state for the object) "),i18n("-"), &pressedOK, UMLApp::app());

    if (pressedOK) {
        setState(state);
    } else {
        cleanup();
    }
}

void ObjectNodeWidget::slotOk()
{
     //   KDialog::accept();
}

#include "objectnodewidget.moc"

