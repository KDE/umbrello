/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CODEGENFACTORY_H
#define CODEGENFACTORY_H

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>

class CodeGenerator;
class UMLDoc;

/**
 * CodeGeneratorFactory lists gives access to the available code generators, their names 
 * and allows them to be created
 *
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class CodeGeneratorFactory {
public:

    CodeGeneratorFactory();
    virtual ~CodeGeneratorFactory();

    /**
     * Returns a QStringList containing the languages offered by this library
     */
    QStringList languagesAvailable();

    /**
     * Returns the name of the generator which implements language l
     */
    QString generatorName(const QString &l);

    //FIXMEnow        virtual QObject* createObject(QObject* parent, const char* name);
    CodeGenerator* createObject(UMLDoc* parent, const char* name);
};

#endif //CODEGENFACTORY_H
