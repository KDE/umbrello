/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlviewdialog.h"

// qt/kde includes
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <q3vbox.h>
#include <QtGui/QLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <q3groupbox.h>
#include <q3textedit.h>
#include <QtGui/QSpinBox>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfontdialog.h>
#include <kdebug.h>
#include <knuminput.h>
#include <kvbox.h>

// local includes
#include "umlscene.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "icon_utils.h"


UMLViewDialog::UMLViewDialog( QWidget * pParent, UMLScene * pScene )
    : KPageDialog(pParent)
{
    setCaption( i18n("Properties") );
    setButtons( Ok | Apply | Cancel | Help );
    setDefaultButton( Ok );
    setModal( true );
    setFaceType( KPageDialog::List );
    showButtonSeparator( true );
    m_pScene = pScene;
    m_options = m_pScene -> getOptionState();
    setupPages();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

UMLViewDialog::~UMLViewDialog()
{
}

void UMLViewDialog::slotOk()
{
    applyPage( pageGeneralItem );
    applyPage( pageDisplayItem );
    applyPage( pageFontItem );
    applyPage( pageColorItem );
    accept();
}

void UMLViewDialog::slotApply()
{
    applyPage( currentPage() );
}

void UMLViewDialog::setupPages()
{
    setupDiagramPropertiesPage();
    setupColorPage();
    setupFontPage();
    setupClassPage();
}

void UMLViewDialog::setupDiagramPropertiesPage()
{
    KVBox *page = new KVBox();
    pageGeneralItem = new KPageWidgetItem( page, i18nc("general settings page", "General") );
    pageGeneralItem->setHeader( i18n("General Settings") );
    pageGeneralItem->setIcon(Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_General) );
    addPage( pageGeneralItem);

    m_diagramProperties = new DiagramPropertiesPage(page);

    m_diagramProperties->diagramName->setText( m_pScene->getName() );
    // [PORT]
    // m_diagramProperties->zoom->setValue(m_pScene->activeView()->currentZoom());
    m_diagramProperties->showOpSigs->setChecked( m_pScene->getShowOpSig() );

    m_diagramProperties->showGrid->setChecked(m_pScene -> getShowSnapGrid());
    m_diagramProperties->snapToGrid->setChecked(m_pScene-> getSnapToGrid());
    m_diagramProperties->snapComponentSizeToGrid->setChecked(m_pScene-> getSnapComponentSizeToGrid());

    m_diagramProperties->gridSpaceX->setValue( m_pScene -> getSnapX());
    m_diagramProperties->gridSpaceY->setValue( m_pScene -> getSnapY());
    m_diagramProperties->lineWidth->setValue( m_pScene -> getLineWidth());
    m_diagramProperties->documentation->setText(m_pScene -> getDoc());
}

void UMLViewDialog::setupClassPage()
{
    if ( m_pScene -> getType() != Uml::dt_Class ) {
        return;
    }
    QFrame * newPage = new QFrame();
    pageDisplayItem = new KPageWidgetItem( newPage, i18nc("classes display options page", "Display") );
    pageDisplayItem->setHeader( i18n("Classes Display Options") );
    pageDisplayItem->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Display) );
    addPage( pageDisplayItem);

    QHBoxLayout * pOptionsLayout = new QHBoxLayout( newPage );
    m_pOptionsPage = new ClassOptionsPage( newPage, &m_options );
    pOptionsLayout -> addWidget( m_pOptionsPage );
}

void UMLViewDialog::setupColorPage()
{
    QFrame * colorPage = new QFrame();
    pageColorItem = new KPageWidgetItem( colorPage, i18nc("diagram colors page", "Color") );
    pageColorItem->setHeader( i18n("Diagram Colors") );
    pageColorItem->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Color) );
    addPage( pageColorItem);

    QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
    m_pColorPage = new UMLWidgetColorPage( colorPage, &m_options );
    m_pColorLayout -> addWidget(m_pColorPage);
}

void UMLViewDialog::setupFontPage()
{
    KVBox *page = new KVBox();
    pageFontItem = new KPageWidgetItem( page, i18n("Font") );
    pageFontItem->setHeader( i18n("Font Settings") );
    pageFontItem->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Font) );
    addPage( pageFontItem);

    m_pChooser = new KFontChooser( (QWidget*)page, false, QStringList(), false);
    m_pChooser -> setFont( m_pScene -> getOptionState().uiState.font );
}

void UMLViewDialog::applyPage( KPageWidgetItem*item )
{
    if ( item==pageGeneralItem )
    {
        checkName();
        // [PORT]
        // m_pScene->activeView()->setZoom( m_diagramProperties->zoom->value() );
        m_pScene->setDoc( m_diagramProperties->documentation->text() );
        m_pScene->setSnapX( m_diagramProperties->gridSpaceX->value() );
        m_pScene->setSnapY( m_diagramProperties->gridSpaceY->value() );
        m_pScene->setLineWidth( m_diagramProperties->lineWidth->value() );
        m_pScene->setSnapToGrid( m_diagramProperties->snapToGrid->isChecked() );
        m_pScene->setSnapComponentSizeToGrid( m_diagramProperties->snapComponentSizeToGrid->isChecked() );
        m_pScene->setShowSnapGrid( m_diagramProperties->showGrid->isChecked() );
        m_pScene->setShowOpSig( m_diagramProperties->showOpSigs->isChecked() );
    }
    else if ( item == pageColorItem )
    {
        m_pColorPage->updateUMLWidget();
        m_pScene->setUseFillColor( m_options.uiState.useFillColor );
        m_pScene->setLineColor( m_options.uiState.lineColor );
        m_pScene->setFillColor( m_options.uiState.fillColor );
    }
    else if ( item == pageFontItem )
    {
        uDebug() << "setting font " << m_pChooser->font().toString() << endl;
        m_pScene->setFont( m_pChooser->font(), true );
    }
    else if ( item == pageDisplayItem )
    {
        if ( m_pScene->getType() != Uml::dt_Class ) {
            return;
        }
        m_pOptionsPage->updateUMLWidget();
        m_pScene->setClassWidgetOptions( m_pOptionsPage );
        //       sig = m_pTempWidget->getShowOpSigs();
        //       showSig = !( sig == Uml::st_NoSig || sig == Uml::st_NoSigNoVis );
        //       options.classState.showOpSig = showSig;
        //       sig = m_pTempWidget->getShowAttSigs();
        //       showSig = !( sig == Uml::st_NoSig || sig == Uml::st_NoSigNoVis );
        //       options.classState.showAttSig = showSig;
        m_pScene->setOptionState( m_options );
    }
}

void UMLViewDialog::checkName()
{
    QString name = m_diagramProperties->diagramName-> text();
    UMLDoc * pDoc = UMLApp::app()->getDocument();
    UMLScene * pScene = pDoc->findView(m_pScene->getType(), name)->umlScene();
    if ( name.length() == 0 ) {
        KMessageBox::sorry(this, i18n("The name you have entered is invalid."),
                           i18n("Invalid Name"), false);
        m_diagramProperties->diagramName->setText( m_pScene->getName() );
        return;
    }
    if ( pScene && pScene != m_pScene ) {
        KMessageBox::sorry(this, i18n("The name you have entered is not unique."),
                           i18n("Name Not Unique"), false);
        m_diagramProperties->diagramName->setText( m_pScene->getName() );
        return;
    }
    m_pScene->setName( name );
    // [PORT]
    pDoc->signalDiagramRenamed(m_pScene->activeView());
}


#include "umlviewdialog.moc"
