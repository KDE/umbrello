/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ASSOCIATIONROLEPAGE_H
#define ASSOCIATIONROLEPAGE_H

#include "dialogpagebase.h"

class AssociationWidget;
class KComboBox;
class QLineEdit;
class KTextEdit;
class ObjectWidget;
class QRadioButton;
class UMLDoc;
class UMLObject;
class VisibilityEnumWidget;

/**
 * Displays properties of a UMLObject in a dialog box.  This is not usually directly
 * called.  The class @ref AssociationPropertiesDialog will set this up for you.
 *
 * @short Display properties on a UMLObject.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class AssociationRolePage : public DialogPageBase
{
    Q_OBJECT
public:
    AssociationRolePage(QWidget *parent, AssociationWidget *a);
    ~AssociationRolePage();

    void apply();

private:
    QLineEdit *m_pRoleALE, *m_pRoleBLE;
    KComboBox *m_pMultiACB, *m_pMultiBCB;
    KTextEdit *m_docA, *m_docB;
    AssociationWidget *m_pAssociationWidget;
    ObjectWidget *m_pWidget;
    QRadioButton *m_ChangeableARB, *m_AddOnlyARB, *m_FrozenARB;
    QRadioButton *m_ChangeableBRB, *m_AddOnlyBRB, *m_FrozenBRB;
    VisibilityEnumWidget *m_visibilityWidgetA;
    VisibilityEnumWidget *m_visibilityWidgetB;

    void constructWidget();
};

#endif
