/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003 Luis De la Parra <lparrab@gmx.net>                 *
 *   copyright (C) 2004-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "refactoringassistant.h"

#include "../umlnamespace.h"
#include "../umldoc.h"
#include "../classifier.h"
#include "../attribute.h"
#include "../operation.h"
#include "../dialogs/classpropdlg.h"
#include "../dialogs/umloperationdialog.h"
#include "../dialogs/umlattributedialog.h"
#include "../object_factory.h"

#include <qpoint.h>
#include <qpopupmenu.h>

#include <typeinfo>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

using std::type_info;


RefactoringAssistant::RefactoringAssistant( UMLDoc *doc, UMLClassifier *obj, QWidget *parent, const char *name ):
        KListView( parent, name ), m_doc( doc )
{
    loadPixmaps();

    setRootIsDecorated( true );
    setAcceptDrops( true );
    setDropVisualizer( false );
    setItemsMovable( true );
    setSelectionModeExt( Single );
    setShowToolTips( true );
    setTooltipColumn( 0 );
    setDragEnabled( true );
    setDropHighlighter( true );
    setFullWidth( true );
    setSorting( -1 );

    addColumn("Name ");

    m_menu = new QPopupMenu(this);

    connect(this,SIGNAL(doubleClicked(QListViewItem*)),this,SLOT(itemExecuted(QListViewItem*)));
    connect(this,SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
            this,SLOT(showContextMenu(KListView*,QListViewItem*,const QPoint&)));

    resize(300,400);

    refactor( obj );
}

RefactoringAssistant::~RefactoringAssistant()
{
    m_umlObjectMap.clear();
    clear();
}

void RefactoringAssistant::refactor( UMLClassifier *obj )
{
    clear();
    m_umlObjectMap.clear();
    m_umlObject = obj;
    if (! m_umlObject )
    {
        return;
    }

    addClassifier( obj, 0, true, true, true );
    QListViewItem *item = firstChild();
    item->setOpen(true);
    for( item = item->firstChild(); item ; item = item->nextSibling() )
        item->setOpen(true);
}


UMLObject* RefactoringAssistant::findUMLObject( const QListViewItem *item )
{
    QListViewItem *i = const_cast<QListViewItem*>(item);
    if( m_umlObjectMap.find(i) == m_umlObjectMap.end() )
    {
        kWarning()<<"RefactoringAssistant::findUMLObject( QListViewItem *item )"
        <<"item with text "<<item->text(0)<<"not found in uml map!"<<endl;
        return 0L;
    }
    return m_umlObjectMap[i];

}

QListViewItem* RefactoringAssistant::findListViewItem( const UMLObject *obj )
{
    UMLObjectMap::iterator end(m_umlObjectMap.end());
    for( UMLObjectMap::iterator it(m_umlObjectMap.begin()) ; it != end ; ++it )
        if( (*it).second == obj )
            return (*it).first;
    kWarning() << "RefactoringAssistant::findListViewItem:"
    << "object id " << ID2STR(obj->getID())
    << "does not have a ListItem" << endl;
    return 0L;
}


void RefactoringAssistant::itemExecuted( QListViewItem *item )
{
    UMLObject *o = findUMLObject( item );
    if(o) editProperties( );
}

void RefactoringAssistant::setVisibilityIcon( QListViewItem *item , const UMLObject *obj )
{
    switch(obj->getVisibility())
    {
      case Uml::Visibility::Public:
        item->setPixmap(0,m_pixmaps.Public);
        break;
      case Uml::Visibility::Protected:
        item->setPixmap(0,m_pixmaps.Protected);
        break;
      case Uml::Visibility::Private:
        item->setPixmap(0,m_pixmaps.Private);
        break;
      case Uml::Visibility::Implementation:
        item->setPixmap(0,m_pixmaps.Implementation);
        break;
        break;
    }
}

