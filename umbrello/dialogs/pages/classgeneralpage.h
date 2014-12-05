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
class QRadioButton;
class QCheckBox;
class KComboBox;
class KLineEdit;
class KTextEdit;

class UMLObject;
class UMLWidget;
class ObjectWidget;
class UMLDoc;
class UMLStereotypeWidget;

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

    void updateObject();

private:
    UMLObject * m_pObject;
    ObjectWidget * m_pWidget;
    UMLWidget * m_pInstanceWidget;
    UMLDoc * m_pUmldoc;

    QGroupBox * m_docGB;
    QGroupBox * m_pButtonGB;
    QLabel * m_pNameL, * m_pInstanceL, * m_pStereoTypeL, * m_pPackageL;
    KLineEdit * m_pClassNameLE, * m_pInstanceLE, * m_pPackageLE;
    KComboBox * m_pPackageCB ;
    QRadioButton * m_pPublicRB, * m_pPrivateRB, * m_pProtectedRB, * m_pImplementationRB;
    QCheckBox * m_pMultiCB, * m_pDrawActorCB, * m_pAbstractCB, * m_pDeconCB;
    KTextEdit * m_doc;
    QGroupBox * m_pDrawAsGB;
    QRadioButton * m_pDefaultRB, * m_pFileRB, * m_pLibraryRB, * m_pTableRB;
    QCheckBox * m_pExecutableCB;
    UMLStereotypeWidget *m_stereotypeWidget;

    void insertStereotypesSorted(const QString& type);

public slots:

    void slotActorToggled(bool state);
};

#endif
