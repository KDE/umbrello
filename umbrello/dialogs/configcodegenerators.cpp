/***************************************************************************
                          configcodegenerators.cpp  -  description
                             -------------------
    begin                : Tue Jun 18 2002
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

#include "configcodegenerators.h"

#include <qlistview.h>
#include <kurl.h>
#include <klibloader.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <qstringlist.h>
#include <qstringlist.h>
#include "selectlanguagesdlg.h"


ConfigCodeGenerators::ConfigCodeGenerators(GeneratorInfoDict dict, QWidget *parent)
		:ConfigCodeGenBase(parent) {
	configuration -> setAllColumnsShowFocus(true);
	configuration -> setResizeMode(QListView::AllColumns);

	QDictIterator<GeneratorInfo> it( dict );
	for( it.toFirst(); it.current(); ++it )
		new QListViewItem(configuration,(*it)->language,(*it)->library,(*it)->object);

}

ConfigCodeGenerators::~ConfigCodeGenerators() {}


void ConfigCodeGenerators::addLanguage() {
	QStringList libsFound;
	KStandardDirs stdDirs;
	libsFound = stdDirs.findAllResources("data","umbrello/codegenerators/*.la",false,true);

	//find all code generator libraries installed
	QFileInfo fi;
	for ( QStringList::Iterator it = libsFound.begin(); it != libsFound.end(); ++it ) {
		fi.setFile(*it);        //leav only filename (no path, no extension)
		*it = fi.fileName().left(fi.fileName().length()-3);
	}
	SelectLanguagesDlg d;
	GeneratorInfo *info;
	QDict<GeneratorInfo> dict;
	dict.setAutoDelete(true);

	QListViewItemIterator it( configuration );
	while ( it.current() != 0 ) {
		info = new GeneratorInfo();
		info->language = it.current()->text(0);
		info->library =  it.current()->text(1);
		info->object = it.current()->text(2);
		dict.insert(info->language,info);
		++it;
	}
	d.offerLanguages(libsFound,dict);
	if(d.exec()) {
		configuration -> clear();
		dict = d.selectedLanguages();
		dict.setAutoDelete(true);
		QDictIterator<GeneratorInfo> it( dict );
		for( it.toFirst(); it.current(); ++it )
			new QListViewItem(configuration,(*it)->language,(*it)->library,(*it)->object);
	}
}

void ConfigCodeGenerators::removeLanguage() {
	if(configuration->currentItem())
		delete configuration->currentItem();
}


GeneratorInfoDict ConfigCodeGenerators::configDictionary() {
	//return the configuration.
	GeneratorInfo *info;
	GeneratorInfoDict dict;
	dict.setAutoDelete(false); //this dict is destroyed, but the one who gets assigned
	//to this, keeps the ponters to the GeneratorInfos

	QListViewItemIterator it( configuration );
	while ( it.current() != 0 ) {
		info = new GeneratorInfo();
		info->language = it.current()->text(0);
		info->library =  it.current()->text(1);
		info->object = it.current()->text(2);
		int suffix=2;
		if(dict.find(info->language)) {
			while(dict.find(info->language+":"+QString::number(suffix)))
				suffix++;
			info->language+=":" + QString::number(suffix);
		}
		dict.insert(info->language,info);
		++it;
	}
	return dict;
}

#include "configcodegenerators.moc"