void RefactoringAssistant::umlObjectModified( const UMLObject *obj )
{
    if( !obj )
        obj = dynamic_cast<const UMLObject*>(sender());
    QListViewItem *item = findListViewItem( obj );
    if( !item )
        return;
    item->setText( 0, obj->getName() );
    if( typeid(*obj) == typeid(UMLOperation) ||
            typeid(*obj) == typeid(UMLAttribute) )
    {
        setVisibilityIcon( item, obj );
    }
}

void RefactoringAssistant::operationAdded( UMLClassifierListItem *o )
{
    UMLOperation *op = static_cast<UMLOperation*>(o);
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(op->parent());
    if(!c)
    {
        kWarning() << "RefactoringAssistant::operationAdded(" << op->getName()
            << ") - Parent of operation is not a classifier!" << endl;
        return;
    }
    QListViewItem *item = findListViewItem( c );
    if( !item )
    {
        return;
    }
    for( QListViewItem *folder = item->firstChild(); folder; folder = folder->nextSibling() )
    {
        if( folder->text(1) == "operations" )
        {
            item = new KListViewItem( folder, op->getName() );
            m_umlObjectMap[item] = op;
            connect( op, SIGNAL( modified() ), this, SLOT( umlObjectModified() ) );
            setVisibilityIcon( item, op );
            break;
        }
    }
}

void RefactoringAssistant::operationRemoved( UMLClassifierListItem *o )
{
    UMLOperation *op = static_cast<UMLOperation*>(o);
    QListViewItem *item = findListViewItem( op );
    if( !item )
    {
        return;
    }
    disconnect( op, SIGNAL( modified() ), this, SLOT( umlObjectModified() ) );
    m_umlObjectMap.erase(item);
    delete item;
}


void RefactoringAssistant::attributeAdded( UMLClassifierListItem *a )
{
    UMLAttribute *att = static_cast<UMLAttribute*>(a);
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(att->parent());
    if(!c)
    {
        kWarning() << "RefactoringAssistant::attributeAdded(" << att->getName()
            << ") - Parent is not a class!" << endl;
        return;
    }
    QListViewItem *item = findListViewItem( c );
    if( !item )
    {
        return;
    }
    for( QListViewItem *folder = item->firstChild(); folder; folder = folder->nextSibling() )
    {
        if( folder->text(1) == "attributes" )
        {
            item = new KListViewItem( folder, att->getName() );
            m_umlObjectMap[item] = att;
            connect( att, SIGNAL( modified() ), this, SLOT( umlObjectModified() ) );
            setVisibilityIcon( item, att );
            break;
        }
    }
}

void RefactoringAssistant::attributeRemoved( UMLClassifierListItem *a )
{
    UMLAttribute *att = static_cast<UMLAttribute*>(a);
    QListViewItem *item = findListViewItem( att );
    if( !item )
    {
        return;
    }
    disconnect( att, SIGNAL( modified() ), this, SLOT( umlObjectModified() ) );
    m_umlObjectMap.erase(item);
    delete item;
}

void RefactoringAssistant::editProperties( )
{
    QListViewItem *item = selectedItem();
    if( item )
    {
        UMLObject *o = findUMLObject( item );
        if( o ) editProperties( o );
    }
}

void RefactoringAssistant::editProperties( UMLObject *obj )
{
    KDialogBase *dia(0);
    Uml::Object_Type t = obj->getBaseType();
    if (t == Uml::ot_Class || t == Uml::ot_Interface)
    {
        dia = new ClassPropDlg(this,obj,0,true);
    }
    else if (t == Uml::ot_Operation)
    {
        dia = new UMLOperationDialog(this,static_cast<UMLOperation*>(obj));
    }
    else if (t == Uml::ot_Attribute)
    {
        dia = new UMLAttributeDialog(this,static_cast<UMLAttribute*>(obj));
    }
    else
    {
        kWarning()<<"RefactoringAssistant::editProperties( UMLObject *o ) caled for unknown type "<<typeid(*obj).name()<<endl;
        return;
    }
    if( dia && dia->exec() )
    {
        // need to update something?
    }
    delete dia;
}

