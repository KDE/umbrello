/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLDOC_H
#define UMLDOC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// system includes
#include <typeinfo>

// qt includes
#include <qdatastream.h>
#include <qmap.h>
#include <qdict.h>
#include <qptrstack.h>

// kde includes
#include <kurl.h>
#include <kdockwidget.h>

// app includes
#include "umlnamespace.h"
#include "optionstate.h"
#include "umlobjectlist.h"
#include "umlassociationlist.h"
#include "umlclassifierlist.h"
#include "umlclasslist.h"
#include "umlinterfacelist.h"
#include "umldatatypelist.h"
#include "umlviewlist.h"
#include "umlattributelist.h"
#include "umlstereotypelist.h"

#define ENC_UNKNOWN 0
#define ENC_UNICODE 1
#define ENC_OLD_ENC 2


// forward declarations
class QDomNode;
class QFile;
class QSplitter;

class KPrinter;

class ClassImport;
class CodeGenerator;
class DocWindow;
class IDChangeLog;
class ObjectWidget;
class UMLWidget;
class UMLOperation;
class UMLPackage;
class UMLEnum;
class UMLApp;

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
  * @author Paul Hensgen	<phensgen@techie.com>
  * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
  */
class UMLDoc : public QObject {
	Q_OBJECT
public:
	/**
	 * Constructor for the fileclass of the application
	 */
	UMLDoc();

	/**
	 * Destructor for the fileclass of the application
	 */
	~UMLDoc();

	/**
	 * Adds a view to the document which represents the document
	 * contents. Usually this is your main view.
	 *
	 * @param view		Pointer to the UMLView to add.
	 */
	void addView(UMLView *view);

	/**
	 * Removes a view from the list of currently connected views.
	 *
	 * @param view		Pointer to the UMLView to remove.
	 * @param enforceOneView	switch to determine if we have a current view or not.
	 *                          most of the time, we DO want this, except when exiting the program.
	 */
	void removeView(UMLView *view , bool enforceOneView = true );

	/**
	 * Sets the modified flag for the document after a modifying
	 * action on the view connected to the document.
	 *
	 * @param _m		The value to set the modified flag to.
	 * @param addToUndo	Whether this is an action which should be
	 *			added to the undo stack.
	 */
	void setModified(bool _m=true, bool addToUndo=true);

	/**
	 * Returns if the document is modified or not. Use this to
	 * determine if your document needs saving by the user on
	 * closing.
	 *
	 * @return	True if this UMLDoc is modified.
	 */
	bool isModified() {
		return m_modified;
	}

	/**
	 * "save modified" - Asks the user for saving if the document
	 * is modified.
	 *
	 * @return	True if document can be closed.
	 */
	bool saveModified();

	/**
	 * Deletes the document's contents.
	 */
	void deleteContents();

	/**
	 * Initializes the document generally.
	 *
	 * @return	True if operation successful.
	 */
	bool newDocument();

	/**
	 * Closes the current document.
	 */
	void closeDocument();

	/**
	 * Loads the document by filename and format and emits the
	 * updateViews() signal.
	 *
	 * @param url		The fileame in KURL format.
	 * @param format	The format (optional.)
	 * @return	True if operation successful.
	 */
	bool openDocument(const KURL& url, const char *format=0);

	/**
	 * Saves the document using the given filename and format.
	 *
	 * @param url		The fileame in KURL format.
	 * @param format	The format (optional.)
	 * @return	True if operation successful.
	 */
	bool saveDocument(const KURL& url, const char *format=0);

	/**
	 * Returns the KURL of the document.
	 *
	 * @return	The KURL of this UMLDoc.
	 */
	const KURL& URL() const;

	/**
	 * Sets the URL of the document.
	 *
	 * @param url		The KURL to set.
	 */
	void setURL(const KURL& url);

	/**
	 * Sets up the signals needed by the program for it to work.
	 */
	void setupSignals();

	/**
	 * Returns true if the given name is unique within its scope.
	 *
	 * @param name		The name to check.
	 * @return	True if name is unique.
	 */
	bool isUnique(QString name);

