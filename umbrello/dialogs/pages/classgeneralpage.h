/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CLASSGENERALPAGE_H
#define CLASSGENERALPAGE_H

#include "dialogpagebase.h"
#include "n_stereoattrs.h"

class DocumentationWidget;
class QGroupBox;
class QGridLayout;
class QLabel;
class QRadioButton;
class QCheckBox;
class QLineEdit;
class KComboBox;
class KLineEdit;

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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ClassGeneralPage : public DialogPageBase
{
    Q_OBJECT
public:

    /**
     * Sets up the ClassGenPage.
     * @param  d       The UMLDoc which controls controls object creation.
     * @param  parent  The parent to the ClassGenPage.
     * @param  o       The UMLObject to display the properties of.
     */
    ClassGeneralPage(UMLDoc *d, QWidget *parent, UMLObject * o);

    /**
     * Sets up the ClassGenPage for an ObjectWidget
     * @param  d       The UMLDoc which controls controls object creation.
     * @param  parent  The parent to the ClassGenPage.
     * @param  o       The ObjectWidget to display the properties of.
     */
    ClassGeneralPage(UMLDoc *d, QWidget *parent, ObjectWidget * o);

    /**
     *  Sets up the ClassGenPage for a UMLWidget instance (used
     *  for component instances on deployment diagrams)
     *
     *  @param  d       The UMLDoc which controls controls object creation.
     *  @param  parent  The parent to the ClassGenPage.
     *  @param  widget  The UMLWidget to display the properties of.
     */
    ClassGeneralPage(UMLDoc* d, QWidget* parent, UMLWidget* widget);

    /**
     * Standard deconstructor.
     */
    ~ClassGeneralPage();

    void apply();

private:
    UMLObject * m_pObject;
    ObjectWidget * m_pWidget;
    UMLWidget * m_pInstanceWidget;
    UMLDoc * m_pUmldoc;

    QGridLayout * m_pNameLayout;
    QCheckBox * m_pMultiCB, * m_pDrawActorCB, * m_pAbstractCB, * m_pDeconCB;
    QCheckBox * m_pExecutableCB;
    DocumentationWidget *m_docWidget;
    UMLObjectNameWidget *m_nameWidget;
    UMLObjectNameWidget *m_instanceNameWidget;
    UMLStereotypeWidget *m_stereotypeWidget;
    QLabel              *m_pTagL [N_STEREOATTRS];
    KLineEdit           *m_pTagLE[N_STEREOATTRS];
    UMLPackageWidget *m_packageWidget;
    UMLArtifactTypeWidget *m_artifactTypeWidget;
    VisibilityEnumWidget *m_visibilityEnumWidget;

    void insertStereotypesSorted(const QString& type);
    void setInstanceWidgetNameIfApplicable(const QString& name) const;

public slots:
    void slotStereoTextChanged(const QString &);
    void slotActorToggled(bool state);
};

#endif
