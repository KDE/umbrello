/***************************************************************************
                          selectlaguagesdlg.cpp  -  description
                             -------------------
    begin                : Mon Jul 15 2002
    copyright            : (C) 2002 by Luis De la Parra
    email                : luis@delaparra.org
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
#include "selectlanguagesdlg.h"

#include <qlistview.h>
#include <klibloader.h>
#include <kdebug.h>

#include "../codegenerator.h"


SelectLanguagesDlg::SelectLanguagesDlg(QWidget *parent, const char *name):
SelectLanguagesBase(parent,name) {
	list -> setAllColumnsShowFocus(true);
	list -> setResizeMode(QListView::AllColumns);
}

SelectLanguagesDlg::~SelectLanguagesDlg() {}


GeneratorInfoDict SelectLanguagesDlg::selectedLanguages( ) {
	GeneratorInfoDict dict;
	dict.setAutoDelete(false); //the receiver is responsible for deleting the Items
	GeneratorInfo *info;
	QListViewItemIterator it( list );
	while ( it.current() != 0 ) {
		if ( ((QCheckListItem*)(it.current()))->isOn()) {
			info = new GeneratorInfo();
			info->language = it.current()->text(0);
			info->library =  it.current()->text(1);
			info->object = it.current()->text(2);
			dict.insert(info->language,info);
		}
		++it;
	}
	return dict;
}


void SelectLanguagesDlg::offerLanguages(const QStringList & libraries, const GeneratorInfoDict &gdict) {
	kdDebug()<<"offering languages..."<<endl;
	KLibLoader *loader = KLibLoader::self();
	if(!loader)
		return;
	CodeGeneratorFactory *fact;
	QStringList languages;
	QCheckListItem *item;
	bool check;
	for(QStringList::ConstIterator libit = libraries.begin(); libit!= libraries.end(); ++libit) {
		fact =(CodeGeneratorFactory*) loader ->factory( (*libit).latin1() );
		if(!fact) {
			kdDebug() << "Error getting factory: error msg :" << loader->lastErrorMessage() << endl;
			continue;
		}
		kdDebug() << "Querying lib for langs" << endl;
		languages = fact->languagesAvailable();

		//select which of the languages offered by this library should be installed (registered)
		//if we dont have a dictionary we select all languages by default, if we have a dictionary
		//we select the languages already installed and let the others unchecked by default

		for ( QStringList::Iterator langit = languages.begin(); langit != languages.end(); ++langit ) {
			check = false;
			item= new QCheckListItem(list,*langit,QCheckListItem::CheckBox);
			item -> setText(1,*libit);
			item->setText(2,fact->generatorName(*langit));
			if( gdict.isEmpty() ) {
				check = true;
			} else {
				QDictIterator<GeneratorInfo> dicit( gdict );
				for(dicit.toFirst() ; dicit.current(); ++dicit ) {
					if(dicit.current()->library == *libit &&
					        dicit.current()->object == fact->generatorName(*langit))
{
kdDebug()<<"Check on generator object selected: "<<dicit.current()->object.ascii()<<endl;
						check = true;
}

				}//end for dicit.toFirst()
			}//end else
			item -> setOn(check);
		}//end for(QStringList::Iterator langit

	}//end for QStringList::ConstIterator libit
}

