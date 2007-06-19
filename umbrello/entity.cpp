/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "entity.h"
// qt/kde includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
// app includes
#include "entityattribute.h"
#include "uniqueconstraint.h"
#include "foreignkeyconstraint.h"
#include "umldoc.h"
#include "uml.h"
#include "uniqueid.h"
#include "clipboard/idchangelog.h"
#include "dialogs/umlentityattributedialog.h"
#include "dialogs/umluniqueconstraintdialog.h"
#include "dialogs/umlforeignkeyconstraintdialog.h"

UMLEntity::UMLEntity(const QString& name, Uml::IDType id) : UMLClassifier(name, id) {
    init();
}

UMLEntity::~UMLEntity() {
    m_List.clear();
}

bool UMLEntity::operator==( UMLEntity& rhs ) {
    return UMLClassifier::operator==(rhs);
}

void UMLEntity::copyInto(UMLEntity *rhs) const
{
    // call base class copy function
    UMLClassifier::copyInto(rhs);

    // copy local data items
    rhs->m_PrimaryKey = m_PrimaryKey;
}

UMLObject* UMLEntity::clone() const
{
    UMLEntity* clone = new UMLEntity();
    copyInto(clone);

    return clone;
}

void UMLEntity::init() {
    m_BaseType = Uml::ot_Entity;
    m_PrimaryKey = NULL;
    connect( this,  SIGNAL( entityAttributeRemoved( UMLClassifierListItem* ) ),
             this, SLOT( slotEntityAttributeRemoved( UMLClassifierListItem* ) ) );
}

UMLAttribute* UMLEntity::createAttribute(const QString &name /*=null*/, UMLObject *type /*=NULL*/) {
    Uml::IDType id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(Uml::ot_EntityAttribute);
    } else {
        currentName = name;
    }
    const Settings::OptionState optionState = Settings::getOptionState();
    Uml::Visibility scope = optionState.classState.defaultAttributeScope;
    UMLEntityAttribute* newAttribute = new UMLEntityAttribute(this, currentName, id, scope, type);

    int button = QDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating attribute via list view
    while (button==QDialog::Accepted && !goodName && name.isNull()) {
        UMLEntityAttributeDialog attributedialog(0, newAttribute);
        button = attributedialog.exec();
        QString name = newAttribute->getName();

        if(name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if ( findChildObject(name) != NULL ) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        } else {
            goodName = true;
        }
    }

    if (button != QDialog::Accepted) {
        delete newAttribute;
        return NULL;
    }

    addEntityAttribute(newAttribute);

    UMLDoc *umldoc = UMLApp::app()->getDocument();
    umldoc->signalUMLObjectCreated(newAttribute);
    return newAttribute;
}

UMLUniqueConstraint* UMLEntity::createUniqueConstraint(const QString &name ){
    Uml::IDType id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        /**
         *  @todo check parameter
         */
        currentName = uniqChildName(Uml::ot_UniqueConstraint);
    } else {
        currentName = name;
    }

    UMLUniqueConstraint* newUniqueConstraint = new UMLUniqueConstraint(this, currentName, id);

    int button = QDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating attribute via list view
    while (button==QDialog::Accepted && !goodName && name.isNull()) {
        UMLUniqueConstraintDialog uniqueConstraintDialog(0, newUniqueConstraint);
        button = uniqueConstraintDialog.exec();
        QString name = newUniqueConstraint->getName();

        if(name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if ( findChildObject(name) != NULL ) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        } else {
            goodName = true;
        }
    }

    if (button != QDialog::Accepted) {
        delete newUniqueConstraint;
        return NULL;
    }

    addConstraint(newUniqueConstraint);

    UMLDoc *umldoc = UMLApp::app()->getDocument();
    emitModified();
    umldoc->signalUMLObjectCreated(newUniqueConstraint);
    return newUniqueConstraint;

}

UMLForeignKeyConstraint* UMLEntity::createForeignKeyConstraint(const QString &name ){

    Uml::IDType id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(Uml::ot_ForeignKeyConstraint);
    } else {
        currentName = name;
    }

    UMLForeignKeyConstraint* newForeignKeyConstraint = new UMLForeignKeyConstraint(this, currentName, id);

    int button = QDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating attribute via list view
    while (button==QDialog::Accepted && !goodName && name.isNull()) {
        UMLForeignKeyConstraintDialog foreignKeyConstraintDialog(0, newForeignKeyConstraint);
        button = foreignKeyConstraintDialog.exec();
        QString name = newForeignKeyConstraint->getName();

        if(name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if ( findChildObject(name) != NULL ) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        } else {
            goodName = true;
        }
    }

    if (button != QDialog::Accepted) {
        return NULL;
    }

    addConstraint(newForeignKeyConstraint);

    UMLDoc *umldoc = UMLApp::app()->getDocument();
    emitModified();
    umldoc->signalUMLObjectCreated(newForeignKeyConstraint);
    return newForeignKeyConstraint;

}

