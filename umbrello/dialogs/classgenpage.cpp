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

// my own header
#include "classgenpage.h"

// qt includes
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qmultilineedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

// kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kcombobox.h>

// my class includes
#include "../umlobject.h"
#include "../objectwidget.h"
#include "../uml.h"
#include "../umldoc.h"
#include "../artifact.h"
#include "../component.h"
#include "../umlview.h"
#include "../stereotype.h"

ClassGenPage::ClassGenPage(UMLDoc* d, QWidget* parent, UMLObject* o) : QWidget(parent) {
    m_pWidget = 0;
    m_pObject = 0;
    m_pInstanceWidget = 0;
    QString name;
    int margin = fontMetrics().height();
    Uml::Object_Type t = o -> getBaseType();
    m_pUmldoc = d;
    if (t == Uml::ot_Class) {
        name = i18n("Class &name:");
    } else if (t == Uml::ot_Actor) {
        name = i18n("Actor &name:");
    } else if (t == Uml::ot_Package) {
        name = i18n("Package &name:");
    } else if (t == Uml::ot_UseCase) {
        name = i18n("Use case &name:");
    } else if (t == Uml::ot_Interface) {
        name = i18n("Interface &name:");
    } else if (t == Uml::ot_Component) {
        name = i18n("Component &name:");
    } else if (t == Uml::ot_Artifact) {
        name = i18n("Artifact &name:");
    } else if (t == Uml::ot_Enum) {
        name = i18n("Enum &name:");
    } else if (t == Uml::ot_Datatype) {
        name = i18n("Datatype &name:");
    } else if (t == Uml::ot_Entity) {
        name = i18n("Entity &name:");
    } else {
        kWarning() << "creating class gen page for unknown widget type" << endl;
    }
    setMinimumSize(310,330);
    QVBoxLayout * topLayout = new QVBoxLayout(this);
    topLayout -> setSpacing(6);

    //setup name
    QGridLayout * m_pNameLayout = new QGridLayout(topLayout, 4, 2);
    m_pNameLayout -> setSpacing(6);
    m_pNameL = new QLabel(this);
    m_pNameL -> setText(name);
    m_pNameLayout -> addWidget(m_pNameL, 0, 0);

    m_pClassNameLE = new QLineEdit(this);
    m_pNameLayout -> addWidget(m_pClassNameLE, 0, 1);
    m_pClassNameLE->setFocus();
    m_pNameL->setBuddy(m_pClassNameLE);

    m_pStereoTypeCB = 0;
    m_pPackageLE = 0;
    m_pAbstractCB = 0;
    m_pDeconCB = 0;

    m_pStereoTypeL = new QLabel(i18n("&Stereotype name:"), this);
    m_pNameLayout -> addWidget(m_pStereoTypeL, 1, 0);

    m_pStereoTypeCB = new KComboBox(true, this);
    m_pNameLayout -> addWidget(m_pStereoTypeCB, 1, 1);

    m_pStereoTypeCB->setCurrentText( o->getStereotype() );
    m_pStereoTypeL->setBuddy(m_pStereoTypeCB);

    if (t == Uml::ot_Interface || t == Uml::ot_Datatype || t == Uml::ot_Enum) {
        m_pStereoTypeCB->setEditable(false);
    }

    if (t == Uml::ot_Class || t == Uml::ot_Interface) {
        m_pPackageL = new QLabel(i18n("&Package name:"), this);
        m_pNameLayout -> addWidget(m_pPackageL, 2, 0);

        m_pPackageLE = new QLineEdit(this);
        m_pNameLayout -> addWidget(m_pPackageLE, 2, 1);

        m_pPackageLE -> setText(o -> getPackage());
        m_pPackageLE -> setEnabled(false);
        m_pPackageL->setBuddy(m_pPackageLE);
    }

    if (t == Uml::ot_Class || t == Uml::ot_UseCase ) {
        QString abstractCaption;
        if ( t == Uml::ot_Class  ) {
            abstractCaption = i18n("A&bstract class");
        } else {
            abstractCaption = i18n("A&bstract use case");
        }
        m_pAbstractCB = new QCheckBox( abstractCaption, this );
        m_pAbstractCB -> setChecked( o -> getAbstract() );
        m_pNameLayout -> addWidget( m_pAbstractCB, 3, 0 );
    }

    if (t == Uml::ot_Component) {
        m_pExecutableCB = new QCheckBox(i18n("&Executable"), this);
        m_pExecutableCB->setChecked( (static_cast<UMLComponent*>(o))->getExecutable() );
        m_pNameLayout->addWidget( m_pExecutableCB, 3, 0 );
    }

    if (t == Uml::ot_Artifact) {
        //setup artifact draw as
        m_pDrawAsBG = new QButtonGroup(i18n("Draw As"), this);
        QHBoxLayout* drawAsLayout = new QHBoxLayout(m_pDrawAsBG);
        drawAsLayout->setMargin(margin);
        m_pDrawAsBG->setExclusive(true);

        m_pDefaultRB = new QRadioButton(i18n("&Default"), m_pDrawAsBG);
        drawAsLayout->addWidget(m_pDefaultRB);

        m_pFileRB = new QRadioButton(i18n("&File"), m_pDrawAsBG);
        drawAsLayout->addWidget(m_pFileRB);

        m_pLibraryRB = new QRadioButton(i18n("&Library"), m_pDrawAsBG);
        drawAsLayout->addWidget(m_pLibraryRB);

        m_pTableRB = new QRadioButton(i18n("&Table"), m_pDrawAsBG);
        drawAsLayout->addWidget(m_pTableRB);

        topLayout->addWidget(m_pDrawAsBG);

        UMLArtifact::Draw_Type drawAs = (static_cast<UMLArtifact*>(o))->getDrawAsType();

        if (drawAs == UMLArtifact::file) {
            m_pFileRB->setChecked(true);
        } else if (drawAs == UMLArtifact::library) {
            m_pLibraryRB->setChecked(true);
        } else if (drawAs == UMLArtifact::table) {
            m_pTableRB->setChecked(true);
        } else {
            m_pDefaultRB->setChecked(true);
        }
    }

    //setup scope
    m_pButtonBG = new QButtonGroup(i18n("Visibility"), this);
    QHBoxLayout * scopeLayout = new QHBoxLayout(m_pButtonBG);
    scopeLayout -> setMargin(margin);
    m_pButtonBG -> setExclusive(true);

    m_pPublicRB = new QRadioButton(i18n("P&ublic"), m_pButtonBG);
    scopeLayout -> addWidget(m_pPublicRB);

    m_pPrivateRB = new QRadioButton(i18n("P&rivate"), m_pButtonBG);
    scopeLayout -> addWidget(m_pPrivateRB);

    m_pProtectedRB = new QRadioButton(i18n("Pro&tected"), m_pButtonBG);
    scopeLayout -> addWidget(m_pProtectedRB);
    topLayout -> addWidget(m_pButtonBG);

    m_pImplementationRB = new QRadioButton(i18n("Imple&mentation"), m_pButtonBG);
    scopeLayout -> addWidget(m_pImplementationRB);
    topLayout -> addWidget(m_pButtonBG);
    //setup documentation
    m_pDocGB = new QGroupBox(this);
    QHBoxLayout * docLayout = new QHBoxLayout(m_pDocGB);
    docLayout -> setMargin(margin);
    m_pDocGB -> setTitle(i18n("Documentation"));

    m_pDoc = new QMultiLineEdit(m_pDocGB);
    docLayout -> addWidget(m_pDoc);
    topLayout -> addWidget(m_pDocGB);

    m_pObject = o;
    //setup fields
    m_pClassNameLE -> setText(o -> getName());
    m_pDoc-> setText(o -> getDoc());
    Uml::Visibility s = o -> getVisibility();
    if(s == Uml::Visibility::Public)
        m_pPublicRB->setChecked(true);
    else if(s == Uml::Visibility::Private)
        m_pPrivateRB->setChecked(true);
    else if(s == Uml::Visibility::Protected)
          m_pProtectedRB->setChecked(true);
    else
        m_pImplementationRB -> setChecked(true);

    // manage stereotypes
    m_pStereoTypeCB -> setDuplicatesEnabled(false);//only allow one of each type in box
    m_pStereoTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    insertStereotype (QString("")); // an empty stereotype is the default
    int defaultStereotype=0;
    bool foundDefaultStereotype = false;
    for (UMLStereotypeListIt it(m_pUmldoc->getStereotypes()); it.current(); ++it) {
        if (!foundDefaultStereotype) {
            if ( m_pObject->getStereotype() == it.current()->getName()) {
                foundDefaultStereotype = true;
            }
            defaultStereotype++;
        }
        insertStereotype (it.current()->getName());
    }
    // lookup for a default stereotype, if the operation doesn't have one
    if (foundDefaultStereotype)
        m_pStereoTypeCB -> setCurrentItem(defaultStereotype);
    else
        m_pStereoTypeCB -> setCurrentItem(-1);

    ///////////
    m_pDoc->setWordWrap(QMultiLineEdit::WidgetWidth);
    //////////
}

