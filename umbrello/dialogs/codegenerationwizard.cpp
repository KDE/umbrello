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
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codegenerationwizard.h"

// qt/kde includes
#include <QPixmap>
#include <qdir.h>
#include <q3listview.h>
#include <qfileinfo.h>
#include <qapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

// local includes
#include "codegenerationoptionspage.h"
#include "../classifier.h"
#include "../codegenerator.h"
#include "../uml.h"
#include "../umldoc.h"
#include "../folder.h"
#include "../umlentitylist.h"
#include "../entity.h"

CodeGenerationWizard::CodeGenerationWizard(UMLClassifierList *classList)
  : Q3Wizard((QWidget*)UMLApp::app()) {

    setupUi(this);

    m_doc = UMLApp::app()->getDocument();
    m_app = UMLApp::app();
    m_availableList -> setAllColumnsShowFocus(true);
    m_availableList -> setResizeMode(Q3ListView::AllColumns);
    m_selectedList  -> setAllColumnsShowFocus(true);
    m_selectedList  -> setResizeMode(Q3ListView::AllColumns);
    m_statusList    -> setAllColumnsShowFocus(true);
    m_statusList    -> setResizeMode(Q3ListView::AllColumns);

    m_CodeGenerationOptionsPage = new CodeGenerationOptionsPage(this);
    connect( m_CodeGenerationOptionsPage, SIGNAL(languageChanged()), this, SLOT(changeLanguage()) );

    insertPage(m_CodeGenerationOptionsPage, i18n("Code Generation Options"), 1);

    UMLClassifierList cList;

    if (classList == NULL) {
        UMLFolder* currRoot = m_doc->currentRoot();
        Uml::Model_Type type = m_doc->rootFolderType(currRoot);

        switch( type ) {
           case Uml::mt_Logical:
               cList = m_doc->getClassesAndInterfaces();
               break;
           case Uml::mt_EntityRelationship:
               foreach( UMLEntity* ent, m_doc->getEntities() ) {
                   cList.append( ent );
               }
               break;
           default:
               break;
        }
        classList = &cList;
    }

    for (UMLClassifier *c = classList->first(); c ; c = classList->next()) {
        new Q3ListViewItem( m_selectedList, c->getFullyQualifiedName());
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
    moveSelectedItems(m_availableList, m_selectedList);

    if (m_selectedList->childCount() > 0) {
        setNextEnabled(currentPage(), true);
    }
}

void CodeGenerationWizard::deselectClass() {
    moveSelectedItems(m_selectedList, m_availableList);

    if (m_selectedList->childCount() == 0) {
        setNextEnabled(currentPage(), false);
    }
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

        for(Q3ListViewItem *item = m_statusList->firstChild(); item;
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
    Q3ListViewItem* item = m_statusList->findItem( concept->getFullyQualifiedName(), 0 );
    if( !item ) {
        kError()<<"GenerationStatusPage::Error finding class in list view"<<endl;
    } else if (generated) {
        item->setText( 1, i18n("Code Generated") );
    } else {
        item->setText( 1, i18n("Not Generated") );
    }
}

void CodeGenerationWizard::populateStatusList() {
    m_statusList->clear();
    for(Q3ListViewItem* item = m_selectedList->firstChild(); item; item = item->nextSibling()) {
        new Q3ListViewItem(m_statusList,item->text(0),i18n("Not Yet Generated"));
    }
}

void CodeGenerationWizard::showPage(QWidget *page) {
    if (indexOf(page) == 2)
    {
        // first save the settings to the selected generator policy
        ((CodeGenerationOptionsPage*)Q3Wizard::page(1))->apply();

        // before going on to the final page, check that the output directory exists and is
        // writable

        // get the policy for the current code generator
        CodeGenerationPolicy *policy = UMLApp::app()->getCommonPolicy();

        // get the output directory path
        QFileInfo info(policy->getOutputDirectory().absPath());
        if(!info.exists())
        {
            if (KMessageBox::questionYesNo(this,
                                           i18n("The folder %1 does not exist. Do you want to create it now?", info.filePath()),
                                           i18n("Output Folder Does Not Exist"), KGuiItem(i18n("Create Folder")), KGuiItem(i18n("Do Not Create"))) == KMessageBox::Yes)
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
                KMessageBox::sorry(this,i18n("%1 does not seem to be a folder. Please choose a valid folder.", info.filePath()),
                                   i18n("Please Choose Valid Folder"));
                return;
            }
        }
    }
    populateStatusList();
    Q3Wizard::showPage(page);
}

CodeGenerator* CodeGenerationWizard::generator() {
    // FIX
    /*
        KLibLoader* loader = KLibLoader::self();
        if(!loader) {
                kDebug()<<"error getting KLibLoader!"<<endl;
                return 0;
        }

        KLibFactory* fact = loader->factory(info->library.latin1());
        if(!fact) {
                kDebug()<<"error getting the Factory"<<endl;
                return 0;
        }

        QObject* o=fact->create(m_doc, 0, info->object.latin1());
        if(!o) {
                kDebug()<<"could not create object"<<endl;
                return 0;
        }

        CodeGenerator* g = (CodeGenerator*)o;
        // g->setDocument(m_doc);
        return g;
    */
    return (CodeGenerator*) NULL;
}

void CodeGenerationWizard::moveSelectedItems(Q3ListView* fromList, Q3ListView* toList) {
   Q3ListViewItemIterator it(fromList, Q3ListViewItemIterator::Selected);
    while (it.current()) {
        Q3ListViewItem* selectedItem = it.current();

        QString name = selectedItem->text(0);
        if (!toList->findItem(name, 0)) {
            new Q3ListViewItem(toList, name);
        }

        ++it;

        //Removed here because it can't (really, shouldn't) be removed while
        //iterator is pointing to it
        fromList->removeItem(selectedItem);
    }
}

// when we change language, we need to update the codegenoptions page
// language-dependent stuff. THe way to do this is to call its "apply" method.
void CodeGenerationWizard::changeLanguage()
{
    QString plStr = m_CodeGenerationOptionsPage->getCodeGenerationLanguage();
    Uml::Programming_Language pl = Model_Utils::stringToProgLang(plStr);
    m_app->setActiveLanguage(pl);
    /* @todo is this needed? if yes adapt to new scheme
     m_CodeGenerationOptionsPage->setCodeGenerator(m_doc->getCurrentCodeGenerator());
     */
    m_CodeGenerationOptionsPage->apply();
}

#include "codegenerationwizard.moc"
