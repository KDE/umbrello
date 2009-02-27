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
#include "objectnodewidget.h"

// qt includes
#include <qpainter.h>
#include <qlayout.h>
#include <qlabel.h>
#include <QVBoxLayout>
#include <QGridLayout>
//#include <pen.h>

// kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <kdialog.h>
#include <kcombobox.h>
#include <cmath>

// app includes
#include "dialog_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "umlview.h"
#include "listpopupmenu.h"
#include "dialogs/objectnodedialog.h"

//Added by qt3to4:
#include <QMouseEvent>
#include <QPolygon>

ObjectNodeWidget::ObjectNodeWidget(UMLView * view, ObjectNodeType objectNodeType, Uml::IDType id )
        : UMLWidget(view, id)
{
    UMLWidget::setBaseType( Uml::wt_ObjectNode );
    setObjectNodeType( objectNodeType );
    setState("");
    updateComponentSize();
}

ObjectNodeWidget::~ObjectNodeWidget() {}

void ObjectNodeWidget::draw(QPainter & p, int offsetX, int offsetY) {
    int w = width();
    int h = height();

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    int textStartY = (h / 2) - (fontHeight / 2);

    setPenFromSettings(p);

    if ( UMLWidget::getUseFillColour() ) {
        p.setBrush( UMLWidget::getFillColour() );
    }

    p.drawRect(offsetX, offsetY, w, h);
    p.setFont( UMLWidget::getFont() );


    switch ( m_ObjectNodeType )
    {
    case Normal : break;
    case Buffer :
        {
            p.setPen(Qt::black);
            p.drawText(offsetX + OBJECTNODE_MARGIN, (offsetY + textStartY/2), w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, "<< centralBuffer >>");
            p.drawText(offsetX + OBJECTNODE_MARGIN, (offsetY + textStartY/2) + fontHeight + 5, w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, getName());
        }
        break;
    case Data :
        {
            p.setPen(Qt::black);
            p.drawText(offsetX + OBJECTNODE_MARGIN, (offsetY + textStartY/2), w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, "<< datastore >>");
            p.drawText(offsetX + OBJECTNODE_MARGIN, (offsetY + textStartY/2) + fontHeight + 5, w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, getName());
        }
        break;
    case Flow :
        {
            QString objectflow_value;
            if(getState() == "-" || getState() == NULL)
            {
                objectflow_value = " ";
            }
            else
            {
                objectflow_value = '[' + getState() + ']';
            }

            p.drawLine(offsetX + 10 , offsetY + h/2, (offsetX + w)-10, offsetY + h/2  );
            p.setPen(Qt::black);
            p.setFont( UMLWidget::getFont() );
            p.drawText(offsetX + OBJECTNODE_MARGIN, offsetY + textStartY/2 - OBJECTNODE_MARGIN , w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, getName());
            p.drawText(offsetX + OBJECTNODE_MARGIN, offsetY + textStartY/2 + textStartY + OBJECTNODE_MARGIN, w - OBJECTNODE_MARGIN * 2, fontHeight, Qt::AlignHCenter, objectflow_value);
        }
        break;
    }
    setPenFromSettings(p);

    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);

}

QSize ObjectNodeWidget::calculateSize() {
    int widthtmp = 10, height = 10,width=10;
    if ( m_ObjectNodeType == Buffer ) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const int textWidth = fm.width("<< centrelBuffer >>");
        const int namewidth = fm.width(getName());
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
        const int namewidth = fm.width(getName());
        height = fontHeight * 2;
        widthtmp = textWidth > OBJECTNODE_WIDTH ? textWidth : OBJECTNODE_WIDTH;
        width = namewidth > widthtmp ? namewidth : widthtmp;
        height = height > OBJECTNODE_HEIGHT ? height : OBJECTNODE_HEIGHT;
        width += OBJECTNODE_MARGIN * 2;
        height += OBJECTNODE_MARGIN * 2 + 5;
    } else if ( m_ObjectNodeType == Flow ) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const int textWidth = fm.width('[' + getState() + ']');
        const int namewidth = fm.width(getName());
        height = fontHeight * 2;
        widthtmp = textWidth > OBJECTNODE_WIDTH ? textWidth : OBJECTNODE_WIDTH;
        width = namewidth > widthtmp ? namewidth : widthtmp;
        height = height > OBJECTNODE_HEIGHT ? height : OBJECTNODE_HEIGHT;
        width += OBJECTNODE_MARGIN * 2;
        height += OBJECTNODE_MARGIN * 4;
    }

    return QSize(width, height);
}