ClassGenPage::ClassGenPage(UMLDoc* d, QWidget* parent, ObjectWidget* o) : QWidget(parent) {
    m_pObject = 0;
    m_pInstanceWidget = 0;
    m_pWidget = o;
    m_pDeconCB = 0;
    m_pMultiCB = 0;
    int margin = fontMetrics().height();
    //int t = o -> getBaseType();
    m_pUmldoc = d;
    setMinimumSize(310,330);
    QGridLayout * topLayout = new QGridLayout(this, 2, 1);
    topLayout -> setSpacing(6);

    //setup name
    QGridLayout * m_pNameLayout = new QGridLayout(topLayout, 3, 2);
    m_pNameLayout -> setSpacing(6);
    m_pNameL = new QLabel(this);
    m_pNameL -> setText(i18n("Class name:"));
    m_pNameLayout -> addWidget(m_pNameL, 0, 0);

    m_pClassNameLE = new QLineEdit(this);
    m_pClassNameLE -> setText(o -> getName());
    m_pNameLayout -> addWidget(m_pClassNameLE, 0, 1);

    m_pInstanceL = new QLabel(this);
    m_pInstanceL -> setText(i18n("Instance name:"));
    m_pNameLayout -> addWidget(m_pInstanceL, 1, 0);

    m_pInstanceLE = new QLineEdit(this);
    m_pInstanceLE -> setText(o -> getInstanceName());
    m_pNameLayout -> addWidget(m_pInstanceLE, 1, 1);
    UMLView *view = UMLApp::app()->getCurrentView();

    m_pDrawActorCB = new QCheckBox( i18n( "Draw as actor" ) , this );
    m_pDrawActorCB -> setChecked( o -> getDrawAsActor() );
    m_pNameLayout -> addWidget( m_pDrawActorCB, 2, 0 );

    if(view -> getType() == Uml::dt_Collaboration) {
        m_pMultiCB = new QCheckBox(i18n("Multiple instance"), this);
        m_pMultiCB -> setChecked(o -> getMultipleInstance());
        m_pNameLayout -> addWidget(m_pMultiCB, 2,1);
        if( m_pDrawActorCB -> isChecked() )
            m_pMultiCB -> setEnabled( false );
    } else//sequence diagram
    {
        m_pDeconCB = new QCheckBox(i18n("Show destruction"), this);
        m_pDeconCB->setChecked(o->getShowDestruction());
        m_pNameLayout -> addWidget(m_pDeconCB, 2,1);
    }
    //setup documentation
    m_pDocGB = new QGroupBox(this);
    topLayout -> addWidget(m_pDocGB, 1, 0);
    QHBoxLayout * docLayout = new QHBoxLayout(m_pDocGB);
    docLayout -> setMargin(margin);
    m_pDocGB -> setTitle(i18n("Documentation"));

    m_pDoc = new QMultiLineEdit(m_pDocGB);
    m_pDoc->setWordWrap(QMultiLineEdit::WidgetWidth);
    m_pDoc-> setText(o -> getDoc());
    docLayout -> addWidget(m_pDoc);
    m_pObject = 0;//needs to be set to zero
    if( m_pMultiCB )
        connect( m_pDrawActorCB, SIGNAL( toggled( bool ) ), this, SLOT( slotActorToggled( bool ) ) );
}