void RefactoringAssistant::showContextMenu(KListView* ,QListViewItem *item, const QPoint &p)
{
    m_menu->clear();
    UMLObject *obj = findUMLObject( item );
    if(obj)
    {// Menu for UMLObjects
        Uml::Object_Type t = obj->getBaseType();
        if (t == Uml::ot_Class)
        {
            m_menu->insertItem(i18n("Add Base Class"),this,SLOT(addBaseClassifier()));
            m_menu->insertItem(i18n("Add Derived Class"),this,SLOT(addDerivedClassifier()));
            //          m_menu->insertItem(i18n("Add Interface Implementation"),this,SLOT(addInterfaceImplementation()));
            m_menu->insertItem(i18n("Add Operation"),this,SLOT(createOperation()));
            m_menu->insertItem(i18n("Add Attribute"),this,SLOT(createAttribute()));
        }
        else if (t == Uml::ot_Interface)
        {
            m_menu->insertItem(i18n("Add Base Interface"),this,SLOT(addSuperClassifier()));
            m_menu->insertItem(i18n("Add Derived Interface"),this,SLOT(addDerivedClassifier()));
            m_menu->insertItem(i18n("Add Operation"),this,SLOT(createOperation()));
        }
        //              else
        //              {
        //              kDebug()<<"No context menu for objects of type "<<typeid(*obj).name()<<endl;
        //              return;
        //              }
        m_menu->insertSeparator();
        m_menu->insertItem(i18n("Properties"),this,SLOT(editProperties()));
    }
    else
    {//menu for other ViewItems
        if( item->text(1) == "operations" )
        {
            m_menu->insertItem(i18n("Add Operation"),this,SLOT(createOperation()));
        }
        else if( item->text(1) == "attributes" )
        {
            m_menu->insertItem(i18n("Add Attribute"),this,SLOT(createAttribute()));
        }
        else
        {
            kWarning()<<"RefactoringAssistant::showContextMenu() "
            <<"called for extraneous item"<<endl;
            return;
        }
    }
    m_menu->exec(p);
}

void RefactoringAssistant::addBaseClassifier()
{
    QListViewItem *item = selectedItem();
    if(!item)
    {
        kWarning()<<"RefactoringAssistant::addBaseClassifier() "
        <<"called with no item selected"<<endl;
        return;
    }
    UMLObject *obj = findUMLObject( item );
    if( !dynamic_cast<UMLClassifier*>(obj) )
    {
        kWarning()<<"RefactoringAssistant::addBaseClassifier() "
        <<"called for a non-classifier object"<<endl;
        return;
    }

    //classes have classes and interfaces interfaces as super/derived classifiers
    Uml::Object_Type t = obj->getBaseType();
    UMLClassifier *super = static_cast<UMLClassifier*>(Object_Factory::createUMLObject(t));
    if(!super)
        return;
    m_doc->createUMLAssociation( obj, super, Uml::at_Generalization );
    //////////////////////   Manually add the classifier to the assitant - would be nicer to do it with
    /////////////////////    a signal, like operations and attributes
    QListViewItem *baseFolder = item->firstChild();
    while( baseFolder->text(0) != i18n("Base Classifiers") )
        baseFolder = baseFolder->nextSibling();
    if(!baseFolder)
    {
        kWarning()<<"Cannot find Base Folder"<<endl;
        return;
    }
    item = new KListViewItem( baseFolder, super->getName() );
    item->setPixmap(0,m_pixmaps.Generalization);
    item->setExpandable( true );
    m_umlObjectMap[item] = super;
    addClassifier( super, item, true, false, true);
    /////////////////////////
}

