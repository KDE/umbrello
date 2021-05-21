/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2016 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLOBJECTPRIVATE_H
#define UMLOBJECTPRIVATE_H

class UMLObjectPrivate {
public:
    UMLObjectPrivate() : isSaved(false) { }
    bool isSaved;
};


#endif // UMLOBJECTPRIVATE_H
