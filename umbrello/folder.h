/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
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

class UMLFolder : public UMLPackage {
    Q_OBJECT
public:
    /**
     * Sets up a Folder.
     *
     * @param name    The name of the Folder.
     * @param id      The unique id of the Folder. A new ID will be generated
     *                if this argument is left away.
     */
    explicit UMLFolder(const QString & name = "", Uml::IDType id = Uml::id_None);

    /**
     * Empty deconstructor.
     */
    virtual ~UMLFolder();

    /**
     * Initializes key variables of the class.
     */
    virtual void init();

    /**
     * Make a clone of this object.
     */
    virtual UMLObject* clone() const;

    /**
     * Set the localized name of this folder.
     * This is set for the predefined root views (Logical,
     * UseCase, Component, Deployment, EntityRelationship,
     * and the Datatypes folder inside the Logical View.)
     */
    void setLocalName(const QString& localName);

    /**
     * Return the localized name of this folder.
     * Only useful for the predefined root folders.
     */
    QString getLocalName();

    /**
     * Add a view to the diagram list.
     */
    void addView(UMLView *view);

    /**
     * Remove a view from the diagram list.
     */
    void removeView(UMLView *view);

    /**
     * Append the views in this folder to the given diagram list.
     *
     * @param viewList       The UMLViewList to which to append the diagrams.
     * @param includeNested  Whether to include diagrams from nested folders
     *                       (default: true.)
     */
    void appendViews(UMLViewList& viewList, bool includeNested = true);

    /**
     * Acivate the views in this folder.
     * "Activation": Some widgets require adjustments after loading from file,
     * those are done here.
     */
    void activateViews();

    /**
     * Seek a view of the given ID in this folder.
     *
     * @param id   ID of the view to find.
     * @return     Pointer to the view if found, NULL if no view found.
     */
    UMLView *findView(Uml::IDType id);

    /**
     * Seek a view by the type and name given.
     *
     * @param type              The type of view to find.
     * @param name              The name of the view to find.
     * @param searchAllScopes   Search in all subfolders (default: true.)
     * @return  Pointer to the view found, or NULL if not found.
     */
    UMLView * findView(Uml::Diagram_Type type, const QString &name, bool searchAllScopes = true);

    /**
     * Set the options for the views in this folder.
     */
    void setViewOptions(const Settings::OptionState& optionState);

    /**
     * Remove all views in this folder.
     */
    void removeAllViews();

    /**
     * Set the folder file name for a separate submodel.
     */
    void setFolderFile(const QString& fileName);

    /**
     * Get the folder file name for a separate submodel.
     */
    QString getFolderFile();

    /**
     * Creates a UML:Model or UML:Package element:
     * UML:Model is created for the predefined fixed folders,
     * UML:Package with stereotype "folder" is created for all else.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
    /**
     * Auxiliary to saveToXMI(): Save the contained objects and diagrams.
     * Can be used regardless of whether saving to the main model file
     * or to an external folder file (see m_folderFile.)
     */
    void saveContents(QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Auxiliary to saveToXMI(): Creates a <UML:Model> element when saving
     * a predefined modelview, or a <UML:Package> element when saving a
     * user created folder. Invokes saveContents() with the newly created
     * element.
     */
    void save(QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Auxiliary to load():
     * Load the diagrams from the "diagrams" in the <XMI.extension>
     */
    bool loadDiagramsFromXMI(QDomNode& diagrams);

    /**
     * Folders in the listview can be marked such that their contents
     * are saved to a separate file.
     * This method loads the separate folder file.
     * CAVEAT: This is not XMI standard compliant.
     * If standard compliance is an issue then avoid folder files.
     *
     * @param path  Fully qualified file name, i.e. absolute directory
     *              plus file name.
     * @return   True for success.
     */
    bool loadFolderFile(const QString& path);

    /**
     * Loads the UML:Component element.
     */
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
