/***************************************************************************
                          codegenerationwizard.cpp  -  description
                             -------------------
    begin                : Wed Jul 24 2002
    copyright            : (C) 2002 by Paul Hensgen
    email                : phensgen@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <qdir.h>
#include <qlistview.h>
#include <qfileinfo.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qapplication.h>

#include "codegenerationwizard.h"
#include "codegenerationoptionspage.h"
#include "../classifier.h"
#include "../codegenerator.h"
#include "../uml.h"
#include "../umldoc.h"

CodeGenerationWizard::CodeGenerationWizard(UMLDoc *doc,
        UMLClassifierList *classList,
        Uml::Programming_Language activeLanguage,
        UMLApp *parent, const char *name)
        :CodeGenerationWizardBase((QWidget*)parent,name) {
    m_doc = doc;
    m_app = parent;
    m_availableList -> setAllColumnsShowFocus(true);
    m_availableList -> setResizeMode(QListView::AllColumns);
    m_selectedList  -> setAllColumnsShowFocus(true);
    m_selectedList  -> setResizeMode(QListView::AllColumns);
    m_statusList    -> setAllColumnsShowFocus(true);
    m_statusList    -> setResizeMode(QListView::AllColumns);

    m_CodeGenerationOptionsPage = new CodeGenerationOptionsPage(doc->getCurrentCodeGenerator(),
                                  activeLanguage, this);
    connect( m_CodeGenerationOptionsPage, SIGNAL(languageChanged()), this, SLOT(changeLanguage()) );

    insertPage(m_CodeGenerationOptionsPage, i18n("Code Generation Options"), 1);

    UMLClassifierList cList;

    if (classList == NULL) {
        cList = m_doc->getClassesAndInterfaces();
        classList = &cList;
    }
    for (UMLClassifier *c = classList->first(); c ; c = classList->next()) {
        new QListViewItem( m_selectedList, c->getFullyQualifiedName());
    }

    setNextEnabled(page(0),m_selectedList->childCount() > 0);

    setFinishEnabled(page(2),true);
    finishButton()->disconnect();
    finishButton()->setText(i18n("&Generate"));
    connect(finishButton(),SIGNAL(clicked()),this,SLOT(generateCode()));
    if ( QApplication::reverseLayout() )
    {
        QPixmap tmpPixmap( *m_addButton->pixmap() );
        m_addButton->setPixmap(*m_removeButton->pixmap());
        m_removeButton->setPixmap(tmpPixmap);
    }
}

CodeGenerationWizard::~CodeGenerationWizard() {}


void CodeGenerationWizard::selectClass() {
    if( !m_availableList->selectedItem() ) {
        return;
    }
    QString name = m_availableList->selectedItem()->text(0);
    if( !m_selectedList->findItem( name,0 ) ) {
        new QListViewItem(m_selectedList, name);
    }
    m_availableList->removeItem( m_availableList->selectedItem() );
    setNextEnabled(currentPage(),true);
}

void CodeGenerationWizard::deselectClass() {
    if( !m_selectedList->selectedItem() ) {
        return;
    }
    QString name = m_selectedList->selectedItem()->text(0);
    if( !m_availableList->findItem(name, 0) ) {
        new QListViewItem(m_availableList, name);
    }
    if(m_selectedList->childCount() == 0) {
        setNextEnabled(currentPage(),false);
    }
    m_selectedList->removeItem( m_selectedList->selectedItem() );
}

void CodeGenerationWizard::generateCode() {
    backButton()->setEnabled(false);

    CodeGenerator* codeGenerator = m_app->getGenerator();

    if (codeGenerator) {

        cancelButton()->setEnabled(false);

        connect( codeGenerator, SIGNAL(codeGenerated(UMLClassifier*, bool)),
                 this, SLOT(classGenerated(UMLClassifier*, bool)) );

        UMLClassifierList cList;
        cList.setAutoDelete(false);

        for(QListViewItem *item = m_statusList->firstChild(); item;
                item = item-> nextSibling()) {
            UMLClassifier *concept =  m_doc->findUMLClassifier(item->text(0));
            cList.append(concept);
        }
        codeGenerator->writeCodeToFile(cList);
        finishButton()->setText(i18n("Finish"));
        finishButton()->disconnect();
        connect(finishButton(),SIGNAL(clicked()),this,SLOT(accept()));

    }
}

void CodeGenerationWizard::classGenerated(UMLClassifier* concept, bool generated) {
    QListViewItem* item = m_statusList->findItem( concept->getFullyQualifiedName(), 0 );
    if( !item ) {
        kdError()<<"GenerationStatusPage::Error finding class in list view"<<endl;
    } else if (generated) {
        item->setText( 1, i18n("Code Generated") );
    } else {
        item->setText( 1, i18n("Not Generated") );
    }
}

void CodeGenerationWizard::populateStatusList() {
    m_statusList->clear();
    for(QListViewItem* item = m_selectedList->firstChild(); item; item = item->nextSibling()) {
        new QListViewItem(m_statusList,item->text(0),i18n("Not Yet Generated"));
    }
}

void CodeGenerationWizard::showPage(QWidget *page) {
    if (indexOf(page) == 2)
    {
        // first save the settings to the selected generator policy
        ((CodeGenerationOptionsPage*)QWizard::page(1))->apply();

        // before going on to the final page, check that the output directory exists and is
        // writable

        // get the policy for the current code generator
        CodeGenerationPolicy *policy = m_doc->getCurrentCodeGenerator()->getPolicy();

        // get the output directory path
        QFileInfo info(policy->getOutputDirectory().absPath());
        if(!info.exists())
        {
            if (KMessageBox::questionYesNo(this,
                                           i18n("The folder %1 does not exist. Do you want to create it now?").arg(info.filePath()),
                                           i18n("Output Folder Does Not Exist"), i18n("Create Folder"), i18n("Do Not Create")) == KMessageBox::Yes)
            {
                QDir dir;
                if(!dir.mkdir(info.filePath()))
                {
                    KMessageBox::sorry(this,i18n("The folder could not be created.\nPlease make sure you have write access to its parent folder or select another, valid, folder."),
                                       i18n("Error Creating Folder"));
                    return;
                }
                //else, directory created
            }
            else // do not create output directory
            {
                KMessageBox::information(this,i18n("Please select a valid folder."),
                                         i18n("Output Folder Does Not Exist"));
                return;
            }
        } else {
            //directory exists.. make sure we can write to it
            if(!info.isWritable())
            {
                KMessageBox::sorry(this,i18n("The output folder exists, but it is not writable.\nPlease set the appropriate permissions or choose another folder."),
                                   i18n("Error Writing to Output Folder"));
                return;
            }
            // it exits and we can write... make sure it is a directory
            if(!info.isDir())
            {
                KMessageBox::sorry(this,i18n("%1 does not seem to be a folder. Please choose a valid folder.").arg(info.filePath()),
                                   i18n("Please Choose Valid Folder"));
                return;
            }
        }
    }
    populateStatusList();
    QWizard::showPage(page);
}

CodeGenerator* CodeGenerationWizard::generator() {
    // FIX
    /*
        KLibLoader* loader = KLibLoader::self();
        if(!loader) {
                kdDebug()<<"error getting KLibLoader!"<<endl;
                return 0;
        }

        KLibFactory* fact = loader->factory(info->library.latin1());
        if(!fact) {
                kdDebug()<<"error getting the Factory"<<endl;
                return 0;
        }

        QObject* o=fact->create(m_doc, 0, info->object.latin1());
        if(!o) {
                kdDebug()<<"could not create object"<<endl;
                return 0;
        }

        CodeGenerator* g = (CodeGenerator*)o;
        // g->setDocument(m_doc);
        return g;
    */
    return (CodeGenerator*) NULL;
}

// when we change language, we need to update the codegenoptions page
// language-dependent stuff. THe way to do this is to call its "apply" method.
void CodeGenerationWizard::changeLanguage()
{
    m_app->setActiveLanguage( m_CodeGenerationOptionsPage->getCodeGenerationLanguage() );
    m_CodeGenerationOptionsPage->setCodeGenerator(m_doc->getCurrentCodeGenerator());
    m_CodeGenerationOptionsPage->apply();
}

#include "codegenerationwizard.moc"