       /**
         * Returns true if the given name is unique within its scope of given package.
         *
         * @param name          The name to check.
         * @return      True if name is unique.
         */
	bool isUnique(QString name, UMLPackage *package); 

	/**
	 * Creates a @ref UMLObject of the given type.
	 *
	 * @param type		The type of @ref UMLObject to create.
	 *			Used to create Actors, Use Cases and concepts.
	 * @param n		A name to give to the object (optional.)
	 * @param parentPkg	The object's parent package (optional.)
	 * @param prepend	Set true if it is desired to prepend the new
	 *			object to the list of objects.
	 *			The default is to append.
	 */
	UMLObject* createUMLObject(Uml::Object_Type type,
				   const QString &n = QString::null,
				   UMLPackage *parentPkg = NULL,
				   bool prepend = false);
	/**
	 * Creates a @ref UMLObject of the given type.
	 *
	 * @param type	The type of @ref UMLObject to create.
	 *		The type_info is translated to a Object_Type and the
	 *		other createUMLObject method is invoked.
	 */
	UMLObject* createUMLObject(const std::type_info &type);

	/**
	 * Creates either an operation or attribute for the parent concept.
	 *
	 * @param o	The parent concept
	 * @param type	The type to create, either an operation or attribute.
	 * @return	The UMLObject created
	 */
	UMLObject* createChildObject(UMLObject* o, Uml::Object_Type type);

	/**
	 * Creates an operation in the current document.
	 * The new operation is initialized with name, id, etc.
	 * If a method with the given profile already exists in the classifier,
	 * no new method is created and the existing operation is returned.
	 * If no name is provided, or if the params are NULL, an Operation
	 * Dialog is shown to ask the user for a name and parameters.
	 * The operation's signature is checked for validity within the parent
	 * classifier.
	 *
	 * @return The new operation, or NULL if the operation could not be
	 *         created because for example, the user canceled the dialog
	 *         or no appropriate name can be found.
	*/
	UMLOperation* createOperation( UMLClassifier *parent,
				       const QString &name = QString::null,
				       UMLAttributeList *params = NULL);

	/**
	 * Creates an attribute for the parent concept.
	 *
	 * @param o	The parent concept
	 * @param name  An optional name, used by when creating through UMLListView
	 * @return	The UMLAttribute created
	 */
	UMLObject* createAttribute(UMLClass* umlclass, const QString &name = QString::null);

	/**
	 * Creates a template for the parent concept.
	 *
	 * @param o	The parent concept
	 * @return	The UMLTemplate created
	 */
	UMLObject* createTemplate(UMLClass* umlclass);

	/**
	 * Creates an enum literal for the parent enum.
	 *
	 * @param o	The parent enum
	 * @return	The UMLEnum created
	 */
	UMLObject* createEnumLiteral(UMLEnum* umlenum);

	/**
	 * Finds or creates a stereotype for the parent object.
	 */
	UMLStereotype* UMLDoc::findOrCreateStereotype(QString name);

	/**
	 * Creates an operation for the parent concept.
	 *
	 * @param o	The parent concept
	 * @return	The UMLOperation created
	 */
	//UMLObject* createOperation(UMLClassifier* classifier);

	/**
	 * Creates an association between two UMLObjects.
	 * NOTE: this method does not check if the association is valid / legal
	 *
	 * @param a	The UMLObject "A" for the association (source)
	 * @param b	The UMLObject "B" for the association (destination)
	 * @param type	The association's type
	 * @return	The Association created
	 */
	UMLAssociation* createUMLAssociation(UMLObject *a, UMLObject *b, Uml::Association_Type type);
	/**
	 * Adds an existing association to the matching concept in the list of concepts.
	 * The selection of the matching concept depends on the association type:
	 * For generalizations, the assoc is added to the concept that matches role A.
	 * For aggregations and compositions , the assoc is added to the concept
	 * that matches role B.
	 *
	 * @param assoc	The association to add
	 */
	void addAssocToConcepts(UMLAssociation* assoc);

