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

#include "associationwidgetdatalist.h"
#include "dialogs/settingsdlg.h"
#include "umlnamespace.h"
#include <kurl.h>

// forward declaration of the UML classes
class QDomNode;
class QFile;
class QSplitter;

class KPrinter;

class DocWindow;
class IDChangeLog;
class InfoWidget;
class ObjectWidget;
class UMLListView;
class UMLObject;
class UMLView;
class UMLViewData;
class UMLWidget;

using namespace Uml;

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
  * @version 1.0
  */
class UMLDoc : public QObject {
	Q_OBJECT
public:
	friend class ClassImport;
	friend class UMLDrag;

	/**
	 * Constructor for the fileclass of the application
	 */
	UMLDoc(QWidget *parent, const char *name=0);

	/**
	 * Destructor for the fileclass of the application
	 */
	~UMLDoc();

	/**
	 * adds a view to the document which represents the document
	 * contents. Usually this is your main view.
	 */
	void addView(UMLView *view);

	/**
	 * removes a view from the list of currently connected views
	 */
	void removeView(UMLView *view);

	/**
	 * sets the modified flag for the document after a modifying
	 * action on the view connected to the document.
	 */
	void setModified(bool _m=true);

	/**
	 * returns if the document is modified or not. Use this to
	 * determine if your document needs saving by the user on
	 * closing.
	 */
	bool isModified() {
		return modified;
	};

	/**
	 * "save modified" - asks the user for saving if the document
	 * is modified
	 *
	 * @return whether the document can be closed
	 */
	bool saveModified();

	/**
	 * deletes the document's contents
	 */
	void deleteContents();

	/**
	 * initializes the document generally
	 */
	bool newDocument();

	/**
	 * closes the acutal document
	 */
	void closeDocument();

	/**
	 * loads the document by filename and format and emits the
	 * updateViews() signal
	 */
	bool openDocument(const KURL& url, const char *format=0);

	/**
	 * saves the document under filename and format.
	 */
	bool saveDocument(const KURL& url, const char *format=0);

	/**
	 * returns the KURL of the document
	 */
	const KURL& URL() const;

	/**
	 * sets the URL of the document
	 */
	void setURL(const KURL& url);

	/**
	 *	Sets up the signals needed by the program for it to work.
	 */
	void setupSignals();

	/**
	 *	Gains a link to the list view so that it can set the signals
	 *	up required by this class.
	 */
	void setupListView(UMLListView *lv);

	/**
	 *	Creates a @ref UMLObject of the given type.
	 *
	 *	@param	type	The type of @ref UMLObject to create.  Used to
	 * create Actors, Use Cases and concepts.
	 */
	void createUMLObject(UMLObject_Type type);

	/**
	 *  Creates either an operation or attribute for the parent concept.
	 *
	 *	@param	o			The parent object
	 *	@param	type	The type to create, either an operation or attribute.
	 */
	UMLObject* createUMLObject(UMLObject* o, UMLObject_Type type);

	/**
	 *  Creates either an attribute for the parent concept.
	 *
	 *	@param	type	The type to create, either an operation or attribute.
	 */
	UMLObject* createAttribute(UMLObject* o);

	/**
	 *  Creates either an operation for the parent concept.
	 *
	 *	@param	type	The type to create, either an operation or attribute.
	 */
	UMLObject* createOperation(UMLObject* o);

	/**
	 *	Creates a diagram of the given type.
	 *
	 *	@param	type	The type of diagram to create.
	 *      @param  askForName  If true shows a dialog box asking for name, else uses a default name.
	 */
	void createDiagram(Diagram_Type type, bool askForName = true);

	/**
	 *	Used to rename a document.  This method takes care of everything.
	 * You just need to give the ID of the diagram to the method.
	 *
	 *	@param	id	The ID of the diagram to rename.
	 */
	void renameDiagram(int id);

	/**
	 *	Used to rename a @ref UMLObject.  The @ref UMLObject is to be an
	 *	actor, use case or concept.
	 *
	 *	@param	o	The object to rename.
	 */
	void renameUMLObject(UMLObject *o);