ClassGenPage::ClassGenPage(UMLDoc* d, QWidget* parent, UMLWidget* widget) : QWidget(parent) {
    m_pWidget = 0;
    m_pObject = 0;
    m_pInstanceWidget = widget;
    m_pDeconCB = 0;
    m_pMultiCB = 0;
    int margin = fontMetrics().height();
    //int t = o -> getBaseType();
    m_pUmldoc = d;
    setMinimumSize(310,330);
    QGridLayout* topLayout = new QGridLayout(this, 2, 1);
    topLayout->setSpacing(6);

    //setup name
    QGridLayout* m_pNameLayout = new QGridLayout(topLayout, 3, 2);
    m_pNameLayout->setSpacing(6);
    m_pNameL = new QLabel(this);
    if (widget->getBaseType() == Uml::wt_Component) {
        m_pNameL->setText(i18n("Component name:"));
    } else if (widget->getBaseType() == Uml::wt_Node) {
        m_pNameL->setText(i18n("Node name:"));
    } else {
        kWarning() << "ClassGenPage called on unknown widget type" << endl;
    }
    m_pNameLayout->addWidget(m_pNameL, 0, 0);

    m_pClassNameLE = new QLineEdit(this);
    m_pClassNameLE->setText(widget->getName());
    m_pNameLayout->addWidget(m_pClassNameLE, 0, 1);

    m_pStereoTypeL = new QLabel(i18n("Stereotype name:"), this);
    m_pNameLayout->addWidget(m_pStereoTypeL, 1, 0);

    m_pStereoTypeCB = new KComboBox(true, this);
    m_pNameLayout->addWidget(m_pStereoTypeCB, 1, 1);

    m_pStereoTypeCB->setCurrentText( widget->getUMLObject()->getStereotype() );
    m_pStereoTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );

    m_pInstanceL = new QLabel(this);
    m_pInstanceL->setText(i18n("Instance name:"));
    m_pNameLayout->addWidget(m_pInstanceL, 2, 0);

    m_pInstanceLE = new QLineEdit(this);
    m_pInstanceLE->setText(widget->getInstanceName());
    m_pNameLayout->addWidget(m_pInstanceLE, 2, 1);

    //setup documentation
    m_pDocGB = new QGroupBox(this);
    topLayout->addWidget(m_pDocGB, 1, 0);
    QHBoxLayout* docLayout = new QHBoxLayout(m_pDocGB);
    docLayout->setMargin(margin);
    m_pDocGB->setTitle(i18n("Documentation"));

    m_pDoc = new QMultiLineEdit(m_pDocGB);
    m_pDoc->setWordWrap(QMultiLineEdit::WidgetWidth);
    m_pDoc->setText(widget->getDoc());
    docLayout->addWidget(m_pDoc);
    m_pObject = 0;//needs to be set to zero
}