	/**
	 * Adds an association.
	 *
	 * @param pAssoc	Pointer to the UMLAssociation to add.
	 */
	void addAssociation(UMLAssociation *pAssoc);

	/**
	 * Removes an association.
	 *
	 * @param pAssoc	Pointer to the UMLAssociation to remove.
	 */
	void removeAssociation(UMLAssociation *pAssoc);

	/**
	 * Finds an association.
	 *
	 * @param assocType	Type of the UMLAssociation to seek.
	 * @param roleAObj	Pointer to the role A UMLCanvasObject.
	 * @param roleBObj	Pointer to the role B UMLCanvasObject.
	 * @param swap		Optional pointer to boolean.
	 *			The bool is set to true if the assocation
	 *			matched with swapped roles, else it is set
	 *			to false.
	 * @return	Pointer to the UMLAssociation found or NULL if not found.
	 */
	UMLAssociation * findAssociation(Uml::Association_Type assocType,
					 const UMLObject *roleAObj,
					 const UMLObject *roleBObj,
					 bool *swap = NULL);

	/**
	 * Creates a diagram of the given type.
	 *
	 * @param type		The type of diagram to create.
	 * @param askForName	If true shows a dialog box asking for name,
	 *			else uses a default name.
	 */
	void createDiagram(Uml::Diagram_Type type, bool askForName = true);

	/**
	 * Removes an @ref UMLObject from the current file.  If this object
	 * is being represented on a diagram it will also delete all those
	 * representations.
	 *
	 * @param o		Pointer to the UMLObject to delete.
	 */
	void removeUMLObject(UMLObject*o);

	/**
	 * Used to rename a document.  This method takes care of everything.
	 * You just need to give the ID of the diagram to the method.
	 *
	 * @param id	The ID of the diagram to rename.
	 */
	void renameDiagram(Uml::IDType id);

	/**
	 * Returns a list of the predefined programming-language types
	 * supported.
	 *
	 * @return	A string list of the programming language types.
	 */
	QStringList getModelTypes();
	// FIXME: Investigate how to make this language dependent.
	//        Currently only a fixed set of default types is supported.

	/**
	 * Used to rename a @ref UMLObject.  The @ref UMLObject is to be an
	 * actor, use case or concept.
	 *
	 * @param o	The object to rename.
	 */
	void renameUMLObject(UMLObject *o);

	/**
	 * Used to rename an operation or attribute of a concept.
	 *
	 * @param o	The attribute or operation to rename.
	 */
	void renameChildUMLObject(UMLObject *o);


	/**
	 * Changes the current view (diagram) to the view with the given ID.
	 *
	 * @param id		The ID of the view to change to.
	 */
	void  changeCurrentView(Uml::IDType id);

	/**
	 * Deletes a diagram from the current file.
	 *
	 * @param id		The ID of the diagram to delete.
	 */
	void removeDiagram(Uml::IDType id);

	/**
	 * Used to find a reference to a @ref UMLObject by its ID.
	 *
	 * @param id		The @ref UMLObject to find.
	 * @return	Pointer to the UMLObject found, or NULL if not found.
	 */
	UMLObject* findObjectById(Uml::IDType id);

	/**
	 * Used to find a @ref UMLObject by its type and name.
	 *
	 * @param name		The name of the @ref UMLObject to find.
	 * @param type		Object_Type of the object to find (optional.)
	 *			When the given type is ot_UMLObject the type is
	 *			disregarded, i.e. the given name is the only
	 *			search criterion.
	 * @param currentObj	Object relative to which to search (optional.)
	 *			If given then the enclosing scope(s) of this
	 *			object are searched before the global scope.
	 * @return	Pointer to the UMLObject found, or NULL if not found.
	 */
	UMLObject* findUMLObject(QString name,
				 Uml::Object_Type type = Uml::ot_UMLObject,
				 UMLObject *currentObj = NULL);