ObjectNodeWidget::ObjectNodeType ObjectNodeWidget::getObjectNodeType() const {
    return m_ObjectNodeType;
}

ObjectNodeWidget::ObjectNodeType ObjectNodeWidget::getObjectNodeType(const QString& objectNodeType) const {
    if (objectNodeType == "Central buffer")
       return ObjectNodeWidget::Buffer;
    if (objectNodeType == "Data store")
       return ObjectNodeWidget::Data;
    if (objectNodeType == "Object Flow")
       return ObjectNodeWidget::Flow;
    // Shouldn't happen
    Q_ASSERT(0);
    return ObjectNodeWidget::Flow;
}

void ObjectNodeWidget::setObjectNodeType( ObjectNodeType objectNodeType ) {
    m_ObjectNodeType = objectNodeType;
    UMLWidget::m_bResizable = true;
}

void ObjectNodeWidget::setObjectNodeType( const QString& objectNodeType ) {
   setObjectNodeType(getObjectNodeType(objectNodeType) );
}

void ObjectNodeWidget::setState(const QString& state){
    m_State = state;
    updateComponentSize();
}

QString ObjectNodeWidget::getState() {
    return m_State;
}

void ObjectNodeWidget::slotMenuSelection(QAction* action) {
    bool ok = false;
    QString name = m_Text;

    ListPopupMenu::Menu_Type sel = m_pMenu->getMenuType(action);
    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        name = KInputDialog::getText( i18n("Enter Object Node Name"), i18n("Enter the name of the object node :"), m_Text, &ok );
        if( ok && name.length() > 0 )
            m_Text = name;
        break;

    case ListPopupMenu::mt_Properties:
        showProperties();
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

void ObjectNodeWidget::showProperties() {
    DocWindow *docwindow = UMLApp::app()->getDocWindow();
    docwindow->updateDocumentation(false);

    ObjectNodeDialog dialog(m_pView, this);
    if (dialog.exec() && dialog.getChangesMade()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->getDocument()->setModified(true);
    }

}



void ObjectNodeWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement objectNodeElement = qDoc.createElement( "objectnodewidget" );
    UMLWidget::saveToXMI( qDoc, objectNodeElement );
    objectNodeElement.setAttribute( "objectnodename", m_Text );
    objectNodeElement.setAttribute( "documentation", m_Doc );
    objectNodeElement.setAttribute( "objectnodetype", m_ObjectNodeType );
    objectNodeElement.setAttribute( "objectnodestate", m_State );
    qElement.appendChild( objectNodeElement );
}

bool ObjectNodeWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "objectnodename", "" );
    m_Doc = qElement.attribute( "documentation", "" );
    QString type = qElement.attribute( "objectnodetype", "1" );
    m_State = qElement.attribute("objectnodestate","");
    setObjectNodeType( (ObjectNodeType)type.toInt() );
    return true;
}

void ObjectNodeWidget::askForObjectNodeType(UMLWidget* &targetWidget){
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

void ObjectNodeWidget::askStateForWidget(){
    bool pressedOK = false;
    QString state = KInputDialog::getText(i18n("Enter Object Flow State"),i18n("Enter State (keep '-' if there is no state for the object) "),i18n("-"), &pressedOK, UMLApp::app());

    if (pressedOK) {
        setState(state);
    } else {
        cleanup();
    }
}

void ObjectNodeWidget::slotOk() {
     //   KDialog::accept();
}

#include "objectnodewidget.moc"

