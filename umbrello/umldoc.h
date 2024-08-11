/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLDOC_H
#define UMLDOC_H

// app includes
#include "basictypes.h"
#include "optionstate.h"
#include "umlobject.h"
#include "umlobjectlist.h"
#include "umlassociationlist.h"
#include "umlclassifierlist.h"
#include "umlentitylist.h"
#include "umlviewlist.h"
#include "umlstereotypelist.h"
#include "umlpackagelist.h"

// qt includes
#include <QUrl>
#include <QMap>

// system includes
#include <typeinfo>

#define ENC_UNKNOWN 0
#define ENC_UNICODE 1
#define ENC_WINDOWS 2
#define ENC_OLD_ENC 3

// forward declarations
class QDomNode;
class QDomElement;
class QPrinter;

class IDChangeLog;
class DiagramsModel;
class ObjectsModel;
class StereotypesModel;
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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLDoc : public QObject
{
    Q_OBJECT
public:

    UMLDoc();
    ~UMLDoc();

    void init();

    void addView(UMLView *view);
    void removeView(UMLView *view, bool enforceOneView = true);
    void setMainViewID(Uml::ID::Type viewID);
    void changeCurrentView(Uml::ID::Type id);
    void activateAllViews();
    void removeAllViews();
    void removeAllObjects();

    void setModified(bool modified = true);
    bool isModified() const;
    bool saveModified();

    bool newDocument();
    void closeDocument();
    bool openDocument(const QUrl& url, const char  *format = nullptr);
    bool saveDocument(const QUrl& url, const char  *format = nullptr);
    const QUrl& url() const;
    void setUrl(const QUrl& url);
    void setUrlUntitled();

    void setupSignals();

    bool isUnique(const QString &name) const;
    bool isUnique(const QString &name, UMLPackage *package) const;

    UMLAssociation* createUMLAssociation(UMLObject *a, UMLObject *b, Uml::AssociationType::Enum type);

    void addAssociation(UMLAssociation *assoc);
    void removeAssociation(UMLAssociation *assoc, bool doSetModified = true);
    UMLAssociation * findAssociation(Uml::AssociationType::Enum assocType,
                                     const UMLObject *roleAObj,
                                     const UMLObject *roleBObj,
                                     bool  *swap = nullptr) const;

    QString createDiagramName(Uml::DiagramType::Enum type, bool askForName = true);
    UMLView* createDiagram(UMLFolder *folder,
                           Uml::DiagramType::Enum type,
                           const QString& name,
                           Uml::ID::Type id = Uml::ID::None);

    void removeDiagram(Uml::ID::Type id);
    void removeDiagramCmd(Uml::ID::Type id);
    void renameDiagram(Uml::ID::Type id);

    void removeUMLObject(UMLObject* umlobject, bool deleteObject = false);
    void renameUMLObject(UMLObject *o);
    void renameChildUMLObject(UMLObject *o);

    UMLObject* findObjectById(Uml::ID::Type id);

    UMLObject* findUMLObject(const QString &name,
                             UMLObject::ObjectType type = UMLObject::ot_UMLObject,
                             UMLObject  *currentObj = nullptr);

    UMLObject* findUMLObjectRaw(Uml::ModelType::Enum,
                                const QString &name,
                                UMLObject::ObjectType type = UMLObject::ot_UMLObject);

    UMLObject* findUMLObjectRaw(UMLFolder *folder,
                                const QString &name,
                                UMLObject::ObjectType type = UMLObject::ot_UMLObject);

    UMLObject* findUMLObjectRecursive(Uml::ModelType::Enum,
                                      const QString &name,
                                      UMLObject::ObjectType type = UMLObject::ot_UMLObject);

    UMLObject* findUMLObjectRecursive(UMLFolder *folder,
                                      const QString &name,
                                      UMLObject::ObjectType type = UMLObject::ot_UMLObject);

    UMLClassifier * findUMLClassifier(const QString &name);

    UMLView * findView(Uml::ID::Type id) const;
    UMLView * findView(Uml::DiagramType::Enum type, const QString &name,
                       bool searchAllScopes = false) const;

    void setName(const QString& name);
    QString name() const;

    void setResolution(qreal resolution);
    qreal resolution() const;
    qreal dpiScale() const;

    Uml::ID::Type modelID() const;

    static bool tagEq (const QString& tag, const QString& pattern);

    virtual void saveToXMI(QIODevice& file);

    short encoding(QIODevice & file);

    virtual bool loadFromXMI(QIODevice& file, short encode = ENC_UNKNOWN);

    bool validateXMI1Header(QDomNode& headerNode);

    bool loadUMLObjectsFromXMI(QDomElement & element);
    void loadExtensionsFromXMI1(QDomNode & node);
    bool loadDiagramsFromXMI1(QDomNode & node);

    void signalDiagramRenamed(UMLView * view);
    void signalUMLObjectCreated(UMLObject * o);

    UMLClassifierList concepts(bool includeNested = true) const;
    UMLClassifierList classesAndInterfaces(bool includeNested = true) const;
    UMLEntityList entities(bool includeNested = true) const;
    UMLFolder * datatypeFolder() const;
    UMLClassifierList datatypes(bool includeInactive = false) const;
    UMLDatatype * findDatatype(QString name, bool includeInactive = false);
    UMLAssociationList associations() const;
    UMLPackageList packages(bool includeNested = true, Uml::ModelType::Enum model = Uml::ModelType::Logical) const;

    void print(QPrinter * pPrinter, DiagramPrintPage * selectPage);

    UMLViewList viewIterator() const;
    UMLViewList views(Uml::DiagramType::Enum type = Uml::DiagramType::Undefined) const;

    bool assignNewIDs(UMLObject* obj);

    bool addUMLObject(UMLObject * object);
    bool addUMLView(UMLView * pView);

    UMLFolder *rootFolder(Uml::ModelType::Enum mt) const;
    Uml::ModelType::Enum rootFolderType(UMLObject *obj) const;

    UMLFolder *currentRoot() const;
    void setCurrentRoot(Uml::ModelType::Enum rootType);

    virtual IDChangeLog* changeLog() const;

    void beginPaste();
    void endPaste();

    Uml::ID::Type assignNewID(Uml::ID::Type oldID);

    void setDocumentation(const QString &doc);
    QString documentation() const;

    void settingsChanged(Settings::OptionState &optionState);

    QString uniqueViewName(const Uml::DiagramType::Enum type) const;

    bool loading() const;
    void setLoading(bool state = true);

    bool importing() const;
    void setImporting(bool state = true);

    bool closing() const;

    void addDefaultDatatypes();
    UMLDatatype * createDatatype(const QString &name);
    void removeDatatype(const QString &name);

    UMLStereotype *createStereotype(const QString &name);
    UMLStereotype *findStereotype(const QString &name) const;
    UMLStereotype *findOrCreateStereotype(const QString &name);
    UMLStereotype *findStereotypeById(Uml::ID::Type id) const;
    void addStereotype(UMLStereotype *s);
    void removeStereotype(UMLStereotype *s);
    void addDefaultStereotypes();
    const UMLStereotypeList& stereotypes() const;

    void writeToStatusBar(const QString &text);

    void resolveTypes();
    bool loadDiagrams1();
    void addDiagramToLoad(UMLFolder *folder, QDomNode node);

    DiagramsModel *diagramsModel() const;
    StereotypesModel *stereotypesModel() const;
    ObjectsModel *objectsModel() const;

    void setLoadingError(const QString &text);

private:
    void initSaveTimer();
    void createDatatypeFolder();

    class Private;
    Private *m_d;

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
     * When a UMLStereotype is no longer referenced anywhere,
     * its refcount drops to zero. It is then removed from the
     * m_stereoList and it is physically deleted.
     */
    UMLStereotypeList m_stereoList;

    QString m_Name; ///< name of this model as stored in the <UML:Model> tag
    Uml::ID::Type m_modelID; ///< xmi.id of this model in the <UML:Model>
    int m_count;   ///< auxiliary counter for the progress bar
    bool m_modified;
    QUrl m_doc_url;
    /**
     * Contains all the UMLObject id changes of paste session.
     */
    IDChangeLog* m_pChangeLog;

    /**
     * true if the we're loading a new document
     */
    bool m_bLoading;

    /**
     * true if the we're importing
     */
    bool m_importing;

    /**
     * Documentation for the project.
     */
    QString m_Doc;

    /**
     * Used for autosave
     */
    QTimer * m_pAutoSaveTimer;

    /**
     * Auxiliary to \<docsettings\> processing
     */
    Uml::ID::Type m_nViewID;

    /**
     * True when type resolution pass has been executed.
     */
    bool m_bTypesAreResolved;

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

    DiagramsModel *m_diagramsModel;
    ObjectsModel *m_objectsModel;
    StereotypesModel *m_stereotypesModel;

    /**
     * Holds widgets coordinates resolution.
     * Unit is dpi.
     */
    qreal m_resolution;

    /**
     * Holds diagram xml nodes on loading
     */
    typedef QMap<UMLFolder*, QList<QDomNode>> DiagramsMap;
    DiagramsMap m_diagramsToLoad;

public Q_SLOTS:
    void slotRemoveUMLObject(UMLObject*o);
    void slotAutoSave();
    void slotDiagramPopupMenu(QWidget* umlview, const QPoint& point);

Q_SIGNALS:
    void sigDiagramCreated(Uml::ID::Type id);
    void sigDiagramRemoved(Uml::ID::Type id);
    void sigDiagramRenamed(Uml::ID::Type t);
    void sigDiagramChanged(Uml::DiagramType::Enum);

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
