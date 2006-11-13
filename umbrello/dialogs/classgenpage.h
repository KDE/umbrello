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

#ifndef CLASSGENPAGE_H
#define CLASSGENPAGE_H

#include <qwidget.h>

class QGroupBox;
class QLabel;
class QLineEdit;
class QButtonGroup;
class QMultiLineEdit;
class QRadioButton;
class QCheckBox;
class KComboBox;

class UMLObject;
class UMLWidget;
class ObjectWidget;
class UMLDoc;

/**
 * Displays properties of a UMLObject in a dialog box.  This is not usually directly
 * called.  The class @ref ClassPropDlg will set this up for you.
 *
 * @short Display properties on a UMLObject.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassGenPage : public QWidget {
    Q_OBJECT
public:

    /**
     *  Sets up the ClassGenPage.
     *
     *  @param  d       The UMLDoc which controls controls object creation.
     *  @param  parent  The parent to the ClassGenPage.
     *  @param  o       The UMLObject to display the properties of.
     */
    ClassGenPage(UMLDoc *d, QWidget *parent, UMLObject * o);

    /**
     *  Sets up the ClassGenPage for an ObjectWidget
     *
     *  @param  d       The UMLDoc which controls controls object creation.
     *  @param  parent  The parent to the ClassGenPage.
     *  @param  o       The ObjectWidget to display the properties of.
     */
    ClassGenPage(UMLDoc *d, QWidget *parent, ObjectWidget * o);

    /**
     *  Sets up the ClassGenPage for a UMLWidget instance (used
     *  for component instances on deployment diagrams)
     *
     *  @param  d       The UMLDoc which controls controls object creation.
     *  @param  parent  The parent to the ClassGenPage.
     *  @param  widget  The UMLWidget to display the properties of.
     */
    ClassGenPage(UMLDoc* d, QWidget* parent, UMLWidget* widget);

    /**
     *  Standard deconstructor.
     */
    ~ClassGenPage();

    /**
     *  Will move information from the dialog into the object.
     *  Call when the ok or apply button is pressed.
     */
    void updateObject();
private:
    QGroupBox *m_pDocGB;
    QButtonGroup        * m_pButtonBG;
    QLabel * m_pNameL, * m_pInstanceL, * m_pStereoTypeL, * m_pPackageL;
    QLineEdit * m_pClassNameLE, *m_pInstanceLE, * m_pPackageLE;
    KComboBox * m_pStereoTypeCB;
    QRadioButton *m_pPublicRB, *m_pPrivateRB, *m_pProtectedRB, *m_pImplementationRB;
    QCheckBox * m_pMultiCB, * m_pDrawActorCB, * m_pAbstractCB, * m_pDeconCB;
    QMultiLineEdit * m_pDoc;
    UMLObject * m_pObject;
    UMLDoc * m_pUmldoc;
    ObjectWidget * m_pWidget;
    UMLWidget* m_pInstanceWidget;
    QButtonGroup* m_pDrawAsBG;
    QRadioButton* m_pDefaultRB,* m_pFileRB,* m_pLibraryRB,* m_pTableRB;
    QCheckBox* m_pExecutableCB;

    void insertStereotype(const QString& type, int index = -1);

public slots:
    /**
     *  When the draw as actor check box is toggled, the draw
     *  as multi instance need to be enabled/disabled.  They
     *  both can't be available at the same time.
     */
    void slotActorToggled( bool state );
};

#endif