UMLObject* UMLEntity::addEntityAttribute(const QString& name, Uml::IDType id) {
    UMLEntityAttribute* literal = new UMLEntityAttribute(this, name, id);
    m_List.append(literal);
    UMLObject::emitModified();
    emit entityAttributeAdded(literal);
    connect(literal,SIGNAL(modified()),this,SIGNAL(modified()));
    return literal;
}

bool UMLEntity::addEntityAttribute(UMLEntityAttribute* attribute, IDChangeLog* Log /* = 0*/) {
    QString name = (QString)attribute->getName();
    if (findChildObject(name) == NULL) {
        attribute->parent()->removeChild(attribute);
        this->insertChild(attribute);
        m_List.append(attribute);
        UMLObject::emitModified();
        emit entityAttributeAdded(attribute);
        connect(attribute,SIGNAL(modified()),this,SIGNAL(modified()));
        return true;
    } else if (Log) {
        Log->removeChangeByNewID( attribute->getID() );
        delete attribute;
    }
    return false;
}

bool UMLEntity::addEntityAttribute(UMLEntityAttribute* attribute, int position) {
    QString name = (QString)attribute->getName();
    if (findChildObject(name) == NULL) {
        attribute->parent()->removeChild(attribute);
        this->insertChild(attribute);
        if ( position >= 0 && position <= (int)m_List.count() )  {
            m_List.insert(position,attribute);
        } else {
            m_List.append(attribute);
        }
        UMLObject::emitModified();
        emit entityAttributeAdded(attribute);
        connect(attribute,SIGNAL(modified()),this,SIGNAL(modified()));
        return true;
    }
    return false;
}

int UMLEntity::removeEntityAttribute(UMLClassifierListItem* literal) {
    if (!m_List.remove((UMLEntityAttribute*)literal)) {
        kDebug() << "can't find att given in list" << endl;
        return -1;
    }
    emit entityAttributeRemoved(literal);
    UMLObject::emitModified();
    // If we are deleting the object, then we don't need to disconnect..this is done auto-magically
    // for us by QObject. -b.t.
    // disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    delete literal;
    return m_List.count();
}

int UMLEntity::entityAttributes() {
    UMLClassifierListItemList entityAttributes = getFilteredList(Uml::ot_EntityAttribute);
    return entityAttributes.count();
}

void UMLEntity::signalEntityAttributeRemoved(UMLClassifierListItem *eattr) {
    emit entityAttributeRemoved(eattr);
}

bool UMLEntity::resolveRef() {
    bool success = UMLClassifier::resolveRef();
    for (UMLObjectListIt oit(m_List); oit.current(); ++oit) {
        UMLObject* obj = oit.current();
        if (obj->resolveRef()) {
            UMLClassifierListItem *cli = static_cast<UMLClassifierListItem*>(obj);
            if (cli->getBaseType() == Uml::ot_EntityAttribute)
                emit entityAttributeAdded(cli);
        }
    }
    return success;
}

void UMLEntity::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement entityElement = UMLObject::save("UML:Entity", qDoc);
    //save operations
    UMLClassifierListItemList entityAttributes = getFilteredList(Uml::ot_EntityAttribute);
    UMLClassifierListItem* pEntityAttribute = 0;
    for (UMLClassifierListItemListIt it(entityAttributes);
         (pEntityAttribute = it.current()) != NULL; ++it) {
        pEntityAttribute->saveToXMI(qDoc, entityElement);
    }

    UMLClassifierListItemList entityConstraints = getFilteredList(Uml::ot_EntityConstraint);
    foreach(UMLClassifierListItem* cli, entityConstraints) {
        cli->saveToXMI(qDoc,entityElement);
    }

    qElement.appendChild(entityElement);
}

bool UMLEntity::load(QDomElement& element) {
    QDomNode node = element.firstChild();
    while( !node.isNull() ) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (Uml::tagEq(tag, "EntityAttribute")) {   // for backward compatibility
            UMLEntityAttribute* pEntityAttribute = new UMLEntityAttribute(this);
            if( !pEntityAttribute->loadFromXMI(tempElement) ) {
                return false;
            }
            m_List.append(pEntityAttribute);
        } else if ( Uml::tagEq( tag, "UniqueConstraint" ) ) {
            UMLUniqueConstraint* pUniqueConstraint = new UMLUniqueConstraint(this);
            if ( !pUniqueConstraint->loadFromXMI(tempElement) ) {
                return false;
            }
            addConstraint( pUniqueConstraint );
        } else if ( Uml::tagEq( tag,"ForeignKeyConstraint" ) ) {
            UMLForeignKeyConstraint* pForeignKeyConstraint = new UMLForeignKeyConstraint(this);
            if ( !pForeignKeyConstraint->loadFromXMI(tempElement) ) {
                return false;
            }

            addConstraint( pForeignKeyConstraint );
        } else if (tag == "stereotype") {
            kDebug() << "UMLEntity::load(" << m_Name
            << "): losing old-format stereotype." << endl;
        } else {
            kWarning() << "unknown child type in UMLEntity::load" << endl;
        }
        node = node.nextSibling();
    }//end while
    return true;
}


