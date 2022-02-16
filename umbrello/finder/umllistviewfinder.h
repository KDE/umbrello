/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLLISTVIEWFINDER_H
#define UMLLISTVIEWFINDER_H

#include "umlfinder.h"

/**
  * UMLListViewFinder provides finding of items in the tree view
  *
  * @author: Ralf Habacker <ralf.habacker@freenet.de>
  */
class UMLListViewFinder : public UMLFinder
{
public:
    UMLListViewFinder();
    virtual ~UMLListViewFinder();
    virtual int collect(Category category, const QString &text);

    virtual Result displayNext();
    virtual Result displayPrevious();
protected:
    bool showItem(Uml::ID::Type id);
};

#endif // UMLLISTVIEWFINDER_H
