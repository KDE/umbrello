/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DOCWINDOW_H
#define DOCWINDOW_H

#include <QtGui/QWidget>

class AssociationWidget;
class KTextEdit;
class UMLObject;
class UMLDoc;
class UMLView;
class UMLWidget;

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class DocWindow : public QWidget
{
    Q_OBJECT
public:
    explicit DocWindow( UMLDoc * doc, QWidget *parent = 0 );
    ~DocWindow();

    void showDocumentation( UMLObject * object, bool overwrite = false );
    void showDocumentation( UMLView * view, bool overwrite = false );
    void showDocumentation( UMLWidget * widget, bool overwrite = false );
    void showDocumentation( AssociationWidget * widget, bool overwrite = false );

    void updateDocumentation( bool clear = false, bool startup = false );

    void newDocumentation( );

    bool isTyping();
    bool isModified();

public slots:
    void slotAssociationRemoved(AssociationWidget* association);
    void slotWidgetRemoved(UMLWidget* widget);

private slots:
    void slotTextChanged();

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

    UMLObject * m_pUMLObject;  ///< The UMLObject we are going to show documentation.
    UMLView *   m_pUMLView;    ///< The UMLView we are going to show documentation.
    UMLDoc *    m_pUMLDoc;     ///< The Project we are going to show documentation.
    UMLWidget * m_pUMLWidget;  ///< The UMLWidget we are going to show documentation.
    AssociationWidget * m_pAssocWidget;  ///< The association we are going to show documentation.

    Showing_Type m_Showing;  ///< Which type of documentation we are showing.

    //visual widgets
    KTextEdit *  m_docTE;

    bool m_modified; ///< state of text content
};

#endif
