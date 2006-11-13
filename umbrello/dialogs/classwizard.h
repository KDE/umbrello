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

#ifndef CLASSWIZARD_H
#define CLASSWIZARD_H
//kde includes
#include <kwizard.h>
//app includes
#include "classgenpage.h"

class ClassifierListPage;
class UMLClassifier;
class UMLDoc;

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassWizard : public KWizard {
public:
    /**
     *  Consrtuctor
     */
    ClassWizard( UMLDoc * pDoc );

    /**
     *  Deconstructor
     */
    ~ClassWizard();

protected:

    /**
     *  Overrides the default method.
     */
    void showPage( QWidget * pWidget );

    /**
     *  Overrides the default method.
     */
    void next();

    /**
     *  Overrides the default method.
     */
    void back();

    /**
     *  Overrides the default method.
     */
    void accept();

    /**
    *   Overrides the default method.
    */
    void reject();

    /**
    *   Setup the wizard pages.
    */
    void setupPages();

    /**
    *   Page 1 - General class info
    */
    ClassGenPage * m_pGenPage;

    /**
    *   Page 2 - Class Attributes
    */
    ClassifierListPage* m_pAttPage;

    /**
    *   Page 3 - Class Operations
    */
    ClassifierListPage* m_pOpPage;

    /**
    *   Document currently opened
    */
    UMLDoc * m_pDoc;

    /**
    *   Class to create
    */
    UMLClassifier * m_pClass;

protected slots:
    /**
     * Opens Umbrello handbook
     */
    void help();

};

#endif
