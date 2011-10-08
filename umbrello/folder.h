/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLFOLDER_H
#define UMLFOLDER_H

#include "package.h"
#include "umlviewlist.h"
#include "optionstate.h"

/**
 * This class manages the UMLObjects and UMLViews of a Folder.
 * This class inherits from UMLPackage which contains most
 * of the information.
 *
 * The UMLDoc class allocates a fixed instance of this class for
 * each of the predefined Logical, UseCase, Component, Deployment, and
 * Entity-Relationship folders.  Further instances are created on demand
 * for user folders.
 *
 * @short Non-graphical management of objects and diagrams of a Folder
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLFolder : public UMLPackage
{
    Q_OBJECT
public:
    explicit UMLFolder(const QString & name = QString(), Uml::IDType id = Uml::id_None);
    virtual ~UMLFolder();

    virtual UMLObject* clone() const;

    void setLocalName(const QString& localName);
    QString localName() const;

    void addView(UMLView *view);

    void removeView(UMLView *view);

    void appendViews(UMLViewList& viewList, bool includeNested = true);

    void activateViews();

    UMLView* findView(Uml::IDType id);
    UMLView* findView(Uml::DiagramType type, const QString &name, bool searchAllScopes = true);

    void setViewOptions(const Settings::OptionState& optionState);

    void removeAllViews();

    void setFolderFile(const QString& fileName);
    QString folderFile() const;

    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    friend QDebug operator<<(QDebug out, const UMLFolder& item);

protected:
    void saveContents(QDomDocument& qDoc, QDomElement& qElement);

    void save(QDomDocument& qDoc, QDomElement& qElement);

    bool loadDiagramsFromXMI(QDomNode& diagrams);

    bool loadFolderFile(const QString& path);

    bool load(QDomElement & element);

private:
    QString m_localName;  ///< i18n name, only used for predefined root folders
    /**
     * If m_folderFile is not empty then it contains a file name to which
     * this folder is saved.
     * In this case the folder file acts as a physically separate submodel.
     * What is saved in the main model is not the folder contents but a
     * reference to the folder file.
     */
    QString m_folderFile;
    UMLViewList m_diagrams;
};

#endif
