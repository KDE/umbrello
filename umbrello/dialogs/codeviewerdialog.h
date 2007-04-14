
/***************************************************************************
                          codeviewerdialog.h  -  description
                             -------------------
    begin                : Fri Aug 1 2003
    copyright            : (C) 2003 by Brian Thomas
    email                : brian.thomas@gsfc.nasa.gov
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CODEVIEWERDIALOG_H
#define CODEVIEWERDIALOG_H

#include <qcolor.h>
#include <qmap.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qtextedit.h>
#include "../codeviewerstate.h"
#include "codeviewerdialogbase.h"

class CodeDocument;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QFrame;

class UMLObject;

/** This class is sooo ugly I don't know where to begin. For now, its a prototype
  * that works, and thats all we need. In the future, a re-write is mandated to
  * bring a bit of beauty to this beast. -b.t.
  */
class CodeViewerDialog : public CodeViewerDialogBase
{
    Q_OBJECT
public:

    CodeViewerDialog ( QWidget* parent, CodeDocument * doc, Settings::CodeViewerState state,
                       const char* name = 0, bool modal = false, WFlags fl = 0 );
    ~CodeViewerDialog ();

    /** return the code viewer state */
    Settings::CodeViewerState getState( );

    QString parentDocName;

    /**
     *  Adds a code document to the tabbed output
         */
    void addCodeDocument( CodeDocument * doc);

protected:

    bool close ( bool alsoDelete );

private:

    Settings::CodeViewerState m_state;

    void initGUI ( const char * name );

public slots:

protected slots:

    virtual void languageChange();

};


#endif // CODEVIEWERDIALOG_H
