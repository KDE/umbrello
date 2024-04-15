/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DOCWINDOW_H
#define DOCWINDOW_H

#include <QWidget>

class AssociationWidget;
class KTextEdit;
class ModifiedWidget;
class QLabel;
class UMLObject;
class UMLDoc;
class UMLScene;
class UMLWidget;

/**
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class DocWindow : public QWidget
{
    Q_OBJECT
public:
    explicit DocWindow(UMLDoc * doc, QWidget *parent = nullptr);
    ~DocWindow();

    void showDocumentation(UMLObject * object, bool overwrite = false);
    void showDocumentation(UMLScene * scene, bool overwrite = false);
    void showDocumentation(UMLWidget * widget, bool overwrite = false);
    void showDocumentation(AssociationWidget * widget, bool overwrite = false);

    void updateDocumentation(bool clear = false, bool startup = false);

    void reset();

    bool isTyping() const;
    void setFocus();

public Q_SLOTS:
    void slotAssociationRemoved(AssociationWidget* association);
    void slotWidgetRemoved(UMLWidget* widget);

private Q_SLOTS:
    void slotTextChanged();
    void slotFocusEnabledChanged(int state);

private:
    /**
     * Used internally to know which type of object we are showing
     * documentation for.
     */
    enum ShowingType {
        st_Project,
        st_UMLScene,
        st_UMLObject,
        st_UMLWidget,
        st_Association
    };

    UMLObject         *m_pUMLObject;    ///< The UMLObject we are going to show documentation.
    UMLScene          *m_pUMLScene;     ///< The UMLScene we are going to show documentation.
    UMLDoc            *m_pUMLDoc;       ///< The Project we are going to show documentation.
    UMLWidget         *m_pUMLWidget;    ///< The UMLWidget we are going to show documentation.
    AssociationWidget *m_pAssocWidget;  ///< The association we are going to show documentation.

    ShowingType m_Showing;  ///< Which type of documentation we are showing.

    QLabel    *m_typeLabel;      ///< label for type icon
    QLabel    *m_nameLabel;      ///< label for name text
    ModifiedWidget *m_modifiedWidget;  ///< label for modified flag icon
    QTextEdit *m_docTE;          ///< documentation widget
    bool m_focusEnabled;

    bool isModified() const;
    QLabel* createPixmapLabel();
    void updateLabel(const QString &name = QString());
    void toForeground();
};

#endif
