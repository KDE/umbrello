/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cppcodegenerator.h"

#include <QRegExp>

const char * CPPMakefileCodeDocument::DOCUMENT_ID_VALUE = "Makefile_DOC";

CPPMakefileCodeDocument::CPPMakefileCodeDocument ()
{
    setFileName(QLatin1String("Makefile")); // default name
    setFileExtension(QString());
    setID(QLatin1String(DOCUMENT_ID_VALUE)); // default id tag for this type of document
}

CPPMakefileCodeDocument::~CPPMakefileCodeDocument ()
{
}

// we add in our code blocks that describe how to generate
// the project here...
void CPPMakefileCodeDocument::updateContent()
{
    // FIX : fill in content
}

/**
 * @return      QString
 */
QString CPPMakefileCodeDocument::toString () const
{
    return QLatin1String("# cpp make build document");
}

// We overwritten by CPP language implementation to get lowercase path
QString CPPMakefileCodeDocument::getPath () const
{
    QString path = getPackage();

    // Replace all white spaces with blanks
    path = path.simplified();

    // Replace all blanks with underscore
    path.replace(QRegExp(QLatin1String(" ")), QLatin1String("_"));

    path.replace(QRegExp(QLatin1String("\\.")),QLatin1String("/"));
    path.replace(QRegExp(QLatin1String("::")), QLatin1String("/"));

    path = path.toLower();

    return path;
}