	/**
	 *	Used to rename an operation or attribute of a concept.
	 *
	 *	@param	o	The attribute or operation to rename.
	 */
	void renameChildUMLObject(UMLObject *o);


	/**
	 *	Changes the current view (diagram) to the view with the given ID.
	 *
	 *	@param	id	The ID of the view to change to.
	 */
	void  changeCurrentView(int id);

	/**
	 *	Deletes a diagram from the current file.
	 *
	 *	@param	id	The ID of the diagram to delete.
	 */
	void removeDiagram(int id);

	/**
	 *	Removes an @ref UMLObject from the current file.  If this object
	 *	is being represented on a diagram it will also delete all those
	 *	representations.
	 *
	 *	@param o The object to delete.
	 */
	void removeUMLObject(UMLObject*o);

	/**
	 *	Used to find a reference to a @ref UMLObject by its ID.
	 *
	 *	@param id	The @ref UMLObject to find.
	 *
	 *	@return Returns a reference to the @ref UMLObject found.  If not found
	 *	it returns 0.
	 */
	UMLObject* findUMLObject(int id);

	/**
	 *	Used to find a @ref UMLObject by its' type and name.
	 *
	 *	@param	type	The type of @ref UMLObject to find.
	 *	@param	name	The name of the @ref UMLObject to find.
	 *
	 *	@return Returns a reference to the @ref UMLObject found.  If not found
	 *	it returns 0.
	 */
	UMLObject* findUMLObject(UMLObject_Type type, QString name);

	/**
	 *	Finds a view (diagram) by the ID given to method.
	 *	@param	id	The ID of the view to search for.
	 *
	 *	@return	Returns a reference to the view found.  If not found returns 0.
	 */
	UMLView * findView(int id);

	/**
	 *	Finds a view (diagram) by the type and name given.
	 *
	 *	@param	type	The type of view to find.
	 *	@param	name	The name of the view to find.
	 *
	 *	@return	Returns a reference to the view found.  If not found returns 0.
	 */
	UMLView * findView(Diagram_Type type, QString name);

	/**
	 *	Used to give a unique ID to any sort of object.
	 *
	 *	@return	Returns a unique ID.
	 */
	int getUniqueID() {
		return ++uniqueID;
	}

	/**
	 *	This method is called if you wish to see the properties of a
	 *	@ref UMLObject.  A dialog box will be displayed from which you
	 *	can change the object's properties.
	 *
	 *	@param	o	The object to show properties for.
	 *	@param page	The page to show.
	 *	@param assoc	Whether to show association page.
	 */
	void showProperties(UMLObject *o, int page = 0, bool assoc = false);

	/**
	 * Use to save or load this classes information
	 *
	 *	@param	s	Pointer to the datastream (file) to save/load from.
	 *	@param	archive	If true will save the classes information, else will
	 *                       load the information.
	 */
	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

	virtual bool saveToXMI( QFile &file );

	virtual bool loadFromXMI( QFile & file );

	/**
	 * Ensures the XMI file is a valid UML file
	 * currently only checks for metamodel=UML
	 *
	 *	@param headerNode the <XMI.header> node
	 */
	bool validateXMIHeader(QDomNode& headerNode);

	bool loadUMLObjectsFromXMI( QDomNode & node );

	bool loadDiagramsFromXMI( QDomNode & node );

	/**
	 * This method is called when a child @ref UMLObject has been updated.
	 *	This will cause the document to emit the relevant signals.
	 *
	 *	@param	o	The object that has been updated.
	 */
	void signalChildUMLObjectUpdate(UMLObject *o);

	/**
	 * This method is called when a child @ref UMLObject has been created.
	 *	This will cause the document to emit the relevant signals.
	 *
	 *	@param	o	The object that has been created.
	 */
	void signalChildUMLObjectCreated(UMLObject *o);

	/**
	 *		Signal a view/diagram has been renamed.
	 */
	void signalDiagramRenamed(UMLView * pView );

