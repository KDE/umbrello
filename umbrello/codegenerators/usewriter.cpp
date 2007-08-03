/***************************************************************************
                          usewriter.cpp -  description
                             -------------------
    begin                : Fri Jan 20 2007
    copyright            : (C) 2007 by Thibault <zenithar> Normand
    email                : thibault.normand@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License js published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "usewriter.h"
#include "../association.h"
#include "../classifier.h"
#include "../operation.h"
#include "../umldoc.h"
#include "../attribute.h"

#include <kdebug.h>

#include <qregexp.h>
#include <qtextstream.h>

USEWriter::USEWriter() {
}

USEWriter::~USEWriter() {}


void USEWriter::writeClass(UMLClassifier *c)
{
    if(!c)
    {
        kDebug()<<"Cannot write class of NULL concept!";
        return;
    }

    QString classname = cleanName(c->getName());
    QString fileName = c->getName().lower();

    //find an appropriate name for our file
    fileName = findFileName(c,".use");
    if (fileName.isEmpty())
    {
        emit codeGenerated(c, false);
        return;
    }

    QFile fileuse;
    if(!openFile(fileuse, fileName))
    {
        emit codeGenerated(c, false);
        return;
    }
    QTextStream use(&filejs);

    //////////////////////////////
    //Start generating the code!!
    /////////////////////////////


    //try to find a heading file (license, coments, etc)
    QString str;
    str = getHeadingFile(".use");
    if(!str.isEmpty())
    {
        str.replace(QRegExp("%filename%"),fileName);
        str.replace(QRegExp("%filepath%"),filejs.name());
        js << str << m_endl;
    }


    //Write class Documentation if there is somthing or if force option
    if(forceDoc() || !c->getDoc().isEmpty())
    {
        js << m_endl << "/**" << m_endl;
        js << "  * class " << classname << m_endl;
        js << formatDoc(c->getDoc(),"  * ");
        js << "  */" << m_endl << m_endl;
    }

    js << "class " << classname << m_endl;
    js << "attributes" << m_endl;


    js << m_endl;

    //finish file

    //close files and notfiy we are done
    fileuse.close();
    emit codeGenerated(c, true);
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

/**
 * returns "JavaScript"
 */
Uml::Programming_Language JSWriter::getLanguage() {
    return Uml::pl_Ocl;
}

const QStringList USEWriter::reservedKeywords() const {

    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords << "model"
        << "class";
    }

    return keywords;
}

#include "usewriter.moc"