void RefactoringAssistant::addDerivedClassifier()
{
    QListViewItem *item = selectedItem();
    if(!item)
    {
        kWarning()<<"RefactoringAssistant::addDerivedClassifier() "
        <<"called with no item selected"<<endl;
        return;
    }
    UMLObject *obj = findUMLObject( item );
    if( !dynamic_cast<UMLClassifier*>(obj) )
    {
        kWarning()<<"RefactoringAssistant::addDerivedClassifier() "
        <<"called for a non-classifier object"<<endl;
        return;
    }

    //classes have classes and interfaces interfaces as super/derived classifiers
    Uml::Object_Type t = obj->getBaseType();
    UMLClassifier *derived = static_cast<UMLClassifier*>(Object_Factory::createUMLObject(t));
    if(!derived)
        return;
    m_doc->createUMLAssociation( derived, obj, Uml::at_Generalization );

    //////////////////////   Manually add the classifier to the assitant - would be nicer to do it with
    /////////////////////    a signal, like operations and attributes
    QListViewItem *derivedFolder = item->firstChild();
    while( derivedFolder->text(0) != i18n("Derived Classifiers") )
        derivedFolder = derivedFolder->nextSibling();
    if(!derivedFolder)
    {
        kWarning()<<"Cannot find Derived Folder"<<endl;
        return;
    }
    item = new KListViewItem( derivedFolder, derived->getName() );
    item->setPixmap(0,m_pixmaps.Subclass);
    item->setExpandable( true );
    m_umlObjectMap[item] = derived;
    addClassifier( derived, item, false, true, true);
    /////////////////////////
}

void RefactoringAssistant::addInterfaceImplementation()
{
    kWarning()<<"RefactoringAssistant::addInterfaceImplementation()"
    <<"not implemented... finish addSuperClassifier() first!!"<<endl;
    return;
    //  QListViewItem *item = selectedListViewItem( );
    //  UMLObject *obj = findUMLObject( item );
    //  if( !dynamic_cast<UMLClassifier*>(obj) )
    //          return;
    //  UMLObject *n = Object_Factory::createUMLObject( Uml::ot_Interface) );
    //  if(!n)
    //          return;
    //  m_doc->createUMLAssociation( n, obj, Uml::at_Realization );
    //  //refresh, add classifier to assistant
}

void RefactoringAssistant::createOperation()
{
    QListViewItem *item = selectedItem();
    if(!item)
    {
        kWarning()<<"RefactoringAssistant::createOperation() "
        <<"called with no item selected"<<endl;
        return;
    }
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(findUMLObject( item ));
    if( !c )
        return;
    c->createOperation();
}

void RefactoringAssistant::createAttribute()
{
    QListViewItem *item = selectedItem();
    if(!item)
    {
        kWarning()<<"RefactoringAssistant::createAttribute() "
        <<"called with no item selected"<<endl;
        return;
    }
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(findUMLObject( item ));
    if( !c )
        return;
    c->createAttribute();
}