	/**
	 * Used to find a reference to a @ref UMLObject given its non-numeric
	 * ID string.
	 * Only used for intermediate processing while loading files
	 * containing objects with non-numeric xmi.id's.
	 *
	 * @param idStr		The AuxId for the @ref UMLObject to find.
	 * @return	Pointer to the UMLObject found, or NULL if not found.
	 */
	UMLObject* findObjectByAuxId(QString idStr);

	/**
	 * Used to find a @ref UMLClassifier by its name.
	 *
	 * @param name		The name of the @ref UMLObject to find.
	 */
	UMLClassifier * findUMLClassifier (QString name);

	/**
	 * Finds a UMLStereotype by its name.
	 *
	 * @param name		The name of the UMLStereotype to find.
	 * @return	Pointer to the UMLStereotype found, or NULL if not found.
	 */
	UMLStereotype * findStereotype(QString name);

	/**
	 * Finds a view (diagram) by the ID given to method.
	 *
	 * @param id		The ID of the view to search for.
	 * @return	Pointer to the view found, or NULL if not found.
	 */
	UMLView * findView(Uml::IDType id);

	/**
	 * Finds a view (diagram) by the type and name given.
	 *
	 * @param type		The type of view to find.
	 * @param name		The name of the view to find.
	 * @return	Pointer to the view found, or NULL if not found.
	 */
	UMLView * findView(Uml::Diagram_Type type, QString name);


	/**
	 * Used to give a unique ID to any sort of object.
	 *
	 * @return	A new unique ID.
	 */
	Uml::IDType getUniqueID();

	/**
	 * This method is called if you wish to see the properties of a
	 * @ref UMLObject.  A dialog box will be displayed from which you
	 * can change the object's properties.
	 *
	 * @param o		The object to show properties for.
	 * @param page		The page to show.
	 * @param assoc		Whether to show association page.
	 * @return		True if we modified the object.
	 */
	bool showProperties(UMLObject *o, int page = 0, bool assoc = false);

	/**
	 * This method is called for saving the given model as a XMI file.
	 * It is virtual and calls the corresponding saveToXMI() functions
	 * of the derived classes.
	 *
	 * @param file		The file to be saved to.
	 */
	virtual void saveToXMI(QIODevice& file);

	/**
	 * Checks the given XMI file if it was saved with correct Unicode
	 * encoding set or not.
	 *
	 * @param file		The file to be checked.
	 */
	short getEncoding(QIODevice & file);

	/**
	 * Load a given XMI model from a file. If the encoding of the file
	 * is already known it can be passed to the function. If this info
	 * isn't given, loadFromXMI will check which encoding was used.
	 *
	 * @param file		The file to be loaded.
	 * @param encode	The encoding used.
	 */
	virtual bool loadFromXMI(QIODevice& file, short encode = ENC_UNKNOWN);

	/**
	 * Ensures the XMI file is a valid UML file.
	 * Currently only checks for metamodel=UML.
	 *
	 * @param headerNode	The <XMI.header> node
	 */
	bool validateXMIHeader(QDomNode& headerNode);

	/**
	 * Loads all UML objects from XMI into the current UMLDoc.
	 *
	 * @return	True if operation successful.
	 */
	bool loadUMLObjectsFromXMI( QDomElement & element );

	/**
	 * Loads umbrello specific extensions from XMI to the UMLDoc.
	 * The extension tags are: <docsettings>, <diagrams>, <listview>,
	 * and <codegeneration>.
	 */
	void loadExtensionsFromXMI(QDomNode & node);

	/**
	 * Loads all diagrams from XMI into the current UMLDoc.
	 *
	 * @return	True if operation successful.
	 */
	bool loadDiagramsFromXMI( QDomNode & node );

	/**
	 * Creates a new Classifier UMLObject (attribute, operation, template).
	 * Used by the clipboard when pasteing them.
	 */
//	UMLObject* makeNewClassifierObject(QString type, QDomElement& element);

	/**
	 * Make a new UMLObject, used by loadFromXMI and clipboard paste.
	 */
	UMLObject* makeNewUMLObject(QString type);