ClassGenPage::~ClassGenPage() {}

void ClassGenPage::insertStereotype( const QString& type, int index )
{
    m_pStereoTypeCB->insertItem( type, index );
    m_pStereoTypeCB->completionObject()->addItem( type );
}

void ClassGenPage::updateObject() {
    if(m_pObject) {
        QString name = m_pClassNameLE -> text();

        m_pObject -> setDoc(m_pDoc -> text());

        if(m_pStereoTypeCB)
            m_pObject -> setStereotype(m_pStereoTypeCB->currentText());
        /**
         * @todo enable the package lineedit field amd add logic for changing the package
        if(m_pPackageLE)
            m_pObject -> setPackage(m_pPackageLE -> text());
         */

        if ( m_pObject->getUMLPackage() == NULL ) {
            kDebug() << k_funcinfo << "Parent package not set, setting it to Logical View folder"<<endl;
            UMLFolder* folder = m_pUmldoc->getRootFolder(  Uml::mt_Logical );
            m_pObject->setUMLPackage( ( UMLPackage* )folder );
        }

        if( m_pAbstractCB )
            m_pObject -> setAbstract( m_pAbstractCB -> isChecked() );
        //make sure unique name
        UMLObject *o = m_pUmldoc -> findUMLObject(name);
        if(o && m_pObject != o) {
            KMessageBox::sorry(this, i18n("The name you have chosen\nis already being used.\nThe name has been reset."),
                               i18n("Name is Not Unique"), false);
            m_pClassNameLE -> setText( m_pObject -> getName() );
        } else
            m_pObject -> setName(name);
        Uml::Visibility s;
        if(m_pPublicRB -> isChecked())
          s = Uml::Visibility::Public;
        else if(m_pPrivateRB -> isChecked())
          s = Uml::Visibility::Private;
        else if(m_pProtectedRB->isChecked())
          s = Uml::Visibility::Protected;
        else
          s = Uml::Visibility::Implementation;
        m_pObject -> setVisibility(s);

        if (m_pObject->getBaseType() == Uml::ot_Component) {
            (static_cast<UMLComponent*>(m_pObject))->setExecutable( m_pExecutableCB->isChecked() );
        }

        if (m_pObject->getBaseType() == Uml::ot_Artifact) {
            UMLArtifact::Draw_Type drawAsType;
            if ( m_pFileRB->isChecked() ) {
                drawAsType = UMLArtifact::file;
            } else if ( m_pLibraryRB->isChecked() ) {
                drawAsType = UMLArtifact::library;
            } else if (m_pTableRB->isChecked() ) {
                drawAsType = UMLArtifact::table;
            } else {
                drawAsType = UMLArtifact::defaultDraw;
            }
            (static_cast<UMLArtifact*>(m_pObject))->setDrawAsType(drawAsType);
        }

    }//end if m_pObject
    else if(m_pWidget) {
        m_pWidget -> setInstanceName(m_pInstanceLE -> text());
        if(m_pMultiCB)
            m_pWidget -> setMultipleInstance(m_pMultiCB -> isChecked());
        m_pWidget -> setDrawAsActor( m_pDrawActorCB -> isChecked() );
        if( m_pDeconCB )
            m_pWidget -> setShowDestruction( m_pDeconCB -> isChecked() );
        QString name = m_pClassNameLE -> text();
        m_pWidget -> setDoc(m_pDoc -> text());
        UMLObject * o = m_pWidget -> getUMLObject();
        UMLObject * old = m_pUmldoc -> findUMLObject(name);
        if(old && o != old) {
            KMessageBox::sorry(this, i18n("The name you have chosen\nis already being used.\nThe name has been reset."),
                               i18n("Name is Not Unique"), false);
        } else
            o -> setName(name);
    } else if (m_pInstanceWidget) {
        m_pInstanceWidget->setInstanceName(m_pInstanceLE->text());
        QString name = m_pClassNameLE->text();
        m_pInstanceWidget->setDoc(m_pDoc->text());
        UMLObject* o = m_pInstanceWidget->getUMLObject();
        UMLObject* old = m_pUmldoc->findUMLObject(name);
        if(old && o != old) {
            KMessageBox::sorry(this, i18n("The name you have chosen\nis already being used.\nThe name has been reset."),
                               i18n("Name is Not Unique"), false);
        } else {
            o->setName(name);
        }
        o->setStereotype( m_pStereoTypeCB->currentText() );
    }
}

void ClassGenPage::slotActorToggled( bool state ) {
    if( m_pMultiCB )
        m_pMultiCB -> setEnabled( !state );
}



#include "classgenpage.moc"