void RefactoringAssistant::addClassifier( UMLClassifier *classifier, QListViewItem *parent, bool addSuper, bool addSub, bool recurse)
{
    QListViewItem *classifierItem, *item;
    if( parent )
    {
        classifierItem = parent;
    }
    else
    {
        classifierItem= new KListViewItem( this, classifier->getName() );
        m_umlObjectMap[classifierItem] = classifier;
    }

    connect( classifier, SIGNAL( modified() ), this, SLOT( umlObjectModified() ) );

    UMLClassifier *klass = dynamic_cast<UMLClassifier*>(classifier);
    if( klass )
    {// only Classes have attributes...
        connect( classifier, SIGNAL(attributeAdded(UMLClassifierListItem*)),
                 this, SLOT(attributeAdded(UMLClassifierListItem*)));
        connect( classifier, SIGNAL(attributeRemoved(UMLClassifierListItem*)),
                 this, SLOT(attributeRemoved(UMLClassifierListItem*)));

        QListViewItem *attsFolder = new KListViewItem( classifierItem, i18n("Attributes"), "attributes" );
        attsFolder->setPixmap(0,SmallIcon("folder_green_open"));
        attsFolder->setExpandable( true );
        UMLAttributeList atts = klass->getAttributeList();
        for( UMLAttribute *att = atts.first(); att; att = atts.next() )
        {
            attributeAdded( att );
        }

    }

    // add operations
    connect( classifier, SIGNAL(operationAdded(UMLClassifierListItem*)),
             this, SLOT(operationAdded(UMLClassifierListItem*)));
    connect( classifier, SIGNAL(operationRemoved(UMLClassifierListItem*)),
             this, SLOT(operationRemoved(UMLClassifierListItem*)));

    QListViewItem *opsFolder = new KListViewItem( classifierItem, i18n("Operations"), "operations" );
    opsFolder->setPixmap(0,SmallIcon("folder_blue_open"));
    opsFolder->setExpandable( true );
    UMLOperationList ops(classifier->getOpList());
    for( UMLOperation *op = ops.first(); op ; op = ops.next() )
    {
        operationAdded( op );
    }

    //if add parents
    if(addSuper)
    {
        QListViewItem *superFolder = new KListViewItem( classifierItem, i18n("Base Classifiers") );
        superFolder->setExpandable( true );
        UMLClassifierList super = classifier->findSuperClassConcepts();
        for( UMLClassifier *cl = super.first(); cl ; cl = super.next() )
        {
            item = new KListViewItem( superFolder, cl->getName() );
            item->setPixmap(0,m_pixmaps.Generalization);
            item->setExpandable( true );
            m_umlObjectMap[item] = cl;
            if( recurse )
            {
                addClassifier( cl, item, true, false, true);
            }

        }
    }
    if(addSub)
    {
        //add derived classifiers
        QListViewItem *derivedFolder = new KListViewItem( classifierItem, i18n("Derived Classifiers") );
        derivedFolder->setExpandable( true );
        UMLClassifierList derived = classifier->findSubClassConcepts();
        for( UMLClassifier *d = derived.first(); d ; d = derived.next() )
        {
            item = new KListViewItem( derivedFolder, d->getName() );
            item->setPixmap(0,m_pixmaps.Subclass);
            item->setExpandable( true );
            m_umlObjectMap[item] = d;
            if( recurse )
            {
                addClassifier( d, item, false, true, true);
            }

        }
    }
}


bool RefactoringAssistant::acceptDrag(QDropEvent *event) const
{
    //first check if we can accept drops at all, and if the operation
    // is a move within the list itself
    if( !acceptDrops() || !itemsMovable() || (event->source()!=viewport()))
    {
        return false;
    }

    RefactoringAssistant *me = const_cast<RefactoringAssistant*>(this);

    //ok, check if the move is valid
    QListViewItem *movingItem = 0, *afterme = 0, *parentItem = 0;
    me->findDrop(event->pos(), parentItem, afterme);
    for( movingItem = firstChild(); movingItem != 0; movingItem = movingItem->itemBelow() )
    {
        if( movingItem->isSelected() )
            break;
    }
    if(!movingItem || !parentItem)
    {   kDebug()<<"moving/parent items not found - can't accept drag!"<<endl;
        return false;
    }

    UMLObject *movingObject;
    if( !(movingObject = me->findUMLObject(movingItem)) )
    {
        kDebug()<<"Moving object not found in uml map!"<<movingItem->text(0)<<endl;
        return false;
    }
    Uml::Object_Type t = movingObject->getBaseType();
    if (t != Uml::ot_Attribute && t != Uml::ot_Operation)
    {
        kDebug()<<"only operations and attributes are movable! - return false"<<endl;
        return false;
    }

    kDebug()<<"parent item is "<<parentItem->text(0)<<endl;
    UMLObject *parentObject = me->findUMLObject(parentItem);
    if( parentObject && dynamic_cast<UMLClassifier*>(parentObject) )
    {
        //droping to a classifier, ok
    }
    else
    {//parent is not a classifier, so maybe  it's a folder.. check types
        if( (parentItem->text(1) == "operations" && t == Uml::ot_Operation)
                || (parentItem->text(1) == "attributes" && t == Uml::ot_Attribute))
        {
            parentObject = me->findUMLObject( parentItem->parent() );
        }
        else
        {
            kDebug()<<"moving to item "<<parentItem->text(0)<<" -- "<<parentItem->text(1)<<" not valid"<<endl;
            return false;
        }
    }
    if (dynamic_cast<UMLClassifier*>(parentObject) &&
            (t == Uml::ot_Attribute || t == Uml::ot_Operation))
    {
        return true;
    }

    kDebug()<<"how did I get here? return false!!"<<endl;
    return false;
}


