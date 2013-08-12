/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ASSOCGENPAGE_H
#define ASSOCGENPAGE_H

#include "basictypes.h"

#include <QList>
#include <QWidget>

class AssociationWidget;
class KComboBox;
class KLineEdit;
class KTextEdit;
class ObjectWidget;
class UMLDoc;
class UMLObject;

/**
 * Displays properties of a UMLObject in a dialog box.  This is not usually directly
 * called.  The class @ref AssocPropDlg will set this up for you.
 *
 * @short Display properties on a UMLObject.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class AssocGenPage : public QWidget
{
    Q_OBJECT
public:
    AssocGenPage(UMLDoc *d, QWidget *parent, AssociationWidget *a);
    ~AssocGenPage();

    void updateObject();

private:
    KLineEdit *m_pAssocNameLE;
    KComboBox *m_pTypeCB;

    /* Choices for the QComboBox, and we store ints and strings
       so we can translate both ways */
    QList<Uml::AssociationType::Enum> m_AssocTypes;
    QStringList m_AssocTypeStrings;
    KTextEdit *m_doc;

    AssociationWidget *m_pAssociationWidget;
    UMLDoc *m_pUmldoc;
    ObjectWidget *m_pWidget;

    void constructWidget();

public slots:
//    /**
//     *  When the draw as actor check box is toggled, the draw
//     *  as multi instance need to be enabled/disabled.  They
//     *  both can't be available at the same time.
//     */
//    void slotActorToggled( bool state );

};

#endif