	/**
	 *	Call to remove all the views (diagrams) in the current file.
	 */
	void removeAllViews();


	/**
	 *	Signal that a UMLObject has been created.
	 *
	 *	@param	o	The object that has been created.
	 */
	void signalUMLObjectCreated(UMLObject * o);

	/**
	 *	Signal that a UMLObject has been changed.
	 *
	 *	@param	o	The object that has been changed.
	 */
	void signalUMLObjectChanged(UMLObject * o);

	/**
	 *	Show a properties dialog for an @ref ObjectWidget.
	 *
	 *	@param	o	The ObjectWidget to represent.
	 */
	void showProperties(ObjectWidget * o);

	/**
	 *	Show a properties dialog for an @ref UMLWidget.
	 *
	 *	@param	o	The UMLWidget to represent.
	 */
	void showProperties(UMLWidget * o);

	/**
	 *	Returns the current view.
	 *
	 *	@return	Returns the current view.
	 */
	UMLView * getCurrentView() {
		return currentView;
	}

	/**
	 *	Returns the list view.
	 *
	 *	@return	Returns the list view.
	 */
	UMLListView * getListView() {
		return listView;
	}

	/**
	 *	Returns a list of concepts
	 *
	 *	@return	Returns a list of concepts
	 */
	QList<UMLConcept> getConcepts();

	/**
	 * Controls the printing of the program.
	 *
	 *	@param	pPrinter	The printer (object) to use.
	 */
	void print(KPrinter * pPrinter);

	/**
	 *  Return the list of views for this document
	 */
	const QList<UMLView> &getViewIterator() const {
		return *pViewList;
	}

	/**
	 * Adds an already created UMLObject to the document, it gets
	 * assigned a new ID, if its name is already in use then the
	 * function appends a number to it to differentiate it from
	 * the others; this number is incremental so if number 1 is in
	 * use then it tries 2 and then 3 and so on
	 */
	bool addUMLObjectPaste(UMLObject* Obj);

	/**
	 * Adds an already created UMLView to the document, it gets
	 * assigned a new ID, if its name is already in use then the
	 * function appends a number to it to differentiate it from
	 * the others; this number is incremental so if number 1 is in
	 * use then it tries 2 and then 3 and so on
	 */
	bool addUMLView(UMLViewData * pViewData );

	/**
	 * Read property of IDChangeLog* m_pChangeLog.
	 */
	virtual IDChangeLog* getChangeLog();

 	/**
	 * Closes a Paste session, Deletes the ChangeLog
	 */
	void endPaste();

	/**
	 * Opens a Paste session, Deletes the Old ChangeLog and
	 * Creates an empty one
	 */
	void beginPaste();

	/**
	 * Assigns a New ID to an Object, and also logs the assignment
	 * to its internal ChangeLog
	 */
	int assignNewID(int OldID);

	/**
	 *   Returns the document window.
	 */
	DocWindow * getDocWindow();

	/**
	 * 	Returns the documentation for the project.
	 */
	QString getDoc() {
		return m_Doc;
	}

	/**
	 * 		Sets the documentation for the project.
	 */
	void setDoc( QString doc ) {
		m_Doc = doc;
	}

	/**
	 * Activate a diagram after loading a new document
	 *
	 * @param viewID View's ID to Activate
	 *
	 * @return Return true on success Activation
	 */
	bool activateView ( int viewID );

	/**
	 * Activate all the diagrams/views after loading so all their
	 *	widgets keep their ids.
	 */
	bool activateAllViews();

	/**
	 * Puts into the list variable a list of associations on all
	 * diagrams that contain the given object.  The view is
	 * given so it doesn't check it's association list to save
	 * time.
	 */
	void getAssciationListAllViews( UMLView * view, UMLObject * object, AssociationWidgetDataList & list );

	/**
	 *  	Passes onto @ref UMLApp that an event equivalent to
	 *  	edit copy has occured.
	 */
	void editCopy();