	/**
	 * Signal a view/diagram has been renamed.
	 */
	void signalDiagramRenamed(UMLView * pView );

	/**
	 * Call to remove all the views (diagrams) in the current file.
	 */
	void removeAllViews();

	/**
	 * Signal that a UMLObject has been created.
	 *
	 * @param o	The object that has been created.
	 */
	void signalUMLObjectCreated(UMLObject * o);

	/**
	 * Show a properties dialog for an @ref ObjectWidget.
	 *
	 * @param o	The ObjectWidget to represent.
	 * @return	True if we modified the object.
	 */
	bool showProperties(ObjectWidget * o);

	/**
	 * Show a properties dialog for an @ref UMLWidget.
	 *
	 * @param o	The UMLWidget to represent.
	 * @return	True if we modified the object.
	 */
	bool showProperties(UMLWidget * o);

	/**
	 * Returns the current view.
	 *
	 * @return	Pointer to the current UMLView.
	 */
	UMLView * getCurrentView() {
		return m_currentView;
	}

	/**
	 * Returns a list of the concepts in this UMLDoc.
	 *
	 * @param includeNested		Whether to include the concepts from
	 *				nested packages (default: true.)
	 * @return	List of UML concepts.
	 */
	UMLClassifierList getConcepts(bool includeNested = true);

	/**
	 * Returns a list of the classes in this UMLDoc.
	 *
	 * @param includeNested		Whether to include the concepts from
	 *				nested packages (default: true.)
	 * @return	List of UML classes.
	 */
	UMLClassList getClasses(bool includeNested = true);

	/**
	 * Returns a list of the classes and interfaces in this UMLDoc.
	 *
	 * @param includeNested		Whether to include the concepts from
	 *				nested packages (default: true.)
	 * @return	List of UML concepts.
	 */
	UMLClassifierList getClassesAndInterfaces(bool includeNested = true);

	/**
	 * Returns a list of the interfaces in this UMLDoc.
	 *
	 * @param includeNested		Whether to include the concepts from
	 *				nested packages (default: true.)
	 * @return	List of UML interfaces.
	 */
	UMLInterfaceList getInterfaces(bool includeNested = true);

	/**
	 * Returns a list of the datatypes in this UMLDoc.
	 *
	 * @return	List of UML datatypes.
	 */
	UMLDatatypeList getDatatypes();

	/**
	 * Returns a list of the associations in this UMLDoc.
	 *
	 * @return	List of UML associations.
	 */
	UMLAssociationList getAssociations();

	/**
	 * Controls the printing of the program.
	 *
	 * @param pPrinter	The printer (object) to use.
	 */
	void print(KPrinter * pPrinter);

	/**
	 * Return the list of views for this document.
	 *
	 * @return	List of UML views.
	 */
	const UMLViewList &getViewIterator() const {
		return m_ViewList;
	}

	/**
	 * Assigns an already created UMLObject a new ID.
	 * If the object is a classifier then the operations/attributes 
	 * are also assigned new IDs.
	 *
	 * @param Obj		Pointer to the UMLObject to add.
	 * @return	True if operation successful.
	 */
	bool assignNewIDs(UMLObject* Obj);

	/**
	 * Adds an already created UMLView to the document, it gets
	 * assigned a new ID, if its name is already in use then the
	 * function appends a number to it to differentiate it from
	 * the others; this number is incremental so if number 1 is in
	 * use then it tries 2 and then 3 and so on
	 *
	 * @param pView		Pointer to the UMLView to add.
	 * @return	True if operation successful.
	 */
	bool addUMLView(UMLView * pView );

	/**
	 * Read property of IDChangeLog* m_pChangeLog.
	 *
	 * @return	Pointer to the IDChangeLog object.
	 */
	virtual IDChangeLog* getChangeLog();

	/**
	 * Closes a paste session, deletes the ChangeLog.
	 */
	void endPaste();

	/**
	 * Opens a Paste session, deletes the old ChangeLog and
	 * creates an empty one.
	 */
	void beginPaste();

