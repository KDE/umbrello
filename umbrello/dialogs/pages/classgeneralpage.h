/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CLASSGENERALPAGE_H
#define CLASSGENERALPAGE_H

#include "dialogpagebase.h"

class QGroupBox;
class QLabel;
class QCheckBox;
class KComboBox;
class KLineEdit;
class KTextEdit;

class UMLObject;
class UMLWidget;
class ObjectWidget;
class UMLDoc;
class UMLArtifactTypeWidget;
class UMLObjectNameWidget;
class UMLPackageWidget;
class UMLStereotypeWidget;
class VisibilityEnumWidget;

/**
 * Displays properties of a UMLObject in a dialog box.  This is not usually directly
 * called.  The class @ref ClassPropertiesDialog will set this up for you.
 *
 * @short Display properties on a UMLObject.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class ClassGeneralPage : public DialogPageBase
{
    Q_OBJECT
public:

    ClassGeneralPage(UMLDoc *d, QWidget *parent, UMLObject * o);
    ClassGeneralPage(UMLDoc *d, QWidget *parent, ObjectWidget * o);
    ClassGeneralPage(UMLDoc* d, QWidget* parent, UMLWidget* widget);
    ~ClassGeneralPage();

    void apply();

private:
    UMLObject * m_pObject;
    ObjectWidget * m_pWidget;
    UMLWidget * m_pInstanceWidget;
    UMLDoc * m_pUmldoc;

    QGroupBox * m_docGB;
    QLabel * m_pInstanceL, * m_pStereoTypeL;
    QCheckBox * m_pMultiCB, * m_pDrawActorCB, * m_pAbstractCB, * m_pDeconCB;
    KTextEdit * m_doc;
    QCheckBox * m_pExecutableCB;
    UMLObjectNameWidget *m_nameWidget;
    UMLObjectNameWidget *m_instanceNameWidget;
    UMLStereotypeWidget *m_stereotypeWidget;
    UMLPackageWidget *m_packageWidget;
    UMLArtifactTypeWidget *m_artifactTypeWidget;
    VisibilityEnumWidget *m_visibilityEnumWidget;

    void insertStereotypesSorted(const QString& type);

public slots:

    void slotActorToggled(bool state);
};

#endif
