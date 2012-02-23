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
#include "docwindow.h"

// qt includes
#include <QtGui/QVBoxLayout>

// kde includes
#include <ktextedit.h>
#include <klocale.h>

// local includes
#include "associationwidget.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlview.h"
#include "umlwidget.h"
#include "umlscene.h"
#include "debug_utils.h"

/**
 * Constructor.
 */
DocWindow::DocWindow( UMLDoc * doc, QWidget *parent )
  : QWidget(parent),
    m_pUMLObject(0),
    m_pUMLView(0),
    m_pUMLDoc(doc),
    m_pUMLWidget(0),
    m_pAssocWidget(0),
    m_Showing(st_Project),
    m_modified(false)
{
    //setup visual display
    QVBoxLayout * docLayout = new QVBoxLayout( this );
    m_docTE = new KTextEdit( this );
    m_docTE->setText( "" );
    docLayout->addWidget( m_docTE);
    docLayout->setMargin(0);
    //m_docTE->setWordWrapMode(QTextEdit::WidgetWidth);

    //show projects documentation to start
    updateDocumentation( true, true );
    connect(m_docTE, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));
}

/**
 * Destructor.
 */
DocWindow::~DocWindow()
{
}

/**
 * Called when a widget wishes to display its documentation in the
 * doc window.  If there was already documentation there, that will
 * be updated before being removed from the view.
 *
 * Also call this function if you update the documentation in another
 * place, such as a properties dialog.  Just set overwrite to true.
 *
 * Overwrite is used when you believe that the documentation window
 * is already displaying documentation for the widget you wish to
 * display.
 * Overwrite just determines whose version is more up to date.
 */
void DocWindow::showDocumentation( UMLObject * object, bool overwrite )
{
    if( object == m_pUMLObject && !overwrite )
        return;
    if( object != m_pUMLObject )
        updateDocumentation( true );
    m_Showing = st_UMLObject;
    if( !object ) {
        m_docTE->setText( m_pUMLDoc->documentation() );
        m_modified = false;
        m_pUMLObject = 0;
        return;
    }
    m_pUMLObject = object;
    m_docTE->setText( m_pUMLObject->doc() );
    m_modified = false;
}

/**
 * This method is the same as the one for UMLObjects except it
 * displays documentation for a diagram.
 */
void DocWindow::showDocumentation( UMLView * view, bool overwrite )
{
    if( view == m_pUMLView && !overwrite )
        return;
    if( view != m_pUMLView ) {
        updateDocumentation( true );
    }
    m_Showing = st_UMLView;
    if( !view ) {
        m_docTE->setText( m_pUMLDoc->documentation() );
        m_pUMLView = 0;
        m_modified = false;
        return;
    }
    m_pUMLView = view;
    m_docTE->setText( m_pUMLView->umlScene()->documentation() );
    m_modified = false;
}

/**
 * This method is the same as the one for UMLObjects except it
 * displays documentation for an object instance (StateWidget/
 * ObjectWidget).
 */
void DocWindow::showDocumentation( UMLWidget * widget, bool overwrite )
{
    if( widget == m_pUMLWidget && !overwrite )
        return;
    if( widget != m_pUMLWidget )
        updateDocumentation( true );
    m_Showing = st_UMLWidget;
    if( !widget ) {
        m_docTE->setText( m_pUMLDoc->documentation() );
        m_pUMLWidget = 0;
        m_modified = false;
        return;
    }
    m_pUMLWidget = widget;
    m_docTE->setText( m_pUMLWidget->documentation() );
    m_modified = false;
}

/**
 * This method is the same as the one for UMLObjects except it
 * displays documentation for an association instance
 * (AssociationWidget).
 */