bool UMLEntity::setAsPrimaryKey(UMLUniqueConstraint* uconstr) {
    if ( uconstr == NULL ) {
        kDebug()<< k_funcinfo<<"NULL value passed. To unset a Primary Key use "
                <<"unsetPrimaryKey()"<<endl;
        return false;
    }

    if ( static_cast<UMLEntity*>( uconstr->parent() ) != this ) {

        kDebug()<< k_funcinfo <<"Parent of "<<uconstr->getName()
                <<" doesn't match with current entity"<<endl;
        return false;
    }

    // check if this constraint already exists as a unique constraint for this entity
    UMLUniqueConstraint* uuc = static_cast<UMLUniqueConstraint*>( findChildObjectById( uconstr->getID() ) );
    if ( uuc == NULL ) {
        addConstraint( uconstr );
        uuc = uconstr;
    }

    m_PrimaryKey = uuc;

    uuc->emitModified();
    emitModified();
    return true;
}

void UMLEntity::unsetPrimaryKey() {
    m_PrimaryKey = NULL;
}

bool UMLEntity::hasPrimaryKey() {
    if ( m_PrimaryKey == NULL ) {
        return false;
    }

    return true;
}

bool UMLEntity::addConstraint(UMLEntityConstraint* constr) {
    if ( findChildObjectById( constr->getID() ) != NULL ) {
        kDebug()<< k_funcinfo<<"Constraint with id "<<ID2STR(constr->getID())
                <<" already exists ";
        return false;
    }

    m_List.append( constr );

    UMLObject::emitModified();
    emit entityConstraintAdded(constr);
    connect(constr,SIGNAL(modified()),this,SIGNAL(modified()));

    return true;
}

bool UMLEntity::removeConstraint(UMLEntityConstraint* constr) {
     if ( findChildObjectById( constr->getID() ) == NULL ) {
        kDebug()<< k_funcinfo<<"Constraint with id "<<ID2STR(constr->getID())
                <<" does not exist ";
        return false;
    }

    if ( m_PrimaryKey == constr ) {
        unsetPrimaryKey();
    }

    m_List.remove( constr );

    UMLObject::emitModified();
    emit entityConstraintRemoved(constr);

    delete constr;
    return true;
}



void UMLEntity::slotEntityAttributeRemoved(UMLClassifierListItem* cli){

    // this function does some cleanjobs related to this entity when the attribute is
    // removed, like, removing the attribute from all constraints

    UMLEntityAttribute* entAtt = static_cast<UMLEntityAttribute*>( cli );
    if ( cli ) {
       UMLClassifierListItemList ual = this->getFilteredList(Uml::ot_UniqueConstraint);

       foreach( UMLClassifierListItem* ucli,  ual ) {
           UMLUniqueConstraint* uuc = static_cast<UMLUniqueConstraint*>( ucli );
           if ( uuc->hasEntityAttribute( entAtt ) ) {

               uuc->removeEntityAttribute(entAtt );
           }
       }
    }

}

UMLClassifierListItemList UMLEntity::getFilteredList(Uml::Object_Type ot){

    if ( ot == Uml::ot_EntityConstraint ) {
        UMLClassifierListItemList ucList,fcList, rcList;
        ucList = UMLClassifier::getFilteredList( Uml::ot_UniqueConstraint );
        fcList = UMLClassifier::getFilteredList( Uml::ot_ForeignKeyConstraint );

        // append the two lists to rcList
        // first the Unique Constraints
        foreach( UMLClassifierListItem* ucli, ucList ) {
            rcList.append( ucli );
        }

        // then the Foreign Key Constraints
        foreach( UMLClassifierListItem* ucli, fcList ) {
            rcList.append( ucli );
        }

        return rcList;
    } else {

        return UMLClassifier::getFilteredList( ot );

    }
}

bool UMLEntity::isPrimaryKey(UMLUniqueConstraint* uConstr){
    if ( uConstr == m_PrimaryKey ) {
        return true;
    }

    return false;
}

#include "entity.moc"