	/**
	 * Assigns a New ID to an Object, and also logs the assignment
	 * to its internal ChangeLog.
	 *
	 * @param OldID		The present ID of the object.
	 * @return	The new ID assigned to the object.
	 */
	Uml::IDType assignNewID(Uml::IDType OldID);

	/**
	 * Returns the documentation for the project.
	 *
	 * @return	The documentation text of this UMLDoc.
	 */
	QString getDoc() const {
		return m_Doc;
	}

	/**
	 * Sets the documentation for the project.
	 *
	 * @param doc		The documentation to set for this UMLDoc.
	 */
	void setDoc( QString doc ) {
		m_Doc = doc;
	}

	/**
	 * Activate all the diagrams/views after loading so all their
	 * widgets keep their IDs.
	 *
	 * @return	True if operation successful.
	 */
	bool activateAllViews();

	/**
	 * Sets the default settings to the given settings.
	 */
	void settingsChanged(Settings::OptionState optionState);


	/**
	 * Returns the version of the old UML files.
	 */
	int getFileVersion(void) {return version;}

	/**
	 * Performs the undo function, loading the document back to the
	 * state is was before the last addToUndoStack()
	 */
	void loadUndoData();

	/**
	 * Performs the redo function, loading the document back to the
	 * state is was before the last undo()
	 */
	void loadRedoData();

	/**
	 * Takes an image of the document and adds it to the UndoStack.
	 * Implemented using the saveToXMI functions.
	 */
	void addToUndoStack();

	/**
	 * Removes all entries from the UndoStack and RedoStack and disables the
	 * undo and redo actions.
	 */
	void clearUndoStack();

	/**
	 * Removes all entries from the RedoStack and disables the
	 * redo action.
	 */
	void clearRedoStack();

	/**
	 * Get the root node for the code generation parameters.
	 */
	QDomElement getCodeGeneratorXMIParams ( QString lang );

	/**
	 * Allow checking to see if saved XMI parameters exist already.
	 */
	bool hasCodeGeneratorXMIParams ( QString lang );

	/**
	 * All the UMLViews (i.e. diagrams)
	 */
	UMLViewList m_ViewList;

	/**
	 * Returns a name for the new object, appended with a number
	 * if the default name is taken e.g. class diagram, class
	 * diagram_1 etc
	 */
	QString uniqViewName(const Uml::Diagram_Type type);

	/**
	 * Returns true when loading a document file.
	 */
	bool loading() const {
		return m_bLoading;
	}

	/**
	 * Return the ClassImport object.
	 */
	ClassImport * classImport();

	/**
	 * Find a code generator by the given language.
	 */
	CodeGenerator * findCodeGeneratorByLanguage (QString lang);

	/**
	 * Add a CodeGenerator object to this UMLDoc
	 */
	bool addCodeGenerator ( CodeGenerator * add_gen );

	/**
	 * Remove and delete a CodeGenerator object from this UMLDoc.
	 * @return boolean - will return false if it couldnt remove a generator.
	 */
	bool removeCodeGenerator ( CodeGenerator * remove_object );

	/** Set the current (active) code generator for this document.
	 */
	void setCurrentCodeGenerator ( CodeGenerator * gen );

	/** Get the current (active) code generator for this document.
	 */
	CodeGenerator* getCurrentCodeGenerator();

	/**
	 * Calls the active code generator to create its default datatypes
	 */
	void addDefaultDatatypes();

	/**
	 * Add a datatype if it doesn't already exist.
	 * Used by code generators and attribute dialogue.
	 */
	void createDatatype(QString name);

	/**
	 * Add a UMLObject to the application.
	 */
	void addObject(UMLObject *o);

	/**
	 * Find a UMLStereotype by its unique ID.
	 */
	UMLStereotype * findStereotypeById(Uml::IDType id);

	/**
	 * Add a UMLStereotype to the application.
	 */
	void addStereotype(const UMLStereotype *s);

	/**
	 * Remove a UMLStereotype from the application.
	 */
	void removeStereotype(const UMLStereotype *s);

