/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DOCWINDOW_H
#define DOCWINDOW_H

#include <qwidget.h>

class AssociationWidget;
class QGroupBox;
class QMultiLineEdit;
class UMLObject;
class UMLDoc;
class UMLView;
class UMLWidget;

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class DocWindow : public QWidget {
    Q_OBJECT
public:
    /**
     * Constructor
     */
    explicit DocWindow( UMLDoc * doc, QWidget *parent = 0, const char *name = 0 );

    /**
     * Deconstructor
     */
    ~DocWindow();

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
    void showDocumentation( UMLObject * object, bool overwrite = false );

    /**
     * This method is the same as the one for UMLObjects except it
     * displays documentation for a diagram.
     */
    void showDocumentation( UMLView * view, bool overwrite = false );

    /**
     * This method is the same as the one for UMLObjects except it
     * displays documentation for an object instance (StateWidget/
     * ObjectWidget).
     */
    void showDocumentation( UMLWidget * widget, bool overwrite = false );

    /**
     * This method is the same as the one for UMLObjects except it
     * displays documentation for an association instance
     * (AssociationWidget).
     */
    void showDocumentation( AssociationWidget * widget, bool overwrite = false );

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
    void updateDocumentation( bool clear = false, bool startup = false );


    /**
     *  Re-initializes the class for a new document.
     */
    void newDocumentation( );

    /**
     * Checks if the user is typing in the documentation edit window
     */
    bool isTyping();

public slots:

    /**
     * An association was removed from the UMLView.
     * If the association removed was the association which documentation is
     * being shown, m_pAssocWidget is set to 0.
     */
    void slotAssociationRemoved(AssociationWidget* association);

    /**
     * A widget was removed from the UMLView.
     * If the association removed was the association which documentation is
     * being shown, m_pUMLWidget is set to 0.
     */
    void slotWidgetRemoved(UMLWidget* widget);

private:
    /**
     * Used internally to know which type of object we are showing
     * documentation for.
     */
    enum Showing_Type {
        st_Project,
        st_UMLView,
        st_UMLObject,
        st_UMLWidget,
        st_Association
    };

    /**
     * A pointer to the UMLObject we are going to show documentation.
     */
    UMLObject * m_pUMLObject;

    /**
     * A pointer to the UMLView we are going to show documentation.
     */
    UMLView * m_pUMLView;

    /**
     * A pointer to the Project we are going to show documentation.
     */
    UMLDoc * m_pUMLDoc;

    /**
     * A pointer to the UMLWidget we are going to show documentation.
     */
    UMLWidget * m_pUMLWidget;

    /**
     * A pointer to the association we are going to show documentation.
     */
    AssociationWidget * m_pAssocWidget;

    /**
     * Which type of documentation we are showing.
     */
    Showing_Type m_Showing;

    //visual widgets
    QMultiLineEdit      * m_pDocMLE;
    QGroupBox   * m_pDocGB;

};

#endif
