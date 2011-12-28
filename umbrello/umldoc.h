/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLDOC_H
#define UMLDOC_H

// app includes
#include "basictypes.h"
#include "listpopupmenu.h"
#include "optionstate.h"
#include "umlobject.h"
#include "umlobjectlist.h"
#include "umlassociationlist.h"
#include "umlclassifierlist.h"
#include "umlentitylist.h"
#include "umlviewlist.h"
#include "umlstereotypelist.h"
#include "umlpackagelist.h"

// kde includes
#include <kurl.h>

// system includes
#include <typeinfo>

#define ENC_UNKNOWN 0
#define ENC_UNICODE 1
#define ENC_OLD_ENC 2

// forward declarations
class QDomNode;
class QDomElement;
class QPrinter;

class IDChangeLog;
class UMLPackage;
class UMLFolder;
class DiagramPrintPage;

/**
 * UMLDoc provides a document object for a document-view model.
 *
 * The UMLDoc class provides a document object that can be used
 * in conjunction with the classes UMLApp and UMLView to create
 * a document-view model for standard KDE applications based on
 * KApplication and KMainWindow. Thereby, the document object
 * is created by the UMLApp instance and contains the document
 * structure with the according methods for manipulation of the
 * document data by UMLView objects. Also, UMLDoc contains the
 * methods for serialization of the document data from and to
 * files.
 *
 * @author Paul Hensgen   <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLDoc : public QObject
{
    Q_OBJECT
public:

    UMLDoc();
    ~UMLDoc();

    void init();

    void addView(UMLView *view);
    void removeView(UMLView *view , bool enforceOneView = true );
    void setMainViewID(Uml::IDType viewID);
    void changeCurrentView(Uml::IDType id);
    void activateAllViews();
    void removeAllViews();

    void setModified(bool modified = true);
    bool isModified();
    bool saveModified();

    bool newDocument();
    void closeDocument();
    bool openDocument(const KUrl& url, const char *format = 0);
    bool saveDocument(const KUrl& url, const char *format = 0);

    const KUrl& url() const;
    void setUrl(const KUrl& url);

    void setupSignals();

    bool isUnique(const QString &name);
    bool isUnique(const QString &name, UMLPackage *package);

    UMLStereotype* findOrCreateStereotype(const QString &name);

    UMLAssociation* createUMLAssociation(UMLObject *a, UMLObject *b, Uml::AssociationType type);

    void addAssociation(UMLAssociation *assoc);
    void removeAssociation(UMLAssociation *assoc, bool doSetModified = true);
    UMLAssociation * findAssociation(Uml::AssociationType assocType,
                                     const UMLObject *roleAObj,
                                     const UMLObject *roleBObj,
                                     bool *swap = 0);

    UMLView* createDiagram(UMLFolder *folder, Uml::DiagramType type, bool askForName = true);
    void removeDiagram(Uml::IDType id);
    void renameDiagram(Uml::IDType id);

    void removeUMLObject(UMLObject* umlobject);
    void renameUMLObject(UMLObject *o);
    void renameChildUMLObject(UMLObject *o);

    UMLObject* findObjectById(Uml::IDType id);

    UMLObject* findUMLObject(const QString &name,
                             UMLObject::ObjectType type = UMLObject::ot_UMLObject,
                             UMLObject *currentObj = 0);

    UMLObject* findUMLObjectRaw(Uml::ModelType::Value,
                                const QString &name,
                                UMLObject::ObjectType type = UMLObject::ot_UMLObject);

    UMLObject* findUMLObjectRaw(UMLFolder *folder,
                                const QString &name,
                                UMLObject::ObjectType type = UMLObject::ot_UMLObject);

    //:TODO: UMLObject* findObjectByAuxId(const QString &idStr);

    UMLClassifier * findUMLClassifier(const QString &name);

    UMLStereotype * findStereotype(const QString &name);

    UMLView * findView(Uml::IDType id);
    UMLView * findView(Uml::DiagramType type, const QString &name,
                       bool searchAllScopes = false);

    void setName(const QString& name);
    QString name() const;

    Uml::IDType modelID() const;

    static bool tagEq (const QString& tag, const QString& pattern);

    virtual void saveToXMI(QIODevice& file);

    short encoding(QIODevice & file);

    virtual bool loadFromXMI(QIODevice& file, short encode = ENC_UNKNOWN);

    bool validateXMIHeader(QDomNode& headerNode);

    bool loadUMLObjectsFromXMI(QDomElement & element);
    void loadExtensionsFromXMI(QDomNode & node);
    bool loadDiagramsFromXMI(QDomNode & node);

    void signalDiagramRenamed(UMLView * view);
    void signalUMLObjectCreated(UMLObject * o);

    UMLClassifierList concepts(bool includeNested = true);
    UMLClassifierList classes(bool includeNested = true);
    UMLClassifierList classesAndInterfaces(bool includeNested = true);
    UMLEntityList entities(bool includeNested = true);
    UMLClassifierList interfaces(bool includeNested = true);
    UMLFolder * datatypeFolder() const;
    UMLClassifierList datatypes();
    UMLAssociationList associations();
    UMLPackageList packages(bool includeNested = true);

    void print(QPrinter * pPrinter, DiagramPrintPage * selectPage);

    UMLViewList viewIterator();

    bool assignNewIDs(UMLObject* obj);

    bool addUMLObject(UMLObject * object);
    bool addUMLView(UMLView * pView );

    UMLFolder *rootFolder(Uml::ModelType mt);
    Uml::ModelType rootFolderType(UMLObject *obj);

    UMLFolder *currentRoot();
    void setCurrentRoot(Uml::ModelType rootType);

    virtual IDChangeLog* changeLog();

    void beginPaste();
    void endPaste();

    Uml::IDType assignNewID(Uml::IDType oldID);

    void setDocumentation(const QString &doc);
    QString documentation() const;

    void settingsChanged(Settings::OptionState optionState);

    QString uniqueViewName(const Uml::DiagramType type);

    bool loading() const;
    void setLoading(bool state = true);

    bool closing() const;

    void addDefaultDatatypes();
    void createDatatype(const QString &name);

    UMLStereotype * findStereotypeById(Uml::IDType id);
    void addStereotype(UMLStereotype *s);
    void removeStereotype(UMLStereotype *s);
    void addDefaultStereotypes();
    const UMLStereotypeList& stereotypes() const;

    void writeToStatusBar(const QString &text);

    void resolveTypes();

    ListPopupMenu::MenuType popupMenuSelection(QAction* action);

private:
    void initSaveTimer();
    void createDatatypeFolder();

    /**
     * Array of predefined root folders.
     */
    UMLFolder *m_root[Uml::ModelType::N_MODELTYPES];

    /**
     * Predefined root folder for datatypes, contained in
     * m_root[Uml::mt_Logical]
     */
    UMLFolder *m_datatypeRoot;

    /**
     * The UMLDoc is the sole owner of all stereotypes.
     * UMLStereotype instances are reference counted.
     * When an UMLStereotype is no longer referenced anywhere,
     * its refcount drops to zero. It is then removed from the
     * m_stereoList and it is physically deleted.
     */
    UMLStereotypeList m_stereoList;

    QString m_Name; ///< name of this model as stored in the <UML:Model> tag
    Uml::IDType m_modelID; ///< xmi.id of this model in the <UML:Model>
    int m_count;   ///< auxiliary counter for the progress bar
    bool m_modified;
    KUrl m_doc_url;

    /**
     * Contains all the UMLObject id changes of paste session.
     */
    IDChangeLog* m_pChangeLog;

    /**
     * true if the we're loading a new document
     */
    bool m_bLoading;

    /**
     * Documentation for the project.
     */
    QString m_Doc;

    /**
     * Used for autosave
     */
    QTimer * m_pAutoSaveTimer;

    /**
     * Auxiliary to <docsettings> processing
     */
    Uml::IDType m_nViewID;

    /**
     * True when type resolution pass has been executed.
     */
    bool m_bTypesAreResolved;

    /**
     * the context menu on the tabs,
     * plugs into umlview::slotMenuSelection()
     */
    ListPopupMenu* m_pTabPopupMenu;

    /**
     * Auxiliary variable for currentRoot():
     * m_pCurrentRoot is only used if UMLApp::app()->currentView()
     * returns 0.
     */
    UMLFolder * m_pCurrentRoot;

    /**
     * True while closeDocument() is executing.
     */
    bool m_bClosing;

public slots:
    void slotRemoveUMLObject(UMLObject*o);
    void slotAutoSave();
    void slotDiagramPopupMenu(QWidget* umlview, const QPoint& point);

signals:
    void sigDiagramCreated(Uml::IDType id);
    void sigDiagramRemoved(Uml::IDType id);
    void sigDiagramRenamed(Uml::IDType t);
    void sigDiagramChanged(Uml::DiagramType);

    void sigObjectCreated(UMLObject *);
    void sigObjectRemoved(UMLObject *);

    /**
     * Reset the status bar.
     */
    void sigResetStatusbarProgress();

    /**
     * Set the total range of the progressbar.
     *
     * @param totalSteps Total range of the progressbar (0..totalSteps)
     */
    void sigSetStatusbarProgressSteps(int totalSteps);

    /**
     * Set the progress position of the progressbar.
     *
     * @param stepPosition   The step position to set.
     */
    void sigSetStatusbarProgress(int stepPosition);

    /**
     * Write text to the status bar.
     */
    void sigWriteToStatusBar(const QString &text);

    /**
     * The diagram being displayed has changed.
     * UMLApp uses this to keep its menu items state up to date.
     */
    void sigCurrentViewChanged();

};

#endif // UMLDOC_H
