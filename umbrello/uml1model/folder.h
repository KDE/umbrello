/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 *
 * TODO: This whole class needs a relook regarding view/scene.
 */
class UMLFolder : public UMLPackage
{
    Q_OBJECT
public:
    explicit UMLFolder(const QString & name = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~UMLFolder();

    virtual UMLObject* clone() const;

    void setLocalName(const QString& localName);
    QString localName() const;

    void addView(UMLView *view);

    void removeView(UMLView *view);

    void appendViews(UMLViewList& viewList, bool includeNested = true);

    void activateViews();

    UMLView* findView(Uml::ID::Type id);
    UMLView* findView(Uml::DiagramType::Enum type, const QString &name, bool searchAllScopes = true);

    void setViewOptions(const Settings::OptionState& optionState);

    void removeAllViews();

    void setFolderFile(const QString& fileName);
    QString folderFile() const;

    bool showPropertiesDialog(QWidget *parent);

    void saveToXMI1(QXmlStreamWriter& writer);

    friend QDebug operator<<(QDebug out, const UMLFolder& item);

protected:
    void saveContents1(QXmlStreamWriter& writer);

    void save1(QXmlStreamWriter& writer);

    bool loadDiagramsFromXMI1(QDomNode& node);

    bool loadFolderFile(const QString& path);

    bool load1(QDomElement & element);

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

    friend class UMLDoc;
};

#endif
