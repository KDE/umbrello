/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "docwindow.h"

// qt/kde includes
#include <qgroupbox.h>
#include <qlayout.h>
#include <qmultilineedit.h>
#include <klocale.h>

// local includes
#include "associationwidget.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlview.h"
#include "umlwidget.h"


DocWindow::DocWindow( UMLDoc * doc, QWidget *parent, const char *name ) : QWidget( parent, name ) {
    //setup visual display
    QHBoxLayout * mainLayout = new QHBoxLayout( this );

    m_pDocGB = new QGroupBox( i18n( "Documentation" ), this );
    mainLayout -> addWidget( m_pDocGB );

    QHBoxLayout * docLayout = new QHBoxLayout( m_pDocGB );
    m_pDocMLE = new QMultiLineEdit( m_pDocGB );
    m_pDocMLE -> setText( "" );
    docLayout -> setMargin( fontMetrics().height() );
    docLayout -> addWidget( m_pDocMLE);
    m_pDocMLE -> setWordWrap(QMultiLineEdit::WidgetWidth);

    //setup the documentation variables
    //show projects documentation to start
    m_pUMLDoc = doc;
    m_Showing = st_Project;
    m_pUMLObject = 0;
    m_pUMLView = 0;
    m_pUMLWidget = 0;
    m_pAssocWidget = 0;
    updateDocumentation( true, true );
}

DocWindow::~DocWindow() {}

void DocWindow::showDocumentation( UMLObject * object, bool overwrite ) {
    if( object == m_pUMLObject && !overwrite )
        return;
    if( object != m_pUMLObject )
        updateDocumentation( true );

    m_Showing = st_UMLObject;
    if( !object ) {
        m_pDocMLE->setText( m_pUMLDoc->getDocumentation() );
        m_pUMLObject = 0;
        return;
    }
    m_pUMLObject = object;
    m_pDocMLE -> setText( m_pUMLObject -> getDoc() );
}

void DocWindow::updateDocumentation( bool clear, bool startup ) {

    bool mark_modified = false;
    if( m_pUMLObject )
    {
        // the file is marked modified, if the documentation differs
        // we don't do this on startup/load of a xmi file, because every time
        // modified is set, we get another undo/redo backup point
        if ( startup == false && m_pDocMLE -> text() != m_pUMLObject -> getDoc() )
        {
            mark_modified = true;
        }
        m_pUMLObject -> setDoc( m_pDocMLE -> text() );

    } else if( m_pUMLView ) {
        // the file is marked modified, if the documentation differs
        // we don't do this on startup/load of a xmi file, because every time
        // modified is set, we get another undo/redo backup point
        if ( startup == false && m_pDocMLE -> text() != m_pUMLView -> getDoc() )
        {
            mark_modified = true;
        }

        m_pUMLView -> setDoc( m_pDocMLE -> text() );
    } else if ( m_pUMLWidget ) {
        // the file is marked modified, if the documentation differs
        // we don't do this on startup/load of a xmi file, because every time
        // modified is set, we get another undo/redo backup point
        if ( startup == false && m_pDocMLE -> text() != m_pUMLWidget -> getDoc() )
        {
            mark_modified = true;
        }

        m_pUMLWidget -> setDoc( m_pDocMLE -> text() );
    } else if( m_pAssocWidget ) {
        // the file is marked modified, if the documentation differs
        // we don't do this on startup/load of a xmi file, because every time
        // modified is set, we get another undo/redo backup point
        if ( startup == false && m_pDocMLE -> text() != m_pAssocWidget -> getDoc() )
        {
            mark_modified = true;
        }

        m_pAssocWidget -> setDoc( m_pDocMLE -> text() );
    } else {
        // the file is marked modified, if the documentation differs
        // we don't do this on startup/load of a xmi file, because every time
        // modified is set, we get another undo/redo backup point
        if ( startup == false && m_pDocMLE -> text() != m_pUMLDoc->getDocumentation() )
        {
            mark_modified = true;
        }

        m_pUMLDoc->setDocumentation( m_pDocMLE->text() );
    }

    // now do the setModified call
    if (mark_modified == true)
        m_pUMLDoc -> setModified( true );

    // we should show the documentation of the whole project
    if( clear ) {
        m_pDocMLE->setText( m_pUMLDoc->getDocumentation() );
        m_pUMLObject = 0;
        m_pUMLView = 0;
        m_pUMLWidget = 0;
        m_pAssocWidget = 0;
        m_Showing = st_Project;
    }

    return;
}

void DocWindow::showDocumentation( UMLView * view, bool overwrite ) {
    if( view == m_pUMLView && !overwrite )
        return;
    if( view != m_pUMLView )
        updateDocumentation( true );
    m_Showing = st_UMLView;
    if( !view ) {
        m_pDocMLE->setText( m_pUMLDoc->getDocumentation() );
        m_pUMLView = 0;
        return;
    }
    m_pUMLView = view;
    m_pDocMLE -> setText( m_pUMLView -> getDoc() );
}

void DocWindow::showDocumentation( UMLWidget * widget, bool overwrite ) {
    if( widget == m_pUMLWidget && !overwrite )
        return;
    if( widget != m_pUMLWidget )
        updateDocumentation( true );
    m_Showing = st_UMLWidget;
    if( !widget ) {
        m_pDocMLE->setText( m_pUMLDoc->getDocumentation() );
        m_pUMLWidget = 0;
        return;
    }
    m_pUMLWidget = widget;
    m_pDocMLE -> setText( m_pUMLWidget -> getDoc() );
}

void DocWindow::showDocumentation( AssociationWidget * widget, bool overwrite ) {
    if( widget == m_pAssocWidget && !overwrite )
        return;
    if( widget != m_pAssocWidget )
        updateDocumentation( true );
    m_Showing = st_Association;
    if( !widget ) {
        m_pDocMLE->setText( m_pUMLDoc->getDocumentation() );
        m_pAssocWidget = 0;
        return;
    }
    m_pAssocWidget = widget;
    m_pDocMLE -> setText( m_pAssocWidget -> getDoc() );
}

void DocWindow::newDocumentation( ) {
    m_pUMLView = 0;
    m_pUMLObject = 0;
    m_pUMLWidget = 0;
    m_pAssocWidget = 0;
    m_Showing = st_Project;
    m_pDocMLE->setText( m_pUMLDoc->getDocumentation() );
}

bool DocWindow::isTyping()
{
    if (m_pDocMLE->hasFocus())
        return true;
    else
        return false;
}

void DocWindow::slotAssociationRemoved(AssociationWidget* association) {
    if (association == m_pAssocWidget || association->getUMLObject() == m_pUMLObject) {
        // In old code, the below line crashed (bugs.kde.org/89860)
        // A hotfix was made and detailed analysis was To Be Done:
        // newDocumentation()
        // However, it seems to have been fixed and the below line seems to work fine
        updateDocumentation(true);
    }
}

void DocWindow::slotWidgetRemoved(UMLWidget* widget) {
    if (widget == m_pUMLWidget || widget->getUMLObject() == m_pUMLObject) {
        updateDocumentation(true);
    }
}

#include "docwindow.moc"