void RefactoringAssistant::movableDropEvent (QListViewItem* parentItem, QListViewItem* afterme)
{
    //when dropping on a class, we have to put the item in the appropriate folder!
    UMLObject *movingObject;
    UMLClassifier *newClassifier;
    QListViewItem *movingItem;

    for( movingItem = firstChild(); movingItem != 0; movingItem = movingItem->itemBelow() )
    {
        if( movingItem->isSelected() )
            break;
    }
    if( !movingItem || (movingItem == afterme) || !(movingObject = findUMLObject(movingItem)) )
    {
        kWarning()<<"Moving item not found or dropping after itself or item not found in uml obj map. aborting. (drop had already been accepted)"<<endl;
        return;
    }
    Uml::Object_Type t = movingObject->getBaseType();
    newClassifier = dynamic_cast<UMLClassifier*>( findUMLObject( parentItem ) );
    if(!newClassifier)
    {
        if ((parentItem->text(1) == "operations" && t == Uml::ot_Operation)
                || (parentItem->text(1) == "attributes" && t == Uml::ot_Attribute))
        {
            newClassifier = dynamic_cast<UMLClassifier*>( findUMLObject( parentItem->parent() ) );
        }
        if(!newClassifier)
        {
            kWarning()<<"New parent of object is not a Classifier - Drop had already been accepted - check!"<<endl;
            return;
        }
    }
    if (t == Uml::ot_Operation)
    {kDebug()<<"moving operation"<<endl;
        UMLOperation *op = static_cast<UMLOperation*>(movingObject);
        if(newClassifier->checkOperationSignature(op->getName(), op->getParmList()))
        {
            QString msg = QString(i18n("An operation with that signature already exists in %1.\n")).arg(newClassifier->getName())
                          +
                          QString(i18n("Choose a different name or parameter list." ));
            KMessageBox::error(this, msg, i18n("Operation Name Invalid"), false);
            return;
        }
        UMLClassifier *oldClassifier = dynamic_cast<UMLClassifier*>(op->parent());
        if(oldClassifier)
            oldClassifier->removeOperation( op );
        newClassifier->addOperation( op );
    }
    else if (t == Uml::ot_Attribute)
    {kDebug()<<"moving attribute - not implemented"<<endl;
        //              UMLAttribute *att = static_cast<UMLAttribute*>(movingObject);
        //              if(!newClassifier->checkAttributeSignature(att))
        //              {
        //                      QString msg = QString(i18n("An attribute with that signature already exists in %1.\n")).arg(newClassifier->getName())
        //                              +
        //                            QString(i18n("Choose a different name or parameter list." ));
        //                      KMessageBox::error(this, msg, i18n("Operation Name Invalid"), false);
        //                      return;
        //              }
        //              oldClassifier->removeAttribute( att );
        //              newClassifier->addAttribute( att );
    }
    //emit moved(moving, afterFirst, afterme);
    emit moved();
}

void RefactoringAssistant::loadPixmaps()
{
    KStandardDirs *dirs = KGlobal::dirs();
    QString dataDir = dirs -> findResourceDir( "data", "umbrello/pics/object.png" );
    dataDir += "/umbrello/pics/";

    m_pixmaps.Public.load( dataDir + "CVpublic_var.png" );
    m_pixmaps.Protected.load( dataDir + "CVprotected_var.png" );
    m_pixmaps.Private.load( dataDir + "CVprivate_var.png" );
    m_pixmaps.Implementation.load( dataDir + "CVimplementation_var.png" );
    m_pixmaps.Generalization.load( dataDir + "generalisation.png" );
    m_pixmaps.Subclass.load( dataDir + "uniassociation.png" );


}




#include "refactoringassistant.moc"