	/**
	 * 	Passes onto @ref UMLApp that an event
	 * 	equivalent to edit cut has occured.
	 */
	void editCut();

	/**
	 * 	Passes onto @ref UMLApp that an event equivalent to
	 * 	edit paste has occured.
	 */
	void editPaste();

	/**
	 * 	Returns the paste state
	 */
	bool getPasteState();

	/**
	 * 	Returns the state on Cut/Copy
	 */
	bool getCutCopyState();

	/**
	 *   Adds a UMLObject thats already created but doesn't change
	 *   any ids or signal.  Used by the list view.  Use
	 *   AddUMLObjectPaste if pasting.
	 */
	void addUMLObject( UMLObject * object ) {
		objectList.append( object );
	}

	/**
	 *   Returns the splitter widget used between the views and
	 *   the listview.
	 */
	QSplitter* getSplitter();

	/**
	 *   Sets the default settings to the given settings.
	 */
	void settingsChanged(SettingsDlg::OptionState optionState);


	/**
	 * Returns the version of the old UML files.
	 */
	int getFileVersion(void) {return version;};


	QList<UMLView> *pViewList;
	InfoWidget* infoWidget;
	UMLListView* listView;
private:
	/**
	 * Returns a name for the new object, appended with a number
	 * if the default name is taken e.g. new actor, new actor_1
	 * etc
	 */
	QString uniqObjectName(const UMLObject_Type type);

	/**
	 * Returns a name for the new object, appended with a number
	 * if the default name is taken e.g. class diagram, class
	 * diagram_1 etc
	 */
	QString uniqViewName(const Diagram_Type type);

	/**
	 *	Sets up the autosave timer.
	 */
	void initSaveTimer();

	QList<UMLObject> objectList;
	int uniqueID;
	bool modified;
	KURL doc_url;
	UMLView* currentView;

	/**
	 * Contains all the UMLObject id changes of paste session
	 */
	IDChangeLog* m_pChangeLog;

	/**
	 * List of Not yet Activated views
	 */
	QList<UMLView> viewsNotActivated;

	/**
	 * true if the we're loading a new document
	 */
	bool loading;

	/**
	 * 	Documentation for the project.
	 */
	QString m_Doc;

	/**
	 *	Used for autosave
	 */
	QTimer * m_pAutoSaveTimer;

	/**
	 * Stores the version of old UML files.
	 */
	 int version;

public slots:

	/**
	 * calls repaint() on all views connected to the document
	 * object and is called by the view by which the document has
	 * been changed.  As this view normally repaints itself, it is
	 * excluded from the paintEvent.
	 */
	void slotUpdateAllViews(UMLView *sender);

	/**
	 *		Called after a specified time to autosave the document.
	 */
	void slotAutoSave();

signals:
	void sigChildObjectChanged(UMLObject *);
	void sigObjectChanged(UMLObject *);
	void sigDiagramRenamed(int t);
	void sigDiagramChanged(Uml::Diagram_Type);
	void sigUpdateItem(int id);

	void sigChildObjectCreated(UMLObject *);
	void sigDiagramRemoved(int id);

	void sigObjectRemoved(UMLObject *);
	void sigDiagramCreated(int id);

	void sigObjectCreated(UMLObject *);
	void sigWidgetUpdated(UMLObject *);

	/**
	 * Reset the status bar
	 */
	void sigResetStatusbarProgress();

	/*
	 * Set the total range of the progressbar
	 *
	 * @param totalSteps Total range of the progressbar (0..totalSteps)
	 */
	void sigSetStatusbarProgressSteps(int totalSteps);


	/*
	 * Set the total range of the progressbar
	 *
	 * @param totalSteps Total range of the progressbar (0..totalStep
	 */
	void sigSetStatusbarProgress(int stepPosition);

	/**
	 * Write text to the status bar
	 */
	void sigWriteToStatusBar(const QString &text);

	/**
	 * The diagram being displayed has changed.
	 * UMLApp uses this to keep it's menu items state up to date.
	 */
	void sigCurrentViewChanged();
};

#endif // UMLDOC_H
