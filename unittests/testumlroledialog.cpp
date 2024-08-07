/*
    SPDX-FileCopyrightText: 2015 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

// app includes
#include "association.h"
#include "stereotype.h"
#include "uml.h"
#include "umlrole.h"
#include "umlroledialog.h"

// qt includes
#include <QApplication>
#include <QtDebug>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    UMLApp app;
    app.setup();

    UMLStereotype object;
    UMLAssociation assoc;
    UMLRole role(&assoc, &object, Uml::RoleType::A);
    QPointer<UMLRoleDialog> dlg = new UMLRoleDialog(nullptr, &role);
    int result = dlg->exec();
    delete dlg;
    return result;
}
