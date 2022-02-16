/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLSCENEFINDER_H
#define UMLSCENEFINDER_H

#include "umlfinder.h"

class UMLView;

/**
  * UMLListViewFinder provides finding of items in the tree view
  *
  * @author: Ralf Habacker <ralf.habacker@freenet.de>
  */
class UMLSceneFinder : public UMLFinder
{
public:
    explicit UMLSceneFinder(UMLView *view);
    virtual ~UMLSceneFinder();
    virtual int collect(Category category, const QString &text);

    virtual Result displayNext();
    virtual Result displayPrevious();

protected:
    Uml::ID::Type m_id; ///< id of scene
    bool showItem(Uml::ID::Type id);
};

#endif // UMLSCENEFINDER_H