void DocWindow::showDocumentation( AssociationWidget * widget, bool overwrite )
{
    if( widget == m_pAssocWidget && !overwrite )
        return;
    if( widget != m_pAssocWidget )
        updateDocumentation( true );
    m_Showing = st_Association;
    if( !widget ) {
        m_docTE->setText( m_pUMLDoc->documentation() );
        m_pAssocWidget = 0;
        m_modified = false;
        return;
    }
    m_pAssocWidget = widget;
    m_docTE->setText( m_pAssocWidget->documentation() );
    m_modified = false;
}

/**
 * Call when you wish move changes in the doc window back into the
 * members documentation.
 *
 * If clear is true the doc window will display the documentation
 * for the current project instead of the widget documentation.
 *
 * This is usually called before displaying a properties dialog.
 *
 * @param clear     If true, show the documentation of current project
 * @param startup   If true, no setModified(true) calls will be done and nothing is pushed to the undo stack
 */
void DocWindow::updateDocumentation( bool clear, bool startup )
{
    // the file is marked modified, if the documentation differs
    // we don't do this on startup/load of a xmi file, because every time
    // modified is set, we get another undo/redo backup point
    if (isModified()) {
        if( m_pUMLObject ) {
            m_pUMLObject->setDoc( m_docTE->toPlainText() );
        } else if( m_pUMLView ) {
            m_pUMLView->umlScene()->setDocumentation( m_docTE->toPlainText() );
        } else if ( m_pUMLWidget ) {
            m_pUMLWidget->setDocumentation( m_docTE->toPlainText() );
        } else if ( m_pAssocWidget ) {
            m_pAssocWidget->setDocumentation( m_docTE->toPlainText() );
        } else {
            m_pUMLDoc->setDocumentation( m_docTE->toPlainText() );
        }

        // now do the setModified call
        if (startup == false) {
            m_pUMLDoc->setModified( true );
        }
    }

    // we should show the documentation of the whole project
    // FIXME: this is exactly what newDocumentation() does
    if( clear ) {
        m_docTE->setText( m_pUMLDoc->documentation() );
        m_modified = false;
        m_pUMLObject = 0;
        m_pUMLView = 0;
        m_pUMLWidget = 0;
        m_pAssocWidget = 0;
        m_Showing = st_Project;
    }
}

/**
 *  Re-initializes the class for a new document.
 */
void DocWindow::newDocumentation( )
{
    m_pUMLView = 0;
    m_pUMLObject = 0;
    m_pUMLWidget = 0;
    m_pAssocWidget = 0;
    m_Showing = st_Project;
    m_docTE->setText( m_pUMLDoc->documentation() );
    m_modified = false;
}

/**
 * Checks if the user is typing in the documentation edit window.
 */
bool DocWindow::isTyping()
{
    if (m_docTE->hasFocus())
        return true;
    else
        return false;
}

/**
 * check if the content in the documentation window has been changed
 * @return true when content has been changed
 */
bool DocWindow::isModified()
{
   return m_modified;
}

/**
 * An association was removed from the UMLView.
 * If the association removed was the association which documentation is
 * being shown, m_pAssocWidget is set to 0.
 */
void DocWindow::slotAssociationRemoved(AssociationWidget* association)
{
    if (association == m_pAssocWidget || association->umlObject() == m_pUMLObject) {
        // In old code, the below line crashed (bugs.kde.org/89860)
        // A hotfix was made and detailed analysis was To Be Done:
        // newDocumentation()
        // However, it seems to have been fixed and the below line seems to work fine
        updateDocumentation(true);
    }
}

/**
 * A widget was removed from the UMLView.
 * If the association removed was the association which documentation is
 * being shown, m_pUMLWidget is set to 0.
 */
void DocWindow::slotWidgetRemoved(UMLWidget* widget)
{
    if (widget == m_pUMLWidget || widget->umlObject() == m_pUMLObject) {
        updateDocumentation(true);
    }
}

/**
 * text from the edit field has been changed
 */
void DocWindow::slotTextChanged()
{
    m_modified = true;
}

#include "docwindow.moc"