	/**
	 * Returns a name for the new object, appended with a number
	 * if the default name is taken e.g. new_actor, new_actor_1
	 * etc.
	 * @param type		The object type.
	 * @param prefix	The prefix to use (optional.)
	 *			If no prefix is given then a type related
	 *			prefix will be chosen internally.
	 */
	QString uniqObjectName(const Uml::Object_Type type, QString prefix = "");

	/**
	 * Write text to the status bar.
	 */
	void writeToStatusBar(const QString &text);

	/**
	 * Setter for m_highestIDforForeignFile.
	 */
	void getHighestIDforForeignFile(int value);

	/**
	 * Getter for m_highestIDforForeignFile.
	 */
	int setHighestIDforForeignFile() const;

private:

	/**
	 * Remove this association from concepts list. This might be
	 * a public method if removeAssociation is removed.
	 */
	void removeAssocFromConcepts(UMLAssociation *assoc);

	/**
	 * Sets up the autosave timer.
	 */
	void initSaveTimer();

	ClassImport * m_classImporter;
	CodeGenerator * m_currentcodegenerator;
	UMLObjectList m_objectList;

	/**
	 * The UMLDoc is the sole owner of all stereotypes.
	 * UMLStereotype instances are reference counted.
	 * When an UMLStereotype is no longer referenced anywhere,
	 * its refcount drops to zero. It is then removed from the
	 * m_stereoList and it is physically deleted.
	 */
	UMLStereotypeList m_stereoList;

	/**
	 * In principle, each model object gets assigned a unique ID.
	 * ("In principle" because there are exceptions - such as UMLRole -
	 * but the aim is to remove those exceptions.)
	 * NOTE: Currently this is an int although Uml::IDType is a string
	 *       (unless ID_USE_INT is defined.) Perhaps it should be changed
	 *       to Uml::IDType but then we need a unique string generator.
	 *       See also UMLView::m_nLocalID.
	 */
	int m_uniqueID;

	int m_count;   ///< auxiliary counter for the progress bar
	bool m_modified;
	KURL m_doc_url;
	UMLView* m_currentView;
	/// Auxiliary variable for loading foreign XMI files
	int m_highestIDforForeignFile;

	/**
	 * A dictionary of the parameters in the save XMI file
	 * sorted by language.
	 */
	QMap<QString, QDomElement> * m_codeGenerationXMIParamMap;

	/**
	 * A dictionary of various code generators we currently have
	 * configured for this UML document.
	 */
	QDict<CodeGenerator> m_codeGeneratorDictionary;

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
	 * Stores the version of old UML files.
	 */
	int version;

	/**
	 * The stack of images of the document added to each time
	 * something is changed.  A QPtrList is used rather than a
	 * QPtrStack to be able to remove the ones off the bottom once
	 * the stack gets too big.
	 */
	QPtrList<QDataStream> undoStack;

	/**
	 * The stack of images of the document added to each time
	 * undo is called.
	 */
	QPtrList<QDataStream> redoStack;

	/**
	 * Auxiliary to <docsettings> processing
	 */
	Uml::IDType m_nViewID;

public slots:

	/**
	 * Adds a UMLObject thats already created but doesn't change
	 * any ids or signal.  Used by the list view.  Use
	 * AddUMLObjectPaste if pasting.
	 */
	void addUMLObject( UMLObject * object );

	void slotRemoveUMLObject(UMLObject*o);


	/**
	 * Calls repaint() on all views connected to the document
	 * object and is called by the view by which the document has
	 * been changed.  As this view normally repaints itself, it is
	 * excluded from the paintEvent.
	 */
	void slotUpdateAllViews(UMLView *sender);

	/**
	 * Called after a specified time to autosave the document.
	 */
	void slotAutoSave();

signals:
	void sigDiagramCreated(Uml::IDType id);
	void sigDiagramRemoved(Uml::IDType id);
	void sigDiagramRenamed(Uml::IDType t);
	void sigDiagramChanged(Uml::Diagram_Type);

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
	 * @param stepPosition	The step position to set.
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
