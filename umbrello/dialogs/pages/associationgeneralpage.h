/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ASSOCIATIONGENERALPAGE_H
#define ASSOCIATIONGENERALPAGE_H

#include "basictypes.h"
#include "dialogpagebase.h"

#include <QList>
#include <QWidget>

class AssociationWidget;
class DocumentationWidget;
class QCheckBox;
class QGridLayout;
class QLabel;
class KComboBox;
class QLineEdit;
class ObjectWidget;
class UMLDoc;
class UMLObject;

/**
 * Displays properties of a UMLObject in a dialog box.  This is not usually directly
 * called.  The class @ref AssociationPropertiesDialog will set this up for you.
 *
 * @short Display properties on a UMLObject.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class AssociationGeneralPage : public DialogPageBase
{
    Q_OBJECT
public:
    AssociationGeneralPage(QWidget *parent, AssociationWidget *a);
    ~AssociationGeneralPage();

    void apply();
    Q_SLOT void slotStereoCheckboxChanged(int state);

private:
    QGridLayout *m_pNameAndTypeLayout;
    QLabel    *m_pAssocNameL;
    QLineEdit *m_pAssocNameLE;   // is used if m_pStereoChkB is unchecked
    KComboBox *m_pAssocNameComB; // is used if m_pStereoChkB is checked
    QCheckBox *m_pStereoChkB;
    KComboBox *m_pTypeCB;

    /* Choices for the QComboBox, and we store ints and strings
       so we can translate both ways */
    QList<Uml::AssociationType::Enum> m_AssocTypes;
    QStringList m_AssocTypeStrings;
    DocumentationWidget *m_docWidget;

    AssociationWidget *m_pAssociationWidget;
    ObjectWidget *m_pWidget;

    void constructWidget();
};

#endif
