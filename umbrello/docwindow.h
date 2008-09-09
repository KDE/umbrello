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
class KTextEdit;
class UMLObject;
class UMLDoc;
class UMLView;
class UMLRectWidget;

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class DocWindow : public QWidget
{
    Q_OBJECT
public:
    /**
     * Constructor
     */
    explicit DocWindow( UMLDoc * doc, QWidget *parent = 0 );

    /**
     * Deconstructor
     */
    ~DocWindow();

    void showDocumentation( UMLObject * object, bool overwrite = false );

    void showDocumentation( UMLView * view, bool overwrite = false );

    void showDocumentation( UMLRectWidget * widget, bool overwrite = false );

    void showDocumentation( AssociationWidget * widget, bool overwrite = false );

    void updateDocumentation( bool clear = false, bool startup = false );


    void newDocumentation( );

    bool isTyping();

public slots:

    void slotAssociationRemoved(AssociationWidget* association);

    void slotWidgetRemoved(UMLRectWidget* widget);

private:

    /**
     * Used internally to know which type of object we are showing
     * documentation for.
     */
    enum Showing_Type {
        st_Project,
        st_UMLView,
        st_UMLObject,
        st_UMLRectWidget,
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
     * A pointer to the UMLRectWidget we are going to show documentation.
     */
    UMLRectWidget * m_pUMLWidget;

    /**
     * A pointer to the association we are going to show documentation.
     */
    AssociationWidget * m_pAssocWidget;

    /**
     * Which type of documentation we are showing.
     */
    Showing_Type m_Showing;

    //visual widgets
    KTextEdit      * m_pDocTE;

};

#endif
