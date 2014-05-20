/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ASSOCIATIONROLEPAGE_H
#define ASSOCIATIONROLEPAGE_H

#include "dialogpagebase.h"

class AssociationWidget;
class KComboBox;
class KLineEdit;
class KTextEdit;
class ObjectWidget;
class QRadioButton;
class UMLDoc;
class UMLObject;

/**
 * Displays properties of a UMLObject in a dialog box.  This is not usually directly
 * called.  The class @ref AssocPropDialog will set this up for you.
 *
 * @short Display properties on a UMLObject.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class AssociationRolePage : public DialogPageBase
{
    Q_OBJECT
public:
    AssociationRolePage(UMLDoc *d, QWidget *parent, AssociationWidget *a);
    ~AssociationRolePage();

    void updateObject();

private:
    KLineEdit *m_pRoleALE, *m_pRoleBLE;
    KComboBox *m_pMultiACB, *m_pMultiBCB;
    KTextEdit *m_docA, *m_docB;
    AssociationWidget *m_pAssociationWidget;
    UMLDoc *m_pUmldoc;
    ObjectWidget *m_pWidget;
    QRadioButton *m_PublicARB, *m_ProtectedARB, *m_PrivateARB, *m_ImplementationARB;
    QRadioButton *m_PublicBRB, *m_ProtectedBRB, *m_PrivateBRB, *m_ImplementationBRB;
    QRadioButton *m_ChangeableARB, *m_AddOnlyARB, *m_FrozenARB;
    QRadioButton *m_ChangeableBRB, *m_AddOnlyBRB, *m_FrozenBRB;

    void constructWidget();

public slots:
//    /**
//     *  When the draw as actor check box is toggled, the draw
//     *  as multi instance need to be enabled/disabled.  They
//     *  both can't be available at the same time.
//     */
//    void slotActorToggled(bool state);

};

#endif
